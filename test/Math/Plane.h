//
//  Plane.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Plane_h
#define Plane_h

#include "Vec4.h"
#include "Vec3.h"

namespace Neko {
    
    ///  Plane.
    class ncPlane
    {
    public:
        
        /**
         *    Constructors.
         */
        ncPlane() : m_normal( 1.0f ), m_intercept( 0.0f ) { }
        ncPlane( const Vec3& normalvec, float distance ) : m_normal( normalvec ), m_intercept( distance ) { }
        
        /**
         *  Classify a point and return a type.
         */
        int32_t                 ClassifyPoint( const Vec3& point );
        
        /**
         *  Normalize plane.
         */
        void                Normalize( void );
        
        /**
         *  Set plane normal.
         */
        inline void SetNormal( const Vec3 & rhs )
        {
            m_normal = rhs;
        }
        
        /**
         *  Set plane intercept.
         */
        inline void SetIntercept( float newIntercept )
        {
            m_intercept = newIntercept;
        }
        
        /**
         *  Create plane normal from given points.
         */
        void                SetFromPoints( const Vec3 & p0, const Vec3 & p1, const Vec3 & p2 );
        bool                Intersect3( const ncPlane & p2, const ncPlane & p3, Vec3 & result );
        
        inline float GetDistanceWithNormals( const Vec3 & point ) const
        {
            return point.x * m_normal.x + point.y * m_normal.y + point.z * m_normal.z + m_intercept;
        }
        
        //
        //  Getters.
        inline float                GetIntercept() const {     return m_intercept; }
        
        inline const                Vec3 & GetNormal() const {     return m_normal; }
        
        /**
         *  Get Plane distance.
         */
        static float PlaneDistance( Vec3 & Normal, Vec3 & Point )
        {
            // This variable holds the distance.
            float distance = 0.0f;
            distance = - ((Normal.x * Point.x) + (Normal.y * Point.y) + (Normal.z * Point.z));
            
            // Return the distance.
            return distance;
        }
        
        static int32_t ClassifySphere( Vec3 &vCenter,
                                      Vec3 &vNormal, Vec3 &vPoint, float radius, float &distance )
        {
            // First we need to find the distance our polygon plane is from the origin.
            float d = (float)ncPlane::PlaneDistance( vNormal, vPoint );
            
            // Find the distance the center point of the sphere is from the polygon's plane.
            distance = (vNormal.x * vCenter.x + vNormal.y * vCenter.y + vNormal.z * vCenter.z + d);
            
            // Intersected the plane.
            if( nkMath::Abs(distance) < radius ) {
                return nkMath::POINT_ON_PLANE;
            }
            
            // Else, in front of plane.
            else if(distance >= radius) {
                return nkMath::POINT_IN_FRONT_OF_PLANE;
            }
            
            // If the sphere isn't intersecting or in FRONT of the plane, it must be BEHIND
            return nkMath::POINT_BEHIND_PLANE;
        }
        
        
        //private:
        
        float m_intercept;
        Vec3 m_normal;
    };
    
    // Simple plane.
    struct ncBasicPlaneStruct {
        Vec3 n;
        float d;
    };
    
}

#endif /* Plane_h */
