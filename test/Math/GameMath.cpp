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
//  GameMath.cpp
//  GameMath library.. :P
//
//  Created by Neko Code on 7/27/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.
//

#include "../Core/Core.h"
#include "../Core/Player/Camera/Camera.h" // For Matrices and another view stuff.
#include "../Platform/Shared/SystemShared.h"
#include "../Math/GameMath.h"

/* Oh my gosh, I can't believe.. I finally have something like this... */
//using namespace nkMath; // Not using namespaces.

namespace Neko {

    //  Definitions.
    template class vec3_template<int>;
    template class vec3_template<float>;
    template class vec3_template<double>;

    /**
     *  Matrix4x4 stuff.
     */
    
    /**
     *  Rotate 4x4 Matrix by axis.
     */
    void ncMatrix4::Rotate( const float angle, Vec3 axis )
    {
        float s = sin( nkMath::DEGTORAD(angle) );
        float c = cos( nkMath::DEGTORAD(angle) );

        axis.Normalize();

        float ux = axis.x;
        float uy = axis.y;
        float uz = axis.z;

        m[0]  = c + (1-c) * ux;
        m[1]  = (1-c) * ux*uy + s*uz;
        m[2]  = (1-c) * ux*uz - s*uy;
        m[3]  = 0;

        m[4]  = (1-c) * uy*ux - s*uz;
        m[5]  = c + (1-c) * pow(uy,2);
        m[6]  = (1-c) * uy*uz + s*ux;
        m[7]  = 0;

        m[8]  = (1-c) * uz*ux + s*uy;
        m[9]  = (1-c) * uz*uz - s*ux;
        m[10] = c + (1-c) * pow(uz,2);
        m[11] = 0;

        m[12] = 0;
        m[13] = 0;
        m[14] = 0;
        m[15] = 1;
    }

    void ncMatrix4::RotateX( const float angle )
    {
        float s = sin( nkMath::DEGTORAD(angle) );
        float c = cos( nkMath::DEGTORAD(angle) );

        //this->Identity();

        m[5]  =  c;
        m[6]  =  s;
        m[9]  = -s;
        m[10] =  c;
    }

    void ncMatrix4::RotateY( const float angle )
    {
        float s = sin( nkMath::DEGTORAD(angle) );
        float c = cos( nkMath::DEGTORAD(angle) );

        //this->Identity();

        m[0]  =  c;
        m[2]  = -s;
        m[8]  =  s;
        m[10] =  c;
    }

    void ncMatrix4::RotateZ( const float angle )
    {
        float s = sin( nkMath::DEGTORAD(angle) );
        float c = cos( nkMath::DEGTORAD(angle) );

        //this->Identity();

        m[0] =  c;
        m[1] =  s;
        m[4] = -s;
        m[5] =  c;
    }

    /**
     *  Scale 4x4 Matrix.
     */
    void ncMatrix4::Scale( const Vec3 &v )
    {
        m[0] = v.x;
        m[5] = v.y;
        m[10] = v.z;
    }
    
    void ncMatrix4::ScaleX( const float &v )
    {
        m[0] = v;
    }
    
    void ncMatrix4::ScaleY( const float &v )
    {
        m[5] = v;
    }
    
    void ncMatrix4::ScaleZ( const float &v )
    {
        m[10] = v;
    }
    
    

    /**
     *  Matrix 4x4 minus operator.
     */
    ncMatrix4 ncMatrix4::operator - ( const ncMatrix4 &v ) const
    {
        ncMatrix4 result;

        result.m[0]  = m[0]  - v.m[0];
        result.m[1]  = m[1]  - v.m[1];
        result.m[2]  = m[2]  - v.m[2];
        result.m[3]  = m[3]  - v.m[3];

        result.m[4]  = m[4]  - v.m[4];
        result.m[5]  = m[5]  - v.m[5];
        result.m[6]  = m[6]  - v.m[6];
        result.m[7]  = m[7]  - v.m[7];

        result.m[8]  = m[8]  - v.m[8];
        result.m[9]  = m[9]  - v.m[9];
        result.m[10] = m[10] - v.m[10];
        result.m[11] = m[11] - v.m[11];

        result.m[12] = m[12] - v.m[12];
        result.m[13] = m[13] - v.m[13];
        result.m[14] = m[14] - v.m[14];
        result.m[15] = m[15] - v.m[15];

        return result;
    }

