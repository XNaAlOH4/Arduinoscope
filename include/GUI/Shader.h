#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "Renderer.h"
#include "hash_map.h"
#include <io.h>

#define DEBUG 0
#define DEBUG_PREFIX "[3D ENGINE SHADER ]\t"

typedef char** SHDR_PrgmSrc;
#define SHDR_PS_SIZE 2
#define init_SHDR_PS() (SHDR_PrgmSrc) MallocOrDie(SHDR_PS_SIZE * sizeof(char*))

void SHDR_PS_Delete(SHDR_PrgmSrc sps, int size)
{
	for(int i = 0; i < size; i++) {
		freeOrDie(sps[i]);
	}
	freeOrDie(sps);
}

void add_SHDR_PS(SHDR_PrgmSrc sps, int type, const char* src, int size) {
	if(type == -1) {return;}
	sps[type] = CallocOrDie(1, size + 1);
	memcpy(sps[type], src, size);
	sps[type][size] = '\0';
}

typedef struct SHADER_STRUCT {
	unsigned id;
	hash_map(int) m_UniformLocationCache;
}Shader;

#define SHDR_WS_CHK(c) ((c) == ' ') | ((c) == '\t') | ((c) == '\n')

SHDR_PrgmSrc parseShader(char* file) {
#if DEBUG
	printf("parseShader: {%s}\n", file);
#endif
	//char * file = fileContent_str(filepath);
	char * p = file, *p_init = file;

	//First, find out where the shader ends, meaning where another shader starts
	//I can use the "#shader" string to differentiate between the shaders

	//skip comments and whitespace
	//_SHDR_skip(&p);
	enum ShaderType {
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	}type;

	SHDR_PrgmSrc sps = init_SHDR_PS();
	type = NONE;

	while(*p) {
		if((*p == '#') & (p[1] == 's')) {
			add_SHDR_PS(sps, type, p_init, p-p_init);
			switch(p[8]) {
				case 'v':
					type = VERTEX;
					break;
				case 'f':
					type = FRAGMENT;
					break;
			}
			skipLine(&p);
			p_init = p;
			continue;
		}
		skipLine(&p);
	}
	add_SHDR_PS(sps, type, p_init, p-p_init-1);
#if DEBUG
	printf("vertex = {%s}\n", sps[0]);
	printf("fragment = {%s}\n", sps[1]);
#endif
	//freeOrDie(file);

	return sps;
}

unsigned int CompileShader(unsigned int type, const char* src) {
#if DEBUG
	printf("Compiling %s shader\n", (type == GL_VERTEX_SHADER)? "vertex":"fragment");
#endif
	GLCall(unsigned int id = glCreateShader(type));
	GLCall(glShaderSource(id, 1, &src, NULL));
	GLCall(glCompileShader(id));

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {
		int length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)CallocOrDie(length, sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		printf("Failed to compile %s: len={%d}\n%s\n", 
			(type == GL_VERTEX_SHADER)? "vertex":"fragment", length, message);
		glDeleteShader(id);
		freeOrDie(message);
		return 0;
	}
#if DEBUG
	printf("Successfully Compiled\n");
#endif
	return id;
}

#define Compiled_Shader_Delete(s) GLCall(glDeleteShader(s))

unsigned Shader_Program(unsigned vs, unsigned fs)
{
	GLCall(unsigned int program = glCreateProgram());
	GLCall(glAttachShader(program, vs));
	GLCall(glAttachShader(program, fs));
	GLCall(glLinkProgram(program));
	GLCall(glValidateProgram(program));
	return program;
}

unsigned CreateShader(const char* vertexShader, const char* fragmentShader) {
	GLCall(unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader));
	GLCall(unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));

	unsigned prog = Shader_Program(vs, fs);

	Compiled_Shader_Delete(vs);
	Compiled_Shader_Delete(fs);
	
	return prog;
}

Shader init_Shader(char* filepath) {
	Shader shader = {0, init_hashMap(int)};
	char * file = fileContent_str(filepath);
	GLCall(SHDR_PrgmSrc src = parseShader(file));
	freeOrDie(file);
	GLCall(shader.id = CreateShader(src[0], src[1]));
	
	SHDR_PS_Delete(src, SHDR_PS_SIZE);

	return shader;
}

Shader init_ShaderStr(char* file) {
	Shader shader = {0, init_hashMap(int)};
	GLCall(SHDR_PrgmSrc src = parseShader(file));
	GLCall(shader.id = CreateShader(src[0], src[1]));
	
	SHDR_PS_Delete(src, SHDR_PS_SIZE);

	return shader;
}

void Shader_Delete(Shader s) {
	GLCall(glDeleteProgram(s.id));
	freeOrDie(s.m_UniformLocationCache);
}
#define ShaderBind(s) GLCall(glUseProgram(s.id))
#define ShaderUnBind() GLCall(glUseProgram(0))

