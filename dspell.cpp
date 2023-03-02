// dspell
// (c) Copyright 2009, Recursive Pizza

#include <stdio.h>
#include <stdlib.h>	// For getenv()
#include <string.h>
#include <ctype.h>
#include <string>
#include <map>
#include <list>
#include "dict.h"
#include "dspell.h"

#ifdef _MSC_VER
#pragma warning(disable: 4996) // stdlib
#endif

static void Usage()
{
	// Note to self: Do NOT use AboutUtils here - to reduce dependances since this is on github
	fprintf(stderr, "Usage: dspell [options are ignored] < file\n");
	exit(1);
}

//-----------------------------------------------------------------------------
// Suggestions

inline bool ceq(const char *a, const char *b)
{
#ifdef __BORLANDC__
	return stricmp(a,b) == 0;
#elif _MSC_VER
	return _stricmp(a,b) == 0;
#else
	return strcasecmp(a,b) == 0;
#endif
}

typedef std::string SPELL_WORD;
typedef enum { W_NOT_IN_DICT, W_THE_WORD, W_SWAP, W_DELETE, W_INSERT } WHERE;

#ifdef __BORLANDC__
typedef std::map<SPELL_WORD, int, std::less<SPELL_WORD> > WORD_MAP;
#else
typedef std::map<SPELL_WORD, int> WORD_MAP;
#endif

typedef std::list<SPELL_WORD> WORD_LIST;

static void Mark(WORD_MAP &a, const char *word, WHERE where)
{
	if (a.find(word) == a.end())
	{
		a[word] = where;
	}
}

static void CheckSuggestions(WORD_MAP &a, WORD_LIST &b)
{
	const char		*word;

#ifdef __BORLANDC__
	b.erase(b.begin(), b.end());
#else
	b.clear();
#endif

	for (WORD_MAP::const_iterator it = a.begin(); it != a.end(); it++)
	{
		word = p->first.c_str();
		if (InDict(word))
		{
			b.push_back(word);
		}
	}
}

inline char *Shuffle(char *dest, const char *src)
{
	return (char *) memmove(dest, src, strlen(src) + 1);
}

static char insert_chars[] = "-abcdefghijklmnopqrstuvwxyz";

static void Suggestions(const char *word_in, WORD_LIST &b)
{
	WORD_MAP::iterator	p;
	WORD_MAP		a;
	int		i;
	int		len, len1;
	char		buf[MAX_WORD];
	char		c;
	int		j;
	const char	*word;
	int		type;

	// Add the word itself
	Mark(a, word_in, W_THE_WORD);

	// Swap adjacent
	for (p = a.begin(); p != a.end(); p++)
	{
		type = (*p).second;
		if (type == W_SWAP) continue; // Its from me

		word = (*p).first.c_str();
		len1 = (int)strlen(word) - 1;
		for (i = 0; i < len1; i++)
		{
			strcpy(buf, word);
			c = buf[i];
			buf[i] = buf[i + 1];
			buf[i + 1] = c;
			Mark(a, buf, W_SWAP);
		}
	}

	// Delete letters
	for (p = a.begin(); p != a.end(); p++)
	{
		type = (*p).second;
		if (type == W_DELETE) continue; // Its from me

		word = (*p).first.c_str();
		len = (int)strlen(word);
		for (i = 0; i < len; i++)
		{
			strcpy(buf, word);
			memmove(&buf[i], &buf[i + 1], len - i);
			Mark(a, buf, W_DELETE);
		}
	}

	// Insert
	for (p = a.begin(); p != a.end(); p++)
	{
		type = (*p).second;
		if (type == W_INSERT) continue; // Its from me

		word = (*p).first.c_str();
		len = (int)strlen(word);
		for (i = 0; i <= len; i++)
		{
			strcpy(buf, word);
			Shuffle(&buf[i + 1], &buf[i]);
			for (j = 0; insert_chars[j]; j++)
			{
				buf[i] = insert_chars[j];
				Mark(a, buf, W_INSERT);
			}
		}
	}

	// Don't want the original word as a suggestion any more
	a.erase(word_in);

	CheckSuggestions(a, b);
}

//--------------------------------------------------------------------------
// Reading input file

static bool IsWordChar(const char c, const int len)
{
	if (isalnum(c)) return true;
	if (c == '\'' && len > 1) return true;
	return false;
}

static bool GetWord(const char *&pRead, char *word, const size_t size, int &char_offset, int &word_offset)
{
	char	*pWord = word;
	int	n = 0;
	int	c;

	word[0] = '\0';

	// Skip leading non-word chars
	for (;;)
	{
		if ((c = *pRead++) == '\0') return false;
		char_offset++;

		if (IsWordChar(c, n))
		{
			pRead--;
			char_offset--;
			break;
		}
	}

	word_offset = char_offset;
	for (;;)
	{
		if ((c = *pRead++) == '\0') break;
		char_offset++;
		if (!IsWordChar(c, n)) break;
		*pWord++ = c;
		n++;
		if (n >= (int)size) break;
	}

	*pWord = '\0';
	return true;
}

static void SpellLine(const char *line)
{
	const char	*pRead = line;
	char		word[MAX_WORD];
	int		char_offset = 0;
	int		word_offset = 0;

	for (;;)
	{
		if (!GetWord(pRead, word, sizeof(word), char_offset, word_offset)) break;

		if (InDict(word))
		{
			printf("*\n");
		}
		else
		{
			WORD_LIST	b;

			Suggestions(word, b);
			if (b.size() == 0)
			{
				printf("# %s %d\n", word, word_offset);
			}
			else
			{
				bool			bNeedComma = false;

				printf("& %s %d %d: ", word, (int)b.size(), word_offset);
				for (WORD_LIST::const_iterator p = b.begin(); p != b.end(); p++)
				{
					if (bNeedComma) printf(", ");
					printf("%s", (*p).c_str());
					bNeedComma = true;
				}
				printf("\n");
			}
		}
	}
}

void Chomp(char *s)
{
	char	*p;

	if ((p = strchr(s, '\n')) != NULL)
	{
		*p = '\0';
	}
}

static void Spell(FILE *f)
{
	char	line[MAX_LINE];

	for (;;)
	{
		if (fgets(line, sizeof(line), f) == NULL) break;
		Chomp(line);
		SpellLine(line);
		printf("\n");
	}
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		if (ceq(argv[1], "-?")) {
			Usage();
			return 0;
		}
	}

	printf("@(#) International Ispell Version 3.1.20 (but really dspell .1)\n");
	Spell(stdin);
	exit(0);
}
