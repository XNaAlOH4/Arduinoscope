/* Author: Brian Tjung
 * Date: 3 June 2023
 * Refactoring my memory library
 * This Library helps to add error handling to
 * the allocation functions in stdlib
 * as well as adds new functions to do more copy techniques
 *
 * Ideas: I am thinking about adding memory alloc tracking to be able to detect
 * memory leaks if any so that I can prevent any future seg faults
 * and so that I can maybe try to optimise space in my future code
 *
 */
#ifndef MEMORY_H
#define MEMORY_H

#define TRACK 0
#define DEBUG 0
#define DEBUG_PREFIX "[MEMORY ERROR]\t"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define MAX_UINT ((1ULL << 32)-1)

unsigned calcAlloclen(unsigned len) {
        if(len <= 0x11) return 0x21;
        return (len&0xfff0) + (((len&0xf) > 8)? 0x21:0x11);
}

unsigned readAlloclen(char * p) {
	return *(unsigned*)(p-8);
}

#if TRACK

// Memory Tracking Variables //
char** ALL_POINTERS;
unsigned * ALL_P_LENGTHS;
unsigned ALL_P_LEN, ALL_P_MAX;
// END OF VARIABLES //

void init_MEM_TRACK() {
	ALL_P_LEN = 0;
	ALL_P_MAX = 8;
	ALL_POINTERS = NULL;
	ALL_P_LENGTHS = NULL;
	ALL_POINTERS = malloc(ALL_P_MAX * sizeof(char*));
	ALL_P_LENGTHS = malloc(ALL_P_MAX * sizeof(unsigned));
	if(ALL_POINTERS == NULL || ALL_P_LENGTHS == NULL) {
		printf(DEBUG_PREFIX "Failed to initialise Pointers for Memory Tracking\n");
		exit(-1);
	}
}

void MEM_TRACK_add(char * p, unsigned len) {
	if(!ALL_P_MAX)printf(DEBUG_PREFIX"Remember to init_MEM_TRACK\n");
	ALL_POINTERS[ALL_P_LEN] = p;
	ALL_P_LENGTHS[ALL_P_LEN++] = len;
	if(ALL_P_LEN >= ALL_P_MAX) {
		ALL_P_MAX *= 2;
		ALL_POINTERS = realloc(ALL_POINTERS, ALL_P_MAX * sizeof(char*));
		ALL_P_LENGTHS = realloc(ALL_P_LENGTHS, ALL_P_MAX * sizeof(unsigned));
	}
}

int MEM_TRACK_getAddr(char * p)
{
	for(int i = 0; i < ALL_P_LEN; i++) {
		if(ALL_POINTERS[i] == p) {
			return i;
		}
	}
	printf("MEM_TRACK get_Addr Error\n");
	return -1;
}

void MEM_TRACK_update(char * old_p, char * new_p, unsigned new_l)
{
	int index = MEM_TRACK_getAddr(old_p);
	if(index == -1) {
		printf(DEBUG_PREFIX "Old Memory Address not found\n");
		exit(-1);
	}
	ALL_POINTERS[index] = new_p;
	ALL_P_LENGTHS[index] = new_l;
}

void MEM_TRACK_print()
{
	printf("\nStart Track Print:\n");
	if(ALL_P_LEN == 0) {
		printf("No allocated memory stored\n\n");
		return;
	}

	for(int i = 0; i < ALL_P_LEN; i++) {
		printf("[%p: %u]\n", ALL_POINTERS[i], ALL_P_LENGTHS[i]);
	}
	printf("\nEnd Track Print:\n\n");
}

#define freeOrDie(p) _freeOrDie((char*)(p))
void _freeOrDie(char * p) {
	if(!p) return;
	int index = MEM_TRACK_getAddr(p);

	if(index == -1) {
		printf(DEBUG_PREFIX "Freeing Memory Address not found\n");
		exit(-1);
	}
	free(p);
	if(index != ALL_P_LEN) {
		unsigned len = ALL_P_MAX - index;
		memmove(ALL_POINTERS+index, ALL_POINTERS+index+1, sizeof(char*) * len);
		memmove(ALL_P_LENGTHS+index, ALL_P_LENGTHS+index+1, sizeof(unsigned)*len);
		ALL_POINTERS[index+len] = NULL;
		ALL_P_LENGTHS[index+len] = 0;
	}
	ALL_P_LEN--;
}