    /**
     *  Matrix 4x4 plus operator.
     */
    ncMatrix4 ncMatrix4::operator + ( const ncMatrix4 &v ) const
    {
        ncMatrix4 result;

        result.m[0]  = m[0]  + v.m[0];
        result.m[1]  = m[1]  + v.m[1];
        result.m[2]  = m[2]  + v.m[2];
        result.m[3]  = m[3]  + v.m[3];

        result.m[4]  = m[4]  + v.m[4];
        result.m[5]  = m[5]  + v.m[5];
        result.m[6]  = m[6]  + v.m[6];
        result.m[7]  = m[7]  + v.m[7];

        result.m[8]  = m[8]  + v.m[8];
        result.m[9]  = m[9]  + v.m[9];
        result.m[10] = m[10] + v.m[10];
        result.m[11] = m[11] + v.m[11];

        result.m[12] = m[12] + v.m[12];
        result.m[13] = m[13] + v.m[13];
        result.m[14] = m[14] + v.m[14];
        result.m[15] = m[15] + v.m[15];

        return result;
    }
    
#ifndef NEKO_SERVER
    
    /**
     *  Matrix 4x4 mutator.
     */
    ncMatrix4 ncMatrix4::operator = ( const aiMatrix4x4t<float> & v )
    {
        ncMatrix4 temp = ncMatrix4( v.a1, v.a2, v.a3, v.a4,
                       v.b1, v.b2, v.b3, v.b4,
                       v.c1, v.c2, v.c3, v.c4,
                       v.d1, v.d2, v.d3, v.d4 );
        
        return temp;
    }
#endif

    /**
     *  Multiply two matrices.
     */
    ncMatrix4 ncMatrix4::operator*( const ncMatrix4 &v ) const
    {
        ncMatrix4 result;

        result.m[0]  = (m[0] * v.m[0]) + (m[4] * v.m[1]) + (m[8] * v.m[2]) + (m[12] * v.m[3]);
        result.m[1]  = (m[1] * v.m[0]) + (m[5] * v.m[1]) + (m[9] * v.m[2]) + (m[13] * v.m[3]);
        result.m[2]  = (m[2] * v.m[0]) + (m[6] * v.m[1]) + (m[10] * v.m[2]) + (m[14] * v.m[3]);
        result.m[3]  = (m[3] * v.m[0]) + (m[7] * v.m[1]) + (m[11] * v.m[2]) + (m[15] * v.m[3]);

        result.m[4]  = (m[0] * v.m[4]) + (m[4] * v.m[5]) + (m[8] * v.m[6]) + (m[12] * v.m[7]);
        result.m[5]  = (m[1] * v.m[4]) + (m[5] * v.m[5]) + (m[9] * v.m[6]) + (m[13] * v.m[7]);
        result.m[6]  = (m[2] * v.m[4]) + (m[6] * v.m[5]) + (m[10] * v.m[6]) + (m[14] * v.m[7]);
        result.m[7]  = (m[3] * v.m[4]) + (m[7] * v.m[5]) + (m[11] * v.m[6]) + (m[15] * v.m[7]);

        result.m[8]  = (m[0] * v.m[8]) + (m[4] * v.m[9]) + (m[8] * v.m[10]) + (m[12] * v.m[11]);
        result.m[9]  = (m[1] * v.m[8]) + (m[5] * v.m[9]) + (m[9] * v.m[10]) + (m[13] * v.m[11]);
        result.m[10] = (m[2] * v.m[8]) + (m[6] * v.m[9]) + (m[10] * v.m[10]) + (m[14] * v.m[11]);
        result.m[11] = (m[3] * v.m[8]) + (m[7] * v.m[9]) + (m[11] * v.m[10]) + (m[15] * v.m[11]);

        result.m[12] = (m[0] * v.m[12]) + (m[4] * v.m[13]) + (m[8] * v.m[14]) + (m[12] * v.m[15]);
        result.m[13] = (m[1] * v.m[12]) + (m[5] * v.m[13]) + (m[9] * v.m[14]) + (m[13] * v.m[15]);
        result.m[14] = (m[2] * v.m[12]) + (m[6] * v.m[13]) + (m[10] * v.m[14]) + (m[14] * v.m[15]);
        result.m[15] = (m[3] * v.m[12]) + (m[7] * v.m[13]) + (m[11] * v.m[14]) + (m[15] * v.m[15]);

        return result;
    }

