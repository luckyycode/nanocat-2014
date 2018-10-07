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
//  QuadtreeNode.cpp
//  Quadtree Level of Detail node. :O
//
//  Created by Neko Vision on 1/8/14.
//  Copyright (c) 2013-2014 Neko Vision. All rights reserved.
//

#include "Quadtree.h"

#include "../../Platform/Shared/SystemShared.h"
#include "../../Core/Core.h"
#include "../../Core/Player/Camera/Camera.h"    // Frustum.
#include "../../Core/Network/MultiplayerServer.h"   // Server world and entities.
#include "../../World/BeautifulEnvironment.h"   // To get mTerrain.

namespace Neko {

    /**
     *  Create new quadtree node and chunk.
     *
     *  @param depth        Recursive step count.
     *  @param pos          Node position.
     *  @param mSize        Node size.
     *  @param minmSize     Minimum chunk size.
     */
    void CQuadtreeNode::Create( uint32_t depth, const Vec2i & pos,
                                const Vec2i & mSize, uint32_t minmSize, int16_t flags, INekoAllocator * allocator )
    {
        SMemoryTempFrame * tempArea = NEKO_NULL;
        
        pAllocator = allocator;
        
        m_iChunkLOD = 0;
        m_terrainChunk = NEKO_NULL;
        m_nodeChildren = NEKO_NULL;
        
        Vec2i nodesize;
        Vec2i newsize;
        
        uint32_t div;
        Vec3 center;   // Bounding box center origin.
        
        div = static_cast<uint32_t>(nkMath::FastPow( 2, depth ));
        nodesize = mSize / div;
        
        m_fLodOffset = ( minmSize * 5.0f ) / 100.0f;

        // Make it to not equal of two.
        if( (nodesize.x % 2) == 0 ) {
            ++nodesize.x;
        }
        
        if( (nodesize.y % 2) == 0 ) {
            ++nodesize.y;
        }

        newsize = nodesize / 2; // Decrease size depending on the depth.

        // Make new terrain chunk if possible.
        if( (uint32_t)nkMath::Max( newsize.x, newsize.y ) < minmSize ) {
            // Alloc a new chunk.
            m_terrainChunk = (CNodeChunk *)pAllocator->Alloc( sizeof(CNodeChunk) );
            // Create a new chunk.
            m_terrainChunk->Load( depth, pos, mSize, (EQuadtreeType)m_qType, true, flags, pAllocator, tempArea );

            m_nodeChildren = NEKO_NULL;
            
            return;
        }

        // Create four nodes for per side.
        m_nodeChildren = (CQuadtreeNode *)pAllocator->Alloc( sizeof(CQuadtreeNode) * 4 /* four children nodes at all world sides */ );

        // Calculate bounding box.
        center = m_boundingBox.GetCenter();

        // Setup node children.
        m_nodeChildren[CHILD_NW].m_boundingBox = SBoundingBox( m_boundingBox.min, center );
        m_nodeChildren[CHILD_NE].m_boundingBox = SBoundingBox( Vec3( center.x, 0.0f, m_boundingBox.min.z ),
                                                            Vec3( m_boundingBox.max.x, 0.0f, center.z ) );
        m_nodeChildren[CHILD_SW].m_boundingBox = SBoundingBox( Vec3( m_boundingBox.min.x, 0.0f, center.z ),
                                                            Vec3( center.x, 0.0f, m_boundingBox.max.z ) );
        m_nodeChildren[CHILD_SE].m_boundingBox = SBoundingBox( center, m_boundingBox.max );

        // Quadtree node type.
        m_nodeChildren[CHILD_NW].m_qType = m_qType;
        m_nodeChildren[CHILD_NE].m_qType = m_qType;
        m_nodeChildren[CHILD_SW].m_qType = m_qType;
        m_nodeChildren[CHILD_SE].m_qType = m_qType;

        // Heightmap nodes.
        Vec2i tNewHMpos[4];
        
        tNewHMpos[CHILD_NW] = pos + Vec2i( 0, 0 );
        tNewHMpos[CHILD_NE] = pos + Vec2i( newsize.x, 0 );
        tNewHMpos[CHILD_SW] = pos + Vec2i( 0, newsize.y );
        tNewHMpos[CHILD_SE] = pos + Vec2i( newsize.x, newsize.y );

        uint32_t i;
        for( i = 0; i < 4; ++i ) {
            m_nodeChildren[i].Create( depth + 1, tNewHMpos[i], mSize, minmSize, flags, allocator );
        }
    }

