//
//  Mat4.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/10/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Mat4_h
#define Mat4_h

#include "Vec3.h"
#include "Vec4.h"

namespace Neko {
    
    /// 4x4 matrix.
    class ncMatrix4
    {
    public:
        float m[16];
        
        /**
         *  Create billboard matrix.
         */
        static void                 CreateBillboardMatrix( ncMatrix4 * out, const Vec3 & right, const Vec3 & up, const Vec3 & look, const Vec3 & pos );
        
        /**
         *  Make a point billboard.
         */
        static void                 MakeBillboardPoint( ncMatrix4 * out, const Vec3 & pos, const Vec3 &camPos, const Vec3 & camUp );
        
        ncMatrix4()
        {
            this->m[0] = 1.0f; this->m[4] = 0.0f; this->m[8] = 0.0f; this->m[12] = 0.0f;
            this->m[1] = 0.0f; this->m[5] = 1.0f; this->m[9] = 0.0f; this->m[13] = 0.0f;
            this->m[2] = 0.0f; this->m[6] = 0.0f; this->m[10] = 1.0f; this->m[14] = 0.0f;
            this->m[3] = 0.0f; this->m[7] = 0.0f; this->m[11] = 0.0f; this->m[15] = 1.0f;
        }
        
        ncMatrix4( float v1, float v2, float v3, float v4,
                  float v5, float v6, float v7, float v8,
                  float v9, float v10, float v11, float v12,
                  float v13, float v14, float v15, float v16 ) {
            this->m[0] = v1; this->m[1] = v2; this->m[2] = v3; this->m[3] = v4;
            this->m[4] = v5; this->m[5] = v6; this->m[6] = v7; this->m[7] = v8;
            this->m[8] = v9; this->m[9] = v10; this->m[10] = v11; this->m[11] = v12;
            this->m[12] = v13; this->m[13] = v14; this->m[14] = v15; this->m[15] = v16;
        }
        
        ncMatrix4( float vArray[16] )
        {
            for( unsigned int i = 0; i < 16; ++i ) {
                this->m[i] = vArray[i];
            }
        }
        
#   if !defined(NEKO_SERVER) && !defined(NEKO_NO_ASSIMP)
        ncMatrix4( const aiMatrix4x4t<float> & m )
        {
            this->m[0] = m.a1; this->m[1] = m.a2; this->m[2] = m.a3; this->m[3] = m.a4;
            this->m[4] = m.b1; this->m[5] = m.b2; this->m[6] = m.b3; this->m[7] = m.b4;
            this->m[8] = m.c1; this->m[9] = m.c2; this->m[10] = m.c3; this->m[11] = m.c4;
            this->m[12] = m.d1; this->m[13] = m.d2; this->m[14] = m.d3; this->m[15] = m.d4;
        }
        
        ncMatrix4( const aiMatrix3x3 & m )
        {
            this->m[0] = m.a1; this->m[1] = m.a2; this->m[2] = m.a3; this->m[3] = 0.0f;
            this->m[4] = m.b1; this->m[5] = m.b2; this->m[6] = m.b3; this->m[7] = 0.0f;
            this->m[8] = m.c1; this->m[9] = m.c2; this->m[10] = m.c3; this->m[11] = 0.0f;
            this->m[12] = 0.0f; this->m[13] = 0.0f; this->m[14] = 0.0f; this->m[15] = 1.0f;
        }
#   endif
        
        inline void MakeZero()
        {
            m[0] = 0.0f; m[4] = 0.0f; m[8] = 0.0f; m[12] = 0.0f;
            m[1] = 0.0f; m[5] = 0.0f; m[9] = 0.0f; m[13] = 0.0f;
            m[2] = 0.0f; m[6] = 0.0f; m[10] = 0.0f; m[14] = 0.0f;
            m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 0.0f;
        }
        
        inline void Identity( void )
        {
            m[0] = 1.0f; m[4] = 0.0f; m[8] = 0.0f; m[12] = 0.0f;
            m[1] = 0.0f; m[5] = 1.0f; m[9] = 0.0f; m[13] = 0.0f;
            m[2] = 0.0f; m[6] = 0.0f; m[10] = 1.0f; m[14] = 0.0f;
            m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
        }
        
        /**
         *  Translate matrix to origin.
         */
        inline      void Translate( const Vec3 &v );
        
        /**
         *  Translate matrix by X value.
         */
        inline void TranslateX( const float v )
        {
            //this->Identity();
            m[12] = v;
        }
        
        /**
         *  Translate matrix by Y value.
         */
        inline void TranslateY( const float v )
        {
            //this->Identity();
            m[13] = v;
        }
        
        /**
         *  Translate matrix by Z value.
         */
        inline void TranslateZ( const float v )
        {
            //this->Identity();
            m[14] = v;
        }
        
        /**
         *  Rotate matrix by axis.
         */
        void                Rotate( const float angle, Vec3 axis );
        
