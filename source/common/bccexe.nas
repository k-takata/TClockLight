; bccexe.nas
;   BorlandC++ helper
;

SEGMENT TEXT align=4 public use32 class=code

	extern	WinMainCRTStartup
..start:
	jmp	WinMainCRTStartup

