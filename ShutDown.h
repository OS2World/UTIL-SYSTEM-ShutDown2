/***********************************************************************\
 *                              Shutdown.c                             *
 *                 Copyright (C) by Stangl Roman, 1993                 *
 *                                                                     *
 * ShutDown.h   ShutDown/2 header file.                                *
 *                                                                     *
\***********************************************************************/

#ifndef IPFC                            /* Don't include if we compile HLP files */
#define         INCL_WIN                /* Environment include files */
#define         INCL_GPI
#define         INCL_DOS
#include        <os2.h>

#include        <stdio.h>               /* C Set/2 include files */
#include        <string.h>
#include        <stdlib.h>
#include        <ctype.h>
#endif

#define SD2_CLASSNAME           "SHUTDOWN"
                                        /* Semaphore to avoid loading SD/2 twice */
#define SD2_SEM                 "\\SEM32\\SD2.SEM"

#define SIZE_X                  200     /* Gauge size */
#define SIZE_Y                  25
#define LEFT_X                  35      /* Left X coordinate of gauge */
#define RIGHT_X                 LEFT_X+SIZE_X
#define DOWN_Y                  55      /* Lower Y coordinate of gauge */
#define UP_Y                    DOWN_Y+SIZE_Y
#define WINDOW_SIZE_X           SIZE_X+2*LEFT_X
#define WINDOW_SIZE_Y           UP_Y+15

#define ID_SHUTDOWN             256
#define IDP_ICON1               256     /* RC has problems on using constants as an ID */
#define IDP_ICON2               257

#define ID_EXIT                 300

#define ID_HELP                 350
#define MAIN_HELP_TABLE         351
#define MAIN_HELP_SUBTABLE      352

#define ID_CONFIG               400
#define GRP_UPS_BATTERYCAPACITY 401
#define IDUBC_HOURS             402
#define IDUBC_MINUTES           403
#define GRP_UPS_ALERTSTART      404
#define IDUAS_HOURS             405
#define IDUAS_MINUTES           406
#define GRP_UPS_ALERTSHUTDOWN   407
#define IDUASD_HOURS            408
#define IDUASD_MINUTES          409
#define GRP_UPS_SHUTDOWN        410
#define IDUS_HOURS              411
#define IDUS_MINUTES            412
#define GRP_SESSION             413
#define IDS_PGMNAME             414
#define IDS_PGMDIRECTORY        415
#define IDS_PGMINPUTS           416
#define GRP_MODE                417
#define IDM_LPT1                418
#define IDM_LPT2                419
#define IDM_LPT3                420
#define IDM_LPT4                421
#define IDM_COM1                422
#define IDM_COM2                423
#define IDM_COM3                424
#define IDM_COM4                425
#define IDM_COM5                426
#define IDM_COM6                427
#define IDM_COM7                428
#define IDM_COM8                429
#define IDM_ADAPTER             430
#define GRP_USERINFO            440
#define IDS_USERINFO            441     /* User entered string to be displayed in ShutDown
                                           dialog box, so this ID is also used in the
                                           ShutDown dialog box */
#define IDS_SHUTDOWNINFO        442     /* String to be displayed after ShutDown has finished */

#define ID_SHUTDOWNDIALOG       500

#define ID_ABOUT                600

#define WM_DRAWUPSBAR                   WM_USER+1
#define WM_DRAWUPSTIME                  WM_USER+2
#define WM_FLASHICON                    WM_USER+3
#define WM_SHUTDOWNDIALOG               WM_USER+4
#define WM_SD2POSITION                  WM_USER+5
#define WM_ENABLEMENU                   WM_USER+6
#define WM_DISABLEMENU                  WM_USER+7

#define LF_POWERFAILED                  WM_USER+8
#define LF_POWERRETURNED                WM_USER+9
#define LF_UPS_ALERTSTARTTIME           WM_USER+10
#define LF_UPS_ALERTSHUTDOWNTIME        WM_USER+11
#define LF_UPS_SHUTDOWNTIME             WM_USER+12
#define LF_UPS_ALERTSTART               WM_USER+13
#define LF_UPS_ALERTSHUTDOWN            WM_USER+14
#define LF_UPS_SHUTDOWN                 WM_USER+15
#define LF_INIFILE_MISSING              WM_USER+16
#define LF_INIFILE_PROBLEM              WM_USER+17
#define LF_PARALLEL_INITIALIZED         WM_USER+18
#define LF_SERIAL_INITIALIZED           WM_USER+19
#define LF_UPS_PASSWORD_USED            WM_USER+20

#define WM_SHUTDOWN                     WM_USER+21

typedef struct _SD2CLIENT       SD2CLIENT;
typedef struct _UPS             UPS;

