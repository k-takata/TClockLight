# ------------------------------------------
# tctimer.mak
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

EXEFILE=$(OUTDIR)\tctimer.exe
LANGID=0x411
RCFILE=$(SRCDIR)\tctimer.rc
RESFILE=tctimer.res
TDSFILE=$(OUTDIR)\tctimer.tds
TCPROPH=$(SRCDIR)\tctimer.h $(SRCDIR)\resource.h $(COMMONDIR)\common.h
COMMONH=$(COMMONDIR)\common.h

OBJS=timermain.obj timerdlg.obj timer.obj\
	tclang.obj langcode.obj selectfile.obj\
	playfile.obj soundselect.obj\
	list.obj utl.obj exec.obj reg.obj font.obj

LIBS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib\
	shell32.lib winmm.lib comctl32.lib dwmapi.lib

all: $(EXEFILE)

CC=cl
LINK=link
RC=rc
RCOPT=/l $(LANGID) /fo 

!IFDEF NODEFAULTLIB

COPT=/c /GS- /W3 /O2 /Oi /DNODEFAULTLIB /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo /MAP
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98
!ENDIF
!IF $(MSVC_MAJOR) >= 14
LIBS=$(LIBS) libvcruntime.lib libucrt.lib
!ENDIF

$(EXEFILE): $(OBJS) nodeflib.obj $(RESFILE)
	$(LINK) $(LOPT) $(OBJS) nodeflib.obj $(RESFILE) $(LIBS) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98
!ENDIF

$(EXEFILE): $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) $(OBJS) $(RESFILE) $(LIBS) /OUT:$@

!ENDIF

{$(COMMONDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<
{$(SRCDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<


# obj files

timermain.obj: $(SRCDIR)\main.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
timerdlg.obj: $(SRCDIR)\dialog.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\dialog.c
timer.obj: $(SRCDIR)\timer.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\timer.c

# common obj files

tclang.obj: $(COMMONDIR)\tclang.c $(COMMONH)
langcode.obj: $(COMMONDIR)\langcode.c $(COMMONH)
selectfile.obj: $(COMMONDIR)\selectfile.c $(COMMONH)
playfile.obj: $(COMMONDIR)\playfile.c $(COMMONH)
soundselect.obj: $(COMMONDIR)\soundselect.c $(COMMONH)
list.obj: $(COMMONDIR)\list.c $(COMMONH)
utl.obj: $(COMMONDIR)\utl.c $(COMMONH)
exec.obj: $(COMMONDIR)\exec.c $(COMMONH)
reg.obj: $(COMMONDIR)\reg.c $(COMMONH)
font.obj: $(COMMONDIR)\font.c $(COMMONH)
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)

# res file

$(RESFILE): $(RCFILE) $(SRCDIR)\tclock.manifest
	$(RC) $(RCOPT)$@ $(RCFILE)