    /**
     *  Multiply 4x4 matrix by value.
     */
    ncMatrix4 ncMatrix4::operator * ( const float f ) const
    {
        ncMatrix4 result;

        result.m[0]  = m[0]  * f;
        result.m[1]  = m[1]  * f;
        result.m[2]  = m[2]  * f;
        result.m[3]  = m[3]  * f;

        result.m[4]  = m[4]  * f;
        result.m[5]  = m[5]  * f;
        result.m[6]  = m[6]  * f;
        result.m[7]  = m[7]  * f;

        result.m[8]  = m[8]  * f;
        result.m[9]  = m[9]  * f;
        result.m[10] = m[10] * f;
        result.m[11] = m[11] * f;

        result.m[12] = m[12] * f;
        result.m[13] = m[13] * f;
        result.m[14] = m[14] * f;
        result.m[15] = m[15] * f;

        return result;
    }

    /**
     *   Determinant.
     */
    float ncMatrix4::Determinant( void ) const
    {
        return ((this->m[0] * this->m[5] * this->m[10]) +
                (this->m[4] * this->m[9] * this->m[2])  +
                (this->m[8] * this->m[1] * this->m[6])  -
                (this->m[8] * this->m[5] * this->m[2])  -
                (this->m[4] * this->m[1] * this->m[10]) -
                (this->m[0] * this->m[9] * this->m[6]));
    }

    //    0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15
    //    a1 a2 a3 a4 b1 b2 b3 b4 c1 c2 c3 c4 d1 d2 d3 d4

    void ncMatrix4::Transpose( void )
    {
//        std::swap( (float&)m[5], (float&)m[1]);
//        std::swap( (float&)m[8], (float&)m[2]);
//        std::swap( (float&)m[9], (float&)m[6]);
//        std::swap( (float&)m[12], (float&)m[3]);
//        std::swap( (float&)m[13], (float&)m[7]);
//        std::swap( (float&)m[14], (float&)m[7]);
        m[0]  = m[0];
        m[1]  = m[4];
        m[2]  = m[8];
        m[3]  = m[12];

        m[4]  = m[1];
        m[5]  = m[5];
        m[6]  = m[9];
        m[7]  = m[13];

        m[8]  = m[2];
        m[9]  = m[6];
        m[10] = m[10];
        m[11] = m[14];

        m[12] = m[3];
        m[13] = m[7];
        m[14] = m[11];
        m[15] = m[15];
    }

    /**
     *
     *  Vector stuff.
     *
     */

    /**
     *  Inverse 3d vector.
     */
    template <typename t>
    void vec3_template<t>::Inverse( void )
    {
        x = -x;
        y = -y;
        z = -z;
    }


    /**
     *  Reflects this vector about the normal vector n.
     */
    template <>
    const Vec3 Vec3::Reflect( const Vec3 &n ) const
    {
        //
        //  v1 - this vector
        //  v2 - reflected vector
        //  n - normal to reflect against
        //
        //  v2 = v1 - 2 * n * Dot(v1, n)
        //

        vec3_template v1 = *this;

        return v1 - 2 * n * Dot( *this, n );
    }

    //template <typename t>
    //vec3_template<t> vec3_template<t>::Normalize( const vec3_template &u ) {
    //    return u / nkMath::FastSqrty(u.x * u.x + u.y * u.y + u.z * u.z);
    //}


