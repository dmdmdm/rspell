// rspell
// (c) Copyright 2003, Recursive Pizza
// Ported to Windows, 2009

#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unordered_map>
#include <string.h>
#include <string>
#include "rspell.h"
#include "dict.h"

#ifdef _WIN32
#include "windows.h"
#else
// Linux
static const char *s_main_dict1 = "/usr/share/dict/words";
static const char *s_main_dict2 = "/usr/dict/words";
#endif

typedef std::unordered_map<std::string, bool> DICT;

#ifdef _MSC_VER
#pragma warning(disable: 4996) // stdlib
#endif

static DICT s_dict;

inline void ToLower(char *s)
{
	char	*p;

	for (p = s; *p; p++)
	{
		*p = tolower(*p);
	}
}

static char *MyStrError() {
#ifdef _WIN32
	return _strerror(NULL);
#else
	return strerror(errno);
#endif
}

static bool LoadDict(const char *file)
{
	FILE	*f;
	char	buf[MAX_WORD];

	fprintf(stderr, "Loading %s\n", file);

	if ((f = fopen(file, "rt")) == NULL)
	{
		fprintf(stderr, "Could not open %s because %s\n", file, MyStrError());
		return false;
	}

	for (;;)
	{
		if (fgets(buf, sizeof(buf), f) == NULL) break;
		Chomp(buf);
		if (buf[0] == '#') continue;
		ToLower(buf);
		s_dict[buf] = true;
	}

	fclose(f);
	return true;
}

static bool IsExists(const char *file)
{
#ifdef _WIN32
#define STAT64 _stat64
#else
#define STAT64 stat64
#endif
	struct STAT64	sb;
	return STAT64(file, &sb) == 0;
}

static void LoadDict()
{
	char 	local_words[MAX_PATH] = "";

#ifdef _WIN32
	// Windows
	LPSTR	p;

	GetModuleFileName(NULL, local_words, sizeof(buf));
	if ((p = strrchr(local_words, '\\')) == NULL)
	{
		p = &buf[lstrlen(load_words) - 1]; // Last char
	}
	lstrcpyn(p + 1, "words.txt", sizeof(buf));

	if (IsExists(local_words))
	{
		LoadDict(local_words);
	}
	else {
		fprintf(stderr, "Dictionary %s is missingn", local_words);
		exit(1);
	}
#else
	// Unix
	if (IsExists(s_main_dict1)) {
		LoadDict(s_main_dict1);
	}
	else if (IsExists(s_main_dict2)) {
		LoadDict(s_main_dict2);
	}
	else {
		fprintf(stderr, "Could not load %s or %s\n", s_main_dict1, s_main_dict2);
	}

	snprintf(local_words, sizeof(local_words), "%s/words", getenv("HOME"));
	if (IsExists(local_words)) {
		LoadDict(local_words);
	}

	if (IsExists("words")) {
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
	char	lower[MAX_WORD];

	if (s_dict.size() == 0)
	{
		LoadDict();
	}

	if (IsNumber(word)) return true;

	portable_strlcpy(lower, word, sizeof(lower));
	ToLower(lower);
	return s_dict[lower];
}
