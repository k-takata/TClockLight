/*-------------------------------------------------------------
  reg.c : read/write settings from/to ini file/registry
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

extern BOOL g_bIniSetting;
extern char g_inifile[];

// registry key
static char *m_mykey = REGMYKEY;

/*------------------------------------------------
  get a string from TClock setting
--------------------------------------------------*/
int GetMyRegStr(const char *section, const char *entry,
	char *val, int cbData, const char *defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, m_mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileString(key, entry, defval, val,
			cbData, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = cbData;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)val, &size) == 0)
			{
				if(size == 0) *val = 0;
				r = size;
				b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE)
		{
			strcpy(val, defval);
			r = strlen(defval);
		}
	}
	
	return r;
}

/*------------------------------------------------
  get a wide-string from TClock setting
--------------------------------------------------*/
int GetMyRegStrW(const char *section, const char *entry,
	wchar_t *dst, int cbData, const char *defval)
{
	char* buf;
	int r;
	
	buf = malloc(cbData+1);
	
	r = GetMyRegStr(section, entry, buf, cbData, defval);
	
	if(r > 0)
		r = MultiByteToWideChar(CP_ACP, 0, buf, -1, dst, cbData);
	else dst[0] = 0;
	
	free(buf);
	
	return r;
}

/*-------------------------------------------
  write a string to TClock setting
---------------------------------------------*/
BOOL SetMyRegStr(const char *section, const char *entry, const char *val)
{
	HKEY hkey;
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, m_mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		const char *p;
		BOOL b_chkflg = FALSE;
		
		r = FALSE;
		p = val;
		while(*p)
		{
			if(*p == '\"' || *p == '\'' || *p == ' ')
			{
				b_chkflg = TRUE; break;
			}
			p++;
		}
		
		if(b_chkflg)
		{
			char *buf = malloc(strlen(val) + 3);
			strcpy(buf, "\"");
			strcat(buf, val);
			strcat(buf, "\"");
			if(WritePrivateProfileString(key, entry, buf, g_inifile))
				r = TRUE;
			free(buf);
		}
		else
		{
			if(WritePrivateProfileString(key, entry, val, g_inifile))
				r = TRUE;
		}
	}
	else
	{
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_SZ,
				(CONST BYTE*)val, strlen(val)) == 0)
			{
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}

/*------------------------------------------------
  get DWORD value from TClock setting
--------------------------------------------------*/
LONG GetMyRegLong(const char *section, const char *entry, LONG defval)
{
	char key[80];
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	LONG r;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, m_mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
	}
	else
	{
		b = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			size = 4;
			if(RegQueryValueEx(hkey, entry, 0, &regtype,
				(LPBYTE)&r, &size) == 0)
			{
				if(size == 4) b = TRUE;
			}
			RegCloseKey(hkey);
		}
		if(b == FALSE) r = defval;
	}
	return r;
}

/*-------------------------------------------
  write DWORD value to TClock setting
---------------------------------------------*/
BOOL SetMyRegLong(const char *section, const char *entry, DWORD val)
{
	HKEY hkey;
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, m_mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		char s[20];
		wsprintf(s, "%d", val);
		r = FALSE;
		if(WritePrivateProfileString(key, entry, s, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegCreateKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_DWORD,
				(CONST BYTE*)&val, 4) == 0)
			{
				r = TRUE;
			}
			RegCloseKey(hkey);
		}
	}
	return r;
}

/*-------------------------------------------
  delete a name=value from TClock setting
---------------------------------------------*/
BOOL DelMyReg(const char *section, const char *entry)
{
	BOOL r;
	char key[80];
	HKEY hkey;
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, m_mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = FALSE;
		if(WritePrivateProfileString(key, entry, NULL, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegOpenKey(HKEY_CURRENT_USER, key, &hkey) == 0)
		{
			if(RegDeleteValue(hkey, entry) == 0)
				r = TRUE;
			RegCloseKey(hkey);
		}
	}
	return r;
}

/*-------------------------------------------
  delete a section from TClock setting
---------------------------------------------*/
BOOL DelMyRegKey(const char *section)
{
	BOOL r;
	char key[80];
	
	if(g_bIniSetting) key[0] = 0;
	else strcpy(key, m_mykey);
	
	if(section && *section)
	{
		if(!g_bIniSetting) strcat(key, "\\");
		strcat(key, section);
	}
	else
	{
		if(g_bIniSetting) strcpy(key, "Main");
	}
	
	if(g_bIniSetting)
	{
		r = FALSE;
		if(WritePrivateProfileSection(key, NULL, g_inifile))
			r = TRUE;
	}
	else
	{
		r = FALSE;
		if(RegDeleteKey(HKEY_CURRENT_USER, key) == 0)
			r = TRUE;
	}
	return r;
}

/*------------------------------------------------
  get a string from registry
--------------------------------------------------*/
int GetRegStr(HKEY rootkey, const char *subkey, const char *entry,
	char *val, int cbData, const char *defval)
{
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	
	b = FALSE;
	if(RegOpenKey(rootkey, subkey, &hkey) == 0)
	{
		size = cbData;
		if(RegQueryValueEx(hkey, entry, 0, &regtype,
			(LPBYTE)val, &size) == 0)
		{
			if(size == 0) *val = 0;
			b = TRUE;
		}
		RegCloseKey(hkey);
	}
	if(b == FALSE)
		strcpy(val, defval);
	
	return strlen(val);
}

/*------------------------------------------------
  get DWORD value from registry
--------------------------------------------------*/
LONG GetRegLong(HKEY rootkey, const char *subkey, const char* entry,
	LONG defval)
{
	HKEY hkey;
	DWORD regtype;
	DWORD size;
	BOOL b;
	int r;
	
	b = FALSE;
	if(RegOpenKey(rootkey, subkey, &hkey) == 0)
	{
		size = 4;
		if(RegQueryValueEx(hkey, entry, 0, &regtype,
			(LPBYTE)&r, &size) == 0)
		{
			if(size == 4) b = TRUE;
		}
		RegCloseKey(hkey);
	}
	if(b == FALSE) r = defval;
	return r;
}

