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
//  LandscapeChunk.cpp
//  Beautiful terrain renderer..
//
//  Created by Neko Code on 2/8/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.
//

// @note this file creates a chunk node
// @todo a new object spawn system with asset streaming
// @todo road(path) mesh quality modes
// @finish gpu instanced object rendering (it's a bit mess below)

#include "../../Core/Core.h"
#include "../../AssetCommon/AssetBase.h"
#include "../../Graphics/OpenGL/GLShader.h"
#include "../../Core/Player/Camera/Camera.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Core/Player/Input/Input.h"
#include "../../Core/Utilities/Utils.h"
#include "../../Graphics/Renderer/ShadowRenderer.h"
#include "../Mesh.h"
#include "../BeautifulEnvironment.h"
#include "LandscapeChunk.h"

//  Landscape chunk.

namespace Neko {
    
    /**
     *  Prepare new terrain.
     */
    LandscapeChunk::LandscapeChunk()
    {
        Clear();
    }
    
    LandscapeChunk::~LandscapeChunk()
    {
        // All stuff gets deleted in 'Destroy' method.
    }
    
    /**
     *  Create new terrain.
     */
    void LandscapeChunk::Load( SGLShader * g_pbEnvShader, SBoundingBox& mbox, const uint32_t chunkObjectsNum, const uint32_t chunkSize, const int32_t startX, const int32_t startY  )
    {
        Clear();
        
        m_foliageObjectCount = chunkObjectsNum;
        
        m_iSize = chunkSize;
        
        // Chunk position.
        this->m_fPositionX = startX;
        this->m_fPositionY = startY;
        
        m_tBoundingBox = mbox;
        m_bRebuildNormals = true;
        
        bNeedsRebuild = true;
        
        // Just to be sure.
        
        m_pChunkShader = g_pbEnvShader; // Every landscape will have own special 'effects'.
        
        
        // Setup road shader. Can be used for every path type ( car road, sandy road etc .. )
        m_pRoadShader = f_AssetBase->FindAssetByName<SGLShader>( "road" );
        m_pRoadShader->Use();
        
        // Set sampler uniforms.
        m_pRoadShader->SetUniform( "samplerTex", 0 );
        m_pRoadShader->SetUniform( "normalTex", 1 );
        m_pRoadShader->SetUniform( "rgbMap", 1 );   // RMA material map for PBR.
        
        // Road mesh uniforms.
        m_iRoadLoc[LOC_MODELVIEW] = m_pRoadShader->UniformLocation( "MVP" );
        m_iRoadLoc[LOC_MODELMATRIX] = m_pRoadShader->UniformLocation( "ModelMatrix" );
  
        m_pRoadShader->Next();
    }
    
    /**
     *  Clear landscape properties.
     */
    void LandscapeChunk::Clear()
    {
        m_Positions = NEKO_NULL;
        m_Normals = NEKO_NULL;
        
        m_tQuadtree = NEKO_NULL;
        
        bActive = false;
        
        m_iFacesDrawn = 0;
        m_iObjectsDrawn = 0;
        
        m_pMeshData = NEKO_NULL;
        m_map_size = 0;
        m_index_size = 0;
        
        m_foliageObjectCount = 256 / 2;
        
        m_iSize = 0;
        
        m_bRebuildNormals = true;
        
        m_iMeshInstanceBaseCount = 0;
    }
    
    /**
     *  Create a new path node.
     *
     *  @param nodeCell Node cell.
     */
    void LandscapeChunk::CreatePathNode( SChunkPathCell * nodeCell )
    {
        // Local space coordinates.
        Vec3   pathPosition( nodeCell->Position.x, nodeCell->HeightAt, nodeCell->Position.y );
        
        // Add node path.
        AddNode( pathPosition, m_pPathNodes[m_iNodePaths - 1]->m_fPathWidth );
        
        // .. and create it!
        CreatePath( 16, true, true );
        
        // Notify us.
        g_Core->p_Console->Print( LOG_INFO,"CreatePathNode(): A new path node was created at <%4.2f, %4.2f, %4.2f>\n", pathPosition.x, pathPosition.y, pathPosition.z );
    }
    
    /**
     *  Get height on the procedural landscape chunk.
     */
    float LandscapeChunk::GetNoiseHeightAt( int32_t x, int32_t z, bool localCoords )
    {
        float   h;
        
        // TODO: Pointer as the argument.
        if( localCoords == true ) {
            h  = g_pbEnv->m_pWorldHandler->GetHeightPointOnNoise( x + m_fNoiseStartX, z + m_fNoiseStartZ );
        } else {
            h  = g_pbEnv->m_pWorldHandler->GetHeightPointOnNoise( x , z  );
        }
        
        return h;
    }
    
    /**
     *  Add a new node to the chunk.
     */
    void LandscapeChunk::AddNode( const Vec3 position, const float width )
    {
        int32_t     iNodes;
        
        SPathNodeObjects    nodePath;
        SPathNodeObjects    * pathNodes = NEKO_NULL;
        
        SPathInfo * pathInfo = m_pPathNodes[m_iNodePaths - 1];
        
        // Increase nodes created value.
        iNodes = pathInfo->m_iPathCount + 1;
        
        // Set the node properties.
        nodePath.Position = position;
        nodePath.Width = width;
        
        // Create a new array of nodes.
        pathNodes = (SPathNodeObjects *)pAllocator->Alloc( sizeof(SPathNodeObjects) * iNodes );
        
        int32_t n = iNodes;
        int32_t i;
        
        // Update nodes.
        for( i = 0; i < n; ++i ) {
            // Set the node width.
            pathNodes[i].Width = width;
            
            // Add a new node.
            if( i != (n - 1) ) {
                pathNodes[i] = pathInfo->m_pNodeObjects[i];
                
            } else {
                pathNodes[i] = nodePath;
                
                g_Core->p_Console->Print( LOG_INFO, "AddPathNode(): A new node at <%4.2f, %4.2f, %4.2f> origin and %4.2f width.\n", pathNodes[i].Position.x, pathNodes[i].Position.y, pathNodes[i].Position.z, pathNodes[i].Width );
                
                // Increase path node count.
                ++pathInfo->m_iPathCount;
            }
        }
        
        // Remove previous data.
        if( pathInfo->m_pNodeObjects != NEKO_NULL ) {
            pAllocator->Dealloc( pathInfo->m_pNodeObjects );
        }
        
        pathInfo->m_pNodeObjects = pathNodes;
    }
    
    /**
     *  Submit chunk modification and place nodes.
     */
    bool LandscapeChunk::FinalizePath()
    {
        
        
        return true;
    }
    
