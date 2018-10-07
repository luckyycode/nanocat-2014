//
//  Vec3.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Vec3_h
#define Vec3_h

#include <math.h>
#include "GameMath.h"
//#include "Mat4.h"

namespace Neko {
    
    template <class t> class vec3_template;
    
    typedef vec3_template<float> Vec3;
    typedef vec3_template<int> Vec3i;
    
    class ncMatrix4;
    
    /// 3-dimensional vector.
    template <class t>
    class vec3_template
    {
    public:
        t x, y, z;
        
        /**
         *  Constructors.
         */
        vec3_template<t>() : x( 0 ), y( 0 ), z( 0 ) { }
        vec3_template<t>( const t x1, const t y1, const t z1 ) : x( x1 ), y( y1 ), z( z1 ) { }
        vec3_template<t>( const t v ) : x(v), y(v), z(v) { }
        
        /**
         *  Squared length.
         */
        inline float LengthSq() const {
            return (x * x) + (y * y) + (z * z);
        }
        
        /**
         *  Cross vectors.
         */
        inline void Cross( const vec3_template<t> &v1, const vec3_template<t> &v2 )
        {
            x = v1.y * v2.z - v1.z * v2.y;
            y = v1.z * v2.x - v1.x * v2.z;
            z = v1.x * v2.y - v1.y * v2.x;
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
        }
        
        /**
         *  Maximum value from vector X Y Z values.
         */
        inline t Max() const
        {
            t maxVal = static_cast<t>( 0 );
            if( this->x > maxVal ) {
                maxVal = this->x;
            }
            
            if( this->y > maxVal ) {
                maxVal = this->y;
            }
            
            if( this->z > maxVal ) {
                maxVal = this->z;
            }
            
            return maxVal;
        }
        
        /**
         *  Find a maximum value form given vector.
         */
        inline vec3_template<t> Max( const vec3_template<t>& r ) const
        {
            vec3_template<t> result;
            
            result.x = x > r.x ? x : r.x;
            result.y = y > r.y ? y : r.y;
            result.z = z > r.z ? z : r.z;
            
            return result;
        }
        
        /**
         *  Vector length.
         */
        inline float Length( void ) {
            return( (float)nkMath::FastSqrty( x * x + y * y + z * z ) );
        }
        
        /**
         *  Mix two vectors.
         */
        inline static Vec3 Mix( const Vec3 &a, const Vec3 &b, float c ) {
            return a * (1.0f - c) + b * c;
        }
        
        /**
         *  Distance between two vectors.
         */
        inline float                Distance( const vec3_template<t> &v1 );
        
        /**
         *  Inverse vector values.
         */
        void                Inverse( void );
        
        /**
         *  Distance between two vector points.
         */
        static float Distance( vec3_template<t> vPoint1, vec3_template<t> vPoint2 ) {
            float distance = sqrt( (vPoint2.x - vPoint1.x) * (vPoint2.x - vPoint1.x) +
                                   (vPoint2.y - vPoint1.y) * (vPoint2.y - vPoint1.y) +
                                   (vPoint2.z - vPoint1.z) * (vPoint2.z - vPoint1.z) );
            
            // Return the distance between the 2 points.
            return (float)distance;
        }
        
        /**
         *  Angle between two points.
         */
        static double AngleBetween( vec3_template<float> & a, vec3_template<float> & b )
        {
            // Get the dot product of the vectors
            float dotProduct = vec3_template<float>::Dot( a, b );
            
            // Get the product of both of the vectors magnitudes
            float vectorsMagnitude = a.Length() * b.Length() ;
            
            // Get the angle in radians between the 2 vectors
            float angle = acosf( dotProduct / vectorsMagnitude );
            
            // Here we make sure that the angle is not a -1.#IND0000000 number, which means indefinate
//#   if !defined(NEKO_SERVER) && !defined(NEKO_NO_ASSIMP)
//            if(__isnan(angle)) // I don't like std, but well..
//                return 0;
//#   endif
            
            // Return the angle in radians
            return angle;
        }
        
        /**
         *  Reflect vector by normal vector.
         */
        const vec3_template<t> Reflect( const vec3_template<t> &n ) const;
        
        /**
         *  Make all vector values to zero.
         */
        inline void MakeNull() { x = y = z = 0.0f; }
        
        /**
         *  Make polygon be normal.
         */
        static Vec3 MakeNormal( Vec3 vPoly[] )
        {
            Vec3 vVec1 = vPoly[2] - vPoly[0];
            Vec3 vVec2 = vPoly[1] - vPoly[0];
            
            Vec3 vNormal = Vec3::CrossTwoVectors( vVec1, vVec2 );
            vNormal.Normalize();
            
            return vNormal;
        }
        
        /**
         *  Operators.
         */
        
        inline vec3_template<t> operator + ( const vec3_template<t>& v ) const {
            vec3_template vResult;
            
            vResult.x = x + v.x;
            vResult.y = y + v.y;
            vResult.z = z + v.z;
            
            return vResult;
        }
        
