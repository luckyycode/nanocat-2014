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
//  Hashmap.h
//  Hash map.
//
//  This file is a part of Neko engine.
//  Created by Neko Code on 2/11/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__Hashtable__
#define __Nanocat__Hashtable__

#include "List.h"
#include "../../Platform/Shared/SystemShared.h"
#include "../GameMemory.h"

namespace Neko {
    // #define HASHTABLE_USE_SIMPLE_HASH
    // #define SIMPLE_ALGORITHM
    
    //
    // Works kinda like as std::map alternative, but it's not. Haha
    
    //  Currently works only with string keys - Improve me. Also works great.
    
    ///   Hashtable implementation.
    template <typename t1, typename t2>
    class CHashMap
    {
    private:
        struct _hashentry
        {
            t1 key;
            t2 value;
            
            SLink m_Link;
            
            struct _hashentry * next;
        };
        
        typedef struct _hashentry hashentry_t;
        
        struct hashtable_s
        {
            size_t size;
            struct _hashentry **table;
        };
        
        
        typedef struct hashtable_s hashtable_t;
        
        ///   Easier access to table owner as array operator.
        template <class k, class v>
        class mapproxy
        {
            CHashMap<k, v> &h;
            k key;
            
        public:
            mapproxy( CHashMap<k, v> &h, k key ) : h(h), key(key) { }
            
            operator v() const
            {
                v pos = h.Get( key );
                if( pos ) {
                    return pos;
                } else {
                    return NEKO_NULL;
                }
            }
            
            mapproxy & operator = ( v const &value )
            {
                h.Set( key, value );
                return *this;
            }
        };
        
    public:
        /**
         *  Array accessor via string.
         */
        mapproxy<t1,t2> operator [] ( const t1& k )
        {
            return mapproxy<t1,t2>( *this, k );
        }
        
        /**
         *  Access to elements via indexing.
         */
        t1 operator [] ( const size_t k )
        {
            return (t1)m_List.m_sList[k];
        }
        
        INekoAllocator  *   pAllocator;
        //! Used *only* to deallocate. It can not to be stack or any other allocator with LIFO order!
        INekoAllocator  *   pOriginalAllocator;
        
        //typedef unsigned (*hashtype)(const t1&);
        //static hashtype HashKey;
        
        /**
         *  Constructor.
         */
        CHashMap()
        {
            _owner = NEKO_NULL;
            _size = -1;
        }
    
        /**
         *  Destructor.
         */
        ~CHashMap()
        {
            _size = -1;
            _owner = NEKO_NULL;
        }
        
        /**
         *  Create a new hashtable.
         */
        bool Create( const size_t size, INekoAllocator * poolAllocator, INekoAllocator * allocator )
        {
            size_t i;
            
            if( size < 1 )  {
                return false;
            }
            
            pOriginalAllocator = poolAllocator;
            pAllocator = allocator;
            
            // Allocate the table itself.
            _owner = (hashtable_t *) pAllocator->Alloc( sizeof( hashtable_t ) );

            if( _owner == NEKO_NULL ) {
                return false;
            }
            
            // Allocate pointers to the head nodes.
            _owner->table = (hashentry_t **) pAllocator->Alloc( sizeof(hashentry_t*) * size );
            
            if( _owner->table == NEKO_NULL ) {
                return false;
            }
            
            for( i = 0; i < size; ++i ) {
                _owner->table[i] = NEKO_NULL;
            }
            
            _owner->size = size;
            _size = size;
            
            SList::CreateList( &m_List );
            
            return true;
        }
        
        /**
         *  Delete table and its nodes.
         */
        void Delete()
        {
            if( !_owner ) {
                return;
            }
            
            if( GetSize() <= 0 ) {
                return;
            }
            
            if( !_owner->table ) {
                return;
            }
            
            size_t i;
            
            for( i = 0; i < _size; ++i ) {
                _owner->table[i] = NEKO_NULL;
            }
            
            CLinearAllocator * allocator = (CLinearAllocator *)pAllocator;
            allocator->Reset();
            
            SList::CreateList( &m_List );
            memset( &m_List, 0x00, sizeof(SList) );
            
            _size = -1;
        }
        
        /**
         *  Get table size.
         */
        inline const size_t                 GetSize() const {       return _size;   }
        
        /**
         *  Get table element count.
         */
        inline const size_t                 GetCount() const    {       return _pairsize;   }
        
        /**
         *  Hashtable.
         */
        inline hashtable_t *                GetOwner() {       return _owner;  }
  
