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
//  Renderer.h
//  Main game renderer.. ^.^
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef renderer_h
#define renderer_h

#include "../../Core/Core.h"
#include "../OpenGL/GLShader.h"
#include "../Framebuffer.h"
#include "../../AssetCommon/Material/MaterialLoader.h"
#include "../../World/MeshBase.h"
#include "../../Core/EventHandler.h"
#include "../../Math/Vec3.h"
#include "../../Math/Mat4.h"
#include "../../Core/Queue.h"

#ifndef NEKO_SERVER

#   if defined(USES_METAL) && !defined(USES_OPENGL)
// External Metal header as accessor to its API.
#include "../Platform/Apple/OSX/MetalExternal.h"  // Why the hell can't I use Metal API in C without external stuff???!?!

#   elif defined(USES_OPENGL)
#define GL_INDEX_TYPE GL_UNSIGNED_INT

#   endif

#include "../RendererMisc.h"    // GPUBuffer, BasicSampler and another enums.

#define UNIQUE

namespace Neko {
    class Metal;
    
    const static uint32_t   MAX_RENDER_QUEUES = 32;
    
    /// Depth test functions.
    enum class DepthFunc : uint32_t
    {
        Never,
        Less,
        LessOrEqual,
        Equal,
        GreaterOrEqual,
        Greater,
        Always,
        
        Dummy   // max
    };
    
    /// Color masks.
    enum ColorMask : uint32_t
    {
        None    = 0x00,
        Red     = 0x01,
        Green   = 0x02,
        Blue    = 0x04,
        Alpha   = 0x08,
        
        RGB     = Red | Green | Blue,
        RGBA    = RGB | Alpha,
        
        Dummy,  // max
    };
    
    /// Default values.
    enum StaticGraphicsInfo : int32_t
    {
        MaxTextureTargets = static_cast<uint32_t>(TextureTarget::Dummy),
        MaxTextureFormats = static_cast<uint32_t>(ETextureFormat::Dummy),
        MaxCompareModes = static_cast<uint32_t>(ECompareMode::Dummy),
        MaxShaderTypes = static_cast<uint32_t>(EShaderType::Dummy),
        MaxRenderTargetsHW = static_cast<uint32_t>(ERenderTarget::Dummy),
        MaxCullModes = static_cast<uint32_t>(ECullMode::Dummy),
        MaxBlendModes = static_cast<uint32_t>(EBlendMode::Dummy),
    };
    
    class CEyeAdaptationPost;
    class CTemporalAAPost;
    
    ///  Main game renderer.
    class CRenderer
    {
        NEKO_NONCOPYABLE( CRenderer );
        
    public:
        
        ///  Scene eye ( viewports ).
        enum ESceneEye {
            EYE_FULL    = 0,
            EYE_LEFT    = 1,
            EYE_RIGHT   = 2
        };

        /// Renderer states.
        struct RenderState
        {
        public:
            
            //! Current bound texture.
            uint32_t currentTextureUnit = 0;
            
            //! Current framebuffer.
            uint32_t currentFramebuffer = 0;
            uint32_t currentReadFramebuffer = 0;
            uint32_t currentDrawFramebuffer = 0;
            
            //! Current array buffer.
            uint32_t currentArrayBuffer = 0;
            
            //! Current index buffer.
            uint32_t currentIndexBuffer = 0;
            
            //! Current shader program.
            uint32_t currentShader = 0;
            
            size_t colorMask = 0;
            float clearDepth = false;
            
            bool clipEnabled = false;
            bool cullEnabled = false;
            bool blendEnabled = false;
            bool depthTestEnabled = false;
            bool depthMask = false;
            bool wireframe = false;
            bool alphaToCoverage = false;
            //
            //            //! Last alpha blend mode.
            //            EBlendMode lastAlphaBlend = EBlendMode::Current;
            //
            //            //! Last culling mode.
            //            ECullMode lastCull = ECullMode::Current;
            
            //! Last depth function.
            DepthFunc lastDepthFunc = DepthFunc::Less;
            //            
            //            //! Last blend mode.
            //            EBlendMode lastColorBlend = EBlendMode::Current;
        };
        
        
        /**
         *  Renderer instance.
         */
        CRenderer();
        ~CRenderer();
        
