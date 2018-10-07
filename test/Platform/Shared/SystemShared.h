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
//
//  Neko engine.
//
//  Common headers and defines..
//
//  Created by Neko Vision on 24/07/2014.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

/*
    This header contains all system headers.
    and some necessary stuff.


 *                ,MMM8&&&.            *
        *        MMMM88&&&&&    .
                MMMM88&&&&&&&
 *              MMM88&&&&&&&&
                MMM88&&&&&&&&
                'MMM88&&&&&&'
                  'MMM8&&&'      *
    *    |\___/|
         )     (             .              '
        >\     /<
          )===(       *
         /     \
         |     |
        /       \           *
        \       /
 _/\_/\_/\__  _/_/\_/\_/\_/\_/\_/\_/\_/\_/\_
 |  |  |  |( (  |  |  |  |  |  |  |  |  |  |
 |  |  |  | ) ) |  |  |  |  |  |  |  |  |  |
 |  |  |  |(_(  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |                    |                    |
 */


#ifndef SHARED_H_INCLUDED
#define SHARED_H_INCLUDED


#   if !defined( NEKO_PLATFORM )
#	define NEKO_PLATFORM

#	if defined( WINAPI_FAMILY ) && (WINAPI_FAMILY == WINAPI_PARTITION_APP)
#		define NEKO_WINRT 1   // Windows Runtime, either on Windows RT or Windows 8
#	elif defined(XBOXONE)
#		define NEKO_XBOXONE 1
#	elif defined(_WIN64)    // note: XBOXONE implies _WIN64
#		define NEKO_WIN64 1
#	elif defined(_M_PPC)
#		define NEKO_X360 1
#	elif defined(__ANDROID__)
#		define NEKO_ANDROID 1
#	elif defined(_WIN32)    // note: _M_PPC implies _WIN32
#		define NEKO_WIN32 1
#	elif defined(__linux__) || defined(__CYGWIN__) // note: __ANDROID__ implies __linux__
#		define NEKO_LINUX 1
#	elif defined(__APPLE__) && (defined(__arm__) || defined(__arm64__))
#		define NEKO_IOS 1
#	elif defined(__APPLE__)
#		define NEKO_OSX 1
#	elif defined(__ORBIS__)
#		define NEKO_PS4 1
#	elif defined(__CELLOS_LV2__)
#		define NEKO_PS3 1
#	elif defined(__SNC__) && defined(__arm__)
#		define NEKO_PSP2 1
#	elif defined(__ghs__)
#		define NEKO_WIIU 1
#	else
#		error "Unknown operating system"
#	endif

// zero unset
#	ifndef NEKO_WINRT
#		define NEKO_WINRT 0
#	endif
#	ifndef NEKO_XBOXONE
#		define NEKO_XBOXONE 0
#	endif
#	ifndef NEKO_WIN64
#		define NEKO_WIN64 0
#	endif
#	ifndef NEKO_X360
#		define NEKO_X360 0
#	endif
#	ifndef NEKO_WIN32
#		define NEKO_WIN32 0
#	endif
#	ifndef NEKO_ANDROID
#		define NEKO_ANDROID 0
#	endif
#	ifndef NEKO_LINUX
#		define NEKO_LINUX 0
#	endif
#	ifndef NEKO_IOS
#		define NEKO_IOS 0
#	endif
#	ifndef NEKO_OSX
#		define NEKO_OSX 0
#	endif
#	ifndef NEKO_PS3
#		define NEKO_PS3 0
#	endif
#	ifndef NEKO_PS4
#		define NEKO_PS4 0
#	endif
#	ifndef NEKO_PSP2
#		define NEKO_PSP2 0
#	endif
#	ifndef NEKO_WIIU
#		define NEKO_WIIU 0
#	endif

#endif // NEKO_PLATFORM

#   if !defined( NEKO_UNUSED )
#	define NEKO_UNUSED(v) (void)v;
#   endif


#   if !defined( NEKO_COMPILER )
#	define NEKO_COMPILER

#	if defined(_MSC_VER)
#		if _MSC_VER >= 1900
#			define NEKO_VC 14
#		elif _MSC_VER >= 1800
#			define NEKO_VC 12
#		elif _MSC_VER >= 1700
#			define NEKO_VC 11
#		elif _MSC_VER >= 1600
#			define NEKO_VC 10
#		elif _MSC_VER >= 1500
#			define NEKO_VC 9
#		else
#			error "Unknown VC version"
#		endif
#	elif defined(__clang__)
#		define NEKO_CLANG 1
#	elif defined(__SNC__)
#		define NEKO_SNC 1
#	elif defined(__ghs__)
#		define NEKO_GHS 1
#	elif defined(__GNUC__) // note: __clang__, __SNC__, or __ghs__ imply __GNUC__
#		define NEKO_GCC 1
#	else
#		error "Unknown compiler"
#	endif

