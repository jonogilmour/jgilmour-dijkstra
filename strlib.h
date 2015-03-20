#ifndef strlib_h
#define strlib_h

int len(char* str);
char* cpystr(char* str);
void stripstr(char* str, char c);
int strIntegrityCheck(char* str, char* check);
int lengthof(char* string);
int keycmp(char* theKey, char* toCompare);
int strfind(char* str, char c);
int countchar(char* str, char c);

#endif
