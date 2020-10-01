//
// Created by Evan Mason on 9/4/20.
// ECE 463 - Project 1
// Cache Design & Memory Hierarchy Design
//
#include <cstdio>
#include <cstdlib>
#include "sim_cache.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim_cache 32 8192 4 7 262144 8 gcc_trace.txt
    argc = 8
    argv[0] = "sim_cache"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    cache_params params;    // look at sim_cache.h header file for the the definition of struct cache_params
    char rw;                // variable holds read/write type read from input file. The array size is 2 because it holds 'r' or 'w' and '\0'. Make sure to adapt in future projects
    unsigned long int addr; // Variable holds the address read from input file

    if(argc != 8)           // Checks if correct number of inputs have been given. Throw error and exit if wrong
    {
        printf("Error: Expected inputs:7 Given inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    params.block_size       = strtoul(argv[1], NULL, 10);
    params.l1_size          = strtoul(argv[2], NULL, 10);
    params.l1_assoc         = strtoul(argv[3], NULL, 10);
    params.vc_num_blocks    = strtoul(argv[4], NULL, 10);
    params.l2_size          = strtoul(argv[5], NULL, 10);
    params.l2_assoc         = strtoul(argv[6], NULL, 10);
    trace_file              = argv[7];

    //Create Cache Memory Hierarchy
    bool L2_active = false;
    Cache cache_L1(params.l1_size, params.block_size, params.l1_assoc);
    Cache cache_L2(params.l2_size, params.block_size, params.l2_assoc);
    if(params.l2_size != 0){
        L2_active = true;
        cache_L1.add_level(cache_L2);
    }

    // Open trace_file in read mode

    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    // Print params
    printf("===== Simulator configuration =====\n"
           "  BLOCKSIZE:                        %lu\n"
           "  L1_SIZE:                          %lu\n"
           "  L1_ASSOC:                         %lu\n"
           "  VC_NUM_BLOCKS:                    %lu\n"
           "  L2_SIZE:                          %lu\n"
           "  L2_ASSOC:                         %lu\n"
           "  trace_file:                       %s\n", params.block_size, params.l1_size, params.l1_assoc, params.vc_num_blocks, params.l2_size, params.l2_assoc, trace_file);

    char str[2];
    while(fscanf(FP, "%s %lx", str, &addr) != EOF)
    {
        rw = str[0];
        cache_L1.request(rw, addr);
    }
    fclose(FP);

    //print contents
    printf("\n===== L1 contents =====\n");
    cache_L1.print_contents();
    if(L2_active){
        printf("\n===== L2 contents =====\n");
        cache_L2.print_contents();
    }


    //print sim results
    printf("\n===== Simulation results =====\n");
    printf("  a. number of L1 reads:                          %d\n", cache_L1.getReads());
    printf("  b. number of L1 read misses:                    %d\n", cache_L1.getReadMisses());
    printf("  c. number of L1 writes:                         %d\n", cache_L1.getWrites());
    printf("  d. number of L1 write misses:                   %d\n", cache_L1.getWriteMisses());
    printf("  e. number of swap requests:                     %d\n", cache_L1.getSwapRequests());
    printf("  f. swap request rate:                           %.4f\n", cache_L1.getSwapRate());
    printf("  g. number of swaps:                             %d\n", cache_L1.getNumSwaps());
    printf("  h. combined L1+VC miss rate:                    %.4f\n", cache_L1.getMissRate(L2_active));
    printf("  i. number writebacks from L1/VC:                %d\n", cache_L1.getWritebacks());
    printf("  j. number of L2 reads:                          %d\n", cache_L2.getReads());
    printf("  k. number of L2 read misses:                    %d\n", cache_L2.getReadMisses());
    printf("  l. number of L2 writes:                         %d\n", cache_L2.getWrites());
    printf("  m. number of L2 write misses:                   %d\n", cache_L2.getWriteMisses());
    printf("  n. L2 miss rate:                                %.4f\n", cache_L2.getMissRate(L2_active));
    printf("  o. number of writebacks from L2:                %d\n", cache_L2.getWritebacks());
    if(L2_active){
        printf("  p. total memory traffic:                        %d\n", cache_L2.getMemoryTraffic());
    }
    else{
        printf("  p. total memory traffic:                        %d\n", cache_L1.getMemoryTraffic());
    }

    return 0;
}

