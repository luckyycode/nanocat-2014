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
//  GameMemory.cpp
//  Neko engine memory management.
//
//  Created by Kawaii Neko on 8/24/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "GameMemory.h"
#include "Core.h"

#   if defined( NEKO_APPLE_FAMILY )
        #include <sys/mman.h> // mmap, munmap for virtual memory management if required
#   endif

//! VirtualAlloc on Windows; mmap on Linux, OSX
//#define USES_VIRTUAL_MMAP_FOR_ALLOCATIONS

//! FIXME: VirtualAlloc crash
//! TODO: debug asserts

// Neko memory manager! *.*

namespace Neko {
    
    /// ============================================================================================
    void * AddPtrSize( void* p, size_t x ) {
        return (void *)( reinterpret_cast<uptr>(p) + x);
    }
    
    const void * AddPtrSize( const void * p, size_t x ) {
        return (const void *)( reinterpret_cast<uptr>(p) + x);
    }
    
    inline void * SubtractPtrSize( void * p, size_t x ) {
        return (void *)( reinterpret_cast<uptr>(p) - x);
    }
    
    inline const void * SubtractPtrSize( const void * p, size_t x ) {
        return (const void *)( reinterpret_cast<uptr>(p) - x);
    } 
    
    /// ============================================================================================
    CLinearAllocator::CLinearAllocator( size_t size, void * ptr )
        : INekoAllocator(size, ptr), pCurPos(ptr), mTempCount(0) {
        // assert size > 0
    }
    
    CLinearAllocator::~CLinearAllocator() {
        pCurPos = NEKO_NULL;
    }
    
    /**
     *  Allocate memory linearly.
     */
    void * CLinearAllocator::Alloc( size_t sz, uint8_t alignment ) {
        // assert size != 0
        
        uint8_t adjustment = GetForwardAlignAdjustment( pCurPos, alignment );
        
        if( used_memory + adjustment + sz > size ) {
            return NEKO_NULL;
        }
        
        uptr alignedAddr = (uptr)pCurPos + adjustment;
        
        pCurPos = (void *)(alignedAddr + sz);
        
        used_memory += sz + adjustment;
        ++num_allocations;
        
        return (void *)alignedAddr;
    }
    
    void CLinearAllocator::Dealloc(void *ptr) {
        // assert - Use "Reset" instead
        Reset();
    }
    
    /**
     *  Reset allocator.
     */
    void CLinearAllocator::Reset()
    {
        mTempCount = 0;
        
        num_allocations = 0;
        used_memory     = 0;
        
        pCurPos = ptr;
    }
    
    CTempAllocator::CTempAllocator( INekoAllocator & allocator ) : pAllocator( allocator ) {
        
    }
    
    /// ============================================================================================
    CPoolAllocator::CPoolAllocator( size_t objsz, uint8_t align, size_t sz, void * mem )
        : INekoAllocator( sz, mem ), objectSize(objsz), objectAlign(align) {
        // assert objectSize > sizeof void ptr
        
        uint8_t adjustment = GetForwardAlignAdjustment( mem, align );
        pFreeList = (void **)AddPtrSize( mem, adjustment );
        
        size_t numObj = (sz - adjustment) / objsz;
        
        void ** ptr = pFreeList;
        
        // Free blocks list.
        for( size_t i(0); i < numObj - 1; ++i ) {
            *ptr = AddPtrSize( ptr, objsz );
            ptr = (void **) * ptr;
        }
        
        *ptr = NEKO_NULL;
    }
    
    CPoolAllocator::~CPoolAllocator() {
        pFreeList = NEKO_NULL;
    }
    
