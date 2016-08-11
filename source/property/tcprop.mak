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
	pagealarm.obj alarmday.obj pagecuckoo.obj\
	pagemouse.obj pagemouse2.obj pagetooltip.obj\
	pagestartbtn.obj selecticon.obj pagestartmenu.obj\
	pagetaskbar.obj pagemisc.obj\
	tclang.obj langcode.obj\
	combobox.obj autoformat.obj localeinfo.obj selectfile.obj \
	playfile.obj soundselect.obj alarmstruct.obj mousestruct.obj\
	utl.obj exec.obj reg.obj font.obj\
	desktop.obj

LIBS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib\
	shell32.lib winmm.lib comctl32.lib

all: $(EXEFILE)

# Visual C++
!IFDEF _NMAKE_VER

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

# Borland C++ Compiler
!ELSE

CC=bcc32
LINK=ilink32
RC=brc32
RCOPT=-r -32 -fo

!IFDEF NODEFAULTLIB
COPT=-c -w -w-8057 -O2 -Oi -d -DNODEFAULTLIB -tW -o
LOPT=/c /C /Gn /x

$(EXEFILE): propmain.obj $(OBJS) nodeflib.obj bccexe.pat $(RESFILE)
	$(LINK) $(LOPT) /Tpe /aa propmain.obj $(OBJS) nodeflib.obj bccexe.pat,$@,,$(LIBS),,$(RESFILE)
	del $(TDSFILE)
	
bccexe.pat: $(COMMONDIR)\bccexe.nas
	nasmw -f obj -o $@ $(COMMONDIR)\bccexe.nas

!ELSE
COPT=-c -w -w-8057 -O2 -Oi -d -tW -o
LOPT=/c /C /Gn /x

$(EXEFILE): propmain.obj $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) /Tpe /aa propmain.obj $(OBJS) c0w32.obj,$@,,$(LIBS) cw32.lib,,$(RESFILE)
	del $(TDSFILE)

!ENDIF

!ENDIF

# obj files

propmain.obj: $(SRCDIR)\main.c $(TCPROPH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
pagecolor.obj: $(SRCDIR)\pagecolor.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagecolor.c
pagesize.obj: $(SRCDIR)\pagesize.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagesize.c
pageformat.obj: $(SRCDIR)\pageformat.c $(TCPROPH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\pageformat.c
pageformat2.obj: $(SRCDIR)\pageformat2.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pageformat2.c
pagemouse.obj: $(SRCDIR)\pagemouse.c $(TCPROPH) $(COMMONDIR)\command.h ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\pagemouse.c
pagemouse2.obj: $(SRCDIR)\pagemouse2.c $(TCPROPH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\pagemouse2.c
pagealarm.obj: $(SRCDIR)\pagealarm.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagealarm.c
alarmday.obj: $(SRCDIR)\alarmday.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\alarmday.c
pagecuckoo.obj: $(SRCDIR)\pagecuckoo.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagecuckoo.c
pagetooltip.obj: $(SRCDIR)\pagetooltip.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagetooltip.c
pagesntp.obj: $(SRCDIR)\pagesntp.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagesntp.c
pagestartbtn.obj: $(SRCDIR)\pagestartbtn.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagestartbtn.c
selecticon.obj: $(SRCDIR)\selecticon.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\selecticon.c
pagestartmenu.obj: $(SRCDIR)\pagestartmenu.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagestartmenu.c
pagetaskbar.obj: $(SRCDIR)\pagetaskbar.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagetaskbar.c
pagemisc.obj: $(SRCDIR)\pagemisc.c $(TCPROPH) ..\config.h
	$(CC) $(COPT)$@ $(SRCDIR)\pagemisc.c

# common obj files

tclang.obj: $(COMMONDIR)\tclang.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\tclang.c
langcode.obj: $(COMMONDIR)\langcode.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\langcode.c
combobox.obj: $(COMMONDIR)\combobox.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\combobox.c
autoformat.obj: $(COMMONDIR)\autoformat.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\autoformat.c
localeinfo.obj: $(COMMONDIR)\localeinfo.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\localeinfo.c
selectfile.obj: $(COMMONDIR)\selectfile.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\selectfile.c
playfile.obj: $(COMMONDIR)\playfile.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\playfile.c
soundselect.obj: $(COMMONDIR)\soundselect.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\soundselect.c
alarmstruct.obj: $(COMMONDIR)\alarmstruct.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\alarmstruct.c
mousestruct.obj: $(COMMONDIR)\mousestruct.c $(COMMONH) $(COMMONDIR)\command.h ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\mousestruct.c
utl.obj: $(COMMONDIR)\utl.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\utl.c
exec.obj: $(COMMONDIR)\exec.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\exec.c
reg.obj: $(COMMONDIR)\reg.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\reg.c
font.obj: $(COMMONDIR)\font.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\font.c
nodeflib.obj: $(COMMONDIR)\nodeflib.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\nodeflib.c
desktop.obj: $(COMMONDIR)\desktop.c $(COMMONH) ..\config.h
	$(CC) $(COPT)$@ $(COMMONDIR)\desktop.c

# res file

$(RESFILE): $(RCFILE) ..\config.h $(SRCDIR)\tclock.manifest
	$(RC) $(RCOPT)$@ $(RCFILE)
