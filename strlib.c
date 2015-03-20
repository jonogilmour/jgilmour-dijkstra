#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "strlib.h"

/*
 Counts the number of characters in a C string and returns the value.
 
 Note: Assumes str is a valid, null-terminated string.
 
 Returns 0 if the string pointer is NULL.
*/
int len(char* str)
{
	if(str == NULL) return 0;
	
	int x = 0;
	while(str[x] != '\n' && str[x] != '\0') {
		x++;
	}
	return x;
}

/*
 Simple O(n) deep copy of a C style string.
 
 Returns the new copied string.
 Returns NULL if the string pointer is NULL.
*/
char* cpystr(char* str)
{
	if(str == NULL) return NULL;
	
	int x;
	int ln = lengthof(str);
	char* retstr = (char*)calloc(ln+1, sizeof(char));
	
	for(x = 0; x < ln; x++) {
		retstr[x] = str[x];
	}
	
	return retstr;
}

/*
 Deletes the character given from a string.
 
 Note: Permanently alters the given string. 
 Use with caution as the string may end prematurely.
*/

void stripstr(char* str, char c)
{
	int x = 0;
	while(str[x] != '\0') {
		if(str[x] == c) {
			str[x] = '\0';
			break;
		}
		x++;
	}
}

/*
 Checks a valid C string for the characters given.
 
 If one character does not match, 0 is returned.
 If the string checks out, 1 is returned.
*/
int strIntegrityCheck(char* str, char* check)
{
	int x = 0;
	int y;
	int toggle;
	
	while(str[x] != '\0') {
		toggle = 1;
		y = 0;
		while(check[y] != '\0') {
			if(toupper(str[x]) == check[y]) toggle = 0;
			y++;
		}
		
		if(toggle) return 0;
		x++;
	}
	return 1;
}


/*
 Returns the length of a null terminated string
*/
int lengthof(char* string)
{
	int x = 0;
	while(string[x] != '\0' && string[x] != '\n') {
		x++;
	}
	return x;
}

/*
 Compares two strings by alphanumeric order
 
 Returns 1 if the key comes after the compare
 Returns 0 if the key is equal to the compare
 Returns -1 if the key comes before the compare
 */
int keycmp(char* theKey, char* toCompare)
{
	int x = 0;
	while((theKey[x] != '\0' && theKey[x] != '\n') &&
		  (toCompare[x] != '\0' && toCompare[x] != '\n')) {
		
		if (theKey[x] < toCompare[x]) {
			return -1;
		}
		if (theKey[x] > toCompare[x]) {
			return 1;
		}
		x++;
	}
	
	int klen = lengthof(theKey);
	int clen = lengthof(toCompare);
	
	if (klen > clen) {
		return 1; //key is longer than compare
	}
	if (klen < clen) {
		return -1; //key is shorter than compare
	}
	return 0; //key is the same as compare
}

/*
 Searches a string for the first occurrence of the character given.
 
 Returns 1 if the character was found.
 Returns 0 if it wasn't
*/
int strfind(char* str, char c)
{
	int x = 0;
	while(str[x] != '\0') {
		if(str[x] == c) return 1;
		x++;
	}
	return 0;
}

int countchar(char* str, char c)
{
	int x = 0;
	int count = 0;
	while(str[x] != '\0') {
		if(str[x] == c) count++;
		x++;
	}
	return count;
}


