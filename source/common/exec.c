/*-------------------------------------------------------------
  exec.c : executing a file
  (C) 1997-2004 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

#define PATH_REL 0
#define PATH_ABS 1
#define PATH_URL 2

/* Statics */
static int GetPathType(const char *src);
static BOOL GetRealFileName(char *dst, const char *src);

extern HINSTANCE g_hInst;
extern char g_mydir[];

/*------------------------------------------------
  relative path -> absolute path
--------------------------------------------------*/
void RelToAbs(char *dst, const char *src)
{
	if(GetPathType(src) == PATH_REL)
	{
		strcpy(dst, g_mydir);
		add_title(dst, src);
	}
	else
		strcpy(dst, src);
}

/*--------------------------------------------------------
  Retreive a file name and option from a command string
----------------------------------------------------------*/
void GetFileAndOption(const char* command, char* fname, char* option)
{
	const char *p;
	char *dp;
	const char *pspace, *pfound;
	char tempfname[MAX_PATH], foundfname[MAX_PATH];
	BOOL bFound = FALSE;
	
	p = command;
	dp = tempfname;
	pspace = pfound = NULL;
	
	while(1)
	{
		if(*p == ' ' || *p == 0)
		{
			if(!pspace) pspace = p;
			
			*dp = 0;
			
			if(GetRealFileName(foundfname, tempfname))
			{
				strcpy(fname, foundfname);
				bFound = TRUE; pfound = p;
			}
			
			if(*p == 0) break;
		}
		*dp++ = *p++;
	}
	
	if(bFound)
	{
		p = pfound;
	}
	else
	{
		p = command;
		dp = fname;
		while(p != pspace) *dp++ = *p++;
		*dp = 0;
	}
	
	while(*p == ' ') p++;
	while(*p) *option++ = *p++;
	*option = 0;
}

/*------------------------------------------------
  Open a file
--------------------------------------------------*/
BOOL ExecFile(HWND hwnd, const char* command)
{
	char fname[MAX_PATH], path[MAX_PATH];
	char *option;
	SHELLEXECUTEINFO sei;
	
	if(*command == 0) return FALSE;
	
	option = malloc(strlen(command));
	if(option == NULL) return FALSE;
	
	memset(&sei,0,sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.nShow = SW_SHOW;
	
	if(GetPathType(command) != PATH_URL)
	{
		GetFileAndOption(command, fname, option);
		
		if(GetPathType(fname) == PATH_ABS)
		{
			strcpy(path, fname);
			del_title(path);
		}
		else strcpy(path, g_mydir);
		
		/*
		WriteDebug(fname);
		WriteDebug(path);
		WriteDebug(option);
		*/
		
		sei.lpFile = fname;
		sei.lpDirectory = path;
		sei.lpParameters = option[0] ? option : NULL;
	}
	else
	{
		sei.lpFile = command;
		sei.lpDirectory = g_mydir;
		sei.lpParameters = NULL;
	}
	
	ShellExecuteEx(&sei);
	
	free(option);
	
	return ((int)sei.hInstApp > 32);
}

/*------------------------------------------------
  relative path / absolute path / URL
--------------------------------------------------*/
int GetPathType(const char *src)
{
	const char *p = src;
	
	if(*p == '\\' && *(p + 1) == '\\') return PATH_ABS;
	else if((('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z')) &&
		*(p + 1) == ':') return PATH_ABS;
	else
	{
		while(*p &&
			(('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z')))
		{
			p++;
			if(*p == ':') return PATH_URL;
		}
	}
	return PATH_REL;
}

/*------------------------------------------------
  find a file and add path
--------------------------------------------------*/
BOOL GetRealFileName(char *dst, const char *src)
{
	char fname[MAX_PATH];
	
	RelToAbs(fname, src);
	
	if(IsFile(fname))
	{
		strcpy(dst, fname);
		return TRUE;
	}
	else
		return FALSE;
}
