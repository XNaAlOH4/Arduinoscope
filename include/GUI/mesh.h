#pragma once
// Just putting mesh here to sort out how to optimise this shit

// IDEAS FOR OPTIMISATION //
//1. Read all values from file into char * and 
//while doing that keep vertCount and faceCount
//so that memory can be allocated all at once 
//without so many reallocations, making setup potentially faster
//	
//	NOTE: this also eliminates the need for a cap variable
//
//2. Keep all values in the same pointer to reduce 1 pointer
//Thus, freeing is just freeing 1 pointer
//
//3. I could create a mesh optimised for startup and
//a mesh that can be updated during run time as separate structs
//to allow for creating of new meshes using an editor

//Start Up mesh
#include <str.h>

#define MESH_VERT 0
#define MESH_TEXX 1
#define MESH_NORM 2
#define MESH_PARA 3
#define MESH_FACE 4
#define MESH_LINE 5

typedef struct {
	float* mem;
	uint32_t size[6];
} SU_mesh;
/*
void mesh_skipdata(char** data)
{
	while(**data != 'v' || **data != 'f')
		(*data)++;
}

void mesh_save(char** data, char* c)
{
	unsigned len = (unsigned)(strchr(*data, '\n') - *data);
	memcpy(c, *data, len);
	*data += len;
	
#if FALSE
	int i;

	char* end = strchr(data, '\n');
	i = end - data;
	char* comment;
	if(comment = strchr(data, '/'))
		i = comment - data;
	c = realloc(c, len + --i);
	c = strncpy(c, data, i);
#endif
}

char getFPV(char * str) {
	char count = 0;
	while(*str != '\n') {
		skipToWS(&str);
		count++;
	}
	return count;
}



SU_mesh init_SUmesh(char* data, char v) {
	SU_mesh m;
	//TODO: get vert and face count
	char *vert, *face;
	unsigned v = 0, f = 0;
	while(*data)
	{
		mesh_skipdata(&data);
		switch(*data) {
			case 'v':
				m.size[0]++;
				mesh_save(data, vert);
				break;
			case 'f':
				m.size[1]++;
				mesh_save(data, face);
				break;
		}
	}

	// init mesh
	// v given will be number of floats per vert
	unsigned char fpv = getFPV(vert);
	m.p = CallocOrDie(m.size[0]*fpv + m.size[1] * 3, 4);

	//add verts
	int i = m.size[0];
	for(; i >= 0; i--)
	{
		for(int j = 0; j < fpv; j++)
			
	}
	//add faces
	while(*face)
	{
		switch(){}
	}

	free(verts);
	free(face);
	return m;
}*/

typedef struct {
	float* verts;
	uint32_t* index;
	uint32_t sizes[4];
}mesh;

mesh * init_mesh() {
	mesh * m = malloc(sizeof(mesh));
	m->verts = CallocOrDie(8, sizeof(float));
	m->index = CallocOrDie(8, sizeof(uint32_t));
	m->sizes[0] = 0; m->sizes[1] = 0;
	m->sizes[2] = 8; m->sizes[3] = 8;
	return m;
}

void meshpushV(mesh * m, float f) {
	if(m->sizes[0] == m->sizes[2]) {
		m->sizes[2] = m->sizes[2] << 1;
		m->verts = ReallocOrDie(m->verts, m->sizes[2] * sizeof(float));
	}
	m->verts[m->sizes[0]++] = f;
}

void meshpushF(mesh* m, unsigned int i) {
	if(m->sizes[1] == m->sizes[3]) { 
		m->sizes[3] = m->sizes[3] << 1;
		m->index = ReallocOrDie(m->index, m->sizes[3] * sizeof(int));
	}
	m->index[m->sizes[1]++] = i;
}

#ifndef MIN
#define MIN(a, b) ( (a) < (b) )? (a):(b)
#endif
#ifndef MAX
#define MAX(a, b) ( (a) > (b) )? (a):(b)
#endif
#include <stdarg.h>

void meshpushVerts(mesh * m, unsigned n, ...)
{
	va_list vl;
	va_start(vl, n);
	
	while(n) {
		int limit = MIN(m->sizes[2] - m->sizes[0], n);
		for(int i = 0; i < limit; i++, n--) {
			m->verts[m->sizes[0]++] = va_arg(vl, double);
		}
		if(m->sizes[0] == m->sizes[2]) {
			m->sizes[2] = m->sizes[2] << 1;
			m->verts = ReallocOrDie(m->verts, (m->sizes[2] * sizeof(float)) );
		}
	}

	va_end(vl);
}

void meshpushFaces(mesh * m, unsigned n, ...)
{
	va_list vl;
	va_start(vl, n);

	while(n) {
		for(int i = 0; i < MIN(m->sizes[3] - m->sizes[1], n); i++, n--) {
			m->index[m->sizes[1]++] = va_arg(vl, unsigned);
		}
		if(m->sizes[1] == m->sizes[3]) {
			m->index = ReallocOrDie(m->index, (m->sizes[3] *= 2) * sizeof(int));
		}
	}

	va_end(vl);
}

void printMesh(mesh *m, int stride) {
	printf("Verts:\n{");
	for(int i = 0; i < m->sizes[0]; i++) {
		printf("%f, ", m->verts[i]);
		if( (i % stride == (stride-1)) ) {
			printf("\n");
		}
	}

	printf("}\nIndexes:\n{");
	for(int i = 0; i < m->sizes[1]; i++) {
		printf("%u, ", m->index[i]);
		if(i % 3 == 2) {
			printf("\n");
		}
	}
	printf("}\n");
}

void printSU(SU_mesh m, int stride) {
	printf("Verts:\n{");
	for(int i = 0; i < m.size[0]; i++) {
		printf("%f, ", m.mem[i]);
		if( (i % stride == (stride-1)) ) {
			printf("\n");
		}
	}

	printf("}\nIndexes:\n{");
	for(int i = 0; i < m.size[1]; i++) {
		printf("%u, ", ((unsigned*)m.mem)[i + m.size[0]]);
		if(i % 3 == 2) {
			printf("\n");
		}
	}
	printf("}\n");
}

void delete_mesh(mesh * m) {
	freeOrDie(m->verts);
	freeOrDie(m->index);
	m->verts = NULL;
	m->index = NULL;
}

char* countMesh(const char* src, unsigned * lens) {
	char* data = (char*)src;
	char* nums = MallocOrDie(strlen(data));
	unsigned len = 0, elen;
	char * prev;

	while(1) {
		elen = 0;
		char e[3] = {data[0], data[1], 0};
		data+=2;
		while(*data != '\n' && *data != 0) {
			skip_whitespace(&data);
			prev = data;
			skipToWS(&data);
			memcpy(nums+len, prev, data-prev);
			len += (unsigned)(data-prev)+1;
			nums[len-1] = '\n';
			elen++;
		}
		switch(e[0]) {
			case 'v':
				switch(e[1]) {
					case ' ':
						lens[MESH_VERT] += elen;
						break;
					case 't':
						lens[MESH_TEXX] += elen;
						break;
					case 'n':
						lens[MESH_NORM] += elen;
						break;
					case 'p':
						lens[MESH_PARA] += elen;
						break;
				}
				break;
			case 'f':
				lens[MESH_FACE] += elen;
				break;
			case 'l':
				lens[MESH_LINE] += elen;
				break;

		}
		if(!*(data++)) break;
	}

	nums[len-1] = '\0';

	return nums;
}

