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
static const char *main_dict1 = "/usr/share/dict/words";
static const char *main_dict2 = "/usr/share/words";
#endif

typedef std::unordered_map<std::string, bool> DICT;

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
	char	buf[MAX_WORD];

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
	char 	local_words[MAX_PATH] = "";

#ifdef _WIN32
	// Windows
	LPSTR	p;

	GetModuleFileName(NULL, local_words, sizeof(local_words));
	if ((p = strrchr(local_words, '\\')) == NULL) {
		p = &local_words[lstrlen(local_words) - 1]; // Last char
	}
	lstrcpyn(p + 1, "words.txt", sizeof(local_words));

	if (IsExists(local_words)) {
		LoadDict(local_words);
	}

#else
	// Unix
	if (IsExists(main_dict1)) {
		LoadDict(main_dict1);
	}
	else if (IsExists(main_dict2)) {
		LoadDict(main_dict2);
	}
	else {
		fprintf(stderr, "Could not load %s or %s\n", main_dict1, main_dict2);
	}

	snprintf(local_words, sizeof(local_words), "%s/words", getenv("HOME"));
	if (IsExists(local_words)) {
		fprintf(stderr, "Loading %s\n", local_words);
		LoadDict(local_words);
	}

	if (IsExists("words")) {
		fprintf(stderr, "Loading 'words' in current folder\n");
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