        /**
         *  Show some information.
         */
        void                RenderDebugUI();
        
        /**
         *  Initialize renderer.
         */
        void                Initialize( GraphicsInterface    * interface );
        
        /**
         *  Update framebuffer object.
         */
        void                UpdateFramebufferObject( const uint32_t w, const uint32_t h );
        
        /**
         *  Adjust element resolution.
         */
        void                AdjustElemSize( float * x, float * y );
        void                AdjustElemSize( float * x, float * y, float * w, float * h );
        
        /**
         *  Refresh renderer ( & update settings ).
         */
        void                Refresh( void );
        
        /**
         *  Remove any rendered thing.
         */
        void                RemoveWorld( const char * msg );
        
        /**
         *  Make screenshot.
         */
        void                MakeScreenshot( void );
        
        /**
         *  Render EVERYTHING!!11
         */
        void                Render( uint32_t msec );
        
        /**
         *  Pre-render ( i.e. update camera, frustum etc ).
         */
        void                PreRender( uint32_t msec );
        
        /**
         *  Preload renderer.
         */
        void                Preload( void );
        
        /**
         *  Shutdown teh renderer.
         */
        void                Shutdown( void );
        
        /**
         *  Delete main eye scene vertex buffers.
         */
        void                DeleteMainBuffers( void );
        
        /**
         *  Draw simple 2d quad.
         */
        void                RenderQuadAt( Vec4  rect, Vec4 & color );
        
        /**
         *  Get intersection point in the world space.
         */
        void                IntersectionPoint( Vec3 & Intersection );
        
        /**
         *  Switches blending feature.
         */
        inline void                 SetBlending( bool enable, EBlendMode color, EBlendMode alpha );
        
        /**
         *  Set viewport size.
         */
        inline void                 SetViewportSize( int32_t w, int32_t h );
        
        /**
         *  Set viewport size to initial state.
         */
        inline void                 ResetViewport() {
            SetViewportSize( renderWidth, renderHeight );
        }
        
        /**
         *  Dequeue renderer calls.
         */
        SFunctionCallInfo                   * DequeueRequests();
        
        /**
         *  Set depth write mode.
         */
        void                SetDepthWriteMode( const bool write );
        
        //! Set depth enabled/disabled.
        
        /**
         *  Toggle depth testing.
         */
        inline void                 SetDepthTest( bool enable );
        
        /**
         *  Toggle depth mode.
         */
        inline void                 SetDepthMode( ECompareMode mode );
        
        /**
         *  Toggle face culling.
         */
        inline void                 SetCulling( bool enable, ECullMode cull );
        
        //! Color clear operations.
        
        /**
         *  Clear screen depth.
         */
        inline void                 ClearDepth();
        
        /**
         *  Clear screen color.
         */
        inline void                 ClearColor();
        
        /**
         *  Clear screen color with depth.
         */
        inline void                 ClearColorDepth();
        
        /**
         *  Clear screen with a color.
         */
        inline void                 ClearWithColor( const float &r, const float &g, const float &b, const float &a );
        
        /**
         *  Clear screen color with a depth with color.
         */
        void                    ClearColorDepthWithColor( const float &r, const float &g, const float &b, const float &a );
        
        
        // Color target operations.
        
        
        /**
         *  Set color target to draw.
         */
        inline void                 SetRenderTarget( ERenderTarget ctIdx );
        
        // GPU buffer operations.
        
        /// ============================================================================================
        /**
         *  Allocate new GPU buffer.
         */
        GPUBuffer                AllocGPUBuffer( uint32_t Size,
                                                EBufferStorageType Type,
                                                EBufferType Usage,
                                                EPrimitiveType Mode = EPrimitiveType::Triangles,
                                                uint32_t Complexity = 0 );
        
        /**
         *  Sanity check.
         */
        void                FinishBuffer( GPUBuffer *Buffer, unsigned int Offset, unsigned int Count );
        
        /**
         *  Use that to prevent console spamming.
         */
        void                AllowBufferLogging( const bool allow );
        
        /**
         *  Update GPU buffer data.
         */
        void                BufferData( GPUBuffer * Buffer, const void * Data, uint32_t Offset, uint32_t Size );
        
