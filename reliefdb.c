#include <stdio.h>
#include <stdlib.h>
#include <limits.h> //for LONG_MAX
#include <string.h>
#include "reliefdb.h"
#include "strlib.h"
#include "objects.h"
#include "intlib.h"

#define MINCITIES 8
#define INF LONG_MAX
#define ZERO_LENGTH 0


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////OTHER FUNCTIONS////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 Search algorithm to find a city with the given id in the given database.
 
 Returns the node found.
 Returns NULL on error; either database is NULL or has no head.
 
 NOTE: The node found is not necessarily the correct node. If the function does not 
 find the node it still returns whatever node it ended up at. This is to make the 
 cdbAdd function faster and much easier to implement. The node returned should still 
 have its id checked against the desired city id after the fact, as it is in the 
 cdbAdd function.
*/
cdbn* CSearch(long int id, cdb* db)
{
	if(db == NULL || db->chead == NULL) return NULL;
	cdbn* curNode = db->chead;
	while(1) {
		if(curNode->cur->id == id || curNode->next == NULL) break;
		curNode = curNode->next;
	}
	
	return curNode;
}



/*
 Checks a path for a given city.
 
 Returns 0 if the city is in the path.
 Returns 1 if it isn't.
 */
int checkPath(cpath* path, long int id)
{
	int x;
	
	for(x = 0; x < path->length; x++) {
		if(path->path[x]->cityid == id) return 0;
	}
	
	return 1;
}

/*
 Returns the closest city to a city, that isn't in the skip array.
 
 Returns NULL if the travel table is empty or the node provided is NULL.
 Returns NULL if there is no nearest city (skip array contains all adjacent cities).
*/
cn* findNearestCity(cdb* db, cn* city, cpath* path, long int* skip, long int skipsize, long int* distance)
{
	if(city == NULL || city->ttsize == 0) return NULL;
	
	int x;
	cn* nearestCity = NULL;
	long int dist = INF;
	
	for(x = 0; x < city->ttsize; x++) {
		// If this city is nearer and isn't in the skip array and isnt in the current path:
		if(
		   city->goes_to[x]->distance < dist
		   && intSearch(skip, skipsize, city->goes_to[x]->cityid) == -1
		   && checkPath(path, city->goes_to[x]->cityid))
		{
			dist = city->goes_to[x]->distance;
			if(city->goes_to[x]->citypntr == NULL) nearestCity = CSearch(city->goes_to[x]->cityid, db)->cur;
			else nearestCity = city->goes_to[x]->citypntr;
		}
	}
	*distance = dist;
	return nearestCity;
}

/*
 Searches the given map's paths for the given city and updates the distance field 
 for that path ONLY if the distance is shorter than the currently stored distance.
*/
void updateDistance(map* map, long int cityID, long int distance)
{
	long int x;
	cpath** dir = map->directions;
	
	for(x = 0; x < map->size; x++) {
		if(dir[x]->endID == cityID) {
			if(distance < dir[x]->totalDistance) dir[x]->totalDistance = distance;
			break;
		}
	}
}

/*
 Finds the index for the given city's path inside a map's pathmap array.
 
 Returns the index, or -1 if not found.
*/
long int findPathForCity(map* map, cn* city)
{
	long int x;
	
	for(x = 0; x < map->size; x++) {
		if(map->directions[x]->endID == city->id) return x;
	}
	
	return -1;
}

/*
 Fills a map with infinite distance, NULL paths for every city ID in the database. 
 This is to initialise and allocate memory for the map's path array.
*/
void fillMap(cdb* db, map* map, cn* begin)
{
	//Start by filling up the map with infinite distance, NULL paths for every ID
	//Remember we don't care what the path is, as long as it's shortest
	int x = 0;
	cdbn* curNode = db->chead;
	map->directions = (cpath**)malloc(sizeof(cpath*) * db->ctsize);
	while(curNode->next != NULL) {
		map->directions[x] = newPath(curNode->cur->id, INF, 0, NULL);
		curNode = curNode->next;
		x++;
	}
	//one more for the last node
	map->directions[x] = newPath(curNode->cur->id, INF, 0, NULL);
}

/*
 Calculates the total distance currently travelled in a path.
 Returns the total distance.
*/
int getTotalDistance(cpath* path, int debug)
{
	int x;
	int totalDistance = 0;
	
	// Calculate the distance travelled by traversing the path and adding up all the distances.
	for(x = 0; x < path->length; x++) {
		totalDistance += path->path[x]->distance;
	}
	
	return totalDistance;
}



void printPath(cpath* path) {
	int x = 0;
	for(; x< path->length; x++) {
		printf("[%ld]", path->path[x]->cityid);
	}
}





