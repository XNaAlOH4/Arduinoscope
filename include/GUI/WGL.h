#ifndef WINDOWS_GL_H
#define WINDOWS_GL_H

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <windowsx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <GL/wglext.h>
#include <stdio.h>
#include <Memory.h>

//#define xkey_F1 0x43
//#define xkey_F11 0x5f
#define xkey_esc VK_ESCAPE
#define xkey_1 '1'
#define xkey_2 '2'
#define xkey_3 '3'
#define xkey_4 '4'
#define xkey_5 '5'
#define xkey_6 '6'
#define xkey_7 '7'
#define xkey_8 '8'
#define xkey_9 '9'
#define xkey_0 '0'
#define xkey_MINUS VK_MINUS
#define xkey_PLUS VK_ADD
#define xkey_q 'Q'
#define xkey_w 'W'
#define xkey_e 'E'
#define xkey_r 'R'
#define xkey_t 'T'
#define xkey_y 'Y'
#define xkey_u 'U'
#define xkey_i 'I'
#define xkey_o 'O'
#define xkey_p 'P'
//#define xkey_OPEN_BRACE 0x22
//#define xkey_CLOSE_BRACE 0x23
#define xkey_a 'A'
#define xkey_s 'S'
#define xkey_d 'D'
#define xkey_f 'F'
#define xkey_g 'G'
#define xkey_h 'H'
#define xkey_j 'J'
#define xkey_k 'K'
#define xkey_l 'L'
//#define xkey_SEMI_COLON 0x2f
//#define xkey_INVERTED_COMMA 0x30
//#define xkey_TILDE 0x31
//#define xkey_BACKSLASH 
#define xkey_z 'Z'
#define xkey_x 'X'
#define xkey_c 'C'
#define xkey_v 'V'
#define xkey_b 'B'
#define xkey_n 'N'
#define xkey_m 'M'
//#define xkey_COMMA 0x3b
#define xkey_DOT VK_DECIMAL
#define xkey_SLASH VK_DIVIDE

#define xkey_BACKSPACE VK_BACK
#define xkey_TAB VK_TAB
#define xkey_SPACE VK_SPACE
#define xkey_CAPS VK_CAPITAL

#define xkey_LSHIFT VK_LSHIFT
#define xkey_RSHIFT VK_RSHIFT
#define xkey_ALT VK_MENU
#define xkey_PAUSE VK_PAUSE
#define xkey_ENTER VK_RETURN
#define xkey_CTRL VK_CONTROL

#define xkey_SHIFT_MOD 1
#define xkey_CTRL_MOD 2

#define xkey_UP VK_UP
#define xkey_DOWN VK_DOWN
#define xkey_LEFT VK_LEFT
#define xkey_RIGHT VK_RIGHT

#define xbutton_LEFT VK_LBUTTON
#define xbutton_MIDDLE VK_MBUTTON
#define xbutton_RIGHT VK_RBUTTON
//#define xbutton_SCROLL_UP 4
//#define xbutton_SCROLL_DOWN 5

typedef unsigned char uchar;

typedef struct {
	int width, height;
	uchar rest : 1,
	      vsync : 1;
	HWND w;
	HDC dc;
} WGL_t;

extern LRESULT WINAPI MainWndProc (HWND, UINT, WPARAM, LPARAM);
extern BOOL bSetupPixelFormat(HDC);
static void find_coreGL(void);
void loadGL();

#define Assert(x) if(!(x)) exit(1);

wchar_t* wnd_name;