int Shader_GetUniformLocation(Shader * shader, char* name) {
	int r = hash_get(shader->m_UniformLocationCache, name);
#if DEBUG
	printf("Shader_GetUnifromLocation: name = {%s}\n", name);
#endif
	if(r != -1) {
#if DEBUG
		printf("hash_get(%s) = %d\n",name, r);
#endif
		return r;
	}

	GLCall(int location = glGetUniformLocation(shader->id, name));

#if DEBUG
		printf("location of %s = %d\n",name, location);
#endif	
	if(location == -1) 
		printf("Warning: uniform %s, doesn't exist.\n",name);

	hash_push(shader->m_UniformLocationCache, name, location);

	return location;
}

#define Shader_SetUniform1i(s, n, v0) Shader_SU(&(s), n, 0, v0)
#define Shader_SetUniform1f(s, n, v0) Shader_SU(&(s), n, 1, v0)
#define Shader_SetUniform2fv(s, n, p, c) Shader_SU(&(s), n, 2, p, c)
#define Shader_SetUniform3fv(s, n, p, c) Shader_SU(&(s), n, 3, p, c)
#define Shader_SetUniform4f(s, n, ...) Shader_SU(&(s), n, 4, __VA_ARGS__)
#define Shader_SetUniform4fv(s, n, p, c) Shader_SU(&(s), n, 5, p, c)
#define Shader_SetUniformMat2f(s, n, c, m) Shader_SU(&(s), n, 6, c, m)
#define Shader_SetUniformMat3f(s, n, c, m) Shader_SU(&(s), n, 7, c, m)
#define Shader_SetUniformMat4f(s, n, c, m) Shader_SU(&(s), n, 8, c.mem, m)

#define Shader_SetUniform2uiv(s,n,p,c) GLCall(glUniform2uiv(Shader_GetUniformLocation(&s, n), c, p))

void Shader_SU(Shader * shader, char* name, unsigned type, ...)
{
#if DEBUG
	printf("hash_map pointer = {%p}\n", shader->m_UniformLocationCache);
	printf("name = %s\n", name);
#endif
	va_list vl;
	va_start(vl, type);
	int location = Shader_GetUniformLocation(shader, name);
	float * p, *mem;
	switch(type) {
		case 0://1i
			GLCall(glUniform1i(location, va_arg(vl, int)));
			break;
		case 1://1f
			GLCall(glUniform1f(location, va_arg(vl, double)));
			break;
		case 2://2fv
			p = va_arg(vl, float*);
			GLCall(glUniform2fv(location, va_arg(vl, unsigned), p));
			break;
		case 3://3fv
			p = va_arg(vl, float*);
			GLCall(glUniform3fv(location, va_arg(vl, unsigned), p));
			break;
		case 4://4f
			GLCall(glUniform4f(location, va_arg(vl, double), va_arg(vl, double), va_arg(vl, double), va_arg(vl, double)));
			break;
		case 5://4fv
			p = va_arg(vl, float*);
			GLCall(glUniform4fv(location, va_arg(vl, unsigned), p));
			break;
		case 6://mat 2fv
			mem = va_arg(vl, float*);
			unsigned count = va_arg(vl, unsigned);
			GLCall(glUniformMatrix2fv(location, count, GL_TRUE, mem));
			//GLCall(glUniformMatrix2fv(location, va_arg(vl, unsigned), GL_TRUE, mem));
			break;
		case 7://mat 3fv
			mem = va_arg(vl, float*);
			GLCall(glUniformMatrix3fv(location, va_arg(vl, unsigned), GL_TRUE, mem));
			break;
		case 8://matrix 4fv
			mem = va_arg(vl, float *);
			GLCall(glUniformMatrix4fv(location, va_arg(vl, unsigned), GL_TRUE, mem));
			break;
	}
	va_end(vl);
}

/*void Shader_SU_1i(Shader * shader, const char* name, int v0) {
	GLCall(glUniform1i(Shader_GetUniformLocation(shader, name), v0));
}

void Shader_SetUniform1f(Shader shader, const char* name, float v0) {
	GLCall(glUniform1f(Shader_GetUniformLocation(shader, name), v0));
}

void Shader_SetUniform4f(Shader shader, const char* name, float v0, float v1, float v2, float v3) {
	GLCall(glUniform4f(Shader_GetUniformLocation(shader, name), v0, v1, v2, v3));
}

void Shader_SetUniformMat4f(Shader shader, const char* name, matrix m) {
	GLCall(glUniformMatrix4fv(Shader_GetUniformLocation(shader, name), 1, GL_TRUE, m.mem));
}*/

#undef DEBUG
#undef DEBUG_PREFIX

