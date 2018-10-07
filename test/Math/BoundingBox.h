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
//         _j__j__j_\     `-------/ /__j__j__j_
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
//  BoundingBox.h
//  3d bounding box.
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//
#ifndef __boundingbox_h__
#define __boundingbox_h__

#include "Vec3.h"

namespace Neko {
    
    ///   Basic bounding box. :o
    class SBoundingBox
    {
    public:
        /**
         *  Create zero'd bounding box.
         */
        SBoundingBox()
        {
            min.MakeNull();
            max.MakeNull();
        }
        
        /**
         *  Bounding box with defined minumum and maximum.
         */
        SBoundingBox( const Vec3& _min, const Vec3& _max )
        {
            min = _min;
            max = _max;
        }
        
        /**
         *  Check if our bounding box contains a point.
         */
        inline bool ContainsPoint( const Vec3& point ) const
        {
            return ( point.x >= min.x && point.y >= min.y && point.z >= min.z &&
                    point.x <= max.x && point.y <= max.y && point.z <= max.z );
        };
        
        /**
         *  Check if bounding box touches another one.
         */
        inline bool ContainsBoundingBox( const SBoundingBox & another )
        {
            return !(min.x > another.max.x || min.y > another.max.y || min.z > another.max.z ||
                     max.x < another.min.x || max.y < another.min.y || max.z < another.min.z);
        }
        
        /**
         *  Static method to check if two bounding boxes are touching each other.
         */
        static inline bool ContainsBoxes( const Vec3& min1, const Vec3& max1,
                                 const Vec3& min2, const Vec3& max2 )
        {
            return !(min1.x > max2.x || min1.y > max2.y || min1.z > max2.z ||
                     max1.x < min2.x || max1.y < min2.y || max1.z < min2.z);
        }

        
        /**
         *  Add point and check if it exceedes our maximum or 
         *  has lower value than minumum point.
         */
        inline void Add( const Vec3& v )
        {
            if( v.x > max.x )
                max.x = v.x;
            
            if( v.x < min.x )
                min.x = v.x;
            
            if( v.y > max.y )
                max.y = v.y;
            
            if( v.y < min.y )
                min.y = v.y;
            
            if( v.z > max.z )
                max.z = v.z;
            
            if( v.z < min.z )
                min.z = v.z;
        };
        
        /**
         *  Add existing bounding box and 'fit' it.
         */
        inline void Add( const SBoundingBox& bb )
        {
            if( bb.max.x > max.x )
                max.x = bb.max.x;
            
            if( bb.min.x < min.x )
                min.x = bb.min.x;
            
            if( bb.max.y > max.y )
                max.y = bb.max.y;
            
            if( bb.min.y < min.y )
                min.y = bb.min.y;
            
            if( bb.max.z > max.z )
                max.z = bb.max.z;
            
            if( bb.min.z < min.z )
                min.z = bb.min.z;
        }
        
        /**
         *  Get half size extent.
         */
        inline Vec3 GetHalfExtent() const
        {
            return (max - min) * 0.5f;
        }
        
        /**
         *  Get size extent.
         */
        inline Vec3 GetExtent() const
        {
            return (max - min);
        }
        
        /**
         *  Find out if bounding boxes intersect each other.
         */
        static inline bool DoBoxesIntersect( Vec3 &min1, Vec3 &max1, Vec3 &min2, Vec3 &max2, float fTolerance )
        {
            if(	min1.x - max2.x >= -fTolerance || max1.x - min2.x <= fTolerance ||
               min1.y - max2.y >= -fTolerance || max1.y - min2.y <= fTolerance ||
               min1.z - max2.z >= -fTolerance || max1.z - min2.z <= fTolerance )
            {
                return false;
            }
            else
            {
                return true;
            }
        }
        
        /**
         *  Set bounding box position.
         */
        inline void Translate( const Vec3& v )
        {
            min = min + v;
            max = max + v;
        }
        
        /**
         *  Get bounding box center position.
         */
        inline Vec3 GetCenter() const
        {
            return (max + min) / 2.0f;
        }
    
    //private:
        Vec3 min, max;
    };
}

#endif // __boundingbox_h__
