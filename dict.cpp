// dspell
// (c) Copyright 2003, Recursive Pizza
// Ported to Windows, 2009

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <map>
#include <string.h>
#include <string>
#include "dspell.h"
#include "dict.h"

#ifdef _WIN32
#include "windows.h"
#else
// Linux
static const char *main_dict = "/usr/share/dict/words";
#endif

#ifdef __BORLANDC__
typedef std::map<std::string, bool, std::less<std::string> > DICT;
#else
typedef std::map<std::string, bool> DICT;
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4996) // stdlib
#endif

static DICT dict;

inline void ToLower(char *s)
{
	char	*p;

	for (p = s; *p; p++)
	{
		*p = tolower(*p);
	}
}

static bool LoadDict(const char *file)
{
	FILE	*f;
	char	buf[MAX_WORD+1];

	if ((f = fopen(file, "rt")) == NULL)
	{
		fprintf(stderr, "Could not open %s\n", file);
		return false;
	}

	for (;;)
	{
		if (fgets(buf, sizeof(buf), f) == NULL) break;
		Chomp(buf);
		ToLower(buf);
		dict[buf] = true;
	}

	fclose(f);
	return true;
}

static bool IsExists(const char *file)
{
	struct stat	sb;

	return stat(file, &sb) == 0;
}

static void LoadDict()
{
	char 	buf[MAX_PATH+1];

#ifdef _WIN32
	// Windows
	LPSTR	p;

	buf[0] = '\0';
	GetModuleFileName(NULL, buf, sizeof(buf));
	if ((p = strrchr(buf, '\\')) == NULL)
	{
		p = &buf[lstrlen(buf)-1]; // Last char
	}
	lstrcpyn(p+1, "words.txt", sizeof(buf));

	if (IsExists(buf))
	{
		LoadDict(buf);
	}

#else
	// Unix
	LoadDict(main_dict);
	snprintf(buf, sizeof(buf), "%s/words", getenv("HOME"));

	if (IsExists(buf))
	{
		LoadDict(buf);
	}

	if (IsExists("words"))
	{
		LoadDict("words");
	}
#endif
}

inline bool IsNumberChar(const char c)
{
	return isdigit(c) || c == '.';
}

inline bool IsNumber(const char *s)
{
	const char	*p;

	for (p = s; *p; p++)
	{
		if (!IsNumberChar(*p)) return false;
	}

	return true;
}

bool InDict(const char *word)
{
	char	lower[MAX_WORD+1];

	if (dict.size() == 0)
	{
		LoadDict();
	}

	if (IsNumber(word)) return true;

#ifdef _MSC_VER
	lstrcpyn(lower, word, sizeof(lower));
#else
	strcpy(lower, word);
#endif

	ToLower(lower);
	return dict[lower];
}