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
//  CWorldHandler.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 10/5/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "../../AssetCommon/AssetBase.h"
#include "../../Core/Utilities/Utils.h"
#include "../../Math/BoundingBox.h"
#include "../BeautifulEnvironment.h"
#include "ChunkHandler.h"


//#include "../../../Libraries/SimplexNoise.h"

namespace Neko {
    
    /**
     *  A thread to build chunks.
     *
     *  @param thread Thread owner.
     *  @param arg1   First argument ( chunk handler ).
     */
    static void ChunkBuildThread( INekoThread * thread, void * arg1, void * arg2 )
    {
        // Get the chunk handler.
        CWorldHandler * chunkHandler = (CWorldHandler*)arg1;
        
        bool stopBuild;
        
        while( true ) {
            // Wait for the signal command.
            chunkHandler->m_chunkBuildEvent->Wait( 1000.0 /* one second */ );
            
            // Build all chunks we have.
            while( true ) {
                chunkHandler->m_chunkBuildLock->Lock();
                
                stopBuild = chunkHandler->RebuildChunks();
                
                chunkHandler->m_chunkBuildLock->Unlock();
                
                // Check if we still have something...
                if( stopBuild == true ) {
                    break;  // Nothing to do.
                }
            }
        }
    }
    
    /**
     *  Sort landscape chunks.
     */
    static int32_t SortChunks( const void * first, const void * second )
    {
        const LandscapeChunk * firstChunk = *(LandscapeChunk**) first;
        const LandscapeChunk * secondChunk = *(LandscapeChunk**) second;
        
        int32_t distA = firstChunk->m_fPositionX * firstChunk->m_fPositionX + firstChunk->m_fPositionY * firstChunk->m_fPositionY;
        int32_t distB = secondChunk->m_fPositionX * secondChunk->m_fPositionX + secondChunk->m_fPositionY * secondChunk->m_fPositionY;
        
        if( distA < distB ) {
            return -1;
        }
        
        if( distA > distB ) {
            return 1;
        }
        
        if( firstChunk->m_fPositionX < firstChunk->m_fPositionX ) {
            return -1;
        }
        
        if( firstChunk->m_fPositionX > secondChunk->m_fPositionX ) {
            return 1;
        }
        
        return firstChunk->m_fPositionY - secondChunk->m_fPositionY;
    }
    
    static const uint32_t kLandscapeCacheSize = Megabyte( 32 );
    