    void * CPoolAllocator::Alloc(size_t size, uint8_t align)
    {
        // assert size == objsz and if alignment == objalign
        
        if( pFreeList == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        void * ptr = pFreeList;
        pFreeList = (void **)(*pFreeList);
        
        used_memory += size;
        ++num_allocations;
        
        return ptr;
    }
    
    void CPoolAllocator::Dealloc( void * ptr )
    {
        *((void **)ptr) = pFreeList;
        pFreeList = (void **)ptr;
        
        used_memory -= objectSize;
        --num_allocations;
    }
    
    /// ============================================================================================
    CStackAllocator::CStackAllocator( size_t sz, void *ptr )
        : INekoAllocator( sz, ptr ), pCurPos(ptr) {
        // assert sz > 0
    }
    
    CStackAllocator::~CStackAllocator() {
        pCurPos = NEKO_NULL;
    }
    
    void    * CStackAllocator::Alloc(size_t sz, uint8_t alignment)
    {
        // assert size != 0
        
        uint8_t adjustment = GetForwardAlignAdjustmentHeader( pCurPos, alignment, sizeof(SAllocHeader) );
        
        if( used_memory + adjustment + sz > size ) {
            printf( "CStackAllocator::Alloc - out of memory\n" );
            return NEKO_NULL;
        }
        
        void * alignedAddr = AddPtrSize( pCurPos, adjustment );
        
        SAllocHeader * header = (SAllocHeader * )SubtractPtrSize(alignedAddr, sizeof(SAllocHeader) );
        header->Adjustment = adjustment;
        
        pCurPos = AddPtrSize( alignedAddr, sz );
        
        used_memory += sz + adjustment;
        ++num_allocations;
        
        return alignedAddr;
    }
    
    void CStackAllocator::Dealloc( void *ptr )
    {
        // assert p == prevPos
        
        SAllocHeader * header = (SAllocHeader *)SubtractPtrSize( ptr, sizeof(SAllocHeader) );
        used_memory -= (uptr)pCurPos- (uptr)ptr + header->Adjustment;
        
        pCurPos = SubtractPtrSize( ptr, header->Adjustment );
        
        --num_allocations;
    }
    
    /// ============================================================================================
    CProxyAllocator::CProxyAllocator( INekoAllocator * allocator )
        :  INekoAllocator( allocator->GetSize(), allocator->GetPtr() ), _allocator(allocator) {
        
    }
    
    CProxyAllocator::~CProxyAllocator() {
        
    }
    
    void * CProxyAllocator::Alloc(size_t size, uint8_t align)
    {
        // assert != 0
        ++num_allocations;
        
        size_t mem = _allocator->GetUsedMem();
        
        void * ptr = _allocator->Alloc( size, align );
        
        used_memory += _allocator->GetUsedMem() - mem;
        
        return ptr;
    }
    
    void CProxyAllocator::Dealloc(void *ptr)
    {
        --num_allocations;
        
        size_t mem = _allocator->GetUsedMem();
        
        _allocator->Dealloc( ptr );
        
        used_memory -= mem - _allocator->GetUsedMem();
    }
    
    /// ============================================================================================
    CFreeNodeAllocator::CFreeNodeAllocator( size_t size, void * ptr )
        : INekoAllocator( size, ptr ), free_blocks((SFreeNode *)ptr)
    {
        // assert size > size of free block
        free_blocks->Size = size;
        free_blocks->Next = NEKO_NULL;
    }
    
    CFreeNodeAllocator::~CFreeNodeAllocator() {
        free_blocks     = NEKO_NULL;
    }
    
    void * CFreeNodeAllocator::Alloc(size_t size, uint8_t align)
    {
        // assert size != 0 and alignment != 0
        
        SFreeNode * prev = NEKO_NULL;
        SFreeNode * free = free_blocks;
        
        while( free != NEKO_NULL )
        {
            // Calculate adjustment.
            uint8_t adjustment = GetForwardAlignAdjustmentHeader(free, align, sizeof(SAllocationHeader) );
            
            size_t totalSize = size + adjustment;
            
            // If allocation desn't fit, try another one.
            if( free->Size < totalSize ) {
                prev = free;
                free = free->Next;
            }
            
            // assert sizeof allocation header >= size of free block
            
            // If allocations in memory not possible
            if( free->Size - totalSize <= sizeof(SAllocationHeader) ) {
                
                // Increase allocation size.
                totalSize = free->Size;
                
                if( prev != NEKO_NULL ) {
                    prev->Next = free->Next;
                } else {
                    free_blocks = free->Next;
                }
            } else {
                // In another case create a new free node with remaining memory.
                SFreeNode * nextBlock = (SFreeNode *)(AddPtrSize(free, totalSize) );
                nextBlock->Size = free->Size - totalSize;
                nextBlock->Next = free->Next;
                
                if( prev != NEKO_NULL ) {
                    prev->Next = nextBlock;
                } else {
                    free_blocks = nextBlock;
                }
            }
            
            uptr alignedAddr = (uptr)free + adjustment;
            
            SAllocationHeader * header = (SAllocationHeader *)(alignedAddr - sizeof(SAllocationHeader) );
            header->Size = totalSize;
            header->Adjustment = adjustment;
            
            used_memory += totalSize;
            ++num_allocations;
            
            
            
            // assert if forward aligned adjustment == 0
            
            return ( void * ) alignedAddr;
        }
        
        // assert if couldn't find free block large enough
        return NEKO_NULL;
    }
    
    void CFreeNodeAllocator::Dealloc(void *ptr)
    {
        // assert if ptr != NEKO_NULL
        
        SAllocationHeader * header = (SAllocationHeader *)SubtractPtrSize(ptr, sizeof(SAllocationHeader) );
        
        uptr blockStart = reinterpret_cast<uptr>(ptr) - header->Adjustment;
        size_t blockSize = header->Size;
        uptr blockEnd = blockStart + blockSize;
        
        SFreeNode * prev = NEKO_NULL;
        SFreeNode * free = free_blocks;
        
        while( free != NEKO_NULL ) {
            if( (uptr)free >= blockEnd ) {
                break;
            }
            
            prev = free;
            free = free->Next;
        }
        
        if( prev == NEKO_NULL ) {
            prev = (SFreeNode *) blockStart;
            prev->Size = blockSize;
            prev->Next = free_blocks;
            
            free_blocks = prev;
        } else if( (uptr) prev + prev->Size == blockStart ) {
            prev->Size += blockSize;
        } else {
            SFreeNode * temp = (SFreeNode *) blockStart;
            temp->Size = blockSize;
            temp->Next = prev->Next;
            
            prev->Next = temp;
            prev = temp;
        }
        
        if( free != NEKO_NULL && (uptr) free == blockEnd ) {
            prev->Size += free->Size;
            prev->Next = free->Next;
        }
        
        --num_allocations;
        used_memory -= blockSize;
    }

    /**
     *  Main game allocator ( uses the main game memory ).
     */
    void InitMem(  GameMemory * game_memory, int64_t size_in_bytes/*, int32_t minblock*/, bool usesMmap )
    {
        uint8_t * mem = NEKO_NULL;
        
        if( usesMmap ) {
            
            // mmap on Unix.
#   if defined( NEKO_UNIX_FAMILY )
            mem = (uint8_t *)mmap( NEKO_NULL, size_in_bytes, PROT_WRITE | PROT_READ, MAP_ANON | MAP_PRIVATE, 0, 0 ) ;
#   else
            mem = (uint8_t *)VirtualAlloc(NEKO_NULL, size_in_bytes, MEM_RESERVE, PAGE_READWRITE);
#   endif
            
        } else {
            mem = (uint8_t *)malloc( (size_t)size_in_bytes ); // (s)brk?
        }
        
        //! Clear the whole memory heap for now.
        memset( mem, 0x00, size_in_bytes );
        
        game_memory->iMemorySize = size_in_bytes;
        game_memory->pMemoryBase = (uint8_t *)mem;
        
        //! Set the memory end pointer.
        game_memory->pHeapEnd = (uint8_t *)game_memory->pMemoryBase + size_in_bytes;

        g_Core->p_Console->Print( LOG_INFO, "InitMem(): A new memory heap with size of %llu mb was created.\n", ByteInMegabyte(size_in_bytes) );
    }
    
    
    /// ============================================================================================
    
    /**
     *  Allocate a new item of given size in temporary memory arena.
     */
    void * PushMemory( SMemoryTempFrame * TempMem, size_t Size )
    {
        // Memory buffer overflow check.
        if( (TempMem->Used + TempMem->allocator->GetUsedMem() + Size) > TempMem->allocator->GetSize() ) {
            g_Core->p_Console->Print( LOG_WARN, "PushMemory(): A new item with size of %d bytes exceedes temporary memory pool size with size of %d bytes.\n", Size, TempMem->allocator->GetSize() );
            
            return NEKO_NULL; // Umm..
        }
        
        if( (int64_t)TempMem->Index != (TempMem->allocator->mTempCount - 1) ) {
            g_Core->p_Console->Print( LOG_WARN, "PushMemory(): Overlaping temporary memory detected.\n" );
            
            return NEKO_NULL;
        }
        
        void * Result = (void *)((uint8_t *)TempMem->allocator->GetPtr() +
                                 (TempMem->allocator->GetSize() - TempMem->Offset - TempMem->Used - Size));
        TempMem->Used += Size;
        
        return Result;
    }
    
    /**
     *  Create temporary memory block.
     */
    SMemoryTempFrame * _PushMemoryFrame( CLinearAllocator * Arena )
    {
        int32_t i;
        
        uint64_t CurMemOffset;
        SMemoryTempFrame * Result;
        
        CurMemOffset = sizeof( SMemoryTempFrame );
        Result = (SMemoryTempFrame *)((uint8_t *)Arena->GetPtr() + Arena->GetSize() - CurMemOffset);
        
        // Loop through all temporary memory arenas.
        for( i = 0; i < Arena->mTempCount; ++i ) {
            CurMemOffset += Result->Used + sizeof( SMemoryTempFrame );
            Result = (SMemoryTempFrame *)((uint8_t *)Arena->GetPtr() + (Arena->GetSize() - CurMemOffset));
            
            if( (CurMemOffset + Arena->GetUsedMem()) > Arena->GetSize() ) {
                g_Core->p_Console->Print( LOG_WARN, "PushMemoryFrame(): No enough memory to allocate temporary memory frame!\n" );
            }
        }
        
        Result->allocator = Arena;
        Result->Used = 0;
        Result->Offset = CurMemOffset;
        Result->Index = Arena->mTempCount;
        
        // Increase temporary memory arenas count.
        ++Arena->mTempCount;
        
        return Result;
    }
    
    /**
     *  Remove memory temporary frame.
     */
    void _PopMemoryFrame( SMemoryTempFrame * TempMem )
    {
        if( TempMem == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_WARN, "PopMemoryFrame(): Tried to remove non-existant temporary memory arena.\n" );
            return;
        }
        
        if( TempMem->allocator->mTempCount < 0 ) {
            g_Core->p_Console->Print( LOG_WARN, "PopMemoryFrame(): Tried to remove temporary memory arena while it wasn't created.\n " );
            return;
        }
        
        if( TempMem->Index != (TempMem->allocator->mTempCount - 1) ) {
            g_Core->p_Console->Print( LOG_WARN, "PopMemoryFrame(): Tried to close wrong temporary memory area.\n " );
            return;
        }
        
        --TempMem->allocator->mTempCount;
    }

