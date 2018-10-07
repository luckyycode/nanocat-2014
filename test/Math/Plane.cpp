//
//  Plane.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#include "Plane.h"
#include "Vec3.h"

namespace Neko {
    
    
    /**
     *  Check point plane.
     */
    int32_t ncPlane::ClassifyPoint( const Vec3& point )
    {
        float distance = point.x * m_normal.x + point.y * m_normal.y + point.z * m_normal.z + m_intercept;
        
        if( distance > nkMath::EPSILON ) {
            return nkMath::POINT_IN_FRONT_OF_PLANE;
        }
        
        if( distance <- nkMath::EPSILON ) {
            return nkMath::POINT_BEHIND_PLANE;
        }
        
        return nkMath::POINT_ON_PLANE;
    }
    
    /**
     *  Normalize plane.
     */
    void ncPlane::Normalize()
    {
        float normalLength = m_normal.Length();
        
        m_normal.x = m_normal.x / normalLength;
        m_normal.y = m_normal.y / normalLength;
        m_normal.z = m_normal.z / normalLength;
        
        m_intercept /= normalLength;
    }
    
    bool ncPlane::Intersect3( const ncPlane & p2, const ncPlane & p3, Vec3 & result )
    {
        float denominator = Vec3::Dot( m_normal, Vec3::CrossTwoVectors( p2.GetNormal(),p3.GetNormal() ) );
        // Scalar triple product of normals.
        if( denominator == 0.0f )
        {
            return false;
        }
        
        Vec3 temp1, temp2, temp3;
        temp1 = (Vec3::CrossTwoVectors(p2.GetNormal(), p3.GetNormal())) * GetIntercept();
        temp2 = (Vec3::CrossTwoVectors(p3.GetNormal(), m_normal)) * p2.GetIntercept();
        temp3 = (Vec3::CrossTwoVectors(m_normal, p2.GetNormal())) * p3.GetIntercept();
        
        result = (temp1 + temp2 + temp3) / (-denominator);
        
        return true;
    }
}
