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
//  LChunk.cpp
//  Level of detail quadtree node chunk.
//
//  This code is a part of Neko engine.
//  Created by Neko Vision on 12/22/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../Core/Core.h"
#include "../Graphics/OpenGL/OpenGLBase.h"  // IBO_OFFSET
#include "../Platform/Shared/SystemShared.h"
#include "../World/BeautifulEnvironment.h"
#include "../World/Foliage.h"
#include "LChunk.h"


//#include "MetalExternal.h"

namespace Neko {

    /**
     *  Initialize new instance for chunk.
     */
    CNodeChunk::CNodeChunk()
    {
        uint32_t i;
        
        for( i = 0; i < TERRAIN_CHUNKS_LOD; ++i ) {
            m_tIndice[i].indices = NEKO_NULL;
        }
        
        bGotAmbient = false;
        fAmbientRadius = 0.0f;
        
        m_bIsReady = false;
        
        m_tCells = NEKO_NULL;
        m_foliageAmount = 0;
    }
    
    /**
     *  Remove chunk.
     */
    CNodeChunk::~CNodeChunk()
    {
        Delete();
    }
    
    /**
     *  Make level of detail chunks.
     */
    void CNodeChunk::Load( const uint32_t level,
                        const Vec2i & pos,
                        const Vec2i & size, const int16_t qtype, const bool hasFoliageData, int16_t flags, INekoAllocator * allocator, SMemoryTempFrame * temp )
    {
        uint32_t i;
        
        pAllocator = allocator;
        pTempMemory = temp;
        
        m_qtType = qtype;
        m_tBiome = (LandscapeChunkType)(rand() % BIOME_COUNT);

        m_iFlags = flags;
        
//        // Allocate foliage array.
//        if( hasFoliageData == true ) {
//            m_tCells = new ncCGrass*[MAX_CHUNK_FOLIAGES];// (ncCGrass**)AllocMemory( &worldPoolMemory, sizeof(ncCGrass*) * MAX_CHUNK_FOLIAGES );
//            
//            for( i = 0; i < MAX_CHUNK_FOLIAGES; ++i ) {
//                m_tCells[i] = new ncCGrass;
//            }
//            
//            m_foliageAmount = 0;
//            
//            // Create foliage object list.
//            SList::CreateList( &m_foliageObjects );
//        }

        // Uh oh. :D
        if( m_qtType == TERRAIN ) {
            // Reset counter.
            m_TerrainObjectCount = 0;
            
            // Create terrain objects.
            SList::CreateList( &m_terrainObjects );
        }
        
        // Create node chunks.
        for( i = 0; i < TERRAIN_CHUNKS_LOD; ++i ) {
            MakeLevelOfDetailIndices( i, level, pos, size );
        }
        
        // Can be used now.
        m_bIsReady = true;
    }
    
    /**
     *  Create threaded data.
     */
    static void CreateIndexData( void * arg )
    {
        if( arg == NEKO_NULL ) {
            return;
        }
        CNodeChunk * chunk = (CNodeChunk *) arg;
        
        const int32_t lod = chunk->m_currentLod;
        
        chunk->m_iElementBuffer[lod] = g_mainRenderer->AllocGPUBuffer( chunk->m_tIndice[lod].count * sizeof(GLuint),
                                                                      EBufferStorageType::IndexArray, EBufferType::Static, EPrimitiveType::TriangleStrip );
        g_mainRenderer->BufferData( &chunk->m_iElementBuffer[lod], &chunk->m_tIndice[lod].indices[0], 0, chunk->m_tIndice[lod].count * sizeof(GLuint) );
        g_mainRenderer->FinishBuffer( &chunk->m_iElementBuffer[lod], 0, chunk->m_tIndice[lod].count );
    }
    