// Zero unset
#	ifndef NEKO_VC
#		define NEKO_VC 0
#	endif
#	ifndef NEKO_CLANG
#		define NEKO_CLANG 0
#	endif
#	ifndef NEKO_SNC
#		define NEKO_SNC 0
#	endif
#	ifndef NEKO_GHS
#		define NEKO_GHS 0
#	endif
#	ifndef NEKO_GCC
#		define NEKO_GCC 0
#	endif

#endif // NEKO_COMPILER

// OS
#define NEKO_WINDOWS_FAMILY (NEKO_WINRT || NEKO_WIN32 || NEKO_WIN64)
#define NEKO_MICROSOFT_FAMILY (NEKO_XBOXONE || NEKO_X360 || NEKO_WINDOWS_FAMILY)
#define NEKO_LINUX_FAMILY (NEKO_LINUX || NEKO_ANDROID)
#define NEKO_APPLE_FAMILY (NEKO_IOS || NEKO_OSX)
#define NEKO_UNIX_FAMILY (NEKO_LINUX_FAMILY || NEKO_APPLE_FAMILY)
// Compilers
#define NEKO_GCC_FAMILY (NEKO_CLANG || NEKO_SNC || NEKO_GHS || NEKO_GCC)

#if !defined( NEKO_ALIGN )
#	if NEKO_MICROSOFT_FAMILY
#		define NEKO_ALIGN( alignment, decl ) __declspec(align(alignment)) decl
#		define NEKO_ALIGN_PREFIX( alignment ) __declspec(align(alignment))
#		define NEKO_ALIGN_SUFFIX( alignment )
#	elif NEKO_GCC_FAMILY
#		define NEKO_ALIGN( alignment, decl ) decl __attribute__((aligned(alignment)))
#		define NEKO_ALIGN_PREFIX( alignment )
#		define NEKO_ALIGN_SUFFIX( alignment ) __attribute__((aligned(alignment)))
#	else
#		define NEKO_ALIGN( alignment, decl )
#		define NEKO_ALIGN_PREFIX( alignment )
#		define NEKO_ALIGN_SUFFIX( alignment )
#	endif
#endif

#define NEKO_INLINE inline


// GCC Specific
#if NEKO_GCC_FAMILY
#	define NEKO_NO_INLINE __attribute__((noinline))

#	if !NEKO_LINUX
#		define NEKO_FORCE_INLINE inline __attribute__((always_inline))
#	endif

#	if !NEKO_SPU
#		define NEKO_PUSH_PACK_DEFAULT _Pragma("pack(push, 8)")
#		define NEKO_POP_PACK _Pragma("pack(pop)")
#	endif

#   define NEKO_BREAKPOINT( id ) __builtin_trap();

#	if (__cplusplus >= 201103L) && ((__GNUC__ > 4) || (__GNUC__ ==4 && __GNUC_MINOR__ >= 6))
#		define NEKO_NULL	NULL
#	else
#		define NEKO_NULL	__null
#	endif

#	define NEKO_ALIGN_OF(T)	__alignof__(T)
#	define NEKO_FUNCTION_SIG	__PRETTY_FUNCTION__

#endif // NEKO_GCC_FAMILY

// Microsoft VC specific
#if NEKO_MICROSOFT_FAMILY

#	pragma inline_depth(255)

#	pragma warning(disable : 4324 )	// C4324: structure was padded due to alignment specifier
#	pragma warning(disable : 4514 )	// 'function' : unreferenced inline function has been removed
#	pragma warning(disable : 4710 )	// 'function' : function not inlined
#	pragma warning(disable : 4711 )	// function 'function' selected for inline expansion

#	define NEKO_NO_ALIAS __declspec( noalias )
#	define NEKO_NO_INLINE __declspec( noinline )
#	define NEKO_FORCE_INLINE __forceinline
#	define NEKO_PUSH_PACK_DEFAULT __pragma( pack( push, 8 ) )
#	define NEKO_POP_PACK __pragma( pack( pop ) )

#	define NEKO_BREAKPOINT( id ) __debugbreak();

#	ifdef __cplusplus
#		define NEKO_NULL	NULL
#	endif

#	define NEKO_ALIGN_OF( T ) __alignof( T )

#	define NEKO_FUNCTION_SIG	__FUNCSIG__

#	define NEKO_INT64(x) (x##i64)
#	define NEKO_UINT64(x) (x##ui64)

#	define NEKO_STDCALL __stdcall
#	define NEKO_CALL_CONV __cdecl

#endif // NEKO_MICROSOFT_FAMILY

#   if !defined( NEKO_FORCE_INLINE )
#	define NEKO_FORCE_INLINE inline
#   endif


// Gcc
#	if NEKO_GCC_FAMILY
// Check for C++11
#			if (__GNUC__ * 100 + __GNUC_MINOR__) >= 407
#				define NEKO_OVERRIDE override
#			endif
#	endif // NEKO_GCC_FAMILY


#	if NEKO_VC
// C4481: nonstandard extension used: override specifier 'override'
#		if _MSC_VER < 1700
#			pragma warning(disable : 4481)
#		endif
#		define NEKO_OVERRIDE	override
#endif // NEKO_VC



