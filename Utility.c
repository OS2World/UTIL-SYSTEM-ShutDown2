/***********************************************************************\
 *                               Utiltiy.c                             *
 *                 Copyright (C) by Stangl Roman, 1993                 *
 *                                                                     *
 * Utility.c    Additional utility procedures for ShutDown/2           *
 *                                                                     *
\***********************************************************************/

static char RCSID[]="@(#) $Header: Utility.c Version 1.00 06,1993 $ (LBL)";

#include        "ShutDown.h"            /* User include files */

/*--------------------------------------------------------------------------------------*\
 * Procedure to initialize a window and its message queue.                              *
 * Req:                                                                                 *
 *      pHab .......... A pointer to be filled with the anchor block of the window      *
 *      pHmq .......... A pointer to be filled with the message queue of the window     *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    WinStartUp(HAB *pHab, HMQ *pHmq)
{
                                        /* Initialize handle of anchor block */
if((*pHab=WinInitialize(0))==NULLHANDLE)
    return(FALSE);
                                        /* Initialize handle of message queue */
if((*pHmq=WinCreateMsgQueue(*pHab, 0))==NULLHANDLE)
    return(FALSE);
return(TRUE);
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to initialize HELP.                                                        *
 * Req:                                                                                 *
 *      hab ........... Anchor block handle                                             *
 *      pHelpFile ..... A pointer to helppanel filename in SD/2 directory               *
 *      pHwndHelp  .... A pointer to a HWND structure                                   *
 * Returns:                                                                             *
 *      pHwndHelp ..... If called sucessfully/unsucessfully hwnd/NULL                   *