    /**
     *  Calculate index array for per LOD.
     */
    void CNodeChunk::MakeLevelOfDetailIndices( const uint32_t lod, const uint32_t level,
                                            const Vec2i & pos, const Vec2i & size )
    {
        Vec2i posData = pos;
        
        uint32_t offset = (uint32_t)nkMath::FastPow( 2, lod );
        uint32_t leveloffset = (uint32_t)nkMath::FastPow( 2, level + lod );
        
        Vec2i posSize = size / leveloffset + Vec2i( 1 );
        
        m_tIndOffsetW[lod] = posSize.x * 2;
        m_tIndOffsetH[lod] = posSize.x - 1;
        
        // Get indice count and create chunks with lowered indice ammount ( more far - less indices ).
        const uint32_t nIndice = posSize.x * (posSize.y - 1) * 2;
        
        m_currentLod = lod;
        
        // Allocate index data.
        m_tIndice[lod].indices = new uint32_t[ nIndice ];// pAllocator->Alloc( sizeof(uint32_t) * nIndice );
        m_tIndice[lod].count = nIndice;

        // Sanity check.
        if( m_tIndice[lod].indices == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "Could not allocate %i indices for LOD%i\n", nIndice, lod );
            
            return;
        }
        
        uint32_t id = 0;
        
        uint32_t j;
        uint32_t i;
        
        uint32_t id0;
        uint32_t id1;
        
        // Fill indicies data for each lod.
        for( j = 0; j < (uint32_t)posSize.y - 1; ++j ) {
            for( i = 0; i < (uint32_t)posSize.x; ++i ) {
                // TODO: count & sizes
                id0 = ((j * (offset) + posData.y) * (size.x) + (i * (offset) + posData.x));
                id1 = ((j * (offset) + posData.y + offset) * (size.x) + (i * (offset) + posData.x));
                
                m_tIndice[lod].indices[id++] = id0;
                m_tIndice[lod].indices[id++] = id1;
            }
        }
        
        // Create GPU buffers with indice data.
        if( m_iFlags & WORLD_THREADED ) {
            g_mainRenderer->Perform( CreateIndexData, (void*)this );
        } else {
            m_iElementBuffer[lod] = g_mainRenderer->AllocGPUBuffer( m_tIndice[lod].count * sizeof(GLuint), EBufferStorageType::IndexArray, EBufferType::Static );
            g_mainRenderer->BufferData( &m_iElementBuffer[lod], &m_tIndice[lod].indices[0], 0, m_tIndice[lod].count * sizeof(GLuint) );
            g_mainRenderer->FinishBuffer( &m_iElementBuffer[lod], 0, m_tIndice[lod].count );
        }
        
