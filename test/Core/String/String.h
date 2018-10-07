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
//  String.h
//  String class implementation.
//
//  This file is a part of Neko engine.
//  Created by Neko Code on 2/17/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__String__
#define __Nanocat__String__

// For istream and ostream.
//#include <fstream>
//#include <iostream>

#include "../../Platform/Shared/SystemShared.h"
#include "../../Math/GameMath.h"

namespace Neko {
    
    ///     String class.
    class CStr
    {
        //!  Standart buffer size.
        const static uint32_t MIN_BUFFER_SIZE = 32;
        
        /**
         *  Test whether a String object contains the same.
         */
        friend uint32_t operator == ( const CStr & one, const char * cstr )
        {
            uint32_t equal = 1;
            uint32_t stlen = 0;
            uint32_t i;
            
            // Find the length.
            while( cstr[stlen] != '\0' ) {
                ++stlen;
            }

            // If the lengths are equal.
            if( one._len == stlen ) {
                for( i = 0; i < one._len; ++i ) {
                    if( one.data[i] != cstr[i] ) {
                        equal = 0;
                    }
                }
            }
            else {
                equal = 0;
            }
            
            return equal;
        }
        /**
         *  Test whether two CStrs are equal ( compare ).
         */
        friend uint32_t operator == ( const CStr &one, const CStr& two )
        {
            uint32_t ret = 1;
            if( one._len != two._len ) {
                ret = 0;
            }
            else
            {
                uint32_t i;
                
                for ( i = 0; i < one._len && ret == 1; ++i ) {
                    if( one.data[i] != two.data[i] ) {
                        ret = 0;
                    }
                }
            }

            return ret;
        }

        /**
         *  Test whether a string contains another string.
         */
        friend uint32_t operator == ( const char * cstr, const CStr& one )
        {
            uint32_t equal = 1;
            uint32_t stlen = 0;

            // Find the length of the array of characters.
            while( cstr[stlen] != '\0' ) {
                ++stlen;
            }

            // If the lengths are equal..
            if( one._len == stlen ) {
                uint32_t i;
                for( i = 0; i < one._len; ++i ) {
                    if( one.data[i] != cstr[i] ) {
                        equal = 0;
                    }
                }
            }
            else {
                equal = 0;
            }

            return equal;
        }

    public:
        
        // Constructors and destructor.
        CStr();


        /**
         *  Constructor that initializes from an array of characters.
         */
        CStr( const char * cstr );

        /**
         *  Copy string constructor.
         */
        CStr( const CStr & old );

        /**
         *  String destructor.
         */
        ~CStr();


        // Accessors.

        /**
         *  Get string length.
         */
        inline const size_t             Length() const  {   return _len;    }

        /**
         *  Return the character at the given position.
         */
        inline uint32_t                 At( int32_t position, char & ch ) const
        {
            uint32_t returnCode = 0;
            
            if( position >= 0 && position < _len )
            {
                ch = data[position];
                returnCode = 1;
            }

            return returnCode;
        }

        /**
         *  Find a string.
         */
        int32_t             Find( int32_t posn, const char * str, int len = -1 )
        {
            if( str == NEKO_NULL ) {
                return -1;
            }
            
            if( len == -1 ){
                len = (int) strlen(str);
            }
            
            // check every position for match
            for( ; posn <= _len - len; posn++ )
            {
                if( strncmp( data + posn, str, len ) == 0 ) {
                    return posn;
                }
            }
            
            return -1;
        }
        
        /**
         *   Find character and return index of found character in string.
         */
        uint32_t                Find( char ) const;

        //  Mutators.
        
        /**
         *   Copy two strings.
         */
        CStr& operator = ( const CStr & two );
        
        /**
         *  Copy character array.
         */
        const CStr & operator = ( const char *two );

        /**
         *  Concatenate a CStr.
         */
        CStr & operator + ( const CStr & two ) const;
        
        //!  Some operations.
        
        inline operator const char * () const   {   return data;    }
        inline char& operator [] ( unsigned x ) {   return data[x]; }
        inline const char& operator [] ( unsigned x ) const {   return data[x]; }
        inline operator const char () const {   return data[0]; }

        inline CStr & operator += ( const char c )
        {
            AppendChar( c );
            return *this;
        }
        
        void DeleteAt( int from,  int len)
        {
            from = nkMath::Max(0, nkMath::Min((int)_len, from));
            len = nkMath::Min((int)_len - from, len);
            if (len <= 0)
                return;
            
            memmove(data+from, data+from+len, _len-(from+len));
            _len -= len;
            data[_len] = '\0';
        }
        
        /**
         *  Get string data.
         */
        inline const char               * c_str() const { return data;    }

        /**
         *  Convert string to lowercase.
         */
        const char              * ToLowerCase()
        {
            uint32_t i;
            
            for( i = 0; i < _len; ++i ) {
                data[i] = (char)tolower( data[i] & 0xff );
            }
            
            return data;
        }
        
        /**
         *  Convert string to uppercase.
         */
        const char          * ToUpperCase()
        {
            uint32_t i;
            
            for( i = 0; i < _len; ++i ) {
                data[i] = (char)toupper( data[i] & 0xff );
            }
            
            return data;
        }
        
        /**
         *  Is it UTF8 string data?
         */
        inline bool             IsUTF8String() const
        {
            uint32_t    i;
            
            for( i = 0; i < _len; ++i ) {
                if( (data[i] % 0x80) != 0 ) {
                    return true;
                }
            }
            
            return false;
        }
        
        /**
         *  Append a string.
         *
         *  @param string <#string description#>
         */
        void        AppendString( const char * string )
        {
            uint32_t s, prev = _len;
            
            s = (uint32_t)strlen( string );
            
            if( (s + _len) + 1 > _datasize ) {
                resize( s + _len);
            }
            
            smemcpy( &data[prev], string, s + 1 );
            
            _len = s + _len;
        }
        
        /**
         *  Append a character.
         *
         *  @param string <#string description#>
         */
        void        AppendChar( const char c )
        {
            if( _len + 2 > _datasize ) {
                resize( 2 + _len);
            }
            
            data[_len] = c;
            
            _len = 1 + _len;
            data[_len] = '\0';  // keep null terminated
        }
        
        /**
         *  Append a string with parameters
         *
         *  @param string <#string description#>
         *  @param ...    <#... description#>
         */
        void        Append( const char * string, ... )
        {
            char data[2048];
            
            va_list args;
            va_start( args, string );
            vsprintf( data, string, args );
            va_end(args);
            
            AppendString( (const char*)data );
        }
        
        /**
         *  Clear a string.
         */
        void        Clear()
        {
            if( data != NEKO_NULL ) {
                free( data );
            }
            
            /* Temporary solution!! Allocate me using Neko memory manager!! */
            data  = (char*)malloc( sizeof( char ) * 32 );
            data[0] = 0;
            
            _datasize = 32;
            _len = 0;
        }
        
        // reset the buffer to length 0
        void Empty()
        {
            _len = 0;
            data[_len] = '\0';
        }
        
    private:
        
        /**
         *  Small memory copy for C string data.
         */
        void                * smemcpy( void* dst, const void* src, const unsigned long len );
        
        /**
         *  Resize string.
         */
        void                resize( unsigned newsize );

        //!  Original data.
        char    * data;
        //!  Data length.
        uint32_t    _len;
        //!  Data size.
        uint32_t    _datasize;
    };
}

#endif /* defined(__Nanocat__String__) */
