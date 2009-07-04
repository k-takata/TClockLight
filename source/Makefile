#
# tclock.mak
#

WORKDIR=work
OUTDIR=out


# Visual C++
!IFDEF _NMAKE_VER

!IF "$(CPU)"=="AMD64" || DEFINED(AMD64)
WIN64=1
!ENDIF

!IFDEF WIN64

WORKDIR=work64
OUTDIR=out64
NODEFLIBOPT=NODEFAULTLIB=1
MAKEOPT=OUTDIR=..\$(OUTDIR) $(NODEFLIBOPT) WIN64=1 -nologo

!ELSE

NODEFLIBOPT=NODEFAULTLIB=1
MAKEOPT=OUTDIR=..\$(OUTDIR) $(NODEFLIBOPT) -nologo

!ENDIF

all:
	-if not exist $(OUTDIR) mkdir $(OUTDIR)
	-if not exist $(WORKDIR) mkdir $(WORKDIR)
	cd $(WORKDIR)
	$(MAKE) -f ..\dll\tcdll.mak SRCDIR=..\dll $(MAKEOPT)
	$(MAKE) -f ..\exe\tclock.mak SRCDIR=..\exe $(MAKEOPT)
	$(MAKE) -f ..\property\tcprop.mak SRCDIR=..\property $(MAKEOPT)
	$(MAKE) -f ..\timer\tctimer.mak SRCDIR=..\timer $(MAKEOPT)
	$(MAKE) -f ..\player\tcplayer.mak SRCDIR=..\player $(MAKEOPT)
	$(MAKE) -f ..\sntp\tcsntp.mak SRCDIR=..\sntp $(MAKEOPT)
	cd ..

# Borland C++ Compiler
!ELSE

NODEFLIBOPT=-D NODEFAULTLIB=1
MAKEOPT=-D OUTDIR=..\$(OUTDIR) $(NODEFLIBOPT)

all:
	-if not exist $(OUTDIR) mkdir $(OUTDIR)
	-if not exist $(WORKDIR) mkdir $(WORKDIR)
	cd $(WORKDIR)
	$(MAKE) -f ..\dll\tcdll.mak -D SRCDIR=..\dll $(MAKEOPT)
	$(MAKE) -f ..\exe\tclock.mak -D SRCDIR=..\exe $(MAKEOPT)
	$(MAKE) -f ..\property\tcprop.mak -D SRCDIR=..\property $(MAKEOPT)
	$(MAKE) -f ..\timer\tctimer.mak -D SRCDIR=..\timer $(MAKEOPT)
	$(MAKE) -f ..\player\tcplayer.mak -D SRCDIR=..\player $(MAKEOPT)
	$(MAKE) -f ..\sntp\tcsntp.mak -D SRCDIR=..\sntp $(MAKEOPT)
	cd ..

!ENDIF

clean:
	-del $(OUTDIR)\*.exe $(OUTDIR)\tcdll.tclock $(OUTDIR)\*.map
	-del /Q $(WORKDIR)\*
