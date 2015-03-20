#include "reliefdb.h"

#ifndef skipdict_h
#define skipdict_h

typedef struct sde skipDictEntry;
typedef struct sdict skipDict;

struct sde
{
	char* key;
	cn* city;
	struct sde** next;
};

struct sdict
{
	skipDictEntry* head;
	int level;
};

float newrand();
int level();
skipDict* newSkipDict();
skipDictEntry* newSkipEntry(char* key, cn* city, int level);
void printSkipDict(skipDict* theSkipDict);
void addSkipEntry(skipDict* theSkipDict, char* key, cn* city);
skipDictEntry* skipDictSearch(skipDict* theSkipDict, char* queryKey);
void skipDictDelete(skipDict* theSkipDict, char* key);
void purgeSkipDict(skipDict* theSkipDict);

#endif