\*--------------------------------------------------------------------------------------*/
BOOL    WinStartHelp(HAB hab, UCHAR *pHelpFile, HWND *pHwndHelp)
{
HELPINIT        HelpInit;

HelpInit.cb=sizeof(HELPINIT);           /* Size of HELPINIT structure */
HelpInit.ulReturnCode=0;                /* Returnvalue from HelpManager */
HelpInit.pszTutorialName=NULL;          /* No tutorial */
                                        /* Ressource of Helptable */
HelpInit.phtHelpTable=(PHELPTABLE)MAKEULONG(MAIN_HELP_TABLE, 0xffff);
                                        /* Ressource in .EXE */
HelpInit.hmodHelpTableModule=NULLHANDLE;
                                        /* No handle */
HelpInit.hmodAccelActionBarModule=NULLHANDLE;
HelpInit.idAccelTable=0;                /* None */
HelpInit.idActionBar=0;                 /* None */
                                        /* Window title of help window */
HelpInit.pszHelpWindowTitle="SD/2 - ShutDown/2 Help";
HelpInit.pszHelpLibraryName=pHelpFile;  /* Library name of help panel via SD/2 directory */
HelpInit.fShowPanelId=0;                /* Panel ID not displayed */
/*                                                                                      *\
 * First assume ShutDown.HLP in HELP path and try to create it from there.              *
\*                                                                                      */
*pHwndHelp=WinCreateHelpInstance(       /* Create help */
    hab,                                /* Anchor block */
    &HelpInit);
                                        /* Test for successful help creation */
if((*pHwndHelp) && (!HelpInit.ulReturnCode))
                                        /* Associate HELP with frame window */
    if(WinAssociateHelpInstance(*pHwndHelp, hwndFrame)!=FALSE)
        return(TRUE);
/*                                                                                      *\
 * Second assume ShutDown.HLP in SD/2's directory and try to create it from there.      *
\*                                                                                      */
HelpInit.ulReturnCode=0;                /* Returnvalue from HelpManager */
                                        /* Library name of help panel via HELP path */
HelpInit.pszHelpLibraryName="ShutDown.hlp";
*pHwndHelp=WinCreateHelpInstance(hab, &HelpInit);
if((*pHwndHelp) && (!HelpInit.ulReturnCode))
    if(WinAssociateHelpInstance(*pHwndHelp, hwndFrame)!=FALSE)
        return(TRUE);
*pHwndHelp=NULLHANDLE;
return(FALSE);
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to close a window and its message queue.                                   *
 * Req:                                                                                 *
 *      pHwndHelp ..... A pointer to HELP window handle                                 *
 *      pHab .......... A pointer to extract the anchor block of the window             *
 *      pHmq .......... A pointer to extract message queue of the window                *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    WinCloseDown(HWND *pHwndHelp, HAB *pHab, HMQ *pHmq)
{
if(!*pHwndHelp)                         /* Release HELP */
    WinDestroyHelpInstance(*pHwndHelp);
if(*pHmq!=NULLHANDLE)                   /* Release handle of message queue */
    WinDestroyMsgQueue(*pHmq);
if(*pHab!=NULLHANDLE)                   /* Release handle of anchor block */
    WinTerminate(*pHab);
                                        /* Any error during WinStartUp */
if((*pHab==NULLHANDLE) || (*pHmq==NULLHANDLE)) return(FALSE);
else return(TRUE);
}

/*--------------------------------------------------------------------------------------*\
 * Procedure opens the profile and writes the UPS structure into.                       *
 * Req:                                                                                 *
 *      pHini ......... A pointer to the handle of the profile                          *
 *      pHab .......... A pointer to extract the anchor block of the window             *
 *      pUps .......... A pointer to the UPS structure                                  *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    Write_Profile(HINI *pHini, HAB *pHab, UPS *pUps)
{
                                        /* First open the profile */
*pHini=PrfOpenProfile(*pHab, pucSD2Profile);
if(*pHini!=NULLHANDLE)
    {
    PrfWriteProfileData(                /* Write binary data to profile */
        *pHini,                         /* Handle of profile */
        SD2_CLASSNAME,                  /* Application name */
        "IDUBC_Hours",                  /* Key name */
        &pUps->IDUBC_Hours,             /* Value data */
        sizeof(UCHAR));                 /* Size of value data */
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUBC_Minutes",
        &pUps->IDUBC_Minutes,
        sizeof(UCHAR));
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUAS_Hours",
        &pUps->IDUAS_Hours,
        sizeof(UCHAR));
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUAS_Minutes",
        &pUps->IDUAS_Minutes,
        sizeof(UCHAR));
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUASD_Hours",
        &pUps->IDUASD_Hours,
        sizeof(UCHAR));
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUASD_Minutes",
        &pUps->IDUASD_Minutes,
        sizeof(UCHAR));
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUS_Hours",
        &pUps->IDUS_Hours,
        sizeof(UCHAR));
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUS_Minutes",
        &pUps->IDUS_Minutes,
        sizeof(UCHAR));
    PrfWriteProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_PgmName",
        pUps->IDS_PgmName);
    PrfWriteProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_PgmDirectory",
        pUps->IDS_PgmDirectory);
    PrfWriteProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_PgmInputs",
        pUps->IDS_PgmInputs);
    PrfWriteProfileData(
        *pHini,
        SD2_CLASSNAME,
        "GRP_Mode",
        &pUps->GRP_Mode,
        sizeof(ULONG));
    PrfWriteProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_UserInfo",
        pUps->IDS_UserInfo);
    return(PrfCloseProfile(*pHini));    /* Close and return result */
    }
else
    {                                   /* Profile couldn't be opened successfully */
                                        /* SHUTDOWN.INI defective logging into logfile */
    DosGetDateTime(&UPS_Current);
    UPS_LogfileIO(LF_INIFILE_PROBLEM, MPFROMP(&UPS_Current));
    return(FALSE);
    }
}