    /// ============================================================================================
    
    /**
     *  Delete (free) memory arena.
     */
    void DeleteMem( GameMemory * game_memory )
    {
#   if defined( USES_VIRTUAL_MMAP_FOR_ALLOCATIONS )
    #   if defined( NEKO_WINDOWS_FAMILY ) // VirtualFree
        
    #   else
        munmap( game_memory->pMemoryBase, game_memory->iMemorySize * sizeof(uint8_t) );
    #   endif
#   else
        free( game_memory->pMemoryBase );
#   endif
        
        memset( game_memory, 0x00, sizeof( GameMemory ) );
        game_memory = NEKO_NULL;
    }
    
    /// ============================================================================================
    
    inline uint8_t GetBackwardAlignAdjustment( const void * address, uint8_t alignment )
    {
        uint8_t adjustment =  reinterpret_cast<uptr>(address) & static_cast<uptr>(alignment - 1);
        
        if( adjustment == alignment ) {
            return 0; // Already aligned
        }
        
        return adjustment;
    }
    
    inline uint8_t GetForwardAlignAdjustmentHeader( const void * address, uint8_t alignment, uint8_t headerSize )
    {
        uint8_t adjustment =  GetForwardAlignAdjustment(address, alignment);
        
        uint8_t neededSpace = headerSize;
        
        if( adjustment < neededSpace ) {
            neededSpace -= adjustment;
            
            // increase so we can fit header struct
            adjustment += alignment * (neededSpace / alignment);
            
            if( neededSpace % alignment > 0 ) {
                adjustment += alignment;
            }
        }
        
        return adjustment;
    }
    
