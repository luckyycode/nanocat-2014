//
//          *                  *
//             __                *
//           ,db'    *     *
//          ,d8/       *        *    *
//          888
//          `db\       *     *
//            `o`_                    **
//         *               *   *    _      *
//               *                 / )
//             *    /\__/\ *       ( (  *
//           ,-.,-.,)    (.,-.,-.,-.) ).,-.,-.
//          | @|  ={      }= | @|  / / | @|o |
//         _j__j__j_\     `-------/ /__j__j__j_
//          ________(               /___________
//          |  | @| \              || o|O | @|
//          |o |  |,'\       ,   ,'"|  |  |  |  hjw
//          vV\|/vV|`-'\  ,---\   | \Vv\hjwVv\//v
//                     _) )    `. \ /
//                    (__/       ) )
//    _   _        _                                _
//   | \ | |  ___ | | __ ___     ___  _ __    __ _ (_) _ __    ___
//   |  \| | / _ \| |/ // _ \   / _ \| '_ \  / _` || || '_ \  / _ \
//   | |\  ||  __/|   <| (_) | |  __/| | | || (_| || || | | ||  __/
//   |_| \_| \___||_|\_\\___/   \___||_| |_| \__, ||_||_| |_| \___|
//                                           |___/
//  SharedWin.h
//  Shared Windows headers and preferences. v:)
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef SharedWin_h
#define SharedWin_h

// I was lazy to rewrite some, so
#define uint    UINT
#define Byte    byte
//#define false   FALSE
//#define true    TRUE

// For MEMORYSTATUS.
// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <winsock.h>
#include <winsock2.h>
#include <fcntl.h>
#ifndef _WIN32
	#include <sys/fcntl.h>
#endif
#include <windows.h>
#include <Windowsx.h>
#include <psapi.h>
#include <math.h>
//#include <cpuid.h>
#include <stdio.h>

// OpenAL 1.1 SDK.
#include <al.h>
#include <alc.h>

#pragma comment( lib, "OpenAL32.lib" )

#define GLEW_STATIC

#define USES_OPENGL

// For uint32_t, uint16_t, uint8_t, int32_t, int16_t, int8_t, etc...
#include <stdint.h> 
#include <inttypes.h> 

// Used for OpenGL extensions, not window creation
// TODO - custom extensions without 3rd party libs
#include <GL/glew.h>
#include <GL/wglew.h>

#include <GL/gl.h> // Y u no gl3?

#define GLEW_STATIC

#pragma comment ( lib, "OpenGL32.lib" )
#pragma comment ( lib, "assimp.lib" ) // temp

#pragma comment ( lib, "Kernel32.lib" )
#pragma comment ( lib, "Ws2_32.lib" )

//#pragma comment( lib, "glew32.lib" )
#pragma comment( lib, "glew32s.lib" )

// DirectX 11 Libraries.
//#pragma comment( lib, "dxgi.lib" )
//#pragma comment( lib, "d3d11.lib" )
//#pragma comment( lib, "d3dx11.lib" )

#pragma comment ( lib, "user32" )

// No GLU on newer OpenGL versions.
#define GLEW_NO_GLU

#if defined(_WIN32) || defined(_WIN64) 
#define snprintf _snprintf 
#define vsnprintf _vsnprintf 
#define strcasecmp _stricmp 
#define strncasecmp _strnicmp 
#define strdup _strdup
#define write _write
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900
	#define snprintf c99_snprintf
	#define vsnprintf c99_vsnprintf
	extern int c99_snprintf(char *outBuf, size_t size, const char *format, ...);
	extern int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap);
#endif
namespace Neko {
    const unsigned int CON_EDIT_ID      = 100;
    const unsigned int CON_INPUT_ID	    = 101;

    const int EXTERNALCONSOLE_TEXTLEN = 512;
}
#endif
