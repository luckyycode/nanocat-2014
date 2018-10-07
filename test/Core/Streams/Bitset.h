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
//  Bitset.h
//  Bitset library. :D
//
//  Created by Kawaii Neko on 8/23/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef Bitset_h
#define Bitset_h

namespace Neko {
    
    /**
     *   Bitset.
     */
    class ncBitset {
    private:
        
        /**
         *  Bitset size.
         */
        size_t Size;
        
        /**
         *  Bitset data.
         */
        Byte * Bits;
        
    public:
        
        /**
         *  Create new bitset.
         */
        inline bool Create( unsigned int num ) {
            if( Bits )
                delete [] Bits;
            
            Bits = NEKO_NULL;
            Size = ( num >> 3 ) + 1;
            Bits = new Byte[Size];
            
            if( !Bits ) {
                return false;
            }
            
            memset( Bits, 0x00, Size );
            
            return true;
        }
        
        /**
         *  Clear all data.
         */
        inline void ClearAll() {
            memset( Bits, 0x00, Size );
        }
        
        /**
         *  Clear bit at index.
         */
        inline void Clear( unsigned int num ) {
            Bits[num >> 3] &= ~(1 << (num & 7));
        }
        
        /**
         *  Set bit at index.
         */
        inline void Set( unsigned int num ) {
            Bits[num >> 3] |= 1 << (num & 7);
        }
        
        /**
         *  Fill all bits.
         */
        inline void SetAll( void ) {
            memset( Bits, 0xFF, Size );
        }
        
        /**
         *  Delete our bitset.
         */
        inline void Delete( void ) {
            ClearAll();
            Size = 0;
            
            delete [] Bits;
        }
        
        /**
         *  Check if bit is set.
         */
        inline const Byte IsSet( unsigned int num ) {
            return Bits[num >> 3] & 1 << (num & 7);
        }
        
        inline const size_t GetSize() {
            return Size;
        }
    };

}

#endif /* Bitset_h */
