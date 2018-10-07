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
//  LTerrainObject.cpp
//  Chunk-system terrain object.
//
//  Created by Neko Code on 1/4/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#include "LTerrainObject.h"
#include "BeautifulEnvironment.h"
#include "../AssetCommon/AssetBase.h"

namespace Neko {
    /**
     *  Default values, used to initialize object array.
     */
    ncLTerrainObject::ncLTerrainObject() : m_tMeshHolder( NEKO_NULL )
    {
   
    }
    
    /**
     *  Remove object.
     */
    ncLTerrainObject::~ncLTerrainObject()
    {
        Delete();
    }
    
    /**
     *  Create new object.
     */
    ncLTerrainObject::ncLTerrainObject( const char * meshName, const Vec4 & meshWorld, INekoAllocator * allocator, const float & scale )
    {
        m_vPosition = Vec3( meshWorld.x, meshWorld.y, meshWorld.z );
        m_fAngle = meshWorld.w;
        
        pAllocator = allocator;
        
        // Create a mesh alias with its handle.
        m_tMeshHolder = (ncMesh *)pAllocator->Alloc( sizeof( ncMesh )  );
        
        m_tMeshHolder->SetObjectPos( m_vPosition );
        m_tMeshHolder->SetRotation( Vec3( 1.0f  * m_fAngle, 1.0f, 1.0f ) );
        
        // Default scale.
        m_tMeshHolder->SetScale( scale );
        
        // Set the mesh.
        m_tMeshHolder->SetOwner( f_AssetBase->p_MeshBase->Find( meshName ) );
        
        // Setup modelview matrix and another stuff.
        m_tMeshHolder->Refresh();
        
        
        m_tMeshHolder->m_pbSphere = *m_tMeshHolder->GetHandle()->GetBoundingSphere();
        
        m_tMeshHolder->m_pbSphere.SetScale( scale );
        m_tMeshHolder->m_pbSphere.SetCenter( m_vPosition );
        
        
        // Set the object flag.
        m_tMeshHolder->SetObjectFlag( CPhysicsObject::PHYS_NONMOVABLE );
        m_tMeshHolder->SetRadius( m_vPosition.Length() - 8.0f );
        //            m_tMeshHolder->SetObjectPhysFlag( CPhysicsObject::PHYS_APPLYGRAVITY );
        
        // Set a default billboard for the object.
        m_pMaterial = f_AssetBase->p_MaterialBase->Find( "tree01_billboard" );
    }
    
    /**
     *  Delete object.
     */
    void ncLTerrainObject::Delete()
    {
        pAllocator->Dealloc( m_tMeshHolder );
    }
    
    /**
     *  Draw terrain object.
     */
    //    float angle = 0.0f;
    void ncLTerrainObject::Draw( uint32_t id, float distance, bool isShadow )
    {
        // Don't render empty model.
        if( m_tMeshHolder == NEKO_NULL ) {
            return;
        }
        
        if(  m_tMeshHolder->GetHandle() == NEKO_NULL ) {
            return;
        }
        
//        // TODO - Tree billboarding.
//        // DONE!!
//        if( distance > 256.0f ) {
//            // Check for a visibility..
//            if( !g_Core->p_Camera->m_CommonFrustum.ContainsSphere( m_vPosition, 32.0f ) ) {
//                
//                return;
//            }
//            
//            CBeautifulEnvironment::SImposterInfo info;
//            info.lightFlag = 3.0f;
//            info.origin = m_vPosition + Vec3( 0.0f, 25.0f, 0.0f );
//            info.size = 32.0f;
//            info.sampler = m_pMaterial;
//            info.lightFlag = 1.0f;  // Include lighting.
//            
//            g_pbEnv->DrawBillboard( &info );
//        } else {    // Render "3d" ( near ) object.
//            // Check for a visibility..
//            if( !g_Core->p_Camera->m_CommonFrustum.ContainsSphere( m_vPosition, 64.0f ) ) {
//                // Check if we're in its bounding box, then don't hide.
//                
//                // temp.
//                Vec3 min ( -1.0, -1.0, -1.0 ), max ( 1.0, 1.0, 1.0 );
//                if( !SBoundingBox::ContainsBoxes( m_tMeshHolder->m_bBox.min, m_tMeshHolder->m_bBox.max, min + g_Core->p_Camera->vEye, max + g_Core->p_Camera->vEye ) ) {
//                    // Close to the object.
//                    
//                    return;
//                }
//            }
//            
            m_tMeshHolder->Render( isShadow );
//        }
    }
}
