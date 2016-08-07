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
	imm32.lib ole32.lib

all: $(EXEFILE)

# all: $(EXEFILE) $(DLLFILE)

# Visual C++
!IFDEF _NMAKE_VER

CC=cl
LINK=link
RC=rc
RCOPT=/fo 

!IFDEF NODEFAULTLIB

COPT=/c /W3 /O2 /Oi /DNODEFAULTLIB /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
#LOPT=/SUBSYSTEM:WINDOWS /NODEFAULTLIB /merge:.rdata=.text /nologo /MAP
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo /MAP
!IFDEF WIN64
COPT=/GS- $(COPT)
!ELSE
LOPT=$(LOPT) /OPT:NOWIN98 /WS:AGGRESSIVE
!ENDIF

$(EXEFILE): main.obj $(OBJS) nodeflib.obj $(RESFILE) TCDLL.lib
	$(LINK) $(LOPT) main.obj nodeflib.obj $(OBJS) $(RESFILE) TCDLL.lib $(LIBS) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo
!IFNDEF WIN64
LOPT=$(LOPT) /OPT:NOWIN98 /WS:AGGRESSIVE
!ENDIF

$(EXEFILE): main.obj $(OBJS) $(RESFILE)  TCDLL.lib
	$(LINK) $(LOPT) main.obj $(OBJS) $(RESFILE) TCDLL.lib $(LIBS) /OUT:$@

!ENDIF

# Borland C++ Compiler
!ELSE

CC=bcc32
LINK=ilink32
RC=brc32
RCOPT=-r -32 -fo

!IFDEF NODEFAULTLIB

COPT=-c -w -w-8057 -O2 -Oi -d -DNODEFAULTLIB -tW -o
LOPT=/c /C /Gn /x

$(EXEFILE): main.obj $(OBJS) nodeflib.obj bccexe.pat $(RESFILE) TCDLL.lib
	$(LINK) $(LOPT) /Tpe /aa main.obj $(OBJS) nodeflib.obj bccexe.pat,$@,,$(LIBS) TCDLL.lib,,$(RESFILE)
	del $(TDSFILE)

bccexe.pat: $(COMMONDIR)\bccexe.nas
	nasmw -f obj -o $@ $(COMMONDIR)\bccexe.nas

!ELSE

COPT=-c -w -w-8057 -O2 -Oi -d -tW -o
LOPT=/c /C /Gn /x

$(EXEFILE): main.obj $(OBJS) $(RESFILE) TCDLL.lib
	$(LINK) $(LOPT) /Tpe /aa main.obj $(OBJS) c0w32.obj,$@,,$(LIBS) cw32.lib TCDLL.lib,,$(RESFILE)
	del $(TDSFILE)

!ENDIF

!ENDIF

# obj files

main.obj: $(SRCDIR)\main.c $(TCLOCKH)
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
main2.obj: $(SRCDIR)\main2.c $(TCLOCKH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\main2.c
wndproc.obj: $(SRCDIR)\wndproc.c $(TCLOCKH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\wndproc.c
cmdopt.obj: $(SRCDIR)\cmdopt.c $(TCLOCKH)
	$(CC) $(COPT)$@ $(SRCDIR)\cmdopt.c
command.obj: $(SRCDIR)\command.c $(TCLOCKH) $(COMMONDIR)\command.h
	$(CC) $(COPT)$@ $(SRCDIR)\command.c
menu.obj: $(SRCDIR)\menu.c $(TCLOCKH) $(COMMONDIR)\command.h
	$(CC) $(COPT)$@ $(SRCDIR)\menu.c
alarm.obj: $(SRCDIR)\alarm.c $(TCLOCKH)
	$(CC) $(COPT)$@ $(SRCDIR)\alarm.c
mouse.obj: $(SRCDIR)\mouse.c $(TCLOCKH) $(COMMONDIR)\command.h ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\mouse.c
mouse2.obj: $(SRCDIR)\mouse2.c $(TCLOCKH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\mouse2.c
sntp.obj: $(SRCDIR)\sntp.c $(TCLOCKH) $(COMMONDIR)\command.h
	$(CC) $(COPT)$@ $(SRCDIR)\sntp.c
about.obj: $(SRCDIR)\about.c $(TCLOCKH)
	$(CC) $(COPT)$@ $(SRCDIR)\about.c

# common obj files

langcode.obj: $(COMMONDIR)\langcode.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\langcode.c
playfile.obj: $(COMMONDIR)\playfile.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\playfile.c
alarmstruct.obj: $(COMMONDIR)\alarmstruct.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\alarmstruct.c
mousestruct.obj: $(COMMONDIR)\mousestruct.c $(COMMONH) $(COMMONDIR)\command.h ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\mousestruct.c
utl.obj: ..\common\utl.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\utl.c
exec.obj: $(COMMONDIR)\exec.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\exec.c
reg.obj: $(COMMONDIR)\reg.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\reg.c
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\nodeflib.c
autoformat.obj: $(COMMONDIR)\autoformat.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\autoformat.c
localeinfo.obj: $(COMMONDIR)\localeinfo.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\localeinfo.c
mixer.obj: $(COMMONDIR)\mixer.c $(COMMONH) ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\mixer.c
vistavol.obj: $(COMMONDIR)\vistavol.cpp ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\vistavol.cpp

# res file

$(RESFILE): $(RCFILE) $(SRCDIR)\tclock.manifest
	$(RC) $(RCOPT)$@ $(RCFILE)
