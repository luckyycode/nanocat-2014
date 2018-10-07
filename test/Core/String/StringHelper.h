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
//  NekoCString.h
//  C String helper.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef CString_h
#define CString_h

//#include "../Console/Console.h"
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <ctype.h>

#	if defined( _MSC_VER )
	#define _CRT_SECURE_NO_WARNINGS
#	endif

//
//  C-style String helper.
//
//  TODO: std::string alternative. - DONE!
//
namespace Neko {

    /**
     *  String helper methods.
     */
    namespace NekoCString
    {
        const static uint32_t BUFFER_COUNT  =	4;
        const static uint32_t BUFFER_SIZE	 =	4096;
        
        //! String buffer.
        static char strBuffer[BUFFER_COUNT][BUFFER_SIZE];
        static uint32_t nextBufferIndex = 0; //! Current string buffer.
        
        //! Custom string print buffer size.
        static const uint32_t MAX_SPRINTF_BUFFER = 16384;
        
        //! Tokenizer parameters.
        static char * lastCommandArguments[256];
        static char tokenizedCommand[8092];
        
        /**
         *  Tokenize a string.
         */
        static int32_t TokenizeString( const char *buffer, const char delimiter = ' ' )
        {
            const char  * text;
            char        * textOut;
            int32_t     argumentCount;
            
            argumentCount = 0;
            
            text = buffer;
            textOut = tokenizedCommand;
            
            while ( true )
            {
                // Find the argument before spacing.
                while ( true ) {
                    while ( *text /* end of the text */ && *text <= delimiter /* delimiter */ ) {
                        ++text; // Character position.
                    }
                    
                    break;
                }
                
                // Quote argument parsing.
                if ( *text == '"' ) {
                    lastCommandArguments[argumentCount] = textOut;
                    
                    ++argumentCount;
                    ++text;
                    
                    while ( *text && *text != '"' /* final quote symbol */ ) {
                        *textOut++ = *text++;
                    }
                    
                    *textOut++ = '\0';  // Terminate string.
                    
                    if ( !*text ) {
                        return argumentCount;
                    }
                    
                    ++text;
                    
                    continue;
                }
                
                lastCommandArguments[argumentCount] = textOut;
                ++argumentCount;
                
                while ( *text > delimiter ) {
                    if ( text[0] == '"' ) {
                        break;
                    }
                    
                    // Last tokenized string.
                    *textOut++ = *text++;
                }
                
                *textOut++ = '\0';
                
                if ( !*text ) {
                    return argumentCount;
                }
            }
            
            return argumentCount;
        }

        
        
        inline static char* rstrip(char* s)
        {
            char* p = s + strlen(s);
            while( p > s && isspace((unsigned char)(*--p)) ) {
                *p = '\0';
            }
            return s;
        }
        
        inline static char* lskip(const char* s)
        {
            while( *s && isspace((unsigned char)(*s)) ) {
                s++;
            }
            return (char*)s;
        }
        
        /**
         *  Null-terminated 'strncpy'
         */
        inline static char* strncpy0(char* dest, const char* src, size_t size)
        {
            strncpy( dest, src, size );
            dest[size - 1] = '\0';
            return dest;
        }
        
        
        inline static char* FindChars(const char* s, const char* chars)
        {
            while (*s && (!chars || !strchr(chars, *s))) {
                s++;
            }
            
            return (char*)s;
        }

        
        /**
         *  Compare string range.
         */
        inline static bool CompareRange( const char * line, const char * another, const uint32_t substr )
        {
            uint32_t    i;
            uint32_t    skips;
            
            skips = 0;
            
            for( i = 0; i < substr; ++i ) {
                if( line[i] == another[i] ) {
                    ++skips;
                }
            }
            
            if( skips == substr ) {
                return true;
            }
            
            return false;
        }
        
        /**
         *  Make boolean value to string.
         */
        inline static const char *const BoolToString( bool b ) {
            return b ? "true" : "false";
        }

        /**
         *  Copy string to destination..
         */
        inline static char * Copy( char * d, const char * s )
        {
            if( d == nullptr ) {
                return nullptr;
            }

            if( s == nullptr ) {
                return nullptr;
            }

            char * dest = d;

            while( *s ) {
                *d++ = *s++;
            }

            *d = 0;

            return dest;
        }

        /**
         *  Remove character from string.
         */
        inline static  void RemoveChar( char * str, char garbage )
        {
            char * src;
            char * dst;

            for( src = dst = str; *src != '\0'; src++ ) {
                *dst = *src;
                if( *dst != garbage ) {
                    dst++;
                }
            }

            *dst = '\0';
        }

        /**
         *  Safe copy string.
         */
        inline static bool SafeCopy( char * dest, const char * src, unsigned long size )
        {
            if ( src == nullptr ) {
                return false;
            }

            if ( size < 1 ) {
                return false;
            }

            strncpy( dest, src, size - 1 );
            dest[size - 1] = 0;

            return true;
        }

        /**
         *  Safe copy character array.
         */
        inline static void SPrintf( char * dest, unsigned long size, const char * fmt, ... )
        {
            va_list	argptr;
            static char	bigbuffer[MAX_SPRINTF_BUFFER];

            va_start( argptr, fmt );

            vsprintf( bigbuffer, fmt, argptr );

            va_end( argptr );

            // Copy now.
            SafeCopy( dest, bigbuffer, size );
        }

        /**
         *  Skip all next lines and return simple one lined string.
         */
        inline static void Chomp( char * s )
        {
            while( *s && *s != '\n' && *s != '\r' ) { s++; }

            *s = 0;
        }

        /**
         *  Skip the chosen character.
         */
        inline static void SkipCharacter( char * s, char a )
        {
            while(*s != a) { s++; }

            *s = 0;
        }

        /**
         *  Combine the parts in one string.
         */
        inline static const char * STR( const char * msg, ... )
        {
            va_list ap;

            char *dest = &strBuffer[nextBufferIndex][0];
            nextBufferIndex = (nextBufferIndex + 1) % BUFFER_COUNT;

            va_start( ap, msg );
            vsprintf( dest, msg, ap );
            va_end( ap );

            return dest;
        }

        /**
         *  Check if string contains next line.
         */
        inline static bool ContainsNextLine( const char * s )
        {
            int32_t i;
            int32_t len;
            
            len = (int32_t)strlen( s );

            for( i = 0; i < len; ++i ) {
                if( s[i] == '\n' ) {
                    return true;
                }
            }

            return false;
        }
    }

}
#endif
