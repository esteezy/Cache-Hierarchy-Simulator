//
// Created by Evan Mason on 9/4/20.
//
#ifndef SIM_CACHE_H
#define SIM_CACHE_H
#include <cmath>
#include <list>
using std::list;

typedef struct cache_params{
    unsigned long int block_size;
    unsigned long int l1_size;
    unsigned long int l1_assoc;
    unsigned long int vc_num_blocks;
    unsigned long int l2_size;
    unsigned long int l2_assoc;
}cache_params;

// Cache Object to instantiate one level of the memory hierarchy
// 
class Cache{
    private:
    class Cache_Block{
        public:
        unsigned int tag;
        unsigned int address;
        int LRU_counter;
        int valid_bit;
        char dirty_bit;
        Cache_Block(){
            tag = 0;
            LRU_counter = 0;
            valid_bit = 0;
            dirty_bit = ' ';
        }; // block constructor 
    };
    //cache specs
    unsigned int blocks;
    unsigned int sets;
    int index_bit_size;
    int block_offset_bit_size;
    int assoc;

    //cache measurements
    int numReads;
    int numReadMisses;
    int numWrites;
    int numWriteMisses;
    int numSwapRequests;
    double swapRequestRate;
    int numSwaps;
    double combinedMissRate;
    int numWriteBacks;

    //contents array - 2D array to hold blocks, accessible by index (array index)
    Cache_Block ** cache_array;
    //pointer to next level in mem hierarchy
    Cache * next_level;

    public:
    Cache(int cache_size, int block_size, int associativity) : numReads(0), numReadMisses(0), numWrites(0), numWriteMisses(0),
        numSwapRequests(0), swapRequestRate(0.0), numSwaps(0), combinedMissRate(0.0), numWriteBacks(0){
        next_level = NULL;
        assoc = associativity;
        blocks = block_size;
        if(cache_size == 0){sets = 0;}
        else {sets = (cache_size) / (associativity * block_size);}
        block_offset_bit_size = log2(blocks);
        index_bit_size = log2(sets);
        cache_array = new Cache_Block * [sets]; //create array - length in sets
        for(unsigned int i = 0; i < sets; i++) {
            int LRU_init = 0;
            cache_array[i] = new Cache_Block[associativity]; //create array - width in assoc
            // initialize LRU counters
            for(int j = 0; j<associativity; j++){
                cache_array[i][j].LRU_counter = LRU_init;
                LRU_init++;
            }
        }
    }; // cache level constructor

    void add_level(Cache &L2){
        next_level = &L2;
    }
    void request(char request_type, unsigned long addr);
    bool cache_search(char request_type, unsigned long addr);
    void block_insert(char request_type, unsigned long addr);
    bool handle_WB(char request_type, unsigned long addr);

    // get cache level specs
    int getReads(){return numReads;}
    int getReadMisses(){return numReadMisses;}
    int getWrites(){return numWrites;}
    int getWriteMisses(){return numWriteMisses;}
    int getSwapRequests(){return numSwapRequests;}
    double getSwapRate(){return swapRequestRate;}
    int getNumSwaps(){return numSwaps;}
    double getMissRate(bool L2_active) {
        if(next_level == NULL && L2_active){
            combinedMissRate = float(numReadMisses)/float (numReads);
        }
        else {
            combinedMissRate = float(numReadMisses + numWriteMisses) / float(numReads + numWrites);
        }
        if(isnan(combinedMissRate)){
            combinedMissRate = 0;
        }
        return combinedMissRate;
    }
    int getWritebacks(){return numWriteBacks;}
    int getMemoryTraffic(){return numWriteBacks+numReadMisses+numWriteMisses;};

    //print contents
    void print_contents();

};


// perform request
void Cache::request(char request_type, unsigned long int addr) {
    // search current cache
    if(!cache_search(request_type, addr)){
        if(request_type == 'w'){
            numWriteMisses++;
        }
        else{
            numReadMisses++;
        }
        handle_WB(request_type, addr);
        if(next_level != NULL){
            next_level->request('r', addr);
        }
        block_insert(request_type, addr);
    }
}

//determine if WB is necessary, if so, make space
bool Cache::handle_WB(char request_type, unsigned long addr) {
    unsigned int index = 0;
    index = (addr>>(block_offset_bit_size))&(sets-1);
    int LRU_block = assoc - 1;
    //LRU policy
    for(int j = 0; j<assoc; j++){
        if(cache_array[index][j].LRU_counter == LRU_block){
            if(cache_array[index][j].dirty_bit == 'D'){
                numWriteBacks++;
                if(next_level != NULL) {
                    next_level->request('w', cache_array[index][j].address);
                }
                cache_array[index][j].dirty_bit = ' ';
                return true;
            }
        }
    }
    return false;
}

//determine cache hit(evals true), or miss (evals false)
bool Cache::cache_search(char request_type, unsigned long addr) {
    unsigned int tag = 0;
    unsigned int index = 0;
    tag = addr>>(index_bit_size+block_offset_bit_size);
    index = (addr>>(block_offset_bit_size))&(sets-1);
    if(request_type == 'w'){
        numWrites++;
    }
    else{
        numReads++;
    }
    // cache search
    for(int j = 0; j<assoc; j++){
        //cache hit
        if((cache_array[index][j].tag == tag) && (cache_array[index][j].valid_bit == 1)){
            // LRU update
            int old_LRU = cache_array[index][j].LRU_counter;
            cache_array[index][j].LRU_counter = -1;
            for(int block = 0; block<assoc; block++){
                if(cache_array[index][block].LRU_counter < old_LRU){
                    cache_array[index][block].LRU_counter++;
                }
            }
            //WB Policy
            if(request_type == 'w'){
                cache_array[index][j].dirty_bit = 'D';
            }
            return true;
        }
    }
    // cache miss
    return false;
}

void Cache::block_insert(char request_type, unsigned long addr) {
    unsigned int tag = 0;
    unsigned int index = 0;
    tag = addr>>(index_bit_size+block_offset_bit_size);
    index = (addr>>(block_offset_bit_size))&(sets-1);
    int LRU_block = assoc - 1;
    //LRU policy
    for(int j = 0; j<assoc; j++){
        if(cache_array[index][j].LRU_counter == LRU_block){
            cache_array[index][j].LRU_counter = -1;
            cache_array[index][j].address = addr;
            cache_array[index][j].valid_bit = 1;
            cache_array[index][j].tag = tag;
            //WB Policy - bring in new block
            if(request_type == 'w'){
                cache_array[index][j].dirty_bit = 'D';
            }
            else{
                cache_array[index][j].dirty_bit = ' ';
            }
        }
        cache_array[index][j].LRU_counter++;
    }
}

//print contents
void Cache::print_contents() {
    for(unsigned int set = 0; set<sets; set++){
        printf("  set   %d:", set);
        int MRU = 0;
        int block = 0;
        for(block = 0; block<assoc; block++){
            if(cache_array[set][block].LRU_counter == MRU){
                printf("   %x  %c", cache_array[set][block].tag, cache_array[set][block].dirty_bit);
                MRU++;
                block = -1;
            }
        }
        printf("\n");
    }
}

#endif
