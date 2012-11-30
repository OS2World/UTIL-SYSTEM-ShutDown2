:userdoc.
:title.ShutDown/2 Help Program
:body.

.****************************************************************************************
.*--------------------------------------------------------------------------------------*
.* The user selected help with F1, the mouse or the keyboard.                           *
.*--------------------------------------------------------------------------------------*
.****************************************************************************************

:h1 res=350.SD/2 Help for Help
:lines align=center.
:color fc=red.
SD/2 - ShutDown/2 Version 1.01
for IBM OS/2 2.x Presentation Manager
Copyright (C) by Stangl Roman 06, 1993
Copyright (C) by Friedrich Schmieder 06, 1993
:color fc=default.
:elines.
:p.Please select one of the following topics for further information:
:ul compact.
:li.:link reftype=hd res=600.About the utility SD/2:elink.
:li.:link reftype=hd res=400.SD/2 Configuration Menu:elink.
:eul.
:p.:link reftype=hd res=350.<Backward>:elink.
                                :link reftype=hd res=600.<Forward>:elink.


.****************************************************************************************
.*--------------------------------------------------------------------------------------*
.* The user selected help from the About SD/2 dialog box.                               *
.*--------------------------------------------------------------------------------------*
.****************************************************************************************

:h1 res=600.Help for About SD/2
:lines align=center.
:color fc=red.
SD/2 - ShutDown/2 Version 1.01
for IBM OS/2 2.x Presentation Manager
Copyright (C) by Stangl Roman 06, 1993
Copyright (C) by Friedrich Schmieder 06, 1993
:color fc=default.
:elines.
:p.E-Mail:
:ul compact.
:li.rstangl@vnet.ibm.com (Roman Stangl)
:li.SFRIEDRICH@vnet.ibm.com (Friedrich Schmieder)
:eul.
:p.
ShutDown/2 - SD/2 is an OS/2 PM application, that controls your PS/2
in conjunction with an external uninterruptable power supply (UPS) control
logic.
It will protect you system from any data losses and hardware damages
caused by unexpected line power failures.
If line power fails, SD/2 and the external UPS control logic will shut down
your system safely.
:p.
The following figure shows, how SD/2 and the external UPS control logic
work together:
:cgraphic.
  ÚÄÄÄÄÄÄÄÄÄ¿                     ÚÄÄÄÄÄÄÄÄÄÄÄÄ¿
  ³ÚÄÄÄÄÄÄÄ¿³                     ³ &BOX.          ³
  ³³       ³³   <ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ> ÃÄÄÄÄÄÄÄÄÄÄÄÄ´
  ³³       ³³                     ³            ³
  ³ÀÄÄÄÄÄÄÄÙ³                     ³            ³
  ÀÄÄÄÂÄÄÂÄÄÙ                     ³            ³
ÚÄÄÄÄÄÁÄÄÁÄÄÄÄ¿      ÚÄÄÄÄÄ¿      ³            ³      ÚÄÄÄÄÄ¿
³  ÍÍÍ  ÍÍÍ &BOX. ³ <ÍÍ> ³     ³ <ÍÍ> ³            ³ <ÍÍ> ³ 220 ³
³             ³      ³     ³      ³            ³      ³  V  ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÙ      ÀÄÄÄÄÄÙ      ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ      ÀÄÄÄÄÄÙ

   Your PS/2       UPS control          UPS         Line power
                      logic
