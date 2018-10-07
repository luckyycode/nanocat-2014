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
//  Frustum.cpp
//  Camera frustum.
//
//  Created by Neko Code on 1/28/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#include "Frustum.h"
#include "../Core/Player/Camera/Camera.h"
#include "../Core/Utilities/Utils.h"

using namespace nkMath;

namespace Neko {
    
    /**
     *  Check if frustum contains point.
     */
    bool CFrustum::ContainsPoint( const Vec3 &point ) const
    {
        uint32_t p;
        for( p = 0; p < 6; ++p ) {
            if( m_vPlane[p].x * point.x + m_vPlane[p].y *
               point.y + m_vPlane[p].z * point.z + m_vPlane[p].w <= 0 ) {
                return false;
            }
        }
        
        return true;
    }
    /**
     *  A bit like 'ContainsPoint'.
     */
    bool CFrustum::ContainsObject( const Vec3& point, const float radius ) const
    {
        float distance = 0.0f;
        uint32_t i;
        
        for( i = 0; i < 6; ++i ) {
            distance = m_vPlane[i].x * point.x +
            m_vPlane[i].y * point.y +
            m_vPlane[i].z * point.z +
            m_vPlane[i].w;
            
            if( distance < -radius ) {
                return false;
            }
        }
        
        return true;
    }
    
    /**
     *  Check if frustum contains sphere.
     */
    const int32_t CFrustum::ContainsSphere( const Vec3& center, const float radius ) const
    {
        uint32_t p;
        uint32_t c = 0;
        float d;
        
        for( p = 0; p < 6; ++p ) {
            d = m_vPlane[p].x * center.x + m_vPlane[p].y * center.y + m_vPlane[p].z * center.z + m_vPlane[p].w;
            
            if( d <= -radius ) {
                return FRUSTUM_OUT;
            }
            
            if( d > radius ) {
                ++c;
            }
        }
        
        return (c == 6) ? FRUSTUM_INTERSECT : FRUSTUM_IN;
    }
    
    /**l
     *  Check if frustum contains bounding box.
     */
    const int32_t CFrustum::ContainsBoundingBox( const SBoundingBox& bbox ) const
    {
        Vec3 tCorners[8] =
        {
            Vec3( bbox.min.x, bbox.min.y, bbox.min.z ),
            Vec3( bbox.max.x, bbox.min.y, bbox.min.z ),
            Vec3( bbox.min.x, bbox.max.y, bbox.min.z ),
            Vec3( bbox.min.x, bbox.min.y, bbox.max.z ),
            Vec3( bbox.max.x, bbox.max.y, bbox.min.z ),
            Vec3( bbox.min.x, bbox.max.y, bbox.max.z ),
            Vec3( bbox.max.x, bbox.min.y, bbox.max.z ),
            Vec3( bbox.max.x, bbox.max.y, bbox.max.z )
        };
        
        uint32_t i = 0;
        
        for( uint32_t p = 0; p < 6; ++p ) {
            uint32_t a = 8;
            uint32_t in = 1;
            
            for( uint32_t c = 0; c < 8; ++c ) {
                float side = m_vPlane[p].x * tCorners[c].x +
                m_vPlane[p].y * tCorners[c].y +
                m_vPlane[p].z * tCorners[c].z +
                m_vPlane[p].w;
                
                if( side < 0 )
                {
                    in = 0;
                    --a;
                }
            }
            
            if( a == 0 ) {
                return FRUSTUM_OUT;
            }
            
            i += in;
        }
        
        if( i == 6 ) {
            return FRUSTUM_IN;
        }
        
        return FRUSTUM_INTERSECT;
        
    }
    
    /**
     *  Check if point in frustum plane.
     */
    const int32_t CFrustum::ClassifyPoint( const Vec4 &normal, const Vec3 &point )
    {
        float distance = point.x * normal.x + point.y * normal.y + point.z * normal.z + normal.w;
        
        if( distance > EPSILON ) {
            return POINT_IN_FRONT_OF_PLANE;
        }
        
        if( distance <- EPSILON ) {
            return POINT_BEHIND_PLANE;
        }
        
        return POINT_ON_PLANE;
    }
    
    /**
     *  Check if point inside of vertices.
     */
    bool CFrustum::IsBoxInside( const Vec3 *points ) const
    {
        
        uint32_t i;
        for( i = 0; i < 6; ++i )
        {
            // Check all bounding box bounding points.
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[0] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[1] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[2] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[3] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[4] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[5] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[6] ) != POINT_BEHIND_PLANE )
                continue;
            if( CFrustum::ClassifyPoint( m_vPlane[i], points[7] ) != POINT_BEHIND_PLANE )
                continue;
            