void MEM_TRACK_delete() {
	for(int i = 0; i < ALL_P_LEN; i++) {
		free(ALL_POINTERS[i]);
	}
	free(ALL_POINTERS);
	free(ALL_P_LENGTHS);
	ALL_POINTERS = NULL;
	ALL_P_LENGTHS = NULL;
	ALL_P_LEN = ALL_P_MAX = 0;
}

void MEM_TRACK_check()
{
	int error = 0;
	for(int i = 0; i < ALL_P_LEN; i++) {
		if(readAlloclen(ALL_POINTERS[i]) != calcAlloclen(ALL_P_LENGTHS[i])) {
			error = 1;
			printf(DEBUG_PREFIX "Uh Oh Corruption at Memory Addr (%p, %u,%u,%u)\n",ALL_POINTERS[i], readAlloclen(ALL_POINTERS[i]), calcAlloclen(ALL_P_LENGTHS[i]), ALL_P_LENGTHS[i]);
		}
	}
	if(error) exit(-1);
}
#else
#define init_MEM_TRACK()
#define MEM_TRACK_print()
#define MEM_TRACK_delete()
#define MEM_TRACK_check()
#define freeOrDie(x) if(x){free(x);x=NULL;}
#endif

/* copyObj is meant to copy a variable length 
 * object (src) to another object (buf) of same size
 */
#define copyObj(buf, src) memcpy(&(buf), &(src), sizeof(src))
#define copyArr(buf, src, len) memcpy(buf, src, sizeof(*(src)) * len)

/* setArr is meant to set all values in an array of type "type"
 * with an object of type "type"
 */
#define setArr(type, buf, val, len) {type tmpValue = val; _setArr((char*)(buf), (char*)&tmpValue, ((uint64_t)sizeof(type) << 32) | len);}
extern void _setArr(char * buf, const char * src, uint64_t ssize);
extern void DupArr(char* buf, unsigned dist, unsigned len);

void _setArr(char * buf, const char * src, uint64_t ssize)
{
	unsigned len = ssize & MAX_UINT;
	ssize >>= 32;

	for(int i = 0; i < len; i++) {
		memcpy(buf+i*ssize, src, ssize);
	}
}

void DupArr(char* buf, unsigned dist, unsigned len) {
	char stop = 0;
	unsigned cpy_len = 0;
	char * cpy_fr = buf - dist;
	while(!stop) {
		if(len <= dist) {
			stop++;
			cpy_len = len;
		}else
			cpy_len = dist;
		memmove(buf, cpy_fr, cpy_len);
		len -= dist;
		buf += cpy_len;
		dist += cpy_len;
	}
}

#define AMALARG ((uint64_t)__LINE__ << 32)

#define MallocOrDie(size)	AllocOrDie(__FILE__, AMALARG | (unsigned)0x614d, (unsigned)(size))
#define CallocOrDie(count,size)	AllocOrDie(__FILE__, AMALARG | (unsigned)0x6143, count, (unsigned)(size))
#define ReallocOrDie(buf, size)	AllocOrDie(__FILE__, AMALARG | (unsigned)0x616552, buf, (unsigned)(size))


void* AllocOrDie(const char * file, uint64_t amalg, ...)
{
	va_list ptr;
	va_start(ptr, amalg);
	
	int line = amalg >> 32;
	const char* func = (char*)&amalg;
	amalg &= MAX_UINT;

	void * buffer = NULL, *old_buffer = NULL;
	size_t size;
	unsigned count = 1;

	switch(func[0]) {
		case 'M':
			buffer = malloc(size = va_arg(ptr, uint64_t));
			break;
		case 'C':
			buffer = calloc(count = va_arg(ptr, uint64_t), size = va_arg(ptr, unsigned int));
			size *= count;
			break;
		case 'R':
			old_buffer = (void *)(va_arg(ptr, uint64_t));
			buffer = realloc(old_buffer, size = va_arg(ptr, size_t));
			break;
	}

	va_end(ptr);

	if(!buffer)
		printf("[%slloc Error -> Size:(%zu), File:%s, Line:%d\n]", func, size, file, line);

#if TRACK
	if(func[0] == 'R')
		MEM_TRACK_update(old_buffer, buffer, size);
	else
		MEM_TRACK_add(buffer, size);
#endif

	return buffer;
}

#undef TRACK
#undef DEBUG
#undef DEBUG_PREFIX
#endif
