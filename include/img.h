#ifndef IMG_H
#define IMG_H

#include "ppm.h"
#include "png.c"
//#include "jpg.h"// JPEG/JFIF
//Exif
//TIFF
//GIF
//BMP
//WebP
//HDR
//HEIF
//AVIF
//There's a lot more
//Might want to include LIFF for microscope image processing cause it's cool
//SVG

// load image from any file type
//extern img_t loadImg(const char * file);
img_t loadImg(const char * file) {
	struct c_File f	= fileContent(file);	
	if(isPNG(f.buffer)) {
		return loadPNG(f); 
	}else {
		return loadPPM(f.buffer);
	}
}

// write to any image file type
extern int writeImg(const char * file, unsigned char* data, char type);

#endif
