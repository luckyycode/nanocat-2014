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
//  BoundingSphere.h
//  Bounding sphere implementation.

#ifndef boundingsphere_h_included
#define boundingsphere_h_included

#include "GameMath.h"

namespace Neko {
    
    /// Bounding sphere.
    struct BoundingSphere
    {
        Vec3 center;
        float radius;
        
        BoundingSphere() {
            center.MakeNull();
            radius = 1.0f;
        }
        
        BoundingSphere(Vec3 aCenter, float aRadius) {
            center = aCenter;
            radius = aRadius;
        }
        
        inline void                 SetCenter( const Vec3 origin ) {       center = origin + center;   }
        inline void                 SetScale( const float scale )   {       radius *= scale;    }
        
        /**
         *  Calculate bounding box.
         */
        static BoundingSphere * Calculate( const Vec3 * aPoints, int32_t count ) {
            Vec3 xmin, xmax, ymin, ymax, zmin, zmax;
            xmin = ymin = zmin = 1000000.0;
            xmax = ymax = zmax = -1000000.0;
            
            for( int32_t i = 0; i < count; ++i ) {
                const Vec3 * p = &aPoints[i];
                
                if(p->x < xmin.x) xmin = *p;
                if(p->x > xmax.x) xmax = *p;
                if(p->y < ymin.y) ymin = *p;
                if(p->y > ymax.y) ymax = *p;
                if(p->z < zmin.z) zmin = *p;
                if(p->z > zmax.z) zmax = *p;
            }
            
            float xSpan = (xmax - xmin).LengthSq();
            float ySpan = (ymax - ymin).LengthSq();
            float zSpan = (zmax - zmin).LengthSq();
            Vec3 dia1 = xmin;
            Vec3 dia2 = xmax;
            float maxSpan = xSpan;
            
            if (ySpan > maxSpan) {
                maxSpan = ySpan;
                dia1 = ymin; dia2 = ymax;
            }
            
            if (zSpan > maxSpan) {
                dia1 = zmin; dia2 = zmax;
            }
            
            Vec3 center = (dia1 + dia2) * 0.5f;
            float sqRad = (dia2 - center).LengthSq();
            float radius = nkMath::FastSqrty(sqRad);
            for( int32_t i = 0; i < count; ++i )
            {
                const Vec3 * p = &aPoints[i];
                
                float d = (*p - center).LengthSq();
                if(d > sqRad) {
                    float r = nkMath::FastSqrty(d);
                    radius = (radius + r) * 0.5f;
                    sqRad = radius * radius;
                    float offset = r - radius;
                    center = (radius * center + offset * *p) / r;
                }
            }
            return new BoundingSphere(center, radius);
        }
    };
}
#endif // boundingsphere_h_included
