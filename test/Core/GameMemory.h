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
//  GameMemory.hpp
//  Neko engine memory management, sort of allocators :P
//
//  Created by Neko on 8/24/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef MemoryPool_cpp
#define MemoryPool_cpp

#include "../Platform/Shared/SystemShared.h"
#include <new>  // allocators redirect to new to call constructor (if required)

namespace Neko {
    // x64 long
#define ByteInMegabyte( Value ) ( Value / (1024LL * 1024LL) )
#define Kilobyte( Value ) ( Value * 1024LL )
#define Megabyte( Value ) ( Kilobyte( Value ) * 1024LL )
    
    
    ///  Game common memory block.
    struct GameMemory
    {
        //! Maximum memory block size to use.
        uint64_t iMemorySize;
        
        //! Memory left.
        uint64_t iMemoryLeft;
        
        //! Virtual memory.
        void * pMemoryBase; // uint8
        //! Pointer to the place where memory block ends.
        void * pHeapEnd; // uint8
        
        //! System used memory.
        int64_t iSystemUsedMem;
        int64_t iSystemFreeMemory;  //! Free memory left.
        int64_t iProcessUsedMemory; //! Process used memory.

    };
    
    ///  Memory object for a sizer.
    struct MemoryObject
    {
        
        // constructors
        
        MemoryObject() {
            Clear();
        }
        
        MemoryObject( const void * pId, size_t size = 0, size_t name = 0 ) : m_pId(pId), m_sSize(size), m_sName(name) {
            
        }
      
        // The objects are sorted by their Id.
        bool operator < (const MemoryObject& right) const {
            return (uintptr_t)m_pId < (uintptr_t)right.m_pId;
        }
        
        bool operator < (const void* right) const {
            return (uintptr_t)m_pId < (uintptr_t)right;
        }
        
        bool operator == (const MemoryObject& right) const {
            return m_pId == right.m_pId;
        }

        
        /**
         *  Clear the current object.
         */
        void    Clear()
        {
            m_pId = NEKO_NULL;
            
            m_sSize = 0;
            m_sName = 0;
        }
        
        //! Unique pointer.
        const void  *   m_pId;
        
        size_t      m_sSize;
        size_t      m_sName;
    };
    
    /// Memory sizer check.
    class INekoSizer
    {
        
    };

    struct SMemoryTempFrame;

    ///  MOVE ME: Asset memory pool.
    struct AssetDataPool
    {
        SMemoryTempFrame * tempPool;
        uint8_t * tempData;
    };

#define uptr uintptr_t
    
    /// ============================================================================================
    
    ///  Forward alignment.
    extern inline void*                 GetForwardAlign(void* address, uint8_t alignment);
    extern inline const void*               GetForwardAlign(const void* address, uint8_t alignment);
    
    ///  Backward alignment.
    extern inline void*                 GetBackwardAlign(void* address, uint8_t alignment);
    extern inline const void*               GetBackwardAlign(const void* address, uint8_t alignment);
    
    ///  Alignments with adjustment.
    extern inline uint8_t               GetForwardAlignAdjustment(const void* address, uint8_t alignment);
    extern inline uint8_t               GetForwardAlignAdjustmentHeader(const void* address, uint8_t alignment, uint8_t headerSize);
    extern inline uint8_t               GetBackwardAlignAdjustment(const void* address, uint8_t alignment);
    
    /// ============================================================================================
    extern void*                 AddPtrSize(void* p, size_t x) ;
    extern const void*               AddPtrSize(const void* p, size_t x);
    extern inline void*                 SubtractPtrSize(void* p, size_t x) ;
    extern inline const void*               SubtractPtrSize(const void* p, size_t x);
    
    /// Memory allocator interface.
    class INekoAllocator
    {
    public:
        INekoAllocator( size_t sz, void * start ) :  ptr(start), size(sz),
                                            used_memory(0), num_allocations(0) {
            
        }
        
        virtual ~INekoAllocator()
        {
            // assert/check for zeros
            
            ptr = NEKO_NULL;
            size   = 0;
        }
        
        /**
         *  Allocate memory piece.
         */
        virtual void *              Alloc( size_t size, uint8_t alignment = 4 ) = 0;
        
        /**
         *  Deallocate memory piece.
         */
        virtual void                Dealloc( void * ptr ) = 0;
        
        /**
         *  Get memory pointer.
         */
        void    *           GetPtr() const  {   return ptr;  }
        
        size_t              GetSize() const {   return size;    }
        size_t              GetUsedMem() const  {   return used_memory; }
        size_t              GetNumAllocs()  const   {   return num_allocations; }
        
//    protected:
        
        //!     Memory pointer.
        void    * ptr;
        
        size_t      size;
        
