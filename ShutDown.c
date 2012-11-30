/***********************************************************************\
 *                              ShutDown.c                             *
 *                 Copyright (C) by Stangl Roman, 1993                 *
 *                                                                     *
 * ShutDown/2 is an UPS (uninteruptable power supply) controlled OS/2  *
 * 2.x ShutDown utility. If the line power fails, the UPS supplies the *
 * power for a short period. This utility shuts the system down, if    *
 * the UPS has been working for a user defined period. The status of   *
 * the UPS is controlled via a serial or parallel port.                *
 *                                                                     *
\***********************************************************************/

static char RCSID[]="@(#) $Header: ShutDown.c Version 1.00 06,1993 $ (LBL)";

#include        "ShutDown.h"            /* User include files */

HAB             hab;                    /* Handle of PM anchor block */
HMQ             hmq;                    /* Handle of message queue */
HWND            hwndFrame;              /* Frame handle of window */
HWND            hwndClient;             /* Client handle of window */
HWND            hwndHelp;               /* Help window handle */
UCHAR           *pucSD2Profile;         /* The buffer holding the filename of the
                                           ShutDown.ini profile */
UCHAR           *pucSD2Logfile;         /* The buffer holding the filename of the
                                           ShutDown.log log-file */
UCHAR           *pucSD2Helpfile;        /* The buffer holding the filename of the HLP file */
HPOINTER        hptrIcon[1];            /* If line power fails, blink with two icons */
HINI            hini;                   /* Handle of profile ShutDown.ini */
UPS             Ups;                    /* Profile data */
SWP             swpScreen;              /* Desktop size */
SWP             swpClient;              /* Client size */
BOOL            LinePower=TRUE;         /* True if line power is available */
DATETIME        UPS_Start;              /* Date&Time when line power fails */
DATETIME        UPS_AlertStart;         /* Date&Time when user should be alert */
DATETIME        UPS_AlertShutDown;      /* Date&Time when user supplied session should be started */
DATETIME        UPS_ShutDown;           /* Date&Time when system is shut down by ShutDown/2 */
DATETIME        UPS_Current;            /* Date&Time of running system clock */
ULONG           TimeDiff;               /* Difference in minutes between two DATATIMEs */
BOOL            ProfileError=FALSE;     /* TRUE if an error occured accessing SHUTDOWN.INI */
BOOL            bEditIniFile=FALSE;     /* TRUE allows to edit/save all entryfields of the INI file */
SD2CLIENT       Sd2;                    /* Structure of SD/2's client window position & size */

/*--------------------------------------------------------------------------------------*\
 * The main procedure.                                                                  *
 * Req:                                                                                 *
 *      argc, argv, envp                                                                *
 * Returns:                                                                             *
 *      int ........... Exitcode (0, or errorlevel)                                     *
\*--------------------------------------------------------------------------------------*/
int main(int argc, char *argv[], char *envp[])
{
ULONG   ulCounter;
QMSG    qmsg;                           /* Message queue */
                                        /* Frame creation control flag */
ULONG   flCreate=FCF_TASKLIST | FCF_SYSMENU | FCF_MENU | FCF_TITLEBAR | FCF_ACCELTABLE |
                 FCF_MINBUTTON | FCF_ICON | FCF_BORDER;
HEV     hevSD2;                         /* Handle of SD/2 semaphore */

/*                                                                                      *\
 * Get the full path and filename of the running copy of PC/2 and change the extension  *
 * .EXE into .ini and .log to open the configuration and log file under these names.    *
\*                                                                                      */
pucSD2Profile=malloc(strlen(argv[0])+1);
pucSD2Logfile=malloc(strlen(argv[0])+1);
pucSD2Helpfile=malloc(strlen(argv[0])+1);
strcpy(pucSD2Profile, argv[0]);
strcpy(pucSD2Logfile, argv[0]);
strcpy(pucSD2Helpfile, argv[0]);
strcpy(strchr(pucSD2Profile, '.'), ".ini");
strcpy(strchr(pucSD2Logfile, '.'), ".log");
strcpy(strchr(pucSD2Helpfile, '.'), ".hlp");
bEditIniFile=FALSE;                     /* Don't allow editing/writing of all entryfields to
                                           SHUTDOWN.INI */
for(ulCounter=1; ulCounter<argc; ulCounter++)
    {
    strupr(argv[ulCounter]);            /* Uppercase */
                                        /* If "magic commandline argument" is found allow
                                           editing/writing of all entryfields in the
                                           configuration dialog */
    if((strstr(argv[ulCounter], "/IBMSERVICE")!=NULL) ||
        (strstr(argv[ulCounter], "-IBMSERVICE")!=NULL))
        {
        DosGetDateTime(&UPS_Current);
        UPS_LogfileIO(LF_UPS_PASSWORD_USED, MPFROMP(&UPS_Current));
        bEditIniFile=TRUE;
        }
    }
do
{
                                        /* Initialize anchor block and message queue */
    if(WinStartUp(&hab, &hmq)==FALSE)
        {                               /* Error string beeps but may not be displayed */
        User_Error("Initialization (of anchor block or message queue) failed - exiting...");
        break;
        }
    if(!WinRegisterClass(               /* Register window class */
        hab,                            /* Handle of anchor block */
        (PSZ)SD2_CLASSNAME,             /* Window class name */
        (PFNWP)SD2_MainWindowProc,      /* Address of window procedure */
        CS_SAVEBITS,                    /* Class style */
        0))                             /* Extra window words */
        {                               /* Error string beeps but may not be displayed */
        User_Error("Initialization (of class registration) failed - exiting...");
        break;
        }
    swpScreen.cx=WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
    swpScreen.cy=WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
                                        /* Get profile before 2'nd thread is created */
    ProfileError=Read_Profile(&hini, &hab, &Ups);
    if(ProfileError==FALSE)             /* Display configuration dialog if profile SHUTDOWN.INI
                                           can't be accessed without an error */
        WinPostMsg(hwndClient, WM_COMMAND, MPFROMSHORT(ID_CONFIG), NULL);
    if((hwndFrame=WinCreateStdWindow(   /* Create a standart window */
        HWND_DESKTOP,                   /* DESKTOP is parent */
        0,                              /* Standard window styles */
        &flCreate,                      /* Frame control flags */
        (PSZ)SD2_CLASSNAME,             /* Client window class name */
        "",                             /* No window text */
        0,                              /* No special class style */
        (HMODULE)0,                     /* Ressource is in .EXE file */
        ID_SHUTDOWN,                    /* Frame window identifier */
        &hwndClient)                    /* Client window handle */
        )==NULLHANDLE)
        {                               /* Error string beeps but may not be displayed */
        User_Error("Initialization (of frame window) failed - exiting...");
        break;
        }
/*                                                                                      *\
 * Check if we are allready loaded before by querying a semaphore that is defined the   *
 * first time SD/2 runs.                                                                *
\*                                                                                      */
    if(DosCreateEventSem(               /* Create a semaphore */
        SD2_SEM,                        /* Name */
        &hevSD2,                        /* Handle */
        (ULONG)0,                       /* Named semaphores are allways shared */
        (BOOL32)FALSE))                 /* Initially set */
        {                               /* If an error occurs, either we can't create
                                           the semaphore or it allready exists. We assume
                                           that it exists, meaning SD/2 allready loaded */
        User_Error("SD/2 allready loaded, you can only load one copy of SD/2 simultaneously - exiting...");
        DosExit(EXIT_THREAD, 1);
        }
/*                                                                                      *\
 * Now initilize Help, if it can't be initialized the we get no help but that's no      *
 * reason to terminate.                                                                 *
\*                                                                                      */
    if(WinStartHelp(hab, pucSD2Helpfile, &hwndHelp)==FALSE)
        User_Error("Can't find PC2.HLP, please check HLP file and HELP - ignoring help requests...");
/*                                                                                      *\
 * Position the window centered on the screen, adjust the client size to defaults and   *
 * set the frame size as the client size corrected with border sizes.                   *
\*                                                                                      */
                                        /* Center horizontally & vertically */
    Sd2.swpSD2Client.x=(swpScreen.cx-Sd2.swpSD2Client.cx)/2;
    Sd2.swpSD2Client.y=(swpScreen.cy-(Sd2.swpSD2Client.cy+
        WinQuerySysValue(HWND_DESKTOP, SV_CYMENU)+
        WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR)))/2;
    WinSetWindowText(hwndFrame, "SD/2 - ShutDown/2");
    if(!WinSetWindowPos(                /* Set window postion */
        hwndFrame,                      /* Window handle */
        HWND_TOP,                       /* Window position at background */
                                        /* Window size */
        Sd2.swpSD2Client.x, Sd2.swpSD2Client.y,
        Sd2.swpSD2Client.cx, Sd2.swpSD2Client.cy+WinQuerySysValue(HWND_DESKTOP, SV_CYMENU)+
            WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR),
                                        /* Window control */
        SWP_DEACTIVATE | SWP_SHOW | SWP_MOVE | SWP_SIZE | SWP_MINIMIZE))
        {
        User_Error("Initialization (of window position) failed - exiting...");
        break;
        }