    /**
     *  Cross two vectors.
     */
    template <>
    Vec3 Vec3::CrossTwoVectors( const Vec3 &v1, const Vec3 &v2 )
    {
        Vec3 finalResult;

        finalResult.x = v1.y * v2.z - v1.z * v2.y;
        finalResult.y = v1.z * v2.x - v1.x * v2.z;
        finalResult.z = v1.x * v2.y - v1.y * v2.x;

        return finalResult;
    }

    /**
     *  Rotate matrix 4x4 by axis.
     */
    ncMatrix4 ncMatrix4::Rotate2( float angle, const Vec3 &u )
    {
        ncMatrix4 Rotate;

        angle = angle / 180.0f * (float)nkMath::PI;

        Vec3 v = Vec3::Normalize(u);

        float c = 1.0f - cos(angle);
        float s = sin(angle);

        Rotate.m[0] = 1.0f + c * (v.x * v.x - 1.0f);
        
        Rotate.m[1] = c * v.x * v.y + v.z * s;
        Rotate.m[2] = c * v.x * v.z - v.y * s;
        Rotate.m[4] = c * v.x * v.y - v.z * s;
        
        Rotate.m[5] = 1.0f + c * (v.y * v.y - 1.0f);
        
        Rotate.m[6] = c * v.y * v.z + v.x * s;
        Rotate.m[8] = c * v.x * v.z + v.y * s;
        Rotate.m[9] = c * v.y * v.z - v.x * s;
        
        Rotate.m[10] = 1.0f + c * (v.z * v.z - 1.0f);

        return Rotate;
    }

    /**
     *  Rotate vector 3d and multiply by matrix 4x4.
     */
    template <>
    Vec3 Vec3::Rotate( const Vec3 &u, float angle, const Vec3 &v )
    {
        Vec4 u2 = Vec4( u, 1.0 );
        Vec4 uRotated = ( ncMatrix4::Rotate2( angle, v ) * u2 );
        return Vec3( uRotated.x, uRotated.y, uRotated.z );
    }

    /**
     *  Multiply matrix 4x4 by 4d vector.
     */
    Vec4 ncMatrix4::operator * ( const Vec4& u )
    {
        Vec4 v;

        v.x = m[0] * u.x + m[4] * u.y + m[8] * u.z + m[12] * u.w;
        v.y = m[1] * u.x + m[5] * u.y + m[9] * u.z + m[13] * u.w;
        v.z = m[2] * u.x + m[6] * u.y + m[10] * u.z + m[14] * u.w;
        v.w = m[3] * u.x + m[7] * u.y + m[11] * u.z + m[15] * u.w;

        return v;
    }

    /**
     *  Transform point from 4x4 matrix.
     */
    template <typename t>
    void vec3_template<t>::TransformPoint( const ncMatrix4 &mat )
    {
        float x1 = x;
        float y1 = y;
        float z1 = z;

        x = x1 * mat.m[0] + y1 * mat.m[4] + z1 * mat.m[8] + mat.m[12];
        y = x1 * mat.m[1] + y1 * mat.m[5] + z1 * mat.m[9] + mat.m[13];
        z = x1 * mat.m[2] + y1 * mat.m[6] + z1 * mat.m[10]+ mat.m[14];
    }

    /**
     *  Transform vector by 4x4 matrix.
     */
    template<> void Vec4::TransformVector( const ncMatrix4 &mat )
    {
        float x1 = x;
        float y1 = y;
        float z1 = z;
        float w1 = w;

        x = x1 * mat.m[0] + y1 * mat.m[4] + z1 * mat.m[8] + w1 * mat.m[11];
        y = x1 * mat.m[1] + y1 * mat.m[5] + z1 * mat.m[9] + w1 * mat.m[12];
        z = x1 * mat.m[2] + y1 * mat.m[6] + z1 * mat.m[10] + w1 * mat.m[13];
    }

