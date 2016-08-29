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
static const char *GetKey(const char *section, char *buf);

/*------------------------------------------------
  get registory key from section name
--------------------------------------------------*/
const char *GetKey(const char *section, char *buf)
{
	if(g_bIniSetting)
	{
		if(section && *section)
			return section;
		else
			return "Main";
	}
	else
	{
		if(section && *section)
		{
			wsprintf(buf, "%s\\%s", m_mykey, section);
			return buf;
		}
		else
			return m_mykey;
	}
}

/*------------------------------------------------
  get a string from TClock setting
--------------------------------------------------*/
int GetMyRegStr(const char *section, const char *entry,
	char *val, int cbData, const char *defval)
{
	char buf[80];
	const char *key;
	int r;
	
	key = GetKey(section, buf);
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileString(key, entry, defval, val,
			cbData, g_inifile);
	}
	else
	{
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
	char* buf;
	int r;
	
	buf = malloc(cbData * sizeof(wchar_t));
	
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
	char buf[80];
	const char *key;
	
	key = GetKey(section, buf);
	
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
				(CONST BYTE*)val, (DWORD)strlen(val)) == 0)
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
	char buf[80];
	const char *key;
	LONG r;
	
	key = GetKey(section, buf);
	
	if(g_bIniSetting)
	{
		r = GetPrivateProfileInt(key, entry, defval, g_inifile);
	}
	else
	{
		r = GetRegLong(HKEY_CURRENT_USER, key, entry, defval);
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
	char buf[80];
	const char *key;
	
	key = GetKey(section, buf);
	
	if(g_bIniSetting)
	{
		char s[20];
		wsprintf(s, "%d", val);
		r = WritePrivateProfileString(key, entry, s, g_inifile);
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
	char buf[80];
	const char *key;
	HKEY hkey;
	
	key = GetKey(section, buf);
	
	if(g_bIniSetting)
	{
		r = WritePrivateProfileString(key, entry, NULL, g_inifile);
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
	char buf[80];
	const char *key;
	
	key = GetKey(section, buf);
	
	if(g_bIniSetting)
	{
		r = WritePrivateProfileSection(key, NULL, g_inifile);
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
	
	return (int)strlen(val);
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

