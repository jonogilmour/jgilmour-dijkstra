#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> //FOR LONG_MAX
#include <ctype.h>
#include "objects.h"
#include "reliefdb.h"
#include "strlib.h"
#include "skipdict.h"
#include "intlib.h"

#define INF LONG_MAX
#define MAX_INT_LENGTH 6
#define MAX_DISTANCE (14*24)



int main(int argc, const char * argv[])
{
	if(argc < 2) {
		printf("usage: relief filename\n");
		exit(EXIT_FAILURE);
	}
	
	const char* filename = argv[1];
	
	FILE* dbfile = fopen(filename, "r");
	
	if(dbfile == NULL) {
		printf("File %s not found\n", argv[1]);
		return 0;
	}
	
	// Setup database, using the number cities given in the file as the size of the database.
	cdb* cityDatabase = newCDB("DefaultName");
	char* temp = (char*)malloc( sizeof(char) * (MAX_INT_LENGTH+1) );
	fgets(temp, MAX_INT_LENGTH, dbfile);
	int numOfCities = atoi(temp);
	cityDatabase->ctsize = numOfCities;
	free(temp);
	
	// Make a new skip list dictionary for the names of the cities
	skipDict* cityNameDict = newSkipDict();
	
	const int ID_MAX = digits(numOfCities);
	const int NM_MAX = 100;
	const int RLF_MAX = 6;
	const int TRAVEL_MAX = (numOfCities - 1) * (ID_MAX + digits(MAX_DISTANCE) + 2); // Max length of an id, plus the max length of a distance, plus 2 characters for the colon and comma.
	const int MAX_LENGTH = ID_MAX + NM_MAX + RLF_MAX + TRAVEL_MAX;
	
	char* buffer = (char*)calloc(sizeof(char), MAX_LENGTH + 1);
	char* cityID;
	char* cityName;
	char* cityRelief;
	char* cityTravel;
	
	cn* thisCity;
	
	rsc *resB, *resF, *resW, *resD, *resM;
	resB = newResource(NULL, INF, NULL);
	resF = newResource(NULL, INF, NULL);
	resW = newResource(NULL, INF, NULL);
	resD = newResource(NULL, INF, NULL);
	resM = newResource(NULL, INF, NULL);
	
	int buflen;
	
	skipDictEntry* returnCity;
	cdbn* cityDBNodeInDistress;
	cn* cityInDistress;
	
	int x;
	int y;
	char currentResource;
	rsc* curResShortestRoute;
	
	cpath* pathToResource;
	
	char* currentCityName;
	
	printf("\nReading file and constructing database...\n");
	
	// For every line in the file, add it to the database.
	while(fgets(buffer, MAX_LENGTH, dbfile) != NULL) {
		cityID = strtok(buffer, "|");
		cityName = strtok(NULL, "|");
		cityRelief = strtok(NULL, "|");
		cityTravel = strtok(NULL, "\n"); //city:dist,city:dist...
		thisCity = newCNode(atoi(cityID), cityName, cityRelief); // Make a new city node with this information.
		if(strfind(cityTravel, ':')) thisCity->goes_to = constructTravelTable(cityTravel, thisCity); // Make the travel table array for the city using the cityTravel line.
		else thisCity->goes_to = NULL;
		cdbAdd(cityDatabase, thisCity); // Add this city to the database.
		addSkipEntry(cityNameDict, cityName, thisCity); //Add the city to the dictionary using the name string as the key.
	}
	
	// Now ask the user for input on disaster area and resources needed.
	while(1) {
		printf("\nPlease input city in distress (ID or name) or type !exit to exit: ");
		fgets(buffer, MAX_LENGTH, stdin);
		stripstr(buffer, '\n');
		
		
		if(!strcmp(buffer, "!exit")) {
			printf("Thank you.\n\n");
			break;
		}
		
		if(!lengthof(buffer)) continue;
		
		
		if( (returnCity = skipDictSearch(cityNameDict, buffer)) != NULL && !strcmp(returnCity->city->name, buffer)) {
			// Name was found.
			cityInDistress = returnCity->city;
		}
		
		else if(strIntegrityCheck(buffer, "0123456789") && (cityDBNodeInDistress = CSearch(strtol(buffer, NULL, 10), cityDatabase)) != NULL && cityDBNodeInDistress->cur->id == strtol(buffer, NULL, 10)) {
			// ID was found.
			cityInDistress = cityDBNodeInDistress->cur;
		}
		
		else {
			printf("City not found. Please try again.\n");
			continue;
		}
		
		printf("\nCity Found: %s (ID %ld)\n", cityInDistress->name, cityInDistress->id);
		
		// Build path map.
		shortestPathsBack(cityDatabase, cityInDistress, resB, resF, resW, resD, resM);
		
		while(1) {
			printf("\nPlease input resources required with no spaces (B, F, W, D, or M) eg 'BFW': ");
			fgets(buffer, MAX_LENGTH, stdin);
			stripstr(buffer, '\n');
			buflen = len(buffer);
			
			if(!buflen) {
				// Empty string entered, just return nearest
				printf("\nNo resources entered, searching for nearest resource...\n");
				buffer[0] = 'B';
				buffer[1] = 'F';
				buffer[2] = 'W';
				buffer[3] = 'D';
				buffer[4] = 'M';
				buffer[5] = '\0';
				buflen = len(buffer);
			}
			
			// Check for invalid characters.
			if(strIntegrityCheck(buffer, "BFWDM")) break;
			
			printf("Invalid character(s) found.\n");
		}
		
		printf("\nFinding shortest paths to resources...\n-----------------------------\n\n");

		// Print out the shortest paths to the resources
		for(x = 0; x < buflen; x++) {
			currentResource = buffer[x];
			currentResource = toupper(currentResource);
			
			switch (currentResource) {
				case 'B':
					curResShortestRoute = resB;
					break;
				
				case 'F':
					curResShortestRoute	= resF;
					break;
					
				case 'W':
					curResShortestRoute = resW;
					break;
				
				case 'D':
					curResShortestRoute = resD;
					break;
				
				case 'M':
					curResShortestRoute = resM;
					break;
					
				default:
					break;
			}
			
			if(curResShortestRoute->city == NULL) {
				printf("Resource %c is not available.\n\n", currentResource);
				continue;
			}
			
			pathToResource = curResShortestRoute->path;
			
			printf("Path for resource %c from city %s (%ld) to disaster zone %s (%ld):\n", currentResource, curResShortestRoute->city->name, curResShortestRoute->city->id, cityInDistress->name, cityInDistress->id);

			for(y = 0; y < pathToResource->length; y++) {
				if(pathToResource->path[y]->citypntr == NULL) {
					currentCityName = CSearch(pathToResource->path[y]->cityid, cityDatabase)->cur->name;
				}
				else currentCityName = pathToResource->path[y]->citypntr->name;
				printf("%d - City: %s (%lu) | Distance: %lu hrs\n", y + 1, currentCityName, pathToResource->path[y]->cityid, pathToResource->path[y]->distance);
			}
			printf("Total Distance: %ld hrs\n\n", pathToResource->totalDistance);
		}
		printf("-----------------------------\n");
		
	}

	// Free all memory.
	purgeSkipDict(cityNameDict);
	free(cityNameDict);
	free(buffer);
	free(resB);
	free(resF);
	free(resW);
	free(resD);
	free(resM);
	purgeDB(cityDatabase);
	
    return 0;
}