    /**
     *  Initialize world chunk handler.
     */
    void CWorldHandler::Initialize( INekoAllocator * allocator )
    {
        // Default chunk size.
        m_iChunkSize = 512;
        // Maximum LOD amount for culling.
        m_iMaxLOD = 3;
        
        m_bUpdateGPUCulling = false;
        
        pAllocatorHandle = allocator;
        pAllocator = allocator;
        
        // Create resource pool for landscape..
        pChunkPoolAllocator = NekoAllocator::newPoolAllocator( sizeof(LandscapeChunk), __alignof(LandscapeChunk), kLandscapeCacheSize, *pAllocatorHandle );
//        // Create stack allocator
//        pStackAllocator = NekoAllocator::newStackAllocator( kLandscapeCacheSize, *pMainAllocProxy );
        
        // Chunks height.
        m_fMaxHeight = nkMath::RandFloatAlt( 2.0f, 128.0f );

        // Landscape shader.
        m_pLandscapeShader = f_AssetBase->FindAssetByName<SGLShader>( "landscape" );
        m_pLandscapeShader->Use();
 
        m_pShaderParams[ModelViewProj] = m_pLandscapeShader->UniformLocation( "MVP" );
        m_pShaderParams[ModelMatrix] = m_pLandscapeShader->UniformLocation( "ModelMatrix" );
        m_pShaderParams[PreviousMatrix] = m_pLandscapeShader->UniformLocation( "PrevMVP" );

        // Four texture samplers.
        m_pLandscapeShader->SetUniform( "diffuseSamplers", 0 );
        // Normal samplers.
        m_pLandscapeShader->SetUniform( "normalSamplers", 1 );
        // PBR map samplers.
        m_pLandscapeShader->SetUniform( "pbrmapSamplers", 2 );
        
        // TODO: snow/rain and other weather effects?
        m_pLandscapeShader->Next();
        
        // Geometry culling shader.
        m_pCullShader = f_AssetBase->FindAssetByName<SGLShader>( "cull" );
        
        m_pCullShader->Use();
        m_pCullShader->SetUniform( "HiZBuffer", 0 );
        m_pCullShader->SetUniform( "LodDistance", 50.f, 1000.f);
        // Uniform locations.
        m_iCullShaderUniforms[ECullShaderUniforms::Offset] = m_pCullShader->UniformLocation( "Offset" );
        
        // Setup subroutine indices.
        m_iCullShaderSubIndexVS[PASS_THROUGH] = m_pCullShader->GetSubroutineIndex( "PassThrough", EShaderType::Vertex );
        m_iCullShaderSubIndexVS[INSTANCE_CLOUD_REDUCTION] = m_pCullShader->GetSubroutineIndex( "InstanceCloudReduction", EShaderType::Vertex);
        m_iCullShaderSubIndexVS[HI_Z_OCCLUSION_CULL] = m_pCullShader->GetSubroutineIndex( "HiZOcclusionCull", EShaderType::Vertex );
        
        m_iCullShaderSubIndexGS[0] = m_pCullShader->GetSubroutineIndex( "PassThrough", EShaderType::Geometry);
        m_iCullShaderSubIndexGS[1] = m_pCullShader->GetSubroutineIndex( "DynamicLOD", EShaderType::Geometry);
        
        m_pCullShader->Next();
        
        // TODO: AllocGPUBuffer?
        
        // Uniform buffer.
        glGenBuffers( 1, &m_iMeshUniformBuffer );
        glBindBuffer( GL_UNIFORM_BUFFER, m_iMeshUniformBuffer );
        
        glBufferData( GL_UNIFORM_BUFFER, sizeof(transform), &transform, GL_STREAM_DRAW );
        glBindBufferBase( GL_UNIFORM_BUFFER, 0, m_iMeshUniformBuffer );
        glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(transform), &transform );
        
        glBindBuffer( GL_UNIFORM_BUFFER, 0 );
        
        // Default landscape matrices.
        m_projectionModelView.Identity();
        m_modelView.Identity();
        
        m_prevModelView = m_modelView;
        
        // Create landscape chunk build thread events.
        m_chunkBuildLock = CCore::CreateLock();
        m_chunkBuildEvent = CCore::CreateEvent();
        m_chunkBuildThread = CCore::CreateThread( INekoThread::PRIORITY_NORMAL, 1, ChunkBuildThread, this, NEKO_NULL );

        // Create chunk build list.
        m_chunkBuildList.Create( 4, pLinearAllocator2 );
        
        fNoise_multiplier = 3.0f;          // TODO: customizable for each landscape
        fNoise_largeNoiseCoef = 1500.1f;
        fNoise_typeNoiseCoef = 250.1f;
        fNoise_medNoiseCoef = 750.1f;
        
        // Noise.
        srand( (uint32_t)time(NEKO_NULL) );
        
        // Don't delete that.
        m_noiseLarge = (ncNoisePerlin *)pAllocator->Alloc( sizeof(ncNoisePerlin) );
        m_noiseMed = (ncNoisePerlin *)pAllocator->Alloc(  sizeof(ncNoisePerlin) );
        m_noiseType = (ncNoisePerlin *)pAllocator->Alloc(  sizeof(ncNoisePerlin) );
        m_noisePath = (ncNoisePerlin *)pAllocator->Alloc( sizeof(ncNoisePerlin) );
        m_noiseFoliage = (ncNoisePerlin *)pAllocator->Alloc( sizeof(ncNoisePerlin) );
        //noiseObjectsMap = new ncNoisePerlin( 5, 1, 1, rand() );
        
