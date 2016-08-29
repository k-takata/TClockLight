/*-------------------------------------------------------------
  langcode.c : convert language identifier to language code
  (C) 1997-2003 Kazuto Sato
  Please read readme.txt about the license.
  
  Written by Kazubon, Nanashi-san
---------------------------------------------------------------*/

#include "common.h"

/* Statics */

static BOOL FindFileChinese(char *dst, int langid, const char* fname);
static BOOL GetLangFileName(char *dst, const char *fname, const char *code);

#define LANG_KYRGYZ     0x40
#define LANG_MONGOLIAN  0x50
#define LANG_GALICIAN   0x56
#define LANG_SYRIAC     0x5a
#define LANG_DIVEHI     0x65
#define LANG_INVARIANT  0x7f

/* Windows Primary Language Identifiers
   and ISO 639 Language Code
   http://www.oasis-open.org/cover/iso639a.html */

static struct {
	int primlang;
	char *code;
} m_langcode[] = 

{
	{ LANG_NEUTRAL, NULL },
	{ LANG_ARABIC,      "ar" },
	{ LANG_BULGARIAN,   "bg" },
	{ LANG_CATALAN,     "ca" },
	{ LANG_CHINESE,     "zh" },
	{ LANG_CZECH,       "cs" },
	{ LANG_DANISH,      "da" },
	{ LANG_GERMAN,      "de" },
	{ LANG_GREEK,       "el" },
	{ LANG_ENGLISH,     "en" },
	{ LANG_SPANISH,     "es" },
	{ LANG_FINNISH,     "fi" },
	{ LANG_FRENCH,      "fr" },
	{ LANG_HEBREW,      "he" },
	{ LANG_HUNGARIAN,   "hu" },
	{ LANG_ICELANDIC,   "is" },
	{ LANG_ITALIAN,     "it" },
	{ LANG_JAPANESE,    "ja" },
	{ LANG_KOREAN,      "ko" },
	{ LANG_DUTCH,       "nl" },
	{ LANG_NORWEGIAN,   "no" },
	{ LANG_POLISH,      "pl" },
	{ LANG_PORTUGUESE,  "pt" },
	{ LANG_ROMANIAN,    "ro" },
	{ LANG_RUSSIAN,     "ru" },
	{ LANG_CROATIAN,    "hr" },
	{ LANG_SERBIAN,     "sr" },
	{ LANG_SLOVAK,      "sk" },
	{ LANG_ALBANIAN,    "sq" },
	{ LANG_SWEDISH,     "sv" },
	{ LANG_THAI,        "th" },
	{ LANG_TURKISH,     "tr" },
	{ LANG_URDU,        "ur" },
	{ LANG_INDONESIAN,  "id" },
	{ LANG_UKRAINIAN,   "uk" },
	{ LANG_BELARUSIAN,  "be" },
	{ LANG_SLOVENIAN,   "sl" },
	{ LANG_ESTONIAN,    "et" },
	{ LANG_LATVIAN,     "lv" },
	{ LANG_LITHUANIAN,  "lt" },
	{ LANG_FARSI,       "fa" }, /* PERSIAN */
	{ LANG_VIETNAMESE,  "vi" },
	{ LANG_ARMENIAN,    "hy" },
	{ LANG_AZERI,       "az" }, /* AZERBAIJANI ? */
	{ LANG_BASQUE,      "eu" },
	{ LANG_MACEDONIAN,  "mk" },
	{ LANG_AFRIKAANS,   "af" },
	{ LANG_GEORGIAN,    "ka" },
	{ LANG_FAEROESE,    "fo" },
	{ LANG_HINDI,       "hi" },
	{ LANG_MALAY,       "ms" },
	{ LANG_KAZAK,       "kk" },
	{ LANG_KYRGYZ,      "ky" },
	{ LANG_SWAHILI,     "sw" },
	{ LANG_UZBEK,       "uz" },
	{ LANG_TATAR,       "tt" },
	{ LANG_BENGALI,     "bn" },
	{ LANG_PUNJABI,     "pa" },
	{ LANG_GUJARATI,    "gu" },
	{ LANG_ORIYA,       "or" },
	{ LANG_TAMIL,       "ta" },
	{ LANG_TELUGU,      "te" },
	{ LANG_KANNADA,     "kn" },
	{ LANG_MALAYALAM,   "ml" },
	{ LANG_ASSAMESE,    "as" },
	{ LANG_MARATHI,     "mr" },
	{ LANG_SANSKRIT,    "sa" },
	{ LANG_MONGOLIAN,   "mn" },
	{ LANG_GALICIAN,    "gl" },
	{ LANG_SINDHI,      "sd" },
	{ LANG_KASHMIRI,    "ks" },
	{ LANG_NEPALI,      "ne" },
	{ LANG_INVARIANT,   NULL },
	{ -1, NULL }
};

