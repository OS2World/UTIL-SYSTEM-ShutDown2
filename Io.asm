;***********************************************************************\
;*                                Io.asm                               *
;*                 Copyright (C) by Stangl Roman, 1992                 *
;* This Code may be freely distributed, provided the Copyright isn't   *
;* removed.                                                            *
;*                                                                     *
;* IO.asm       Additional utility procedures for ShutDown/2 with IOPL *
;*                                                                     *
;* Part of this code from: Keith Murray (murrayk@prism.cs.orst.edu)    *
;*                         Joel Armengaud (joe2@vnet.ibm.com)          *
;*                                                                     *
;***********************************************************************/

IOPLCSEG SEGMENT PARA PUBLIC USE16 'CODE'
      ASSUME  CS:IOPLCSEG, DS:NOTHING

        db      "@(#) $Header: Io.asm Version 0.10 01,1993 $ (LBL)";

args    struc                           ; Structure for arguments passed on stack
                dw      0h              ;   BP+00 : Saved BP
                dw      0h              ;   BP+02 : IP of return address
                dw      0h              ;   BP+04 : CS of return address
        arg1    dw      0h              ;   BP+06 : First parameter passed
        arg2    dw      0h              ;   BP+08 : Second parameter passed
args    ends

PUBLIC  IN8                             ; IN arg1
IN8     PROC    FAR
        PUSH    BP
        MOV     BP,SP
        MOV     DX,[BP+06h]             ; Get port
        IN      AL,DX                   ; Get data from port
        MOV     AH,0h
        POP     BP
        RET     2                       ; Clear stack
IN8     ENDP

PUBLIC  OUT8                            ; OUT arg1, arg2
OUT8    PROC    FAR
        PUSH    BP
        MOV     BP,SP
        MOV     DX,[BP+08h]             ; Get port
        MOV     AX,[BP+06h]             ; Get data
        OUT     DX,AL                   ; Out data at port
        POP     BP
        RET     4                       ; Clear stack
OUT8    ENDP

IOPLCSEG   ENDS
   END