#include <errno.h>
#include <time.h>

#include <fcntl.h>
//#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>

/*                ,MMM8&&&.            *
        *        MMMM88&&&&&    .
                MMMM88&&&&&&&
 *              MMM88&&&&&&&&
                MMM88&&&&&&&&
                'MMM88&&&&&&'
                  'MMM8&&&'      *
    *    |\___/|
         )     (             .              '
        >\     /<
          )===(       *
         /     \
         |     |
        /       \           *
        \       /
 _/\_/\_/\__  _/_/\_/\_/\_/\_/\_/\_/\_/\_/\_
 |  |  |  |( (  |  |  |  |  |  |  |  |  |  |
 |  |  |  | ) ) |  |  |  |  |  |  |  |  |  |
 |  |  |  |(_(  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |                    |                    |
 */



// We don't need this on mobile devices.
// #define OCULUSVR_SUPPORTED

/*
     Mac OS and iOS.
*/

#if defined( NEKO_APPLE_FAMILY )

// Use this while building iOS build.
//#define iOS_BUILD

#include "SharedApple.h"

    #define MAX_PATH            256

    /*      Default Mac key codes.      */

    const int       KEY_ENTER = 13;
    const int       KEY_SPACE = 49;
    const int       KEY_BACKSPACE = 127;
    const int       KEY_TILDE = 50;
    const int       KEY_ESCAPE = 53;

    const int       KEY_ARROW_UP = 126;
    const int       KEY_ARROW_DOWN = 125;
    const int       KEY_ARROW_LEFT = 123;
    const int       KEY_ARROW_RIGHT = 124;

#elif NEKO_WINDOWS_FAMILY

    #include "SharedWin.h"

    const int       KEY_ENTER = 13;
    const int       KEY_SPACE = 32;
    const int       KEY_BACKSPACE = 8;
    const int       KEY_TILDE = -64;
    const int       KEY_ESCAPE = 53;

    const int       KEY_ARROW_UP = 126;
    const int       KEY_ARROW_DOWN = 125;
    const int       KEY_ARROW_LEFT = 123;
    const int       KEY_ARROW_RIGHT = 124;

#include <unistd.h>
#include <sys/time.h>
#include <dirent.h>

#elif NEKO_LINUX_FAMILY

    #include "SharedLinux.h"

    #error Implement me!
    #warning Implement me!

#endif


/*
                *     ,MMM8&&&.            *
        *           MMMM88&&&&&    .                       *       *
           *       MMMM88&&&&&&&      *
    *              MMM88&&&&&&&&                   *
             *     MMM88&&&&&&&&           *           *
                  'MMM88&&&&&&'               *               *
        *            'MMM8&&&'      *    _
         |\___/|                      \\                *
   *    =) ^Y^ (=   |\_/|              ||    '      *
         \  ^  /    )a a '._.-""""-.  //                *       *
    *     )=*=(    =\T_= /    ~  ~  \//             *
         /     \     `"`\   ~   / ~  /    *                 *
  *      |     |         |~   \ |  ~/                               *
        /| | | |\         \  ~/- \ ~\           *
        \| | |_|/|        || |  // /`
 /_\_/\_//_// __//\_/\_/\_((_|\((_//\_/\_/\_
 |  |  |  | \_) |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |                    |                    |
*/
typedef unsigned char byte;
typedef unsigned char Byte;

// Default directory & file names.
#define DEFAULT_EXEC_PATH                       "Debug"

// Command headers.
const Byte   COMMANDHEADER_ACK = 22;
const Byte   COMMANDHEADER_MOVE = 23;
const Byte   COMMANDHEADER_SERVERACK = 21;
const Byte   COMMANDHEADER_SERVERENTITY = 24;
const Byte      COMMANDHEADER_INITSTATE = 27;


// TinyThread++
//#include "tinythread.h"
// Moved.

#endif

/*          *           *         *             *   *
        *        *    ,MMM8&&&&.            *      *
                    MMMM88&&&&&&    .                         *
                   MMMM88&&&&&&&&
        *           MMM88&&&&&&&&&         *
             *      MMM88&&&&&&&&&  *
                    'MMM88&&&&&&'             *
          *           'MMM8&&&'      *    *       *
         |\___/|     /\___/\
    *    )     (     )    ~( .      *       '
        =\     /=   =\~    /=
    *     )===(       ) ~ (     *               *
         /     \     /     \        *
         |     |     ) ~   (            *
        /       \   /     ~ \   *               *
        \       /   \~     ~/
 _/\_/\_/\__  _/_/\_/\__~__/_/\_/\_/\_/\_/\_
 |  |  |  |( (  |  |  | ))  |  |  |  |  |  |
 |  |  |  | ) ) |  |  |//|  |  |  |  |  |  |
 |  |  |  |(_(  |  |  (( |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |\)|  |  |  |  |  |  |
 |                    |                    |
*/