    inline uint8_t GetForwardAlignAdjustment( const void * address, uint8_t alignment )
    {
        uint8_t adjustment =  alignment - ( reinterpret_cast<uptr>(address) & static_cast<uptr>(alignment - 1) );
        
        if( adjustment == alignment ) {
            return 0; // Already aligned
        }
        
        return adjustment;
    }
    
    inline const void* GetBackwardAlign(const void* address, uint8_t alignment) {
        return (void*)( reinterpret_cast<uptr>(address) & static_cast<uptr>(~(alignment-1)) );
    }
    
    inline void* GetBackwardAlign(void* address, uint8_t alignment) {
        return (void*)( reinterpret_cast<uptr>(address) & static_cast<uptr>(~(alignment-1)) );
    }
    
    inline const void* GetForwardAlign(const void* address, uint8_t alignment) {
        return (void*)( ( reinterpret_cast<uptr>(address) + static_cast<uptr>(alignment-1) ) & static_cast<uptr>(~(alignment-1)) );
    }
    
    inline void* GetForwardAlign(void* address, uint8_t alignment) {
        return (void*)( ( reinterpret_cast<uptr>(address) + static_cast<uptr>(alignment-1) ) & static_cast<uptr>(~(alignment-1)) );
    }
    /// ============================================================================================
}