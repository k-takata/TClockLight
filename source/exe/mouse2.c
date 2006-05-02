/*-------------------------------------------------------------
  mouse2.c : file drop, and etc.
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "tclock.h"

/* Globals */
void OnDropFiles(HWND hwnd, HDROP hdrop);

/* Statics */
static const char *m_section = "Mouse";

/*------------------------------------------------
   when files dropped to the clock
--------------------------------------------------*/
void OnDropFiles(HWND hwnd, HDROP hdrop)
{
	char fname[MAX_PATH], app[MAX_PATH];
	char *buf, *p;
	int nType; // 1:Recycle Bin 2:Open With... 3:Copy to 4:Move to
	int i, num;
	
	nType = GetMyRegLong(m_section, "DropFiles", 0);
	if(nType < 1 || 4 < nType) return;
	if(nType != 1) // Open With, Copy to, Move to
	{
		GetMyRegStr(m_section, "DropFilesApp", app, MAX_PATH, "");
		if(app[0] == 0) return;
	}
	
	num = DragQueryFile(hdrop, (UINT)-1, NULL, 0);
	if(num <= 0) return;
	buf = malloc(num*(MAX_PATH+3));
	if(buf == NULL) return;
	
	p = buf;
	for(i = 0; i < num; i++)
	{
		DragQueryFile(hdrop, i, fname, MAX_PATH);
		
		// Open With...
		if(nType == 2)
		{
			strcpy(p, "\"");
			strcat(p, fname);
			strcat(p, "\"");
			p += strlen(p);
			if(num > 1 && i < num - 1) *p++ = ' ';
		}
		// Recycle Bin, Copy to, Move to
		else
		{
			strcpy(p, fname); p += strlen(p) + 1;
		}
	}
	*p = 0;
	DragFinish(hdrop);
	
	// Open With...
	if(nType == 2)
	{
		char appabs[MAX_PATH], path[MAX_PATH];
		SHELLEXECUTEINFO sei;
		
		RelToAbs(appabs, app);
		
		strcpy(path, appabs);
		del_title(path);
		
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.lpFile = appabs;
		sei.lpDirectory = path;
		sei.lpParameters = buf;
		sei.nShow = SW_SHOW;
		ShellExecuteEx(&sei);
	}
	// Recycle Bin, Copy to, Move to
	else
	{
		SHFILEOPSTRUCT shfos;
		
		memset(&shfos, 0, sizeof(SHFILEOPSTRUCT));
		shfos.hwnd = NULL;
		if(nType == 1) shfos.wFunc = FO_DELETE;
		else if(nType == 3) shfos.wFunc = FO_COPY;
		else if(nType == 4) shfos.wFunc = FO_MOVE;
		shfos.pFrom = buf;
		if(nType == 3 || nType == 4) shfos.pTo = app;
		shfos.fFlags = FOF_ALLOWUNDO|FOF_NOCONFIRMATION;
		SHFileOperation(&shfos);
	}
	
	free(buf);
}

