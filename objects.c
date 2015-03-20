#include <stdio.h>
#include <stdlib.h>
#include "strlib.h"
#include "reliefdb.h"
#include "objects.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////CONSTRUCTORS///////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Creates a new, empty city database with the given groupname.
 The citytable is allocated as empty with space for MINCITIES to prevent gratuitous
 reallocation.
 Returns the new city database
 */
cdb* newCDB(char* name)
{
	cdb* cityDB = (cdb*)malloc(sizeof(cdb));
	
	cityDB->groupname = cpystr(name);
	cityDB->ctsize = 0;
	cityDB->chead = NULL;
	
	return cityDB;
}

/*
 Creates a new cdb node with the information provided.
 Returns the freshly created node.
*/
cdbn* newCDBNode(cdb* db, cn* cur, cdbn* prev, cdbn* next)
{
	cdbn* newNode = (cdbn*)malloc(sizeof(cdbn));
	
	newNode->cur = cur;
	newNode->prev = prev;
	newNode->next = next;
	
	return newNode;
}

/*
 Constructs and returns a new city node with the given information.
 The citynode returned should be passed into a travel table
 constructor afterwards.
*/
cn* newCNode(long int id, char* nm, char* resources)
{
	cn* node = (cn*)malloc(sizeof(cn));
	
	node->id = id;
	node->name = cpystr(nm);
	node->resources = cpystr(resources);
	node->ttsize = 0;
	node->goes_to = NULL;
	
	return node;
}

/*
 Constructs a new travel table node with the information given.
 The citypntr is not allocated here, instead being allocated elsewhere
 during optimisation.
*/
tt* newTTable(long int cityid, long int distance)
{
	tt* table = (tt*)malloc(sizeof(tt));
	
	table->citypntr = NULL;
	table->cityid = cityid;
	table->distance = distance;
	
	return table;
}

/*
 Constructs a new empty map.
*/
map* newMap()
{
	map* newMap = (map*)malloc(sizeof(map));
	
	newMap->directions = NULL;
	
	return newMap;
}

/*
 Constructs a new cpath with the information given.
*/
cpath* newPath(long int id, long int tdist, long int length, tt** travelTable)
{
	cpath* path = (cpath*)malloc(sizeof(cpath));
	
	path->endID = id;
	path->totalDistance = tdist;
	path->length = length;
	path->path = travelTable;
	
	return path;
}

/*
 Creates a new resource structure with the information given.
 
 Returns the newly created resource.
*/
rsc* newResource(cn* city, long int dist, cpath* path)
{
	rsc* res = (rsc*)malloc(sizeof(rsc));
	
	res->city = city;
	res->totalDistance = dist;
	res->path = path;
	
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////COPY METHODS//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Returns a deep copy of a traveltable
*/
tt* copyTT(tt* ttToCopy)
{
	tt* newTT = (tt*)malloc(sizeof(tt));
	newTT->citypntr = ttToCopy->citypntr;
	newTT->cityid = ttToCopy->cityid;
	newTT->distance = ttToCopy->distance;
	
	return newTT;
}


/*
 Returns a deep copy of a citypath
*/
cpath* copyPath(cpath* pathToCopy)
{
	cpath* newPath = (cpath*)malloc(sizeof(cpath));
	
	newPath->endID = pathToCopy->endID;
	newPath->totalDistance = pathToCopy->totalDistance;
	newPath->length = pathToCopy->length;
	newPath->path = (tt**)malloc(sizeof(tt*) * newPath->length);
	
	
	int x;
	for(x = 0; x < newPath->length; x++) {
		newPath->path[x] = copyTT(pathToCopy->path[x]);
	}
	
	return newPath;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////SET METHODS//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 Initialises a path's travel table array to default.
*/
void initPathTT(cpath* path, long int length)
{
	long int x;
	for (x = 0; x < length; x++) {
		path->path[x] = newTTable(-1, 0);
	}
}

/*
 Sets a travel table to the values given.
*/
void setTravelTable(tt* tt, cn* city, long int dist)
{
	tt->cityid = city->id;
	tt->citypntr = city;
	tt->distance = dist;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////ADD/DELETE METHODS//////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Adds a new city data node to the city database provided in a sorted manner.
*/
void cdbAdd(cdb* db, cn* node)
{
	cdbn* toAdd = CSearch(node->id, db);
	if(db == NULL) return;
	if(toAdd != NULL && toAdd->cur->id == node->id) return; // Node already in database, no need to add it
	cdbn* newNode;
	
	if(db->chead == NULL) {
		// DB has no head, make it
		db->chead = newCDBNode(db, node, NULL, NULL);
	}
	
	// Node comes after toAdd
	else if(toAdd->cur->id < node->id) {
		newNode = newCDBNode(db, node, toAdd, toAdd->next);
		if(toAdd->next != NULL) toAdd->next->prev = newNode;
		toAdd->next = newNode;
	}
	
	// Node comes before toAdd
	else if(toAdd->cur->id > node->id) {
		newNode = newCDBNode(db, node, toAdd->prev, toAdd);
		if(toAdd == db->chead) db->chead = newNode;
		else toAdd->prev->next = newNode;
		toAdd->prev = newNode;
	}
}