    /**
     *  Delete indices.
     */
    void CQuadtreeNode::DeleteIndices()
    {
        if( m_terrainChunk != NEKO_NULL ) {
            for( uint32_t i(0); i < 7; ++i ) {
                delete [] m_terrainChunk->m_tIndice[i].indices; // @replace me!!
                m_terrainChunk->m_tIndice[i].indices = NEKO_NULL;
            }
//            pAllocator->Dealloc( m_terrainChunk->m_tIndice[0].indices );
        }
        
        if( m_nodeChildren != NEKO_NULL ) {
            uint32_t i;
            for( i = 0; i < 4; ++i ) {
                m_nodeChildren[i].DeleteIndices();
            }
        }
    }
    
    /**
     *  Calculate bounding box for node.
     */
    void CQuadtreeNode::CalculateBoundingBoxAndCollisionData( const Vec3 *vertices, bool needsCollision, bool needsBoundingBox  )
    {
        uint32_t i;
        uint32_t * tIndices;
        
        SLink   * head;
        SLink   * cur;
        
        // Create bounding box if we need to.
        if( needsBoundingBox )
        {
            m_boundingBox.min.y = 100000.0f;
            m_boundingBox.max.y = -100000.0f;
            
            if( m_terrainChunk != NEKO_NULL ) {
                tIndices = m_terrainChunk->m_tIndice[0].indices;
                
                for( i = 0; i < m_terrainChunk->m_tIndice[0].count; ++i ) {
                    const Vec3 &vertex = vertices[tIndices[i]];
                    
                    if( vertex.y > m_boundingBox.max.y ) {
                        m_boundingBox.max.y = vertex.y;// + 64.0f;
                    }
                    
                    if( vertex.y < m_boundingBox.min.y ) {
                        m_boundingBox.min.y = vertex.y;
                    }
                }

                // Now we need to make a bounding box for
                // each object on this quadtree landscape.
                if( m_qType == TERRAIN )
                {
                    head = &m_terrainChunk->m_terrainObjects.m_sList;
                    
                    for( cur = head->m_pNext; cur != head; cur = cur->m_pNext )  {
                        // Get terrain object.
                        ncLTerrainObject * obj = (ncLTerrainObject *) cur->m_ptrData;
              
                        // Get object data.
                        ncMesh * mesh = obj->GetMesh();
                        
                        // Get object bounding box.
                        SBoundingBox * bbox = &mesh->GetHandle()->GetBoundingBox();
                        mesh->SetDimensions( bbox->min, bbox->max );
                 
                        // Set bounding box position.
                        mesh->SetBBoxOrigin( obj->GetPosition() );
                        
                        // Add bounding box data to common ones.
                        m_boundingBox.Add( *mesh->GetBoundingBox() );
                    }
                }
            }
            
            // Chunk bounding boxes.
            if( m_nodeChildren != NEKO_NULL )
            {
                uint32_t i;
                
                for( i = 0; i < 4; ++i ) {
                    m_nodeChildren[i].CalculateBoundingBoxAndCollisionData( vertices, false, true );
                    
                    if( m_boundingBox.min.y > m_nodeChildren[i].m_boundingBox.min.y ) {
                        m_boundingBox.min.y = m_nodeChildren[i].m_boundingBox.min.y;
                    }
                    
                    if( m_boundingBox.max.y < m_nodeChildren[i].m_boundingBox.max.y ) {
                        m_boundingBox.max.y = m_nodeChildren[i].m_boundingBox.max.y;
                    }
                }
            }
        }
        
        // Do we need to create collision chunks?
        if( needsCollision )
        {
            if( m_terrainChunk != NEKO_NULL ) {
                // Closest level of detail.
                if( m_iChunkLOD < 1 ) {
                    tIndices = m_terrainChunk->m_tIndice[0].indices;
                    
                    m_iColNum = m_terrainChunk->m_tIndice[0].count;
                    
                    // TODO: memory overlap check.
                    m_pCollisionChunks = (Vec3 *)pAllocator->Alloc( sizeof(Vec3) * m_iColNum );
                    
                    uint32_t i;
                    // Ok so we got size for each collision block.
                    // Just like previous function which generates Bounding boxes.
                    //printf( "LOLKA LAL: %i\n", tIndicesSize );
                    for( i = 0; i < m_iColNum; ++i ) {
                        m_pCollisionChunks[i] = vertices[tIndices[i]];
                    }
                }
            }
            
            // Chunk bounding boxes.
            if( m_nodeChildren != NEKO_NULL ) {
                uint32_t i;
                for( i = 0; i < 4; ++i ) {
                    m_nodeChildren[i].CalculateBoundingBoxAndCollisionData( vertices, true, false );
                }
            }
        }
    }
    
