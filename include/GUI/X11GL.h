#ifndef X11GL_H
#define X11GL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glext.h>

#include <sys/time.h>
#include <unistd.h>

#define DEBUG 0
#define DEBUG_PREFIX "[X11GL ERROR]\t"

#define xkey_F1 0x43
#define xkey_F11 0x5f
#define xkey_esc 0x9
#define xkey_1 0xa
#define xkey_2 0xb
#define xkey_3 0xc
#define xkey_4 0xd
#define xkey_5 0xe
#define xkey_6 0xf
#define xkey_7 0x10
#define xkey_8 0x11
#define xkey_9 0x12
#define xkey_0 0x13
#define xkey_MINUS 0x14
#define xkey_PLUS 0x15
#define xkey_q 0x18
#define xkey_w 0x19
#define xkey_e 0x1a
#define xkey_r 0x1b
#define xkey_t 0x1c
#define xkey_y 0x1d
#define xkey_u 0x1e
#define xkey_i 0x1f
#define xkey_o 0x20
#define xkey_p 0x21
#define xkey_OPEN_BRACE 0x22
#define xkey_CLOSE_BRACE 0x23
#define xkey_a 0x26
#define xkey_s 0x27
#define xkey_d 0x28
#define xkey_f 0x29
#define xkey_g 0x2a
#define xkey_h 0x2b
#define xkey_j 0x2c
#define xkey_k 0x2d
#define xkey_l 0x2e
#define xkey_SEMI_COLON 0x2f
#define xkey_INVERTED_COMMA 0x30
#define xkey_TILDE 0x31
#define xkey_BACKSLASH 0x33
#define xkey_z 0x34
#define xkey_x 0x35
#define xkey_c 0x36
#define xkey_v 0x37
#define xkey_b 0x38
#define xkey_n 0x39
#define xkey_m 0x3a
#define xkey_COMMA 0x3b
#define xkey_DOT 0x3c
#define xkey_SLASH 0x3d

#define xkey_BACKSPACE 0x16
#define xkey_TAB 0x17
#define xkey_SPACE 0x41
#define xkey_CAPS 0x42

#define xkey_LSHIFT 0x32
#define xkey_RSHIFT 0x3e
#define xkey_ENTER 0x24
#define xkey_CTRL 0x25

#define xkey_SHIFT_MOD 1
#define xkey_CTRL_MOD 2

#define xkey_UP 0x6f
#define xkey_DOWN 0x74
#define xkey_LEFT 0x71
#define xkey_RIGHT 0x72

#define xbutton_LEFT 1
#define xbutton_MIDDLE 2
#define xbutton_RIGHT 3
#define xbutton_SCROLL_UP 4
#define xbutton_SCROLL_DOWN 5

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

/*static double GetMilliseconds() {
	static struct timeval s_tTimeVal;
	gettimeofday(&s_tTimeVal, NULL);
	double time = s_tTimeVal.tv_sec * 1000.0; // sec to ms
	time += s_tTimeVal.tv_usec / 1000.0; // us to ms
	return time;
}*/

static char isExtensionSupported(const char *extList, const char *extension) {
	return strstr(extList, extension) != 0;
}

typedef struct {
	int scrId, width, height;
	XEvent ev;
	GLXContext context;
	Window w;
	Colormap colmap;
	Atom delwindow;
	Display * d;
	Screen* scr;
	XVisualInfo* visual;
} XGL_t;

#define Xwindow_setup(gl, name, mask) \
	XStoreName(gl.d, gl.w, name);\
	XSelectInput(gl.d, gl.w, mask);\
	XClearWindow(gl.d, gl.w);\
	XMapRaised(gl.d, gl.w);\

#define Xwindow_Setup(gl, name, mask) \
	XStoreName(gl->d, gl->w, name);\
	XSelectInput(gl->d, gl->w, mask);\
	XClearWindow(gl->d, gl->w);\
	XMapRaised(gl->d, gl->w);

#define glxDestroyDisplay(gl) _glxDestroyDisplay(&gl)
void _glxDestroyDisplay(XGL_t *gl)
{
	if(!gl->d) return;
	glXDestroyContext(gl->d, gl->context);
	XFree(gl->visual);
	XFreeColormap(gl->d, gl->colmap);
	XDestroyWindow(gl->d, gl->w);
	XCloseDisplay(gl->d);
	gl->d = NULL;
	gl->visual = NULL;
}

Display* XGL_openDisplay(char * name) {
	Display * d = XOpenDisplay(name);
	if (d == NULL) {
		printf(DEBUG_PREFIX "Could not open display\n");
		exit(1);
	}
	return d;
}

XGL_t XGL_openScreen(char * name) {
	XGL_t scr = {0};
	scr.d = XGL_openDisplay(name);
	scr.scr = DefaultScreenOfDisplay(scr.d);
	scr.scrId = DefaultScreen(scr.d);
	return scr;
}

void XGL_versionCheck(XGL_t gl)
{
	// Check GLX version
	GLint majorGLX, minorGLX = 0;
	glXQueryVersion(gl.d, &majorGLX, &minorGLX);
	if (majorGLX <= 1) {
		if(minorGLX < 2) { 
			printf("GLX 1.3 or greater is required.\n");
			XCloseDisplay(gl.d);
			exit(1);
		}
	}
}

