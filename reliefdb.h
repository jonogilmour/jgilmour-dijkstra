#ifndef reliefdb_h
#define reliefdb_h

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////STRUCTURES/////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 A travel table is a node for the list of cities that link to a certain city.
 The city's ID and the distance to that city are stored.
 A placeholder for the actual pointer to that city is stored as well in order
 to speed up the application.
 */
typedef struct traveltable {
	struct citynode* citypntr;
	long int cityid;
	long int distance;
} tt;

/*
 A citynode is a node in a city database that contains all the
 information about that city, including a travel table of the cities
 that link to it.
 The size of the travel table is stored in ttsize.
 */
typedef struct citynode {
	long int id;
	char* name;
	long int ttsize;
	tt** goes_to;
	char* resources;
	struct map* pathmap;
} cn;

/*
 A city database node is a wrapper that makes it easier to keep the
 collection of city nodes sorted within the citydb.
 Next and Prev are pointers that form the sorted doubly linked list.
 This is the node that this cdbn points to.
 */
typedef struct cdbnode {
	struct cdbnode* next;
	struct cdbnode* prev;
	cn* cur;
} cdbn;

/*
 A citydb is a database containing a list of all the citydb nodes stored
 from the database input file.
 The groupname is an optional use name for the group of cities used for printing
 eg "Australia", or "West Africa".
 The size of the city table is stored in ctsize.
 */
typedef struct citydb {
	char* groupname;
	long int ctsize;
	cdbn* chead;
} cdb;

/*
 A citypath (cpath) is a wrapper for paths taken from one city to another.
 
 - endID is the destination of this path.
 - totalDistance is the total number of hours involved in this journey.
 - length is the number of cities in the path (including the start and end).
 - path is a list of travel tables set in the order of the journey.
*/
typedef struct citypath {
	long int endID;
	long int totalDistance;
	long int length;
	tt** path;
} cpath;

/*
 A map is stored in a citynode and contains all the shortest path from 
 that city to all other accessible cities.
 
 - resources is a string representing the resources available to the city. This
   string can be checked in order to determine which resource list should 
   be searched prior to finding the shortest distance to that resource 
   (saves a lot of time).
 - size is the number of paths in the directions array, aka the number of cities
   accessible from the city with this map.
 - directions is a list of shortest paths to all cities accessible from this city
*/
typedef struct map {
	char* resources;
	long int size;
	cpath** directions;
} map;

/*
 A resource is a wrapper for a distance to a resource and the city that the resource is at.
 
 - endID is the destination of this path.
 - totalDistance is the total number of hours involved in this journey.
 - length is the number of cities in the path (including the start and end).
 - path is a list of travel tables set in the order of the journey.
*/
typedef struct resource {
	cn* city;
	long int totalDistance;
	cpath* path;
} rsc;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////FUNCTIONS//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

cdbn* CSearch(long int id, cdb* db);
int checkPath(cpath* path, long int id);
cn* findNearestCity(cdb* db, cn* city, cpath* path, long int* skip, long int skipsize, long int* distance);
void updateDistance(map* map, long int cityID, long int distance);
long int findPathForCity(map* map, cn* city);
void fillMap(cdb* db, map* map, cn* begin);
int getTotalDistance(cpath* path, int debug);
void updateShortestPathsToResources(cn* city, long int distance, cpath* path, rsc* resB, rsc* resF, rsc* resW, rsc* resD, rsc* resM);
cn* moveToCity(cdb* db, cpath* path, long int pathIndex);
int updateMapWithPath(map* map, long int mapIndex, cpath* path, long int pathIndex, cn* currentCity, long int totalDistance);
tt** constructTravelTable(char* travelString, cn* city);
void shortestPaths(cdb* db, cn* begin, cn* destination, rsc* resB, rsc* resF, rsc* resW, rsc* resD, rsc* resM);
void shortestPathsBack(cdb* db, cn* destination, rsc* resB, rsc* resF, rsc* resW, rsc* resD, rsc* resM);
void printPath(cpath* path);
void purgeDB(cdb* db);

#endif