    /**
     *    Draw terrain objects.
     */
    int32_t CQuadtreeNode::DrawObjects( int16_t flags )
    {
        if( m_nodeChildren == NEKO_NULL ) {
            if( m_iChunkLOD >= 0 ) {
                return m_terrainChunk->DrawObjects( (flags & (int32_t)EWorldFlag::Reflection) ? (TERRAIN_CHUNKS_LOD - 1) : (int32_t)m_iChunkLOD, m_fDistance, (flags & (int32_t)EWorldFlag::Shadow) );
            } else {
                return 0;
            }
        }
        
        /**     Recurse to children nodes.  **/
        else
        {
            uint32_t ret;
            uint32_t i;
            
            ret = 0;
            if( m_iChunkLOD >= 0 ) {
                for( i = 0; i < 4; ++i ) {
                    ret += m_nodeChildren[i].DrawObjects( flags );
                }
            }

            return ret;
        }
    }

    /**
     *    Destroy a node with its nodes.
     */
    void CQuadtreeNode::Destroy()
    {
        
        
        // Delete collision chunks if exists.
        if( m_pCollisionChunks != NEKO_NULL ) {
            pAllocator->Dealloc( m_pCollisionChunks );
            m_pCollisionChunks = NEKO_NULL;
        }
        
        if( m_nodeChildren != NEKO_NULL ) {
            uint32_t i;
            
            for( i = 0; i < 4; ++i ) {
                m_nodeChildren[i].Destroy();
                memset( &m_nodeChildren[i], 0x00, sizeof(CQuadtreeNode) );
            }

            pAllocator->Dealloc( m_nodeChildren );
            m_nodeChildren = NEKO_NULL;
        }
        
        if( m_terrainChunk != NEKO_NULL )
        {
            m_terrainChunk->Delete();
            pAllocator->Dealloc( m_terrainChunk );
            
            m_terrainChunk = NEKO_NULL;
        }
    }
    
