#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>  
#include "cachesim.h"


//Define a Block so syntax is easier
typedef struct _Block
{
	uint tagId;
  	uint dirty;
  	uint valid;
} Block;


counter_t accesses = 0, write_hits = 0, read_hits = 0, 
          write_misses = 0, read_misses = 0, writebacks = 0;

//Globals
Block *cache;
uint globalWays;
uint globalCacheSize;
uint globalNumSets;

/* Use this method to initialize your cache data structure
* You should do all of your memory allocation here
*/
void cachesim_init(uint blocksize, uint cachesize, uint ways) 
{
	globalWays = ways;
	globalCacheSize = cachesize;
	uint numBlocks = cachesize/blocksize;
	uint numSets = numBlocks/ways;
	globalNumSets = numSets;
	//Because the difficult of setting up a 2D array we just use a 1D array
	//And use the ways * setIndex to find the first index of blocks from a set.
	cache = (Block *)malloc(sizeof(Block) * numBlocks);
	for (int i = 0; i < numBlocks; i++) {
		Block *initialBlock = cache + i;
		initialBlock->valid = 0;
		initialBlock->tagId = 0;
		initialBlock->dirty = 0;
	}
}

/* Clean up dynamically allocated memory when the program terminates */
void cachesim_destruct()
{

	if(cache){free(cache);}
}

/* Called on each access
* 'write' is a boolean representing whether a request is read(0) or write(1)
*/
void cachesim_access(addr_t physical_addr, uint write) 
{
	accesses++;
	//Block offset is determined by the size of the Cache
	uint blockOffset = physical_addr%globalCacheSize;
	physical_addr = physical_addr/globalCacheSize;

	//We want to take the necessary bits out for the set index
	uint setIndex = physical_addr%globalNumSets;
	physical_addr = physical_addr/globalNumSets;

	//The rest of the bits are the tag.
	uint tag = physical_addr;

	//Now to search the cache for the correct tag with the setIndex.
	uint searchIndexStart = setIndex * globalWays;
	uint allValid = 1;
	int inValidIndex = -1;
	//This iterates through a set.
	//It goes globalWays amount of times. 
	for(int i = 0; i < globalWays; i++) {
		Block iterator = cache[searchIndexStart+i];
		if(iterator.valid == 0) {
			allValid = 0;
			inValidIndex = searchIndexStart+i;
		}
		//HIT 
		if(iterator.valid == 1 && tag == iterator.tagId) {
			if(write == 1){			//WRITE HIT
				iterator.dirty = 1;
				write_hits++;
				return;
			} else {				//READ HIT
				read_hits++;
				return;
			}
		}
	}
	//If this for has completed and no hits were found, it is a miss.
	//MISS
	if(write == 1) {				//WRITE MISS
		write_misses++;
		if(allValid == 0){ //We can fill it up with a new one.

		} 
		else { //If there is none we must evict one.
			/*TODO EVICTION PROCESS*/
		}
	} else {						//READ MISS
		read_misses++;
		if(allValid == 0){ //We can fill it up with a new one.

		}
		else { //If there is none we must evict one.
			/*TODO EVICTION PROCESS*/
		}
	}
}

/* You may not change this method in your final submission!!!!! 
*   Furthermore, your code should not have any extra print statements
*/
void cachesim_print_stats() 
{
  printf("%llu, %llu, %llu, %llu, %llu, %llu\n", accesses, read_hits, write_hits, 
                                                read_misses, write_misses, writebacks);
}

int main(){
	unsigned long long pa = 0x1a2fbb8;
	cachesim_init(64, 8192, 4);
	cachesim_destruct();
}
