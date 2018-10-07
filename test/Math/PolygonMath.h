//
//  PolygonMath.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef PolygonMath_h
#define PolygonMath_h

#include "Vec3.h"
#include "Vec4.h"

namespace Neko {
    
    /**
     *  Graphic stuff.
     */
    namespace ncPolygonMath
    {
        bool InsidePolygon( Vec3 & vIntersection, Vec3 Poly[], uint32_t verticeCount );
        Vec3 ClosestPointOnLine( Vec3 & vA, Vec3  &vB, Vec3 & vPoint );
        bool EdgeSphereCollision( Vec3 &vCenter, Vec3 vPolygon[], int vertexCount, float radius );
        Vec3 CalculateBezierPoint( float t, const Vec3 & p0, const Vec3 & p1, const Vec3 & p2, const Vec3 & p3 );
    }
    
}

#endif /* PolygonMath_h */