    /**
     *  Draw a terrain node.
     */
    int32_t CQuadtreeNode::Draw( CFrustum * frustum, int32_t options )
    {
        // Fix me.
        m_iChunkLOD = -1;

        // Get Bounding box center and radius of it.
        Vec3 center;
        Vec3 radlen;

        int32_t isInFrustum;
        int32_t boxInFrustum;

        /*const*/ float radius;
        m_bCanBeSeen = true;

        center = m_boundingBox.GetCenter();
        radlen = m_boundingBox.max - center;

        radius = radlen.Length();
        
        // Chunk frustum culling ( visibility ) test.
        if( options & (int32_t)EWorldFlag::PlayerEye ) {
            // Is camera inside of chunk's bounding box?
            if( !m_boundingBox.ContainsPoint( /*frustum->GetEyePosition()*/g_Core->p_Camera->vEye ) ) {
                isInFrustum = frustum->ContainsSphere( center, radius );
                m_bCanBeSeen = true;
                
                options &= ~(int32_t)EWorldFlag::PlayerEye;
                
                // Check frustum.
                switch( isInFrustum ) {
                    // Intersection out.
                    case CFrustum::FRUSTUM_OUT:
                        m_bCanBeSeen = false;
                        return 0;

                    // Intersection in.
                    case CFrustum::FRUSTUM_IN:
                        m_bCanBeSeen = true;
                        options &= ~(int32_t)EWorldFlag::PlayerEye;
                        break;

                    // Full intersection.
                    case CFrustum::FRUSTUM_INTERSECT:
                    {
                        boxInFrustum = frustum->ContainsBoundingBox( m_boundingBox );

                        switch( boxInFrustum ) {
                            case CFrustum::FRUSTUM_IN:
                                m_bCanBeSeen = true;
                                options &= ~(int32_t)EWorldFlag::PlayerEye;
                                break;

                            case CFrustum::FRUSTUM_OUT:
                                m_bCanBeSeen = false;
                                options &= ~(int32_t)EWorldFlag::PlayerEye;
                                return 0;
                        }
                    }
                }
            }
        } else if( options & (int32_t)EWorldFlag::Shadow ) {
            // Shadow frustum.
            for( int32_t i(0); i < g_pbEnv->m_pbEnvShadows->m_iShadowSplits; ++i ) {
                if( SBoundingBox::ContainsBoxes( g_pbEnv->m_pbEnvShadows->m_bBoundingBox[i].min, g_pbEnv->m_pbEnvShadows->m_bBoundingBox[i].min, m_boundingBox.min, m_boundingBox.max ) ) {
                    m_bCanBeSeen = true;
                }
                
//                if( g_pbEnv->m_pbEnvShadows->m_frustumSplits[i].ContainsSphere( center, radius ) ) {
//                    m_bCanBeSeen = true;
//                } else {
//                    m_bCanBeSeen =false;
//                }
            }
        }

        m_iChunkLOD = 0;

        if( m_nodeChildren == NEKO_NULL ) {
            if( (options & (int32_t)EWorldFlag::Reflection)  ) {   // Water reflection.
                // Render water reflection.
                if( m_qType != OCEAN ) {
                    // Oops.
                    return m_terrainChunk->DrawGround( TERRAIN_CHUNKS_LOD - 1 );
                }
            } else if( options & (int32_t)EWorldFlag::Cubemap  ) {  // Cubemap render.
                // Render to cubemap.
                if( m_qType != OCEAN ) {
                    // Oops.
                    return m_terrainChunk->DrawGround( TERRAIN_CHUNKS_LOD );
                }
            } else {                                        // World.
                Vec3   vEyeToChunk;
                uint32_t lod;

                // Calculate distance between chunk and camera position to
                // set right level of detail.
                vEyeToChunk = this->m_boundingBox.GetCenter() - /*frustum->GetEyePosition()*/g_Core->p_Camera->vEye;

                m_fDistance = vEyeToChunk.Length();

                lod = 1;
                if( options & (int32_t)EWorldFlag::Shadow ) {
                    // NOTE NOTE MOTE Mustn't to be less or equal one
                    // since it doesn't need to check collision and another stuff!
                    lod = 4;    // TODO: Customizable value.
                } else {
                    if( m_qType == TERRAIN ) {
                        if( m_fDistance > TERRAIN_CHUNK_LOD5 )
                            lod = 6;
                        else if( m_fDistance > TERRAIN_CHUNK_LOD4 )
                            lod = 5;
                        else if( m_fDistance > TERRAIN_CHUNK_LOD3 )
                            lod = 4;
                        else if( m_fDistance > TERRAIN_CHUNK_LOD2 )
                            lod = 3;
                        else if( m_fDistance > TERRAIN_CHUNK_LOD1 )
                            lod = 2;
                        else if( m_fDistance > TERRAIN_CHUNK_LOD0 )
                            lod = 1;
                    } else if( m_qType == OCEAN ) {
                        m_fDistance -= 1792.0f;// just to be sure
                        
                        // Ocean.
                        if( m_fDistance > OCEAN_CHUNK_LOD5 )
                            lod = 6;
                        else if( m_fDistance > OCEAN_CHUNK_LOD4 )
                            lod = 5;
                        else if( m_fDistance > OCEAN_CHUNK_LOD3 )
                            lod = 4;
                        else if( m_fDistance > OCEAN_CHUNK_LOD2 )
                            lod = 3;
                        else if( m_fDistance > OCEAN_CHUNK_LOD1 )
                            lod = 2;
                        else if( m_fDistance > OCEAN_CHUNK_LOD0 )
                            lod = 1;
                    }
                }
                
                // We check only closer level of detail to gain just A LOT MORE of performance
                // and check if collision chunk even exists.
                // If everything is okay, simply check it.
                
                // First LoD number is enough for us. Optionally we can make it more and we'll lose some
                // performance, but in this case it is really useless. So I'll leave '1' here.
                if( lod <= 1 && m_pCollisionChunks != NEKO_NULL/* means m_iColNum > 0 && qType != OCEAN */ ) {
                    // Temp routine. ( 0 - me )
                    CServerClient * client = g_Core->p_Server->GetClientByNum( 0 );
                    g_pWorldPhysics->UpdateWorldNode( this, client->m_pEntity );
                }
                
                // Set chunk LoD.
                m_iChunkLOD = lod;
                
                return m_terrainChunk->DrawGround( lod ); // Finally draw it.
            }
        } else {
            uint32_t ret;
            uint32_t i;
            
            ret = 0;
            
            if( m_nodeChildren!=NEKO_NULL ) {
                for( i = 0; i < 4; ++i ) {
                    ret += m_nodeChildren[i].Draw( frustum, options );
                }
            }
            
            return ret;
        }

        return 0;
    }

    /** 
     *  Draw foliage object on quadtree.
     */
    int32_t CQuadtreeNode::DrawFoliage()
    {
        if( !m_nodeChildren ) {
            if( m_iChunkLOD >= 0 ) {
                return m_terrainChunk->DrawFoliage( (int32_t)m_iChunkLOD, m_fDistance );
            } else {
                return 0;
            }
        } else {
            uint32_t ret;
            uint32_t i;

            ret = 0;

            if( m_iChunkLOD >= 0 ) {
                for( i = 0; i < 4; ++i ) {
                    ret += m_nodeChildren[i].DrawFoliage();
                }
            }
            
            return ret;
        }
    }

  
}
