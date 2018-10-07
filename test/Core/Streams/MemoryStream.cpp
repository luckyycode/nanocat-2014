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
//  MemoryStream.cpp
//  Memory stream implementation. :P
//
//  Created by Kawaii Neko on 8/23/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "MemoryStream.h"

namespace Neko {
    
    /**
     *  Create stream with existing data.
     */
    ncMemoryStream::ncMemoryStream( Byte ** f ) : buffer(f), last_pos(0), read_size(0)
    {
        
    }
    
    /**
     *  Create empty stream.
     */
    ncMemoryStream::ncMemoryStream() : buffer(NEKO_NULL), last_pos(0)
    {
        
    }
    
    /**
     *  Set memory stream source buffer.
     */
    void ncMemoryStream::SetSourceBuffer( Byte ** src )
    {
        // A new buffer is about to set, so reset read values.
        last_pos = 0;
        read_size = 0;
        
        buffer = src;
    }
    
    /**
     *  Read from defined stream.
     */
    unsigned long int ncMemoryStream::Read( void *buf, size_t size, size_t n )
    {
        if( buffer == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "MemoryStream::Read() while buffer is not set.\n" );
            return -1;
        }
        
        unsigned long int to_read = size * n;
        memcpy( buf, (*buffer + last_pos), to_read );
        last_pos += to_read;
        
        return n;
    }
    
    /**
     *  Read from any byte stream.
     */
    unsigned long int ncMemoryStream::Read( void *buf, size_t size, size_t n, Byte ** source )
    {
        unsigned char * pBuffer = *source;
        
        unsigned long int to_read = size * n;
        memcpy( buf, (pBuffer + last_pos), to_read );
        last_pos += to_read;
        
        return n;
    }
    
    /**
     *  Memory stream alternative for 'fgets'.
     */
    char *sgets( char * str, int num, char **input )
    {
        char *next = *input;
        int  numread = 0;
        
        while ( numread + 1 < num && *next )
        {
            int32_t isnewline = ( *next == '\n' );
            *str++ = *next++;
            numread++;
            // newline terminates the line but is included
            if ( isnewline )
                break;
        }
        
        if ( numread == 0 )
            return NEKO_NULL;  // "eof"
        
        // must have hit the null terminator or end of line
        *str = '\0';  // null terminate this tring
        // set up input for next call
        *input = next;
        return str;
    }
    
    
    /**
     *  Read line from string stream.
     */
    char * str_readline( char * dst, uint32_t maxlen, const char * input, int32_t & charsRead )
    {
        int32_t c;
        char * p = NEKO_NULL;
        
        c = 0;
        for( p = dst, --maxlen; maxlen > 0; --maxlen ) {
            char c = input[charsRead];
            if( c == '\0' ) {
                break;
            }
            
            *p++ = c;
            ++charsRead;
            
            if( c == '\n' ) {
                break;
            }
        }
        
        *p = 0;
        
        if( p == dst || c == EOF ) {
            return NEKO_NULL;
        }
        
        return (p);
    }
    
    
    /**
     *      Memory stream read.
     */
    size_t MEM_fread( void *buf, size_t size, size_t n, Byte **f)
    {
        memcpy(buf, *f, size * n);
        *f += size * n;
        return n;
    }
    
    
    /**
     *     Memory seek.
     */
    int MEM_fseek(void *buf, size_t position, memseek_t wh )
    {
        unsigned int newpos;
        
        switch (wh)
        {
            case MEM_SEEK_SET:
//                newpos = (int) position;
                break;
                
            case MEM_SEEK_CUR:
                //newpos = (int) (stream->position + position);
                break;
                
            case MEM_SEEK_END:
                // newpos = (int) (stream->buflen + position);
                break;
            default:
                return -1;
        }
        
        return 1;
    }
}