struct _SD2CLIENT
{                                       /* Structure of SD/2'c client window */
SWP     swpSD2Client;                   /* SD/2's client position & size on display */
SWP     swpSD2Gauge;                    /* Gauge position & size in client window */
SWP     swpSD2CountDown;                /* Position & size of countdown string */
};

struct  _UPS
{                                       /* Structure thata contains profile data */
UCHAR   IDUBC_Hours;                    /* UPS battery capacity */
UCHAR   IDUBC_Minutes;
UCHAR   IDUAS_Hours;                    /* Alert user after ... */
UCHAR   IDUAS_Minutes;
UCHAR   IDUASD_Hours;                   /* Start user defined session for user shutdown at ... */
UCHAR   IDUASD_Minutes;
UCHAR   IDUS_Hours;                     /* ShutDown/2 automatic shut down at ... */
UCHAR   IDUS_Minutes;
UCHAR   IDS_PgmName[129];
UCHAR   IDS_PgmDirectory[129];
UCHAR   IDS_PgmInputs[129];
ULONG   GRP_Mode;
UCHAR   IDS_UserInfo[257];              /* User editable information displayed in the
                                           ShutDown dialog box */
};
                                        /* Procedures */
extern int      main(int argc, char *argv[], char *envp[]);
extern BOOL     WinStartUp(HAB *pHab, HMQ *pHmq);
extern BOOL     WinStartHelp(HAB hab, UCHAR *pHelpFile, HWND *pHwndHelp);
extern BOOL     WinCloseDown(HWND *pHwndHelp, HAB *pHab, HMQ *pHmq);
extern BOOL     Write_Profile(HINI *pHini, HAB *pHab, UPS *pUps);
extern BOOL     Read_Profile(HINI *pHini, HAB *pHab, UPS *pUps);
extern BOOL     Copy_DateTime(DATETIME *pDestination, DATETIME *pSource);
extern BOOL     Add_DateTime(DATETIME *pDestination, UCHAR *pulHours, UCHAR *pulMinutes);
extern BOOL     Cmp_DateTime(DATETIME *pDestination, DATETIME *pSource);
extern ULONG    Diff_DateTime(DATETIME *pPrevious, DATETIME *pLater);
extern USHORT   User_Error(UCHAR *pucError);
extern BOOL     UPS_PortReset(void);
extern BOOL     UPS_LinePower(BOOL Set);
extern BOOL     UPS_ThreadCreate(BOOL bCreate, PFNTHREAD pUPS_Thread);
extern void     UPS_Thread(ULONG ulThreadArg);
extern BOOL     UPS_LogfileIO(ULONG ulMsg, MPARAM mpParam);
extern _Far16 _Pascal       OUT8(USHORT Port, UCHAR Data);
extern UCHAR _Far16 _Pascal IN8(USHORT Port);


                                        /* Window procedures */
extern MRESULT  EXPENTRY SD2_MainWindowProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
extern MRESULT  EXPENTRY SD2_ConfigDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
extern MRESULT  EXPENTRY SD2_ShutDownDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);
extern MRESULT  EXPENTRY SD2_AboutDialogProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);

extern HAB      hab;                    /* Handle of PM anchor block */
extern HMQ      hmq;                    /* Handle of message queue */
extern HWND     hwndFrame;              /* Frame handle of window */
extern HWND     hwndClient;             /* Client handle of window */
extern HWND     hwndHelp;               /* Help window handle */
extern UCHAR    *pucSD2Profile;         /* The buffer holding the filename of the
                                           ShutDown.ini profile */
extern UCHAR    *pucSD2Logfile;         /* The buffer holding the filename of the
                                           ShutDown.log log-file */
extern UCHAR    *pucSD2Helpfile;        /* The buffer holding the filename of the HLP file */
extern HPOINTER hptrIcon[1];            /* If line power fails, blink with two icons */
extern HINI     hini;                   /* Handle of profile ShutDown.ini */
extern UPS      Ups;                    /* Profile data */
extern SWP      swpScreen;              /* Desktop size */
extern SWP      swpClient;              /* Client size */
extern BOOL     LinePower;              /* True if line power is available */
extern DATETIME UPS_Start;              /* Date&Time when line power fails */
extern DATETIME UPS_AlertStart;         /* Date&Time when user should be alert */
extern DATETIME UPS_AlertShutDown;      /* Date&Time when user supplied session should be started */
extern DATETIME UPS_ShutDown;           /* Date&Time when system is shut down by ShutDown/2 */
extern DATETIME UPS_Current;            /* Date&Time of running system clock */
extern BOOL     ProfileError;           /* TRUE if an error occured accessing SHUTDOWN.INI */