    /**
     *  Create a new path mesh.
     *
     *  @param smoothingLevel Smoothing level.
     *  @param flatten        Flatten mesh?
     *  @param road           Is it road node?
     *
     *  @note   Use only for local chunk space!
     *  @TODO   Indepent mesh in the world space.
     */
    void LandscapeChunk::CreatePath( int32_t smoothingLevel, bool flatten, bool road )
    {
        // Current path node information.
        SPathInfo * pathInfo = m_pPathNodes[m_iNodePaths - 1];
        SMemoryTempFrame * tempMem = _PushMemoryFrame( pLinearAllocator2 );
        
        // Delete previous mesh.
        if( pathInfo->m_pNodeMesh != NEKO_NULL ) {
            pathInfo->m_pNodeMesh->Destroy();
            pAllocator->Dealloc( pathInfo->m_pNodeMesh );
        }
        
        pathInfo->m_pNodeMesh = (CBasicMesh *)pAllocator->Alloc( sizeof(CBasicMesh) );
        pathInfo->m_iNodeFlags = 0;
        
        // Check if we got path nodes.
        if( pathInfo->m_pNodeObjects == NEKO_NULL || pathInfo->m_iPathCount < 2 ) {
            g_Core->p_Console->Print( LOG_INFO, "CreatePath(): Error, no node data found.\n" );
            return;
        }

        int32_t     i;
        int32_t     j;
        
        int32_t n = pathInfo->m_iPathCount ;
        
        // Strip.
        const int32_t verticesPerNode = 2 * (smoothingLevel + 1) * 2;
        const int32_t trianglesPerNode = 6 * (smoothingLevel + 1);
        
        const int32_t verticesCount = verticesPerNode * (n - 1);
        const int32_t trianglesCount = trianglesPerNode * (n - 1);
        
        // UV data.
        Vec2 * uvs = (Vec2 *)PushMemory( tempMem, sizeof(Vec2) * verticesCount );
        // Vertex data.
        Vec3 * newVertices = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * verticesCount );
        
        // Index data.
        uint32_t * newTriangles = (uint32_t *)PushMemory( tempMem, sizeof(uint32_t) * trianglesCount );
        
        // Will be removed, node object data with temporary nodes.
        pathInfo->m_nodeObjectVerts = new Vec3[verticesCount];
        
        int32_t nextVertex      = 0;
        int32_t nextTriangle    = 0;
        int32_t nextUV          = 0;
        
        // variables for splines and perpendicular extruded points
        float * cubicX = (float *)PushMemory( tempMem, sizeof(float) * n );
        float * cubicZ = (float *)PushMemory( tempMem, sizeof(float) * n );
        
        Vec3   handle1Tween;
        