/*                                                                                      *\
 * Here we loop dispatching the messages...                                             *
\*                                                                                      */
    while(WinGetMsg(hab, &qmsg, 0, 0, 0))
        WinDispatchMsg(hab, &qmsg);     /* Dispatch messages to window procedure */
    WinDestroyWindow(hwndFrame);        /* Close window */
} while (FALSE);

if(WinCloseDown(&hwndHelp, &hab, &hmq)==FALSE) DosExit(EXIT_THREAD, 1);
else DosExit(EXIT_THREAD, 0);
return(0);                              /* Dummy, but avoids compiler message */
}

/*--------------------------------------------------------------------------------------*\
 * This procedure is the ShutDown/2 window procedure.                                   *
\*--------------------------------------------------------------------------------------*/
MRESULT EXPENTRY SD2_MainWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
static HPS      hps;

switch(msg)
{
case WM_CREATE:                         /* Create window by WinCreateStdWindow() */
    {
    POINTL      ptlString[TXTBOX_COUNT];
    SIZEL       sizelClient;
    HDC         hdcClient;
    ULONG       counter;
                                        /* First call default window procedure */
    WinDefWindowProc(hwnd, msg, mp1, mp2);
                                        /* Load default values first */
    Sd2.swpSD2Client.cx=WINDOW_SIZE_X;
    Sd2.swpSD2Client.cy=WINDOW_SIZE_Y;
    Sd2.swpSD2Gauge.x=LEFT_X;
    Sd2.swpSD2Gauge.y=DOWN_Y;
    Sd2.swpSD2Gauge.cx=SIZE_X;
    Sd2.swpSD2Gauge.cy=SIZE_Y;
    Sd2.swpSD2CountDown.x=LEFT_X-25;
    Sd2.swpSD2CountDown.y=DOWN_Y-50;
    hps=WinBeginPaint(hwnd, NULLHANDLE, NULL);
                                        /* Query text box */
    if(GpiQueryTextBox(hps, sizeof("Automatic ShutDown in 00:00:00"),
        "Automatic ShutDown in 00:00:00", TXTBOX_COUNT, ptlString))
        {                               /* On no error adjust client size according to textbox */
        if(Sd2.swpSD2Client.cx<(ptlString[TXTBOX_TOPRIGHT].x+20))
            {
            Sd2.swpSD2Client.cx=ptlString[TXTBOX_TOPRIGHT].x+20;
            Sd2.swpSD2Gauge.x=(Sd2.swpSD2Client.cx-SIZE_X)/2;
            }
            Sd2.swpSD2CountDown.x=(Sd2.swpSD2Client.cx-ptlString[TXTBOX_TOPRIGHT].x)/2;
        }
    WinEndPaint(hps);
                                         /* Now obtain a presentation space handle */
    sizelClient.cx=Sd2.swpSD2Client.cx; sizelClient.cy=Sd2.swpSD2Client.cy;
    hdcClient=WinOpenWindowDC(hwnd);
    hps=GpiCreatePS(                    /* Create PS */
        hab,                            /* Handle of anchor-block */
        hdcClient,                      /* Handle of device-context */
        &sizelClient,                   /* PS size */
        PU_PELS | GPIA_ASSOC);          /* PS options */
                                        /* Now load the two pointers used as icons */
    for(counter=IDP_ICON1; counter<=IDP_ICON2; counter++)
       hptrIcon[counter-IDP_ICON1]=WinLoadPointer(HWND_DESKTOP, NULLHANDLE, counter);
                                        /* Create secondary thread */
    UPS_ThreadCreate(TRUE, (PFNTHREAD)UPS_Thread);
    WinInvalidateRect(hwndFrame, NULL, FALSE);
    }
    break;

case WM_PAINT:                          /* Redraw client window */
    {
    HPS         hps;
    RECTL       rc;
    POINTL      ptlString[TXTBOX_COUNT];
    POINTL      ptl;
    ULONG       ticks;
                                        /* Obtain a presentation space */
    hps=WinBeginPaint(hwnd, NULLHANDLE, &rc);
                                        /* Clear with background color */
    WinFillRect(hps, &rc, CLR_BACKGROUND);
                                        /* Draw capacity bar */
    ptl.x=Sd2.swpSD2Gauge.x-3; ptl.y=Sd2.swpSD2Gauge.y-3;
    GpiMove(hps, &ptl);
    GpiSetColor(hps, CLR_BLUE);
    ptl.x+=(Sd2.swpSD2Gauge.cx+6); ptl.y+=(Sd2.swpSD2Gauge.cy+6);
    GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
    GpiSetColor(hps, CLR_DEFAULT);      
                                        /* Make ticks for every 10 percent under bar */
    for(ticks=0; ticks<=SIZE_X; ticks+=SIZE_X/10)
        {
        ptl.x=Sd2.swpSD2Gauge.x-1+ticks; ptl.y=Sd2.swpSD2Gauge.y-15;
        GpiMove(hps, &ptl);
        ptl.x=Sd2.swpSD2Gauge.x+2+ticks; ptl.y=Sd2.swpSD2Gauge.y-5;
        GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
        }
    ptl.y=Sd2.swpSD2Gauge.y-30;         /* Write centered percentages under gauge */
    GpiQueryTextBox(hps, sizeof("100 %"), "100 %", TXTBOX_COUNT, ptlString);
    ptl.x=Sd2.swpSD2Gauge.x-ptlString[TXTBOX_TOPRIGHT].x/2;
    GpiCharStringAt(hps, &ptl, sizeof("100 %"), "100 %");
    GpiQueryTextBox(hps, sizeof("50 %"), "50 %", TXTBOX_COUNT, ptlString);
    ptl.x=Sd2.swpSD2Gauge.x+SIZE_X/2-ptlString[TXTBOX_TOPRIGHT].x/2;
    GpiCharStringAt(hps, &ptl, sizeof("50 %"), "50 %");
    GpiQueryTextBox(hps, sizeof("0 %"), "0 %", TXTBOX_COUNT, ptlString);
    ptl.x=Sd2.swpSD2Gauge.x+SIZE_X-ptlString[TXTBOX_TOPRIGHT].x/2;
    GpiCharStringAt(hps, &ptl, sizeof("0 %"), "0 %");
    WinEndPaint(hps);                   /* End paint */
    WinSendMsg(hwnd, WM_DRAWUPSBAR, MPFROMLONG(0), NULL);
    WinSendMsg(hwnd, WM_DRAWUPSTIME, MPFROMLONG(0), NULL);
    WinSendMsg(hwnd, WM_FLASHICON, NULL, NULL);
    }
    break;

/*                                                                                      *\
 * Draw the UPS bar that displays the percentage of time the UPS has power in.          *
 * (ULONG) MPARAM1 ............ Seconds to shutdown                                     *
\*                                                                                      */
case WM_DRAWUPSBAR:
    {
    POINTL              ptl;
    static ULONG        PercentageOld=0;
    static ULONG        PercentageNew;

    if((LONGFROMMP(mp1)==0) && (LinePower==TRUE))
        {                               /* If line power is there, draw 100 % green bar */
        PercentageNew=0;
        PercentageOld=100;
        }
    if((LONGFROMMP(mp1)==0) && (LinePower==FALSE))
        {                               /* If a repaint is forced, repaint with previous value */
        PercentageNew=PercentageOld;
        PercentageOld=0;
        }
    if(LONGFROMMP(mp1)!=0)              /* New percentage to shutdown, correct for left
                                           to right drawing by subtracting from 100 */
        PercentageNew=100-((LONGFROMMP(mp1))*100)/
            (Ups.IDUS_Hours*60*60+Ups.IDUS_Minutes*60);
    if(PercentageNew!=PercentageOld)    /* Redraw bar if changed */
        {
                                        /* Move to lower/left edge of danger color bar */
        ptl.x=Sd2.swpSD2Gauge.x; ptl.y=Sd2.swpSD2Gauge.y;
        GpiMove(hps, &ptl);
                                        /* Draw danger bar to upper/right edge */
        ptl.x+=PercentageNew*(SIZE_X/100); ptl.y+=SIZE_Y;
        GpiSetColor(hps, CLR_RED);      /* Draw danger color bar */
        GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
        GpiMove(hps, &ptl);             /* Move to lowerr/right edge of power color bar */
        ptl.x=Sd2.swpSD2Gauge.x+SIZE_X; ptl.y=Sd2.swpSD2Gauge.y;
        GpiSetColor(hps, CLR_GREEN);    /* Draw power color bar */
        GpiBox(hps, DRO_OUTLINEFILL, &ptl, 0, 0);
        PercentageOld=PercentageNew;
        }
    }
    break;

/*                                                                                      *\
 * Display the hours and minutes in which the shutdown will occur.                      *
 * (ULONG) MPARAM1 ............ Seconds to shutdown                                     *
\*                                                                                      */
case WM_DRAWUPSTIME:
    {
    POINTL              ptl;
    static UCHAR        ucBuffer[64];
    static ULONG        SecondsOld=0;
    static ULONG        SecondsNew;
    ULONG               Hours;
    ULONG               Minutes;
    ULONG               Seconds;

                                        /* Position text */
    ptl.x=Sd2.swpSD2CountDown.x; ptl.y=Sd2.swpSD2CountDown.y;
    if((LONGFROMMP(mp1)==0) && (LinePower==TRUE))
        {                               /* If line power is there, then display no
                                           time to ShutDown */
                                        /* Erase old text to background color */
            GpiSetColor(hps, CLR_BACKGROUND);
            GpiCharStringAt(hps, &ptl, strlen(ucBuffer), ucBuffer);
        }
    else
        {
        if((LONGFROMMP(mp1)==0) && (LinePower==FALSE))
            {                           /* If a repaint is forced, repaint with previous value */
            SecondsNew=SecondsOld;
            SecondsOld=0;
            }
        if(LONGFROMMP(mp1)!=0)
            SecondsNew=LONGFROMMP(mp1); /* Get new seconds to shutdown */
        if(SecondsNew!=SecondsOld)      /* Redraw string if changed */
            {
                                        /* Erase old text to background color */
            GpiSetColor(hps, CLR_BACKGROUND);
            GpiCharStringAt(hps, &ptl, strlen(ucBuffer), ucBuffer);
                                        /* Format new text */
            Hours=SecondsNew/(60*60);
            Minutes=(SecondsNew-(Hours*60*60))/60;
            Seconds=SecondsNew-(Hours*60*60)-(Minutes*60);
            sprintf(ucBuffer, "Automatic ShutDown in %02d:%02d:%02d",
                Hours, Minutes, Seconds);
            GpiSetColor(hps, CLR_RED);
                                        /* Write new text */
            GpiCharStringAt(hps, &ptl, strlen(ucBuffer), ucBuffer);
            SecondsOld=SecondsNew;          /* Update to new time */
            }
        }
    }
    break;

/*                                                                                      *\
 * Flash the icon by xoring the selection bit.                                          *
\*                                                                                      */
case WM_FLASHICON:
    {
    static      counter=0;

    counter^=0x1;                       /* Select the other icon (there are only two) */
    if(LinePower==FALSE)
        WinSendMsg(hwndFrame, WM_SETICON, MPFROMLONG(hptrIcon[counter]), NULL);
    else WinSendMsg(hwndFrame, WM_SETICON, MPFROMLONG(hptrIcon[0]), NULL);
    WinInvalidateRect(hwndFrame, NULL, FALSE);
    }
    break;

/*                                                                                      *\
 * Display the ShutDown dialog box.                                                     *
\*                                                                                      */
case WM_SHUTDOWNDIALOG:
    WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, SD2_ShutDownDialogProc, 0, ID_SHUTDOWNDIALOG, 0);
    break;