:ecgraphic.
:p.
The UPS control logic is attached to the UPS on one side, and to your PS/2
on the other side. Your PS/2 is also attached to the UPS.
The UPS has a battery inside, to supply your system with power for
short periods, if line power fails.
Such periods are typically 5 to 30 minutes, enough time to
shut down your system to avoid data losses or hardware damages.
:p.
The UPS control logic is attached to your PS/2 on any serial or parallel
port.
SD/2 snoops at the configured port for line power failures, which
are indicated by the UPS control logic.
Power failures may vary in their duration, short failures are simply
ignored, because the UPS has enough battery capacity to allow further
operation of your system.
Some failures however, may be too long to allow further operation 
of your system, so SD/2 shuts it down.
Immediately after your system was shut down, it is safe to power it off,
and this job is done by the UPS control logic.
:p.
If line power reappears again, the UPS control logic will power on your
system again.
Your system will be rebooted and normal operation will continue.
:p.
SD/2 and the UPS control logic define 4 time stamps, which are
defined according to the size of the UPS and the power requirements of
your system:
:parml break=all.
:pt.UPS Battery Capacity
:pd.This time stamp determines the maximum period the UPS can be allowed
to supply your system before the UPS battery is empty.
:pt.Alert Start
:pd.This time stamp determines the duration of line power failures that
can safely be ignored without interrupting your system.
:pt.Alert ShutDown
:pd.This time stamp determines when an user defined application is
started, to prepare your system for a shut down.
:pt.SD/2 ShutDown
:pd.This time stamp determines when your system will be shut down after
line power disappears.
Once this time stamp is reached, a reappearing line power can't prevent
your system from shut down.
:eparml.
:p.
SD/2 can be controlled from Parallel or Serial ports, you just have to find
out from your PC's User's Guide and BIOS which physical connector is configured
for which address.
For example, the logically first printer port LPT1 historically was controlled
by the physical address 0x3BC, however since advanced printer functions as
ECP or EPP were introduced, LPT1 is controlled by the physical address 0x278.
In other words, there is no standard rule, but (HW and BIOS) implementation specific,
in the :link reftype=hd res=400.<Setup dialog>:elink. you have to ensure
that the physical addresses match.
:p.
For parallel ports SD/2 senses the status of the pins :hp2.+BUSY (pin 11):ehp2. and
:hp2.-ACK (pin 10):ehp2., an external logic is controlled by the pins
:hp2.-SLCT IN (pin 17):ehp2. and :hp2.-INIT (pin 16):ehp2..
Due to the way these pins are used, you should even be able to loop your UPS logic
between your PC and printer and the printer should still work.
SD/2 detects that the UPS is running from its internal battery when both pins 11 and
10 are connected to ground (e.g. for a test plug, just connect ground from pins 25
and 24 to pins 11 and 10, once you attacht that plug, SD/2 will start its processing).
:p.
For serial ports SD/2 senses the status of the pins :hp2.CTS (pin 5 on DB25):ehp2. and
:hp2.CD (pin 8 on DB25):ehp2., an external logic is controlled by the pins
:hp2.DTR (pin 20 on DB25):ehp2. and :hp2.RTS (pin 4 on DB25):ehp2..
Due to the way these pins are used, you should even be able to loop your UPS logic
between your PC and your serail device and the serial device should still work.
SD/2 detects that the UPS is running from its internal battery when pin 5 (on DB25)
is connected to positive voltage (a logical "0" in serial communication terms) and
pin 8 (on DB25) is connected to negative voltage (a logical "1" in serial communication
terms) (e.g. of course you can build a test plug too, but it's a little bit more
difficult to the the serial RS232 voltage levels, you might e.g. use DOS' DEBUG to
output them on pins unused by SD/2 or on a different serial port).
:p.:link reftype=hd res=350.<Backward>:elink.
                                :link reftype=hd res=400.<Forward>:elink.


.****************************************************************************************
.*--------------------------------------------------------------------------------------*
.* The user selected help from the Configuration/Setup SD/2 dialog box.                 *
.*--------------------------------------------------------------------------------------*
.****************************************************************************************

:h1 res=400.Help for SD/2 Setup
:p.
This dialog is used to setup the cooperation between SD/2 and the UPS
control logic. Some entryfields are editable by the user, some entryfields
are predefined for a particular UPS and PS/2 configuration.
:p.
For the following explanation assume that line power fails at 10&colon.30
and the configuration corresponds to your UPS and PC/2 configuration:
:parml break=all.
:pt.UPS Battery Capacity
:pd.Assume that the UPS battery capacity is 0&colon.10.
Because the battery of the UPS lasts at least 10 minutes, the system
must be shut down at 10&colon.40 at the latest, to avoid data losses or hardware
damages due to uncontrolled power off of your system. All time stamps must
occur between 10&colon.30 and 10&colon.40.
:pt.Alert Start
:pd.Assume that the alert start is 0&colon.1.
Therefor at 10&colon.31 SD/2 will inform the user that the line power failed, and
the system is operating from the UPS battery.
The first minute is ignored, because it is safe to ignore short glitches
of up to one minute.
:pt.Alert ShutDown
:pd.Assume that the alert shut down is 0&colon.4.
At 10&colon.34, a user defined application will be started to prepare the system
for shut down.
:pt.SD/2 ShutDown
:pd.Assume that SD/2 shut down is 0&colon.7.
At 10&colon.37 SD/2 will signalize the UPS control logic to power off your system
after SD/2 will have shut down your system, which lasts about 2 seconds.
Thus 10&colon.37 is the point of no return, even if line power reappears, your
system will be shut down.
If line power reappears before this time stamp (f.e. at 10&colon.35), SD/2 will
restart the countdown at the beginning, repeating this procedure immediately
after the next line power disappearance.
:pt.Application started at Alert ShutDown.
:pd.These entryfields allow the user to select the application that is
started after the Alert ShutDown time stamp is reached.
The user has to insert at least the path and filename of an application.
:pt.UPS controls SD/2 via
:pd.These radiobuttons allow the user to select the port of the PS/2
where SD/2 and the UPS control logic communicate over. The user can
select any serial or parallel port.
:caution.
Be sure that the UPS control logic is attached to the port that is
specified! Data may be lost or hardware may be damaged, if SD/2 is
configured to a port where the UPS control logic is not attached
and line power fails!
:ecaution.
:pt.ShutDown User Information
:pd.This multiline entryfield contains the text that is displayed,
immediately before your system is shut down, until your system is
reset by ALT+CTRL+DEL or by a power off.
The UPS control logic will power off your system shortly after this
message is displayed.
:eparml.
:warning.
Be sure to verify all data before you press :hp2.OK:ehp2. to prevent
misfunction of SD/2 and the UPS control logic!
:ewarning.
:p.:link reftype=hd res=600.<Backward>:elink.
                                :link reftype=hd res=350.<Forward>:elink.

:euserdoc.

