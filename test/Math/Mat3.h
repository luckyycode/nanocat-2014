//
//  Mat3.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Mat3_h
#define Mat3_h

#include "Vec4.h"
#include "Vec3.h"

namespace Neko {
    
    /**
     *  Cast to float.
     */
    static inline void sincosf( float angle, float* pSin,float* pCos )
    {
        *pSin = float(sin(angle));
        *pCos = float(cos(angle));
    }
    
    /// 3x3 Matrix.
    class CMatrix3x3
    {
    public:
        
        /**
         *  Default constructor.
         */
        CMatrix3x3()
        {
            m[0] = 0.0f;    m[4] = 0.0f;    m[8] = 0.0f;
            m[1] = 0.0f;    m[5] = 0.0f;
            m[2] = 0.0f;    m[6] = 0.0f;
            m[3] = 0.0f;    m[7] = 0.0f;
        }
        
        float m[9];
        
        /**
         *  Reset matrix.
         */
        inline void SetZero()
        {
            m[0] = 0.0f; m[1] = 0.0f; m[2] = 0.0f;
            m[3] = 0.0f; m[4] = 0.0f; m[5] = 0.0f;
            m[6] = 0.0f; m[7] = 0.0f; m[8] = 0.0f;
        }
        
        /**
         *  Set rotation by X axis.
         */
        void SetRotationX( const float & rad )
        {
            float s, c;
            
            sincosf( rad, &s, &c );
            
            m[0] = 1.0f;
            m[1] = 0.0f;
            m[2] = 0.0f;
            m[3] = 0.0f;
            m[4] = c;
            m[5] = -s;
            m[6] = 0.0f;
            m[7] = s;
            m[8] = c;
        }
        
        /**
         *  Set rotation by Y axis.
         */
        void SetRotationY( const float & rad )
        {
            float s, c;
            
            sincosf( rad, &s, &c );
            
            m[0] = c;
            m[1] = 0.0f;
            m[2] = s;
            m[3] = 0.0f;
            m[4] = 1.0f;
            m[5] = 0.0f;
            m[6] = -s;
            m[7] = 0.0f;
            m[8] = c;
        }
        
        /**
         *  Set rotation by Z axis.
         */
        void SetRotationZ( const float & rad )
        {
            float s, c;
            
            sincosf( rad, &s, &c );
            
            m[0] = c;       // m00
            m[1] = -s;      // m01
            m[2] = 0.0f;    // m02
            m[3] = s;       // m10
            m[4] = c;       // m11
            m[5] = 0.0f;    // m12
            m[6] = 0.0f;    // m20
            m[7] = 0.0f;    // m21
            m[8] = 1.0f;    // m22
        }
        
        /**
         *  Multiply two matrices.
         */
        inline friend CMatrix3x3 operator * ( const CMatrix3x3 &l, const CMatrix3x3 &r )
        {
            CMatrix3x3 m;
            
            m.m[0] = l.m[0]*r.m[0] +	l.m[1]*r.m[3] +	l.m[2]*r.m[6];
            m.m[1] = l.m[0]*r.m[1] +	l.m[1]*r.m[4] +	l.m[2]*r.m[7];
            m.m[2] = l.m[0]*r.m[2] +	l.m[1]*r.m[5] +	l.m[2]*r.m[8];
            m.m[3] = l.m[3]*r.m[0] +	l.m[4]*r.m[3] +	l.m[5]*r.m[6];
            m.m[4] = l.m[3]*r.m[1] +	l.m[4]*r.m[4] +	l.m[5]*r.m[7];
            m.m[5] = l.m[3]*r.m[2] +	l.m[4]*r.m[5] +	l.m[5]*r.m[8];
            m.m[6] = l.m[6]*r.m[0] +	l.m[7]*r.m[3] + l.m[8]*r.m[6];
            m.m[7] = l.m[6]*r.m[1] +	l.m[7]*r.m[4] +	l.m[8]*r.m[7];
            m.m[8] = l.m[6]*r.m[2] +	l.m[7]*r.m[5] +	l.m[8]*r.m[8];
            
            return m;
        }
        
        /**
         *  Multiply vector 3d by matrix.
         */
        inline static Vec3 Multiply( const CMatrix3x3 &m, const Vec3 &p )
        {
            Vec3 v;
            
            v.x	=	m.m[0] * p.x + m.m[1] * p.y + m.m[2] * p.z;
            v.y	=	m.m[3] * p.x + m.m[4] * p.y + m.m[5] * p.z;
            v.z	=	m.m[6] * p.x + m.m[7] * p.y + m.m[8] * p.z;
            
            return	v;
        }
    };
    
    
}

#endif /* Mat3_h */
