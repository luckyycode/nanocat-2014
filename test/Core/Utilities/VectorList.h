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
//  VectorList.h
//  Neko engine
//
//  This file is a part of Neko engine.
//  Created by Neko Code on 2/11/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef VectorList_h
#define VectorList_h

#include "../../Platform/Shared/SystemShared.h"
#include "../GameMemory.h" // Neko Memory management.

namespace Neko {
    
    static const int32_t VECTORLIST_INIT_CAPACITY = 4;
    
    ///  Vector array.
    template<class t>
    class CVectorList
    {
    public:
        
        /**
         *  Array operator access.
         */
        t operator [] ( const size_t k ) const
        {
            if( _items == NEKO_NULL ) {
                return NEKO_NULL;
            } else {
                return _items[k];
            }
        }
        
        /**
         *  Create empty vector list.
         */
        inline void Create( const size_t size, INekoAllocator * allocator = NEKO_NULL )
        {
            pAllocator = allocator;
            
            _size = size;
            _count = 0;
            _items = (t*)pAllocator->Alloc( sizeof( t ) * _size ); /*malloc*/
        }
        
        /**
         *  Resize vector list.
         */
        inline void Resize( const size_t capacity )
        {
            t * items;
            
            items = (t*)pAllocator->Alloc( sizeof( t ) * capacity ); /*malloc*/
            size_t i;
            
            for( i = 0; i < GetCount(); ++i ) {
                items[i] = _items[i];
            }
            
            pAllocator->Dealloc( _items ); /*free*/
            
            if( items != NEKO_NULL ) {
                _items = items;
                _size = capacity;
            }
        }
        
        /**
         *  Sort items.
         */
        inline void Sort( int32_t ( *compareFunc)(const void*, const void*) )
        {
            qsort( _items, _count, sizeof(void*), compareFunc );
        }
        
        /**
         *  Get vector list count.
         */
        inline const size_t                 GetCount() const {     return _count; }
        
        /**
         *  Get vector list total size.
         */
        inline const size_t                 GetSize() const {     return _size; }
        
        /**
         *  Add new element to vector.
         */
        inline void PushBack( t val )
        {
            if( _size == _count ) {
                Resize( _size * 2 );
            }
            
            _items[_count] = val;
            _count++;
        }
        
        /**
         *  Remove from the end of array.
         */
        inline const void * Pop()
        {
            if( _count == 0 ) {
                return NEKO_NULL;
            }
            
            const void * result = _items[_count - 1];
            --_count;
            
            return result;
        }
        
        /**
         *  Set element in vector list.
         */
        inline void Set( const int idx, t * val )
        {
            if( idx >= 0 && idx < _count ) {
                _items[idx] = val;
            }
        }
        
        /**
         *  Get element in vector list.
         */
        inline t * Get( const int32_t idx ) const
        {
            if( idx >= 0 && idx < _count ) {
                return _items[idx];
            }
            // else...
            return NEKO_NULL;
        }
        
        /**
         *  Delete element at index in vector list.
         */
        inline void Delete( const int32_t idx )
        {
            if( idx < 0 || idx >= _count ) {
                return;
            }
            
            _items[idx] = NEKO_NULL;
            
            size_t i;
            
            for( i = 0; i < _count - 1; ++i ) {
                _items[i] = _items[i + 1];
                _items[i + 1] = NEKO_NULL;
            }
            
            --_count;
            
            if( _count > 0 && _count == _size / 4 ) {
                Resize( _size / 2 );
            }
        }
        
        /**
         *  Cleanup vector list.
         */
        inline void Clear()
        {
            pAllocator->Dealloc( _items );/*free*/
            _items = NEKO_NULL;
            
            _count = -1;
            _size = -1;
        }
        
        /**
         *  Cleanup vector list.
         */
        inline void                 ClearAll()  {       _count = 0;     }
        
    protected:
    private:
       
        INekoAllocator  * pAllocator = 0;   // linear by always
        
        //!  Vector elements.
        t * _items;
        
        //!  Vector total size and count.
        size_t _size /* total size */, _count /* added items */;
    };
}

#endif /* VectorList_h */
