#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdint.h>
#include <math.h>
#include "Memory.h"

#define HASH_MAP_SIZE 2048

typedef char* hash_base;
#define hash_map(type) type*

hash_base *init_hashMap_(size_t size) {
	hash_base * hb = MallocOrDie(sizeof(hash_base));
	hb = CallocOrDie(HASH_MAP_SIZE + 1, size);
	
	switch(size) {
		case 1: {
			setArr(char, hb,(char)-1, HASH_MAP_SIZE);
		}break;
		case 2: {
			setArr(short, hb,(short)-1, HASH_MAP_SIZE);
		}break;
		case 4: {
			setArr(int,hb,-1, HASH_MAP_SIZE);
		}break;
		case 8: {
			setArr(long,hb,-1l, HASH_MAP_SIZE);
		}break;
	}

	return hb;
}
#define init_hashMap(type) (hash_map(type)) init_hashMap_(sizeof(type))

#define hash(key) str_hash(key)
uint32_t str_hash(char* key) {
	uint32_t hashVal = 0;
	while(*key != '\0') {
		hashVal = (hashVal << 4) + *(key++);
		long g = hashVal & 0xF0000000L;
		if(g != 0) hashVal ^= g >> 24;
		hashVal &= ~g;
	}
	return hashVal % HASH_MAP_SIZE;
}

uint32_t java_hash_(char* key, size_t size) {
	uint32_t result = 0;
	uint64_t offset = (uint64_t)key;
	const uint32_t h1 = 31;

	for(size_t i = 0; i < size; i++) {
		result = key[offset++ % size] + h1 * result;
	}
	
	return result % HASH_MAP_SIZE;
}

uint32_t myhash_(char * key, size_t size) {
	uint32_t result = 0;
	const uint32_t h1 = 31;

	for(size_t i = 0; i < size; i++) {
		result += key[size-i-1] + h1 * result;
		// result = result * 1001 + offset | (result ^ key[i]);
	}
	
	return result % HASH_MAP_SIZE;
}

#define hash_push(hm, key, data) hm[hash(key)] = data
#define hash_get(hm, key) hm[hash(key)]

#endif