    template <typename t>
    vec3_template<t> vec3_template<t>::TransformNormal( const vec3_template &pv, const ncMatrix4 &mat )
    {
        vec3_template out;

        out.x = mat.m[0] * pv.x + mat.m[4] * pv.y + mat.m[8] * pv.z;
        out.y = mat.m[1] * pv.x + mat.m[5] * pv.y + mat.m[9] * pv.z;
        out.z = mat.m[2] * pv.x + mat.m[6] * pv.y + mat.m[10] * pv.z;

        return out;
    }

    template <>
    void Vec3::TransformVector( const ncMatrix4 &mat )
    {
        float x1 = x;
        float y1 = y;
        float z1 = z;

        x = x1 * mat.m[0] + y1 * mat.m[4] + z1 * mat.m[8];
        y = x1 * mat.m[1] + y1 * mat.m[5] + z1 * mat.m[9];
        z = x1 * mat.m[2] + y1 * mat.m[6] + z1 * mat.m[10];
    }

    /**
     *  Graphical stuff.
     */
    namespace ncPolygonMath
    {
        /**
         *  Is line inside of polygon?
         */
        static const double MATCH_FACTOR = 0.99;           // Used to cover up the error in floating point.
        
        bool InsidePolygon( Vec3 & vIntersection, Vec3 Poly[], uint32_t verticeCount )
        {
            double Angle = 0.0;
            uint32_t i;
            
            // Create temp vectors.
            Vec3 vA, vB;
            
            for( i = 0; i < verticeCount; ++i )
            {
                vA = Poly[i] - vIntersection;
                // Subtract the point from the next vertex.
                vB = Poly[(i + 1) % verticeCount] - vIntersection;
                
                Angle += Vec3::AngleBetween(vA, vB);
            }
            
            if( Angle >= (MATCH_FACTOR * (2.0 * nkMath::PI)) )
            {
                // If the angle is greater than 2 PI, (360 degrees)
                return true;
            }
            
            return false;
        }
        
        /**
         *  Find closest point on given line ( vA-vB ).
         */
        Vec3 ClosestPointOnLine( Vec3 & vA, Vec3 & vB, Vec3 & vPoint )
        {
            // Create the vector from end point vA to our point vPoint.
            Vec3 vVector1 = vPoint - vA;
            
            // Create a normalized direction vector from end point vA to end point vB.
            Vec3 vVector2 = Vec3::Normalize( vB - vA );
            
            // Find the distance of the line segment.
            float d = Vec3::Distance( vA, vB );
            
            // This gives us the distance from our projected vector from vA.
            float t = Vec3::Dot( vVector2, vVector1 );
            
            if( t <= 0 )
                return vA;
            
            // If our projected distance from vA, "t", is greater than or equal to the magnitude
            // or distance of the line segment, it must be closest to the end point vB. So, return vB.
            if( t >= d )
                return vB;
            
            // Here we create a vector that is of length t and in the direction of vVector2.
            Vec3 vVector3 = vVector2 * t;
            
            // To find the closest point on the line segment.
            Vec3 vClosestPoint = vA + vVector3;
            
            // Return the closest point on the line segment
            return vClosestPoint;
        }
        
        /**
         *  Is given point sphere edge?
         */
        bool EdgeSphereCollision( Vec3 &vCenter,
                                 Vec3 vPolygon[], int vertexCount, float radius )
        {
            uint32_t i;
            Vec3 vPoint;
            
            // Go through all of the vertices in the polygon
            for( i = 0; i < vertexCount; ++i )
            {
                // This returns the closest point on the current edge to the center of the sphere.
                vPoint = ClosestPointOnLine( vPolygon[i], vPolygon[(i + 1) % vertexCount], vCenter );
                
                // Now, we want to calculate the distance between the closest point and the center
                float distance = Vec3::Distance( vPoint, vCenter );
                
                // If the distance is less than the radius, there must be a collision so return true
                if( distance < radius )
                {
                    return true;
                }
            }
            
            // The was no intersection of the sphere and the edges of the polygon
            return false;
        }
        
