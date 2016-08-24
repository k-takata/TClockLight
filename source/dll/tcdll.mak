# -------------------------------------------
# tcdll.mak
#--------------------------------------------

!IFNDEF SRCDIR
SRCDIR=.
!ENDIF

!IFNDEF COMMONDIR
COMMONDIR=..\common
!ENDIF

!IFNDEF OUTDIR
OUTDIR=..\out
!ENDIF

DLLFILE=$(OUTDIR)\tcdll.tclock
RCFILE=$(SRCDIR)\tcdll.rc
RESFILE=tcdll.res
LIBFILE=tcdll.lib
DEFFILE=$(SRCDIR)\tcdll.def
TDSFILE=$(OUTDIR)\tcdll.tds
TCDLLH=$(SRCDIR)\tcdll.h $(COMMONDIR)\common.h
COMMONH=$(COMMONDIR)\common.h

OBJS=dllmain.obj dllmain2.obj dllwndproc.obj draw.obj\
	format.obj formattime.obj tooltip.obj userstr.obj\
	startbtn.obj startmenu.obj taskbar.obj taskswitch.obj traynotify.obj\
	bmp.obj newapi.obj dllutl.obj\
	sysinfo.obj net.obj hdd.obj cpu.obj battery.obj mixer.obj \
	vistavol.obj desktop.obj \
	exec.obj utl.obj reg.obj font.obj localeinfo.obj

LIBS=kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib\
	ole32.lib comctl32.lib dwmapi.lib

!IFDEF WIN64
DLLBASE=0x60066040000
!ELSE
DLLBASE=0x66040000
!ENDIF

all: $(DLLFILE)

CC=cl
LINK=link
RC=rc
RCOPT=/fo 

!IFDEF NODEFAULTLIB

COPT=/c /GS- /W3 /O2 /Oi /DNODEFAULTLIB /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /DLL /merge:.rdata=.text /nologo /BASE:$(DLLBASE) /MAP
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98
!ENDIF
!IF $(MSVC_MAJOR) >= 14
LIBS=$(LIBS) libvcruntime.lib
!ENDIF

$(DLLFILE): $(OBJS) nodeflib.obj $(RESFILE)
	$(LINK) $(LOPT) $(OBJS) nodeflib.obj $(RESFILE) $(LIBS) /DEF:$(DEFFILE) /IMPLIB:$(LIBFILE) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /DLL /merge:.rdata=.text /nologo /BASE:$(DLLBASE) /MAP
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98
!ENDIF

$(DLLFILE): $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) $(OBJS) $(RESFILE) $(LIBS) /DEF:$(DEFFILE) /IMPLIB:$(LIBFILE) /OUT:$@

!ENDIF

{$(COMMONDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<
{$(SRCDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<


# obj files

dllmain.obj: $(SRCDIR)\main.c $(TCDLLH)
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
dllmain2.obj: $(SRCDIR)\main2.c $(TCDLLH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\main2.c
dllwndproc.obj: $(SRCDIR)\wndproc.c $(TCDLLH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\wndproc.c
format.obj: $(SRCDIR)\format.c $(TCDLLH) ..\config.h
formattime.obj: $(SRCDIR)\formattime.c $(TCDLLH)
tooltip.obj: $(SRCDIR)\tooltip.c $(TCDLLH)
userstr.obj: $(SRCDIR)\userstr.c $(TCDLLH)
draw.obj: $(SRCDIR)\draw.c $(TCDLLH)
startbtn.obj: $(SRCDIR)\startbtn.c $(TCDLLH) ..\config.h
startmenu.obj: $(SRCDIR)\startmenu.c $(TCDLLH) ..\config.h
taskbar.obj: $(SRCDIR)\taskbar.c $(TCDLLH) ..\config.h
taskswitch.obj: $(SRCDIR)\taskswitch.c $(TCDLLH) ..\config.h
traynotify.obj: $(SRCDIR)\traynotify.c $(TCDLLH) ..\config.h
bmp.obj: $(SRCDIR)\bmp.c $(TCDLLH)
dllutl.obj: $(SRCDIR)\dllutl.c $(TCDLLH)
newapi.obj: $(SRCDIR)\newapi.c $(TCDLLH)
sysinfo.obj: $(SRCDIR)\sysinfo.c $(TCDLLH) ..\config.h
net.obj: $(SRCDIR)\net.c $(TCDLLH) ..\config.h
hdd.obj: $(SRCDIR)\hdd.c $(TCDLLH) ..\config.h
cpu.obj: $(SRCDIR)\cpu.c $(TCDLLH) ..\config.h
battery.obj: $(SRCDIR)\battery.c $(TCDLLH) ..\config.h

# common obj files

utl.obj: $(COMMONDIR)\utl.c $(COMMONH)
exec.obj: $(COMMONDIR)\exec.c $(COMMONH)
reg.obj: $(COMMONDIR)\reg.c $(COMMONH)
font.obj: $(COMMONDIR)\font.c $(COMMONH)
localeinfo.obj: $(COMMONDIR)\localeinfo.c $(COMMONH)
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)
mixer.obj: $(COMMONDIR)\mixer.c $(COMMONH) ..\config.h
vistavol.obj: $(COMMONDIR)\vistavol.cpp ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\vistavol.cpp
desktop.obj: $(COMMONDIR)\desktop.c $(COMMONH) ..\config.h

# res file

$(RESFILE): $(RCFILE)
	$(RC) $(RCOPT)$@ $(RCFILE)

