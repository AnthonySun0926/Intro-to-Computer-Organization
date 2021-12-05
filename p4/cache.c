/*
 * EECS 370, University of Michigan
 * Project 4: LC-2K Cache Simulator
 * Instructions are found in the project spec.
 */

#include <stdio.h>
#include <math.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

extern int mem_access(int addr, int write_flag, int write_data);

enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;
    int valid;
    int lruLabel;//largest = lru
    int set;
    int tag;
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(int, int, enum actionType);
void printCache();
int lruIndex(cacheStruct cache, int set);

/*
 * Set up the cache with given command line parameters. This is 
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet){
    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;
    for (int i = 0; i < cache.numSets; i++)
    {
        for (int j = 0; j < cache.blocksPerSet; j++)
        {
            cache.blocks[i * cache.blocksPerSet + j].dirty = 0;
            cache.blocks[i * cache.blocksPerSet + j].valid = 0; 
            cache.blocks[i * cache.blocksPerSet + j].set = i;
            cache.blocks[i * cache.blocksPerSet + j].lruLabel = 0;
            cache.blocks[i * cache.blocksPerSet + j].tag = -1;
        }
    }
}

/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data) {
    int currBO = addr % cache.blockSize;
    int currSet = (addr / cache.blockSize) % cache.numSets;
    int currTag = addr / cache.blockSize / cache.numSets;
    // sw
    if (write_flag)
    {
        // check if cache hit
        int blockIndex = currSet*cache.blocksPerSet;
        while (blockIndex < (currSet+1)*cache.blocksPerSet && currTag != cache.blocks[blockIndex].tag)
        {
            blockIndex++;
        }
        // hit
        if (blockIndex < (currSet+1)*cache.blocksPerSet)
        {
            // update cache line data
            cache.blocks[blockIndex].data[currBO] = write_data;
            cache.blocks[blockIndex].dirty = 1;
            cache.blocks[blockIndex].valid = 1;
            // update lru
            for (int i = currSet*cache.blocksPerSet; i < (currSet+1)*cache.blocksPerSet; i++)
            {
                if (cache.blocks[i].lruLabel < cache.blocks[blockIndex].lruLabel)
                {
                    cache.blocks[i].lruLabel++;
                }
            }
            cache.blocks[blockIndex].lruLabel = 0;
            // print action
            printAction(addr,1,processorToCache);
            return 0;
        }
        // miss
        else {
            //check if eviction needed
            int emptyBlock = currSet*cache.blocksPerSet;
            while (emptyBlock < (currSet+1)*cache.blocksPerSet && cache.blocks[emptyBlock].valid)
            {
                emptyBlock++;
            }
            // eviction needed
            if (emptyBlock == (currSet+1)*cache.blocksPerSet)
            {
                int lruBlock = lruIndex(cache,currSet);
                int blockAddr = ((cache.blocks[lruBlock].tag * cache.numSets) + cache.blocks[lruBlock].set) * cache.blockSize;
                // if block needs to be evicted has dirty bit = 1.
                if (cache.blocks[lruBlock].dirty)
                {
                    // update mem with the needing evicted lru block
                    for (int i = 0; i < cache.blockSize; i++)
                    {
                        int currAddr = blockAddr + i;
                        mem_access(currAddr,1,cache.blocks[lruBlock].data[i]);
                    }
                    printAction(blockAddr,cache.blockSize,cacheToMemory);
                }
                // lru block not dirty
                else 
                {
                    printAction(blockAddr,cache.blockSize,cacheToNowhere);
                }
                // update cache: replace the lru block from mem
                for (int j = 0; j < cache.blockSize; j++)
                {
                    cache.blocks[lruBlock].data[j] = mem_access(addr - currBO + j,0,write_data);
                }
                printAction(addr - currBO,cache.blockSize,memoryToCache);
                // update cache line data
                cache.blocks[lruBlock].data[currBO] = write_data;
                cache.blocks[lruBlock].tag = currTag;
                cache.blocks[lruBlock].dirty = 1;
                cache.blocks[lruBlock].valid = 1;
                printAction(addr,1,processorToCache);
                //update lru
                for (int i = currSet*cache.blocksPerSet; i < (currSet+1)*cache.blocksPerSet; i++)
                {
                    cache.blocks[i].lruLabel++;
                }
                cache.blocks[lruBlock].lruLabel = 0;  
                return 0;  
            }
            // exist empty blocks in the set
            else 
            {
                // read from mem
                for (int j = 0; j < cache.blockSize; j++)
                {
                    cache.blocks[emptyBlock].data[j] = mem_access(addr - currBO + j,0,write_data);
                }
                printAction(addr - currBO,cache.blockSize,memoryToCache);                
                // upadte cache line data
                cache.blocks[emptyBlock].data[currBO] = write_data;
                cache.blocks[emptyBlock].tag = currTag;
                cache.blocks[emptyBlock].dirty = 1;
                cache.blocks[emptyBlock].valid = 1;
                printAction(addr,1,processorToCache);
                // update lru
                for (int i = currSet*cache.blocksPerSet; i < (currSet+1)*cache.blocksPerSet; i++)
                {
                    cache.blocks[i].lruLabel++;
                }
                cache.blocks[emptyBlock].lruLabel = 0;    
                return 0;                
            }
        }
    }
    // fetch/lw
    else
    {
        // check if cache hit
        int blockIndex = currSet*cache.blocksPerSet;
        while (blockIndex < (currSet+1)*cache.blocksPerSet && currTag != cache.blocks[blockIndex].tag)
        {
            blockIndex++;
        }
        // if hit
        if (blockIndex < (currSet+1)*cache.blocksPerSet)
        {
            printAction(addr, 1, cacheToProcessor);
            // update lru
            for (int i = currSet*cache.blocksPerSet; i < (currSet+1)*cache.blocksPerSet; i++)
            {
                if (cache.blocks[i].lruLabel < cache.blocks[blockIndex].lruLabel)
                {
                    cache.blocks[i].lruLabel++;
                }
            }
            cache.blocks[blockIndex].lruLabel = 0;          
            return cache.blocks[blockIndex].data[currBO];
        }
        // if miss
        else
        {
            // load block from mem
            //check if eviction needed
            int emptyBlock = currSet*cache.blocksPerSet;
            while (emptyBlock < (currSet+1)*cache.blocksPerSet && cache.blocks[emptyBlock].valid)
            {
                emptyBlock++;
            }
            // eviction needed
            if (emptyBlock == (currSet+1)*cache.blocksPerSet)
            {
                int lruBlock = lruIndex(cache,currSet);
                int blockAddr = ((cache.blocks[lruBlock].tag * cache.numSets) + cache.blocks[lruBlock].set) * cache.blockSize;
                // if block needs to be evicted has dirty bit = 1.
                if (cache.blocks[lruBlock].dirty)
                {
                    // update mem with the needing evicted lru block
                    for (int i = 0; i < cache.blockSize; i++)
                    {
                        int currAddr = blockAddr + i;
                        mem_access(currAddr,1,cache.blocks[lruBlock].data[i]);
                    }
                    printAction(blockAddr,cache.blockSize,cacheToMemory);
                }
                // lru block not dirty
                else
                {
                    printAction(blockAddr,cache.blockSize,cacheToNowhere);
                }
                // update cache: replace the lru block from mem
                for (int j = 0; j < cache.blockSize; j++)
                {
                    cache.blocks[lruBlock].data[j] = mem_access(addr - currBO + j,0,write_data);
                }
                cache.blocks[lruBlock].tag = currTag;
                cache.blocks[lruBlock].valid = 1;
                cache.blocks[lruBlock].dirty = 0;
                printAction(addr - currBO,cache.blockSize,memoryToCache);
                //update lru
                for (int i = currSet*cache.blocksPerSet; i < (currSet+1)*cache.blocksPerSet; i++)
                {
                    cache.blocks[i].lruLabel++;
                }
                cache.blocks[lruBlock].lruLabel = 0;  
                // return cache line data
                printAction(addr, 1, cacheToProcessor);
                return cache.blocks[lruBlock].data[currBO];                  
            }
            // exist empty blocks in the set
            else 
            {
                // read from mem
                for (int j = 0; j < cache.blockSize; j++)
                {
                    cache.blocks[emptyBlock].data[j] = mem_access(addr - currBO + j,0,write_data);
                }
                cache.blocks[emptyBlock].tag = currTag;
                cache.blocks[emptyBlock].valid = 1;
                cache.blocks[emptyBlock].dirty = 0;
                printAction(addr - currBO,cache.blockSize,memoryToCache);
                // update lru
                for (int i = currSet*cache.blocksPerSet; i < (currSet+1)*cache.blocksPerSet; i++)
                {
                    cache.blocks[i].lruLabel++;
                }
                cache.blocks[emptyBlock].lruLabel = 0;     
                // return cache line data
                printAction(addr, 1, cacheToProcessor);
                return cache.blocks[emptyBlock].data[currBO];                               
            }
        }            
    }
}

int lruIndex(cacheStruct cache, int set){
    int max = cache.blocks[set*cache.blocksPerSet].lruLabel;
    int res = set*cache.blocksPerSet;
    for (int i = set*cache.blocksPerSet + 1; i < (set+1)*cache.blocksPerSet; i++)
    {
        if (cache.blocks[i].lruLabel > max)
        {
            max = cache.blocks[i].lruLabel;
            res = i;
        }
    }
    return res;
}

/*
 * print end of run statistics like in the spec. This is not required,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 */
void printStats(){
    printCache();
    printf("End of run statistics:\n");
    return;
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache()
{
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            printf("\t\t[ %i ]: {", block);
            for (int index = 0; index < cache.blockSize; ++index) {
                printf(" %i", cache.blocks[set * cache.blocksPerSet + block].data[index]);
            }
            printf(" }\n");
        }
    }
    printf("end cache\n");
}