/*                                                                                      *\
 * Resize/position the SD/2 main window.                                                *
\*                                                                                      */
case WM_SD2POSITION:
    WinSetWindowPos(hwndFrame, HWND_TOP, swpClient.x, swpClient.y,
        swpClient.cx, swpClient.cy, LONGFROMMP(mp1));
    break;

/*                                                                                      *\
 * Enable the required menubar menus because LinePower is TRUE.                         *
\*                                                                                      */
case WM_ENABLEMENU:
    {
    HWND        hwndMenu;
                                        /* Get menubar handle */
    hwndMenu=WinWindowFromID(hwndFrame, FID_MENU);
                                        /* Enable Exit menu */
    WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_EXIT, TRUE),
        MPFROM2SHORT(MIA_DISABLED, FALSE));
                                        /* Enable Config Menu */
    WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_CONFIG, TRUE),
        MPFROM2SHORT(MIA_DISABLED, FALSE));
    }
    break;

/*                                                                                      *\
 * Disable the required menubar menus because LinePower is FALSE.                       *
\*                                                                                      */
case WM_DISABLEMENU:
    {
    HWND        hwndMenu;
                                        /* Get menubar handle */
    hwndMenu=WinWindowFromID(hwndFrame, FID_MENU);
                                        /* Disable Exit menu */
    WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_EXIT, TRUE),
        MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
                                        /* Disable Config Menu */
    WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(ID_CONFIG, TRUE),
        MPFROM2SHORT(MIA_DISABLED, MIA_DISABLED));
    }
    break;

