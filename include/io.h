#ifndef CUSTOM_IO_H
#define CUSTOM_IO_H

#include <Memory.h>

#define DEBUG 0
#define DEBUG_PREFIX "[IO ERROR]:\t"

#define FILE_ERROR 1

#if DEBUG
#include <errno.h>
#endif

//Standard Includes//
#include <stdio.h>
#include <stdlib.h>

struct c_File {
	char *end, *buffer;
};

FILE *openFile(const char * file, const char * arg)
{
	FILE * fp = fopen(file, arg);
	if(!fp) {
#if DEBUG
		switch(errno) {
			case 2:
				fprintf(stderr, DEBUG_PREFIX"File (%s) cannot be found\n", file);
				//Find out if directory exists

				//Find out if file exists
				break;
			case 13:
				fprintf(stderr, DEBUG_PREFIX"Permission to open File (%s) has been denied\n", file);
				break;
		}
#else
		fprintf(stderr, DEBUG_PREFIX " File (%s) cannot be opened\n", file);
#endif
		exit(FILE_ERROR);
	}

	return fp;
}

long fileSize(FILE * fp)
{
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return size;
}

struct c_File fileContent(const char * file)
{
	struct c_File f;
	FILE * fp = openFile(file, "rb");
	long fs = fileSize(fp);
       	f.end = (char*)(long long)fs;
	fclose(fp);
	fp = openFile(file, "rb");	
	f.buffer = MallocOrDie(fs);
	fread(f.buffer, 1, fs, fp);
	if(feof(fp) || ferror(fp)) {
		printf("Error ?? = %u\n", ferror(fp));
	}
	fclose(fp);
	f.end = f.buffer + fs;
	return f;
}

char * fileContent_str(const char * file)
{
	unsigned len;
	
	FILE * fp = openFile(file, "r");
	len = fileSize(fp);
	char * f = MallocOrDie(len+1);
	fread(f, 1, len, fp);
	fclose(fp);
	f[len] = '\0';

	return f;
}

void skipLine(char ** p) {
	while((**p) && (**p != '\n'))(*p)++;
	if(**p)(*p)++;
}

char* findEOL(char * p) {
	while(*p && *p != '\n')p++;
	if(*p)p++;
	return p;
}

char *saveLine(char ** p) {
	long eol = findEOL(*p) - *p;
	char *r = MallocOrDie(eol);
	memcpy(r, *p, eol);
	return r;
}

//char writeMem(char* s);

#undef DEBUG
#undef DEBUG_PREFIX

#endif