static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static int StringsAreEqual(const char* src, const char* dst, size_t dstlen)
{
    while (*src && dstlen-- && *dst)
    {
        if (*src++ != *dst++)
        {
            return 0;
        }
    }

    return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

static void GetWglFunctions(void)
{
    // to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    Assert(dummy && "Failed to create dummy window");

    HDC dc = GetDC(dummy);
    Assert(dc && "Failed to get device context for dummy window");

    PIXELFORMATDESCRIPTOR desc =
    {
        .nSize = sizeof(desc),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
    };

    int format = ChoosePixelFormat(dc, &desc);
    if (!format)
    {
        FatalError("Cannot choose OpenGL pixel format for dummy window!");
    }

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    Assert(ok && "Failed to describe OpenGL pixel format");

    // reason to create dummy window is that SetPixelFormat can be called only once for the window
    if (!SetPixelFormat(dc, format, &desc))
    {
        FatalError("Cannot set OpenGL pixel format for dummy window!");
    }

    HGLRC rc = wglCreateContext(dc);
    Assert(rc && "Failed to create OpenGL context for dummy window");

    ok = wglMakeCurrent(dc, rc);
    Assert(ok && "Failed to make current OpenGL context for dummy window");

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB =
        (void*)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB)
    {
        FatalError("OpenGL does not support WGL_ARB_extensions_string extension!");
    }

    const char* ext = wglGetExtensionsStringARB(dc);
    Assert(ext && "Failed to get OpenGL WGL extension string");

    const char* start = ext;
    for (;;)
    {
        while (*ext != 0 && *ext != ' ')
        {
            ext++;
        }

        size_t length = ext - start;
        if (StringsAreEqual("WGL_ARB_pixel_format", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
            wglChoosePixelFormatARB = (void*)wglGetProcAddress("wglChoosePixelFormatARB");
        }
        else if (StringsAreEqual("WGL_ARB_create_context", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
            wglCreateContextAttribsARB = (void*)wglGetProcAddress("wglCreateContextAttribsARB");
        }
        else if (StringsAreEqual("WGL_EXT_swap_control", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
            wglSwapIntervalEXT = (void*)wglGetProcAddress("wglSwapIntervalEXT");
        }

        if (*ext == 0)
        {
            break;
        }

        ext++;
        start = ext;
    }

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT)
    {
        FatalError("OpenGL does not support required WGL extensions for modern context!");
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
}

// I need to make a function to create windows in windows not a pain in the ass

// Create GL context + Register window class + create window + create device context
void CreateGLWindow(WGL_t* gl, int width, int height, const char* name, HINSTANCE instance)
{
	GetWglFunctions();

	int wchars_num = MultiByteToWideChar( CP_UTF8, 0, name, -1, NULL, 0);
	wnd_name = CallocOrDie(sizeof(wchar_t), wchars_num);
	MultiByteToWideChar( CP_UTF8, 0, name, -1, wnd_name, wchars_num);
	WNDCLASSEXW wc =
	{
		.cbSize = sizeof(wc),
		.lpfnWndProc = MainWndProc,
		.hInstance = instance,
		.hIcon = LoadIcon(NULL, IDI_APPLICATION),
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.lpszClassName = L"opengl_window_class",
	};

	Assert( RegisterClassExW(&wc) && "Failed to register window class");

	DWORD exstyle = WS_EX_APPWINDOW,
	      style = WS_OVERLAPPEDWINDOW;

	gl->width = width;
	gl->height = height;
	gl->w = CreateWindowExW(
		exstyle, wc.lpszClassName, wnd_name, style,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, wc.hInstance, NULL);
	Assert(gl->w && "Failed to create window");

	gl->dc = GetDC(gl->w);
	Assert(gl->dc && "Failed to get window device context");

	{
		// Set pixel format for OpenGL context
		int pix_attr[] = 
		{
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 24,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0,
		};
		int format;
		UINT formats;
		if(!wglChoosePixelFormatARB(gl->dc, pix_attr, NULL, 1, &format, &formats) || formats == 0)
		{
			FatalError("OpenGL does not support required pixel format!");
		}
		PIXELFORMATDESCRIPTOR desc = {.nSize = sizeof(desc)};
		int ok = DescribePixelFormat(gl->dc, format, sizeof(desc), &desc);
		Assert(ok && "Failed to describe OpenGL pixel format");
		if(!SetPixelFormat(gl->dc, format, &desc))
		{
			FatalError("Cannot set OpenGL selected pixel format!");
		}
	}

	int attrib[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0,
	};

	HGLRC rc = wglCreateContextAttribsARB(gl->dc, NULL, attrib);
	if(!rc)
	{
		FatalError("OpenGL version 3.0 not supported");
	}
	BOOL ok = wglMakeCurrent(gl->dc, rc);
	Assert(ok && "Failed to make current OpenGL context");

	loadGL();
}

// opengl loader

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef __gl_glcorearb_h_

#include <KHR/khrplatform.h>
typedef khronos_ssize_t GLsizeiptr;
typedef khronos_intptr_t GLintptr;
#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765
#define GL_QUERY_COUNTER_BITS             0x8864
#define GL_CURRENT_QUERY                  0x8865
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F
#define GL_READ_ONLY                      0x88B8
#define GL_WRITE_ONLY                     0x88B9
#define GL_READ_WRITE                     0x88BA
#define GL_BUFFER_ACCESS                  0x88BB
#define GL_BUFFER_MAPPED                  0x88BC
#define GL_BUFFER_MAP_POINTER             0x88BD
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA
#define GL_SAMPLES_PASSED                 0x8914
#define GL_SRC1_ALPHA                     0x8589

#endif

int CGL_VERSION_1_1,
    CGL_VERSION_1_3,
    CGL_VERSION_1_5,
    CGL_VERSION_2_0,
    CGL_VERSION_3_0,
    CGL_VERSION_3_1;

#define GL_FUNCTIONS_1_1(X)\
/*X(PFNGLDRAWARRAYSPROC,			glDrawArrays)\
X(PFNGLDRAWELEMENSPROC,		 	glDrawElements)\
X(PFNGLBINDTEXTUREPROC,			glBindTexture)\
X(PFNGLDELETETEXTURESPROC,		glDeleteTextures)\
X(PFNGLGENTEXTURESPROC,			glGenTextures)*/

#define GL_FUNCTIONS_1_3(X)\
X(PFNGLACTIVETEXTUREPROC,		glActiveTexture)

#define GL_FUNCTIONS_1_5(X)\
X(PFNGLBINDBUFFERPROC,			glBindBuffer)\
X(PFNGLDELETEBUFFERSPROC,		glDeleteBuffers)\
X(PFNGLGENBUFFERSPROC,			glGenBuffers)\
X(PFNGLBUFFERDATAPROC,			glBufferData)\
X(PFNGLBUFFERSUBDATAPROC,		glBufferSubData)

#define GL_FUNCTIONS_2_0(X)\
X(PFNGLATTACHSHADERPROC,		glAttachShader)\
X(PFNGLBINDATTRIBLOCATIONPROC,		glBindAttribLocation)\
X(PFNGLCOMPILESHADERPROC,		glCompileShader)\
X(PFNGLCREATEPROGRAMPROC,		glCreateProgram)\
X(PFNGLCREATESHADERPROC,		glCreateShader)\
X(PFNGLDELETEPROGRAMPROC,		glDeleteProgram)\
X(PFNGLDELETESHADERPROC,		glDeleteShader)\
X(PFNGLDETACHSHADERPROC,		glDetachShader)\
X(PFNGLDISABLEVERTEXATTRIBARRAYPROC,	glDisableVertexAttribArray)\
X(PFNGLENABLEVERTEXATTRIBARRAYPROC,	glEnableVertexAttribArray)\
X(PFNGLGETPROGRAMIVPROC,		glGetProgramiv)\
X(PFNGLGETPROGRAMINFOLOGPROC,		glGetProgramInfoLog)\
X(PFNGLGETSHADERIVPROC,			glGetShaderiv)\
X(PFNGLGETSHADERINFOLOGPROC,		glGetShaderInfoLog)\
X(PFNGLGETSHADERSOURCEPROC,		glGetShaderSource)\
X(PFNGLGETUNIFORMLOCATIONPROC,		glGetUniformLocation)\
X(PFNGLGETUNIFORMFVPROC,		glGetUniformfv)\
X(PFNGLGETUNIFORMIVPROC,		glGetUniformiv)\
X(PFNGLLINKPROGRAMPROC,			glLinkProgram)\
X(PFNGLSHADERSOURCEPROC,		glShaderSource)\
X(PFNGLUSEPROGRAMPROC,			glUseProgram)\
X(PFNGLUNIFORM1FPROC,			glUniform1f)\
X(PFNGLUNIFORM2FPROC,			glUniform2f)\
X(PFNGLUNIFORM3FPROC,			glUniform3f)\
X(PFNGLUNIFORM4FPROC,			glUniform4f)\
X(PFNGLUNIFORM1IPROC,			glUniform1i)\
X(PFNGLUNIFORM2IPROC,			glUniform2i)\
X(PFNGLUNIFORM3IPROC,			glUniform3i)\
X(PFNGLUNIFORM4IPROC,			glUniform4i)\
X(PFNGLUNIFORM1FVPROC,			glUniform1fv)\
X(PFNGLUNIFORM2FVPROC,			glUniform2fv)\
X(PFNGLUNIFORM3FVPROC,			glUniform3fv)\
X(PFNGLUNIFORM4FVPROC,			glUniform4fv)\
X(PFNGLUNIFORM1IVPROC,			glUniform1iv)\
X(PFNGLUNIFORM2IVPROC,			glUniform2iv)\
X(PFNGLUNIFORM3IVPROC,			glUniform3iv)\
X(PFNGLUNIFORM4IVPROC,			glUniform4iv)\
X(PFNGLUNIFORMMATRIX2FVPROC,		glUniformMatrix2fv)\
X(PFNGLUNIFORMMATRIX3FVPROC,		glUniformMatrix3fv)\
X(PFNGLUNIFORMMATRIX4FVPROC,		glUniformMatrix4fv)\
X(PFNGLVALIDATEPROGRAMPROC,		glValidateProgram)\
X(PFNGLVERTEXATTRIBPOINTERPROC,		glVertexAttribPointer)

#define GL_FUNCTIONS_3_0(X)\
X(PFNGLGETUNIFORMUIVPROC,		glGetUniformuiv)\
X(PFNGLBINDFRAGDATALOCATIONPROC, 	glBindFragDataLocation)\
X(PFNGLGETFRAGDATALOCATIONPROC, 	glGetFragDataLocation)\
X(PFNGLUNIFORM1UIPROC,			glUniform1ui)\
X(PFNGLUNIFORM2UIPROC,			glUniform2ui)\
X(PFNGLUNIFORM3UIPROC,			glUniform3ui)\
X(PFNGLUNIFORM4UIPROC,			glUniform4ui)\
X(PFNGLUNIFORM1UIVPROC,			glUniform1uiv)\
X(PFNGLUNIFORM2UIVPROC,			glUniform2uiv)\
X(PFNGLUNIFORM3UIVPROC,			glUniform3uiv)\
X(PFNGLUNIFORM4UIVPROC,			glUniform4uiv)\
X(PFNGLBINDRENDERBUFFERPROC,		glBindRenderbuffer)\
X(PFNGLDELETERENDERBUFFERSPROC,		glDeleteRenderbuffers)\
X(PFNGLGENRENDERBUFFERSPROC,		glGenRenderbuffers)\
X(PFNGLRENDERBUFFERSTORAGEPROC,		glRenderbufferStorage)\
X(PFNGLGETRENDERBUFFERPARAMETERIVPROC,	glGetRenderbufferParameteriv)\
X(PFNGLISFRAMEBUFFERPROC,		glIsFramebuffer)\
X(PFNGLBINDFRAMEBUFFERPROC,		glBindFramebuffer)\
X(PFNGLDELETEFRAMEBUFFERSPROC,		glDeleteFramebuffers)\
X(PFNGLGENFRAMEBUFFERSPROC,		glGenFramebuffers)\
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC,	glCheckFramebufferStatus)\
X(PFNGLFRAMEBUFFERTEXTURE1DPROC,	glFramebufferTexture1D)\
X(PFNGLFRAMEBUFFERTEXTURE2DPROC,	glFramebufferTexture2D)\
X(PFNGLFRAMEBUFFERTEXTURE3DPROC,	glFramebufferTexture3D)\
X(PFNGLFRAMEBUFFERRENDERBUFFERPROC,	glFramebufferRenderbuffer)\
X(PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC,	glGetFramebufferAttachmentParameteriv)\
X(PFNGLGENERATEMIPMAPPROC,		glGenerateMipmap)\
X(PFNGLBLITFRAMEBUFFERPROC,		glBlitFramebuffer)\
X(PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC,	glRenderbufferStorageMultisample)\
X(PFNGLFRAMEBUFFERTEXTURELAYERPROC,	glFramebufferTextureLayer)\
X(PFNGLMAPBUFFERRANGEPROC,		glMapBufferRange)\
X(PFNGLFLUSHMAPPEDBUFFERRANGEPROC,	glFlushMappedBufferRange)\
X(PFNGLBINDVERTEXARRAYPROC,		glBindVertexArray)\
X(PFNGLDELETEVERTEXARRAYSPROC,		glDeleteVertexArrays)\
X(PFNGLGENVERTEXARRAYSPROC,		glGenVertexArrays)

#define GL_FUNCTIONS_3_1(X)\
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced)\
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced)