case WM_DESTROY:                        /* Destroy loaded ressources */
    {
    ULONG       counter;

    GpiDestroyPS(hps);
    UPS_ThreadCreate(FALSE, NULL);      /* Kill secondary thread */
    for(counter=IDP_ICON1; counter<=IDP_ICON2; counter++)
        WinDestroyPointer(hptrIcon[counter]);
    }
    break;

case WM_CLOSE:
    if(WinMessageBox(                   /* Ask the user if he really wants to exit */
        HWND_DESKTOP,
        HWND_DESKTOP,
        "Are you sure you want to close SD/2 that controls your UPS?",
        "SD/2 - ShutDown/2 Information",
        ID_SHUTDOWN,
        MB_OKCANCEL | MB_ICONQUESTION | MB_DEFBUTTON2)
        ==MBID_OK)
            WinPostMsg(hwnd, WM_QUIT, NULL, NULL);
    else return((MRESULT)TRUE);
    break;

case WM_COMMAND:
    {
    USHORT      command;

    command=SHORT1FROMMP(mp1);          /* Extract the command value */
    switch(command)
    {
    case ID_EXIT:                       /* User selected F3 to shutdown SD/2 */
        WinPostMsg(hwnd, WM_CLOSE, 0, 0);
        break;

    case ID_CONFIG:
        if(!WinDlgBox(                  /* Start Configuration dialog box */
            HWND_DESKTOP,               /* DESKTOP is parent */
            HWND_DESKTOP,               /* DESKTOP is owner */
            SD2_ConfigDialogProc,       /* Dialog procedure of Program Installation
                                           dialog */
            0,                          /* Ressource is .EXE file */
            ID_CONFIG,                  /* ID of Configure SD/2 dialog */
            0))                         /* No initialization data */
        User_Error("Can't load Configuration dialog...");
        UPS_PortReset();                /* Set the port to the default value */
        break;

    case ID_ABOUT:
        if(!WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, SD2_AboutDialogProc, 0, ID_ABOUT, 0))
        User_Error("Can't load Configuration dialog...");
        break;

    case ID_HELP:                       /* Display general help panel */
        if(hwndHelp!=NULLHANDLE) WinSendMsg(
            hwndHelp,                   /* Help window */
            HM_DISPLAY_HELP,            /* Display a help panel */
            MPFROMSHORT(ID_HELP),       /* Panel ID in ressource file */
            HM_RESOURCEID);             /* MP1 points to the help window identity */
        break;
    }
    break;
    }

default:                                /* Default window procedure must be called */
    return((MRESULT)WinDefWindowProc(hwnd, msg, mp1, mp2));
}
return((MRESULT)FALSE);                 /* We have handled the message */
}

/*                                                                                      *\
 * Just for clearification: The logical value at a physical pin (e.g. +BUSY on pin 11   *
 * for parallel ports) may be inverted by some hardware so that the binary value read   *
 * from a port may be inverse (e.g. a positive voltage on +BUSY on pin 11 is inverted   *
 * and read as a -BUSY being a logical 0 from port 3BD. This is no bug, but derived     *
 * from the way the hardware was designed in IBM PC1 times.                             *
 *                                                                                      *
 * Parallel ports: LPT1 3BC-3BE (3BE)                                                   *
 *                 LPT2 378-37A (37A)                                                   *
 *                 LPT3 278-27A (27A)                                                   *
 *                                                                                      *
 * Port 3BE xxxxxxxx used to output ShutDown signal.                                    *
 *              x    ... -SLCT IN on LPT Pin 17 | SLCT IN on 3BE                        *
 *               x   ... -INIT    on LPT Pin 16 | -INIT   on 3BE                        *
 *          xxxx01xx ... LPT default            | xxxx11xx ... 3BE default              *
 *          xxxx10xx ... LPT SD/2               | xxxx00xx ... 3BE SD/2                 *
 * Thus to set ShutDown signal on LPT interface, the data read from 3BE is anded with   *
 * 11110011. This output would initialize a not selected printer.                       *
\*                                                                                      */
/*                                                                                      *\
 * Port 3BD xxxxxxxx used to get UPS status ShutDown signal.                            *
 *          x        ... +BUSY on LPT Pin 11 | -BUSY on 3BD                             *
 *           x       ... -ACK  on LPT Pin 10 | -ACK on 3BD                              *
 *          11xxxxxx ... LPT default idle    | 01xxxxxx ... 3BE default idle printer    *
 *          10xxxxxx ... LPT default ackn.   | 00xxxxxx ... 3BE default ackn. printer   *
 *          01xxxxxx ... LPT default busy    | 11xxxxxx ... 3BE default busy printer    *
 *          00xxxxxx ... LPT SD/2            | 10xxxxxx ... 3BE SD/2 UPS line failure   *
 * Thus to get UPS line power status we read from 3BD, and with 11000000 and compare    *
 * the result with 10000000. This status would correspond to a printer that is busy     *
 * and ready at the same time.                                                          *
\*                                                                                      */
/*                                                                                      *\
 * Serial ports: COM1  3F8- 3FF ( 3FC,  3FE)                                            *
 *               COM2  2F8- 2FF ( 2FC,  2FE)                                            *
 *               COM3 3220-3227 (3224, 3226)                                            *
 *               COM4 3228-322F (322C, 322E)                                            *
 *               COM5 4220-4227 (4224, 4226)                                            *
 *               COM6 4228-422F (422C, 422E)                                            *
 *               COM7 5220-5227 (5224, 5226)                                            *
 *               COM8 5228-522F (522C, 522E)                                            *
 *                                                                                      *
 * Port 3FC xxxxxxxx used to output ShutDown signal (+/- ... high/low RS232 voltage)    *
 *                 x ... DTR on COM Pin 20 | -DTR on 3FC                                *
 *                x  ... RTS on COM Pin 4  | -RTS on 3FC                                *
 *          xxxxxx++ ... V24 default       | xxxxxx11 ... 3FC default                   *
 *          xxxxxx-+ ... V24 SD/2          | xxxxxx01 ... 3FC SD/2                      *
 * This output would request data on a not ready communication interface.               *
\*                                                                                      */
/*                                                                                      *\
 * Port 3FE xxxxxxxx used to get UPS status ShutDown signal.                            *
 *             x     ... CTS on COM Pin 5 | -CTS on 3FE                                 *
 *          x        ... CD  on COM Pin 8 | -CD  on 3FE                                 *
 *          -xx-xxxx ... V24 default      | 0xx0xxxx ... 3FE default                    *
 *          +xx-xxxx ... V24 SD/2         | 1xx0xxxx ... 3FE UPS line failure SD/2      *
 * This output would indicate a communication interface, ready to send without a        *
 * carrier available.                                                                   *
\*                                                                                      */


