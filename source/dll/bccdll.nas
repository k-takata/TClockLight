; bccdll.nas
;   BorlandC++ helper
;

SEGMENT MYDATA align=4 public use32

	global	_g_hhook
_g_hhook:
	dd	0
	global	_g_hwndTClockMain
_g_hwndTClockMain:
	dd	0
	global	_g_hwndClock
_g_hwndClock:
	dd	0
