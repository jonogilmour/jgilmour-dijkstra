#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "skipdict.h"
#include "strlib.h"
#include "reliefdb.h"

#define PVAL 0.05
#define MAX_LEVEL 10

/*
 Produces a new floating point pseudorandom number.
*/
float newrand()
{
	return ((float)rand()/RAND_MAX);
}

/*
 Returns a new, random level.
*/
int level()
{
	static int firstrun = 1;
	int level = 0;
	
	if(firstrun) {
		srand((unsigned int)clock());
		firstrun = 0;
	}
	
	while(newrand() < PVAL && level < MAX_LEVEL) level++;
	
	return level;
}

/*
 Returns a new, empty skip list dictionary.
*/
skipDict* newSkipDict()
{
	skipDict* theSkipDict = (skipDict*)malloc(sizeof(skipDict));
	theSkipDict->level = 0;
	theSkipDict->head = newSkipEntry("NULL", NULL, MAX_LEVEL);
	return theSkipDict;
}

/*
 Returns a new skip list node with the key and value filled in.
*/
skipDictEntry* newSkipEntry(char* key, cn* city, int level)
{
	skipDictEntry* theEntry = (skipDictEntry*)malloc(sizeof(skipDictEntry));
	theEntry->next = (skipDictEntry**)calloc(level + 1, sizeof(skipDictEntry*)); // Set all pointers to NULL.
	theEntry->key = cpystr(key);
	theEntry->city = city;
	return theEntry;
}

/*
 Adds a new dictionary entry into the dictionary provided.
 Returns the number of key comparisons made.
 Takes a pointer "added" that is set if the entry is successfully added.
*/
void addSkipEntry(skipDict* theSkipDict, char* key, cn* city)
{
	int x;
	skipDictEntry* curNode = theSkipDict->head;
	skipDictEntry** nodesToUpdate = (skipDictEntry**)calloc(sizeof(skipDictEntry*),MAX_LEVEL + 1);

	for(x = theSkipDict->level; x >= 0; x--) {
		
		/*
		 Search for all the nodes that will eventually be pointing to the new node
		 and add them to list of nodes to update.
		*/
		while(curNode->next[x] != NULL && keycmp(curNode->next[x]->key, key) < 0) {
			curNode = curNode->next[x];
		}
		nodesToUpdate[x] = curNode;
	}
	
	// Travel to the very next node along. This is where our new node will go.
	curNode = curNode->next[0];
		
	// If the new place for the node is either empty or the existing node is not a duplicate, add the node.
	if(curNode == NULL || keycmp(curNode->key, key)) { 
		int newlevel = level(); // Get a new level for this node
		
		/*
		 If the height of the new node will be greater than the height of the header, then we
		 have to set the header node to be updated so it points to this new node. 
		 Also, we need to update the level of the skip list
		*/
        if(newlevel > theSkipDict->level) { 
    		for(x = theSkipDict->level + 1; x <= newlevel; x++) {
        		nodesToUpdate[x] = theSkipDict->head;
    		}
    		theSkipDict->level = newlevel;
		}
		
		// Create a new node and give it the key, value, and level.
		curNode = newSkipEntry(key, city, newlevel);
		
		/*
		 Lastly, set all of the nodes we need to update to point to this new node
		 and also set the new node to point to what the updated nodes were pointing to previously.
		 This "seats" the new node in between the previous nodes, and what the previous nodes were pointing to.
		*/
		for(x = 0; x <= newlevel; x++) {
			curNode->next[x] = nodesToUpdate[x]->next[x];
			nodesToUpdate[x]->next[x] = curNode;
		}
	}
}

/*
 Searches for a key in the given skip list dictionary.
 Returns the node if the key is found, otherwise it returns NULL if the key isn't in the dictionary.
*/
skipDictEntry* skipDictSearch(skipDict* theSkipDict, char* queryKey) {
	skipDictEntry* curNode = theSkipDict->head;
	
	if(curNode->next[0] == NULL) return NULL; // Cannot search an empty list.
	
	int x;
	
	/*
	 "Look ahead" at the next node on level "x". If the value of that node is 
	 less than our query key, then drop a level. Continue until we are at level 0.
	*/
	for(x = theSkipDict->level; x >= 0; x--) {
		
		while(curNode->next[x] != NULL && keycmp(curNode->next[x]->key, queryKey) < 0) {
			curNode = curNode->next[x];
		}
	}
	curNode = curNode->next[0];
	
	if(curNode != NULL && !keycmp(curNode->key, queryKey)) {
		return curNode;
	}
	
	return NULL;
}

/*
 Deletes an entry with the given key from the skip list dictionary
 This function does NOT assume that the entry is in the dictionary
*/
void skipDictDelete(skipDict* theSkipDict, char* key) {
	int comparisons = 0;
	int x;
	skipDictEntry* curNode = theSkipDict->head;
	skipDictEntry** nodesToUpdate = (skipDictEntry**)calloc(sizeof(skipDictEntry*),MAX_LEVEL + 1);
	for(x = theSkipDict->level; x >= 0; x--) {
		/*
		 Search for all the nodes that point to this node
		 and add them to list of nodes to update.
		*/
		while(curNode->next[x] != NULL && keycmp(curNode->next[x]->key, key) < 0) {
			comparisons++;
			curNode = curNode->next[x];
		}
		
		nodesToUpdate[x] = curNode;
	}
	
	curNode = curNode->next[0];
	
	if(!strcmp(curNode->key, key)) {
		/* 
		 Move up the levels of the nodes to update and make them 
		 point to the nodes that curNode points to, essentially 
		 breaking the ties to curNode.
		*/
        for(x = 0; x <= theSkipDict->level; x++) {
			if(nodesToUpdate[x]->next[x] != curNode) break;
			nodesToUpdate[x]->next[x] = curNode->next[x];
		}
		
		// Delete the node.
		free(curNode->key);
		free(curNode->next);
        free(curNode);
		
		// Finally, check through the list to see if the skip list level needs to be lowered.
        while(theSkipDict->level > 0 && theSkipDict->head->next[theSkipDict->level] == NULL) {
			theSkipDict->level--;
		}
    }
}

/*
 Deletes everything in a skip list dictionary, but not the dictionary itself
*/
void purgeSkipDict(skipDict* theSkipDict) {
	skipDictEntry* curNode = theSkipDict->head->next[0];
	skipDictEntry* nextNode;
	while(curNode != NULL) {
		nextNode = curNode->next[0];
		skipDictDelete(theSkipDict, curNode->key);
		curNode = nextNode;
	}
}
