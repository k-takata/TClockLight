# ------------------------------------------
# tcprop.mak
#-------------------------------------------

!IFNDEF SRCDIR
SRCDIR=.
!ENDIF

!IFNDEF COMMONDIR
COMMONDIR=..\common
!ENDIF

EXEFILE=..\out\tcprop.exe
RCFILE=$(SRCDIR)\tcprop.rc
RESFILE=tcprop.res
TDSFILE=..\out\tcprop.tds
TCPROPH=$(SRCDIR)\tcprop.h $(SRCDIR)\resource.h $(COMMONDIR)\common.h
COMMONH=$(COMMONDIR)\common.h

OBJS=pagecolor.obj pagesize.obj pageformat.obj pageformat2.obj pageanalog.obj\
	pagealarm.obj alarmday.obj pagecuckoo.obj\
	pagemouse.obj pagemouse2.obj pagetooltip.obj\
	pagestartbtn.obj selecticon.obj pagestartmenu.obj\
	pagetaskbar.obj pagemisc.obj\
	tclang.obj langcode.obj list.obj\
	combobox.obj autoformat.obj localeinfo.obj selectfile.obj \
	playfile.obj soundselect.obj alarmstruct.obj mousestruct.obj\
	utl.obj exec.obj reg.obj font.obj

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

COPT=/c /W3 /O2 /Oi /DNODEFAULTLIB /Fo
LOPT=/SUBSYSTEM:WINDOWS /NODEFAULTLIB /OPT:NOWIN98

$(EXEFILE): propmain.obj $(OBJS) nodeflib.obj $(RESFILE)
	$(LINK) $(LOPT) propmain.obj nodeflib.obj $(OBJS) $(RESFILE) $(LIBS) /OUT:$@

!ELSE

COPT=/c /W3 /O2 /Oi /Fo
LOPT=/SUBSYSTEM:WINDOWS /OPT:NOWIN98

$(EXEFILE): propmain.obj $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) propmain.obj $(OBJS) $(RESFILE) $(LIBS) /OUT:$@

!ENDIF

# Borland C++ Compiler
!ELSE

CC=bcc32
LINK=ilink32
RC=brcc32
RCOPT=-32 -fo

!IFDEF NODEFAULTLIB
COPT=-c -w -w-8057 -O2 -Oi -d -DNODEFAULTLIB -tW -o
LOPT=/aa /Tpe /c /C /Gn /x

$(EXEFILE): propmain.obj $(OBJS) nodeflib.obj bccexe.pat $(RESFILE)
	$(LINK) $(LOPT) propmain.obj $(OBJS) nodeflib.obj bccexe.pat,$@,,$(LIBS),,$(RESFILE)
	del $(TDSFILE)
	
bccexe.pat: $(COMMONDIR)\bccexe.nas
	nasmw -f obj -o $@ $(COMMONDIR)\bccexe.nas

!ELSE
COPT=-c -w -w-8057 -O2 -Oi -d -tW -o
LOPT=/aa /Tpe /c /C /Gn /x

$(EXEFILE): propmain.obj $(OBJS) $(RESFILE)
	$(LINK) $(LOPT) propmain.obj $(OBJS) c0w32.obj,$@,,$(LIBS) noeh32.lib cw32.lib,,$(RESFILE)
	del $(TDSFILE)

!ENDIF

!ENDIF

# obj files

propmain.obj: $(SRCDIR)\main.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\main.c
pagecolor.obj: $(SRCDIR)\pagecolor.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagecolor.c
pagesize.obj: $(SRCDIR)\pagesize.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pagesize.c
pageformat.obj: $(SRCDIR)\pageformat.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pageformat.c
pageformat2.obj: $(SRCDIR)\pageformat2.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pageformat2.c
pageanalog.obj: $(SRCDIR)\pageanalog.c $(TCPROPH)
	$(CC) $(COPT)$@ $(SRCDIR)\pageanalog.c
pagemouse.obj: $(SRCDIR)\pagemouse.c $(TCPROPH) $(COMMONDIR)\command.h
	$(CC) $(COPT)$@ $(SRCDIR)\pagemouse.c
pagemouse2.obj: $(SRCDIR)\pagemouse2.c $(TCPROPH)
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
pagemisc.obj: $(SRCDIR)\pagemisc.c $(TCPROPH)
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
list.obj: $(COMMONDIR)\list.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\list.c
alarmstruct.obj: $(COMMONDIR)\alarmstruct.c $(COMMONH)
	$(CC) $(COPT)$@ $(COMMONDIR)\alarmstruct.c
mousestruct.obj: $(COMMONDIR)\mousestruct.c $(COMMONH) $(COMMONDIR)\command.h
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

# res file

$(RESFILE): $(RCFILE)
	$(RC) $(RCOPT)$@ $(RCFILE)