/*--------------------------------------------------------------------------------------*\
 * This procedure resets the PS/2's port, where the status of ShutDown is written to    *
 * the UPS.                                                                             *
\*--------------------------------------------------------------------------------------*/
BOOL    UPS_PortReset(void)
{
BOOL    bFlag=FALSE;
UCHAR   Data;
USHORT  Port;
USHORT  LPT_Ports[]={0x3BC, 0x378, 0x278};
USHORT  COM_Ports[]={0x3F8, 0x2F8, 0x3220, 0x3228, 0x4220, 0x4228, 0x5220, 0x5228};

switch(Ups.GRP_Mode)                    /* Get corresponding I/O address */
{
case IDM_LPT1:
case IDM_LPT2:
case IDM_LPT3:
    Port=LPT_Ports[Ups.GRP_Mode-IDM_LPT1];
    Port+=0x2;                          /* Address LPT control port */
    Data=IN8(Port);                     /* Get state of control port */
    Data=Data&0x0C;                     /* Set control port to signalize normal operation
                                           of SD/2 */
    OUT8(Port, Data);
                                        /* Log parallel port initialization to logfile */
    DosGetDateTime(&UPS_Current);
    UPS_LogfileIO(LF_PARALLEL_INITIALIZED, MPFROMP(&UPS_Current));
    bFlag=TRUE;
    break;

case IDM_COM1:
case IDM_COM2:
case IDM_COM3:
case IDM_COM4:
case IDM_COM5:
case IDM_COM6:
case IDM_COM7:
case IDM_COM8:
    Port=COM_Ports[Ups.GRP_Mode-IDM_COM1];
    Port+=0x4;                          /* Address COM modem control port */
    Data=IN8(Port);                     /* Get state of modem control port */
    Data|=0x03;                         /* Set modem control port to normal operation
                                           of SD/2 */
    OUT8(Port, Data);
                                        /* Log serial port initialization to logfile */
    DosGetDateTime(&UPS_Current);
    UPS_LogfileIO(LF_SERIAL_INITIALIZED, MPFROMP(&UPS_Current));
    bFlag=TRUE;
    break;
}
return(bFlag);
}

/*--------------------------------------------------------------------------------------*\
 * This procedure reads of writes the UPS status data from/to the PS/2's ports.         *
\*--------------------------------------------------------------------------------------*/
BOOL    UPS_LinePower(BOOL Set)
{
BOOL    bFlag=FALSE;
UCHAR   Data;
USHORT  Port;
USHORT  LPT_Ports[]={0x3BC, 0x378, 0x278};
USHORT  COM_Ports[]={0x3F8, 0x2F8, 0x3220, 0x3228, 0x4220, 0x4228, 0x5220, 0x5228};

switch(Ups.GRP_Mode)                    /* Get corresponding I/O address */
{
case IDM_LPT1:
case IDM_LPT2:
case IDM_LPT3:
    Port=LPT_Ports[Ups.GRP_Mode-IDM_LPT1];
    if(Set==TRUE)
        {                               /* Signal ShutDown to external logic */
        Port+=0x2;                      /* Address LPT control port */
        Data=IN8(Port);                 /* Get state of control port */
        Data=Data&0xF3;                 /* Set control port to signalize ShutDown */
        OUT8(Port, Data);
        bFlag=TRUE;
        }
    else
        {                               /* Get external status from UPS */
        Port+=0x1;                      /* Address LPT status port */
        Data=IN8(Port);                 /* Get state of status port */
        if((Data&0xC0)==0x80)           /* +BUSY and -ACKNOWLEDGE must both be set. This
                                           should never occur during regular printing */
            {
            LinePower=FALSE;
            bFlag=FALSE;
            }
        else
            {
            LinePower=TRUE;
            bFlag=TRUE;
            }
        }
    break;

case IDM_COM1:
case IDM_COM2:
case IDM_COM3:
case IDM_COM4:
case IDM_COM5:
case IDM_COM6:
case IDM_COM7:
case IDM_COM8:
    Port=COM_Ports[Ups.GRP_Mode-IDM_COM1];
    if(Set==TRUE)
        {                               /* Signal ShutDown to external logic */
        Port+=0x4;                      /* Address COM modem control port */
        Data=IN8(Port);                 /* Get state of modem control port */
        Data&=0xFC;                     /* Set modem control port to signalize ShutDown */
        Data|=0x01;
        OUT8(Port, Data);
        bFlag=TRUE;
        }
    else
        {                               /* Get external status from UPS */
        Port+=0x6;                      /* Address COM modem status port */
        Data=IN8(Port);                 /* Get state of modem status port */
        if((Data&0x90)==0x80)           /* Test for UPS line power failure */
            {
            LinePower=FALSE;
            bFlag=FALSE;
            }
        else
            {
            LinePower=TRUE;
            bFlag=TRUE;
            }
        }
    break;
}
return(bFlag);
}