        /**
         *  Set GPU buffer vertex pointer.
         */
        void                BufferPointer( GPUBuffer * Buffer, uint32_t Index, uint32_t Size, uint32_t Type, bool Normalized, uint32_t Stride, const void * Pointer );
        void                BufferIPointer( GPUBuffer * Buffer, uint32_t Index, uint32_t Size, uint32_t Type, uint32_t Stride, const void * Pointer );
        
        /**
         *  Delete GPU buffer.
         */
        void                DeleteGPUBuffer( GPUBuffer * Buffer );
        
        /**
         *  Total GPU buffers allocated.
         */
        uint32_t    m_iGPUBuffersAllocated;
        
        /**
         *  Bind a GPU buffer.
         *
         *  @param Buffer GPU buffer to bind.
         */
        inline void             BindBuffer( GPUBuffer * Buffer ) const;
        
        /// ============================================================================================
        
        /**
         *  Bind texture.
         */
        inline void                 BindTexture( uint32_t Index, uint32_t TexId, TextureTarget Type = TextureTarget::Texture2D );
        
        /**
         *  Unbind texture.
         */
        inline void                 UnbindTexture( uint32_t Index );
        
        /**
         *  Check if we have errors in the graphical interface.
         */
        static void                 CheckAPIErrors( const char * function, const int32_t line );
        
        
        /**
         *  Create OpenGL vertex array.
         */
        const GLuint                CreateVertexArray();
        
        /**
         *  Unbind vertex arrays.
         */
        inline void                 UnbindVertexArray();
        
        
        /**
         *  Prepare framebuffers.
         */
        void                    PrepareFramebuffers( uint32_t msec );
        
        /**
         *  Render to shaders.
         */
        void                    RenderToShader( uint32_t time, ESceneEye eye );
        
        /**
         *  Precache renderer.
         */
        void                Precache( void );
        
        /**
         *  Read pixels using current graphics API.
         */
        void                ReadPixel( int32_t x, int32_t y, float & z );
        
        /**
         *  Get scene postprocess shader.
         */
        inline SGLShader            * GetSceneShader()     {   return sceneShader;     }
        
        
        //! Current allocator is used to allocate subclasses.
        INekoAllocator      * pAllocator;
        
        //! Single screenpass mesh for geometry shader.
        CBasicMesh *   m_pScreenSinglePass;
        
        //!  Renderer time.
        uint32_t    Time;
        
        //! Renderer states.
        RenderState     currentState;
        
        /// ============================================================================================
        //! Renderer method call for queue.
        typedef void (*RendererCall)( void * );
        
        //! Renderer thread tasks to perform.
        CQueue   * m_pRenderQueue;
        
        //! Thread events lock.
        INekoThreadLock  * m_pRenderLock;
        
        /**
         *  Perform on the renderer thread.
         *
         *  @note - Not queued!
         */
        inline void Perform( FunctionPtr task, void * arg )
        {
            SFunctionCallInfo * info = new SFunctionCallInfo();
            
            info->m_bActive = false;
            info->m_pArg = arg;
            info->m_pFunction = task;
            
            m_pRenderQueue->Put( (void*)info );
        }
        
        /// ============================================================================================
        /// Font manager.
        class CCoreFontRenderer * p_CoreFont;
        
        //! Currently used graphical API.
        GraphicsInterface       * m_pGraphicsInterface;
        
        
        // Framebuffers.
        CFramebuffer  * g_waterReflectionBuffer;  //! Water( planar ) reflection pass.
        CFramebuffer  * g_sceneBuffer[3];         //! Scene render pass.
        CFramebuffer  * g_compositeBuffer;        //! Final pass.
        CFramebuffer  * g_miscCompositeBuffer;        //! Final pass with dedicated passes.
        
        CFramebuffer    * g_pMiscFramebuffer;   //! framebuffer used temporary to initialize some material presets
  
        CFramebuffer    * g_pFrameHistoryBuffer;
        CFramebuffer    * g_pCopyBuffer; //! frame history
        
        CFramebuffer    * m_pPhaseLutTarget, * m_pLightLutTarget;
        
        SMaterial       * m_pFrameHistory[5];
        
        CFramebuffer    * g_pUIBuffer = 0;
        
        //! Textures for framebuffer switching.
        SMaterial    * m_pCompositeTex , * m_pCompositeTex2;
        