GLXFBConfig XGL_chooseFB(XGL_t * gl, GLint * glxAttribs) {
	if(glxAttribs == NULL) {
		//glxAttribs = malloc(sizeof(GLint) * 23);
		GLint tmpArr[] = {
			GLX_X_RENDERABLE    , True,
			GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
			GLX_RENDER_TYPE     , GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
			GLX_RED_SIZE        , 8,
			GLX_GREEN_SIZE      , 8,
			GLX_BLUE_SIZE       , 8,
			GLX_ALPHA_SIZE      , 8,
			GLX_DEPTH_SIZE      , 24,
			GLX_STENCIL_SIZE    , 8,
			GLX_DOUBLEBUFFER    , True,
			None
		};
		glxAttribs = tmpArr;
	}

	int fbcount;

	GLXFBConfig* fbc = glXChooseFBConfig(gl->d, gl->scrId, glxAttribs, &fbcount);
	if (!fbc) {
		printf("Failed to retrieve framebuffer.\n");
		XCloseDisplay(gl->d);
		exit(1);
	}

	// Pick the FB config/visual with the most samples per pixel
	int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
	for (int i = 0; i < fbcount; ++i) {
		XVisualInfo *vi = glXGetVisualFromFBConfig( gl->d, fbc[i] );
		if ( vi != 0) {
			int samp_buf, samples;
			glXGetFBConfigAttrib( gl->d, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf );
			glXGetFBConfigAttrib( gl->d, fbc[i], GLX_SAMPLES       , &samples  );

			if ( best_fbc < 0 || (samp_buf && samples > best_num_samp) ) {
				best_fbc = i;
				best_num_samp = samples;
			}
			if ( worst_fbc < 0 || !samp_buf || samples < worst_num_samp )
				worst_fbc = i;
			worst_num_samp = samples;
		}
		XFree( vi );
	}
	GLXFBConfig bestFbc = fbc[ best_fbc ];
	XFree( fbc ); // Make sure to free this!

	
	gl->visual = glXGetVisualFromFBConfig( gl->d, bestFbc );
	if (gl->visual == 0) {
		printf("Could not create correct visual window.\n");
		XCloseDisplay(gl->d);
		exit(1);
	}
	
	if (gl->scrId != gl->visual->screen) {
		printf("screenId(%d) does not match visual->screen(%d).\n", gl->scrId, gl->visual->screen);
		XCloseDisplay(gl->d);
		exit(1);

	}

	return bestFbc;
}

void XGL_openWindow(XGL_t * gl, unsigned width, unsigned height, GLXFBConfig bestFbc)
{
	// Open the window
	XSetWindowAttributes windowAttribs;
	windowAttribs.border_pixel = BlackPixel(gl->d, gl->scrId);
	windowAttribs.background_pixel = WhitePixel(gl->d, gl->scrId);
	windowAttribs.override_redirect = True;
	windowAttribs.colormap = XCreateColormap(gl->d, RootWindow(gl->d, gl->scrId), gl->visual->visual, AllocNone);
	windowAttribs.event_mask = ExposureMask;
	gl->width = width;
	gl->height = height;
	gl->w = XCreateWindow(gl->d, RootWindow(gl->d, gl->scrId), 0, 0, width, height, 0, gl->visual->depth, InputOutput, gl->visual->visual, CWBackPixel | CWColormap | CWBorderPixel | CWEventMask, &windowAttribs);

	// Redirect Close
	gl->delwindow = XInternAtom(gl->d, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(gl->d, gl->w, &gl->delwindow, 1);

	// Create GLX OpenGL context
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
	
	int context_attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		None
	};

	const char *glxExts = glXQueryExtensionsString( gl->d,  gl->scrId );
	if (!isExtensionSupported( glxExts, "GLX_ARB_create_context")) {
		//printf("GLX_ARB_create_context not supported\n");
		gl->context = glXCreateNewContext( gl->d, bestFbc, GLX_RGBA_TYPE, 0, True );
	}else
		gl->context = glXCreateContextAttribsARB( gl->d, bestFbc, 0, True, context_attribs );

	XSync( gl->d, False );

	// Verifying that context is a direct context
	/*if (!glXIsDirect (gl->d, gl->context))
		printf("Indirect GLX rendering context obtained\n");
	else
		printf("Direct GLX rendering context obtained\n");
	*/

	glXMakeCurrent(gl->d, gl->w, gl->context);

	/*printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	*/

	gl->colmap = windowAttribs.colormap;
}

void XGLOpenWindow(XGL_t *gl, int w, int h, char* name, long mask) {
	*gl = XGL_openScreen(NULL);
	XGL_versionCheck(*gl);
	GLXFBConfig bestFbc = XGL_chooseFB(gl, NULL);
	XGL_openWindow(gl, w, h, bestFbc);
	Xwindow_Setup(gl, name, mask);
}

#undef DEBUG
#undef DEBUG_PREFIX
#endif