        // Noise parameters will be set in world creationism method.
        m_noisePath->Set( 1, -4.0f, 128.0f, rand() );
        m_noiseFoliage->Set( 5, 1.0f, 1.0f, rand() );
    }
    
    /**
     *  Create a new path base.
     *
     *  @note   Called only once for each node path.
     */
    void CWorldHandler::CreatePathNodeBase( const Vec2 & origin )
    {
        SPathInfo * path = (SPathInfo *)pAllocator->Alloc( sizeof(SPathInfo) );
        
        path->m_fPathWidth = 12.0f;
        path->m_pMaterial = f_AssetBase->p_MaterialBase->Find( "road01" );  //@todo customize
        
        float X = floorf( origin.x );
        float Y = floorf( origin.y );
        
        LandscapeChunk * lc = FindNodeAt( Vec3( X, 0.0f, Y ) );
        // Now we have access to its vertex data, starting from zero to 'm_iSize' property.
        // Now, make camera origin relative to chunk vertex data.
        // Use chunk's position data for correct results.
        
        if( lc == NEKO_NULL ) {
            return;
        }
        
        lc->CreateNodeCache( path, MAX_PATH_NODE_PIECES );
    }
    
    /**
     *  Create a new path.
     */
    void CWorldHandler::CreatePathNodeAt( const Vec2 & origin )
    {
        float X = floorf( origin.x );
        float Y = floorf( origin.y );
        
        LandscapeChunk * lc = FindNodeAt( Vec3( X, 0.0f, Y ) );
        
        // Probably a chunk without road cache data.
        if( lc->GetPathNode() == NEKO_NULL ) {
            CreatePathNodeBase( Vec2( X, Y ) );
        }
        
        float rayHit = lc->GetHeightAt( X, Y );
        
        SChunkPathCell p0;
        
        p0.Position = Vec2( X, Y );
        p0.HeightAt = rayHit;
        
        lc->CreatePathNode( &p0 );
    }
    
    /**
     *  Set random noise parameters such as frequency, power, etc..
     *
     *  @param largeNoise  Large noise pattern.
     *  @param mediumNoise Medium noise pattern.
     *  @param smallNoise  Type noise.
     */
    void CWorldHandler::SetNoiseParameters(const Neko::NoisePerlinParams &largeNoise, const Neko::NoisePerlinParams &mediumNoise, const Neko::NoisePerlinParams &smallNoise)
    {
        m_noiseLarge->Set( largeNoise.octave, largeNoise.freq, largeNoise.amp, (int32_t)largeNoise.seed );
        m_noiseMed->Set( mediumNoise.octave, mediumNoise.freq, mediumNoise.amp, (int32_t)mediumNoise.seed );
        m_noiseType->Set( smallNoise.octave, smallNoise.freq, smallNoise.amp, (int32_t)smallNoise.seed );
    }
    
    /**
     *  Create a first chunk.
     */
    void CWorldHandler::Create()
    {
        SMemoryTempFrame * tempWorldMem = NEKO_NULL;
        
        int32_t x;
        int32_t z;
        
        // Temporary memory pool for chunk data allocation.
        tempWorldMem = Neko::_PushMemoryFrame( pLinearAllocator2 );
        
        int32_t originX;
        int32_t originZ;
        
        // Create a first chunk with six (3x3) pieces.
        for( x = 0; x <= 2; ++x ) {
            for( z = 0; z <= 2; ++z ) {
                originX = (x - 1) * m_iChunkSize - m_iChunkSize / 2;
                originZ = (z - 1) * m_iChunkSize - m_iChunkSize / 2;
                
                // Bounding box.
                SBoundingBox bbox( Vec3( -512.0f, -12.0f, -512.0f ), Vec3( 512.0f, 412.0f, 512.0f ) );
                
                // Set bounding box center position.
                bbox.Translate( Vec3( originX + originX, 0.0f, originZ + originZ ) );
                
                LandscapeChunk * chunk = (LandscapeChunk *)pChunkPoolAllocator->Alloc( sizeof(LandscapeChunk) );
                chunk->Load( m_pLandscapeShader, bbox, 128, 512, originX + originX, originZ + originZ );
                
                chunk->m_fNoiseStartX = (x - 1) * m_iChunkSize;
                chunk->m_fNoiseStartZ = (z - 1) * m_iChunkSize;
                
                chunk->Create( pStackAllocator, tempWorldMem, false );
                
                m_landscapeChunks[x][z] = chunk;
            }
        }
        
        //        CreateChunk( 512, 512, 0, 0, Vec3(0.0f), tempWorldMem );
        //        CreateChunk( 512, 512, 0, 512, Vec3(0, 0.0f, 512 + 512), tempWorldMem );
        
        _PopMemoryFrame( tempWorldMem );
        
        m_iChunkSize = 512;
        
        m_bIsActive = true;
    }
    
    /**
     *  Destroy the landscapes.
     */
    void CWorldHandler::Destroy()
    {
        int32_t     x;
        int32_t     z;
        
        LandscapeChunk * chunk = NEKO_NULL;
        
        for( x = 0; x <= 2; ++x ) {
            for( z = 0; z <= 2; ++z ) {
                chunk = m_landscapeChunks[x][z];
                
                chunk->Destroy();
                
                pChunkPoolAllocator->Dealloc( chunk );
                chunk = NEKO_NULL;
            }
        }
        
        NekoAllocator::deleteStackAllocator( (CStackAllocator*)pChunkPoolAllocator, pAllocatorHandle );
    }

    /**
     *  Find chunk node at origin.
     *
     *  @param pos Origin.
     */
    LandscapeChunk * CWorldHandler::FindNodeAt( const Vec3 & pos )
    {
        int32_t     x;
        int32_t     z;
        
        LandscapeChunk * chunk = NEKO_NULL;
        
        for( x = 0; x <= 2; ++x ) {
            for( z = 0; z <= 2; ++z ) {
                chunk = m_landscapeChunks[x][z];
                
                // Check if chunk is created.
                if( chunk->GetQuadtree() != NEKO_NULL ) {
                    const SBoundingBox * bbox = chunk->GetBoundingBox();
                    if( bbox->ContainsPoint( pos ) ) {
                        return chunk;
                    }
                }
            }
        }
        
        return chunk;
    }
    
    /**
     *  Rebuild chunks.
     */
    bool CWorldHandler::RebuildChunks()
    {
        bool rebuild = false;

        // Check if we have something to build.
        if( m_chunkBuildList.GetCount() > 0 ) {
            SMemoryTempFrame * worldMemory;
            LandscapeChunk * chunk;
            
            // Create temporary memory area for chunk allocations.
            worldMemory = _PushMemoryFrame( pLinearAllocator2 );
            
            chunk = (LandscapeChunk*)m_chunkBuildList.Pop();
            chunk->Create( pStackAllocator, worldMemory );
            
            // If we still have something to build..
            rebuild = (m_chunkBuildList.GetCount() > 0);
            
            // Close memory area.
            _PopMemoryFrame( worldMemory );
        }
        
        
        return rebuild;
    }
    
    /**
     *  Invoked on creation.
     */
    void CWorldHandler::OnChunkCreated( LandscapeChunk *chunk )
    {

    }
    
    /**
     *  Invoked on removal.
     */
    void CWorldHandler::OnChunkDestroyed( LandscapeChunk *chunk )
    {
        
    }
    
    /**
     *  Update landscape chunks and see if we need to create/delete them.
     *
     *  @TODO       Neko allocator
     *  @note       Simply replaces chunks by the new ones.
     */
    void CWorldHandler::UpdateChunks()
    {
        const Vec3 * cameraPosition = &g_Core->p_Camera->lastPosition;
        
        // x-aligned chunks.
        if( cameraPosition->x > m_landscapeChunks[2][0]->m_fPositionX ) {
            
            for( int32_t z = 0; z <= 2; ++z ) {
                OnChunkDestroyed( m_landscapeChunks[0][z] );
                // Clear the chunk and delete it from the memory.
                m_landscapeChunks[0][z]->Destroy();
                pChunkPoolAllocator->Dealloc(m_landscapeChunks[0][z]);
                m_landscapeChunks[0][z] = NEKO_NULL;
                
                // Replace previous chunks.
                m_landscapeChunks[0][z] = m_landscapeChunks[1][z];
                m_landscapeChunks[1][z] = m_landscapeChunks[2][z];
                
                // Nice for a noise start origin.
                int32_t originX = m_landscapeChunks[1][z]->m_fPositionX + 512;
                int32_t originZ = m_landscapeChunks[1][z]->m_fPositionY;
                // Bounding box.
                SBoundingBox bbox( Vec3(-512, -12.0f, -512), Vec3( 512, 412.0f, 512 ) );
                bbox.Translate( Vec3( originX + 512, 0.0f, originZ ) );
                
                // Create a new chunk.
                LandscapeChunk * chunk = (LandscapeChunk *)pChunkPoolAllocator->Alloc( sizeof(LandscapeChunk) );
                chunk->Clear();
                chunk->Load( m_pLandscapeShader, bbox, 128, 512, originX + 512, originZ );
                chunk->m_fNoiseStartX = m_landscapeChunks[1][z]->m_fNoiseStartX + 512;
                chunk->m_fNoiseStartZ = m_landscapeChunks[1][z]->m_fNoiseStartZ;
                
                
                m_landscapeChunks[2][z] = chunk;
                OnChunkCreated( chunk );
            }
        } else if( cameraPosition->x < m_landscapeChunks[1][0]->m_fPositionX ) {
            
            for( int32_t z = 0; z <= 2; ++z ) {
                OnChunkDestroyed( m_landscapeChunks[2][z] );
                m_landscapeChunks[2][z]->Destroy();
                pChunkPoolAllocator->Dealloc( m_landscapeChunks[2][z] ) ;
                m_landscapeChunks[2][z] = NEKO_NULL;
                
                // Replace previous chunks.
                m_landscapeChunks[2][z] = m_landscapeChunks[1][z];
                m_landscapeChunks[1][z] = m_landscapeChunks[0][z];
                
                int32_t originX = m_landscapeChunks[1][z]->m_fPositionX - 512;
                int32_t originZ = m_landscapeChunks[1][z]->m_fPositionY;
                
                LandscapeChunk * chunk = (LandscapeChunk *)pChunkPoolAllocator->Alloc( sizeof(LandscapeChunk) );
                chunk->Clear();
                
                SBoundingBox bbox( Vec3(-512, -12.0f, -512), Vec3( 512, 412.0f, 512 ) );
                bbox.Translate( Vec3( originX - 512, 0.0f, originZ ) );
                
                chunk->Load( m_pLandscapeShader, bbox, 128, 512, originX - 512, originZ );
                chunk->m_fNoiseStartX = m_landscapeChunks[1][z]->m_fNoiseStartX - 512;
                chunk->m_fNoiseStartZ = m_landscapeChunks[1][z]->m_fNoiseStartZ;
                
                m_landscapeChunks[0][z] = chunk;
                OnChunkCreated( chunk );
            }
        }
        
        // z-aligned chunks.
        if( cameraPosition->z > m_landscapeChunks[0][2]->m_fPositionY ) {
            
            for( int32_t x = 0; x <= 2; ++x ) {
                OnChunkDestroyed( m_landscapeChunks[x][0] );
                m_landscapeChunks[x][0]->Destroy();
                pChunkPoolAllocator->Dealloc( m_landscapeChunks[x][0] );
                m_landscapeChunks[x][0] = NEKO_NULL;
                
                // Replace previous chunks.
                m_landscapeChunks[x][0] = m_landscapeChunks[x][1];
                m_landscapeChunks[x][1] = m_landscapeChunks[x][2];
                
                int32_t originX = m_landscapeChunks[x][1]->m_fPositionX;
                int32_t originZ = m_landscapeChunks[x][1]->m_fPositionY + 512;
                
                SBoundingBox bbox( Vec3(-512, -12.0f, -512), Vec3( 512, 412.0f, 512 ) );
                bbox.Translate( Vec3( originX, 0.0f, originZ + 512 ) );
                
                LandscapeChunk * chunk = (LandscapeChunk *)pChunkPoolAllocator->Alloc( sizeof(LandscapeChunk) );
                chunk->Clear();
                chunk->Load( m_pLandscapeShader, bbox, 128, 512, originX, originZ + 512 );
                chunk->m_fNoiseStartX = m_landscapeChunks[x][1]->m_fNoiseStartX;
                chunk->m_fNoiseStartZ = m_landscapeChunks[x][1]->m_fNoiseStartZ + 512;
                
                m_landscapeChunks[x][2] = chunk;
                OnChunkCreated( chunk );
            }
        } else if( cameraPosition->z < m_landscapeChunks[0][1]->m_fPositionY ) {
            
            for( int32_t x = 0; x <= 2; ++x ) {
                OnChunkDestroyed( m_landscapeChunks[x][2] );
                m_landscapeChunks[x][2]->Destroy();
                pChunkPoolAllocator->Dealloc(m_landscapeChunks[x][2]);
                m_landscapeChunks[x][2] = NEKO_NULL;
                
                // Replace previous chunks.
                m_landscapeChunks[x][2] = m_landscapeChunks[x][1];
                m_landscapeChunks[x][1] = m_landscapeChunks[x][0];
                
                int32_t originX = m_landscapeChunks[x][1]->m_fPositionX;
                int32_t originZ = m_landscapeChunks[x][1]->m_fPositionY - 512;
                
                SBoundingBox bbox( Vec3(-512, -12.0f, -512), Vec3( 512, 412.0f, 512 ) );
                bbox.Translate( Vec3( originX, 0.0f, originZ - 512 ) );
                
                LandscapeChunk * chunk = (LandscapeChunk *)pChunkPoolAllocator->Alloc( sizeof(LandscapeChunk) );
                chunk->Clear();
                chunk->Load( m_pLandscapeShader, bbox, 128, 512, originX, originZ - 512 );
                chunk->m_fNoiseStartX = m_landscapeChunks[x][1]->m_fNoiseStartX;
                chunk->m_fNoiseStartZ = m_landscapeChunks[x][1]->m_fNoiseStartZ - 512;
                
                m_landscapeChunks[x][0] = chunk;
                
                OnChunkCreated( chunk );
            }
        }
        
        int32_t     x;
        int32_t     z;
        
        // Fill a list with chunks to build.
        m_chunkBuildList.ClearAll();
        
        for( x = 0; x <= 2; ++x ) {
            for( z = 0; z <= 2; ++z ) {
                // Check if we need to rebuild chunks.
                if( m_landscapeChunks[x][z]->NeedsRebuild() == true ) {
                    m_chunkBuildList.PushBack( m_landscapeChunks[x][z] );
                }
            }
        }
        
        // Sort chunks.
        m_chunkBuildList.Sort( SortChunks );
    }

    /**
     *  Update world culling.
     */
    void CWorldHandler::UpdateCulling()
    {
        if( !m_bUpdateGPUCulling ) {
            return;
        }
        
        transform.MVP = g_Core->p_Camera->ViewProjectionMatrix;
        transform.ModelView = g_Core->p_Camera->ViewMatrix;
        transform.Viewport = Vec4( 0.0, 0.0, g_mainRenderer->renderWidth, g_mainRenderer->renderHeight );

        glEnable( GL_RASTERIZER_DISCARD );

        // render tree instances and apply culling
        m_pCullShader->Use();
        // Bind Hi-Z buffer.
        g_mainRenderer->BindTexture( 0, g_mainRenderer->g_sceneBuffer[0]->GetUnitDepth() );
        
        glBindBuffer( GL_UNIFORM_BUFFER, m_iMeshUniformBuffer );
        glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof(transform), &transform );
        // Select shader subroutine ( move me ).
        glUniformSubroutinesuiv( GL_VERTEX_SHADER, 1, &m_iCullShaderSubIndexVS[HI_Z_OCCLUSION_CULL] );
        glUniformSubroutinesuiv( GL_GEOMETRY_SHADER, 1, &m_iCullShaderSubIndexGS[1] );
        
        LandscapeChunk  * chunk = NEKO_NULL;
        
        for( int32_t x = 0; x <= 2; ++x ) {
            for( int32_t z = 0; z <= 2; ++z ) {
                chunk = m_landscapeChunks[x][z];
                
                if( chunk->GetQuadtree() != NEKO_NULL ) {
                    // Update landscape object culling.
                    m_pCullShader->SetUniform( m_iCullShaderUniforms[CWorldHandler::ECullShaderUniforms::Offset], static_cast<float>(0), static_cast<float>(0) );
                    
                    chunk->UpdateGPUCulledObjects();
                }
            }
        }
        
        g_mainRenderer->UnbindTexture( 0 );
        m_pCullShader->Next();
    
        glDisable( GL_RASTERIZER_DISCARD );
    }

    /**
     *  Update chunk handler.
     */
    void CWorldHandler::Update()
    {
        int32_t     x;
        int32_t     z;
        
        m_chunkBuildLock->Lock();
        
        // Update chunks, create or delete if needed.
        UpdateChunks();
        
        // Check if we have something in the build list..
        if( m_chunkBuildList.GetCount() > 0 ) {
            // Call the build event.
            m_chunkBuildEvent->Signal();
        }
        
        // Perform on this thread.
        for( x = 0; x <= 2; ++x ) {
            for( z = 0; z <= 2; ++z ) {
                if( m_landscapeChunks[x][z]->NeedsRebuild() == false ) {
                    LandscapeChunk * chunk = m_landscapeChunks[x][z];
                    
                    chunk->CreateBuffers( m_iMaxLOD );
                }
            }
        }
        
        m_chunkBuildLock->Unlock();
        
    }

    void CWorldHandler::CreateInstancedObjects()
    {
       
    }
    
    // Default inverse scale.
    const static float INVERSE_SCALE = -1.0f;
    // For a planar reflections.
    const static Vec3 INVERSE_VECSCALE( 1.0f, 1.0 * INVERSE_SCALE, 1.0 );
    const static Vec3 DEFAULT_VECSCALE( 1.0f, 1.0f, 1.0f ); // Default scale.
    