/* Windows SubLanguage Identifiers
   and ISO 3166 Country Code
   http://www.oasis-open.org/cover/country3166.html */

static struct {
	int primlang;
	int sublang;
	char *code;
} m_countrycode[] = 

{
	{ LANG_ARABIC, SUBLANG_ARABIC_SAUDI_ARABIA,        "sa" },
	{ LANG_ARABIC, SUBLANG_ARABIC_IRAQ,                "iq" },
	{ LANG_ARABIC, SUBLANG_ARABIC_EGYPT,               "eg" },
	{ LANG_ARABIC, SUBLANG_ARABIC_LIBYA,               "ly" },
	{ LANG_ARABIC, SUBLANG_ARABIC_ALGERIA,             "dz" },
	{ LANG_ARABIC, SUBLANG_ARABIC_MOROCCO,             "ma" },
	{ LANG_ARABIC, SUBLANG_ARABIC_TUNISIA,             "tn" },
	{ LANG_ARABIC, SUBLANG_ARABIC_OMAN,                "om" },
	{ LANG_ARABIC, SUBLANG_ARABIC_YEMEN,               "ye" },
	{ LANG_ARABIC, SUBLANG_ARABIC_SYRIA,               "sy" },
	{ LANG_ARABIC, SUBLANG_ARABIC_JORDAN,              "jo" },
	{ LANG_ARABIC, SUBLANG_ARABIC_LEBANON,             "lb" },
	{ LANG_ARABIC, SUBLANG_ARABIC_KUWAIT,              "kw" },
	{ LANG_ARABIC, SUBLANG_ARABIC_UAE,                 "ae" },
	{ LANG_ARABIC, SUBLANG_ARABIC_BAHRAIN,             "bh" },
	{ LANG_ARABIC, SUBLANG_ARABIC_QATAR,               "qa" },
	{ LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL,       "tw" },  /* Taiwan */
	{ LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED,        "cn" },  /* PRC */
	{ LANG_CHINESE, SUBLANG_CHINESE_HONGKONG,          "hk" },
	{ LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE,         "sg" },
	{ LANG_CHINESE, SUBLANG_CHINESE_MACAU,             "mo" },
	{ LANG_DUTCH,   SUBLANG_DUTCH,                     "nl" },
	{ LANG_DUTCH,   SUBLANG_DUTCH_BELGIAN,             "be" }, 
	{ LANG_ENGLISH, SUBLANG_ENGLISH_US,                "us" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_UK,                "gb" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_AUS,               "au" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_CAN,               "ca" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_NZ,                "nz" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_EIRE,              "ie" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_SOUTH_AFRICA,      "za" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_JAMAICA,           "jm" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_BELIZE,            "bz" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_TRINIDAD,          "tt" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_ZIMBABWE,          "zw" },
	{ LANG_ENGLISH, SUBLANG_ENGLISH_PHILIPPINES,       "ph" },
	{ LANG_FRENCH,  SUBLANG_FRENCH,                    "fr" },
	{ LANG_FRENCH,  SUBLANG_FRENCH_BELGIAN,            "be" },
	{ LANG_FRENCH,  SUBLANG_FRENCH_CANADIAN,           "ca" },
	{ LANG_FRENCH,  SUBLANG_FRENCH_SWISS,              "ch" },
	{ LANG_FRENCH,  SUBLANG_FRENCH_LUXEMBOURG,         "lu" },
	{ LANG_FRENCH,  SUBLANG_FRENCH_MONACO,             "mc" },
	{ LANG_GERMAN,  SUBLANG_GERMAN,                    "de" },
	{ LANG_GERMAN,  SUBLANG_GERMAN_SWISS,              "ch" },
	{ LANG_GERMAN,  SUBLANG_GERMAN_AUSTRIAN,           "at" },
	{ LANG_GERMAN,  SUBLANG_GERMAN_LUXEMBOURG,         "lu" },
	{ LANG_GERMAN,  SUBLANG_GERMAN_LIECHTENSTEIN,      "li" },
	{ LANG_ITALIAN, SUBLANG_ITALIAN,                   "it" },
	{ LANG_ITALIAN, SUBLANG_ITALIAN_SWISS,             "ch" },
	{ LANG_KOREAN,  SUBLANG_KOREAN,                    "kr" },
	{ LANG_LITHUANIAN, SUBLANG_LITHUANIAN,             "lt" },
	{ LANG_MALAY,   SUBLANG_MALAY_MALAYSIA,            "my" },
	{ LANG_MALAY,   SUBLANG_MALAY_BRUNEI_DARUSSALAM,   "bn" },
	{ LANG_PORTUGUESE, SUBLANG_PORTUGUESE_BRAZILIAN,   "br" },
	{ LANG_PORTUGUESE, SUBLANG_PORTUGUESE,             "pt" },
	{ LANG_SPANISH, SUBLANG_SPANISH,                   "es" },
	{ LANG_SPANISH, SUBLANG_SPANISH_MEXICAN,           "mx" },
	{ LANG_SPANISH, SUBLANG_SPANISH_GUATEMALA,         "gt" },
	{ LANG_SPANISH, SUBLANG_SPANISH_COSTA_RICA,        "cr" },
	{ LANG_SPANISH, SUBLANG_SPANISH_PANAMA,            "pa" },
	{ LANG_SPANISH, SUBLANG_SPANISH_VENEZUELA,         "ve" },
	{ LANG_SPANISH, SUBLANG_SPANISH_COLOMBIA,          "co" },
	{ LANG_SPANISH, SUBLANG_SPANISH_PERU,              "pe" },
	{ LANG_SPANISH, SUBLANG_SPANISH_ARGENTINA,         "ar" },
	{ LANG_SPANISH, SUBLANG_SPANISH_ECUADOR,           "ec" },
	{ LANG_SPANISH, SUBLANG_SPANISH_CHILE,             "cl" },
	{ LANG_SPANISH, SUBLANG_SPANISH_URUGUAY,           "uy" },
	{ LANG_SPANISH, SUBLANG_SPANISH_PARAGUAY,          "py" },
	{ LANG_SPANISH, SUBLANG_SPANISH_BOLIVIA,           "bo" },
	{ LANG_SPANISH, SUBLANG_SPANISH_EL_SALVADOR,       "sv" },
	{ LANG_SPANISH, SUBLANG_SPANISH_HONDURAS,          "hn" },
	{ LANG_SPANISH, SUBLANG_SPANISH_NICARAGUA,         "ni" },
	{ LANG_SPANISH, SUBLANG_SPANISH_PUERTO_RICO,       "pr" },
	{ LANG_SWEDISH, SUBLANG_SWEDISH,                   "se" },
	{ LANG_SWEDISH, SUBLANG_SWEDISH_FINLAND,           "fi" },
	{ LANG_URDU,    SUBLANG_URDU_PAKISTAN,             "pk" },
	{ LANG_URDU,    SUBLANG_URDU_INDIA,                "in" },
	{ -1, -1, NULL }
};