        // Points of curve data.
        Vec3 * g1 = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * (smoothingLevel + 1) );
        Vec3 * g2 = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * (smoothingLevel + 1) );
        Vec3 * g3 = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * (smoothingLevel + 1) );
        
        Vec3   oldG2;
        Vec3   extrudedPointL;
        Vec3   extrudedPointR;
        
        // Initialise start node origins.
        for( i = 0; i < n; ++i ) {
            cubicX[i] = pathInfo->m_pNodeObjects[i].Position.x;
            cubicZ[i] = pathInfo->m_pNodeObjects[i].Position.z;
        }
        
        // Create the mesh.
        for( i = 0; i < n; ++i ) {
            g1 = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * (smoothingLevel + 1) );
            g2 = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * (smoothingLevel + 1) );
            g3 = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * (smoothingLevel + 1) );
            
            extrudedPointL = Vec3( 0.0f );
            extrudedPointR = Vec3( 0.0f );
            
            if( i == 0 ) {   // First mesh vertices.
                newVertices[nextVertex] = pathInfo->m_pNodeObjects[0].Position;
                ++nextVertex;
                
                uvs[0] = Vec2(0.0f, 1.0f);
                ++nextUV;
                
                newVertices[nextVertex] = pathInfo->m_pNodeObjects[0].Position;
                ++nextVertex;
                
                uvs[1] = Vec2(1.0f, 1.0f);
                ++nextUV;
                
                continue;
            }
            
            // Smooth Bezier curve/Spline generation.
            for( j = 0; j < smoothingLevel + 1; ++j )
            {
                if( i == 1 ) {
                    if( j != 0 ) {
                        newVertices[nextVertex] = newVertices[nextVertex - 2];    // Vertices.
                        ++nextVertex;
                        newVertices[nextVertex] = newVertices[nextVertex - 2];
                        ++nextVertex;
                        
                        uvs[nextUV] = Vec2(0.0f, 1.0f);        // UVs.
                        ++nextUV;
                        uvs[nextUV] = Vec2(1.0f, 1.0f);
                        ++nextUV;
                    } else {
                        oldG2 = pathInfo->m_pNodeObjects[0].Position;
                    }
                } else {
                    newVertices[nextVertex] = newVertices[nextVertex -  2]; // Vertices.
                    ++nextVertex;
                    newVertices[nextVertex] = newVertices[nextVertex - 2];
                    ++nextVertex;
                    
                    uvs[nextUV] = Vec2( 0.0f, 1.0f );    // UVs.
                    ++nextUV;
                    uvs[nextUV] = Vec2( 1.0f, 1.0f );
                    ++nextUV;
                }
                
                float u = (float)j / (float)(smoothingLevel + 1);
                
                // Calculate Spline curve cubic.
                NekoUtils::Cubic * X = NekoUtils::calcNaturalCubic( n - 1, cubicX );
                NekoUtils::Cubic * Z = NekoUtils::calcNaturalCubic( n - 1, cubicZ );
                
                // A point of curve.
                Vec3   tweenPoint( X[i - 1].eval( u ), 0.0f, Z[i - 1].eval( u ) );
                
                g2[j] = tweenPoint;
                g1[j] = oldG2;
                g3[j] = g2[j] - g1[j];
                oldG2 = g2[j];
                
                // Create perpendicular points for vertices.
                extrudedPointL = Vec3( -g3[j].z, 0.0f, g3[j].x );
                extrudedPointR = Vec3( g3[j].z, 0.0f, -g3[j].x );
                
                extrudedPointL.Normalize();
                extrudedPointR.Normalize();
                
                extrudedPointL = extrudedPointL * (float)pathInfo->m_fPathWidth;
                extrudedPointR = extrudedPointR * (float)pathInfo->m_fPathWidth;
                
                // Height at the terrain
                tweenPoint.y = GetHeightAt( tweenPoint.x, tweenPoint.z );
                
                // create vertices at the perpendicular points
                newVertices[nextVertex] = tweenPoint + extrudedPointR;
                
                // Modifiers.
                if( road == false ) {
                    newVertices[nextVertex].y = (GetHeightAt( (int32_t)newVertices[nextVertex].x,
                                                             (int32_t)newVertices[nextVertex].z ) +
                                                 newVertices[nextVertex - 2].y) / 2.0f;
                } else {
                    newVertices[nextVertex].y = GetHeightAt( (int32_t)newVertices[nextVertex].x,
                                                            (int32_t)newVertices[nextVertex].z );
                }
                
                // Update object vertices.
                pathInfo->m_nodeObjectVerts[nextVertex] = newVertices[nextVertex];
                ++nextVertex;
                
                newVertices[nextVertex] = tweenPoint + extrudedPointL;
                
                if( road == false ) {
                    newVertices[nextVertex].y = (GetHeightAt( (int32_t)newVertices[nextVertex].x,
                                                             (int32_t)newVertices[nextVertex].z ) +
                                                 newVertices[nextVertex - 2].y) / 2.0f;
                } else {
                    newVertices[nextVertex].y = GetHeightAt( (int32_t)newVertices[nextVertex].x,
                                                            (int32_t)newVertices[nextVertex].z );
                }
                
                pathInfo->m_nodeObjectVerts[nextVertex] = newVertices[nextVertex];
                ++nextVertex;
                
                uvs[nextUV] = Vec2(0.0f, 0.0f);
                ++nextUV;
                uvs[nextUV] = Vec2(1.0f, 0.0f);
                ++nextUV;
                
                // Used later to update the handles.
                if( i == 1 ) {
                    if( j == 0 ) {
                        handle1Tween = tweenPoint;
                    }
                }
                
                // Flatten mesh
                if( flatten && road == false ) {
                    if( newVertices[nextVertex - 1].y < newVertices[nextVertex - 2].y ) {
                        extrudedPointL = extrudedPointL * 1.5f;
                        extrudedPointR = extrudedPointR * 1.2f;
                        
                        newVertices[nextVertex - 1] = tweenPoint + extrudedPointL;
                        newVertices[nextVertex - 2] = tweenPoint + extrudedPointR;
                        
                        newVertices[nextVertex - 1].y = newVertices[nextVertex - 2].y;
                    } else if( newVertices[nextVertex - 1].y > newVertices[nextVertex - 2].y ) {
                        extrudedPointR = extrudedPointR * 1.5f;
                        extrudedPointL = extrudedPointL * 1.2f;
                        
                        newVertices[nextVertex - 2] = tweenPoint + extrudedPointR;
                        newVertices[nextVertex - 1] = tweenPoint + extrudedPointL;
                        
                        newVertices[nextVertex - 2].y = newVertices[nextVertex - 1].y;
                    }
                }
                
                delete X;
                delete Z;
                
                // Create triangles...
                // TODO: Topology.
                
                newTriangles[nextTriangle] = (verticesPerNode * (i - 1)) + (4 * j);     // 0
                ++nextTriangle;
                newTriangles[nextTriangle] = (verticesPerNode * (i - 1)) + (4 * j) + 1; // 1
                ++nextTriangle;
                newTriangles[nextTriangle] = (verticesPerNode * (i - 1)) + (4 * j) + 2; // 2
                ++nextTriangle;
                newTriangles[nextTriangle] = (verticesPerNode * (i - 1)) + (4 * j) + 1; // 1
                ++nextTriangle;
                newTriangles[nextTriangle] = (verticesPerNode * (i - 1)) + (4 * j) + 3; // 3
                ++nextTriangle;
                newTriangles[nextTriangle] = (verticesPerNode * (i - 1)) + (4 * j) + 2; // 2
                ++nextTriangle;
            }
        }
        
        // Update handles
        g2[0] = handle1Tween;
        g1[0] = pathInfo->m_pNodeObjects[0].Position;
        g3[0] = g2[0] - g1[0];
        
        extrudedPointL = Vec3( -g3[0].z, 0.0f, g3[0].x );
        extrudedPointR = Vec3( g3[0].z, 0.0f, -g3[0].x );
        
        extrudedPointL.Normalize();
        extrudedPointR.Normalize();
        
        extrudedPointL = extrudedPointL * pathInfo->m_pNodeObjects[0].Width;
        extrudedPointR = extrudedPointR * pathInfo->m_pNodeObjects[0].Width;
        
        // Reset starting vertex data.
        newVertices[0] = pathInfo->m_pNodeObjects[0].Position + extrudedPointR;
        newVertices[0].y = GetHeightAt( (int32_t)newVertices[0].x, (int32_t)newVertices[0].z );
        
        newVertices[1] = pathInfo->m_pNodeObjects[0].Position + extrudedPointL;
        newVertices[1].y = GetHeightAt( (int32_t)newVertices[1].x, (int32_t)newVertices[1].z );
        
        if( road == true ) {
            // Set the mesh vertex heights.
            for( i = 0; i < (verticesPerNode * (n - 1)); ++i ) {
                float x = floorf(newVertices[i].x);
                float z = floorf(newVertices[i].z);
                
                // Generate a new height value.
                newVertices[i].y = GetHeightAt( x, z ) + 2.05f;
                
                // TODO: terrain deformation.
            }
        }
        
        //  - Generate mesh normals.
        
        // Use 'uvs', 'newVertices', 'newTriangles'.
        Vec3 * vertexNormals = (Vec3 *)PushMemory( tempMem, sizeof(Vec3) * verticesCount);
        
        for( i = 0; i < verticesCount; ++i ) {
            vertexNormals[i] = Vec3( 0.0f, 1.0f, 0.0f ); // Y aligned.
        }
        
        // Create mesh on GPU.
        pathInfo->m_pNodeMesh->Create( newVertices, vertexNormals, uvs, newTriangles, verticesCount, trianglesCount );
        
        g_Core->p_Console->Print( LOG_INFO, "CreatePath(): A new node was added at <%4.2f, %4.2f, %4.2f>\n", pathInfo->m_pNodeObjects[0].Position.x, pathInfo->m_pNodeObjects[0].Position.y, pathInfo->m_pNodeObjects[0].Position.z );
        
        _PopMemoryFrame( tempMem );
        
        // we don't want to see the mesh
        // TODO: visible meshes if we need to
        pathInfo->m_iNodeFlags = 1;
    }
    
    /**
     *  Create node cache.
     */
    void LandscapeChunk::CreateNodeCache( SPathInfo * pathInfo, const uint32_t maxNodes )
    {
        int32_t i;
        SPathInfo * info;
        
        m_pPathNodes[m_iNodePaths] = pathInfo;
        
        info = m_pPathNodes[m_iNodePaths];
        // Create path node data.
        info->m_pNodeObjects = (SPathNodeObjects *)pAllocator->Alloc( sizeof(SPathNodeObjects) * maxNodes );
        
        for( i = 0; i < maxNodes; ++i ) {
            info->m_pNodeObjects[i].Position = Vec3( 1.0f );
            info->m_pNodeObjects[i].Width = 8.0f;
        }
        
        info->m_iPathCount = 0;
        m_iNodePaths++;
        
    }
    
    /**
     *  Update objects to cull.
     */
    void LandscapeChunk::UpdateGPUCulledObjects()
    {
        int32_t     i, k;
        
        glBeginTransformFeedback( GL_POINTS );
        
        for( k = 0; k < m_iMeshInstanceBaseCount; ++k ) {
            glBindVertexArray( m_pMeshInstances[k]->m_iCullVertexArray );
            
            for( i = 0; i < 3; ++i ) {
                glBeginQueryIndexed( GL_PRIMITIVES_GENERATED, i, m_pMeshInstances[k]->m_iCullQuery[i] );
            }
            
            // TODO: is visible?
//            if( m_tQuadtree->nodeRoot->m_bCanBeSeen ) {
            glDrawArrays( GL_POINTS, 0, m_foliageObjectCount );
//            }
            
            for( i = 0; i < 3; i++ ) {
                glEndQueryIndexed( GL_PRIMITIVES_GENERATED, i );
            }
        }
        
        glEndTransformFeedback();
    }
    
    /**
     *  Render non-culled objects.
     */
    void LandscapeChunk::RenderGPUCulledObjects()
    {
        const ncMatrix4 & m_projectionMatrix = g_Core->p_Camera->ViewProjectionMatrix;
        
        ncMesh ** mesh = NEKO_NULL;
        
        int32_t     i;
        int32_t     k;
        
        for( k = 0; k < m_iMeshInstanceBaseCount; ++k ) {
            // Select a mesh instance base.
            mesh = m_pMeshInstances[k]->m_pMesh;
            
            for( i = 0; i < 3; ++i ) {
                f_AssetBase->p_MeshBase->fxmodel->Use();
                
                // Set properties.
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MPVM], 1, false, m_projectionMatrix.m );
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MTIME], g_Core->GetTime() * 0.001f );
                
                glGetQueryObjectiv(m_pMeshInstances[k]->m_iCullQuery[i], GL_QUERY_RESULT, (GLint*)&m_pMeshInstances[k]->m_iVisibleObjects[i]);
                