        //!  Frame per msec.
        uint32_t        frameMsec;
        
        // Some settings.
        uint32_t        renderWidth;    //! Viewport width.
        uint32_t        renderHeight;   //! Viewport height.
        
        uint32_t     particleOffscreenWidth;
        uint32_t     particleOffscreenHeight;
        
        uint32_t     pointlightOffscreenWidth;
        uint32_t     pointlightOffscreenHeight;
        
        uint32_t    renderHalfWidth;
        uint32_t    renderHalfHeight;
        
        uint32_t    windowWidth;
        uint32_t    windowHeight;
        
        int32_t     m_iWindowX;
        int32_t     m_iWindowY;
        
        float       renderAspectWidth, renderAspectHeight;
        float       renderAspectRatio;
        
        uint32_t     displayWidth;
        uint32_t     displayHeight;
        
        uint8_t     m_bRequiresPrecision:1; // ES
        
        uint8_t     m_bUseUniformBuffers:1;
        
        //!  Renderer state.
        bool Initialized;
        
        //!  'true' if renderer is in HDR mode.
        bool    m_bIsHDR;
        
        /// User interface.
        class UserInterface * p_UserInterface;
        
        //    private:
        
        bool bAllowBufferLogging; //! GPU buffer creation logging.
        
        /// ============================================================================================
        //! Renderer uniforms.
        enum RenderDeferredUniforms
        {
            // Deferred shader uniforms.
            RLightPosition_Time = 0,
            RGlobalFogColor,
            RSkyColor,
            RLight2DPosition,
            RSunColor,
            RSunWorldPos,
            RSunLuminance,
            RCameraPosition,
            RInvertedViewProj,
            RPreviousViewProj,
            // Draw2d shader uniforms.
            RDraw2D_ScaleOrigin,
            RDraw2D_Color,
            //  Composite uniforms.
            RInvertedViewProj2,
            // Hi-Z uniforms.
            RLastMipSize,
            // Bloom shader uniforms.
            RBlurSize,
            // Texel size (bloom).
            RTexelSize,
        };

        uint32_t    _renderUniforms[24];

        // Renderer common stuff.
        // Just leave it here.
        // Scene 2d filter.
        SGLShader  * sceneShader;       //! First composite.
        SGLShader  * compositeShader;   //! Final composite.
        SGLShader  * compositeShader2;   //! Final composite.
        SGLShader  * compositeShader3;   //! Final composite.
        
        SGLShader  * draw2dshader;      //! 2d drawing.
        SGLShader   * hiZshader;        //! Hi-Z cull shader.
        SGLShader   * downsampleShader; //! Downsampling and blurring shader.
        //        SGLShader   * cullShader;       //! Culling technology shader.
        
        SGLShader       * m_pBrdfShader = 0;
        
        SGLShader           * m_pMiscShader, * m_pCopyShader2;
        
        SGLShader           * m_pComputePhaseLut;
        SGLShader           * m_pComputeLightLut;
        
        SGLShader           * m_pVolumetricLight;
        
        uint32_t            m_iMiscUniforms[4];
        
        // Downsample shader routines.
        uint32_t        m_iDSRoutine[4];
        
        uint32_t        m_iBlurShaderRoutines[2];   // Vertical/Horizontal
        
        // @todo: use IPostProcessFX
        
        //! Histogram-based eye adaptation to light.
        CEyeAdaptationPost  * m_pEyeAdaptationPost;
        //! Screenspace Temporal Anti-aliasing.
        CTemporalAAPost     * m_pTemporalAAPost;
        
        //! Bloom spread.
        float               m_fBlurSize ; //! Blur size.
        float               m_fBloomIntensity ;    //! 0.0f..0.5f
        float               m_fLensDirtIntensity ;   //! 0.0f..0.95f
        
        //! Shader uniforms.
        //! Used for main 2d screen filter.
        //! Note/Todo: I should make something like dictionary.
        
        //! ------------
        
        //! Quad mesh for 2d rendering.
        CBasicMesh  * g_pEyeMesh[3];
        
        // bool    m_updatePostProcess;
        
        // Precached materials.
        
