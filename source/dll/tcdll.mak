# -------------------------------------------
# tcdll.mak
#--------------------------------------------

!IFNDEF SRCDIR
SRCDIR=.
!ENDIF

!IFNDEF COMMONDIR
COMMONDIR=..\common
!ENDIF

DLLFILE=..\out\tcdll.tclock
RCFILE=$(SRCDIR)\tcdll.rc
RESFILE=tcdll.res
LIBFILE=tcdll.lib
DEFFILE=$(SRCDIR)\tcdll.def
TDSFILE=..\out\tcdll.tds
TCDLLH=$(SRCDIR)\tcdll.h $(COMMONDIR)\common.h
COMMONH=$(COMMONDIR)\common.h

OBJS=dllmain.obj dllmain2.obj dllwndproc.obj draw.obj\
	format.obj formattime.obj tooltip.obj userstr.obj\
	startbtn.obj startmenu.obj taskbar.obj taskswitch.obj traynotify.obj\
	bmp.obj newapi.obj dllutl.obj\
	sysinfo.obj net.obj hdd.obj cpu.obj battery.obj mixer.obj\
	dlexec.obj dlutl.obj dlreg.obj dlfont.obj dllocaleinfo.obj

LIBS=kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib


# Visual C++
!IFDEF _NMAKE_VER
CC=cl
LINK=link
RC=rc
RCOPT=/fo 

!IFDEF NODEFAULTLIB

COPT=/c /W3 /O2 /Oi /DNODEFAULTLIB /Fo
LOPT=/SUBSYSTEM:WINDOWS /DLL /OPT:NOWIN98

$(DLLFILE): $(OBJS) nodeflib.obj $(RESFILE)
	$(LINK) $(LOPT) $(OBJS) nodeflib.obj $(RESFILE) $(LIBS) /DEF:$(DEFFILE) /IMPLIB:$(LIBFILE) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /Fo
LOPT=/SUBSYSTEM:WINDOWS /DLL /OPT:NOWIN98

$(DLLFILE): $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) $(OBJS) $(RESFILE) $(LIBS) /DEF:$(DEFFILE) /IMPLIB:$(LIBFILE) /OUT:$@

!ENDIF

# Borland C++ Compiler
!ELSE
CC=bcc32
LINK=ilink32
RC=brcc32
RCOPT=-32 -fo

!IFDEF NODEFAULTLIB
COPT=-c -w -w-8057 -O2 -Oi -d -DNODEFAULTLIB -tWD -tWM -o
LOPT=/c /C /Gn /Tpd /x

$(DLLFILE): $(OBJS) nodeflib.obj bccdll.pat $(RESFILE)
	IMPLIB $(LIBFILE) $(DEFFILE)
	$(LINK) $(LOPT) $(OBJS) nodeflib.obj bccdll.pat,$@,,$(LIBS),$(DEFFILE),$(RESFILE)
	del $(TDSFILE)

!ELSE
COPT=-c -w -w-8057 -O2 -Oi -d -tWD -tWM -o
LOPT=/c /C /Gn /Tpd /x

$(DLLFILE): $(OBJS) bccdll.pat $(RESFILE)
	IMPLIB $(LIBFILE) $(DEFFILE)
	$(LINK) $(LOPT) $(OBJS) bccdll.pat c0d32x.obj,$@,,$(LIBS) noeh32.lib cw32mt.lib,$(DEFFILE),$(RESFILE)
	del $(TDSFILE)

!ENDIF

bccdll.pat: $(SRCDIR)\bccdll.nas
	nasmw -f obj -o $@ $(SRCDIR)\bccdll.nas

!ENDIF

# obj files

dllmain.obj: $(SRCDIR)\main.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
dllmain2.obj: $(SRCDIR)\main2.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\main2.c
dllwndproc.obj: $(SRCDIR)\wndproc.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\wndproc.c
format.obj: $(SRCDIR)\format.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\format.c
formattime.obj: $(SRCDIR)\formattime.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\formattime.c
tooltip.obj: $(SRCDIR)\tooltip.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\tooltip.c
userstr.obj: $(SRCDIR)\userstr.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\userstr.c
draw.obj: $(SRCDIR)\draw.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\draw.c
startbtn.obj: $(SRCDIR)\startbtn.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\startbtn.c
startmenu.obj: $(SRCDIR)\startmenu.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\startmenu.c
taskbar.obj: $(SRCDIR)\taskbar.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\taskbar.c
taskswitch.obj: $(SRCDIR)\taskswitch.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\taskswitch.c
traynotify.obj: $(SRCDIR)\traynotify.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\traynotify.c
bmp.obj: $(SRCDIR)\bmp.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\bmp.c
dllutl.obj: $(SRCDIR)\dllutl.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\dllutl.c
newapi.obj: $(SRCDIR)\newapi.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\newapi.c
sysinfo.obj: $(SRCDIR)\sysinfo.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\sysinfo.c
net.obj: $(SRCDIR)\net.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\net.c
hdd.obj: $(SRCDIR)\hdd.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\hdd.c
cpu.obj: $(SRCDIR)\cpu.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\cpu.c
battery.obj: $(SRCDIR)\battery.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\battery.c

# common obj files

dlutl.obj: $(COMMONDIR)\utl.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\utl.c
dlexec.obj: $(COMMONDIR)\exec.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\exec.c
dlreg.obj: $(COMMONDIR)\reg.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\reg.c
dlfont.obj: $(COMMONDIR)\font.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\font.c
dllocaleinfo.obj: $(COMMONDIR)\localeinfo.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\localeinfo.c
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\nodeflib.c
mixer.obj: $(COMMONDIR)\mixer.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\mixer.c

# res file

$(RESFILE): $(RCFILE)
	$(RC) $(RCOPT)$@ $(RCFILE)

