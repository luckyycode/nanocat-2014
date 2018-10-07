//
//  CWorldHandler.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 10/5/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef CWorldHandler_hpp
#define CWorldHandler_hpp

#include "LandscapeChunk.h"
#include "../../Core/Core.h"
#include "../../Core/Utilities/Noise.h"
#include "../../Core/Utilities/VectorList.h"

namespace Neko {
 
    /// Landscape chunk handler.
    class CWorldHandler
    {
        NEKO_NONCOPYABLE( CWorldHandler );

    public:
        
        /**
         *  Initialize chunk handler.
         */
        void                Initialize( INekoAllocator * allocator );
        
        /**
         *  Set noise parameters for the world creationism.
         */
        void                SetNoiseParameters( const NoisePerlinParams & largeNoise, const NoisePerlinParams & mediumNoise, const NoisePerlinParams & smallNoise );
        
        /**
         *  Create a first chunk.
         */
        void                Create();
        
        /**
         *  Destroy landscapes.
         */
        void                Destroy();
        
        /**
         *  Create a chunk.
         *
         *  @param sizeX        Chunk size by X.
         *  @param sizeZ        Chunk size by Z.
         *  @param worldMemory  Memory used for the world creationism.
         */
        void                CreateChunk( const int32_t sizeX, const int32_t sizeZ, const int32_t posX, const int32_t posZ, const Vec3 & chunkOrigin, SMemoryTempFrame * worldMemory );
        
        /**
         *  Render a landscape.
         *
         *  @param eye   Renderer viewport.
         *  @param flags Rendering options.
         */
        void                Render( CRenderer::ESceneEye eye, int32_t flags );

        /**
         *  Update chunk handler.
         */
        void                Update();
      
        /**
         *  Update chunks.
         */
        void                UpdateChunks();
        
        /**
         *  Create a new path base.
         */
        void                CreatePathNodeBase( const Vec2 & origin );
        
        /**
         *  Create a new path.
         */
        void                CreatePathNodeAt( const Vec2 & origin );
        
        /**
         *  Rebuild chunks.
         */
        bool                RebuildChunks();
        
        /**
         *  Invoked on chunk creation.
         */
        void                    OnChunkCreated( LandscapeChunk * chunk );
        
        /**
         *  Invoked on chunk removal.
         */
        void                    OnChunkDestroyed( LandscapeChunk * chunk );
        
        /**
         *  Update landscape object culling.
         */
        void                UpdateCulling();
        
        /**
         *  Draw objects with culling processed on GPU.
         *
         *  @param msec Milliseconds.
         */
        void            DrawGPUCulledObjects( uint32_t msec );
        
        /**
         *  Find the closest chunk where I am standing on.
         */
        LandscapeChunk  *                   FindNodeAt( const Vec3 & pos );
        
        CWorldHandler();
        ~CWorldHandler();
        
        /**
         *  Get height value from noise.
         *
         *  @param x X value.
         *  @param y Z value.
         *
         *  @return Height ( y ) value on the point.
         */
        float                   GetHeightPointOnNoise( int32_t x, int32_t y );
//        float                   GetHeightForWorldPos( float x, float y );
        
        /**
         *  Modify chunk vertex.
         */
        float                   GetModifierPoint( SEditVertices * surfaceVertices, int32_t vertexId, int32_t x, int32_t y );
        
        /**
         *  Create instanced objects.
         */
        void                CreateInstancedObjects();
        
        //!  Chunk build events.
        INekoThreadEvent *   m_chunkBuildEvent;
        //!  Chunk build thread.
        INekoThread *    m_chunkBuildThread;
        //!  Chunk build thread accessor.
        INekoThreadLock *    m_chunkBuildLock;

        //!  Chunk build list.
        CVectorList<LandscapeChunk *>  m_chunkBuildList;
        
        /**
         *  Is chunk handler active?
         */
        inline bool                 IsActive() const    {   return  m_bIsActive;    }
   
        /**
         *  Get noise for foliage.
         */
        inline const ncNoisePerlin  *               GetFoliageNoise() {       return m_noiseFoliage;  }
        
        //!  Landscape chunks.
        LandscapeChunk      * m_landscapeChunks[3][3];
        
        //!  Default chunk size.
        uint32_t    m_iChunkSize;

        enum CullMode {
            PASS_THROUGH = 0,					// pass through, no culling
            INSTANCE_CLOUD_REDUCTION = 1,		// instance cloud reduction
            HI_Z_OCCLUSION_CULL = 2,			// Hi-Z occlusion culling
        };
        
        //! Occlusion cull shader uniforms.
        enum ECullShaderUniforms {
            Offset = 0,
        };
        
        int32_t     m_iCullShaderUniforms[3];
        
        SGLShader               * m_pCullShader;
        
        struct Transform {
            ncMatrix4 ModelView;
            ncMatrix4 MVP;
            Vec4 Viewport;
        };
        
        Transform transform;
        
        GLuint      m_iMeshUniformBuffer;
        uint32_t    m_iMaxLOD;      // Configurable value.
        
        uint32_t m_iCullShaderSubIndexVS[3];				// cull vertex shader subroutine indices
        uint32_t m_iCullShaderSubIndexGS[2];				// cull geometry shader subroutine indices
        
        SMaterialProp   * m_pGroundMaterialArray;

        //! Update occlusion culling?
        bool        m_bUpdateGPUCulling;
//    private:

        INekoAllocator      * pStackAllocator = 0;
        
        INekoAllocator   * pAllocator = 0;
        INekoAllocator   * pAllocatorHandle = 0;
        INekoAllocator      * pChunkPoolAllocator = 0;
        
        //!  Procedural stuff.
        ncNoisePerlin       * m_noiseLarge;
        ncNoisePerlin       * m_noiseMed;
        ncNoisePerlin       * m_noiseType;
        ncNoisePerlin       * m_noisePath;      //! Paths & roads.
        ncNoisePerlin       * m_noiseFoliage;   //! Foliage noise.

        ///  Noise parameters.
        ///  NOTE: Must be the same value for all chunks!!
        
        float   fNoise_multiplier;        //! Noise map multiplier.
        float   fNoise_largeNoiseCoef;    //! Large noise type pixel size.
        float   fNoise_typeNoiseCoef;     //! Object noise type pixel size.
        float   fNoise_medNoiseCoef;
        
        //!  Max chunk height.
        float       m_fMaxHeight;
        
        //!  Can it build chunks?
        bool        m_bIsActive;
        
        
        //! Landscape shader.
        SGLShader  * m_pLandscapeShader;

        //! Landscape shader uniforms.
        enum WorldLandscapeUniforms {
            ModelViewProj  = 0,
            ModelMatrix,
            PreviousMatrix,
            Dummy
        };
        
        //! Terrain shader uniforms.
        uint32_t m_pShaderParams[4];
        
        /**
         *  Model view.
         */
        ncMatrix4   m_modelView;
        ncMatrix4   m_prevModelView;
        ncMatrix4   m_projectionModelView; // MVP

    };
}

#endif /* CWorldHandler_hpp */