        //!  Lens texture.
        SMaterialProp               *_lens_texture;
        SMaterialProp               *m_pLutSampler; //! HaarmPeterDuikerFilmicToneMapping
        
    };
    
    /// Post-processing class interface.
    UNIQUE class IPostProcess {
        
        NEKO_NONCOPYABLE( IPostProcess );
        
    public:
        
        /**
         *  Initialize post-process.
         *
         *  @param renderer Active renderer interface.
         *  @note   Called on renderer preload.
         */
        void                Init( CRenderer * renderer );
        
        /**
         *  Apply post-process.
         */
        void                Render();
        
        /**
         *  Update. Called once!
         */
        void                Update();
        
        /**
         *  Destroy current post-process.
         */
        void                Destroy();
        
    private:
        
        //! Is current postprocess subroutine active?
        bool        m_bActive;
    };

    /// Temporal anti-aliasing post-process.
    class CTemporalAAPost
    {
    public:
        
        void            Init( CRenderer * renderer );
        
        void            Render();
        
        void            Update();
        
        void            Disable();
        
        
        int32_t     m_iReprojectionIndex;
        
        ncMatrix4           m_reprojectionMatrix[2];    // history matrix
        
        CFramebuffer    * g_pReprojectionBuffer; //! frame history
        SGLShader       * m_pShader;
        
        CRenderer       * m_pRenderer;
        
        enum ShaderUniforms
        {
            JitterOffset = 0,
            Time,
        };
        
        uint32_t    m_iUniforms[8];
        
        //! AA filtering.
        enum ETemporalFilter {
            MinMax3x3,
            MinMax3x3Rounded,
            MinMax4TapVarying,
        };
        
        ETemporalFilter    m_eFiltering;
        bool    m_bUnjitterColorSamples;
        bool    m_bUnjitterNeighborhood;
        bool    m_bUnjitterReprojection;
        bool    m_bUseYCoCg;
        bool    m_bUseClipping;
        bool    m_bUseDilation;
        bool    m_bUseMotionBlur;
        bool    m_bUseOptimizations;
        
        float   m_fFeedbackMin = 0.88f; // (0f, 1f)
        float   m_fFeedbackMax = 0.97f; // (0f, 1f)
        
        float   m_fMotionBlurStrength = 0.25f;
        bool    m_bMotionBlurIgnoreFF = false;
        
    };
    
    /// Eye adaptation post-process.
    class CEyeAdaptationPost
    {
        NEKO_NONCOPYABLE( CEyeAdaptationPost );
        
    public:
        
        /**
         *  Initialize postprocess.
         */
        void                Init( bool lowResolution, CRenderer * renderer );
        
        /**
         *  Render postprocess.
         */
        void                Render( uint32_t frameMsec );
        
        /**
         *  Disable current postprocess pass.
         */
        void                Disable();
        
        void                Update();
        
        
        //! Renderer handle.
        CRenderer       *   pRenderer;
        
        //! Postprocessing shader.
        SGLShader       *   m_pShader;
        
        int32_t         m_iMeshCount;   // Sprite 2d mesh.
        int32_t         m_iHistogramSizeX, m_iHistogramSizeY;
        CBasicMesh      ** m_pMeshes;
        
        
        //! Shader uniforms.
        enum HistogramShaderUniforms
        {
            // Histogram analysis subroutine
            ExposureOffset = 0,
            MinMaxSpeedTime,
            Pixel9098Count,
            Coefs,
            
            // Histogram creationism subroutine
            Range,
            Size,
            Count
        };
        
        uint32_t        m_iHistogramSR[6];  // Histogram shader subroutines.
        uint32_t        m_iUniforms[8];
        
        //! Holds the current and previous 1x1 render targets.
        CFramebuffer    * g_pBrightnessRT, * g_pBrightnessRTTemp;
        CFramebuffer    * g_pHistogramFramebuffer;  //! Histogram framebuffer data.
       
        // Settings..
        
        int32_t     m_iMipLevel;
        
        float   m_fMinimumExposure;
        float   m_fMaximumExposure;
        
        float   m_fRange;
        
        float   m_fAdaptationSpeedUp;
        float   m_fAdaptationSpeedDown;
        
        float   m_fBrightnessMultiplier;
        
        float   m_fExposureOffset;
        float   m_fAverageThresholdMin;
        float   m_fAverageThresholdMax;
        
