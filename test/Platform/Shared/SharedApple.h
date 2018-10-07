
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
//         _j__j__j_)     `-------/ /__j__j__j_
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
//  SharedMac.h
//  Common includes and definitions for Mac/iOS systems. :)
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef SharedMac_h
#define SharedMac_h

#ifndef __APPLE__
    #warning SharedApple.h must be used only with Apple devices.
#endif

#include <mach/mach.h>
#include <pwd.h>

#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/param.h>

// Network includes.
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <CoreFoundation/CoreFoundation.h>

// OpenAL
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

//#define NEKO_EDITOR

//#define iOS_BUILD

#define USES_OPENGL
//#define NEKO_SERVER

//#define USES_METAL

// Since something wrong happened in El Capitan, nextEventMatchingMask keeps crashing.
//#define USES_GLFW

#if defined(USES_OPENGL) && defined(NEKO_SERVER) 
    #error Couldn't use graphics and server defines together
#endif

/*
    List of supported Apple devices:  
 
        iPhone 5s and above.
        iPad Air and above.
 
        All Mac computers with OpenGL 3.0 and above.
        All iOS devices with OpenGL ES 3.0 support.
 
    Use iOS_BUILD define when building an iOS build.
*/
 
#ifdef iOS_BUILD
    // OpenGL ES 3.0.
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
#else
    // OpenGL 3.0 and above.
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
#endif

// Some common functions.
//const char  *GetSystemVersion();
//const char  *GetBundlePath();

//static void MessageBox( const char *_title, const char *_msg );
//void    MassageBox( const char *_title, const char *_msg );
//void    SetCursorPos( int x, int y );
//void    OSX_GetDeltaMousePosition( int *x, int *y );

#endif