            // None inside.
            return false;
        }
        
        return true;
    }
    
    /**
     *  Update frustum.
     */
    void CFrustum::Extract( const Vec3& eye )
    {
        // TODO: for loop
        
        // Calculate clip matrix.
        // Don't use premultiplied matrix here due shadow frustum artifacts ( blinking meshes ).
        m_ModelProjection = g_Core->p_Camera->ProjectionMatrix * g_Core->p_Camera->ViewMatrix;
        
        float t;
        
        // Right plane.
        m_vPlane[0].x = m_ModelProjection.m[ 3] - m_ModelProjection.m[ 0];
        m_vPlane[0].y = m_ModelProjection.m[ 7] - m_ModelProjection.m[ 4];
        m_vPlane[0].z = m_ModelProjection.m[11] - m_ModelProjection.m[ 8];
        m_vPlane[0].w = m_ModelProjection.m[15] - m_ModelProjection.m[12];
        
        // Normalize.
        t = nkMath::FastSqrty( m_vPlane[0].x * m_vPlane[0].x + m_vPlane[0].y * m_vPlane[0].y + m_vPlane[0].z * m_vPlane[0].z );
        m_vPlane[0].x /= t;
        m_vPlane[0].y /= t;
        m_vPlane[0].z /= t;
        m_vPlane[0].w /= t;
        
        // Left plane.
        m_vPlane[1].x = m_ModelProjection.m[ 3] + m_ModelProjection.m[ 0];
        m_vPlane[1].y = m_ModelProjection.m[ 7] + m_ModelProjection.m[ 4];
        m_vPlane[1].z = m_ModelProjection.m[11] + m_ModelProjection.m[ 8];
        m_vPlane[1].w = m_ModelProjection.m[15] + m_ModelProjection.m[12];
        
        // Normalize.
        t = nkMath::FastSqrty( m_vPlane[1].x * m_vPlane[1].x + m_vPlane[1].y * m_vPlane[1].y + m_vPlane[1].z * m_vPlane[1].z );
        m_vPlane[1].x /= t;
        m_vPlane[1].y /= t;
        m_vPlane[1].z /= t;
        m_vPlane[1].w /= t;
        
        // Bottom plane.
        m_vPlane[2].x = m_ModelProjection.m[ 3] + m_ModelProjection.m[ 1];
        m_vPlane[2].y = m_ModelProjection.m[ 7] + m_ModelProjection.m[ 5];
        m_vPlane[2].z = m_ModelProjection.m[11] + m_ModelProjection.m[ 9];
        m_vPlane[2].w = m_ModelProjection.m[15] + m_ModelProjection.m[13];
        
        // Normalize.
        t = nkMath::FastSqrty( m_vPlane[2].x * m_vPlane[2].x + m_vPlane[2].y * m_vPlane[2].y + m_vPlane[2].z * m_vPlane[2].z );
        m_vPlane[2].x /= t;
        m_vPlane[2].y /= t;
        m_vPlane[2].z /= t;
        m_vPlane[2].w /= t;
        
        // Top plane.
        m_vPlane[3].x = m_ModelProjection.m[ 3] - m_ModelProjection.m[ 1];
        m_vPlane[3].y = m_ModelProjection.m[ 7] - m_ModelProjection.m[ 5];
        m_vPlane[3].z = m_ModelProjection.m[11] - m_ModelProjection.m[ 9];
        m_vPlane[3].w = m_ModelProjection.m[15] - m_ModelProjection.m[13];
        
        // Normalize.
        t = nkMath::FastSqrty( m_vPlane[3].x * m_vPlane[3].x + m_vPlane[3].y * m_vPlane[3].y + m_vPlane[3].z * m_vPlane[3].z );
        m_vPlane[3].x /= t;
        m_vPlane[3].y /= t;
        m_vPlane[3].z /= t;
        m_vPlane[3].w /= t;
        
        // Far plane.
        m_vPlane[4].x = m_ModelProjection.m[ 3] - m_ModelProjection.m[ 2];
        m_vPlane[4].y = m_ModelProjection.m[ 7] - m_ModelProjection.m[ 6];
        m_vPlane[4].z = m_ModelProjection.m[11] - m_ModelProjection.m[10];
        m_vPlane[4].w = m_ModelProjection.m[15] - m_ModelProjection.m[14];
        
        // Normalize.
        t = FastSqrty( m_vPlane[4].x * m_vPlane[4].x + m_vPlane[4].y * m_vPlane[4].y + m_vPlane[4].z * m_vPlane[4].z );
        m_vPlane[4].x /= t;
        m_vPlane[4].y /= t;
        m_vPlane[4].z /= t;
        m_vPlane[4].w /= t;
        
        // Near plane.
        m_vPlane[5].x = m_ModelProjection.m[ 3] + m_ModelProjection.m[ 2];
        m_vPlane[5].y = m_ModelProjection.m[ 7] + m_ModelProjection.m[ 6];
        m_vPlane[5].z = m_ModelProjection.m[11] + m_ModelProjection.m[10];
        m_vPlane[5].w = m_ModelProjection.m[15] + m_ModelProjection.m[14];
        
        // Normalize.
        t = FastSqrty( m_vPlane[5].x * m_vPlane[5].x + m_vPlane[5].y * m_vPlane[5].y + m_vPlane[5].z * m_vPlane[5].z );
        m_vPlane[5].x /= t;
        m_vPlane[5].y /= t;
        m_vPlane[5].z /= t;
        m_vPlane[5].w /= t;
    }
}