        float   m_fHistLogMin;
        float   m_fHistLogMax;
        
        float m_fHistMin;
        
        Vec4 m_vHistCoefs;
        
        float m_fHistogramSize;
    };
    
    /**
     *  Set color target to draw.
     */
    inline void CRenderer::SetRenderTarget(ERenderTarget ctIdx)
    {
        uint32_t target = m_pGraphicsInterface->GetAPIRenderTarget( ctIdx );
        
#   if defined( USES_OPENGL )
        glDrawBuffer( (GLenum)target );
#   else
        
#   endif
    }
    
    /**
     *  Set viewport size.
     */
    inline void CRenderer::SetViewportSize( int32_t w, int32_t h )
    {
#ifdef USES_OPENGL
        glViewport( 0, 0, w, h );
#else
        
#endif
    }
    
    /**
     *  Clear screen color.
     */
    inline void CRenderer::ClearDepth()
    {
#ifdef USES_OPENGL
        //        glClear ( GL_DEPTH_BUFFER_BIT );
        
        // Clear render target.
        const GLfloat one = 1.0f;
        glClearBufferfv(GL_DEPTH, 0, &one);
        
#else
        
#endif
    }
    
    /**
     *  Toggle depth testing.
     */
    inline void CRenderer::SetDepthTest( bool enable )
    {
        if ((enable != currentState.depthTestEnabled))
        {
            currentState.depthTestEnabled = enable;
            
#ifdef USES_OPENGL
            (enable ? glEnable : glDisable)(GL_DEPTH_TEST);
#else
            
#endif
        }
    }
    
    /**
     *  Toggle depth mode.
     */
    inline void CRenderer::SetDepthMode( ECompareMode mode )
    {
#   ifdef USES_OPENGL
        GLuint result = m_pGraphicsInterface->GetAPICompareMode( mode );
        glDepthFunc( result );
#   else
        
#   endif
    }
    
    /**
     *  Toggle faceculling.
     */
    inline void CRenderer::SetCulling(bool enable, ECullMode cull)
    {
        if ((currentState.cullEnabled != enable)) {
            currentState.cullEnabled = enable;
            
            (enable ? glEnable : glDisable)(GL_CULL_FACE);
        }
        
        glCullFace( cull == ECullMode::Back ? GL_BACK : GL_FRONT );
    }
    
    /**
     *  Toggle blend mode.
     */
    inline void CRenderer::SetBlending( bool enable, EBlendMode color, EBlendMode alpha )
    {
        if ((currentState.blendEnabled != enable)) {
            currentState.blendEnabled = enable;
            
            (enable ? glEnable : glDisable)(GL_BLEND);
        }
        
        uint32_t * colorModePair = m_pGraphicsInterface->GetAPIBlendMode( color );
        uint32_t * alphaModePair = m_pGraphicsInterface->GetAPIBlendMode( alpha );
        
#   if defined( USES_OPENGL )
        glBlendFuncSeparate( colorModePair[0], colorModePair[1], alphaModePair[0], alphaModePair[1] );
#   else
        
#   endif
    }
    
    /**
     *  Bind texture.
     */
    inline void CRenderer::BindTexture( uint32_t Index, uint32_t TexId, TextureTarget Type )
    {
        const uint32_t type = m_pGraphicsInterface->GetAPITextureTarget( Type );
        
#   if defined( USES_OPENGL )
        glActiveTexture( GL_TEXTURE0 + Index );
        glBindTexture( (GLenum)type, TexId );
#   endif
    }
    
    /**
     *  Unbind texture.
     */
    inline void CRenderer::UnbindTexture( uint32_t Index )
    {
#   if defined( USES_OPENGL )
        glActiveTexture( GL_TEXTURE0 + Index );
        glBindTexture( GL_TEXTURE_2D, 0 );
#   endif
    }
    
    
    /**
     *  Bind a GPU buffer.
     *
     *  @param Buffer GPU buffer to bind.
     */
    inline void CRenderer::BindBuffer( GPUBuffer * Buffer ) const
    {
#   if defined( USES_OPENGL )
        glBindBuffer( GL_ARRAY_BUFFER, Buffer->Handle );
#   else
        
#   endif
    }
    
