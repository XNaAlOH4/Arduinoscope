#ifndef RENDERER_MODEL_H
#define RENDERER_MODEL_H

#include "mesh.h"
#include "Buffers.h"

typedef struct {
	VertexArray va;
	VertexBuffer vb;
	IndexBuffer ib;
} Model;

void Model_Delete(Model m) {
	VertexArray_Delete(m.va);
	VertexBuf_Delete(m.vb);
	IndexBuf_Delete(m.ib);
}

SU_mesh loadMesh(const char *data) {
	SU_mesh m = {NULL, {0,0}};
	char* nums = countMesh(data, m.size),*num = nums;
	
	unsigned size = 0;
	for(int i = 0; i < 6; i++) {
		size += m.size[i];
	}
	m.mem = CallocOrDie(4, size+1);
	
	for(int i = 0; i < m.size[MESH_VERT]; i++) {
		m.mem[i] = fstr(&num);
		num++;
	}

	unsigned * mem = (unsigned*)m.mem+m.size[MESH_VERT];
	for(int i = 0; i < m.size[MESH_FACE]; i++) {
		mem[i] = istr(&num);
		num++;
	}

	freeOrDie(nums);
	return m;
}

Model loadModel(const char* data, const char* format) {
	Model m;
	SU_mesh mesh = loadMesh(data);
	m.va = init_VertexArray();
	m.vb = init_VertexBuf(mesh.mem, mesh.size[MESH_VERT]);
	m.ib = init_IndexBuf((unsigned*)mesh.mem+mesh.size[MESH_VERT], mesh.size[MESH_FACE]);

	freeOrDie(mesh.mem);
	AddBufferStr(m.va, m.vb, format);
	return m;
}

Model loadDModel(unsigned bsize, unsigned isize, const char* format)
{
	Model m;
	m.va = init_VertexArray();
	m.vb = init_DVertexBuf(bsize);
	m.ib = init_DIndexBuf(isize);

	AddBufferStr(m.va, m.vb, format);
	return m;
}

#endif
