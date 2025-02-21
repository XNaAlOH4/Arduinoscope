#ifndef CUSTOM_GUI_H
#define CUSTOM_GUI_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)	
	#include "WGL.h"

   //define something for Windows (32-bit and 64-bit, this part is common)
   #ifdef _WIN64
      //define something for Windows (64-bit only)
   #else
      //define something for Windows (32-bit only)
   #endif
#elif __APPLE__
	#include <TargetConditionals.h>
	#import <Cocoa/Cocoa.h>
	#import <OpenGL/gl.h>
	#include <OpenGL/glu.h>

    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __ANDROID__
    // Below __linux__ check should be enough to handle Android,
    // but something may be unique to Android.
#elif __linux__ || __unix__ // all unices not caught above

#include "X11GL.h"

#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif


#include "Renderer.h"

extern char Initialize(int w, int h);
extern char Update();
extern void Resize(int w, int h);
extern void Shutdown();
#endif
