#===================================================================
#
#   ShutDown/2 Makefile
#
#===================================================================

# ICC Flags
#   /Ti     Include debug information for IPMD
#   /Kb+    Warning, if no prototypes found (prevents from incorrect params)
#   /c      Compile only, we link more than one ressource
#   /Se     Allow IBM C language extentions and migration
#   /Ms     Set the default linkage to _System
#   /Re     Produce Code in IBM C Set/2 Run Time environment
#   /ss     Allow // as comments
#   /Gm+    Link with multitasking libraries, because we're multithreaded
#   /Ge-    Link with libraries that assume a DLL
CFLAGS = /Ti /Kb+ /c /Se /Ms /Re /ss /Gm+

# LINK386 Flags
#   /DEBUG  Include debug information for IPMD
#   /NOE    NO Extented dictionary, don't assume any library dependencies
#   /NOD    NO Default library, ignore the library names in object files
#   /A:16   Align on paragraph for PM programs
#   /M      Produce map
#   /BASE   Base over 1Meg ?
LFLAGS = /NOFREE /DEBUG /NOE /NOD /ALIGN:16 /MAP /NOI

# MicroSoft MASM 6.0 Flags
#   /c      Compile only, we link more than one ressource
#   /Cx     Preserve case in publics, externs
#   /Cp     Preserve case of user identifiers
#   /Fl     Generate listings
MFLAGS = /c /Cx /Cp /Fl

# Libraries
#   DD4MBS  Multitasking standard library
#   OS2386  OS/2 2.0 Link library
LIBS = DDE4MBS + OS2386

CC = icc $(CFLAGS)
ASM = ML $(MFLAGS)
LINK = LINK386 $(LFLAGS)

HEADERS = 

ALL_OBJ = ShutDown.obj Utility.obj Io.obj

all: ShutDown.exe ShutDown.hlp

clean:
    -!del *.map
    -!del *.lst
    -!del *.exe
    -!del *.obj
    -!del *.res

save:
    -!del *.map
    zip -9 ShutDown *
    unzip -t ShutDown

ShutDown.obj: ShutDown.c $(HEADERS)
    $(CC) ShutDown.c

Utility.obj: Utility.c $(HEADERS)
    $(CC) Utility.c

Io.obj: Io.asm
    $(ASM) Io.asm

ShutDown.hlp: ShutDown.ipp
    ipfcprep ShutDown.ipp ShutDown.ipf /D IPFC
    ipfc ShutDown.ipf

ShutDown.l: Makefile
    echo $(ALL_OBJ)                 >  ShutDown.l
    echo ShutDown.exe               >> ShutDown.l
    echo ShutDown.map               >> ShutDown.l
    echo $(LIBS)                    >> ShutDown.l
    echo ShutDown.def               >> ShutDown.l

ShutDown.res: ShutDown.rc ShutDown.h ShutDown.dlg
    rc -r ShutDown.rc

ShutDown.exe: $(ALL_OBJ) ShutDown.def ShutDown.l ShutDown.res
    $(LINK) @ShutDown.l
    rc ShutDown.res ShutDown.exe

