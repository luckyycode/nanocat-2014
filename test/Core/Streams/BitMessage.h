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
//  BitMessage.hpp
//  Bit message implementation. :)
//
//  Created by Kawaii Neko on 8/23/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef BitMessage_cpp
#define BitMessage_cpp

// Basic includes because I can't include shared header here.
typedef  unsigned char Byte;

#include <stdint.h>	// basic types
#include <stdio.h>	// printf
#include <memory>

#define NEKO_LOG printf

#   if !defined( NEKO_CONVERTER )
#include "../../Platform/Shared/SystemShared.h"
#   endif

namespace Neko {
    
    ///  Bit message.
    struct SBitMessage
    {
    public:
        
        /**
         *  Create empty bitmessage.
         */
        SBitMessage() : Size(0), MaxSize(0), Data(nullptr), DataRead(0)
        {
            
        }
        
        ~SBitMessage()
        {
            
        }
        
        /**
         *  Create bitmessage with existing data and size.
         */
        SBitMessage( const Byte * data, const size_t size, bool copy = false )
        {
            this->Data = (Byte*)data;
            MaxSize = size;
            
            AllowOverflow = true;
            Overflowed = false;
            
            if( !copy ) {
                Size = 0;
            } else {
                Size = size;
            }
        }
        
        
        /**
         *  Initialize bit message.
         */
        inline void Init( const Byte * data, const size_t size, bool copy = false )
        {
            this->Data = (Byte*)data;
            MaxSize = size;
            
            AllowOverflow = true;
            Overflowed = false;
            
            if( !copy ) {
                Size = 0;
            } else {
                Size = size;
            }
        }
        
        //!  Maximum message size.
        size_t		MaxSize;
        //!  Current message size.
        size_t		Size;
        //!  Data currently read.
        unsigned int     DataRead;
        //!  Is message corrupted?
        bool    Corrupted;
        //!  Allows overflow?
        bool    AllowOverflow;
        //!  Is message data overflowed?
        bool    Overflowed;
        //!  Message data.
        Byte    * Data;
        
        /**
         *  Clear bit message.
         */
        inline void Clear()
        {
            Size = 0;
            MaxSize = 0;
            Data = nullptr;
            DataRead = 0;
        }
        
        /**
         *  Create bit message.
         */
        inline void Create( const Byte * data, const unsigned int len )
        {
            MaxSize = len;
            Size = 0;
            Data = (Byte*)data;
        }
        
        /**
         *  Get available free size.
         */
        inline void * GetSpace( const unsigned int length )
        {
            void    *g_data;
            
            if( Size + length > MaxSize ) {
                if( !AllowOverflow ) {
                    NEKO_LOG( "BitMessage(): No overflow allowed.\n" );
                }
                
                if( length > MaxSize ) {
                    NEKO_LOG( "BitMessage(): Given length is more than its max allowed size." );
                }
                
                Overflowed = true;
                
                NEKO_LOG( "BitMessage(): Overflowed ( Now: %i MaxSize: %zu ) \n", Size + length, MaxSize );
                
                Clear();
            }
            
            g_data = Data + Size;
            Size += length;
            
            return g_data;
        }
        
        /**
         *  Write character.
         */
        inline void WriteChar( int c )
        {
            Byte    *g_buf;
            g_buf       = (Byte*)GetSpace( 1 );
            g_buf[0]    = c;
        }
        
        /**
         *  Write byte.
         */
        inline void WriteBool( bool c )
        {
            Byte    *g_buf;
            
            g_buf       = (Byte*)GetSpace( 1 );
            g_buf[0]    = (bool)c;
        }
        
        /**
         *  Write byte.
         */
        inline void WriteByte( int c )
        {
            Byte    *g_buf;
            
            g_buf       = (Byte*)GetSpace( 1 );
            g_buf[0]    = c;
        }
        
        /**
         *  Write signed integer ( 16 bits ).
         */
        inline void WriteInt16( int c )
        {
            Byte    *g_buf;
            
            g_buf       = (Byte*)GetSpace( 2 );
            g_buf[0]    = c & 0xff;
            g_buf[1]    = c >> 8;
        }
        
        /**
         *  Write signed integer ( 16 bits ).
         */
        inline void WriteInt16( int c, int bits )
        {
            Byte    *g_buf;
            
            g_buf       = (Byte*)GetSpace( bits );
            g_buf[0]    = c & 0xff;
            g_buf[1]    = c >> 8;
        }
        
        /**
         *  Write long.
         */
        inline void WriteLong( int c )
        {
            Byte    *g_buf;
            
            g_buf       = (Byte*)GetSpace( 4 );
            g_buf[0]    = c & 0xff;
            g_buf[1]    = (c >> 8) & 0xff;
            g_buf[2]    = (c >> 16) & 0xff;
            g_buf[3]    = c >> 24;
        }
        
        // Main write function for Bitstream.
#pragma mark - TODO: fail checks.
        /**
         *  Write data to bit message.
         */
        inline void Write( void *data, int length )
        {
            if( !memcpy( GetSpace( length ), data, length ) ) {
                NEKO_LOG( "BitMessage::Write() failed to copy data." );
            }
        }
        