        /**
         *  Calculate bezier patch.
         */
        Vec3 CalculateBezierPoint( float t, const Vec3 & p0, const Vec3 & p1, const Vec3 & p2, const Vec3 & p3 )
        {
            float u = 1.0f - t;
            float tt = t * t;
            float uu = u * u;
            float uuu = uu * u;
            float ttt = tt * t;
            
            Vec3 p = uuu * p0;
            p = p + 3.0f * uu * t * p1;
            p = p + 3.0f * u * tt * p2;
            p = p + ttt * p3;
            
            return p;
        }
        
    }

    // ---

    /**
     *      FFT stuff.
     *      http://www.keithlantz.net/
     *      http://www.keithlantz.net/2011/11/ocean-simulation-part-two-using-the-fast-fourier-transform/
     */
    unsigned int ncFFTComplex::additions = 0;
    unsigned int ncFFTComplex::multiplications = 0;

    ncFFTComplex::ncFFTComplex() : a( 0.0f ), b( 0.0f )
    {

    }

    ncFFTComplex::ncFFTComplex( float a, float b ) : a( a ), b( b )
    {

    }

    ncFFTComplex ncFFTComplex::Conjugate()
    {
        return ncFFTComplex( this->a, -this->b );
    }

    ncFFTComplex ncFFTComplex::operator*( const ncFFTComplex& c ) const
    {
        ++ncFFTComplex::multiplications;
        return ncFFTComplex( this->a * c.a - this->b * c.b, this->a * c.b + this->b * c.a);
    }

    ncFFTComplex ncFFTComplex::operator+( const ncFFTComplex& c ) const
    {
        ++ncFFTComplex::additions;
        return ncFFTComplex( this->a + c.a, this->b + c.b );
    }

    ncFFTComplex ncFFTComplex::operator-( const ncFFTComplex& c ) const
    {
        ++ncFFTComplex::additions;
        return ncFFTComplex( this->a - c.a, this->b - c.b );
    }

    ncFFTComplex ncFFTComplex::operator-() const
    {
        return ncFFTComplex( -this->a, -this->b );
    }

    ncFFTComplex ncFFTComplex::operator*( const float c ) const
    {
        return ncFFTComplex( this->a * c, this->b * c );
    }

    ncFFTComplex& ncFFTComplex::operator=( const ncFFTComplex& c )
    {
        this->a = c.a; this->b = c.b;
        return *this;
    }

    void ncFFTComplex::Reset()
    {
        ncFFTComplex::additions = 0;
        ncFFTComplex::multiplications = 0;
    }

    ncFFT::ncFFT( uint32_t Ns ) : N(Ns), reversed(0), T(0)
    {
        Create( this->N );
    }
    
    ncFFT::ncFFT() : N(0), reversed(0), T(0)
    {
        
    }
    
    /**
     *  Create new fft operation.
     */
    void ncFFT::Create( uint32_t N )
    {
        c[0] = c[1] = 0;
        
        this->N = N;
        log_2_N = log( N ) / log( 2 );
        
        which = 0;
        T = 0;
        reversed = (uint32_t *)pAllocator->Alloc( sizeof(uint32_t) * N );
        
        for( uint32_t i = 0; i < N; ++i ) {
            reversed[i] = Reverse(i);
        }
        
        int32_t pow2 = 1;
        T = (ncFFTComplex **)pAllocator->Alloc( sizeof(ncFFTComplex *) * log_2_N );;		// prep T
        
        for( uint32_t i = 0; i < log_2_N; ++i ) {
            T[i] = (ncFFTComplex *)pAllocator->Alloc( sizeof(ncFFTComplex) * pow2 );
            
            for( uint32_t j = 0; j < pow2; ++j ) {
                T[i][j] = t( j, pow2 * 2 );
            }
            
            pow2 *= 2;
        }
        
        c[0] = (ncFFTComplex *)pAllocator->Alloc( sizeof(ncFFTComplex) * N );
        c[1] = (ncFFTComplex *)pAllocator->Alloc( sizeof(ncFFTComplex) * N );
        
        which = 0;
    }

