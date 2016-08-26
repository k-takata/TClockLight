# ------------------------------------------
# tclock.mak
#-------------------------------------------

!IFNDEF SRCDIR
SRCDIR=.
!ENDIF

!IFNDEF COMMONDIR
COMMONDIR=..\common
!ENDIF

!IFNDEF OUTDIR
OUTDIR=..\out
!ENDIF

EXEFILE=$(OUTDIR)\tclock.exe
DLLFILE=$(OUTDIR)\tclock.dll
DEFFILE=
RCFILE=$(SRCDIR)\tclock.rc
RESFILE=tclock.res
TDSFILE=$(OUTDIR)\tclock.tds
TCLOCKH=$(SRCDIR)\tclock.h $(COMMONDIR)\common.h
COMMONH=$(COMMONDIR)\common.h

OBJS=main2.obj wndproc.obj cmdopt.obj command.obj menu.obj\
	alarm.obj mouse.obj mouse2.obj about.obj\
	langcode.obj utl.obj exec.obj reg.obj autoformat.obj localeinfo.obj\
	playfile.obj alarmstruct.obj mousestruct.obj mixer.obj vistavol.obj

LIBS=kernel32.lib user32.lib gdi32.lib advapi32.lib shell32.lib winmm.lib\
	imm32.lib ole32.lib dwmapi.lib

all: $(EXEFILE)

# all: $(EXEFILE) $(DLLFILE)

CC=cl
LINK=link
RC=rc
RCOPT=/fo 

!IFDEF NODEFAULTLIB

COPT=/c /GS- /W3 /O2 /Oi /DNODEFAULTLIB /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
#LOPT=/SUBSYSTEM:WINDOWS /NODEFAULTLIB /merge:.rdata=.text /nologo /MAP
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo /MAP
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98 /WS:AGGRESSIVE
!ENDIF
!IF $(MSVC_MAJOR) >= 14
LIBS=$(LIBS) libvcruntime.lib
!ENDIF

$(EXEFILE): main.obj $(OBJS) nodeflib.obj $(RESFILE) TCDLL.lib
	$(LINK) $(LOPT) main.obj nodeflib.obj $(OBJS) $(RESFILE) TCDLL.lib $(LIBS) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98 /WS:AGGRESSIVE
LOPT=$(LOPT) /WS:AGGRESSIVE
!ENDIF

$(EXEFILE): main.obj $(OBJS) $(RESFILE)  TCDLL.lib
	$(LINK) $(LOPT) main.obj $(OBJS) $(RESFILE) TCDLL.lib $(LIBS) /OUT:$@

!ENDIF

{$(COMMONDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<
{$(SRCDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<


# obj files

main.obj: $(SRCDIR)\main.c $(TCLOCKH)
main2.obj: $(SRCDIR)\main2.c $(TCLOCKH) ..\config.h
wndproc.obj: $(SRCDIR)\wndproc.c $(TCLOCKH) ..\config.h
cmdopt.obj: $(SRCDIR)\cmdopt.c $(TCLOCKH)
command.obj: $(SRCDIR)\command.c $(TCLOCKH) $(COMMONDIR)\command.h
menu.obj: $(SRCDIR)\menu.c $(TCLOCKH) $(COMMONDIR)\command.h
alarm.obj: $(SRCDIR)\alarm.c $(TCLOCKH)
mouse.obj: $(SRCDIR)\mouse.c $(TCLOCKH) $(COMMONDIR)\command.h ..\config.h
mouse2.obj: $(SRCDIR)\mouse2.c $(TCLOCKH) ..\config.h
sntp.obj: $(SRCDIR)\sntp.c $(TCLOCKH) $(COMMONDIR)\command.h
about.obj: $(SRCDIR)\about.c $(TCLOCKH)

# common obj files

langcode.obj: $(COMMONDIR)\langcode.c $(COMMONH)
playfile.obj: $(COMMONDIR)\playfile.c $(COMMONH)
alarmstruct.obj: $(COMMONDIR)\alarmstruct.c $(COMMONH)
mousestruct.obj: $(COMMONDIR)\mousestruct.c $(COMMONH) $(COMMONDIR)\command.h ..\config.h
utl.obj: ..\common\utl.c $(COMMONH)
exec.obj: $(COMMONDIR)\exec.c $(COMMONH)
reg.obj: $(COMMONDIR)\reg.c $(COMMONH)
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)
autoformat.obj: $(COMMONDIR)\autoformat.c $(COMMONH)
localeinfo.obj: $(COMMONDIR)\localeinfo.c $(COMMONH)
mixer.obj: $(COMMONDIR)\mixer.c $(COMMONH) ..\config.h
vistavol.obj: $(COMMONDIR)\vistavol.cpp ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\vistavol.cpp

# res file

$(RESFILE): $(RCFILE) $(SRCDIR)\tclock.manifest
	$(RC) $(RCOPT)$@ $(RCFILE)
