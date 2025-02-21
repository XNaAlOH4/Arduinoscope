#ifndef RENDERER_H
#define RENDERER_H

#ifndef CUSTOM_GUI_H
#include "GUI.h"
#endif

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define ASSERT(x) if(!(x)) exit(1);
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__))

//#define GLClearError() while(glGetError() != GL_NO_ERROR)
void GLClearError()
{
	while(glGetError() != GL_NO_ERROR);

}

bool GLLogCall(const char* function, const char* file, int line) 
{
	GLenum error;
	while( (error = glGetError()) )
	{
		printf("[OpenGL Error] (%d), (%s)\n", error, gluErrorString(error));
		printf("-> %s, file:%s, line%d\n",function, file, line);
		return false;
	}
	return true;
}

#include <Memory.h>
#include <string.h>
#include "Shader.h"
#include "Model.h"

typedef struct {
	int model_count, shader_count;
	Model * models;
	Shader *shaders;
} Renderer;

#define Renderer_Delete(r) _Renderer_Delete(&r)
void _Renderer_Delete(Renderer* r)
{
	for(int i = 0; i < r->model_count; i++)
		Model_Delete(r->models[i]);
	if(r->model_count)freeOrDie(r->models);
	for(int i = 0; i < r->shader_count; i++)
		Shader_Delete(r->shaders[i]);
	if(r->shader_count)freeOrDie(r->shaders);
	memset(r, 0, sizeof(*r));
}

Renderer init_Renderer(int m, int s) {
	Renderer r;
	if(m)r.models = CallocOrDie(sizeof(Model), m);
	if(s)r.shaders = CallocOrDie(sizeof(Shader), s);
	r.model_count = m;
	r.shader_count = s;
	return r;
}

/* Renderer_AddModel adds a model with mesh_data d at index i and format f*/
#define Renderer_AddModel(r, d, i, f) r.models[i] = loadModel(d, f)
#define Renderer_AddDModel(r, i, bs, is, f) r.models[i] = loadDModel(bs, is, f)

void RendererBind(Renderer r, int m, int s) {
	VertexArrBind(r.models[m].va);
	IndexBufBind(r.models[m].ib);
	ShaderBind(r.shaders[s]);
}

void RendererBindNS(Renderer r, int m) {
	VertexArrBind(r.models[m].va);
	IndexBufBind(r.models[m].ib);
}

void RendererUnBind() {
	VertexArrUnBind();
	ShaderUnBind();
	IndexBufUnBind();
}

#define BufferRender(m, opt) GLCall(glDrawArrays(opt, 0, m.vb.count))
#define BufferRenderInst(m, opt, inst) GLCall(glDrawArraysInstanced(opt, 0, m.vb.count, inst))
#define IndexRender(m, opt) GLCall(glDrawElements(opt, m.ib.count, GL_UNSIGNED_INT, NULL))
#define IndexRenderInst(m, opt, inst) GLCall(glDrawElementsInstanced(opt, m.ib.count, GL_UNSIGNED_INT, NULL, inst));

#define Renderer_Fill(r,m) IndexRender(r.models[m], GL_TRIANGLES)
#define Renderer_Draw(r,m) IndexRender(r.models[m], GL_LINE_LOOP)
#define Renderer_Lines(r,m) IndexRender(r.models[m], GL_LINES)
#define Renderer_Line(r,m) IndexRender(r.models[m], GL_LINE_STRIP)
#define Renderer_Dot(r,m) IndexRender(r.models[m], GL_POINTS)

#define Renderer_AFill(r,m) BufferRender(r.models[m], GL_TRIANGLES)
#define Renderer_ADraw(r,m) BufferRender(r.models[m], GL_LINE_LOOP)
#define Renderer_ALines(r,m) BufferRender(r.models[m], GL_LINES)
#define Renderer_ALine(r,m) BufferRender(r.models[m], GL_LINE_STRIP)
#define Renderer_ADot(r,m)  BufferRender(r.models[m], GL_POINTS)

#define Renderer_FillInstance(r,m,inst) IndexRenderInst(r.models[m], GL_TRIANGLES, inst)
#define Renderer_LinesInstance(r,m,inst) IndexRenderInst(r.models[m], GL_LINE_STRIP,inst)
#define Renderer_DotInstance(r,m,inst) BufferRenderInst(r.models[m], GL_POINTS,inst)

#endif