        // Index data is going to be deleted after collision/bounding box calculation.
    }
    
    /**
     *  Destroy chunks.
     */
    void CNodeChunk::Delete()
    {
        uint32_t i;
        
        // Delete index data.
        for( i = 0; i < TERRAIN_CHUNKS_LOD; ++i ) {
            // Delete buffers on GPU.
            g_mainRenderer->DeleteGPUBuffer( &m_iElementBuffer[i] );
            
            // Delete indice data.
            if( m_tIndice[i].indices != NEKO_NULL ) {
//                pAllocator->Dealloc(m_tIndice[i].indices);// m_tIndice[i].indices;
                delete [] m_tIndice[i].indices;
                m_tIndice[i].indices = NEKO_NULL;
            }
        }
        
        SLink   * head;
        SLink   * cur;
        SLink   * next;
        
        ncLTerrainObject * object;
        ncCGrass * foliage;
        
        head = &m_terrainObjects.m_sList;
        cur = head->m_pNext;
        
        // Delete objects.
        while( cur != head ) {
            next = cur->m_pNext;
            
            object = (ncLTerrainObject*)cur->m_ptrData;
            
            // Delete object.
//            pAllocator->Dealloc( object );
            
            cur = next;
        }
        
        // Link the list off.
        SList::CreateList( &m_terrainObjects );
        
//        head = &m_foliageObjects.m_sList;
//        cur = head->m_pNext;
//        
//        // Delete objects.
//        while( cur != head ) {
//            next = cur->m_pNext;
//            
//            foliage = (ncCGrass *)cur->m_ptrData;
//            
//            // Delete object.
//            pAllocator->Dealloc( foliage );
//            
//            cur = next;
//        }
//        
//        // Link the list off.
//        SList::CreateList( &m_foliageObjects );
//        
//        pAllocator->Dealloc(m_tCells);
//        m_tCells = NEKO_NULL;
    }
    
    /**
     *  Add foliage layer ( grass, brushes, flowers etc )
     */
    void CNodeChunk::AddFoliageLayer( float x, float y, float z )
    {
        // Quadtree type.
        if( m_qtType == OCEAN ) {
            // Uhhhh........
            g_Core->p_Console->Print( LOG_ERROR, "Couldn't add object to Ocean mesh (yet).\n" );
            
            return;
        }
        
        // Safity checks.
        if( m_foliageAmount > (MAX_CHUNK_FOLIAGES - 2) ) {
            return;
        }
        
        // Set cell position and rotation.
        ncCGrass * cell = m_tCells[m_foliageAmount];
        cell->Set( x, y, z );
        
        SList::AddHead( &m_foliageObjects, &cell->m_Link, cell );
        
        ++m_foliageAmount;
    }
    
    /**
     *  Add new object.
     */
    void CNodeChunk::AddObject( ncLTerrainObject * object )
    {
        if( object == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "Couldn't add object to chunk, object is null.\n" );
            return;
        }
        
        if( m_qtType == OCEAN ) {
            g_Core->p_Console->Print( LOG_ERROR, "Couldn't add object to Ocean chunk.\n" );
            return;
        }
        
        SList::AddHead( &m_terrainObjects, &object->m_Link, object );
        ++m_TerrainObjectCount;
    }
    
    /**
     *  Draw foliage.
     */
    int32_t CNodeChunk::DrawFoliage( uint32_t lod, float distance )
    {
//        // Enable Sample alpha to coverage (antialiased transparency! :P).
//        // USE INSTANCED RENDERING!
//        if( lod > TERRAIN_CHUNK_LOD1 ) {
//            return 0;
//        }
//        
//        SLink * head;
//        SLink * cur;
//        
//        ncCGrass * foliage;
//        
//        head = &m_foliageObjects.m_sList;
//        
//        // Render all the cells.
//        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
//            foliage = (ncCGrass *) cur->m_ptrData;
//            
//            if( !g_Core->p_Camera->m_CommonFrustum.ContainsPoint( foliage->vPos) ) {
//                // ContainsSphere( mCell[i].mPos, 0.0f ) )
//                continue;
//            }
//            
//            g_pbEnv->PassGrassUniforms( foliage->vPos.x,
//                                     foliage->vPos.y,
//                                     foliage->vPos.z, foliage->fRotation );
//            foliage->Render();
//        }
//        
        return 0;
    }
    
    /**
     *   Draw objects.
     */
    int32_t CNodeChunk::DrawObjects( int32_t lod, float distance, bool isShadow )
    {
//        unsigned int i;

        SLink   * head;
        SLink   * cur;
        
        ncLTerrainObject    * object;
        
//        if( lod > TERRAIN_CHUNK_LOD1 )
//            return 0;
        
        head = &m_terrainObjects.m_sList;
        
        // Real objects.
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            object = (ncLTerrainObject *)cur->m_ptrData;
            
            if( object != NEKO_NULL ) {
                object->Draw( lod, distance, isShadow );
            }
        }
        
        return m_TerrainObjectCount;
    }

    /**
     *  Draw object.
     */
    int CNodeChunk::DrawGround( uint32_t lod )
    {
#   if defined( USES_OPENGL )

        // Because this is optimal Level of Detail.
        if( lod > 0 ) {
            --lod;
        }
        
        // Bind current LoD buffer.
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_iElementBuffer[lod].Handle );
        
        // Render all index offset data by height offsets.
        for( uint32_t j = 0; j < m_tIndOffsetH[lod]; ++j ) {
            glDrawElements( GL_TRIANGLE_STRIP, (int32_t)m_tIndOffsetW[lod], GL_UNSIGNED_INT,
                           IBO_OFFSET( (j * m_tIndOffsetW[lod]) ) );
        }
        
        // Index data been already set, so we just use IBO_OFFSET to go to needed index.
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
#   endif
        // Ground chunk drawn.
        return 1;
    }
    
}