//        Vec3 playerPos = Vec3( g_Core->p_Camera->vEye / 512.0f ) * 512.0f;
//        Vec3 chunkPos = Vec3( 512.0 + playerPos.x, 0.0f, 512.0f + playerPos.z );

    /**
     *  Render landscapes!
     *
     *  @param eye   Renderer viewport.
     *  @param flags Rendering parameters.
     */
    void CWorldHandler::Render( CRenderer::ESceneEye eye, int32_t flags )
    {
        int32_t     x;
        int32_t     z;
        
        // Render now.
        if( (flags & (int32_t)EWorldFlag::Reflection) ) {
            m_modelView.Scale( INVERSE_SCALE );
        } else {
            m_modelView.Scale( DEFAULT_VECSCALE );
        }
        
        // View-projection matrix.
        m_projectionModelView = g_Core->p_Camera->ViewProjectionMatrix * m_modelView;
        
        ncMatrix4 prevView = g_Core->p_Camera->prevViewProj * m_prevModelView;
        
        // Use landscape shader.
        m_pLandscapeShader->Use();
        // Set matrices.
        m_pLandscapeShader->SetUniform( m_pShaderParams[ModelViewProj], 1, false, m_projectionModelView.m );
        m_pLandscapeShader->SetUniform( m_pShaderParams[ModelMatrix], 1, false, m_modelView.m );
        m_pLandscapeShader->SetUniform( m_pShaderParams[PreviousMatrix], 1, false, prevView.m );
        
        // Don't bind anything while in shadow rendering.
        if( !(flags & (int32_t)EWorldFlag::Shadow ) ) {
            // Bind textures.
            g_pbEnv->BindTerrainSummerTextures();
            
            
        } else {
            // Use shadow shader..
            
        }
        
        
        LandscapeChunk * chunk /*= NEKO_NULL*/;
		// Render chunks.
		for (x = 0; x <= 2; ++x) {
			for (z = 0; z <= 2; ++z) {
				chunk = m_landscapeChunks[x][z];

				if (chunk != NEKO_NULL && chunk->GetQuadtree() != NEKO_NULL) {
					// Render a chunk!
					chunk->Render(eye, flags);
				}
			}
		}

        // Unbind textures.
        g_mainRenderer->UnbindTexture( 0 );
        m_pLandscapeShader->Next();
        
        // Render instanced objects.
        if( m_bUpdateGPUCulling ) {
            for( x = 0; x <= 2; ++x ) {
                for( z = 0; z <= 2; ++z ) {
                    chunk = m_landscapeChunks[x][z];
                    
                    if( chunk->GetQuadtree() != NEKO_NULL ) {
                        chunk->RenderGPUCulledObjects();
                    }
                }
            }
        }


        // Render objects and foliages.
        for( x = 0; x <= 2; ++x ) {
            for( z = 0; z <= 2; ++z ) {
                chunk = m_landscapeChunks[x][z];
                if( chunk->GetQuadtree() != NEKO_NULL ) {
                    // Render a chunk!
                    
                    // We don't need shadows for path nodes.
                    if( !( flags & (int32_t)EWorldFlag::Shadow ) ) {
                        chunk->RenderPathNodes();
                    }
                    
                    if( !(flags & (int32_t)EWorldFlag::Cubemap) ) {
                        chunk->RenderEntities( eye, flags );
                    }
                }
            }
        }
    }

    /**
     *  Modify a vertex.
     */
    float CWorldHandler::GetModifierPoint( SEditVertices * surfaceVertices, int32_t vertexId, int32_t x, int32_t y )
    {
        // Get actual landscape height in the world coordinates.
        float height = GetHeightPointOnNoise( x, y );

        
        return height;
    }
    
    /**
     *  Get height point on noise.
     *
     *  Called for each different landscape chunk but with the same parameters.
     */
    float CWorldHandler::GetHeightPointOnNoise( int32_t x, int32_t y )
    {
        // TODO: 3d noise
        // TODO: vegetation noise
        
        float height  = m_noiseLarge->Get( x / fNoise_largeNoiseCoef, y / fNoise_largeNoiseCoef ) + 0.8f;
        height = height * height * height * 100.0f;
        
        // Large noise.
        if( height > m_fMaxHeight ) {
            float tp = m_noiseType->Get( x / fNoise_medNoiseCoef, y / fNoise_medNoiseCoef ) * 2.0f;
            
            // Medium noise.
            if( tp > 0.0f ) {
                float heightMac2 = m_noiseMed->Get( x / fNoise_typeNoiseCoef, y / fNoise_typeNoiseCoef ) + 0.6f;
                heightMac2 = heightMac2 * heightMac2 * heightMac2 * fNoise_multiplier;
                
                if( tp > 0.5 ) {
                    height = heightMac2 + fNoise_multiplier;
                } else {
                    tp *= 2.0f;
                    height = height * ( 1.0f - tp ) + ( heightMac2 + fNoise_multiplier ) * tp;
                }
            }
        }
  
        
        return height;
    }

    
    /**
     *  Constructor.
     */
    CWorldHandler::CWorldHandler() : m_bIsActive( false )
    {
        m_noiseLarge = NEKO_NULL;
        m_noiseMed = NEKO_NULL;
    }
    
    /**
     *  Destructor.
     */
    CWorldHandler::~CWorldHandler()
    {
        
    }
}