/*
 Reads the resource list for the given city and updates the given resource 
 pointers if the distance to that resource is less than the stored distance 
 to that resource (ie a new shortest path to a resource has been found).
*/
void updateShortestPathsToResources(cn* city, long int distance, cpath* path, rsc* resB, rsc* resF, rsc* resW, rsc* resD, rsc* resM)
{
	int x = 0;
	rsc* curres;
	
	while (city->resources[x] != '\0') {

		switch (city->resources[x]) {
			case 'B':
				curres = resB;
				break;
			
			case 'F':
				curres = resF;
				break;
				
			case 'W':
				curres = resW;
				break;
				
			case 'D':
				curres = resD;
				break;
				
			case 'M':
				curres = resM;
				break;
			
			case 'X':
				curres = NULL;
				break;
				
			default:
				printf("WARNING: Invalid resource (%c) found in city with ID %ld. Skipping.\n", city->resources[x], city->id);
				curres = NULL;
				break;
		}
		
		if(curres == NULL) {
			x++;
			continue;
		}
		
		if(distance < curres->totalDistance) {
			curres->city = city;
			curres->path = copyPath(path);
			curres->totalDistance = distance;
		}
		
		x++;
	}
}



/*
 Searches the database for the given city in the path.
 
 Returns a pointer to the city (could be NULL if city doesn't exist).
*/
cn* moveToCity(cdb* db, cpath* path, long int pathIndex)
{
	return CSearch(path->path[pathIndex]->cityid, db)->cur;
}

/*
 Adds or updates the given map with the path.
 Adds it if it isn't already in the map.
 Updates it if it is already in the map.
 
 Returns 1 if the city was added.
 Returns 0 if the city was not added or if it was just updated.
*/
int updateMapWithPath(map* map, long int mapIndex, cpath* path, long int pathIndex, cn* currentCity, long int totalDistance)
{
	long int i = findPathForCity(map, currentCity);
	
	// Add this city's path into the map it if it isn't there.
	if(i == -1) {
		map->directions[mapIndex] = copyPath(path);
		map->size++;
		return 1;
	}
	// Otherwise if the current path is shorter than the stored one, update the stored path.
	else if(map->directions[i]->totalDistance > totalDistance) {
		map->directions[i] = copyPath(path);
	}
	return 0;
}

/*
 Takes the travel string and makes a travel table array out of it.
 
 Returns the newly created travel table array.
*/
tt** constructTravelTable(char* travelString, cn* city)
{
	city->ttsize = countchar(travelString, ':');
	
	tt** newtt = (tt**)malloc(sizeof(tt*) * city->ttsize);
	int index = 0;
	char* tempString;
	long int id;
	long int dist;
	int a = 0;
	
	tempString = strtok(travelString, ":");
	
	do {
		a++;
		id = atol(tempString);
		dist = atol(strtok(NULL, ","));
		newtt[index] = newTTable(id, dist);
		index++;
	}
	
	while((tempString = strtok(NULL, ":\n")) != NULL);
	return newtt;
}