        size_t      used_memory;
        size_t      num_allocations;
    };
    
    extern void InitMem(  GameMemory * game_memory, int64_t size_in_bytes/*, int32_t minblock*/, bool usesMmap = false );
    extern void DeleteMem( GameMemory * game_memory );

    
    /// Linear allocator.
    /// ============================================================================================
    class CLinearAllocator : public INekoAllocator
    {
    public:
        CLinearAllocator( size_t size, void * ptr );
        ~CLinearAllocator();
        
        void    *           Alloc( size_t size, uint8_t alignment = 8 ) override;
        void                Dealloc( void * ptr ) override;
        
        void        Reset();
        
        
        //! Temp memory count.
        int32_t     mTempCount;
        
    private:
        
        CLinearAllocator( const CLinearAllocator & );   // Don't allow copy constructor. ( Use NEKO_NONCOPYABLE ?? )
        CLinearAllocator &              operator = (const CLinearAllocator & );
        
        void        * pCurPos;
    };
    
    
    extern void * PushMemory(SMemoryTempFrame *TempMem, size_t Size);
    //! Create temporary memory pool using the memory pool handler.
    extern SMemoryTempFrame * _PushMemoryFrame( CLinearAllocator * Arena );
    extern void _PopMemoryFrame( SMemoryTempFrame *TempMem ); // Delete it.
    
    /// ============================================================================================
    class CTempAllocator
    {
    public:
        CTempAllocator( INekoAllocator & allocator );
        ~CTempAllocator();
        
        void    *           Push( size_t sz, uint8_t alignment );
        void                Clear();
        
    private:
        INekoAllocator      & pAllocator;
    };
    
    /// Pool allocator.
    /// ============================================================================================
    class CPoolAllocator : public INekoAllocator
    {
    public:
        CPoolAllocator( size_t objsz, uint8_t alignment, size_t sz, void * mem );
        ~CPoolAllocator();
        
        void    *           Alloc( size_t size, uint8_t align ) override;
        void                Dealloc( void * ptr ) override;
        
    private:
        
        CPoolAllocator( const CPoolAllocator & );
        CPoolAllocator &            operator = ( const CPoolAllocator & );
        
        size_t      objectSize;
        uint8_t     objectAlign;
        
        void        **  pFreeList;
    };

    /// Stack allocator.
    /// ============================================================================================
    class CStackAllocator : public INekoAllocator
    {
    public:
        CStackAllocator( size_t size, void * ptr );
        ~CStackAllocator();
        
        void    *           Alloc( size_t sz, uint8_t alignment ) override; // ???
        void                Dealloc( void * ptr ) override;
        
    private:
        
        CStackAllocator( const CStackAllocator & );
        CStackAllocator &               operator = ( const CStackAllocator & );
        
        struct SAllocHeader {
            uint8_t       Adjustment;
        };
        
        void    * pCurPos;
    };
    
    /// Proxy allocator. Used to track allocations.
    /// ============================================================================================
    class CProxyAllocator : public INekoAllocator
    {
    public:
        CProxyAllocator( INekoAllocator * allocator );
        ~CProxyAllocator();
        
        void    *           Alloc( size_t size, uint8_t align ) override;
        void               Dealloc( void * ptr ) override;
        
    private:
        
        CProxyAllocator( const CProxyAllocator & );
        CProxyAllocator &               operator = ( const CProxyAllocator & );
        
        INekoAllocator  * _allocator;
    };
    
    /// Free node allocator.
    /// ============================================================================================
    class CFreeNodeAllocator : public INekoAllocator
    {
    public:
        CFreeNodeAllocator( size_t size, void * ptr );
        ~CFreeNodeAllocator();
        
        void    *           Alloc( size_t size, uint8_t align ) override;
        void                Dealloc( void * ptr ) override;
        
    private:
        
        struct SAllocationHeader
        {
            size_t  Size;
            uint8_t Adjustment;
        };
        
        struct SFreeNode
        {
            size_t      Size;
            SFreeNode   * Next;
        };
        
        CFreeNodeAllocator( const CFreeNodeAllocator & );
        CFreeNodeAllocator  &           operator = ( const CFreeNodeAllocator & );
        
        SFreeNode   * free_blocks;
    };
    
    /// ============================================================================================
    
    namespace NekoAllocator
    {
        // Stack allocator
        
        inline CStackAllocator* newStackAllocator( size_t size, INekoAllocator& allocator )
        {
            void* p = allocator.Alloc( size + sizeof(CStackAllocator), __alignof(CStackAllocator) );
            return new (p) CStackAllocator( size, AddPtrSize(p, sizeof(CStackAllocator)) );
        }
        