/* Externs */
extern char g_mydir[];

/*--------------------------------------------------------
  convert Windows language identifier
        to RFC3066 language code
----------------------------------------------------------*/
BOOL LangIDToLangCode(char *dst, int langid, BOOL bCountry)
{
	int prim, sub;
	int i;
	
	prim = PRIMARYLANGID((WORD)langid);
	sub  = SUBLANGID((WORD)langid);
	
	*dst = 0;
	for(i = 0; m_langcode[i].primlang >= 0; i++)
	{
		if(m_langcode[i].primlang == prim)
		{
			if(m_langcode[i].code)
				strcpy(dst, m_langcode[i].code);
			break;
		}
	}
	if(m_langcode[i].primlang < 0) return FALSE;
	
	if(!bCountry) return TRUE;
	
	for(i = 0; m_countrycode[i].sublang >= 0; i++)
	{
		if(m_countrycode[i].primlang == prim &&
			m_countrycode[i].sublang == sub)
		{
			if(m_countrycode[i].code)
			{
				strcat(dst, "-");
				strcat(dst, m_countrycode[i].code);
			}
			break;
		}
	}
	return TRUE;
}

/*--------------------------------------------------------
  find  tclang-en-us.txt or tclang-en.txt or tclang.txt
----------------------------------------------------------*/
BOOL FindFileWithLangCode(char *dst, int langid, const char* fname)
{
	char code[20];
	
	if(LangIDToLangCode(code, langid, TRUE))
	{
		if(GetLangFileName(dst, fname, code))
			return TRUE;
		
		if(PRIMARYLANGID((WORD)langid) == LANG_CHINESE)
		{
			if(FindFileChinese(dst, langid, fname)) return TRUE;
		}
		
		if(LangIDToLangCode(code, langid, FALSE))
		{
			if(GetLangFileName(dst, fname, code))
				return TRUE;
		}
	}
	
	if(GetLangFileName(dst, fname, "")) return TRUE;
	return FALSE;
}