//                printf( "instanceBase=%i visibleObjects=%i\n", k, m_pMeshInstances[k]->m_iVisibleObjects[is] );
                
                // Render objects.
                if ( m_pMeshInstances[k]->m_iVisibleObjects[i] > 0 ) {
                    mesh[i]->Render( false, m_pMeshInstances[k]->m_iVisibleObjects[i] );
                }
                
                f_AssetBase->p_MeshBase->fxmodel->Next();
            }
        }
    }
    
    /**
     *  Create terrain chunk.
     */
    void LandscapeChunk::Create( INekoAllocator * allocator, SMemoryTempFrame * tempMemory, bool threaded )
    {
        Vec3       * NormalFaces = NEKO_NULL;
        
        pAllocatorHandle = pMainAllocProxy;
        // Create stack base for each chunk to watch chunk creation order.
        pAllocator = NekoAllocator::newStackAllocator( Megabyte(16), *pAllocatorHandle );
 
        m_iNodePaths = 0;
        m_bThreaded = threaded;
        
//        // Create path node data and reset it.
//        m_pPathNodes = (SPathInfo **)pAllocator->Alloc( sizeof(SPathInfo*) * MAX_PATH_NODES );
        
        m_iSizeM1 = m_iSize;
        
        // Make sizes not equal of two ( 1024 -> 1025, 256 -> 257, etc ), we need an index to finish.
        if( m_iSize % 2 == 0 ) { ++m_iSize; }
        
        this->m_map_size = ( m_iSize * m_iSize );
        this->m_index_size = m_map_size * 2;
        
        // Create buffers now.
        m_Positions = (Vec3*)PushMemory( tempMemory, sizeof(Vec3) * m_map_size );
        m_Normals = (Vec3*)PushMemory( tempMemory, sizeof(Vec3) * m_map_size );
        
        // Sanity check.
        if( m_Positions == NEKO_NULL || m_Normals == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "CreateChunk(): failed to allocate buffer data." );
            return;
        }
        
        int32_t     id;
        int32_t     z;
        int32_t     x;
        
        float   h;    // Vertex height value.
        
//        // Create a vertex data for landscape chunk post-process.
//        m_verticesToEdit.Create( m_map_size );
        
        for( z = 0; z < m_iSize; ++z )
        {
            for( x = 0; x < m_iSize; ++x )
            {
                id = (z * m_iSize + x);
                
                // Noise start point values are set in chunk handler.
                h  = 10.0f;//g_pbEnv->m_pWorldHandler->GetModifierPoint( &m_verticesToEdit, id, this->m_fNoiseStartX + x, this->m_fNoiseStartZ + z );  // TODO: Pointer as the argument.
                
                /* Vertex data. Bounding box is used to set the chunk position, that helps us to keep nice position values for each vertex. */
                
                m_Positions[id].y = m_tBoundingBox.min.y + h * (m_tBoundingBox.max.y - m_tBoundingBox.min.y) / m_iSizeM1;
                m_Positions[id].x = m_tBoundingBox.min.x + (float)x * (m_tBoundingBox.max.x - m_tBoundingBox.min.x) / m_iSizeM1;
                m_Positions[id].z = m_tBoundingBox.min.z + (float)z * (m_tBoundingBox.max.z - m_tBoundingBox.min.z) / m_iSizeM1;
            }
        }
        
//        m_pBoundingSphere = BoundingSphere::Calculate( m_Positions, m_map_size );
        
        // Postprocess modifying. Adds roads, paths and such kind of ways, also rocks, etc..
