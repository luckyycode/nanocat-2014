//
//  Vec4.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Vec4_h
#define Vec4_h

#include "GameMath.h"

namespace Neko {
    
    template <class t> class vec4_template;
    template <class t> class vec3_template;
    
    typedef vec4_template<float> Vec4;
    typedef vec4_template<int> Vec4i;
    typedef vec4_template<int8_t> Vec4i8;
    
    class ncMatrix4;
    
    /// 4-dimensional vector.
    template <class t> class vec4_template {
    public:
        
        t x;
        t y;
        t z;
        t w;
        
        /**
         *  Constructors.
         */
        vec4_template() : x( 0 ), y( 0 ), z( 0 ), w( 0 ) { }
        vec4_template( const t x1, const t y1, const t z1, const t w1 ) : x( x1 ), y( y1 ), z( z1 ), w( w1 ) { }
        vec4_template( const t v ) :  x( v ), y( v ), z( v ), w( v ) { }
        vec4_template( const vec3_template<t> & k, t w1 ) : x( k.x ),  y( k.y ), z( k.z ), w( w1 ) { }
        
        void                TransformVector( const ncMatrix4 &mat );

        /**
         *  Addition.
         */
        vec4_template operator + ( const vec4_template<t>& v ) const
        {
            return vec4_template<t>(x + v.x, y + v.y, z + v.z, w + v.w );
        }
        
        /**
         *  Difference.
         */
        vec4_template operator - (const vec4_template<t>& v) const
        {
            return vec4_template<t>( x - v.x, y - v.y, z - v.z, w - v.w );
        }
        
        /**
         *  Multiplication.
         */
        vec4_template operator * (const t v) const
        {
            return vec4_template<t>( x * v, y * v, z * v, w * v );
        }
        
        /**
         *  Division.
         */
        vec4_template operator / (const t v) const
        {
            return vec4_template<t>( x / v, y / v, z / v, w / v );
        }
        
        /**
         *  Squared length.
         */
        inline float LengthSq() const
        {
            return (x * x) + (y * y) + (z * z) + (w * w);
        }
        
        /**
         *  Mix two vectors.
         */
        inline static vec4_template<t> Mix( const vec4_template<t> &a, const vec4_template<t> &b, float c )
        {
            return a * (1.0f - c) + b * c;
        }
        
        /**
         *  Return three-dimensional component.
         */
        inline vec3_template<t>                 xyz() const     {   return vec3_template<t>( x, y, z );    }
        
        /**
         *  Vector length.
         */
        inline float Length( void )
        {
            return( (float)nkMath::FastSqrty( x * x + y * y + z * z + w * w ) );
        }
        
        
        /**
         *  Addition.
         */
        inline vec4_template& operator += (const vec4_template& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            
            return *this;
        }
        
        /**
         *  Difference.
         */
        inline vec4_template& operator -= (const vec4_template& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
            
            return *this;
        }
        
        /**
         *  Multiplication.
         */
        inline vec4_template& operator *= (float f)
        {
            x *= f;
            y *= f;
            z *= f;
            w *= f;
            
            return *this;
        }
        /**
         *  Division.
         */
        inline vec4_template& operator /= (float f)
        {
            f = 1.0f / f;
            
            x *= f;
            y *= f;
            z *= f;
            w *= f;
            
            return *this;
        }

        
        /**
         *  Normalize vector.
         */
        inline void Normalize()
        {
            const float fLength = Length();
            
            if( fLength == 1.0f || fLength == 0.0f )
                return;
            
            x = x / fLength;
            y = y / fLength;
            z = z / fLength;
            //            w = w / fLength;
        }
        
        
        // Static functions.
        static inline float Dot( const vec4_template<t> &v1, const vec4_template<t> &v2 ) {
            return( v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w );
        }
        
    };
    
    
    
    // R G B A
    const static vec4_template<float> COLOR_RED( 1.0f, 0.0f, 0.0f, 1.0f );
    const static vec4_template<float> COLOR_WHITE( 1.0f, 1.0f, 1.0f, 1.0f );
    const static vec4_template<float> COLOR_BLUE( 0.0f, 0.0f, 1.0f, 1.0f );
    const static vec4_template<float> COLOR_GREEN( 0.0f, 1.0f, 0.0f, 1.0f );
    
}

#endif /* Vec4_h */