/*--------------------------------------------------------------------------------------*\
 * Procedure opens the profile and reads the UPS structure from.                        *
 * Req:                                                                                 *
 *      pHini ......... A pointer to the handle of the profile                          *
 *      pHab .......... A pointer to extract the anchor block of the window             *
 *      pUps .......... A pointer to the UPS structure                                  *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    Read_Profile(HINI *pHini, HAB *pHab, UPS *pUps)
{
ULONG   ulSize;
UCHAR   *uc='\0';
                                        /* First open the profile */
*pHini=PrfOpenProfile(*pHab, pucSD2Profile);
while(TRUE)
    {
    ulSize=sizeof(UCHAR);
    if(PrfQueryProfileData(             /* Query binary data from profile */
        *pHini,                         /* Handle of profile */
        SD2_CLASSNAME,                  /* Application name */
        "IDUBC_Hours",                  /* Key name */
        &pUps->IDUBC_Hours,             /* Value data */
        &ulSize)==FALSE)                /* Size of value data */
        { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUBC_Minutes",
        &pUps->IDUBC_Minutes,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUAS_Hours",
        &pUps->IDUAS_Hours,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUAS_Minutes",
        &pUps->IDUAS_Minutes,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUASD_Hours",
        &pUps->IDUASD_Hours,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUASD_Minutes",
        &pUps->IDUASD_Minutes,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUS_Hours",
        &pUps->IDUS_Hours,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "IDUS_Minutes",
        &pUps->IDUS_Minutes,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    ulSize=sizeof(ULONG);
    if(PrfQueryProfileData(
        *pHini,
        SD2_CLASSNAME,
        "GRP_Mode",
        &pUps->GRP_Mode,
        &ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    ulSize=sizeof(Ups.IDS_PgmName);
    if(PrfQueryProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_PgmName",
        uc,
        pUps->IDS_PgmName,
        ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_PgmDirectory",
        uc,
        pUps->IDS_PgmDirectory,
        ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    if(PrfQueryProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_PgmInputs",
        uc,
        pUps->IDS_PgmInputs,
        ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    ulSize=sizeof(Ups.IDS_UserInfo);
    if(PrfQueryProfileString(
        *pHini,
        SD2_CLASSNAME,
        "IDS_UserInfo",
        uc,
        pUps->IDS_UserInfo,
        ulSize)==FALSE) { *pHini=NULLHANDLE; break; }
    return(PrfCloseProfile(*pHini));    /* Close and return result */
    }
if(*pHini==NULLHANDLE)
    {
                                        /* SHUTDOWN.INI missing logging into logfile */
    DosGetDateTime(&UPS_Current);
    UPS_LogfileIO(LF_INIFILE_MISSING, MPFROMP(&UPS_Current));
    pUps->IDUBC_Hours=0;                /* Assume default values */
    pUps->IDUBC_Minutes=10;
    pUps->IDUAS_Hours=0;
    pUps->IDUAS_Minutes=1;
    pUps->IDUASD_Hours=0;
    pUps->IDUASD_Minutes=4;
    pUps->IDUS_Hours=0;
    pUps->IDUS_Minutes=7;
    pUps->GRP_Mode=IDM_LPT1;
    strcpy(pUps->IDS_PgmName, "CMD.EXE");
    strcpy(pUps->IDS_PgmDirectory, "");
    strcpy(pUps->IDS_PgmInputs, "");
    strcpy(pUps->IDS_UserInfo, "An error occured accessing SHUTDOWN.INI, please correct "\
        "as soon as possible!");
    return(FALSE);
    }
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to copy a DATETIME structure to another.                                   *
 * Req:                                                                                 *
 *      pDestination .. A pointer to the destination DATETIME structure                 *
 *      pSource ....... A pointer to the source DATETIME structure                      *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If pDestination>=pSource/pDestination<pSource                   *
\*--------------------------------------------------------------------------------------*/
BOOL    Copy_DateTime(DATETIME *pDestination, DATETIME *pSource)
{
pDestination->seconds=pSource->seconds;
pDestination->minutes=pSource->minutes;
pDestination->hours=pSource->hours;
pDestination->day=pSource->day;
pDestination->month=pSource->month;
pDestination->year=pSource->year;
return(TRUE);
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to add hours/minutes to a DATETIME structure.                              *
 * Req:                                                                                 *
 *      pDestination .. A pointer to the destination DATETIME structure                 *
 *      pucHours ...... Hours to add                                                    *
 *      pucMinutes .... Minutes to add
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    Add_DateTime(DATETIME *pDestination, UCHAR *pucHours, UCHAR *pucMinutes)
{
ULONG   Days_Month[]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

if((pDestination->minutes+=*pucMinutes)>=60)
    {                                   /* Adjust minutes */
    pDestination->minutes-=60;
    pDestination->hours++;
    }
if((pDestination->hours+=*pucHours)>=24)
    {                                   /* Adjust hours */
    pDestination->hours-=24;
    pDestination->day++;
    }
                                        /* Adjust for calendar adjustments */
if((pDestination->year%4==0) && (pDestination->year%100)!=0) Days_Month[1]=29;
if(pDestination->day>Days_Month[(pDestination->month)-1])
    {                                   /* Adjust days, beware of 0-based months */
    pDestination->day-=Days_Month[(pDestination->month)-1];
    pDestination->month++;
    }
if(pDestination->month>12)
    {                                   /* Adjust months */
    pDestination->month-=12;
    pDestination->year++;
    }
return(TRUE);
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to compare a DATETIME structure with another DATETIME structure.           *
 * Req:                                                                                 *
 *      pDestination .. A pointer to the destination DATETIME structure                 *
 *      pSource ....... A pointer to the DATETIME structure to compare                  *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If pDestination>=pSource/pDestination<pSource                   *
\*--------------------------------------------------------------------------------------*/
BOOL    Cmp_DateTime(DATETIME *pDestination, DATETIME *pSource)
{
if(pDestination->year>pSource->year) return(TRUE);
if(pDestination->year<pSource->year) return(FALSE);
if(pDestination->month>pSource->month) return(TRUE);
if(pDestination->month<pSource->month) return(FALSE);
if(pDestination->day>pSource->day) return(TRUE);
if(pDestination->day<pSource->day) return(FALSE);
if(pDestination->hours>pSource->hours) return(TRUE);
if(pDestination->hours<pSource->hours) return(FALSE);
if(pDestination->minutes>pSource->minutes) return(TRUE);
if(pDestination->minutes<pSource->minutes) return(FALSE);
if(pDestination->seconds>pSource->seconds) return(TRUE);
if(pDestination->seconds<pSource->seconds) return(FALSE);
return(TRUE);                           /* Both structures are exactly the same */
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to calculate the difference between two DATETIME structures in seconds.    *
 * Req:                                                                                 *
 *      pPrevious .. .. A pointer to the destination DATETIME structure                 *
 *      pLater ........ A pointer to the DATETIME structure to compare                  *
 * Returns:                                                                             *
 *      ulDiff ........ Difference in seconds                                           *
\*--------------------------------------------------------------------------------------*/
ULONG   Diff_DateTime(DATETIME *pPrevious, DATETIME *pLater)
{
ULONG   ulPrevious;
ULONG   ulLater;
                                        /* First date/time */
ulPrevious=pPrevious->hours*60*60+pPrevious->minutes*60+pPrevious->seconds;
                                        /* Last date/time */
ulLater=pLater->hours*60*60+pLater->minutes*60+pLater->seconds;
                                        /* On day wrap around add one day */
if(ulLater<ulPrevious) ulLater+=24*60*60;
return(ulLater-ulPrevious);
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to display a message box to the user.                                      *
 * Req:                                                                                 *
 *      pucError ...... Pointer to an error string                                      *
 * Returns:                                                                             *
 *      usResponse .... Response of the user (the button pressed)                       *
\*--------------------------------------------------------------------------------------*/
USHORT  User_Error(UCHAR *pucError)
{
return(WinMessageBox(                   /* Display the error on the screen */
    HWND_DESKTOP,                       /* Handle of parent-window */
    HWND_DESKTOP,                       /* Handle of owner-window */
    pucError,                           /* Message-box window message */
                                        /* Message-box window title */
    "SD/2 - ShutDown/2 Error Information",
    ID_SHUTDOWN,                        /* ID passed to HK_HELP and WM_HELP */
                                        /* Message-box window style */
    MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1));
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to create/close a secondary thread.                                        *
 * Req:                                                                                 *
 *      bCreate ....... Create/close a thread                                           *
 *      pUPS_Thread ... Pointer to a function implementing the thread                   *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    UPS_ThreadCreate(BOOL bCreate, PFNTHREAD pUPS_Thread)
{
static ULONG    ulThreadArg=0;
static ULONG    ulThreadFlag=0;
static ULONG    ulStackSize=8192;
static TID      tidThread;

if(bCreate==TRUE)
    {
    DosCreateThread(                    /* Create a secondary thread */
        &tidThread,                     /* Thread ID of created thread */
        pUPS_Thread,                    /* Pointer to thread */
        ulThreadArg,                    /* Thread arguments */
        ulThreadFlag,                   /* Thread flags */
        ulStackSize);                   /* Thread's stack size */
    return(TRUE);
    }
else
    {
    DosKillThread(                      /* Kill the secondary thread */
        tidThread);                     /* Thread ID of thread to be killed */
    return(TRUE);
    }
}

/*--------------------------------------------------------------------------------------*\
 * Procedure to write to logfile.                                                       *
 * Req:                                                                                 *
 *      ulMsg ......... Message ID                                                      *
 *      mpParam ....... Pointer to parameter                                            *
 * Ref:                                                                                 *
 *      pucSD2Logfile . Pointer to name of logfile                                      *
 * Returns:                                                                             *
 *      TRUE/FALSE .... If called sucessfully/unsucessfully                             *
\*--------------------------------------------------------------------------------------*/
BOOL    UPS_LogfileIO(ULONG ulMsg, MPARAM mpParam)
{
FILE            *Logfile;
DATETIME        *pDT=PVOIDFROMMP(mpParam);

if((Logfile=fopen(pucSD2Logfile, "a"))==NULL)
    return(FALSE);                      /* Error, couldn't open logfile */
switch(ulMsg)
{
case LF_POWERFAILED:
    {                                   /* Line power failed */
    fprintf(Logfile, "*********************************************************************\n");
    fprintf(Logfile, "*** SD/2 detected: Line power failed at:   %4d/%02d/%02d at %02d:%02d:%02d ***\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "*********************************************************************\n");
    break;
    }
case LF_POWERRETURNED:
    {                                   /* Line power returned */
    fprintf(Logfile, "*********************************************************************\n");
    fprintf(Logfile, "*** SD/2 detected: Line power returned at: %4d/%02d/%02d at %02d:%02d:%02d ***\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "*********************************************************************\n\n");
    break;
    }
case LF_UPS_ALERTSTARTTIME:
    {                                   /* Planned Alert will start at... */
    fprintf(Logfile, "    Planning Alert start at:               %4d/%02d/%02d at %02d:%02d:%02d\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    break;
    }

case LF_UPS_ALERTSHUTDOWNTIME:
    {                                   /* Planned user configurable program will be started at...*/
    fprintf(Logfile, "    Planning User-Application start at:    %4d/%02d/%02d at %02d:%02d:%02d\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    break;
    }

case LF_UPS_SHUTDOWNTIME:
    {                                   /* Planned OS/2 DosShutdown will be called at...*/
    fprintf(Logfile, "    Planning OS/2 ShutDown at:             %4d/%02d/%02d at %02d:%02d:%02d\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    break;
    }

case LF_UPS_ALERTSTART:
    {                                   /* Alert will start at...*/
    fprintf(Logfile, "    Alert started at:                      %4d/%02d/%02d at %02d:%02d:%02d\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    break;
    }
case LF_UPS_ALERTSHUTDOWN:
    {                                   /* The user configurable program will be started at...*/
    fprintf(Logfile, "    User-Application started at:           %4d/%02d/%02d at %02d:%02d:%02d\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    break;
    }
case LF_UPS_SHUTDOWN:
    {                                   /* OS/2 DosShutdown will be called at...*/
    fprintf(Logfile, "    OS/2 ShutDown started at:              %4d/%02d/%02d at %02d:%02d:%02d\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    break;
    }
case LF_INIFILE_MISSING:
    {                                   /* SD/2 profile is missing or defective */
    fprintf(Logfile, "\n+-------------------------------------------------------------------+\n");
    fprintf(Logfile, "| ! SD/2 detected: Profile defective at:   %4d/%02d/%02d at %02d:%02d:%02d ! |\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "| ! Creating default SHUTDOWN.INI - please adjust immediately!    ! |\n");
    fprintf(Logfile, "+-------------------------------------------------------------------+\n\n");
    break;
    }
case LF_INIFILE_PROBLEM:
    {                                   /* SD/2 profile can't be accessed */
    fprintf(Logfile, "\n+-------------------------------------------------------------------+\n");
    fprintf(Logfile, "| ! SD/2 detected: Profile defective at:   %4d/%02d/%02d at %02d:%02d:%02d ! |\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "| ! Can't access SHUTDOWN.INI - Serious problem, please correct!  ! |\n");
    fprintf(Logfile, "+-------------------------------------------------------------------+\n\n");
    break;
    }
case LF_PARALLEL_INITIALIZED:
    {                                   /* UPS <-> SD/2 interface at parallel port */
    fprintf(Logfile, "\n*********************************************************************\n");
    fprintf(Logfile, "*** Parallel port initialized for UPS <-> SD/2 communication      ***\n");
    fprintf(Logfile, "*** Current System Date & Time is:         %4d/%02d/%02d at %02d:%02d:%02d ***\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "*********************************************************************\n\n");
    break;
    }
case LF_SERIAL_INITIALIZED:
    {                                   /* UPS <-> SD/2 interface at serial port */
    fprintf(Logfile, "\n*********************************************************************\n");
    fprintf(Logfile, "*** Serial port initialized for UPS <-> SD/2 communication        ***\n");
    fprintf(Logfile, "*** Current System Date & Time is:         %4d/%02d/%02d at %02d:%02d:%02d ***\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "*********************************************************************\n\n");
    break;
    }
case LF_UPS_PASSWORD_USED:
    {                                   /* The program was invoked with the undocumented
                                           commandline option [-|/]IBMSERVICE */
    fprintf(Logfile, "\n+-------------------------------------------------------------------+\n");
    fprintf(Logfile, "| ! SD/2 detected: Service applied at:     %4d/%02d/%02d at %02d:%02d:%02d ! |\n",
        pDT->year, pDT->month, pDT->day, pDT->hours, pDT->minutes, pDT->seconds);
    fprintf(Logfile, "| !                                                               ! |\n");
    fprintf(Logfile, "| ! The configuration service option is used. Please be sure that ! |\n");
    fprintf(Logfile, "| ! all changes of the configuration correspond to their equiva-  ! |\n");
    fprintf(Logfile, "| ! lent settings of the UPS control logic.                       ! |\n");
    fprintf(Logfile, "| ! Warning! SD/2 and the UPS control logic may not operate       ! |\n");
    fprintf(Logfile, "| ! correctly, if this service isn't performed by IBM!            ! |\n");
    fprintf(Logfile, "+-------------------------------------------------------------------+\n\n");
    break;
    }
}
fclose(Logfile);
return(TRUE);
}

