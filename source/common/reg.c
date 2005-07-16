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
static const char *m_mykey = REGMYKEY;

/*------------------------------------------------
  get a string from TClock setting
--------------------------------------------------*/
int GetMyRegStr(const char *section, const char *entry,
	char *val, int cbData, const char *defval)
{
	char key[80];
	int r;

	if(g_bIniSetting)
	{
		if(section && *section)
			strcpy(key, section);
		else
			strcpy(key, "Main");
		r = GetPrivateProfileString(key, entry, defval, val, cbData, g_inifile);
	}
	else
	{
		strcpy(key, m_mykey);
		if(section && *section)
		{
			strcat(key, "\\");
			strcat(key, section);
		}
		r = GetRegStr(HKEY_CURRENT_USER, key, entry, val, cbData, defval);
	}

	return r;
}

/*------------------------------------------------
  get a wide-string from TClock setting
--------------------------------------------------*/
int GetMyRegStrW(const char *section, const char *entry,
	wchar_t *dst, int cbData, const char *defval)
{
	char *buf;
	int r;
	
	buf = malloc(cbData * sizeof(wchar_t));
	
	r = GetMyRegStr(section, entry, buf, cbData, defval);
	
	if(r > 0)
		r = MultiByteToWideChar(CP_ACP, 0, buf, -1, dst, cbData);
	else
		dst[0] = 0;
	
	free(buf);
	
	return r;
}

/*-------------------------------------------
  write a string to TClock setting
---------------------------------------------*/
BOOL SetMyRegStr(const char *section, const char *entry, const char *val)
{
	char key[80];
	BOOL r = FALSE;

	if(g_bIniSetting)
	{
		const char *p = val;
		BOOL b_chkflg = FALSE;
		
		if(section && *section)
			strcpy(key, section);
		else
			strcpy(key, "Main");
		
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
		HKEY hkey;
		
		strcpy(key, m_mykey);
		if(section && *section)
		{
			strcat(key, "\\");
			strcat(key, section);
		}
		
		if(RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
			&hkey, NULL) == ERROR_SUCCESS)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_SZ, (const BYTE*)val,
				strlen(val)+1) == ERROR_SUCCESS)
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
	LONG r;

	if(g_bIniSetting)
	{
		if(section && *section)
			strcpy(key, section);
		else
			strcpy(key, "Main");
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
	}
	else
	{
		strcpy(key, m_mykey);
		if(section && *section)
		{
			strcat(key, "\\");
			strcat(key, section);
		}
		r = GetRegLong(HKEY_CURRENT_USER, key, entry, defval);
	}

	return r;
}

/*-------------------------------------------
  write DWORD value to TClock setting
---------------------------------------------*/
BOOL SetMyRegLong(const char *section, const char *entry, DWORD val)
{
	char key[80];
	BOOL r = FALSE;
	
	if(g_bIniSetting)
	{
		char s[16];
		
		if(section && *section)
			strcpy(key, section);
		else
			strcpy(key, "Main");
		wsprintf(s, "%d", val);
		
		if(WritePrivateProfileString(key, entry, s, g_inifile))
			r = TRUE;
	}
	else
	{
		HKEY hkey;
		
		strcpy(key, m_mykey);
		if(section && *section)
		{
			strcat(key, "\\");
			strcat(key, section);
		}
		
		if(RegCreateKeyEx(HKEY_CURRENT_USER, key, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
			&hkey, NULL) == ERROR_SUCCESS)
		{
			if(RegSetValueEx(hkey, entry, 0, REG_DWORD,
				(const BYTE*)&val, 4) == ERROR_SUCCESS)
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
	char key[80];
	BOOL r = FALSE;
	
	if(g_bIniSetting)
	{
		if(section && *section)
			strcpy(key, section);
		else
			strcpy(key, "Main");
		
		if(WritePrivateProfileString(key, entry, NULL, g_inifile))
			r = TRUE;
	}
	else
	{
		HKEY hkey;
		
		strcpy(key, m_mykey);
		if(section && *section)
		{
			strcat(key, "\\");
			strcat(key, section);
		}
	
		if(RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, KEY_WRITE, &hkey)
			== ERROR_SUCCESS)
		{
			if(RegDeleteValue(hkey, entry) == ERROR_SUCCESS)
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
	char key[80];
	BOOL r = FALSE;
	
	if(g_bIniSetting)
	{
		if(section && *section)
			strcpy(key, section);
		else
			strcpy(key, "Main");
		
		if(WritePrivateProfileSection(key, NULL, g_inifile))
			r = TRUE;
	}
	else
	{
		strcpy(key, m_mykey);
		if(section && *section)
		{
			strcat(key, "\\");
			strcat(key, section);
		}
		
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
	DWORD size = cbData;
	BOOL b = FALSE;
	
	if(RegOpenKeyEx(rootkey, subkey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		if(RegQueryValueEx(hkey, entry, NULL, NULL,
			(LPBYTE)val, &size) == ERROR_SUCCESS)
		{
			if(size == 0) *val = 0;
			b = TRUE;
		}
		RegCloseKey(hkey);
	}
	if(!b)
		strcpy(val, defval);
	
	return strlen(val);
}

/*------------------------------------------------
  get DWORD value from registry
--------------------------------------------------*/
LONG GetRegLong(HKEY rootkey, const char *subkey, const char *entry,
	LONG defval)
{
	HKEY hkey;
	int r;
	DWORD size = 4;
	BOOL b = FALSE;
	
	if(RegOpenKeyEx(rootkey, subkey, 0, KEY_READ, &hkey) == ERROR_SUCCESS)
	{
		if(RegQueryValueEx(hkey, entry, NULL, NULL,
			(LPBYTE)&r, &size) == ERROR_SUCCESS)
		{
			if(size == 4) b = TRUE;
		}
		RegCloseKey(hkey);
	}
	return b ? r : defval;
}