//        Modify( false );
        
        // Normals.
        const uint32_t facesNum = ((m_iSize * 2) * m_iSize);
        // FIXME: does not fit memory pool
        NormalFaces = new Vec3[facesNum];//(Vec3*)Push( tempMemory, sizeof(Vec3) * facesNum );
        
        // Sanity check.
        if( NormalFaces == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "CreateChunk(): Failed to allocate %i normal faces for a chunk." );
            return;
        }
        
        //! Calculate terrain normals.
        CalculateNormals( 0, 0, NormalFaces, true );
        
        delete [] NormalFaces;
        
        // Initially, we don't render it.
        bRecreateBuffers = true;
        bNeedsRebuild = false;
        bActive = true;

        g_Core->p_Console->Print( LOG_INFO, "CreateLandscapeChunk(): successfully created at <%i, %i>!\n", m_fPositionX, m_fPositionY );
    }
    
    /**
     *  Create terrain quadtree and make bounding box.
     *
     *  @param chunking     Minimal chunk pieces amount.
     */
    void LandscapeChunk::CreateQuadtree( const uint32_t chunking )
    {
//        static boowl YEA = true;
        if( m_tQuadtree != NEKO_NULL ) {
            return;
        }
        
        // Generate a terrain quadtree.
        m_tQuadtree = (CQuadtree*)pAllocator->Alloc( sizeof(CQuadtree) );
        m_tQuadtree->Create( TERRAIN, &m_tBoundingBox, Vec2i( m_iSize ), chunking, WORLD_DUMMY, pMainAllocProxy );
    }
    
    /**
     *  Finish quadtree creation.
     */
    void LandscapeChunk::FinishQuadtree( const int32_t maxLod )
    {
        // Must be called before bounding box/collision generation!!
//        MakeObjects( maxLod );
        
        m_tQuadtree->CalculateBoundingBoxAndCollisionData( &m_Positions[0], true /* has bounding box(es) */, true /* has collision */ );
        
        m_tQuadtree->DeleteIndices();
    }
    
    /**
     *   Create vertex objects.
     */
    void LandscapeChunk::CreateVertexObjects()
    {
        // Generate vertex buffer objects.
        
        // Chunk mesh.
        m_pMeshData = (CBasicMesh *)pAllocator->Alloc( sizeof(CBasicMesh), __alignof(CBasicMesh) );
        m_pMeshData->Create( m_Positions, m_Normals, NEKO_NULL, NEKO_NULL, m_map_size, 0 );
        
        // Refresh settings.
        Refresh();
    }
    
    /**
     *  Generate normals.
     *
     *  @param diameter Normal diameter.
     *  @param index    Start index.
     *  @param cheap    Nice normals?
     */
    void LandscapeChunk::CalculateNormals( int32_t diameter, uint32_t index, Vec3 * normalFaces, bool cheap )
    {
        Vec3 * Positions = m_Positions;
        Vec3 * face_p = normalFaces;
        
        uint32_t i;
        
        if( cheap == true ) {
            // Rough estimate face normals.
            
            if( diameter == 0 ) {
                for( i = 0; i < m_map_size; ++i )
                {
                    // Three vertices forming plane.
                    Vec3 & p0 = Positions[i];
                    Vec3 & p1 = Positions[i + 1];
                    Vec3 & p2 = Positions[i + m_iSizeM1];
                    
                    // Calculate vectors.
                    Vec3 v0 = p1 - p0;
                    Vec3 v1 = p2 - p0;
                    
                    // Cross and normalize.
                    face_p[i].Cross( v1, v0 );
                    face_p[i].Normalize();
                    
                    m_Normals[i] = face_p[i];
                }
            } else {
                int32_t     min, max, rad;
                int32_t     diff;
                
                uint32_t    c;
                
                rad = diameter / 2;
                min = index - (rad * m_iSizeM1) - rad;
                max = index + (rad * m_iSizeM1) + rad;
                
                diff = max - min;
                
                // Corrent out-of-bound values.
                if( max > m_map_size-m_iSizeM1 ) {
                    max = m_map_size-m_iSizeM1;
                    min = max - diff;
                }
                
                if( min < 0 ) {
                    min = 0;
                    max = diff;
                }
                
                c = 0;
                for( i = min; i < max; ++i )
                {
                    Vec3 & p0 = Positions[i];
                    Vec3 & p1 = Positions[i + 1];
                    Vec3 & p2 = Positions[i + m_iSizeM1];
                    
                    Vec3 v0 = p1 - p0;
                    Vec3 v1 = p2 - p0;
                    
                    face_p[i].Cross( v1, v0 );
                    face_p[i].Normalize();
                    
                    m_Normals[i] = face_p[i];
                    
                    ++c; // wow, such c++, much similar
                    if( c >= diameter ) {
                        c = 0;
                        i += m_iSizeM1 - diameter;
                    }
                }
            }
            
            return;
        }
        
        // Full average vertex normals, expensive!
        if( m_bRebuildNormals == true )
        {
            m_bRebuildNormals = false;
            
            // Face normals.
            uint32_t    faces = ((m_iSizeM1 * 2) * m_iSizeM1) - m_iSizeM1;
            
            int32_t n;
            int32_t z;
            int32_t x;
            
            for( i = 0; i < faces - m_iSizeM1; i += 2 )
            {
                n = (i / 2);
                
                Vec3 p0 = Positions[n];
                Vec3 p1 = Positions[n + 1];
                Vec3 p2 = Positions[n + m_iSizeM1];
                Vec3 p3 = Positions[n + m_iSizeM1 + 1];
                
                Vec3 v0 = p1 - p0;
                Vec3 v1 = p2 - p0;
                Vec3 v2 = p2 - p1;
                Vec3 v3 = p3 - p1;
                
                face_p[i].Cross( v1, v0 );
                face_p[i + 1].Cross( v2, v3 );
            }
            
            i = 0;
            int face;
            
            int32_t total_faces;
            // Vertex (averaged) normals.
            for( z = 0; z < m_iSizeM1; ++z )
            {
                for( x = 0; x < m_iSizeM1; ++x )
                {
                    i = ((m_iSizeM1 * z) + x);
                    
                    Vec3 norm( 0.0f, 0.0f, 0.0f );
                    total_faces = 0;
                    
                    if( z > 0 ) {
                        face = (i * 2) - (m_iSizeM1 * 2);
                        if( x > 0 ) {
                            total_faces += 2;
                            norm = norm + face_p[face - 2];
                            norm = norm + face_p[face - 1];
                        }
                        
                        if( x < m_iSizeM1 - 1 ) {
                            total_faces += 2;
                            norm = norm + face_p[face];
                            norm = norm + face_p[face + 1];
                        }
                    }
                    
                    if( z < m_iSizeM1 ) {
                        face = (i * 2);
                        
                        if( x > 0 ) {
                            total_faces += 2;
                            norm = norm + face_p[face - 2];
                            norm = norm + face_p[face - 1];
                        }
                        
                        if( x < m_iSizeM1 - 1 ) {
                            total_faces += 2;
                            norm = norm + face_p[face];
                            norm = norm + face_p[face + 1];
                        }
                    }
                    
                    if( total_faces ) { // "Sanity" check.
                        
                        // Normalise result.
                        m_Normals[i] = norm / total_faces;
                        m_Normals[i].Normalize();
                    }
                }
            }
        }
    }
    
    //! Noise level for foliage noise.
    const static float OBJECTS_MIN = 0.35f;
    
    //! Noise level for folige (grass) noise.
    const static float FOLIAGES_MIN = 0.8f;

    static bool spawned = true;
    
    /**
     *  Spawn objects.
     */
    void LandscapeChunk::MakeObjects( const int32_t maxLod )
    {
//        if( spawned == false ) {
//            return;
//        }
        m_pMeshInstances[m_iMeshInstanceBaseCount] = new SInstancedObjectDrawInfo();
        
        m_pMeshInstances[m_iMeshInstanceBaseCount]->m_pMesh[0] = g_pbEnv->CreateObject( "tree_spring1.obj", Vec3(0.0f), Vec3(0.0f), 3.0 );
        m_pMeshInstances[m_iMeshInstanceBaseCount]->m_pMesh[1] = g_pbEnv->CreateObject( "tree_spring2.obj", Vec3(0.0f), Vec3(0.0f), 3.0 );
        m_pMeshInstances[m_iMeshInstanceBaseCount]->m_pMesh[2] = g_pbEnv->CreateObject( "tree_spring3.obj", Vec3(0.0f), Vec3(0.0f), 3.0 );
        
        // create empty buffer for the instance data
        uint32_t bufferSize = m_foliageObjectCount * sizeof(SInstancedObjectDrawInfo::InstanceData);
        
        glGenBuffers( 1, &m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceObjectData );
        glBindBuffer( GL_ARRAY_BUFFER, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceObjectData );
        glBufferData( GL_ARRAY_BUFFER, bufferSize, NEKO_NULL, GL_STATIC_DRAW );
        
        // fill the buffer with data
        SInstancedObjectDrawInfo::InstanceData * data = (SInstancedObjectDrawInfo::InstanceData *)glMapBufferRange( GL_ARRAY_BUFFER, 0, bufferSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
        
        uint dataIndex = 0;
        for( int i = 0; i <m_foliageObjectCount; i++ ) {
            data[dataIndex].position = Vec3(  nkMath::RandFloatAlt( -1000.0, 1512.0f ), 10,  nkMath::RandFloatAlt( -1000.0, 1512.0f ) );
            dataIndex++;
        }
        
        glUnmapBuffer( GL_ARRAY_BUFFER );
        
        
        // create the full instance buffer
        glGenBuffers( 3, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceBufferObjects );
        
        // it must have sufficient storage to store info about all instances
        uint32_t instanceBufferSize = m_foliageObjectCount * sizeof(SInstancedObjectDrawInfo::InstanceData) * 64;
        
        for( int32_t i = 0; i < 3; ++i ) {
            glBindBuffer( GL_ARRAY_BUFFER, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceBufferObjects[i] );
            glBufferData( GL_ARRAY_BUFFER, instanceBufferSize, NEKO_NULL, GL_DYNAMIC_COPY );
            
            glBindVertexArray( m_pMeshInstances[m_iMeshInstanceBaseCount]->m_pMesh[i]->m_hOwner->GetVertexArray() );
            // configure vertex array for drawing
            glBindBuffer( GL_ARRAY_BUFFER, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceBufferObjects[i] );
            glEnableVertexAttribArray( 3 );
            glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0 );
            glVertexAttribDivisor( 3, 1 );
            glBindVertexArray( 0 );
            
            // configure instance buffers for transform feedback
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, i, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceBufferObjects[i]);
        }
        
        // create vertex array for culling
        glGenVertexArrays( 1, &m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iCullVertexArray );
        glBindVertexArray( m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iCullVertexArray );
        glBindBuffer( GL_ARRAY_BUFFER, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iInstanceObjectData );
        glEnableVertexAttribArray( 0 );   // position
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0 );
        
        glBindVertexArray( 0 );
        
        // create query object to retrieve culled primitive count
        glGenQueries( 3, m_pMeshInstances[m_iMeshInstanceBaseCount]->m_iCullQuery );
        
        g_pbEnv->m_pWorldHandler->m_bUpdateGPUCulling = true;
        ++m_iMeshInstanceBaseCount;
        spawned = false;
