# Cache-Hierarchy-Simulator
Trace-driven cache simulator, written in C++.

Simulates operation of an L1 or L1+L2 cache, with the option of adding additional levels in the memory hierarchy. 
Makefile creates a 'sim_cache' executable that takes 7 input arguments relating to block size, cache size, associativity, etc.

E.g. Running "./sim_cache 16 1024 4 0 0 0 gcc_trace.txt" runs the simulator as a 4-way set associative 1024B L1 cache with a 16B block size. Following zeros refer to the size (in Bytes) of the L1 victim cache, the L2 cache size (in Bytes), and the L2 associativity, respectively. 
Likewise, running "./sim_cache 32 2048 2 0 8192 4 gcc_trace.txt" creates a memory hierarchy simulator of a 2-way set associative 2KB L1 cache, AND a 4-way set associative 8KB L2 cache, with both levels in the memory hierarchy having a 32B block size.

Both the L1 and/or L2 caches utilize the Least-Recently-Used Replacement policy (LRU) and the Write-Back-Write-Allocate write policy (WBWA). 

Simulator is driven with traces that contain CPU requests, either write ('w') or read ('r') requests, with an associated 32-bit requested memory block.

Each Simulation ends with results displayed pertaining to L1 and/or L1+L2's operation. For example, results include: writes, reads, read misses, L1+VC miss rate, L1 and/or L2 writes, just to name a few. These results are critical in determining aspects of how well a certain cache memory hierarchy design operates for a given trace file. These results allow me to study how effective certain designs are by calculating Average Access Time (AAT), and Compulsory/Capacity/Conflict miss rates. 
