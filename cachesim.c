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
  	uint LRUOrder;
} Block;


counter_t accesses = 0, write_hits = 0, read_hits = 0, 
          write_misses = 0, read_misses = 0, writebacks = 0;

//Globals
Block *cache;
uint globalWays;
uint globalBlockSize;
uint globalNumSets;
int onetime = 0;

/* Use this method to initialize your cache data structure
* You should do all of your memory allocation here
*/
void cachesim_init(uint blocksize, uint cachesize, uint ways) 
{
	globalWays = ways;
	globalBlockSize = blocksize;
	uint numBlocks = cachesize/blocksize;
	uint numSets = numBlocks/ways;
	globalNumSets = numSets;
	//Because the difficult of setting up a 2D array we just use a 1D array
	//And use the ways * setIndex to find the first index of blocks from a set.
	cache = (Block *)malloc(sizeof(Block) * numBlocks);
	for (int i = 0; i < numBlocks; i++) {
		(cache + i)->valid = 0;
		(cache + i)->tagId = 0;
		(cache + i)->dirty = 0;
		(cache + i)->LRUOrder = 0;
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
	uint blockOffset = physical_addr%globalBlockSize;
	physical_addr = physical_addr/globalBlockSize;
	

	//We want to take the necessary bits out for the set index
	uint setIndex = physical_addr%globalNumSets;
	physical_addr = physical_addr/globalNumSets;
	//The rest of the bits are the tag.
	uint tag = physical_addr;
	
	

	//Now to search the cache for the correct tag with the setIndex.
	uint searchIndexStart = setIndex * globalWays;
	uint allValid = 1;	//Keeps track of whether all valid.
	int invalidIndex = -1; //This does mark the last valid index.
	int lowestLRUOrderIndex = searchIndexStart; //This marks the lowestLRUOrder index.
	uint lowestLRUOrder = (cache+searchIndexStart)->LRUOrder;
	//This iterates through a set.
	//It goes globalWays amount of times. 
	for(int i = 0; i < globalWays; i++) {
		//HIT 
		if((cache + (searchIndexStart+i))->valid == 1 
			&& tag == (cache + (searchIndexStart+i))->tagId) {
			if(write == 1){			//WRITE HIT
				(cache + (searchIndexStart+i))->dirty = 1;
				(cache + (searchIndexStart+i))->LRUOrder = accesses;
				write_hits++;
				return;
			} else {				//READ HIT
				read_hits++;
				(cache + (searchIndexStart+i))->LRUOrder = accesses;
				return;
			}
		}
		//These two are used in case of MISS
		//If there are any invalid blocks, this will record their location.
		if((cache + (searchIndexStart+i))->valid == 0) {
			allValid = 0;
			invalidIndex = searchIndexStart+i;
		}
		if((cache + (searchIndexStart+i))->LRUOrder < lowestLRUOrder){
			lowestLRUOrder = (cache + (searchIndexStart+i))->LRUOrder;
			lowestLRUOrderIndex = searchIndexStart+i;
		}
	}
	//If the previous for has completed and no hits were found, it is a miss.
	//MISS
	if(write == 1) {				//WRITE MISS
		write_misses++;
		if(allValid == 0){ //We can fill it up with a new one.
			(cache + invalidIndex)->tagId = tag;
			(cache + invalidIndex)->valid = 1;
			(cache + invalidIndex)->dirty = 1;
			(cache + invalidIndex)->LRUOrder = accesses;
		} 
		else { //If there is none we must evict one.
			//We have to find the block with the lowest LRUOrder
			if((cache + lowestLRUOrderIndex)->dirty == 1){	//If we evict a dirty block, writeback
				writebacks++;}
			(cache + lowestLRUOrderIndex)->dirty = 1;
			(cache + lowestLRUOrderIndex)->tagId = tag;
			(cache + lowestLRUOrderIndex)->valid = 1;
			(cache + lowestLRUOrderIndex)->LRUOrder = accesses;
		}
	} else {						//READ MISS
		read_misses++;
		if(allValid == 0){ //We can fill it up with a new one.
			(cache + invalidIndex)->tagId = tag;
			(cache + invalidIndex)->valid = 1;
			(cache + invalidIndex)->dirty = 0;
			(cache + invalidIndex)->LRUOrder = accesses;
		}
		
		else { //We must evict one.
			if((cache + lowestLRUOrderIndex)->dirty == 1){	//If we evict a dirty block, writeback
				writebacks++;} 		
			(cache + lowestLRUOrderIndex)->dirty = 0;
			(cache + lowestLRUOrderIndex)->tagId = tag;
			(cache + lowestLRUOrderIndex)->valid = 1;
			(cache + lowestLRUOrderIndex)->LRUOrder = accesses;
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


