#ifndef RENDERER_TEXTURE_H
#define RENDERER_TEXTURE_H

#include "Renderer.h"
#include <IMG/img.h>

#define DEBUG 0
#define DEBUG_PREFIX "[Texture Error]"

typedef struct {
	unsigned id, width, height, bpp;
	unsigned char* buf;
}Texture;

#define Texture_Bind(t) GLCall(glBindTexture(GL_TEXTURE_2D, t.id));
#define Texture_UnBind() GLCall(glBindTexture(GL_TEXTURE_2D, 0));
#define Texture_BindID(t) GLCall(glBindTexture(GL_TEXTURE_2D, t));
#define SET_ACTIVE_TEX(x) GLCall(glActiveTexture(GL_TEXTURE0+x));
#define GLTEX2D_PARAM(p, v) GLCall(glTexParameteri(GL_TEXTURE_2D, p, v))
#define Texture_Delete(t) GLCall(glDeleteTextures(1, &tex.id));

Texture build_Texture(const char * filepath) {
	Texture tex;
	img_t img = loadImg(filepath);
	GLCall(glGenTextures(1, &tex.id));
	Texture_Bind(tex);

	GLTEX2D_PARAM(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLTEX2D_PARAM(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLTEX2D_PARAM(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLTEX2D_PARAM(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	int col_val, int_form;
	switch(img.bpp) {
		case 1:
			col_val = GL_RED;
			int_form = GL_R8;
			break;
		case 3:
			col_val = GL_RGB;
			int_form = GL_RGB8;
			break;
		case 4:
			col_val = GL_RGBA;
			int_form = GL_RGBA8;
			break;
	}	

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, int_form, img.width, img.height, 0, col_val, GL_UNSIGNED_BYTE, img.buffer));
	Texture_UnBind();

	freeOrDie(img.buffer);

	return tex;
}

Texture init_Texture(img_t img) {
	Texture t;
	GLCall(glGenTextures(1, &t.id));
	
	SET_ACTIVE_TEX(0);
	Texture_Bind(t);

	GLCall(glEnable(GL_BLEND));
	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	GLTEX2D_PARAM(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLTEX2D_PARAM(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	GLTEX2D_PARAM(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLTEX2D_PARAM(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	int col_val, int_form;
	switch(img.bpp) {
		case 1:
			col_val = GL_RED;
			int_form = GL_R8;
			break;
		case 2:
			col_val = GL_RG;
			int_form = GL_RG8;
			break;
		case 3:
			col_val = GL_RGB;
			int_form = GL_RGB8;
			break;
		case 4:
			col_val = GL_RGBA;
			int_form = GL_RGBA8;
			break;
	}	

	//GLCall(glTexImage2D(GL_TEXTURE_2D, 0, int_form, img.width, img.height, 0, col_val, GL_UNSIGNED_BYTE, img.buffer));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, int_form, img.width, img.height>760? 760:img.height, 0, col_val, GL_UNSIGNED_BYTE, img.buffer));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));
	Texture_UnBind();
	return t;
}

void delete_Texture(Texture * tex) {
	GLCall(glDeleteTextures(1, &tex->id));
}

void Bind_Texture(Texture tex, int slot) {
	GLCall(glActiveTexture(GL_TEXTURE0 + slot));
	GLCall(glBindTexture(GL_TEXTURE_2D, tex.id));
}

#undef DEBUG
#undef DEBUG_PREFIX
#endif
