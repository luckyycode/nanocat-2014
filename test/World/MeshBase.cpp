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
//  MeshObject.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 10/15/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "MeshBase.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Graphics/GraphicsManager.h"

namespace Neko {

    /**
     *  Render a mesh.
     */
    void CBasicMesh::DrawIndexed()
    {
#   if defined( USES_OPENGL )
        glBindVertexArray( m_VAO );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vIndexArray.Handle );
        glDrawElements( m_iPrimitiveType, m_iFaceCount, GL_UNSIGNED_INT, NEKO_NULL );
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        
        glBindVertexArray( 0 );
#   else
        
#   endif
        
    }
    
    /**
     *  Render a mesh.
     */
    void CBasicMesh::DrawArrays()
    {
#   if defined( USES_OPENGL )
        glBindVertexArray( m_VAO );
        
        glBindBuffer( GL_ARRAY_BUFFER, m_vVertex.Handle );
        glDrawArrays( m_iPrimitiveType, 0, m_iVertexCount );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
        glBindVertexArray( 0 );
#   else
        
#   endif
        
    }
    
    /**
     *  Set mesh owner.
     */
    void ncMesh::SetOwner( CMesh * owner )
    {
        m_hOwner = owner;
    }
    
    /**
     *  Create basic mesh on the GPU.
     */
    void CBasicMesh::Create( const Vec3 *positions, const Vec3 *normals, const Vec2 *uvs, const uint32_t *indices, const int32_t vertexCount, const int32_t indexCount, EPrimitiveType type )
    {
        uint32_t    attributeId = 0;
        
        // Set the drawing primitive mode.
        m_iPrimitiveType = g_pGraphicsManager->GetCurrentInterface()->GetAPIPrimitiveType( type );
        
        // Set the size.
        m_iFaceCount = indexCount;
        m_iVertexCount = vertexCount;
        
        // Create vertex array.
        m_VAO = g_mainRenderer->CreateVertexArray();
        
        // Vertex buffer.
        m_vVertex = g_mainRenderer->AllocGPUBuffer( vertexCount * sizeof(Vec3), EBufferStorageType::Array, EBufferType::Static );
        g_mainRenderer->BufferData( &m_vVertex, &positions[0], 0, vertexCount * sizeof(Vec3) );
         g_mainRenderer->BufferPointer( &m_vVertex, 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        
        // Normal buffer.
        if( normals != NEKO_NULL ) {
            ++attributeId;
            
            m_vNormals = g_mainRenderer->AllocGPUBuffer( vertexCount * sizeof(Vec3), EBufferStorageType::Array, EBufferType::Static );
            g_mainRenderer->BufferData( &m_vNormals, &normals[0], 0, vertexCount * sizeof(Vec3) );
            g_mainRenderer->BufferPointer( &m_vNormals, attributeId, 3, GL_FLOAT, GL_FALSE, 0, 0 );
            g_mainRenderer->FinishBuffer( &m_vNormals, 0, vertexCount * sizeof(Vec3) );
        }
        
        // UV buffer.
        if( uvs != NEKO_NULL ) {
            ++attributeId;
            
            m_vUVs = g_mainRenderer->AllocGPUBuffer( vertexCount * sizeof(Vec2), EBufferStorageType::Array, EBufferType::Static );
            g_mainRenderer->BufferData( &m_vUVs, &uvs[0], 0, vertexCount * sizeof(Vec2) );
            g_mainRenderer->BufferPointer( &m_vUVs, attributeId, 2, GL_FLOAT, GL_FALSE, 0, 0 );
            g_mainRenderer->FinishBuffer( &m_vUVs, 0, vertexCount * sizeof(Vec2) );
        }
        
        // Index array.
        if( indices != NEKO_NULL ) {
            m_vIndexArray = g_mainRenderer->AllocGPUBuffer( indexCount * sizeof(uint32_t), EBufferStorageType::IndexArray, EBufferType::Static );
            g_mainRenderer->BufferData( &m_vIndexArray, &indices[0], 0, indexCount * sizeof(uint32_t) );
            g_mainRenderer->FinishBuffer( &m_vIndexArray, 0, indexCount * sizeof(uint32_t) );
        }
        
        // Sanity check.
        g_mainRenderer->FinishBuffer( &m_vVertex, 0, vertexCount * sizeof(Vec3) );
        g_mainRenderer->UnbindVertexArray();
    }
    
    /**
     *  Destroy a mesh.
     */
    void CBasicMesh::Destroy()
    {
        // Means that every buffer was created.
        if( m_vVertex.Handle != 0 ) {
            g_mainRenderer->DeleteGPUBuffer( &m_vVertex );
        }
        
        if( m_vNormals.Handle != 0 ) {
            g_mainRenderer->DeleteGPUBuffer( &m_vNormals );
        }
        
        if( m_vUVs.Handle != 0 ) {
            g_mainRenderer->DeleteGPUBuffer( &m_vUVs );
        }
        
        if( m_vIndexArray.Handle != 0 ) {
            g_mainRenderer->DeleteGPUBuffer( &m_vIndexArray );
        }
        
#   if defined( USES_OPENGL )
        glDeleteVertexArrays( 1, &m_VAO );
#   endif
    }
}