    /**
     *  Unbind vertex arrays.
     */
    inline void CRenderer::UnbindVertexArray()
    {
#   if defined( USES_OPENGL )
        glBindVertexArray( 0 );
#   else
        
#   endif
    }
    
    /**
     *  Clear screen with a color.
     */
    inline void CRenderer::ClearWithColor( const float &r, const float &g, const float &b, const float &a )
    {
#   if defined( USES_OPENGL )
        glClear ( GL_COLOR_BUFFER_BIT );
        glClearColor( r, g, b, a );
#   else
        
#   endif
    }
    
    
    
    
    //! Maximum number of phase terms in a medium
    const uint32_t MAX_PHASE_TERMS = 4;
    

    
    //! Describes one component of the phase function
    struct PhaseTerm
    {
        PhaseFunctionType ePhaseFunc;	//!< Phase function this term uses
        Vec3 vDensity;			    //!< Optical density in [R,G,B]
        float fEccentricity;		    //!< Degree/direction of anisotropy (-1, 1) (HG only)
    };
    
    
    //! Volume Medium Description
    struct MediumDesc
    {
        Vec3 vAbsorption;		//!< Absorpsive component of the medium
        uint32_t uNumPhaseTerms;    //!< Number of valid phase terms
        
        //! Phase term definitions
        PhaseTerm PhaseTerms[MAX_PHASE_TERMS];
        
    };
    
    
    //! Parameters for Volume Generation
    struct VolumeDesc
    {
        float fTargetRayResolution;         //!< Target minimum ray width in pixels
        uint32_t uMaxMeshResolution;        //!< Maximum geometric resolution of the mesh. Accounts for requested tessellation quality.
        float fDepthBias;			        //!< Amount to bias ray geometry depth
        ETessellationQuality eTessQuality;   //!< Quality level of tessellation to use
    };

    
    
    //! Post-Processing Behavior Description
    struct PostprocessDesc
    {
        ncMatrix4 mUnjitteredViewProj;		//!< Camera view projection without jitter
        float fTemporalFactor;				//!< Weight of pixel history smoothing (0.0 for off)
        float fFilterThreshold;				//!< Threshold of frame movement to use temporal history
        UpsampleQuality eUpsampleQuality;	//!< Quality of upsampling to use
        Vec3 vFogLight;                  //!< Light to use as "faked" multiscattering
        float fMultiscatter;                //<! strength of faked multiscatter effect
        bool bDoFog;						//!< Apply fogging based on scattering
        bool bIgnoreSkyFog;				    //!< Ignore depth values of (1.0f) for fogging
        float  fBlendfactor;				//!< Blend factor to use for compositing
        
    };

    //! Specifies the class of light source
    enum class LightType
    {
        UNKNOWN = -1,
        DIRECTIONAL,    //!< Simple analytic directional light (like the sun)
        SPOTLIGHT,      //!< Spotlight with frustum shadow map and angular falloff
        OMNI,			//!< Omni-directional local light source
        COUNT
    };

    //! External definition. MOVE ME.
    extern CRenderer *g_mainRenderer;
    
    
    // Renderer console variables.
    extern SConsoleVar      * OpenGL_Version;                   // OpenGL version.
    
    
    extern SConsoleVar    * Use_UniformBuffers; //! Use uniform buffers?
    
    extern SConsoleVar      *  Render_Fullscreen;                      // Is game running full screen?
    extern SConsoleVar      * Render_VSync;                           // Is vertical syncing enabled?
    extern SConsoleVar      * Render_Width;                       // Renderer width.
    extern SConsoleVar      * Render_Height;                      // Renderer height.
    extern SConsoleVar      * Render_HalfWidth;
    extern SConsoleVar      * Render_OVR;                             // Virtual realilty.
    extern SConsoleVar      * Render_UseAppleMTE;
    
    extern bool         Render_Secret;                          // Psssss!
    extern bool         Render_Secret2;                          // Psssss!
    extern bool       Render_Screenshot;
    
    extern SConsoleVar        * Window_Width;
    extern SConsoleVar        * Window_Height;
    
    extern SConsoleVar        * Window_OriginX;
    extern SConsoleVar        * Window_OriginY;
}
#endif // NEKO_SERVER

#endif
