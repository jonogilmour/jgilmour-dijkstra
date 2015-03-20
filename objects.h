#include "reliefdb.h"

#ifndef copylib_h
#define copylib_h

cdb* newCDB(char* name);
cdbn* newCDBNode(cdb* db, cn* cur, cdbn* prev, cdbn* next);
cn* newCNode(long int id, char* nm, char* resources);
tt* newTTable(long int cityid, long int distance);
map* newMap();
cpath* newPath(long int id, long int tdist, long int length, tt** travelTable);
rsc* newResource(cn* city, long int dist, cpath* path);

tt* copyTT(tt* ttToCopy);
cpath* copyPath(cpath* pathToCopy);

void initPathTT(cpath* path, long int length);
void setTravelTable(tt* tt, cn* city, long int dist);

void cdbAdd(cdb* db, cn* node);

#endif
