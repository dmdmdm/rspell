// rspell
// (c) Copyright 2003, Recursive Pizza

enum { MAX_WORD = 256 };
enum { MAX_PATH = 256 };
enum { MAX_LINE = 5 * 1024 };

void portable_strlcpy(char *dest, const char *src, const size_t size);

void Chomp(char *);