//        // Add objects.
//        const int32_t highv = GetChunkSize();
//        const int32_t lowv = m_iSize * 2;
//        
//        uint32_t i;
//        /*const*/ int32_t randIndCol;
//        
//        Vec3 randVertPos = Vec3( 0.0f );
//        const Vec3 * BufferPosition = m_Positions;
//        
//        // Chunk & node.
//        CQuadtreeNode * node;
//        CNodeChunk * chunk;
//        
//        // Vegetation ID.
//        uint32_t randomVegetable;
//        
//        // Foliage model name.
//        const char * treeName;
//        
//        float veg, veg2;
//        
//        ncNoisePerlin * noise = (ncNoisePerlin*)g_pbEnv->chunkHandler.GetFoliageNoise();
//        
//        //! Using 'chunk-by-chunk' method.
//        //! Instead of rendering big amount of meshes we sort it by chunks ( which are going to be occlusion'd ) and every mesh has it's parent - a chunk! woot
//        //! i.e. we render grass by pieces, 150-250 pieces per chunk.
//        for( i = 0; i < m_foliageObjectCount; ++i )
//        {
//            randIndCol = nkMath::LargeRandom() % (highv - lowv + 1) + lowv;
//            randVertPos = Vec3( m_Positions[randIndCol].x, BufferPosition[randIndCol].y, m_Positions[randIndCol].z );
//         
//            printf( "object at %4.2f %4.2f %4.2f\n", randVertPos.x, randVertPos.y, randVertPos.z );
//            
//            //! Pick a point.
//            veg  = (noise->Get(randVertPos.x / 1500.1f, randVertPos.z / 1500.1f) + 1.0f) * 0.5f;
//            veg2  = (noise->Get(randVertPos.x / 150.1f, randVertPos.z / 150.1f) + 1.0f) * 0.2f;
//            
//            //            if( randVertPos.y < 150.0f  ) {
//            //                if ( veg > OBJECTS_MIN ) {
//            //! Add lower level objects.
//            //
//            //                    //! Pick good enough position.
//            //                    if( randVertPos.y <= g_pbEnv->GetMinWaterLevel() ) {
//            //                        --i;
//            //                        continue;
//            //                    }
//            //
////            //! Find the node.
////            node = m_tQuadtree->FindNode( Vec2( randVertPos.x, randVertPos.z ) );
////            if( node == NEKO_NULL ) {
////                --i;
////                continue;
////            }
////            
////            //! Find node chunk.
////            chunk = node->GetChunk();
////            if( chunk->CanBeUsed() == false ) {
////                --i;
////                continue;
////            }
//////            
////            data[dataIndex].position = Vec3( randVertPos );
////            //                    data[dataIndex].scale = Vec3(32.0f);
////            dataIndex++;
//            //                    //! Add foliages by biomes.
//            //                    switch( chunk->GetBiomeType() ) {
//            //                            /**     Forest biome.   **/
//            //                        case FOREST:
//            //                        {
//            //                            //chunk->SetBiomeAmbient( node->m_BBox.GetCenter(), 20.0f, 1900.0f, 5.0, "eveningbirds" );
//            //                            randomVegetable = rand() % 1;
//            //                            treeName = NekoCString::STR( "firtree0%i.obj", randomVegetable + 1 );
//            //
//            //                            chunk->AddObject( new ncLTerrainObject( treeName,
//            //                                                                   Vec4(randVertPos.x, randVertPos.y - 1.0f, randVertPos.z, (float)nkMath::Random(360) ), 16.7f
//            //                                                                   ) );
//            //                            break;
//            //                        }
//            //
//            //                            /**     Snowy biome.    **/
//            //                        case SNOWY:
//            //                        {
//            //                            //chunk->SetBiomeAmbient( node->m_BBox.GetCenter(), 20.0f, 1900.0f, 5.0, "eveningbirds" );
//            //                            randomVegetable = rand() % 1;
//            //                            treeName = NekoCString::STR( "shrub_spring%i.obj", randomVegetable + 1 );
//            //
//            //                            chunk->AddObject( new ncLTerrainObject( treeName,
//            //                                                                   Vec4(randVertPos.x, randVertPos.y - 1.0f, randVertPos.z, (float)nkMath::Random(360) ), 0.2f
//            //                                                                   ) );
//            //                        }
//            //                            break;
//            //
//            //                            /**     Sandy biome.    **/
//            //                        case SANDY:
//            //                        {
//            //                            //chunk->SetBiomeAmbient( node->m_BBox.GetCenter(), 20.0f, 1900.0f, 5.0, "eveningbirds" );
//            //                            randomVegetable = rand() % 3;
//            //                            treeName = NekoCString::STR( "tree_spring%i.obj", randomVegetable + 1 );
//            //
//            //                            chunk->AddObject( new ncLTerrainObject( treeName,
//            //                                                                   Vec4(randVertPos.x, randVertPos.y - 1.0f, randVertPos.z, (float)nkMath::Random(360) ), 1.2f
//            //                                                                   ) );
//            //                        }
//            //                            break;
//            //                    }
//            //                }
//            //                else
//            //                {
//            //                    // SOMETHING
//            //
//            //
//            //                    //! Pick good enough position. ( BEACH )
//            //                    if( randVertPos.y <= 5.0f || randVertPos.y >= 12.0f ) {
//            //                        --i;
//            //                        continue;
//            //                    }
//            //
//            //                    //! Find the node.
//            //                    node = m_tQuadtree->FindNode( Vec2( randVertPos.x, randVertPos.z ) );
//            //                    if( node == NEKO_NULL ) {
//            //                        --i;
//            //                        continue;
//            //                    }
//            //
//            //                    //! Find node chunk.
//            //                    chunk = node->GetChunk();
//            //                    if( chunk->CanBeUsed() == false ) {
//            //                        --i;
//            //                        continue;
//            //                    }
//            //
//            //                    data[dataIndex].position = Vec3( randVertPos );
//            ////                    data[dataIndex].scale = Vec3(32.0f);
//            //                    dataIndex++;
//            ////                    chunk->AddObject( new ncLTerrainObject( "shrub_spring%i.obj",
//            ////                                                           Vec4(randVertPos.x, randVertPos.y - 1.0f, randVertPos.z, (float)nkMath::Random(360) ), 2.125f
//            ////                                                           ) );
//            //                }
//            //            }
//        }
//        
//        //        randIndCol = 0;
//        //        i = 0;
//        //
//        //        //! Add foliages.
//        //        for( i = 0; i < 49152	; ++i )
//        //        {
//        //            randIndCol = nkMath::LargeRandom() % (highv - lowv + 1) + lowv;
//        //
//        //            randVertPos = Vec3( BufferPosition[randIndCol].x - 1.0f, BufferPosition[randIndCol].y, BufferPosition[randIndCol].z - 1.0f );
//        //
//        //            //! Foliage point.
//        //            veg  = (noise->Get(randVertPos.x / 1500.1f, randVertPos.z / 1500.1f) + 1.0f) * 0.5f;
//        //
//        //            if( randVertPos.y < 150.0f  )
//        //            {
//        //                if ( veg < FOLIAGES_MIN )
//        //                {
//        //                    //! Randomize a bit.
//        //                    randVertPos.x += nkMath::RandFloatAlt( -4.0f, 2.0f );
//        //                    randVertPos.z += nkMath::RandFloatAlt( -4.0f, 2.0f );
//        //
//        //                    //! Check for a water level.
//        //                    if( randVertPos.y < g_pbEnv->GetMinWaterLevel() - 7.0 )
//        //                    {
//        //                        --i;
//        //                        continue;
//        //                    }
//        //
//        //                    //! Find the node.
//        //                    node = m_tQuadtree->FindNode( Vec2( randVertPos.x, randVertPos.z ) );
//        //                    if( node == NEKO_NULL )
//        //                    {
//        //                        --i;
//        //                        continue;
//        //                    }
//        //
//        //                    //! Node chunk.
//        //                    chunk = node->GetChunk();
//        //                    if( chunk == NEKO_NULL )
//        //                    {
//        //                        --i;
//        //                        continue;
//        //                    }
//        //
//        //                    //! Add a foliage.
//        //                    chunk->AddFoliageLayer( randVertPos.x, randVertPos.y + 0.21f/* + 1.2f*/, randVertPos.z );
//        //                }
//        //            }
//        //        }
//        
      }
    
    //    const static float HEIGHT_DESCALE = 16.0f;
    
    /**
     *  Smoothen the landscape vertex edges.
     */
    void LandscapeChunk::Modify( bool smoothAll )
    {
        m_verticesToEdit.Destroy();
    }
    
    /**
     *  Refresh terrain. Some settings refresh rarely, so we won't put them in the loop.
     */
    void LandscapeChunk::Refresh()
    {
        
    }
    
    /**
     *  Create buffers and misc. stuff.
     */
    void LandscapeChunk::CreateBuffers( const int32_t maxLod )
    {
        if( bRecreateBuffers == false ) {
            return;
        }
        
        // Quadtree system creates some buffers and I don't want them to show up.
        g_mainRenderer->AllowBufferLogging(false);
        
//        if( m_bThreaded == false )
//        {
        CreateQuadtree( 64 );
        FinishQuadtree( maxLod );
//        }
        
        CreateVertexObjects();
        
        bRecreateBuffers = false;
        
        g_mainRenderer->AllowBufferLogging(true);
    }
    
    /**
     *  Remove terrain.
     */
    void LandscapeChunk::Destroy()
    {
        //        delete m_pBoundingSphere;
//        if( m_pPathNodes != NEKO_NULL ) {
//            for( int32_t i(m_iNodePaths); i > 0; --i ) {
//                if( m_pPathNodes[i] != NEKO_NULL ) {
//                    pAllocator->Dealloc( m_pPathNodes[i] );
//                    m_pPathNodes[i] = NEKO_NULL;
//                }
//            }
//        }
//        
//        if( m_pPathNodes != NEKO_NULL ) {
//            pAllocator->Dealloc( m_pPathNodes );
//            m_pPathNodes = NEKO_NULL;
//        }

        // Delete a quadtree.
        if( m_tQuadtree != NEKO_NULL ) {
            m_tQuadtree->Destroy();
            
            pAllocator->Dealloc( m_tQuadtree );
            m_tQuadtree = NEKO_NULL;
        }
        
        // Check if mesh was created.
        if( m_pMeshData != NEKO_NULL ) {
            m_pMeshData->Destroy();
            pAllocator->Dealloc( m_pMeshData ) ;
            m_pMeshData = NEKO_NULL;
        }
        
        NekoAllocator::deleteStackAllocator( (CStackAllocator *)pAllocator, pAllocatorHandle );
    }
    
    /**
     *  Render terrain objects.
     */
    void LandscapeChunk::RenderObjects( CRenderer::ESceneEye eye, int16_t flags )
    {
        if( m_tQuadtree == NEKO_NULL ) {
            return;
        }
        
        if( Use_UniformBuffers->Get<bool>() ) {
            glBindBuffer( GL_UNIFORM_BUFFER, f_AssetBase->p_MeshBase->m_iMeshUniformBuffer );
        }
        
        m_iObjectsDrawn = m_tQuadtree->DrawObjects( flags );
        
        if( Use_UniformBuffers->Get<bool>() ) {
            glBindBuffer( GL_UNIFORM_BUFFER, 0 );
        }
    }
    
    /**
     *  Render foliage.
     */
    void LandscapeChunk::DrawFoliage()
    {
        if( !bActive ) {
            return;
        }
        
        if( m_tQuadtree == NEKO_NULL ) {
            return;
        }
        
        m_tQuadtree->DrawFoliage();
    }

    /**
     *  Render path nodes ( roads, ways, etc.. ).
     */
    void LandscapeChunk::RenderPathNodes()
    {
        // Check if we have enough road nodes created.
        if( m_iNodePaths == 0 ) {
            return;
        }
        
        if( m_pPathNodes == NEKO_NULL ) {
            return;
        }
        
        uint32_t    i;
        
        SPathInfo   * pathInfo;
        ncMatrix4   modelMatrix;
        ncMatrix4   viewMatrix;
        
        // Use road mesh shader.
        m_pRoadShader->Use();
        
        viewMatrix = g_Core->p_Camera->ViewProjectionMatrix * modelMatrix;
        
        m_pRoadShader->SetUniform( m_iRoadLoc[LOC_MODELVIEW], 1, GL_FALSE, viewMatrix.m );
        m_pRoadShader->SetUniform( m_iRoadLoc[LOC_MODELMATRIX], 1, GL_FALSE, modelMatrix.m );

        // Render all node path meshes.
        for( i = 0; i < m_iNodePaths; ++i )
        {
            pathInfo = m_pPathNodes[i];
            
            if( pathInfo->m_iNodeFlags == 0 ) {
                continue;
            }
            
            // Bind textures.
            g_mainRenderer->BindTexture( 0, pathInfo->m_pMaterial->m_pDiffuse->GetId() );
            g_mainRenderer->BindTexture( 1, pathInfo->m_pMaterial->m_pNormal->GetId() );
            g_mainRenderer->BindTexture( 2, pathInfo->m_pMaterial->m_pSpecular->GetId() );
            
            // Draw the path node mesh.
            pathInfo->m_pNodeMesh->DrawIndexed();
        }
        
        g_mainRenderer->BindTexture( 0, 0 );
        
        m_pRoadShader->Next();
        
    }
    
    /**
     *  Render entities.
     */
    void LandscapeChunk::RenderEntities( CRenderer::ESceneEye eye, int16_t flags )
    {
        //**  Draw objects & foliage now. **
#   if !defined( NEKO_EDITOR )
        if( !(flags & (int32_t)EWorldFlag::Reflection) )
        {
            RenderObjects( eye, flags );
//            if( !(flags & (int32_t)EWorldFlag::Shadow) )
//            {
//                //! Don't render grass shadow yet. TODO!
//                DrawFoliage();
//            }
        }
#   endif
        
    }
    
    /**
     *  Render terrain.
     */
    void LandscapeChunk::UpdateFrame( SGLShader * cullShader )
    {

        
        
    }
    
    /**
     *  Render terrain.
     */
    void LandscapeChunk::Render( CRenderer::ESceneEye eye, int32_t flags )
    {
        if( !bActive ) {
            return;
        }
        
        if( m_tQuadtree == NEKO_NULL ) {
            return;
        }
        
        m_pMeshData->BindVertexArray();
        
        //        m_pChunkShader->Use();
        
        // Draw the ground.
        m_iFacesDrawn = m_tQuadtree->Draw( flags );
        
        //        m_pChunkShader->Next();
        
        g_mainRenderer->UnbindVertexArray();
    }
    
    /**
     *  Get random position on terrain.
     */
    const Vec3 &LandscapeChunk::GetRandomPosition()
    {
        const uint32_t randIndCol = nkMath::LargeRandom() % (m_iSize - m_iSize + 1) + m_iSize;
        return m_Positions[randIndCol];
    }
    
    
}