        inline void deleteStackAllocator( CStackAllocator* stackAllocator, INekoAllocator* allocator )
        {
            stackAllocator->~CStackAllocator();
            allocator->Dealloc(stackAllocator);
        }
        
        // Pool allocator
        
        inline CPoolAllocator* newPoolAllocator( size_t objectSize, uint8_t objectAlignment, size_t size, INekoAllocator& allocator )
        {
            void* p = allocator.Alloc( size + sizeof(CPoolAllocator), __alignof(CPoolAllocator) );
            return new (p) CPoolAllocator( objectSize, objectAlignment, size, AddPtrSize( p, sizeof(CPoolAllocator) ) );
        }
        
        inline void deletePoolAllocator( CPoolAllocator* poolAllocator, INekoAllocator* allocator )
        {
            poolAllocator->~CPoolAllocator();
            allocator->Dealloc(poolAllocator);
        }
        
        // Linear allocator
        
        inline CLinearAllocator* newLinearAllocator( size_t size, INekoAllocator& allocator )
        {
            void* p = allocator.Alloc( size + sizeof(CLinearAllocator), __alignof(CLinearAllocator) );
            return new (p) CLinearAllocator( size, AddPtrSize(p, sizeof(CLinearAllocator)) );
        }
        
        inline void deleteLinearAllocator( CLinearAllocator& linearAllocator, INekoAllocator& allocator )
        {
            linearAllocator.~CLinearAllocator();
            allocator.Dealloc(&linearAllocator);
        }
        
        // Proxy allocator
        
        inline CProxyAllocator* newProxyAllocator( INekoAllocator* allocator )
        {
            void* p = allocator->Alloc( sizeof(CProxyAllocator), __alignof(CProxyAllocator) );
            return new (p) CProxyAllocator( allocator );
        }
        
        inline void deleteProxyAllocator( CProxyAllocator& proxyAllocator, INekoAllocator* allocator )
        {
            proxyAllocator.~CProxyAllocator();
            allocator->Dealloc( &proxyAllocator );
        }
        
        // Free node allocator
        
        inline CFreeNodeAllocator * newFreeListAllocator(size_t size, INekoAllocator& allocator)
        {
            void* p = allocator.Alloc( size + sizeof(CFreeNodeAllocator), __alignof(CFreeNodeAllocator) );
            return new (p) CFreeNodeAllocator( size, AddPtrSize(p, sizeof(CFreeNodeAllocator)) );
        }
        
        inline void deleteFreeListAllocator( CFreeNodeAllocator & freeListAllocator, INekoAllocator & allocator )
        {
            freeListAllocator.~CFreeNodeAllocator();
            allocator.Dealloc( &freeListAllocator );
        }
    };
    
    namespace NekoAllocator
    {
        template <class T>
        T * AllocateNew(INekoAllocator* allocator) {
            return new (allocator->Alloc(sizeof(T), __alignof(T))) T;
        }
        
        template <class T>
        T * AllocateNew(INekoAllocator* allocator, const T& t) {
            return new (allocator->Alloc(sizeof(T), __alignof(T))) T(t);
        }
        
        template<class T>
        void DeallocateDelete(INekoAllocator* allocator, T& object)
        {
            object.~T();
            allocator->Dealloc(&object);
        }
        
        template<class T> T* AllocateArray(INekoAllocator* allocator, size_t length)
        {
//            ASSERT(length != 0);
            
            uint8_t headerSize = sizeof(size_t) / sizeof(T);
            
            if( sizeof(size_t) % sizeof(T) > 0 ) {
                headerSize += 1;
            }
            
            //Allocate extra space to store array length in the bytes before the array
            T* p = ( (T*) allocator->Alloc(sizeof(T)*(length + headerSize), __alignof(T)) ) + headerSize;
            
            *( ((size_t*)p) - 1 ) = length;
            
            for( size_t i = 0; i < length; ++i ) {
                new (&p[i]) T;
            }
            
            return p;
        }
        
        template<class T> void DeallocateArray(INekoAllocator* allocator, T* array)
        {
            //            ASSERT(array != NEKO_NULL);
            
            size_t length = *( ((size_t*)array) - 1 );
            
            for( size_t i = 0; i < length; ++i ) {
                array[i].~T();
            }
            
            //Calculate how much extra memory was allocated to store the length before the array
            uint8_t headerSize = sizeof(size_t) / sizeof(T);
            
            if( sizeof(size_t) % sizeof(T) > 0 ) {
                headerSize += 1;
            }
            
            allocator->Dealloc(array - headerSize);
        }
    }
    

    
    ///  Temporary memory pool.
    struct SMemoryTempFrame
    {
        CLinearAllocator  *  allocator;
        size_t Used;
        
        uint64_t Offset;
        uint32_t Index;
    };
    
}

#endif /* MemoryPool_cpp */
