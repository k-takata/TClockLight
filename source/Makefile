#
# tclock.mak
#

# Visual C++
!IFDEF _NMAKE_VER

NODEFLIBOPT=NODEFAULTLIB=1

all:
	-mkdir out
	-mkdir work
	cd work
	$(MAKE) -f ..\dll\tcdll.mak SRCDIR=..\dll $(NODEFLIBOPT)
	$(MAKE) -f ..\exe\tclock.mak SRCDIR=..\exe $(NODEFLIBOPT)
	$(MAKE) -f ..\property\tcprop.mak SRCDIR=..\property $(NODEFLIBOPT)
	$(MAKE) -f ..\timer\tctimer.mak SRCDIR=..\timer $(NODEFLIBOPT)
	$(MAKE) -f ..\player\tcplayer.mak SRCDIR=..\player $(NODEFLIBOPT)
	$(MAKE) -f ..\sntp\tcsntp.mak SRCDIR=..\sntp $(NODEFLIBOPT)
	cd ..

# Borland C++ Compiler
!ELSE

NODEFLIBOPT=-D NODEFAULTLIB=1

all:
	-mkdir out
	-mkdir work
	cd work
	$(MAKE) -f ..\dll\tcdll.mak -D SRCDIR=..\dll $(NODEFLIBOPT)
	$(MAKE) -f ..\exe\tclock.mak -D SRCDIR=..\exe $(NODEFLIBOPT)
	$(MAKE) -f ..\property\tcprop.mak -D SRCDIR=..\property $(NODEFLIBOPT)
	$(MAKE) -f ..\timer\tctimer.mak -D SRCDIR=..\timer $(NODEFLIBOPT)
	$(MAKE) -f ..\player\tcplayer.mak -D SRCDIR=..\player $(NODEFLIBOPT)
	$(MAKE) -f ..\sntp\tcsntp.mak -D SRCDIR=..\sntp $(NODEFLIBOPT)
	cd ..

!ENDIF