/*--------------------------------------------------------------------------------------*\
 * This procedure is a secondary thread that flashed the icon and requests bar redraws. *
\*--------------------------------------------------------------------------------------*/
void    UPS_Thread(ULONG ulThreadArg)
{
UPS_PortReset();                        /* Set the port to the default value */
do
{
/*                                                                                      *\
 * As long as there is no indication, that line power fails, just wait for an           *
 * indication by checking the UPS control input.                                        *
\*                                                                                      */
                                        /* First enable required menubar menus */
    WinPostMsg(hwndClient, WM_ENABLEMENU, NULL, NULL);
    do
    {
        DosSleep(0250);
                                        /* Query UPS and break out of loop if line power fails */
        if(UPS_LinePower(FALSE)==FALSE) break;
    } while(LinePower==TRUE);
/*                                                                                      *\
 * As long as there is an indication, that line power fails, get the current time,      *
 * adjust the checkmarks from this time on, begin flashing the icon. After the Alarm    *
 * Start timestamp is reached, restore the window. Before the Alarm Start timestamp is  *
 * reached, any drawing is ignored because of the minimized state (used during debug).  *
 * If line power reappears, than go back to sleep state.                                *
\*                                                                                      */
                                        /* Disable required menubar menus */
    WinPostMsg(hwndClient, WM_DISABLEMENU, NULL, NULL);
    DosGetDateTime(&UPS_Current);       /* Get current date&time */
                                        /* Power failure logging into logfile */
    UPS_LogfileIO(LF_POWERFAILED, MPFROMP(&UPS_Current));
    Copy_DateTime(&UPS_Start, &UPS_Current);
    Copy_DateTime(&UPS_AlertStart, &UPS_Current);
    Copy_DateTime(&UPS_AlertShutDown, &UPS_Current);
    Copy_DateTime(&UPS_ShutDown, &UPS_Current);
                                        /* Add timestamps */
    Add_DateTime(&UPS_AlertStart, &Ups.IDUAS_Hours, &Ups.IDUAS_Minutes);
    Add_DateTime(&UPS_AlertShutDown, &Ups.IDUASD_Hours, &Ups.IDUASD_Minutes);
    Add_DateTime(&UPS_ShutDown, &Ups.IDUS_Hours, &Ups.IDUS_Minutes);
                                        /* Displaying planned timestamps */
    UPS_LogfileIO(LF_UPS_ALERTSTARTTIME, MPFROMP(&UPS_AlertStart));
    UPS_LogfileIO(LF_UPS_ALERTSHUTDOWNTIME, MPFROMP(&UPS_AlertShutDown));
    UPS_LogfileIO(LF_UPS_SHUTDOWNTIME, MPFROMP(&UPS_ShutDown));
    while(LinePower==FALSE)
    {                                   /* Wait for Alert Start timestamp to restore SD/2 */
                                        /* Query UPS and break out of loop if line
                                           power exists */
        if(UPS_LinePower(FALSE)==TRUE) break;
        DosSleep(0250);
        WinPostMsg(hwndClient, WM_FLASHICON, NULL, NULL);
        DosGetDateTime(&UPS_Current);
        TimeDiff=Diff_DateTime(&UPS_Current, &UPS_ShutDown);
        WinPostMsg(hwndClient, WM_DRAWUPSBAR, MPFROMLONG(TimeDiff), NULL);
        WinPostMsg(hwndClient, WM_DRAWUPSTIME, MPFROMLONG(TimeDiff), NULL);
        if((Cmp_DateTime(&UPS_Current, &UPS_AlertStart))==TRUE)
            {
                                        /* Alert start logging into logfile */
            DosGetDateTime(&UPS_Current);
            UPS_LogfileIO(LF_UPS_ALERTSTART, MPFROMP(&UPS_Current));
            DosBeep(1000,250);
            DosBeep(500,250);
            DosBeep(1000,250);
                                        /* Restore SD/2's window */
            WinPostMsg(hwndClient, WM_SD2POSITION, MPFROMLONG(SWP_RESTORE), NULL);
            break;                      /* Exit loop */
            }
    }
    while(LinePower==FALSE)
    {                                   /* SD/2 is restored, update bar and wait for
                                           timestamps to start user defined session */
                                        /* Query UPS and break out of loop if line
                                           power exists */
        if(UPS_LinePower(FALSE)==TRUE) break;
        DosSleep(0250);
        WinPostMsg(hwndClient, WM_FLASHICON, NULL, NULL);
        DosGetDateTime(&UPS_Current);
        TimeDiff=Diff_DateTime(&UPS_Current, &UPS_ShutDown);
        WinPostMsg(hwndClient, WM_DRAWUPSBAR, MPFROMLONG(TimeDiff), NULL);
        WinPostMsg(hwndClient, WM_DRAWUPSTIME, MPFROMLONG(TimeDiff), NULL);
        if((Cmp_DateTime(&UPS_Current, &UPS_AlertShutDown))==TRUE)
            {
            STARTDATA   StartData;
            ULONG       SessID;
            PID         Pid;

                                        /* Alert ShutDown logging into logfile */
            DosGetDateTime(&UPS_Current);
            UPS_LogfileIO(LF_UPS_ALERTSHUTDOWN, MPFROMP(&UPS_Current));
            StartData.Length=50;        /* Length of StartData */
                                        /* Independent session */
            StartData.Related=SSF_RELATED_INDEPENDENT;
                                        /* Background application */
            StartData.FgBg=SSF_FGBG_BACK;
                                        /* No trace */
            StartData.TraceOpt=SSF_TRACEOPT_NONE;
                                        /* Session title string */
            StartData.PgmTitle="ShutDown called by SD/2";
                                        /* Program path-name string */
            StartData.PgmName=Ups.IDS_PgmName;
                                        /* Input arguments */
            StartData.PgmInputs=Ups.IDS_PgmInputs;
            StartData.TermQ=0;          /* No termination queue */
            StartData.Environment=0;    /* No environment */
                                        /* Inherit from PC/2's environment */
            StartData.InheritOpt=SSF_INHERTOPT_PARENT;
                                        /* Session type */
            StartData.SessionType=SSF_TYPE_DEFAULT;
            StartData.IconFile=0;       /* No icon, use default */
            StartData.PgmHandle=0;      /* Don't use installation file */
                                        /* Session initial state */
            StartData.PgmControl=SSF_CONTROL_VISIBLE;
                                        /* Initial window size */
            StartData.InitXPos=100;
            StartData.InitYPos=100;
            StartData.InitXSize=300;
            StartData.InitYSize=250;
/*                                                                                      *\
 * Test for x:(...] where x is a drive and set the currenct working drive to this       *
 * drive.                                                                               *
\*                                                                                      */
            if((strlen(Ups.IDS_PgmDirectory)>=2) && (Ups.IDS_PgmDirectory[1]==':'))
                {
                UCHAR   ucDrive;
                                        /* Then get drive letter (only if one's there */
                ucDrive=tolower(Ups.IDS_PgmDirectory[0]);
                                        /* 1=A, 2=B, 3=C,... */
                DosSetDefaultDisk(++ucDrive-'a');
                }
/*                                                                                      *\
 * Test for a directory and set the current working directory to it, if one exists.     *
\*                                                                                      */
            if(strlen(Ups.IDS_PgmDirectory)>=2)
                                        /* Only if there's one */
                DosSetCurrentDir(Ups.IDS_PgmDirectory);
            else DosSetCurrentDir("\\");
/*                                                                                      *\
 * Now start the session.                                                               *
\*                                                                                      */
            DosStartSession(            /* Start the new session */
                &StartData,             /* Session data */
                &SessID,                /* Session ID of new session */
                &Pid);                  /* Process ID of new session */
            break;                      /* Exit loop */
            }
    }
    while(LinePower==FALSE)
    {                                   /* Wait for timestamp to Shut Down system with SD/2 */
                                        /* Query UPS and break out of loop if line
                                           power exists */
        if(UPS_LinePower(FALSE)==TRUE) break;
        DosSleep(0250);
        WinPostMsg(hwndClient, WM_FLASHICON, NULL, NULL);
        DosGetDateTime(&UPS_Current);
        TimeDiff=Diff_DateTime(&UPS_Current, &UPS_ShutDown);
        WinPostMsg(hwndClient, WM_DRAWUPSBAR, MPFROMLONG(TimeDiff), NULL);
        WinPostMsg(hwndClient, WM_DRAWUPSTIME, MPFROMLONG(TimeDiff), NULL);
        if((Cmp_DateTime(&UPS_Current, &UPS_ShutDown))==TRUE)
            {
                                        /* Set SD/2 a the focus window */
            WinSetFocus(HWND_DESKTOP, hwndFrame);
            UPS_LinePower(TRUE);        /* Signalize external hardware to ShutDown this
                                           system after a reasonable delay, allowing us
                                           to ShutDown OS/2 */
                                        /* ShutDown logging into logfile */
            DosGetDateTime(&UPS_Current);
            UPS_LogfileIO(LF_UPS_SHUTDOWN, MPFROMP(&UPS_Current));
            WinPostMsg(hwndClient, WM_SHUTDOWNDIALOG, NULL, NULL);
            while(TRUE) DosSleep(1000); /* Loop endless until Power off */
            }
    }
    DosGetDateTime(&UPS_Current);       /* Get current date&time */
                                        /* Power returned logging into logfile */
    UPS_LogfileIO(LF_POWERRETURNED, MPFROMP(&UPS_Current));
                                        /* Redraw bar to green */
    WinPostMsg(hwndClient, WM_DRAWUPSBAR, MPFROMLONG(0), NULL);
                                        /* Flash icon back to green */
    WinPostMsg(hwndClient, WM_FLASHICON, NULL, NULL);
                                        /* Minimize SD/2's window */
    WinPostMsg(hwndClient, WM_SD2POSITION, MPFROMLONG(SWP_MINIMIZE), NULL);
} while(TRUE);
}