        /**
         *  Get linked-list.
         */
        inline SList *              GetList() {     return &m_List; }
        
//    protected:
        //!  Owner Table.
        hashtable_t *_owner;
        
        //!  Table size.
        size_t _size;
        
        //!  Actual table size.
        size_t _pairsize;
       
        SList       m_List;
        
    private:

        /**
         *  Hash a string key for a particular hash map.
         */
        size_t hash_find( t1 key )
        {
            if( !_owner ) {
                return -1;
            }
            
            if( !key ) {
                return -1;
            }
            
#   if defined( SIMPLE_ALGORITHM )
            unsigned long int hashval = 0;
            int32_t i = 0;
            
            while( hashval < ULONG_MAX && i < strlen( key ) ) {
                hashval = hashval << 8;
                hashval += key[ i ];
                ++i;
            }
            
            return hashval % _owner->size;
#   else
            size_t hash, i;
            
#   if defined( HASHTABLE_USE_SIMPLE_HASH )
            for ( hash = i = 0; i < strlen(key); hash = hash << 8, hash += key[ ++i ] );
#   else   // Use Jenkins' "One At a Time Hash" Perl "Like" Hashing.
            // http://en.wikipedia.org/wiki/Jenkins_hash_function
            for ( hash = i = 0; i < strlen(key); ++i ) {
                hash += key[i], hash += ( hash << 10 ), hash ^= ( hash >> 6 );
            }
            hash += ( hash << 3 ), hash ^= ( hash >> 11 ), hash += ( hash << 15 );
#   endif
            
            return hash % _owner->size;
#   endif
        }
        
        /**
         *  Create a key-value pair.
         */
        hashentry_t *hash_newpair( t1 key, t2 value )
        {
            hashentry_t * newpair;
            
            if( ( newpair = (hashentry_t *)pAllocator->Alloc(sizeof(hashentry_t) ) ) == NEKO_NULL ) {
                return NEKO_NULL;
            }
            
            if( ( newpair->key = (char*) new char[strlen( key ) + 1] ) == NEKO_NULL ) {
                return NEKO_NULL;
            }
            
            //if( ( newpair->key = new char[strlen(key)] ) == NEKO_NULL ) {
            //    return NEKO_NULL;
            //}
            
            memcpy( (char*)newpair->key, key, strlen(key) + 1 );
            
            if( ( newpair->value = value ) == NEKO_NULL ) {
                return NEKO_NULL;
            }
            
            newpair->next = NEKO_NULL;
            
            // add a new link
            SList::AddHead( &m_List, &newpair->m_Link, (void *)newpair->key );
            
            return newpair;
        }
        
        /**
         *  Insert a key-value pair into a hash table.
         */
        void Set( t1 key, t2 value )
        {
            size_t bin = 0;
            
            hashentry_t *newpair = NEKO_NULL;
            hashentry_t *next = NEKO_NULL;
            hashentry_t *last = NEKO_NULL;
            
            bin = hash_find( key );
            
            next = _owner->table[ bin ];
            
            while( next != NEKO_NULL && next->key != NEKO_NULL && strcmp( key, next->key ) > 0 ) {
                last = next;
                next = next->next;
            }
            
            // There's already a pair. Replace it.
            if( next != NEKO_NULL && next->key != NEKO_NULL && strcmp( key, next->key ) == 0 ) {
                
//                delete next->value;
                pOriginalAllocator->Dealloc( next->value );
                
                next->value = value;
                
                // Could't find it. Make a new pair.
            } else {
                newpair = hash_newpair( key, value );
                
                // Start of list.
                if( next == _owner->table[ bin ] ) {
                    newpair->next = next;
                    _owner->table[ bin ] = newpair;
                    
                    // End of list.
                } else if ( next == NEKO_NULL ) {
                    last->next = newpair;
                    
                    // Middle of the list
                } else  {
                    newpair->next = next;
                    last->next = newpair;
                }
                
                ++_pairsize;
            }
        }
        
        /**
         *  Get key-value pair from a hash table.
         */
        t2 Get( t1 key )
        {
            size_t bin = 0;
            hashentry_t *pair;
            
            bin = hash_find( key );
            
            // looking for our value.
            pair = _owner->table[ bin ];
            while( pair != NEKO_NULL && pair->key != NEKO_NULL && strcmp( key, pair->key ) > 0 ) {
                pair = pair->next;
            }
            
            // Found anything?
            if( pair == NEKO_NULL || pair->key == NEKO_NULL || strcmp( key, pair->key ) != 0 ) {
                return NEKO_NULL;
            } else {
                return pair->value;
            }
        }
    };
}

#endif /* defined(__Nanocat__Hashtable__) */
