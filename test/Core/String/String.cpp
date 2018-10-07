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
//  String.cpp
//  String class implementation.
//
//  Created by Neko Code on 2/17/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#include "String.h"
#include "../Core.h" // Memory string allocations.

//  Custom string class implementation.
//
//
// Don't use temporary memory patterns on strings!!
// TODO: permanent rare-use case memory pool.
// dafak o.o

namespace Neko {
    
    /**
     *  Constructor.
     */
    CStr::CStr() : _datasize( 32 ), _len( 0 )
    {
        
        /* Temporary solution!! Allocate me using Neko memory manager!! */
        data  = (char*)malloc( sizeof( char ) * 32 );
        data[0] = 0;
        
        _datasize = 32;
        _len = 0;
    }
    
    CStr::~CStr()
    {
        free( data );// [] data;
    }
    
    /**
     *  Equal ( mutator ) operator to another CStr struct.
     */
    CStr& CStr::operator = ( const CStr & two )
    {
        if( data != 0 && _len > 0 )
        {
            free( data );
        }
        
        int32_t     i;
        
        _len = two._len;
        data = (char*)malloc( sizeof( char ) * _len );
        
        for( i = 0; i < _len; ++i )
        {
            data[i] = two.data[i];
        }
        
        return *this;
    }
    
    /**
     *  Equal ( mutator ) operator to a C string.
     */
    const CStr & CStr::operator = ( const char *two )
    {
        
        uint32_t s;
        
        s = (uint32_t)strlen( two );
        
        if( s + 1 > (uint32_t)_datasize ) {
            resize( s << (1 - _datasize) );
        }
        
        smemcpy( data, two, s + 1 );
        
        _len = s;
        return *this;
        
        //return *this;
    }
    
    /**
     *  Concatenate a string.
     */
    CStr & CStr::operator + ( const CStr & two ) const
    {
        static CStr result;
        int32_t i;
        
        // If both strings are empty...
        if( _len == 0 && two._len == 0 )
        {
            result._len = 0;
            result.data = NEKO_NULL;
        }
        else
        {
            int32_t j;
            
            // Allocate new string.
            result._len = _len + two._len;
            result.data = (char*)malloc( sizeof( char ) * result._len );
            
            // Copy new string to allocated string.
            for( i = 0; i < _len; ++i )  {
                result.data[i] = data[i];
            }
            
            j = 0;
            
            // Copy the new string to the end.
            while( j < two._len )
            {
                result.data[i] = two.data[j];
                
                ++i;
                ++j;
            }
        }
        
        return result;
    }

    
    /**
     * Constructor that initializes from an array of characters.
     */
    CStr::CStr( const char * cstr )
    {
        uint32_t s;
        
        s = (uint32_t)strlen( cstr );
        data = (char*)malloc( sizeof( char ) * (s + MIN_BUFFER_SIZE) );
        
        smemcpy( data, cstr, s + 1 );
        
        _datasize = s + MIN_BUFFER_SIZE;
        _len = s;
    }
    
    /**
     *  Copy constructor.
     */
    CStr::CStr( const CStr & old )
    {
        // Check if we are copying the empty string.
        _len = old._len;
        if( _len == 0 )
        {
            data = NEKO_NULL;
        }
        else
        {
            int32_t     i;
            data = (char*)malloc( sizeof( char ) * _len );
            
            for( i = 0; i < _len; ++i )
            {
                data[i] = old.data[i];
            }
        }
    }
    
    /**
     *  String/small memory copy.
     */
    void * CStr::smemcpy( void * dst, const void * src, const unsigned long len )
    {
        if( !dst || !src || len == 0  )
        {
            return NEKO_NULL;
        }
        
        char* d = (char*)dst;
        char* s = (char*)src;
        
        unsigned long n = 0;

        while( n < len )
        {
            *d= *s;
            
            ++d;
            ++s;
            ++n;
        }
        
        return dst;
    }
    
    /**
     *  Resize string.
     */
    void CStr::resize( uint32_t newsize )
    {
        // Minimum buffer size.
        if( newsize < MIN_BUFFER_SIZE ) {
            newsize = MIN_BUFFER_SIZE;
        }
        
        char * tmp = (char*)malloc( sizeof( char ) * (_datasize + newsize) );

        if( data != NEKO_NULL )
        {
            smemcpy( tmp, data, _len + 1 );
            free( data );
        }
        
        data = tmp;
        _datasize += newsize;
    }
}