        /**
         *  Rotate matrix by X value.
         */
        void                RotateX( const float angle );
        /**
         *  Rotate matrix by Y value.
         */
        void                RotateY( const float angle );
        /**
         *  Rotate matrix by Z value.
         */
        void                RotateZ( const float angle );
        
        /**
         *  Scale matrix.
         */
        void                Scale( const Vec3 &scale );
        
        void                ScaleX( const float &scale );
        void                ScaleY( const float &scale );
        void                ScaleZ( const float &scale );
        
        /**
         *  Find matrix determinant.
         */
        float               Determinant( void ) const;
        //void Inverse( ncMatrix4 & ret );

        /**
         *  Transpose matrix.
         */
        void                Transpose( void );
        
#   if !defined(NEKO_SERVER) && !defined(NEKO_NO_ASSIMP)
        ncMatrix4 operator              = ( const aiMatrix4x4t<float> & v ) ;
#   endif
        ncMatrix4 operator              + ( const ncMatrix4& v ) const;
        ncMatrix4 operator              - ( const ncMatrix4& v ) const;
        ncMatrix4 operator              * ( const ncMatrix4& v ) const;
        ncMatrix4 operator              * ( const float f ) const;
        vec4_template<float> operator          * ( const vec4_template<float>&v );
        vec3_template<float> operator              * ( const vec3_template<float>&v ) const;
        
        inline float operator[] ( const int idx ) {
            return m[idx];
        }
        
        inline const float operator[] ( const int idx ) const {
            return m[idx];
        }
        
        
        friend vec4_template<float>            operator * (const ncMatrix4 &Matrix, const vec4_template<float> &u);
        
        
        // View stuff.
        void                CreatePerspective( float fovy, float aspect_ratio, float near_plane, float far_plane );
        void                CreateOrtho3D( float xMin, float xMax, float yMin, float yMax, float zMin, float zMax );
        
        /**
         *  Make matrix to look at desired origin.
         */
        inline static ncMatrix4 LookAt( const Vec3 &eye, const Vec3 &center, const Vec3 &up )
        {
            Vec3 x, y, z;
            ncMatrix4 M;
            
            // make rotation matrix
            
            // Z vector
            z.x = eye.x - center.x;
            z.y = eye.y - center.y;
            z.z = eye.z - center.z;
            z = Vec3::Normalize(z);
            
            // Y vector
            y.x = up.x;
            y.y = up.y;
            y.z = up.z;
            
            // X vector = Y cross Z
            x = Vec3::CrossTwoVectors(y,z);
            
            // Recompute Y = Z cross X
            y = Vec3::CrossTwoVectors(z,x);
            
            // cross product gives area of parallelogram, which is < 1.0 for
            // non-perpendicular unit-length vectors; so normalize x, y here
            x = Vec3::Normalize(x);
            y = Vec3::Normalize(y);
            
            M.m[0] = x.x; M.m[4] = x.y; M.m[8] = x.z; M.m[12] = -x.x * eye.x - x.y * eye.y - x.z*eye.z;
            M.m[1] = y.x; M.m[5] = y.y; M.m[9] = y.z; M.m[13] = -y.x * eye.x - y.y * eye.y - y.z*eye.z;
            M.m[2] = z.x; M.m[6] = z.y; M.m[10] = z.z; M.m[14] = -z.x * eye.x - z.y * eye.y - z.z*eye.z;
            M.m[3] = 0.0; M.m[7] = 0.0; M.m[11] = 0.0; M.m[15] = 1.0;
            return M;
        }
        
        /**
         *  Get Bias matrix.
         */
        inline static const ncMatrix4 GetBiasMatrix()
        {
            // Stuff.
            ncMatrix4 BiasMatrix = ncMatrix4( 0.5f, 0.0f, 0.0f, 0.0f,
                                             0.0f, 0.5f, 0.0f, 0.0f,
                                             0.0f, 0.0f, 0.5f, 0.0f,
                                             0.5f, 0.5f, 0.5f, 1.0f );
            return BiasMatrix;
        }
        
        /**
         *  Get inversed Bias matrix.
         */
        inline static const ncMatrix4 GetBiasMatrixInverse()
        {
            ncMatrix4 BiasMatrixInverse = ncMatrix4( 2.0f, 0.0f, 0.0f, 0.0f,
                                                    0.0f, 2.0f, 0.0f, 0.0f,
                                                    0.0f, 0.0f, 2.0f, 0.0f,
                                                    -1.0f, -1.0f, -1.0f, 1.0f );
            return BiasMatrixInverse;
        }
        
        /**
         *  Rotate matrix by axis.
         */
        inline static ncMatrix4 Rotate2( const float angle, const Vec3 &u );
    };
    
    /**
     *  Translate the matrix.
     */
    inline void ncMatrix4::Translate( const Vec3 &v )
    {
        //this->Identity();
        
        m[12] = v.x;
        m[13] = v.y;
        m[14] = v.z;
    }
}

#endif /* Mat4_h */
