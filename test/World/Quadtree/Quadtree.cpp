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
//  Quadtree.cpp
//  Quadtree system for scenes. :OO
//
//  Created by Neko Vision on 1/8/14.
//  Copyright (c) 2013-2014 Neko Vision. All rights reserved.
//

// CPU culled.

#include "Quadtree.h"

#include "../../Platform/Shared/SystemShared.h"
#include "../../Core/Core.h"
#include "../../Core/Player/Camera/Camera.h"                 // Frustum.

#include "../BeautifulEnvironment.h"   // To get landscapes.

namespace Neko {

    /**
     *  Find node by a position.
     */
    CQuadtreeNode * CQuadtree::FindNode( const Vec2 & pos )
    {
        if( nodeRoot == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        CQuadtreeNode * node = nodeRoot;
        CQuadtreeNode * child;
        
        uint32_t i = 0;
        while( !node->isChild() ) {
            for( i = 0; i < 4; ++i ) {
                child = &node->m_nodeChildren[i];
                if( child->m_boundingBox.ContainsPoint( Vec3( pos.x, child->m_boundingBox.GetCenter().y, pos.y ) ) ) {
                    node = child;
                    break;
                }
            }
            
            if( i >= 4 ) {
                return NEKO_NULL;
            }
        }
        
        return node;
    }
    
    /**
     *  Create new quadtree.
     */
    void CQuadtree::Create( EQuadtreeType type, SBoundingBox *pBBox, const Vec2i & mSize, uint32_t minmSize, const int16_t flags, INekoAllocator * allocator )
    {
        pAllocator= allocator;
        
        // Create root node.
        nodeRoot = (CQuadtreeNode *)pAllocator->Alloc( sizeof(CQuadtreeNode) );
        if( nodeRoot == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "CreateQuadtree(): failed to allocate its root.\n" );
            return;
        }
        
		// I don't want spammish console!
		// But I want detailed stuffy.
		g_mainRenderer->AllowBufferLogging(false);

        nodeRoot->m_boundingBox = *pBBox;
        nodeRoot->m_qType = type;
        
        // Recursively create root nodes.
        nodeRoot->Create( 0, Vec2i( 0, 0 ), mSize, minmSize, flags, pAllocator );
        g_Core->p_Console->Print( LOG_INFO, "CreateQuadtree(): A new quadtree with size %ix%i and %i chunks was created.\n", mSize.x, mSize.y, minmSize );

		g_mainRenderer->AllowBufferLogging(true);
    }
    
    /**
     *  Draw a terrain quadtree with nodes.
     */
    int32_t CQuadtree::Draw( int32_t options )
    {
        if( nodeRoot == NEKO_NULL ) {
            return 0;
        }

        return nodeRoot->Draw( &g_Core->p_Camera->m_CommonFrustum, options );
    }
    
    /**
     *  Draw foliage objects.
     */
    int32_t CQuadtree::DrawFoliage()
    {
        //glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
        
        g_pbEnv->BeginGrassRender();
        
        int32_t drawn = nodeRoot->DrawFoliage();
        
        g_pbEnv->EndGrassRender();
        
        // And disable Sample alpha to coverage.
        //glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        
        return drawn;
    }
    
    /**
     *  Draw node objects.
     */
    int32_t CQuadtree::DrawObjects( int16_t flags )
    {
#   if defined( USES_OPENGL )
        int32_t drawn = nodeRoot->DrawObjects( flags );
        
        return drawn;
#   else
        return 0;
#   endif
    }
    
    /**
     *  Delete chunk indices.
     */
    void CQuadtree::DeleteIndices()
    {
        nodeRoot->DeleteIndices();
    }
    
    /**
     *  Calculate bounding box from vertices.
     */
    void CQuadtree::CalculateBoundingBoxAndCollisionData( const Vec3* vertices, bool needsCollision, bool needsBoundingBox )
    {
        nodeRoot->CalculateBoundingBoxAndCollisionData( vertices, needsCollision, needsBoundingBox );
    }
    
    /**
     *   Destroy the whole quadtree with its nodes.
     */
    void CQuadtree::Destroy()
    {
        if( nodeRoot != NEKO_NULL ) {
            nodeRoot->Destroy();
            
            pAllocator->Dealloc( nodeRoot );
            
            nodeRoot = NEKO_NULL;
        }
    }
}