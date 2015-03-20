#include <stdio.h>
#include "intlib.h"

/*
 Brute force search on an unsorted integer array.
 
 Returns the array index of the integer found
 Returns -1 if not found.
 */
long int intSearch(long int* array, long int size, long int query)
{
	if (array == NULL) return -1;
	
	long int x;
	
	for(x = 0; x < size; x++) {
		if(array[x] == query) return x;
	}
	
	return -1;
}

/*
 Makes an integer array all zeroes
 */
void zeroOut(long int* array, long int size)
{
	long int x;
	
	for(x = 0; x < size; x++) {
		array[x] = -1;
	}
}

/*
 Returns the number of digits in a number
 Eg, 24 has 2, 58748 has 5.
*/
int digits(long int n)
{
	int x = 0;
	do
	{
		x++;
		n /= 10;
	}
	while(n);
	return x;
}
