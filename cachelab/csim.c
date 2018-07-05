#include "cachelab.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <getopt.h>
#include <math.h>

typedef unsigned long long int mem_addr;

typedef struct {
	int s;
	int b;
	int S;
	int E;
	int B;

	int hits;
	int misses;
	int evicts;
} cache_param;

typedef struct {
	int recentUsed;
	int valid;
	mem_addr tag;
	char *block;
} block;

typedef struct {
	block *blocks;
} set;

typedef struct {
	set *sets;
} cache;

//return index if the requested block is empty, unless -1
int emptyBlock(set my_set, cache_param cache_parameter) {
	int num_blocks = cache_parameter.E;
	int index;
	block my_block;

	for (index = 0; index < num_blocks; index++) {
		my_block = my_set.blocks[index];
		if (my_block.valid == 0) {
			return index;
		}
	}
	return -1;
}

//return which block to evict
int evictBlock(set my_set, cache_param cache_parameter, int *usedBlock) {
	int num_blocks = cache_parameter.E;
	int maxFreq = my_set.blocks[0].recentUsed;
	int minFreq = my_set.blocks[0].recentUsed;
	int minFreqIndex = 0;
	int blockIndex;

	for (blockIndex = 1; blockIndex < num_blocks; blockIndex++) {
		if (minFreq > my_set.blocks[blockIndex].recentUsed) {
			minFreqIndex = blockIndex;
			minFreq = my_set.blocks[blockIndex].recentUsed;
		}
		if (maxFreq < my_set.blocks[blockIndex].recentUsed) {
			maxFreq = my_set.blocks[blockIndex].recentUsed;
		}
	}

	usedBlock[0] = minFreq;
	usedBlock[1] = maxFreq;
	return minFreqIndex;
}

//clear the cache
void clear(cache old_cache, long long num_sets, int num_blocks, long long block_size) {
	int setIndex;

	for (setIndex = 0; setIndex < num_sets; setIndex++) {
		set cache_set = old_cache.sets[setIndex];

		if (cache_set.blocks != NULL) {
			free(cache_set.blocks);
		}
	}
	if (old_cache.sets != NULL) {
		free(old_cache.sets);
	}
}

cache_param cacheAccess(cache my_cache, cache_param cache_parameter, mem_addr addr) {
	int blockIndex;
	int isCacheFull = 1;
	int num_blocks = cache_parameter.E;
	int num_hits = cache_parameter.hits;
	int tagSize = 64 - (cache_parameter.s + cache_parameter.b);
	mem_addr tag = addr >> (64 - tagSize);
	unsigned long long setIndex = (addr << tagSize) >> (cache_parameter.b + tagSize);

	set my_set = my_cache.sets[setIndex];

	for (blockIndex = 0; blockIndex < num_blocks; blockIndex++) {
		if (my_set.blocks[blockIndex].valid == 1) {
			if (my_set.blocks[blockIndex].tag == tag) {
				cache_parameter.hits++;
				my_set.blocks[blockIndex].recentUsed++;
			}
		} else if ((my_set.blocks[blockIndex].valid == 0) && isCacheFull) {
			isCacheFull = 0;
		}
	}

	if (num_hits == cache_parameter.hits) {
		cache_parameter.misses++;

		int *usedBlock = (int *) malloc(sizeof(int) * 2);
		int minFreqIndex = evictBlock(my_set, cache_parameter, usedBlock);

		if (isCacheFull == 1) {
			cache_parameter.evicts++;
			my_set.blocks[minFreqIndex].tag = tag;
			my_set.blocks[minFreqIndex].recentUsed = usedBlock[1] + 1;
		} else {
			int emptyIndex = emptyBlock(my_set, cache_parameter);
			my_set.blocks[emptyIndex].tag = tag;
			my_set.blocks[emptyIndex].valid = 1;
			my_set.blocks[emptyIndex].recentUsed = usedBlock[1] + 1;
		}
		free(usedBlock);
	}
	return cache_parameter;
}

long long getpow(int exp) {
	long long pow = 1;
	pow = pow << exp;
	return pow;
}
	
int main(int argc, char **argv)
{
	cache new_cache;
	cache_param cache_parameter;
	bzero(&cache_parameter, sizeof(cache_parameter));
	long long num_sets;
	long long blockSize;

	FILE *open_trace;
	char trace_cmd;
	mem_addr addr;
	int size;

	char *trace_file;
	char c;

	while ((c = getopt(argc, argv, "s:E:b:t:vh")) != -1) {
		switch (c) {
			case 's':
				cache_parameter.s = atoi(optarg);
				break;
			case 'E':
				cache_parameter.E = atoi(optarg);
				break;
			case 'b':
				cache_parameter.b = atoi(optarg);
				break;
			case 't':
				trace_file = optarg;
				break;
			case 'h':
				exit(0);
			default:
				exit(1);
		}
	}

	num_sets = pow(2.0, cache_parameter.s);
	blockSize = getpow(cache_parameter.b);
	cache_parameter.hits = 0;
	cache_parameter.misses = 0;
	cache_parameter.evicts = 0;
	
	set cache_set;
	block cache_block;
	
	// build a new cache
	new_cache.sets = (set *) malloc(sizeof(set) * num_sets);

	for (int setIndex = 0; setIndex < num_sets; setIndex++) {
		cache_set.blocks = (block *) malloc(sizeof(cache_block) * cache_parameter.E);
		new_cache.sets[setIndex] = cache_set;

		for (int blockIndex = 0; blockIndex < cache_parameter.E; blockIndex++) {
			cache_block.recentUsed = 0;
			cache_block.valid = 0;
			cache_block.tag = 0;
			cache_set.blocks[blockIndex] = cache_block;
		}
	}

	open_trace = fopen(trace_file, "r");
	if (open_trace != NULL) {
		while (fscanf(open_trace, " %c %llx,%d", &trace_cmd, &addr, &size) == 3) {
			switch (trace_cmd) {
				case 'I' :
					break;
				case 'L':
					cache_parameter = cacheAccess(new_cache, cache_parameter, addr);
					break;
				case 'S':
					cache_parameter = cacheAccess(new_cache, cache_parameter, addr);
					break;
				case 'M':
					cache_parameter = cacheAccess(new_cache, cache_parameter, addr);
					cache_parameter = cacheAccess(new_cache, cache_parameter, addr);
					break;
				default:
					break;
			}
		}
	}
	printSummary(cache_parameter.hits, cache_parameter.misses, cache_parameter.evicts);
	clear(new_cache, num_sets, cache_parameter.E, blockSize);
	fclose(open_trace);
    	return 0;
}