/*
 Finds all the paths from the initial city (begin) to all other cities and places them in a map, assigned to the initial node.
 Also finds the shortest paths to available resources to the destination city.
 
 See project report for detailed information on how this function works
*/
void shortestPaths(cdb* db, cn* begin, cn* destination, rsc* resB, rsc* resF, rsc* resW, rsc* resD, rsc* resM)
{
	long int completedBranches[begin->ttsize];
	long int completedBranchesIndex = 0;
	long int doNotEnter[db->ctsize];
	long int doNotEnterIndex = 0;
	
	long int currentPathIndex = 0;
	long int pathMapIndex = 0;
	long int distanceToNextCity = 0;
	long int totalDistanceT = 0;
	
	int stopExplore = 0;
	
	cn* currentCity = begin;
	
	zeroOut(doNotEnter, db->ctsize);
	zeroOut(completedBranches, begin->ttsize);
	
	cpath* currentPath = newPath(begin->id, ZERO_LENGTH, 0, (tt**)malloc(sizeof(tt*) * db->ctsize));
	initPathTT(currentPath, db->ctsize);
	
	map* pathmap = newMap("");
	fillMap(db, pathmap, begin);
	
	
	setTravelTable(currentPath->path[0], begin, 0);
	currentPath->length = 1;
	pathMapIndex += updateMapWithPath(pathmap, pathMapIndex, currentPath, currentPathIndex, currentCity, totalDistanceT);
	currentPath->endID = begin->id;
	currentPathIndex = 1;
	
	
	while(completedBranchesIndex < begin->ttsize) {
		
		currentCity = findNearestCity(db, begin, currentPath, completedBranches, completedBranchesIndex, &distanceToNextCity);

		if(currentCity == NULL) {
			// Empty path, nowhere to go.
			break;
		}
		
		setTravelTable(currentPath->path[currentPathIndex], currentCity, distanceToNextCity);
		currentPath->length++;
		currentPath->endID = currentCity->id;
		
		totalDistanceT = getTotalDistance(currentPath,1);
		currentPath->totalDistance = totalDistanceT;
		
		if(currentCity == destination) updateShortestPathsToResources(begin, totalDistanceT, currentPath, resB, resF, resW, resD, resM);
		pathMapIndex += updateMapWithPath(pathmap, pathMapIndex, currentPath, currentPathIndex, currentCity, totalDistanceT);
		
		completedBranches[completedBranchesIndex] = currentCity->id;
		completedBranchesIndex++;
		
		
		while(1) {
			
			if(currentCity == begin) {
				break;
			}
			
			currentCity = findNearestCity(db, currentCity, currentPath, NULL, 0, &distanceToNextCity);
			
			
			while(currentCity == NULL) {
				
				currentCity = moveToCity(db, currentPath, currentPathIndex);

				doNotEnter[doNotEnterIndex] = currentCity->id;
				doNotEnterIndex++;
				
				if(currentCity == begin) {
					stopExplore = 1;
					break;
				}
				
				
				
				currentPathIndex--;
				currentPath->length--;
				
				currentCity = moveToCity(db, currentPath, currentPathIndex);
				
				if(currentCity == begin) {
					stopExplore = 1;
					break;
				}
				
				currentCity = findNearestCity(db, currentCity, currentPath, doNotEnter, doNotEnterIndex, &distanceToNextCity);
			}
			
			if(stopExplore) {
				stopExplore = 0;
				break;
			}
			
			currentPathIndex++;
			
			setTravelTable(currentPath->path[currentPathIndex], currentCity, distanceToNextCity);
			currentPath->length++;
			currentPath->endID = currentCity->id;

			totalDistanceT = getTotalDistance(currentPath,1);
			currentPath->totalDistance = totalDistanceT;
			
			if(currentCity == destination) updateShortestPathsToResources(begin, totalDistanceT, currentPath, resB, resF, resW, resD, resM);
			
			pathMapIndex += updateMapWithPath(pathmap, pathMapIndex, currentPath, currentPathIndex, currentCity, totalDistanceT);
		}
		
		
		zeroOut(doNotEnter, db->ctsize);
		doNotEnterIndex = 0;
		
		free(currentPath);
		currentPath = newPath(-1, 0, ZERO_LENGTH, (tt**)malloc(sizeof(tt*) * db->ctsize));
		initPathTT(currentPath, db->ctsize);
		setTravelTable(currentPath->path[0], begin, 0);
		currentPath->length = 1;
		currentPathIndex = 1;
		
	}
	
	begin->pathmap = pathmap;
	free(currentPath);
}


cdbn* nextNode(cdb* db, cdbn* node) {
	if(node == NULL) return db->chead;
	else return node->next;
}

/*
 Runs the shortest paths algorithm for every city in the database
*/
void shortestPathsBack(cdb* db, cn* destination, rsc* resB, rsc* resF, rsc* resW, rsc* resD, rsc* resM)
{
	if(db == NULL || db->ctsize == 0 || db->chead == NULL ||db->chead->cur == NULL) {
		printf("EMPTY DATABASE ERROR\n");
	}
	
	cdbn* node = NULL;
	
	int x;
	
	for(x = 0; x < db->ctsize; x++) {
		node = nextNode(db, node);
		shortestPaths(db, node->cur, destination, resB, resF, resW, resD, resM);
	}
}





/*
 The following functions free memory for different types of structures
*/
void purgePath(cpath* path)
{
	if(path == NULL) return;
	int x;
	
	for(x = 0; x < path->length; x++) {
		if(path->path[x] == NULL) continue;
		free(path->path[x]);
	}
	free(path->path);
	free(path);
}

void purgeMap(map* map)
{
	if(map == NULL) return;
	int x;
	
	for(x = 0; x < map->size; x++) {
		if(map->directions[x] == NULL) continue;
		purgePath(map->directions[x]);
	}
	free(map->directions);
	free(map);
}


void purgeCNode(cn* node)
{
	if(node == NULL) return;
	
	int x;
	
	for(x = 0; x < node->ttsize; x++) {
		if(node->goes_to[x] == NULL) continue;
		free(node->goes_to[x]);
	}
	purgeMap(node->pathmap);
	free(node->name);
	free(node->resources);
	free(node);
}

void purgeCDBNode(cdbn* node)
{
	purgeCNode(node->cur);
}



void purgeDB(cdb* db)
{
	if(db->chead == NULL) return;
	
	cdbn* node = db->chead;
	cdbn* next = node->next;
	int x;
	
	for(x = 0; x < db->ctsize; x++) {
		purgeCDBNode(node);
		
		if(next == NULL) break;
		
		node = next;
		next = node->next;
	}

	
	free(db);
}