        inline vec3_template<t> operator - ( const vec3_template<t>& v ) const {
            vec3_template<t> vResult;
            
            vResult.x = x - v.x;
            vResult.y = y - v.y;
            vResult.z = z - v.z;
            
            return vResult;
        }
        
        inline vec3_template<t> operator - () const {
            vec3_template<t> vResult;
            
            vResult.x = -x;
            vResult.y = -y ;
            vResult.z = -z;
            
            return vResult;
        }
        
        vec3_template<t> operator * ( const vec3_template<t>& v ) const {
            vec3_template vResult;
            
            vResult.x = x * v.x;
            vResult.y = y * v.y;
            vResult.z = z * v.z;
            
            return vResult;
        }
        
        inline vec3_template<t> operator * ( const t v ) const {
            vec3_template<t> vResult;
            
            vResult.x = x * v;
            vResult.y = y * v;
            vResult.z = z * v;
            
            return vResult;
        }
        
        inline const vec3_template<t> operator / ( const float v ) const {
            vec3_template vResult;
            vResult.x = x / v;
            vResult.y = y / v;
            vResult.z = z / v;
            
            return vResult;
        }
        
        inline friend const vec3_template<t> operator * ( const int c, const vec3_template<t> &v ) {
            return vec3_template<t>( v.x * c, v.y * c, v.z * c );
        }
        
        inline friend const vec3_template<t> operator * ( const float c, const vec3_template<t> &v ) {
            return vec3_template( v.x * c, v.y * c, v.z * c );
        }
        
        inline friend const vec3_template<t> operator - ( const float num, const vec3_template<t> &u )  {
            return vec3_template<t>( num - u.x, num - u.y, num - u.z );
        }
        
        inline friend const vec3_template<t> operator + ( const float num, const vec3_template<t> &u )  {
            return vec3_template<t>( num + u.x, num + u.y, num + u.z );
        }
        
        void TransformPoint( const ncMatrix4 &mat );
        void TransformVector( const ncMatrix4 &mat );
        
        // Static functions.
        static inline float Dot( const vec3_template<t> &v1, const vec3_template<t> &v2 ) {
            return( v1.x * v2.x + v1.y * v2.y + v1.z * v2.z  );
        }
        
        static vec3_template<t> CrossTwoVectors( const vec3_template<t> &v1, const vec3_template<t> &v2 );
        static inline vec3_template<t> Normalize( const vec3_template<t> &u ) {
            return u / nkMath::FastSqrty(u.x * u.x + u.y * u.y + u.z * u.z);
        }
        
        static vec3_template<t> Rotate( const vec3_template<t> &u, float angle, const vec3_template<t> &v );
        static vec3_template<t> TransformNormal( const vec3_template<t> &pv, const ncMatrix4 &mat );
        
        
        /**
         *  Convert spherical coordinates to cartesian coordinates.
         */
        inline static Vec3 OrbitalToUnity(float radius, float theta, float phi)
        {
            Vec3 res;
            
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);
            float sinPhi   = sinf(phi);
            float cosPhi   = cosf(phi);
            
            res.z = radius * sinTheta * cosPhi;
            res.y = radius * cosTheta;
            res.x = radius * sinTheta * sinPhi;
            
            return res;
        }
        
        /**
         *  Convert spherical coordinates to cartesian coordinates.
         */
        inline static Vec3 OrbitalToLocal(float theta, float phi)
        {
            Vec3 res;
            
            float sinTheta = sinf(theta);
            float cosTheta = cosf(theta);
            float sinPhi   = sinf(phi);
            float cosPhi   = cosf(phi);
            
            res.z = sinTheta * cosPhi;
            res.y = cosTheta;
            res.x = sinTheta * sinPhi;
            
            return res;
        }
        
    };
    
    template<> Vec3 Vec3::Rotate( const Vec3 &u, float angle, const Vec3 &v );
    template<> void Vec3::TransformVector( const ncMatrix4 &mat );
    template<> Vec3 Vec3::CrossTwoVectors( const Vec3 &v1, const Vec3 &v2 );
    //template<> Vec3 Vec3::MakeNormal( Vec3 vPoly[] );
    //template<> const Vec3 Vec3::Reflect( const Vec3 &n ) const;
    
    
    /**
     *  Distance between vectors.
     */
    template <typename t>
    inline float vec3_template<t>::Distance( const vec3_template &v1 )
    {
        float dx = this->x - v1.x;
        float dy = this->y - v1.y;
        float dz = this->z - v1.z;
        
        return nkMath::FastSqrty(dx * dx + dy * dy + dz * dz);
    }
    
    
    
    // Vector directions.
    const static vec3_template <float>VECTOR_UP( 0.0f, 1.0f, 0.0f );
    const static vec3_template <float>VECTOR_ZERO( 0.0f, 0.0f, 0.0f );
    const static vec3_template <float>VECTOR_BACK( 0.0f, 0.0f, -1.0f );
    const static vec3_template <float>VECTOR_DOWN( 0.0f, -1.0f, 0.0f );
    const static vec3_template <float>VECTOR_RIGHT( 1.0f, 0.0f, 0.0f );
    
}

#endif /* Vec3_h */
