//
//  cachesim.c
//  
//
//  Created by Frankie Willard on 4/10/21.
//

//#include "cachesim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct cache_frame {
    char data[64];
    
    //Metadata since it's a frame
    int valid_bit;
    
    //Dont care for write through- denote that block has been modified and those changes not reflected in memory
    int dirty_bit;
    int myTag;
    
    //Could add counter to help track LRU status
    int LRUcount;
    
};

/*
struct cache_set {
    //cacheKB / blockSize / ways
    struct cache_frame* myBlock;
}*/

//Step 3- Declare in global scope
char memory[65536];
//Initialize all to 0

int main(int argc, char* argv[])
{
    FILE *fileptr;
    int cacheKB;
    int ways;
    char cacheType[3];
    int blockSize;
    
    //Initializes parameter
    fileptr = fopen(argv[1], "r");
    cacheKB = atoi(argv[2]);
    ways = atoi(argv[3]);
    strcpy(cacheType, argv[4]);
    blockSize = atoi(argv[5]);
    
    //Calculations
    int num_frames = (cacheKB * 1024) / blockSize;
    int num_sets = num_frames / ways;
    int offset_bits = log(blockSize) / log(2);
    int index_bits = log(num_sets) / log(2);
    //Word size is 16
    int tag_bits = 16 - offset_bits - index_bits;
    
    //Declares 2D array
    struct cache_frame myCache[num_sets][ways];
    
    for(int k = 0; k < num_sets; k++)
    {
        for(int l = 0; l < ways; l++)
        {
            myCache[k][l].valid_bit = 0;
            myCache[k][l].LRUcount = 0;
        }
    }
    
    //Do we construct blocks for every
    
    char current_line[160];
    char accessType[6];
    
    int lrucounter = 1;

    //Use fscanf instead of fgets- Piazza
    while(fscanf(fileptr, "%s", accessType) != EOF)
    {
        int addrToAccess;
        int accessSize;
        //int valToStore;
        char valueToStore[blockSize];

        fscanf(fileptr, "%x %i", &addrToAccess, &accessSize);
        
        bool isStore = strcmp(accessType, "store") == 0;
        
        if(isStore)
        {
            //Piazza
            for(int i = 0; i < accessSize; i++)
            {
                fscanf(fileptr, "%02hhx", valueToStore + i);
            }
        }
            
        //CHECK FOR HIT
        
        //Calculate values
        int block_offset = addrToAccess % blockSize;
        int index = (addrToAccess / blockSize) % num_sets;
        int tag = addrToAccess / (num_sets * blockSize);
        
        
        printf("%s %04x", accessType, addrToAccess);
        
        
        //Load hit/miss same, store hit/miss differs depending on cache type
        
        char accessSuccess[10];
        
        strcpy(accessSuccess, "miss");
        
        //Iterate through cache based on set=index
        
        
        bool cacheFull = true;
        
        int LRUindex = 0;
        
        
        int LRUvalue = myCache[index][0].LRUcount;
        
        
                
        //Iterate over the ways of cache[index][i]
        for(int i = 0; i < ways; i++)
        {
            if(myCache[index][i].valid_bit == 0)
            {
                //Cache block empty- read data and tag into cache
                //Assuming that data in front of it will be nonempty                
                LRUindex = i;
                cacheFull = false;
                break;
            }
            
            //printf("\n%i  and %i\n", myCache[index][i].myTag, myCache[index][i].LRUcount);
            
            //Check if tag of cache equal to tag of data
            if(tag == myCache[index][i].myTag && myCache[index][i].valid_bit == 1)
            {
                strcpy(accessSuccess, "hit");
                printf(" hit");
                myCache[index][i].LRUcount = lrucounter;
                
                if(isStore)
                {
                    //Write data to cache if wb
                    //Write data to cache and memory if wt
                    
                    
                    //Loop from block offset to accessSize + block offset
                    //Set data in cache equal valueToStore
                        //Use memcpy not strcpy as it wont break upon seeing NULL char
                    memcpy(((myCache[index][i].data) + block_offset), (valueToStore), accessSize);
                    
                    //myCache[index][i].myTag = tag;
                    //myCache[index][i].valid_bit = 1;
                    
                    if(strcmp(cacheType, "wt") == 0)
                    {
                        memcpy( (memory + addrToAccess), (valueToStore), accessSize);
                    }
                    
                    //Mark as dirty, will update in memory when evicted
                    if(strcmp(cacheType, "wb") == 0)
                    {
                        myCache[index][i].dirty_bit = 1;
                    }
                    break;
                }
                
                
                //Load hit
                if(!isStore)
                {
                    //Print access size bytes of data starting at the address specified after load in trace file
                    
                    //Want data from myCache[index][i] -> data
                    //From data[block_offset] to data[block_offset + accessSize]
                    printf(" ");
                    for(int j = block_offset; j < accessSize + block_offset; j++)
                    {
                        printf("%02hhx", *( (myCache[index][i].data) + j));
                    }
                    break;
                }
                
            }
            //Update LRU
            
            if(myCache[index][i].LRUcount < LRUvalue)
            {
                LRUindex = i;
                LRUvalue = myCache[index][i].LRUcount;
            }
        }
        
        if(strcmp(accessSuccess, "miss") == 0)
        {
            printf(" miss");
            
            //Could just be store wb and load

            //Store miss
            if(isStore)
            {
                //Move block from memory into cache- make sure that you move entire block, make space if set is full
                                
                if(strcmp(cacheType, "wt") == 0)
                {
                    //Write through cache
                    
                    //for(int j = 0; j < accessSize; j++)
                    //{
                        memcpy( (memory + addrToAccess), (valueToStore), accessSize);



                    //}
                    
                }
                
                if(strcmp(cacheType, "wb") == 0)
                {
                    myCache[index][LRUindex].LRUcount = lrucounter;

                    myCache[index][LRUindex].valid_bit = 1;
                    
                    /*
                    if(!cacheFull)
                    {
                        printf(" space ");
                    }
                    
                    if(myCache[index][LRUindex].dirty_bit == 0)
                    {
                        printf(" clean ");
                    }*/
                    
                    if(cacheFull && myCache[index][LRUindex].dirty_bit == 1)
                    {
                        //Assume offset is 0
                        
                        
                        //printf(" SMeviction ");
                        
                        //int pastmem = (((tag << index_bits) + index) << offset_bits) + 0;
                        int memAddr = (((myCache[index][LRUindex].myTag << index_bits) + index) << offset_bits) + 0;
                        
                        //printf(" %i %i ", pastmem, memAddr);
                        
                        //Write to memory
                        //for(int j = 0; j < blockSize; j++)
                        //{
                            memcpy( (memory + memAddr), (myCache[index][LRUindex].data), blockSize);
                        //}
                    }
                    
                    myCache[index][LRUindex].myTag = tag;

                    
                    //Bring block from memory into cache
                    //for(int j = 0; j < blockSize; j++)
                    //{
                        memcpy((myCache[index][LRUindex].data), (memory + int(addrToAccess / blockSize) * blockSize), blockSize);
                    //}
                    //for(int j = 0; j < accessSize; j++)
                    //{
                        memcpy((myCache[index][LRUindex].data + block_offset), (valueToStore), accessSize);
                    //}
                    //Set cache block to dirty
                    myCache[index][LRUindex].dirty_bit = 1;

                }

                
                //Write data same as store hit-
                //Write data to cache if wb, update block now in cache, set block as dirty, write to memory when block going to be evicted
                
                //Write data to memory if wt (not sure about cache)
            }
            if(!isStore)
            {
                myCache[index][LRUindex].LRUcount = lrucounter;

                //Print access size bytes of data starting at address specified after word "load" in the trace file
                
                //But also need to move block from memory into cache, kick out lru block if set is full
                                
                //Evict from LRU if replacing a block that has dirty data in cache, and cache is writeback
                
                if(cacheFull && strcmp(cacheType, "wb") == 0 && myCache[index][LRUindex].dirty_bit == 1)
                {
                    //printf(" LMeviction ");
                    int memAddr = (((myCache[index][LRUindex].myTag << index_bits) + index) << offset_bits) + 0;

                    //Write to memory
                    //for(int j = 0; j < blockSize; j++)
                    //{
                        //Assume offset is 0
                        //WHAT IS HERE- set bits and tag bits with block offset is 0
                    memcpy( (memory + memAddr), (myCache[index][LRUindex].data), blockSize);
                    //}
                    
                    myCache[index][LRUindex].dirty_bit = 0;

                }
                
                //memory[address] - memory[address + blockSize]
                
                //for(int j = 0; j < blockSize; j++)
                //{
                    memcpy((myCache[index][LRUindex].data), (memory + int (addrToAccess / blockSize) * blockSize), blockSize);
               
                //memcpy((myCache[index][LRUindex].data), (memory + addrToAccess), blockSize);

                //}
                
                
                
                myCache[index][LRUindex].myTag = tag;
                myCache[index][LRUindex].valid_bit = 1;
                
                
                printf(" ");
                                
                for(int j = block_offset; j < accessSize + block_offset; j++)
                {
                    printf("%02hhx", *( (myCache[index][LRUindex].data) + j));
                }
                
            }
        }
        
        lrucounter++;
        
        
        //Miss scenario 1: No data in the cache, read data and tag into cache
        //Miss scenario 2: Data in cache, cycle through, if no null, then use LRU
        
        
        //If miss read data from memory array
        
        
        printf("\n");

        
        //I/O
        
        //Move up printing to be earlier in code
        //Only need to print accessType and addr
        /*
        printf("%s %04x %i", accessType, addrToAccess, accessSize);
        
        if(strcmp(accessType, "store") == 0) {
            printf(" ");
            //Piazza
            for(int i = 0; i < accessSize; i++)
            {
                printf("%02hhx", *(valueToStore + i));
            }
        }
        
        printf("\n");
         */
        
        
        
    }
    
    
    
    fclose(fileptr);
    
    return 0;
    //Cache set- 2d array with block as element, row is set, cols = associatvity
    
    //Or dynamically allocate
    //struct cache_set cache[ways];
}