#define GL_FUNCTIONS(X)\
GL_FUNCTIONS_1_1(X)\
GL_FUNCTIONS_1_3(X)\
GL_FUNCTIONS_1_5(X)\
GL_FUNCTIONS_2_0(X)\
GL_FUNCTIONS_3_0(X)\
GL_FUNCTIONS_3_1(X)

#define X(type, name) static type name;
GL_FUNCTIONS(X)
#undef X

static void find_coreGL(void) {
	const char *v = (const char*)glGetString(GL_VERSION);
	int major = v[0] - '0',
	    minor = v[2] - '0';
#define MINMAX(a,b) (major == a && minor >= b) || (major > a)
	CGL_VERSION_1_1 = MINMAX(1,0);
	CGL_VERSION_1_3 = MINMAX(1,3);
	CGL_VERSION_1_5 = MINMAX(1,5);
	CGL_VERSION_2_0 = MINMAX(2,0);
	CGL_VERSION_3_0 = MINMAX(3,0);
	CGL_VERSION_3_1 = MINMAX(3,1);
#undef MINMAX
}

void loadGL()
{
	find_coreGL();
#define X(type, name) name = (type)wglGetProcAddress(#name);
	if(CGL_VERSION_1_1) {
		GL_FUNCTIONS_1_1(X);
	}
	if(CGL_VERSION_1_3) {
		GL_FUNCTIONS_1_3(X);
	}
	if(CGL_VERSION_1_5) {
		GL_FUNCTIONS_1_5(X);
	}
	if(CGL_VERSION_2_0) {
		GL_FUNCTIONS_2_0(X);
	}
	if(CGL_VERSION_3_0) {
		GL_FUNCTIONS_3_0(X);
	}
	if(CGL_VERSION_3_1) {
		GL_FUNCTIONS_3_1(X);
	}
#undef X
}

#undef GL_FUNCTIONS_1_1
#undef GL_FUNCTIONS_1_3
#undef GL_FUNCTIONS_1_5
#undef GL_FUNCTIONS_2_0
#undef GL_FUNCTIONS_3_0
#undef GL_FUNCTIONS_3_1
#undef GL_FUNCTIONS
#endif
  
