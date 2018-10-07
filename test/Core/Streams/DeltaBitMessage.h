//
//  DeltaBitMessage.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/24/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef DeltaBitMessage_cpp
#define DeltaBitMessage_cpp

#include "BitMessage.h"
#include "../../Platform/Shared/SystemShared.h"

namespace Neko {
    
    //! Maximum deltastream storage size.
    static const int32_t   MAX_DELTASTREAM_SIZE = 4096;
    
    ///   Delta bit stream.
    class DeltaBitMessage
    {
    public:
        //! Bitstream storage.
        SBitMessage * _bitStream;
        SBitMessage * _baseStream;
        SBitMessage * _newBaseStream;
        
        /**
         *  Create a new delta bit stream.
         *
         *  @note Look for a 'MAX_DELTASTREAM_SIZE' property for default delta stream size.
         *  @param old - An existing bit stream.
         *  @param message - A message to write in.
         */
        DeltaBitMessage( SBitMessage * old, SBitMessage * message )
        {
            Byte defaultBuffer[MAX_DELTASTREAM_SIZE];
            static SBitMessage newStreamBase;  // Keeps in memory.
            
            newStreamBase.Init( defaultBuffer, sizeof(defaultBuffer) );
            newStreamBase.DataRead = 0; // Start reading from the beginning.
            
            // Create a new bitstream.
            if( old == NEKO_NULL ) {
                _baseStream = NEKO_NULL;
                _newBaseStream = &newStreamBase;
                _bitStream = message;
            } else {
                // Default state.
                old->DataRead = 0;
                // Keeps in memory too, but has new data.
                static SBitMessage newStream( old->Data, old->Size, true );
                
                _baseStream = &newStream;
                _baseStream->DataRead = 0;
                
                _newBaseStream = &newStreamBase;
                _bitStream = message;
            }
        }
        
        /**
         *  Write a boolean value.
         */
        inline void WriteBool( bool value )
        {
            _baseStream->ReadBool();
            _newBaseStream->WriteBool( value );
            _bitStream->WriteBool( value );
        }
        
        /**
         *  Write byte value.
         */
        inline void WriteByte( Byte value )
        {
            _newBaseStream->WriteByte( value );
            
            if( _baseStream != NEKO_NULL && value == _baseStream->ReadByte() ) {
                _bitStream->WriteBool( false );
            } else {
                _bitStream->WriteBool( true );
                _bitStream->WriteByte( value );
            }
        }
        
        /**
         *  Write an integer value.
         */
        inline void WriteInt16( int16_t value )
        {
            _newBaseStream->WriteInt16( value );
            
            if( _baseStream != NEKO_NULL && value == _baseStream->ReadInt32() ) {
                _bitStream->WriteBool( false );
            } else {
                _bitStream->WriteBool( true );
                _bitStream->WriteInt16( value );
            }
        }
        
        /**
         *  Write a float value.
         */
        inline void WriteFloat( float value ) {
            _newBaseStream->WriteFloat( value );
            
            if( _baseStream != NEKO_NULL && value == _baseStream->ReadFloat() ) {
                _bitStream->WriteBool( false );
            } else {
                _bitStream->WriteBool( true );
                _bitStream->WriteFloat( value );
            }
        }
        
        
        /**
         *  Read boolean value.
         */
        inline bool ReadBool()
        {
            bool value = _bitStream->ReadBool();
            _baseStream->ReadBool();
            _newBaseStream->WriteBool( value );

            return value;
        }
        
        /**
         *  Read byte value.
         */
        inline Byte ReadByte()
        {
            Byte value;
            
            if( _baseStream == NEKO_NULL ) {
                value = (Byte)0;
            } else {
                 value = _baseStream->ReadByte();
            }
            
            if( _bitStream->ReadBool() ) {
                value = _bitStream->ReadByte();
            }
            
            _newBaseStream->WriteByte( value );
            
            return value;
        }
        
        /**
         *  Read integer value.
         */
        inline int16_t ReadInt16()
        {
            int16_t value;
            
            if( _baseStream == NEKO_NULL ) {
                value = (int16_t)0 ;
            } else {
                value = _baseStream->ReadInt32();
            }
            
            
            
            if( _bitStream->ReadBool() ) {
                value = _bitStream->ReadInt32();
            }
            
            _newBaseStream->WriteInt16( value );
            
            return value;
        }
        
        /**
         *  Read float value.
         */
        inline float ReadFloat()
        {
            float value;
            
            if( _baseStream == NEKO_NULL ) {
                value = 0.0f ;
            } else {
                value = _baseStream->ReadFloat();
            }
            
            if( _bitStream->ReadBool() ) {
                value = _bitStream->ReadFloat();
            }
            
            _newBaseStream->WriteFloat( value );
            
            return value;
        }
    };
};

#endif /* DeltaBitMessage_cpp */
