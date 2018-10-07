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
//  Foliage.cpp
//  Neko
//
//  Created by Neko Code on 5/1/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#include "Foliage.h"
#include "BeautifulEnvironment.h"

// This is a CPU implementation of foliage rendering.

#if !defined( NEKO_SERVER )

// GRASS. This Grass class can be also flowers and any other small foliage.
// ncGrass class is single grass instance which has data for position, rotation and type.
// Grass uses quadtree and renders in it.
namespace Neko {
    
    /**
     *  New grass piece.
     */
    ncCGrass::ncCGrass( const float& mPositionX,
                       const float& mPositionY,
                       const float& mPositionZ )
    {
        Set( mPositionX, mPositionY, mPositionZ );
    }
    
    ncCGrass::ncCGrass()
    {
        // Yeah!!
    }
    
    /**
     *  Set foliage properties.
     */
    void ncCGrass::Set( const float &mPositionX, const float &mPositionY, const float &mPositionZ )
    {
        // Just store the passed position.
        vPos.x = mPositionX;
        vPos.y = mPositionY;
        vPos.z = mPositionZ;
        
        // Give it random rotation.
        // It's applied in vertex shader, because I don't want
        // to pass matrix for every grass piece, so
        float rad_angle = float(rand() % 180) * nkMath::ANG2RAD;
        
        fRotation = rad_angle;
    }
    
    /**
     *  Set grass type.
     */
//    void ncCGrass::SetType( ncCGrassTypes type ) {
//        m_type = type;
//    }
    
    /**
     *  Render grass piece.
     */
    void ncCGrass::Render()
    {
        // Compute grass cell distance to camera.
        float mDist[3];
        
        mDist[0] = g_Core->p_Camera->vEye.x - vPos.x;
        if(mDist[0] < 0.0f) mDist[0] = -mDist[0];
        
        mDist[1] = g_Core->p_Camera->vEye.y - vPos.y;
        if(mDist[1] < 0.0f) mDist[1] = -mDist[1];
        
        mDist[2] = g_Core->p_Camera->vEye.z - vPos.z;
        if(mDist[2] < 0.0f) mDist[2] = -mDist[2];
        
        float m_fGrassLod2 = g_pbEnv->GetGrassLod2();
        float m_fGrassLod1 = g_pbEnv->GetGrassLod1();
        
        // Now based upon distance in X and Z axes decide which LOD to use.
        if(mDist[0] < m_fGrassLod2 && mDist[1] < m_fGrassLod2 && mDist[2] < m_fGrassLod2 ) {
            //  Render high-detail mesh here.
            RenderCell();
        } else if(mDist[0] < m_fGrassLod1 && mDist[1] < m_fGrassLod1 && mDist[2] < m_fGrassLod1 ) {
            //  Here we render a mid-detail mesh.
            RenderCell();
        }
    }
    
    /**
     *  Render grass cell.
     */
    void ncCGrass::RenderCell()
    {
        glDrawElements( GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, NEKO_NULL );
    }
    
    /**
     *  Render low poly cell.
     */
    void ncCGrass::RenderLowLodCell()
    {
        glDrawElements( GL_TRIANGLE_STRIP, 2, GL_UNSIGNED_INT, NEKO_NULL );
    }
}

#endif