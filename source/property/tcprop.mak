# ------------------------------------------
# tcprop.mak
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

EXEFILE=$(OUTDIR)\tcprop.exe
RCFILE=$(SRCDIR)\tcprop.rc
RESFILE=tcprop.res
TDSFILE=$(OUTDIR)\tcprop.tds
TCPROPH=$(SRCDIR)\tcprop.h $(SRCDIR)\resource.h $(COMMONDIR)\common.h
COMMONH=$(COMMONDIR)\common.h

OBJS=pagecolor.obj pagesize.obj pageformat.obj pageformat2.obj\
	pageanalog.obj pagealarm.obj alarmday.obj pagecuckoo.obj\
	pagemouse.obj pagemouse2.obj pagetooltip.obj\
	pagestartbtn.obj selecticon.obj pagestartmenu.obj\
	pagetaskbar.obj pagemisc.obj\
	tclang.obj langcode.obj list.obj\
	combobox.obj autoformat.obj localeinfo.obj selectfile.obj \
	playfile.obj soundselect.obj alarmstruct.obj mousestruct.obj\
	utl.obj exec.obj reg.obj font.obj

LIBS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib\
	shell32.lib winmm.lib comctl32.lib dwmapi.lib

all: $(EXEFILE)

CC=cl
LINK=link
RC=rc
RCOPT=/fo 

!IFDEF NODEFAULTLIB

COPT=/c /GS- /W3 /O2 /Oi /DNODEFAULTLIB /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo /MAP
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98
!ENDIF
!IF $(MSVC_MAJOR) >= 14
LIBS=$(LIBS) libvcruntime.lib
!ENDIF

$(EXEFILE): propmain.obj $(OBJS) nodeflib.obj $(RESFILE)
	$(LINK) $(LOPT) propmain.obj nodeflib.obj $(OBJS) $(RESFILE) $(LIBS) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /D_CRT_SECURE_NO_WARNINGS /nologo /Fo
LOPT=/SUBSYSTEM:WINDOWS /merge:.rdata=.text /nologo
!IFNDEF WIN64
#LOPT=$(LOPT) /OPT:NOWIN98
!ENDIF

$(EXEFILE): propmain.obj $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) propmain.obj $(OBJS) $(RESFILE) $(LIBS) /OUT:$@

!ENDIF

{$(COMMONDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<
{$(SRCDIR)\}.c{}.obj::
	$(CC) $(COPT).\ $<


# obj files

propmain.obj: $(SRCDIR)\main.c $(TCPROPH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
pagecolor.obj: $(SRCDIR)\pagecolor.c $(TCPROPH)
pagesize.obj: $(SRCDIR)\pagesize.c $(TCPROPH)
pageformat.obj: $(SRCDIR)\pageformat.c $(TCPROPH) ..\config.h
pageformat2.obj: $(SRCDIR)\pageformat2.c $(TCPROPH)
pageanalog.obj: $(SRCDIR)\pageanalog.c $(TCPROPH)
pagemouse.obj: $(SRCDIR)\pagemouse.c $(TCPROPH) $(COMMONDIR)\command.h ..\config.h
pagemouse2.obj: $(SRCDIR)\pagemouse2.c $(TCPROPH) ..\config.h
pagealarm.obj: $(SRCDIR)\pagealarm.c $(TCPROPH)
alarmday.obj: $(SRCDIR)\alarmday.c $(TCPROPH)
pagecuckoo.obj: $(SRCDIR)\pagecuckoo.c $(TCPROPH)
pagetooltip.obj: $(SRCDIR)\pagetooltip.c $(TCPROPH)
pagesntp.obj: $(SRCDIR)\pagesntp.c $(TCPROPH)
pagestartbtn.obj: $(SRCDIR)\pagestartbtn.c $(TCPROPH)
selecticon.obj: $(SRCDIR)\selecticon.c $(TCPROPH)
pagestartmenu.obj: $(SRCDIR)\pagestartmenu.c $(TCPROPH)
pagetaskbar.obj: $(SRCDIR)\pagetaskbar.c $(TCPROPH)
pagemisc.obj: $(SRCDIR)\pagemisc.c $(TCPROPH) ..\config.h

# common obj files

tclang.obj: $(COMMONDIR)\tclang.c $(COMMONH)
langcode.obj: $(COMMONDIR)\langcode.c $(COMMONH)
combobox.obj: $(COMMONDIR)\combobox.c $(COMMONH)
autoformat.obj: $(COMMONDIR)\autoformat.c $(COMMONH)
localeinfo.obj: $(COMMONDIR)\localeinfo.c $(COMMONH)
selectfile.obj: $(COMMONDIR)\selectfile.c $(COMMONH)
playfile.obj: $(COMMONDIR)\playfile.c $(COMMONH)
soundselect.obj: $(COMMONDIR)\soundselect.c $(COMMONH)
list.obj: $(COMMONDIR)\list.c $(COMMONH)
alarmstruct.obj: $(COMMONDIR)\alarmstruct.c $(COMMONH)
mousestruct.obj: $(COMMONDIR)\mousestruct.c $(COMMONH) $(COMMONDIR)\command.h ..\config.h
utl.obj: $(COMMONDIR)\utl.c $(COMMONH)
exec.obj: $(COMMONDIR)\exec.c $(COMMONH)
reg.obj: $(COMMONDIR)\reg.c $(COMMONH)
font.obj: $(COMMONDIR)\font.c $(COMMONH)
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)

# res file

$(RESFILE): $(RCFILE) ..\config.h $(SRCDIR)\tclock.manifest
	$(RC) $(RCOPT)$@ $(RCFILE)
