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
//  Ocean.h
//  Beautiful ocean renderer.
//
//  Created by Neko Code on 11/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __neko_ocean_h__
#define __neko_ocean_h__

#include "../Graphics/OpenGL/GLShader.h"
#include "../Math/BoundingBox.h"
#include "../Math/GameMath.h"
#include "../Platform/Shared/SystemShared.h"

// CPU culled ocean.

#if !defined( NEKO_SERVER )

namespace Neko {
    
    /**
     *  Ocean animation types.
     */
    enum OceanGenType
    {
        CPU_OCEAN = 0,
        GPU_OCEAN
    };
    
    struct CWorldOceanFFTComplex
    {
        // Used with discrete fourier transform.
        ncFFTComplex h;	        // Wave height.
        
        Vec2   D;		        // Displacement.
        Vec3   n;		        // Normal.
    };
    
#define WTIMEDENSITYCLIP_UNIFORM 0
#define WMODELMATRIX_UNIFORM 1
#define WMVP_UNIFORM 2
#define WLIGHTPOSITION_UNIFORM 3
#define WCAMERAPOSITION_UNIFORM 4
#define WWATERCOLOR_UNIFORM 5
#define WLIGHTCOLOR_UNIFORM 6
    
    ///   Beautiful ocean.
    class CWorldOcean
    {
        NEKO_NONCOPYABLE( CWorldOcean );
        
    public:
        //!  Gravity constant.
        float   oGravity;
        
        //!  Dimension. N should be a power of 2.
        int32_t     N, Nplus1, N2, Nplus1N;
        
        //!  Phillips spectrum parameter. Affects heights of waves.
        float   A;
        
        //!  Length constant.
        float   Length;
        
        //!  Wind direction.
        Vec2   oWind;
        
        //!  Complex stuff for fast fourier transform.
        ncFFTComplex    *h_tilde, *h_tilde_slopex, *h_tilde_slopez, *h_tilde_dx, *h_tilde_dz;
        
        //!  Fast fourier transform.
        ncFFT               *fft;
        
        //!  Vertices for vertex buffer object.
        Vec3       * vertices;
        
        //!  Ocean normals.
        Vec3       * normals;
        
        Vec3 * hTilde0;
        Vec3 * hTilde0mk;
        
        Vec3 * firstOrigin;
        
        //!  Texture coordinates.
        Vec2       *uvCoords;

        //!  Vertex buffer objects.
        GPUBuffer   vbo_vertices, vbo_uv, vbo_normals, vbo_indices;
        GLuint      vao_buffer;
        
        //!  Ocean water shader.
        SGLShader  *oShader;
        
        //!  Ocean bounding box.
        SBoundingBox   m_BoundingBox;
        
        //!  Shader uniforms.
        uint32_t    oShaderUniforms[7];
        
        //!  Ocean normal map and foam textures.
        SMaterialProp  * oNormalMap, * oSeaFoam;
 
        //!  Quadtree used to create Level of Detail and Visible planes.
        CQuadtree  * m_pQuadtree;
    
        //!  Water color.
        Vec4i   m_waterColor;
        
        //!  Model view.
        ncMatrix4   m_modelViewPos;
        
        bool    bRenderOcean;    // Render ocean?
        bool    bHasAnimation;
        
        //!  Ocean animation type.
        OceanGenType    oceanType;
        
        inline float                Dispersion( int n_prime, int m_prime );		// Deep water.
        inline float                PhillipsSpectrum( int n_prime, int m_prime );		    // Phillips spectrum.
        
        inline ncFFTComplex                 hTilde_0( int n_prime, int m_prime );
        inline ncFFTComplex                 hTilde( float t, int n_prime, int m_prime );

        INekoAllocator      * pAllocator = 0;
        INekoAllocator      * pAllocatorHandle = 0;
        
    protected:
    public:
        /**
         *  Constructor.
         */
        CWorldOcean( SBoundingBox& boundingBox, const int N, const float A, const Vec2 & w, const float length,
                bool hasAnimation, OceanGenType genType );
        ~CWorldOcean();
        
        /**
         *  Default constructor.
         */
        CWorldOcean();
        
        /**
         *  Create ocean surface.
         */
        void                Create( SBoundingBox& boundingBox, const int N, const float A, const Vec2 & w, const float length,
                                            bool hasAnimation, OceanGenType genType );
        
        /**
         *  Check for visible vertices.
         */
        inline bool                 IsOceanVerticeVisible( int index2 );
        
        /**
         *  Set water color.
         */
        void                SetWaterColor( const float & _r, const float & _g, const float & _b, const float & _a );
        
        /**
         *    Remove ocean.
         */
        void                Release();
        
        /**
         *  Refresh ocean.
         */
        void                Refresh();
        
        /**
         *  Visible faces.
         */
        int  mFacesDrawn;
        const uint32_t              GetFacesDrawn() const {      return mFacesDrawn; }
        
        /**
         *  Generate ocean waves!
         */
        void                EvaluateWaves( float t );
        void                EvaluateWavesFFT( float t );
        
        CWorldOceanFFTComplex               h_D_and_n(Vec2 x, float t);
        
        /**
         *  As it says, remove FFT stuff.
         */
        void                RemoveFFTStuff();

        /**
         *  Draw ocean!
         */
        void                    Render();
        
        /**
         *  Create buffers and init some stuff.
         */
        void                Create();
        
        
        /**
         *  Set parameters.
         */
        void                SetParameters();
        
        //ncMatrix4           m_modelViewPos;
        //ncMatrix4           projectionModelView;
    };
}
#endif // NEKO_SERVER

#endif // __ocean_h__
