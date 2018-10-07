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
//  MemoryStream.hpp
//  Memory stream implementation. :P
//
//  Created by Kawaii Neko on 8/23/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef MemoryStream_cpp
#define MemoryStream_cpp

#include "../Core.h" // non-copyable define.
#include "../../Platform/Shared/SystemShared.h"
#include "../Console/Console.h" // Logging.

namespace Neko {
    
    /**
     *  Memory seek flags.
     */
    enum memseek_t {
        MEM_SEEK_SET = 0,
        MEM_SEEK_CUR,
        MEM_SEEK_END
    };
    
    /**
     *  Memory stream.
     */
    class ncMemoryStream {
//        NEKO_NONCOPYABLE( ncMemoryStream );
        
    public:
        
        /**
         *  Create stream with existing data.
         */
        ncMemoryStream( Byte ** f );
        
        /**
         *  Create empty stream.
         */
        ncMemoryStream();
        
        /**
         *  Set memory stream source.
         */
        void SetSourceBuffer( Byte ** src );
        
        /**
         *  Read from defined stream.
         */
        unsigned long int Read( void *buf, size_t size, size_t n );
        
        /**
         *  Read from any byte stream.
         */
        unsigned long int Read( void *buf, size_t size, size_t n, Byte ** source );
        
    private:
        unsigned long int read_size;
        unsigned long int last_pos;
        
        Byte ** buffer;
    };

    
    extern char * str_readline( char * dst, uint32_t maxlen, const char * input, int32_t & charsRead );
    extern size_t MEM_fread(void *buf, size_t size, size_t n, Byte **f);
    extern char *sgets( char * str, int num, char **input );

}

#endif /* MemoryStream_cpp */
