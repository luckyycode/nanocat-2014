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
//  Frustum.h
//  Frustum. :O
//
//  Created by Neko Code on 1/28/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__Frustum__
#define __Nanocat__Frustum__

// Do not include on top, causes conflicts.
// Fix me: make Frustum.cpp
// Fixed.
#include "BoundingBox.h"
#include "Vec4.h"
#include "Mat4.h"

namespace Neko {

    template <class t> class vec4_template;
    
    ///   Frustum.
    class CFrustum
    {
        
    public:
        
        static const int32_t FRUSTUM_OUT        = 0;
        static const int32_t FRUSTUM_IN         = 1;
        static const int32_t FRUSTUM_INTERSECT  = 2;

        /**
         *  Make frustum projection.
         */
        void            Extract( const Vec3& eye );
        
        
        /**
         *  Frustum contains point?
         */
        bool                ContainsPoint( const Vec3&point ) const;
        
        /**
         *  Frustum contains object?
         */
        bool                ContainsObject( const Vec3& point, const float radius ) const;
        
        /**
         *  Check if bounding box inside.
         */
        bool                IsBoxInside( const Vec3 *points ) const;
        
        
        const int32_t           ContainsBoundingBox( const SBoundingBox& bbox ) const;
        
        /**
         *  Check if frustum contains sphere.
         */
        const int32_t               ContainsSphere( const Vec3 & center, const float radius ) const;
 
        /**
         *  Get frustum plane.
         */
        inline Vec4                GetFrustumPlane( const int32_t x ) {       return m_vPlane[x]; }
        
        /**
         *  Get view projection.
         */
        inline ncMatrix4 &              GetModelView()   {       return m_ModelProjection;   }
        
        /**
         *   Where our point belongs.
         */
        static const int32_t                ClassifyPoint( const Vec4 &normal, const Vec3& point );
        
    public:
   
        float   far_dist, near_dist;
        float   fov;
        float   ratio;
        
        Vec3 center;
        
        //
        //  First three floats - normals.
        //  Fourth float - Intersect value.
        //
        Vec4        m_vPlane[8];          // Six planes.
        
        ncMatrix4 m_ModelProjection;            // Matrice Projection * Modelview.
    };
}


#endif