        /**
         *  Write float value.
         */
        inline void WriteFloat( float f )
        {
            union {
                float   f;
                int     l;
            } rev;
            
            rev.f = f;
            rev.l = rev.l;
            
            Write( &rev.l, 4 );
        }
        
        /**
         *  Write string.
         */
        inline void WriteString( const char * s )
        {
            if ( !s ) {
                // Empty string, skip it
                // and write empty space to keep something.
                Write( (Byte*)"", 1 );
            } else {
                // Ignore this damned warning.
#pragma mark - Ignore this damned warning.
                Write( (Byte*)s, (unsigned int)(strlen(s) + 1) );
            }
        }
        
        /**
         *  Write coordinate.
         */
        inline void WriteCoord( float f )
        {
            WriteByte( ((int)f * 256 / 360) & 255 );
        }
        
        /**
         *  Write angle.
         */
        inline void WriteAngle( float f )
        {
            WriteByte( ((int)f * 256 / 360) & 255 );
        }
        //void WriteData( byte *data, int length );
        //void ReadData( byte **data, int length );
        
        /**
         *  Start reading data from bit message.
         */
        inline void BeginReading()
        {
            DataRead = 0;
            Corrupted = false;
        }
        
        /**
         *  Read character.
         */
        inline int ReadChar()
        {
            int c;
            
            if( DataRead + 1 > Size ) {
                Corrupted = true;
                return -1;
            }
            
            c = (signed char)Data[DataRead];
            ++DataRead;
            
            return c;
        }
        
        /**
         *  Read boolean.
         */
        inline bool ReadBool()
        {
            bool c;
            
            if( DataRead + 1 > Size ) {
                Corrupted = true;
                return -1;
            }
            
            c = (bool)Data[DataRead];
            ++DataRead;
            
            return c;
        }
        
        /**
         *  Read byte.
         */
        inline int32_t ReadByte()
        {
            int32_t     c;
            
            if( DataRead + 1 > Size ) {
                Corrupted = true;
                return -1;
            }
            
            c = (Byte)Data[DataRead];
            ++DataRead;
            
            return c;
        }
        
        /**
         *  Read integer.
         */
        inline int32_t ReadInt32()
        {
            int32_t c;
            
            if( DataRead + 2 > Size ) {
                Corrupted = true;
                return -1;
            }
            
            c = (short)( Data[DataRead] + (Data[DataRead + 1] << 8) );
            
            DataRead += 2;
            
            return c;
        }
        
        /**
         *  Read integer.
         */
        inline int32_t ReadInt32( int bits )
        {
            int32_t c;
            
            if( DataRead + bits > Size ) {
                Corrupted = true;
                return -1;
            }
            
            c = (short)( Data[DataRead] + (Data[DataRead + 1] << 8) );
            
            DataRead += bits;
            
            return c;
        }
        
        /**
         *  Read long.
         */
        inline int ReadLong()
        {
            int c;
            
            if( DataRead + 4 > Size ) {
                Corrupted = true;
                return -1;
            }
            
            c = Data[DataRead] + (Data[DataRead + 1] << 8) + (Data[DataRead + 2] << 16) + (Data[DataRead + 3] << 24);
            
            DataRead += 4;
            
            return c;
        }
        
        /**
         *  Read float.
         */
        inline float ReadFloat()
        {
            union {
                Byte    b[4];
                float   f;
                int     l;
            } rev;
            
            rev.b[0] = Data[DataRead];
            rev.b[1] = Data[DataRead + 1];
            rev.b[2] = Data[DataRead + 2];
            rev.b[3] = Data[DataRead + 3];
            DataRead += 4;
            
            rev.l = rev.l;
            
            return rev.f;
        }
        
        /**
         *  Read string.
         */
        inline const char * ReadString()
        {
            static char g_string[2048];
            int l, c;
            
            l = 0;
            do {
                c = ReadChar();
                if( c == -1 || c == 0 ) {
                    break;    // Read until string terminate.
                }
                
                g_string[l] = c;
                
                l++;
            }
            while( l < sizeof(g_string) - 1 );
            
            g_string[l] = 0;
            
            return g_string;
        }
        
        /**
         *  Read coordinate.
         */
        inline float ReadCoord()
        {
            return ReadInt32() * (1.0 / 8);
        }
        
        /**
         *  Read angle.
         */
        inline float ReadAngle( void )
        {
            return ReadChar() * ( 360.0 / 256 );
        }
        
        
        /** Delta writters/readers. **/
        inline void WriteDeltaByte( Byte value, Byte old )
        {
            WriteBool( value != old );
            
            if( value != old ) {
                WriteByte( value );
            }
        }
        
        inline void WriteDeltaInt16( int16_t value, int16_t old )
        {
            WriteBool( value != old );
            
            if( value != old ) {
                WriteInt16( value );
            }
        }
        
        inline void WriteDeltaLong( int32_t value, int32_t old )
        {
            WriteBool( value != old );
            
            if( value != old ) {
                WriteLong( value );
            }
        }
        
        inline void WriteDeltaFloat( float value, float old )
        {
            WriteBool( value != old );
            
            if( value != old ) {
                WriteFloat( value );
            }
        }
        
        inline int16_t ReadDeltaInt16( int16_t old )
        {
            if( ReadBool() ) {
                return  ReadInt32();
            }
            
            return old;
        }
        
        
    };
}

#endif /* BitMessage_cpp */