BOOL FindFileChinese(char *dst, int langid, const char* fname)
{
	char code[20];
	int prim = PRIMARYLANGID((WORD)langid);
	int sub = SUBLANGID((WORD)langid);
	int trad[3] = { SUBLANG_CHINESE_TRADITIONAL, 
		  SUBLANG_CHINESE_HONGKONG,
		  SUBLANG_CHINESE_MACAU };
	int simp[2] = { SUBLANG_CHINESE_SIMPLIFIED,
		  SUBLANG_CHINESE_SINGAPORE };
	int i, j;
	
	for(i = 0; i < 3; i++)
	{
		if(sub == trad[i])
		{
			for(j = 0; j < 3; j++)
			{
				if(sub != trad[j])
				{
					int langid2 = MAKELANGID(prim, trad[j]);
					
					if(LangIDToLangCode(code, langid2, TRUE))
					{
						if(GetLangFileName(dst, fname, code))
							return TRUE;
					}
				}
			}
		}
	}
	for(i = 0; i < 2; i++)
	{
		if(sub == simp[i])
		{
			for(j = 0; j < 2; j++)
			{
				if(sub != simp[j])
				{
					int langid2 = MAKELANGID(prim, simp[j]);
					
					if(LangIDToLangCode(code, langid2, TRUE))
					{
						if(GetLangFileName(dst, fname, code))
							return TRUE;
					}
				}
			}
		}
	}
	
	return FALSE;
}


BOOL GetLangFileName(char *dst, const char *fname, const char *code)
{
	char title[MAX_PATH], ext[20];
	
	strcpy(title, fname);
	del_ext(ext, title);
	
	strcpy(dst, g_mydir);
	add_title(dst, title);
	
	if(code[0])
	{
		strcat(dst, "-"); strcat(dst, code);
	}
	if(ext[0])
	{
		strcat(dst, "."); strcat(dst, ext);
	}
	
	if(IsFile(dst)) return TRUE;
	
	strcpy(dst, g_mydir);
	add_title(dst, LANGDIR);
	add_title(dst, title);
	
	if(code[0])
	{
		strcat(dst, "-"); strcat(dst, code);
	}
	if(ext[0])
	{
		strcat(dst, "."); strcat(dst, ext);
	}
	
	if(IsFile(dst)) return TRUE;
	
	return FALSE;
}

/*--------------------------------------------------------
  check if 'lang' directory exists
----------------------------------------------------------*/
BOOL DoesLangDirExist(void)
{
	char buf[MAX_PATH];
	
	strcpy(buf, g_mydir);
	add_title(buf, LANGDIR);
	return IsDirectory(buf);
}
