#
# tclock.mak
#

NODEFLIBOPT=NODEFAULTLIB=1

all:
	-mkdir out
	-mkdir work
	cd work
	$(MAKE) -f ..\dll\tcdll.mak SRCDIR=..\dll
	$(MAKE) -f ..\exe\tclock.mak SRCDIR=..\exe $(NODEFLIBOPT)
	$(MAKE) -f ..\property\tcprop.mak SRCDIR=..\property $(NODEFLIBOPT)
	$(MAKE) -f ..\timer\tctimer.mak SRCDIR=..\timer $(NODEFLIBOPT)
	$(MAKE) -f ..\player\tcplayer.mak SRCDIR=..\player $(NODEFLIBOPT)
	$(MAKE) -f ..\sntp\tcsntp.mak SRCDIR=..\sntp $(NODEFLIBOPT)
	cd ..

