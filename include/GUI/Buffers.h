#include "Renderer.h"

#ifndef RENDERER_BUFFERS_H
#define RENDERER_BUFFERS_H

// Might not be used //
#define DEBUG 0
#define DEBUG_PREFIX "[OPENGL Buffers]\t"
#if DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif
// End of might not be used //

#define GLBUFBind(t, id) GLCall(glBindBuffer(GL_ARRAY_BUFFER+t, id))
#define IndexBufBind(ib) GLBUFBind(1, ib.id)
#define VertexBufBind(vb) GLBUFBind(0, vb.id)
#define IndexBufUnBind() GLBUFBind(1, 0)
#define VertexBufUnBind() GLBUFBind(0, 0)

#define init_glBufFunc(name, typ, btyp) typ name(void * data, unsigned size, char type)\
{\
	typ r = {0, size, NULL};\
	if(type == 4) r.mem = CallocOrDie(size, 4);\
	GLCall(glGenBuffers(1, &r.id));\
	GLBUFBind(btyp, r.id);\
	GLCall(glBufferData(GL_ARRAY_BUFFER+btyp, (size) * 4, data, GL_STATIC_DRAW+type));\
	return r;\
}

struct GLBUF_STRUCT {
	unsigned id, count;
	void* mem;
};
// Vertex Buffer //

typedef struct {
	unsigned id,
		 count;
	float* mem;
} VertexBuffer;

#define init_VertexBuf(data, size)  _init_VertexBuf(data, size, 0)
#define init_DVertexBuf(size) 	   _init_VertexBuf(NULL, size, 4)
init_glBufFunc(_init_VertexBuf, VertexBuffer, 0)

// Index Buffer //

typedef struct {
	unsigned id,
		 count,
		 *mem;
} IndexBuffer;

#define init_IndexBuf(data, size)  _init_IndexBuf(data, size, 0)
#define init_DIndexBuf(size) 	_init_IndexBuf(NULL, size, 4)
init_glBufFunc(_init_IndexBuf, IndexBuffer, 1)


void SendBufData(void* data, unsigned int size, char type) {
	ASSERT(data != NULL);
	ASSERT(size != 0);
	// If this function fails, please check how much dynamic memory you allocated //
	GLCall(glBufferSubData(GL_ARRAY_BUFFER+type, 0, size, data));
}

#define SendVBData(data, count) SendBufData(data, (count)*4, 0)
#define SendIBData(data, count) SendBufData(data, (count)*4, 1)

void GLBUF_Delete(struct GLBUF_STRUCT *b)
{
	GLCall(glDeleteBuffers(1, &b->id));
	freeOrDie(b->mem);
	b->mem = NULL;
}

#define VertexBuf_Delete(vb) GLBUF_Delete((struct GLBUF_STRUCT*)(&vb))
#define IndexBuf_Delete(ib) GLBUF_Delete((struct GLBUF_STRUCT*)(&ib))

// Vertex Array //

typedef unsigned int VertexArray;

#ifdef __MACH__

VertexArray init_VertexArray() {
	VertexArray va;
	GLCall(glGenVertexArraysAPPLE(1, &va));
	return va;
}

#define VertexArrBind(va) GLCall(glBindVertexArrayAPPLE(va));
#define VertexArrUnBind() GLCall(glBindVertexArrayAPPLE(0));
#define VertexArray_Delete(va) GLCall(glDeleteVertexArraysAPPLE(1, &va));

#else
VertexArray init_VertexArray() {
	VertexArray va;
	GLCall(glGenVertexArrays(1, &va));
	return va;
}

#define VertexArrBind(va) GLCall(glBindVertexArray(va));
#define VertexArrUnBind() VertexArrBind(0);
#define VertexArray_Delete(va) GLCall(glDeleteVertexArrays(1, &va));

#endif

typedef struct {
	long long type : 32;
	long long size : 4;
	long long normalised : 4;
}glinfo;

glinfo getGLInfo(char type) {
	switch(type) {
		case 'f': return (glinfo){GL_FLOAT, 4, 0};
		case 'i': return (glinfo){GL_UNSIGNED_INT, 4, 0};
		case 'c': return (glinfo){GL_UNSIGNED_BYTE, 1, 1};
		case 'd': return (glinfo){GL_DOUBLE, 8, 0};
	}
	ASSERT(false);
	return (glinfo){};
}

unsigned int getSizeofType(char type) {
	switch(type) {
		case 'f': return 4;
		case 'i': return 4;
		case 'c': return 1;
		case 'd': return 8;
		default:
			  printf(DEBUG_PREFIX"getSizeofType unexpected type {%c/%d} given\n",type, type);
			  break;
	}
	ASSERT(false);
	return 0;
}

void AddBufferStr(const VertexArray va, const VertexBuffer vb, const char* layout) {
	VertexArrBind(va);
	VertexBufBind(vb);
	unsigned long long offset = 0;
	unsigned stride = 0;

	debug("layout = {%s}\n", layout);

	for(unsigned i = 0; layout[i] != '\0'; i+=2) { 
		stride += (layout[i] - '0') * getSizeofType(layout[i+1]);
		if(layout[i+2] == ' ' || layout[i+2] == '\t') i++;
	}
	
	debug("Starting to add buffers\n");
	
	for(unsigned i = 0, j = 0; layout[i] != '\0'; i+=2,j++) {
		glinfo info = getGLInfo(layout[i+1]);
		int count = layout[i] - '0';
		GLCall(glEnableVertexAttribArray(j));

	debug("AttribArray = %d, Count = %d, type = %d, normalised = %d\n,stride = %u, offset = %lu\n", 
		j,
		count,
		info.type,
		info.normalised,
		stride,
		offset);

		GLCall(glVertexAttribPointer(j, count, 
					info.type, 
					info.normalised, 
					stride, 
					(const void*)offset));
		offset += count * info.size;
		if(layout[i+2] == ' ' || layout[i+2] == '\t') i++;
		//if(!(layout[i+2] ^ ' ' ^ layout[i+2] ^ '\t')) i++;
	}
	VertexBufUnBind();
	VertexArrUnBind();
}

#undef debug
#undef DEBUG
#undef DEBUG_PREFIX
#endif
