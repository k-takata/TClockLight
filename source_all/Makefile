#
# tclock.mak
#

WORKDIR=work
OUTDIR=out


# Check target
!IF "$(CPU)"=="AMD64" || "$(TARGET_CPU)"=="x64" || "$(PLATFORM)"=="x64" || "$(PLATFORM)"=="X64" || DEFINED(AMD64)
WIN64=1
!ENDIF

# Check VC++ version
!if [echo MSVCVER=_MSC_VER> msvcver.c && $(CC) /EP msvcver.c > msvcver.~ 2> nul]
!error Cannot run Visual C++ to determine its version. Make sure cl.exe is in your PATH.
!else
!include msvcver.~
!if [del msvcver.c msvcver.~]
!endif
!endif

!if $(MSVCVER) < 1900
MSVC_MAJOR = ($(MSVCVER) / 100 - 6)
!else
MSVC_MAJOR = ($(MSVCVER) / 100 - 5)
!endif

# Calculate MSVC_MAJOR. (Convert the expression to the actual value.)
!if [(set /a MSVC_MAJOR="$(MSVC_MAJOR)" > nul) && set MSVC_MAJOR > msvcmajor.~] == 0
!include msvcmajor.~
!if [del msvcmajor.~]
!endif
!endif


!IFDEF WIN64

WORKDIR=work64
OUTDIR=out64
#NODEFLIBOPT=NODEFAULTLIB=1
MAKEOPT=OUTDIR=..\$(OUTDIR) $(NODEFLIBOPT) MSVC_MAJOR=$(MSVC_MAJOR) WIN64=1 -nologo

!ELSE

#NODEFLIBOPT=NODEFAULTLIB=1
MAKEOPT=OUTDIR=..\$(OUTDIR) $(NODEFLIBOPT) MSVC_MAJOR=$(MSVC_MAJOR) -nologo

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

clean:
	-del $(OUTDIR)\*.exe $(OUTDIR)\tcdll.tclock $(OUTDIR)\*.map
	-del /Q $(WORKDIR)\*