    ncFFT::~ncFFT()
    {
        if( c[0] ) pAllocator->Dealloc( c[0] ) ;
        if( c[1] ) pAllocator->Dealloc( c[1] );

        if( T != NEKO_NULL ) {
            for( int i = 0; i < log_2_N; ++i ) {
                if ( T[i] ) {
                    pAllocator->Dealloc( T[i] );
                }
            }
            
            delete [] T;
        }

        if( reversed != NEKO_NULL ) {
            pAllocator->Dealloc(reversed) ;
        }
    }

    unsigned int ncFFT::Reverse( unsigned int i )
    {
        unsigned int res = 0;
        for( int j = 0; j < log_2_N; ++j ) {
            res = (res << 1) + (i & 1);
            i >>= 1;
        }

        return res;
    }

    ncFFTComplex ncFFT::t( unsigned int x, unsigned int N )
    {
        float coeff = nkMath::PI2 * x / N;
        return ncFFTComplex( cos(coeff ), sin( coeff ) );
    }
    

    /**
     *
     *  Matrix.
     *
     */
    
    /**
     *  Create billboard matrix.
     */
    void ncMatrix4::CreateBillboardMatrix( ncMatrix4 * out, const Vec3 & right, const Vec3 & up, const Vec3 & look, const Vec3 & pos )
    {
        out->m[0] = right.x;
        out->m[1] = right.y;
        out->m[2] = right.z;
        out->m[3] = 0.0f;
        out->m[4] = up.x;
        out->m[5] = up.y;
        out->m[6] = up.z;
        out->m[7] = 0.0f;
        out->m[8] = look.x;
        out->m[9] = look.y;
        out->m[10] = look.z;
        out->m[11] = 0.0f;
        // Translation.
        out->m[12] = pos.x;
        out->m[13] = pos.y;
        out->m[14] = pos.z;
        out->m[15] = 1.0f;
    }
    
    /**
     *  Create projectional perspective.
     */
    void ncMatrix4::CreatePerspective( float fovy, float aspect_ratio, float near_plane, float far_plane )
    {
        this->Identity();

        m[15] = 0;

        float y_scale = nkMath::Cot( (fovy * nkMath::ANG2RAD) / 2.0f );
        float x_scale = y_scale / aspect_ratio;
        float frustum_length = far_plane - near_plane;

        m[ 0] = x_scale;
        m[ 5] = y_scale;
        m[10] = -( (far_plane + near_plane) / frustum_length);
        m[11] = -1;
        m[14] = -( (2 * near_plane * far_plane) / frustum_length );
    }
    
    /**
     *  Make a point billboard.
     */
    void ncMatrix4::MakeBillboardPoint( ncMatrix4 * out, const Vec3 & pos, const Vec3 &camPos, const Vec3 & camUp )
    {
        Vec3 look;
        Vec3 right;
        Vec3 up;
        
        look = camPos - pos;
        look.Normalize();
        
        right = Vec3::CrossTwoVectors( camUp, look );
        up = Vec3::CrossTwoVectors( look, right );
        
        CreateBillboardMatrix( out, right, up, look, pos );
    }
    
    /**
     *  Create orthogonal perspective.
     */
    void ncMatrix4::CreateOrtho3D( float left, float right, float bottom, float top, float zNear, float zFar )
    {
        float sx = 2.0f / (right - left);
        float sy = 2.0f / (top - bottom);
        float sz = 2.0f / (zFar - zNear);
        
        float tx = -(right + left) / (right - left);
        float ty = -(top + bottom) / (top - bottom);
        float tz = -(zFar + zNear) / (zFar - zNear);
        
        //matrix is stored column major
        m[0] = sx,        m[4] = 0.0f, m[ 8] = 0.0f,  m[12] = tx;
        m[1] = 0.0f,  m[5] = sy,   m[ 9] = 0.0f,  m[13] = ty;
        m[2] = 0.0f,  m[6] = 0.0f, m[10] = sz,        m[14] = tz;
        m[3] = 0.0f,  m[7] = 0.0f, m[11] = 0.0f,  m[15] = 1.0f;
    }

}