/*--------------------------------------------------------------------------------------*\
 * This procedure is the configuration dialog window procedure.                         *
\*--------------------------------------------------------------------------------------*/
MRESULT EXPENTRY SD2_ConfigDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch(msg)
{
case WM_INITDLG:
    {
    SWP         swp;

    WinQueryWindowPos(                  /* Query position of dialog window */
        hwnd,                           /* Handle of dialog window */
        &swp);                          /* Fill with position */
    WinSetWindowPos(                    /* Set dialog window position */
        hwnd,                           /* Handle of dialog window */
        HWND_TOP,                       /* Position on top and center of DESKTOP */
        (swpScreen.cx-swp.cx)/2, (swpScreen.cy-swp.cy)/2, 0, 0, SWP_MOVE);
/*                                                                                      *\
 * Set the maximum number of chars accepted from the entryfield (thus overwriting the   *
 * default number of 63 and load the data from the UPS structure Ups into the           *
 * entryfields.                                                                         *
\*                                                                                      */
    {
    ULONG       IDs[]={IDUBC_HOURS, IDUBC_MINUTES, IDUAS_HOURS, IDUAS_MINUTES,
                       IDUASD_HOURS, IDUASD_MINUTES, IDUS_HOURS, IDUS_MINUTES,
                       IDS_PGMNAME, IDS_PGMDIRECTORY, IDS_PGMINPUTS};
    void        *Adds[]={&Ups.IDUBC_Hours, &Ups.IDUBC_Minutes, &Ups.IDUAS_Hours, &Ups.IDUAS_Minutes,
                         &Ups.IDUASD_Hours, &Ups.IDUASD_Minutes, &Ups.IDUS_Hours, &Ups.IDUS_Minutes,
                         &Ups.IDS_PgmName, &Ups.IDS_PgmDirectory, &Ups.IDS_PgmInputs};
    UCHAR       Buffer[129];
    USHORT      Size=2;
    ULONG       Counter;

    for(Counter=0; Counter<=10; Counter++)
        {
        if(Counter==8) Size=128;        /* For session start structure overwrite default 2 */
        WinSendDlgItemMsg(              /* Send message to dialog window */
            hwnd,                       /* Handle of dialog window */
            IDs[Counter],               /* Entryfield id */
            EM_SETTEXTLIMIT,            /* Set text limit to 2 or 128 */
            MPFROMSHORT(Size),
            (MPARAM)NULL);              /* No additional parameter */
        if(Counter>=8)
            WinSetDlgItemText(          /* Load the default text of the entryfield */
                hwnd, IDs[Counter], Adds[Counter]);
        else
            {                           /* Convert ULONG to Ascii */
            _ltoa(*(UCHAR *)Adds[Counter], Buffer, 10);
            WinSetDlgItemText(          /* Load the default text of the entryfield */
                hwnd, IDs[Counter], Buffer);
            }
        }
    }
/*                                                                                      *\
 * The entryfields that contain the timestamps are readonly by default to minimized     *
 * bad user input for a predefined external hardware configuration. These fields will   *
 * be set to readwrite if the magic keyword [-|/]IBMSERVICE is given on the command     *
 * line.                                                                                *
\*                                                                                      */
    if(bEditIniFile==TRUE)
        {
        USHORT  usIniEFs[]={IDUBC_HOURS, IDUBC_MINUTES, IDUAS_HOURS, IDUAS_MINUTES,
                            IDUASD_HOURS, IDUASD_MINUTES, IDUS_HOURS, IDUS_MINUTES};
        USHORT  usTemp;

        for(usTemp=0; usTemp<=(sizeof(usIniEFs)/sizeof(USHORT)); usTemp++)
            WinSendDlgItemMsg(hwnd, usIniEFs[usTemp],
                                        /* Set to enable read/write */
                EM_SETREADONLY, MPFROMSHORT(FALSE), (MPARAM)NULL);
        }
/*                                                                                      *\
 * Now we preselect the radiobutton of the UPS control mode.                            *
\*                                                                                      */
    WinSendDlgItemMsg(                  /* Send message to radiobutton */
        hwnd,                           /* Handle of dialog window */
        Ups.GRP_Mode,                   /* UPS mode: ? radiobutton */
        BM_SETCHECK,                    /* Set it to pressed */
        MPFROMSHORT(TRUE),
        (MPARAM)NULL);
/*                                                                                      *\
 * Now we preenter the IDS_USERINFO multi-line entryfield and adjust the length to 256. *
\*                                                                                      */
    WinSendDlgItemMsg(hwnd, IDS_USERINFO, MLM_SETTEXTLIMIT,
        MPFROMSHORT(sizeof(Ups.IDS_UserInfo)-1), (MPARAM)NULL);
    WinSetDlgItemText(hwnd, IDS_USERINFO, Ups.IDS_UserInfo);
    break;
    }

case WM_HELP:                           /* Help pressed */
    {
    WinSendMsg(
        hwndHelp,                       /* Help window */
        HM_DISPLAY_HELP,                /* Display a help panel */
        MPFROMSHORT(ID_CONFIG),         /* Panel ID in ressource file */
        HM_RESOURCEID);                 /* MP1 points to the help window identity */
    }
    break;

case WM_COMMAND:
    {
    USHORT      command;

    command=SHORT1FROMMP(mp1);          /* Extract the command value */
    switch(command)
    {
    case DID_OK:
/*                                                                                      *\
 * Get the entries from the user into the UPS structure Ups.                            *
\*                                                                                      */
        {
        BOOL    ReInput=FALSE;
        ULONG   IDs[]={IDUBC_HOURS, IDUBC_MINUTES, IDUAS_HOURS, IDUAS_MINUTES,
                       IDUASD_HOURS, IDUASD_MINUTES, IDUS_HOURS, IDUS_MINUTES,
                       IDS_PGMNAME, IDS_PGMDIRECTORY, IDS_PGMINPUTS};
        void    *Adds[]={&Ups.IDUBC_Hours, &Ups.IDUBC_Minutes, &Ups.IDUAS_Hours, &Ups.IDUAS_Minutes,
                         &Ups.IDUASD_Hours, &Ups.IDUASD_Minutes, &Ups.IDUS_Hours, &Ups.IDUS_Minutes,
                         &Ups.IDS_PgmName, &Ups.IDS_PgmDirectory, &Ups.IDS_PgmInputs};
        UCHAR   Buffer[129];
        ULONG   Counter;

        for(Counter=0; Counter<=10; Counter++)
            {
            WinQueryDlgItemText(         /* Query data entered in the entryfields
                                            into Ups structure */
                hwnd, IDs[Counter], sizeof(Buffer)-1, Buffer);
            if(Counter>=8)              /* Copy either the numeric value or the string */
                strcpy(Adds[Counter], Buffer);
            else                        /* into the Ups structure */
                *(ULONG *)Adds[Counter]=atol(Buffer);
            }
/*                                                                                      *\
 * Now we query the radiobutton of the UPS control mode. The buttons have consecutive   *
 * IDs so we just loop to find one, and there must be one.                              *
\*                                                                                      */
                                        /* UPS mode: ? radiobutton */
        for(Counter=IDM_LPT1; Counter<=IDM_ADAPTER; Counter++)
                if(WinQueryButtonCheckstate(hwnd, Counter)) Ups.GRP_Mode=Counter;
/*                                                                                      *\
 * Now we query the IDS_USERINFO multi-line entryfield.                                 *
\*                                                                                      */
        WinQueryDlgItemText(hwnd, IDS_USERINFO, sizeof(Ups.IDS_UserInfo)-1, Ups.IDS_UserInfo);
/*                                                                                      *\
 * Now test all input timestamps that they are in range of the battery capacity and     *
 * that the timestamps occur sequentially.                                              *
\*                                                                                      */
        if(Ups.IDUAS_Hours>=Ups.IDUBC_Hours)
            if(Ups.IDUAS_Minutes>=Ups.IDUBC_Minutes)
                {
                ReInput=TRUE;
                                        /* Clear the text to 0 */
                WinSetDlgItemText(hwnd, IDUAS_HOURS, "0");
                WinSetDlgItemText(hwnd, IDUAS_MINUTES, "0");
                }
        if(Ups.IDUASD_Hours>=Ups.IDUBC_Hours)
            if(Ups.IDUASD_Minutes>=Ups.IDUBC_Minutes)
                {
                ReInput=TRUE;
                WinSetDlgItemText(hwnd, IDUASD_HOURS, "0");
                WinSetDlgItemText(hwnd, IDUASD_MINUTES, "0");
                }
        if(Ups.IDUS_Hours>=Ups.IDUBC_Hours)
            if(Ups.IDUS_Minutes>=Ups.IDUBC_Minutes)
                {
                ReInput=TRUE;
                WinSetDlgItemText(hwnd, IDUS_HOURS, "0");
                WinSetDlgItemText(hwnd, IDUS_MINUTES, "0");
                }
        if(Ups.IDUAS_Hours>=Ups.IDUASD_Hours)
            if(Ups.IDUAS_Minutes>Ups.IDUASD_Minutes)
                {
                ReInput=TRUE;
                WinSetDlgItemText(hwnd, IDUAS_HOURS, "0");
                WinSetDlgItemText(hwnd, IDUAS_MINUTES, "0");
                }
        if(Ups.IDUASD_Hours>=Ups.IDUS_Hours)
            if(Ups.IDUASD_Minutes>Ups.IDUS_Minutes)
                {
                ReInput=TRUE;
                WinSetDlgItemText(hwnd, IDUASD_HOURS, "0");
                WinSetDlgItemText(hwnd, IDUASD_MINUTES, "0");
                }
        if(ReInput==TRUE) return((MRESULT)FALSE);
                                        /* Write back to profile */
        WinSetPointer(                  /* Set wait pointer */
            HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_WAIT, FALSE));
        Write_Profile(&hini, &hab, &Ups);
        WinSetPointer(                  /* Set arrow pointer */
            HWND_DESKTOP, WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE));
        }
        break;

    case DID_CANCEL:
        break;

    default:
        return(WinDefDlgProc(hwnd, msg, mp1, mp2));
    }
    WinDismissDlg(hwnd, TRUE);          /* Clear up dialog */
    break;
    }

default: 
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}
return((MRESULT)FALSE);
}

/*--------------------------------------------------------------------------------------*\
 * This systemmodel dialog is displayed immediatly before system ShudDown. It has not   *
 * buttons to dismiss it.                                                               *
\*--------------------------------------------------------------------------------------*/
MRESULT EXPENTRY SD2_ShutDownDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch(msg)
{
case WM_INITDLG:
    {
    SWP         swp;

    WinQueryWindowPos(hwnd, &swp);
    WinSetWindowPos(hwnd, HWND_TOP, (swpScreen.cx-swp.cx)/2, (swpScreen.cy-swp.cy)/2,
        0, 0, SWP_MOVE);
    }
                                        /* Now set user entered text */
    if(strlen(Ups.IDS_UserInfo)!=0)
        WinSetDlgItemText(hwnd, IDS_USERINFO, Ups.IDS_UserInfo);
                                        /* Post message to actually perform ShutDown */
    WinPostMsg(hwnd, WM_SHUTDOWN, NULL, NULL);
    break;

case WM_SHUTDOWN:
/*                                                                                      *\
 * Wait for 2 seconds and then shut down OS/2.                                          *
\*                                                                                      */
    DosSleep(02000);
    DosShutdown(0);
    WinSetDlgItemText(hwnd, IDS_SHUTDOWNINFO, "The UPS battery capacity is now empty. "\
        "All disk activity has now been stopped and the power should be turned off, either "\
        "by the PS/2's power switch manually or via an external logic.");
    break;

default:
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}
return((MRESULT)FALSE);
}

/*--------------------------------------------------------------------------------------*\
 * This window procedure handles the About SD/2 dialog.                                 *
\*--------------------------------------------------------------------------------------*/
MRESULT EXPENTRY SD2_AboutDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
switch(msg)
{
case WM_INITDLG:
    {
    SWP         swp;

    WinQueryWindowPos(hwnd, &swp);
    WinSetWindowPos(hwnd, HWND_TOP, (swpScreen.cx-swp.cx)/2, (swpScreen.cy-swp.cy)/2,
        0, 0, SWP_MOVE);
    }
    break;

case WM_HELP:                           /* Help pressed */
    WinSendMsg(
        hwndHelp,                       /* Help window */
        HM_DISPLAY_HELP,                /* Display a help panel */
        MPFROMSHORT(ID_ABOUT),          /* Panel ID in ressource file */
        HM_RESOURCEID);                 /* MP1 points to the help window identity */
    break;

case WM_COMMAND:
    {
    USHORT      command;

    command=SHORT1FROMMP(mp1);          /* Extract the command value */
    switch(command)
    {
    case DID_OK:
        break;

    default:
        return(WinDefDlgProc(hwnd, msg, mp1, mp2));
    }
    WinDismissDlg(hwnd, TRUE);          /* Clear up dialog */
    break;
    }

default:
   return(WinDefDlgProc(hwnd, msg, mp1, mp2));
}
return((MRESULT)FALSE);
}


