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
//  Renderer.cpp
//  Main game renderer.. ;p
//
//  Created by Neko Vision on 31/12/2013.
//  Copyright (c) 2013 Neko Vision. All rights

//#include "StaticWorld.h"
#include "../../AssetCommon/AssetBase.h"
#include "../../Core/Console/Console.h"
#include "../../Core/Console/ConsoleCommand.h"
#include "../../Core/Core.h"
#include "../../Core/Player/Camera/Camera.h"
#include "../../Core/Player/Input/Input.h"
#include "../../Core/ScriptSupport/Scripting.h"
#include "../../Core/UserInterface/UserMenu.h"
#include "../../Core/Utilities/Utils.h"
#include "../../Platform/Shared/System.h"
#include "../../World/BeautifulEnvironment.h"
#include "../../World/Mesh.h"
#include "../GraphicsManager.h"
#include "../OpenGL/OpenGLBase.h"
#include "FontRenderer.h"
#include "Renderer.h"
#include "ShadowRenderer.h" // woah woah


#   if !defined( NEKO_SERVER ) // ???

namespace Neko {
    
    /**  Main beautiful game renderer.   **/
    
    //! Maximum bloom passes ( downsample, blur ).
    const static int32_t MAX_BLOOM_PASSES  = 6;
    
    //  Add some CVars.
    
    //! Current rendering context.
    SConsoleVar * Render_Context    = 0;
    
    //!  Is fullscreen mode turned on?
    SConsoleVar * Render_Fullscreen = 0;

    //!  Vertical synchronization.
    SConsoleVar * Render_VSync = 0;

    //!  Renderer viewport width.
    SConsoleVar * Render_Width = 0;
    //!  Renderer viewport height.
    SConsoleVar * Render_Height = 0;
    
    //!  Turn on HDR renderer?
    SConsoleVar * Render_IsHDR = 0;
    
    //!  Window width.
    SConsoleVar * Window_Width = 0;
    //!  Window height.
    SConsoleVar * Window_Height = 0;
    
    //! Uh oh, a bit of magic we have got.
    SConsoleVar * Render_OVR = 0;
    
    //!  Take a screenshot.
    bool Render_Screenshot = false;
    //!  u lil sneaky
    bool Render_Secret = false;
    bool Render_Secret2 =  false;
    

    //! Use Apple multithreaded engine.
#   if defined( __APPLE__ )
    SConsoleVar * Render_UseAppleMTE = 0;
#   endif // __APPLE__
    
    
    /**     Shader properties.      **/
    
    //! SSAO kernel size.
    SConsoleVar * GFX_SSAOKernelSize = 0;
    //! Should we use Hi-Z based culling?
    SConsoleVar * GFX_UseHiZCulling = 0;
    //! Correct high quality bloom ( with several mipmap samplers ).
    SConsoleVar * GFX_UseHQCBloom = 0;
    //! Eye adaptation.
    SConsoleVar * GFX_UseEyeAdaptation = 0;
    SConsoleVar * GFX_UseEyeAdaptationQuality = 0;
    //! Anti-aliasing method.
    SConsoleVar * GFX_AntiAliasing = 0;
    
    SConsoleVar * GFX_MotionBlur = 0;
    SConsoleVar * GFX_Use2DSunShafts = 0;
    SConsoleVar * GFX_UseLensFlare = 0;
    SConsoleVar * GFX_LensCA = 0;
    
    //! Use physically-based volumetric lighting?
    SConsoleVar * GFX_VolumetricLighting = 0;
    SConsoleVar * GFX_VolumetricLightingQuality = 0;
    
    SConsoleVar * Use_UniformBuffers = 0;
    
    /**
     *  Constructor.
     */
    CRenderer::CRenderer() : renderWidth( 0 ), renderHeight( 0 ), renderHalfWidth(0), renderHalfHeight(0), windowWidth(0), windowHeight( 0 )
    {
        // Total GPU buffers allocated.
        m_iGPUBuffersAllocated = 0;
        
        Initialized = false;
        
        m_pGraphicsInterface = NEKO_NULL;    // will be set later in interface initialization
        
        // Renderer framebuffers.
        g_waterReflectionBuffer = NEKO_NULL;
        g_sceneBuffer[EYE_LEFT] = NEKO_NULL;  // two eyes for vr
        g_sceneBuffer[EYE_RIGHT] = NEKO_NULL;
        g_sceneBuffer[EYE_FULL] = NEKO_NULL;
    }
    
    /**
     *  Destructor.
     */
    CRenderer::~CRenderer()
    {
        // Use 'shutdown' instead.
    }
    
    /**
     *  Initialize console variables and preload some stuff.
     */
    void CRenderer::Preload()
    {
#   if defined( USES_LUA )
        // Read scripts.
        g_Core->p_ScriptBase->LoadScripts(); // TODO: Move me!
#   endif
        
        // Initial values.
        Initialized = false;
        m_iGPUBuffersAllocated = 0;
        
        // Register some console variables.
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Registering renderer console variables\n" );
        
        Render_Context =  g_Core->p_Console->RegisterCVar( ECvarGroup::Display, "sRenderingContext", "Rendering context API", "OpenGL", CVFlag::NeedsRefresh, ECvarType::String );
        Render_Width = g_Core->p_Console->RegisterCVar( ECvarGroup::Display, "iViewportWidth", "Viewport width.", 640, CVFlag::NeedsRefresh, ECvarType::Int );
        Render_Height = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "iViewportHeight", "Viewport height.", 480, CVFlag::NeedsRefresh, ECvarType::Int );
        Render_OVR = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bUseVR", "Virtual reality mode.", false, CVFlag::NeedsRefresh, ECvarType::Int );
        Render_Fullscreen = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bFullscreen", "Fullscreen mode?", false, CVFlag::NeedsRefresh, ECvarType::Int );
        Render_VSync = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bVSync", "Vertical synchronization?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        Render_IsHDR = g_Core->p_Console->RegisterCVar( ECvarGroup::Display, "bRenderHDR", "Render in High Dynamic Range?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        
        Window_Width = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "iWindowWidth", "Window size width.", 0, CVFlag::NeedsRefresh, ECvarType::Int );
        Window_Height = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "iWindowHeight", "Window size height.", 0, CVFlag::NeedsRefresh, ECvarType::Int );
        
        GFX_SSAOKernelSize = g_Core->p_Console->RegisterCVar( ECvarGroup::Display, "iSSAOKernelSize", "Ambient occlusion kernel size", 16, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_UseHiZCulling = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bUseHiZCulling", "Use Hierarchical-Z occlusion culling?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        
        GFX_UseHQCBloom = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bUseHQBloom", "Use corrected high quality bloom?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_UseEyeAdaptation = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bUseEyeAdaptation", "Eye adaptation?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_UseEyeAdaptationQuality = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "iEyeAdaptation", "Eye adaptation quality", 0, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_AntiAliasing = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "sAntialiasing", "Anti-aliasing", "TAA", CVFlag::NeedsRefresh, ECvarType::String );
        GFX_MotionBlur = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bUseMotionBlur", "Motion blur", true, CVFlag::NeedsRefresh, ECvarType::Int );
    
        GFX_Use2DSunShafts = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bSunShafts", "Basic sun shafts", true, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_UseLensFlare = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bLensFlare", "Lens flare", true, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_LensCA = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bLensCA", "Chromatic aberation", true, CVFlag::NeedsRefresh, ECvarType::Int );
        
        GFX_VolumetricLighting = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bVolumetricLighting", "Use volumetric lighting?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        GFX_VolumetricLightingQuality = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bVolumetricLightingQuality", "Volumetric lighting quality?", 2, CVFlag::NeedsRefresh, ECvarType::Int );
        
        Use_UniformBuffers = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bUniformBuffers", "Use uniform shader buffers?", false, CVFlag::NeedsRefresh, ECvarType::Int);
        
#   if defined( __APPLE__ )
        Render_UseAppleMTE = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bAppleMTE", "Use Apple Multithreaded GL engine.", true, CVFlag::NeedsRefresh, ECvarType::Int ); // Apple's Multithreaded engine. -- Good for high vertex throughput.
#   endif
        
        g_Core->p_Console->LoadIni( "neko" );
        
        bAllowBufferLogging = true;

        g_Core->p_Console->Print( LOG_INFO, "Done preloading renderer..\n" );
    }
    
    /**
     *  Create basic sprite.
     */
    static void CreateSprite( const int32_t idx, const int32_t x, const int32_t y, Vec3 * vertices, uint32_t * triangles, Vec2 * uv0, Vec2 uv, const float halfWidth, const float halfHeight, const float step )
    {
        const int32_t vIdx = idx * 3;
        const int32_t tIdx = idx * 3;
        
        float offset = step * 0.0f;
        
        uv0[vIdx + 0] = uv;
        uv0[vIdx + 1] = uv;
        uv0[vIdx + 2] = uv;
        
        // Setup indices.
        triangles[tIdx + 0] = vIdx + 0;
        triangles[tIdx + 1] = vIdx + 2;
        triangles[tIdx + 2] = vIdx + 1;
        
        // Vertices.
        vertices[vIdx + 0] = Vec3( -1.0f + offset, -1.0f, 0.0f );
        vertices[vIdx + 2] = Vec3( -1.0f + offset, 1.0f, 0.0f );
        vertices[vIdx + 1] = Vec3( -1.0f + step * 1.5f + offset, 1.0f, 0.0f );
    }
    
    /// ============================================================================================
    
    
    const static int32_t kHistogramLutSize = 64;
    const float kHistogramLutStep = 2.0f / kHistogramLutSize;
    
    /**
     *  Initialize photorealistic eye adaptation postprocess.
     */
    void CEyeAdaptationPost::Init( bool lowResolution, CRenderer * renderer )
    {
        SMemoryTempFrame * tempMemory = NEKO_NULL;    // create mesh buffers
        
        pRenderer = renderer;
        
        // Load shader
        f_AssetBase->PrecacheShader( "EyeAdaptation", COMPILE_BASIC );
        
        // Find shader.
        m_pShader = f_AssetBase->FindAssetByName<SGLShader>( "EyeAdaptation" );
        
        // Create framebuffers.
        g_pHistogramFramebuffer = (CFramebuffer*)pRenderer->pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pHistogramFramebuffer->Create( kHistogramLutSize, 1, false, true, true, false, false, false, GL_R16F );
        
        // Two render targets because we need to copy a previous buffer.
        g_pBrightnessRT = (CFramebuffer*)pRenderer->pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pBrightnessRT->Create( 1, 1, false, true, true, false, false, false, GL_R16F );
        g_pBrightnessRTTemp = (CFramebuffer*)pRenderer->pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pBrightnessRTTemp->Create( 1, 1, false, true, true, false, false, false, GL_R16F );
        
        // Build a histogram.
        int32_t size = lowResolution ? 8 : 4; // 2x less resolution
        m_iMipLevel = lowResolution ? 3 : 2; // 4x less resolution
        
        m_fExposureOffset = 0.0f;
        m_fAverageThresholdMin = 0.00345f;
        m_fAverageThresholdMax = 0.0085f;
        
        m_fMinimumExposure = 0.9725f;
        m_fMaximumExposure = 2.05f;
        
        m_fRange = 8.0f;
        m_fAdaptationSpeedUp = 0.005f;
        m_fAdaptationSpeedDown = 0.0015f;
        m_fBrightnessMultiplier = 1.0f;
        
        m_fHistMin = 0.005f;
        
        m_fHistLogMin = log2f( m_fHistMin );
        m_fHistLogMax = log2f( 5.0f );
        m_fHistogramSize = 0.35f;
        
        m_iHistogramSizeX = pRenderer->renderWidth / size;
        m_iHistogramSizeY = pRenderer->renderHeight / size;
        
        int32_t maxTriangles = 65000 / 3;
        int32_t totalTriangles = m_iHistogramSizeX * m_iHistogramSizeY;
        
        m_iMeshCount = static_cast<int32_t>( ceilf( (1.0f * totalTriangles) / (1.0f * maxTriangles) ) );
        m_pMeshes = new CBasicMesh *[m_iMeshCount];
        
        int32_t currentPixel = 0;
        
        float spriteWidth = 2.0f / kHistogramLutSize;
        float spriteHeigth = 2.0f;
        
        Vec2 halfPixelSize = Vec2( 1.0f / m_iHistogramSizeX * 0.5f, 1.0f / m_iHistogramSizeY * 0.5f );
        
        tempMemory = _PushMemoryFrame( pLinearAllocator2 );
        
        //@todo use Neko memory allocator
        for( int32_t k( 0 ); k < m_iMeshCount; ++k ) {
            CBasicMesh * currentMesh = new CBasicMesh;
            
            int32_t nbQuads = totalTriangles;
            if( totalTriangles > maxTriangles ) {
                nbQuads = maxTriangles;
            }
            
            totalTriangles -= nbQuads;
            
            Vec3 * vertices = (Vec3 *)PushMemory( tempMemory, sizeof(Vec3) * nbQuads * 3 );
            uint32_t * triangles = (uint32_t *)PushMemory( tempMemory, sizeof(uint32_t) * nbQuads * 3 );
            Vec2 * uv0 = (Vec2 *)PushMemory( tempMemory, sizeof(Vec2) * nbQuads * 3 );
            
            int32_t x;
            int32_t y;
            
            for( int32_t i( 0 ); i < nbQuads; ++i ) {
                x = currentPixel % m_iHistogramSizeX;
                y = (currentPixel - x) / m_iHistogramSizeX;
                
                CreateSprite( i, x, y, vertices, triangles, uv0,  Vec2(float(x) / float(m_iHistogramSizeX) + halfPixelSize.x, 1.0f - ((float(y) / float(m_iHistogramSizeY)) + halfPixelSize.y)), spriteWidth * 0.5f, spriteHeigth * 0.5f, kHistogramLutStep );
                
                ++currentPixel;
            }
            
            currentMesh->Create(vertices, NEKO_NULL, uv0, triangles, nbQuads * 3, nbQuads * 3, EPrimitiveType::Triangles);
            
            m_pMeshes[k] = currentMesh;
        }
        
        _PopMemoryFrame( tempMemory );
        
        Update();
    }
    
    /**
     *  Update postprocess.
     */
    void CEyeAdaptationPost::Update()
    {
        // Setup shader uniforms.
        m_pShader->Use();
        
        // Shader subroutines.
        m_iHistogramSR[0] = m_pShader->GetSubroutineIndex( "Default", EShaderType::Pixel );
        m_iHistogramSR[1] = m_pShader->GetSubroutineIndex( "HistogramAnalysis", EShaderType::Pixel );
        m_iHistogramSR[2] = m_pShader->GetSubroutineIndex( "HistogramVert", EShaderType::Vertex );  // histogram creationism
        m_iHistogramSR[3] = m_pShader->GetSubroutineIndex( "DefaultVert", EShaderType::Vertex );    // fullscreen
        
        // Samplers..
        m_pShader->SetUniform( "renderedTexture", 0 );
        m_pShader->SetUniform( "histogramLut", 1 );
        m_pShader->SetUniform( "previousBrightness", 2 );
        
        m_iUniforms[Pixel9098Count] = m_pShader->UniformLocation("PhotoPixelCount");
        m_iUniforms[Coefs] = m_pShader->UniformLocation( "vCoefs" );
        m_iUniforms[ExposureOffset] = m_pShader->UniformLocation( "fExposureOffset" );
        m_iUniforms[MinMaxSpeedTime] = m_pShader->UniformLocation("vMinMaxSpeedDt");
        //        m_iUniforms[Range] = m_pShader->UniformLocation("fValueRange");
        m_iUniforms[Size] = m_pShader->UniformLocation("fSize");
        m_iUniforms[Count] = m_pShader->UniformLocation("fLutSize");
        
        m_fHistLogMax = log2f( m_fRange );
        float delta = m_fHistLogMax - m_fHistLogMin;
        float coef0 = 1.0f / delta;
        float coef1 = -(coef0 * m_fHistLogMin);
        
        m_vHistCoefs = Vec4( coef0, coef1, m_fHistMin, m_fRange );
        
        const float numPixels = m_iHistogramSizeX * m_iHistogramSizeY;
        m_pShader->SetUniform( m_iUniforms[Pixel9098Count], numPixels * m_fAverageThresholdMin,
                              numPixels * m_fAverageThresholdMax );
        m_pShader->SetUniform( m_iUniforms[Coefs], m_vHistCoefs );
        m_pShader->SetUniform( m_iUniforms[ExposureOffset], -1.0f * m_fExposureOffset );
        m_pShader->SetUniform( m_iUniforms[Range], (float)m_fRange );
        m_pShader->SetUniform( m_iUniforms[Size], (float)kHistogramLutStep );
        m_pShader->SetUniform( m_iUniforms[Count], (float)kHistogramLutSize );
        m_pShader->SetUniform( "iMipLevel", m_iMipLevel );  // downsampling ( 2x/3x )
        
        
        m_pShader->Next();
    }
    
    /**
     *  Render eye adaptation.
     */
    void CEyeAdaptationPost::Render( uint32_t frameMsec )
    {
        // TODO: make a support for compute shaders
        
        //  Histogram creationism pass.
        // Create a lookup texture.
        g_pHistogramFramebuffer->BindForDrawing();
        
        pRenderer->SetViewportSize( kHistogramLutSize, 1 );
        pRenderer->ClearColorDepth();
        
        m_pShader->Use();
        
        // Select subroutine.
        glUniformSubroutinesuiv( GL_VERTEX_SHADER, 1, &m_iHistogramSR[2] );
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_iHistogramSR[0] );
        
        // using downsampled level in shader
        pRenderer->BindTexture( 0, pRenderer->m_pCompositeTex2->Image.GetId() );
        
        // draw histogram
        for( uint32_t i(0); i < m_iMeshCount; ++i ) {
            m_pMeshes[i]->DrawArrays();
        }
        
        g_pHistogramFramebuffer->UnbindDrawing();
        
        // Eye adaptation brightness (saves previous in its buffer).
        g_pBrightnessRT->BindForDrawing();
        
        pRenderer->SetRenderTarget( ERenderTarget::RT0 );
        pRenderer->SetViewportSize( 1, 1 );
        pRenderer->ClearColor();
        
        // Select histogram analysis subroutine.
        glUniformSubroutinesuiv( GL_VERTEX_SHADER, 1, &m_iHistogramSR[3] );
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_iHistogramSR[1] );
        
        // Adaptation time.
        m_pShader->SetUniform( m_iUniforms[MinMaxSpeedTime], Vec4(1.0f / m_fMaximumExposure, 1.0f / m_fMinimumExposure, m_fAdaptationSpeedDown * frameMsec, m_fAdaptationSpeedUp * frameMsec ) );
        
        pRenderer->BindTexture( 1, g_pHistogramFramebuffer->GetUnitRT0() );
        pRenderer->BindTexture( 2, g_pBrightnessRTTemp->GetUnitRT0() );    // previous data
        
        pRenderer->g_pEyeMesh[0]->DrawArrays();        // render quad
        
        m_pShader->Next();
        g_pBrightnessRT->UnbindDrawing();
        
        // Copy brightness data.
        {
            pRenderer->m_pCopyShader2->Use();
            g_pBrightnessRTTemp->BindForDrawing();
            pRenderer->ClearColor();
            
            pRenderer->BindTexture( 0, g_pBrightnessRT->GetUnitRT0() );
            pRenderer->g_pEyeMesh[0]->DrawArrays();
            
            pRenderer->m_pCopyShader2->Next();
            g_pBrightnessRTTemp->UnbindDrawing();
        }
    }
    
    /**
     *  Destroy the postprocess.
     *
     *  @todo Use Neko memory system.
     */
    void CEyeAdaptationPost::Disable()
    {
        for( int32_t i(m_iMeshCount); i > 0; --i ) {
            m_pMeshes[i]->Destroy();
            delete m_pMeshes[i];
        }
        
        delete [] m_pMeshes;
        
        g_pBrightnessRT->Delete();
        pRenderer->pAllocator->Dealloc( g_pBrightnessRT );
        
        g_pHistogramFramebuffer->Delete();
        pRenderer->pAllocator->Dealloc( g_pHistogramFramebuffer );
        
        
        g_Core->p_Camera->m_bJitter = false;
    }
    
    /// ============================================================================================
    
    /**
     *  Initialize Screenspace Temporal Anti-aliasing (SSTAA)
     *
     *  @param pRenderer Active renderer.
     */
    void CTemporalAAPost::Init(Neko::CRenderer *pRenderer)
    {
        m_pRenderer = pRenderer;
        
        m_iReprojectionIndex = -1;
        m_eFiltering = ETemporalFilter::MinMax3x3Rounded;
        
        m_bUnjitterColorSamples = true;
        m_bUnjitterNeighborhood = false;
        m_bUnjitterReprojection = false;
        
        m_bUseYCoCg = false;
        m_bUseClipping = false;
        m_bUseDilation = true;
        m_bUseMotionBlur = GFX_MotionBlur->Get<bool>();
        m_bUseOptimizations = false; // clipAABB, less branching
        
        m_fFeedbackMin = 0.88f;
        m_fFeedbackMax = 0.97f;
        
        m_fMotionBlurStrength = 1.0f;
        m_bMotionBlurIgnoreFF = false;
        
        CStr str( NekoCString::STR( "#define ZNEAR %f\n#define ZFAR %f\n", g_Core->p_Camera->m_fNearDist, g_Core->p_Camera->m_fFarDist ) );
        
        // Filtering modes..
        switch( m_eFiltering ) {
            case ETemporalFilter::MinMax3x3:
                str.Append( "#define MINMAX_3X3\n" );
                break;
                
            case ETemporalFilter::MinMax3x3Rounded:
                str.Append( "#define MINMAX_3X3_ROUNDED\n" );
                break;
                
            case ETemporalFilter::MinMax4TapVarying:
                str.Append( "#define MINMAX_4TAP_VARYING\n" );
                break;
        }
        
        // Preferences
        if( m_bUseDilation )    {     str.Append( "#define USE_DILATION\n" ); }
        if( m_bUseClipping )    {     str.Append( "#define USE_CLIPPING\n" ); }
        if( m_bUseYCoCg )       {     str.Append( "#define USE_YCOCG\n" ); }
        if( m_bUseOptimizations )   {     str.Append( "#define USE_OPTIMIZATION\n" ); }
        if( m_bUseMotionBlur )      {     str.Append( "#define USE_MOTION_BLUR\n" ); }
        if( m_bUnjitterColorSamples ) {     str.Append( "#define UNJITTER_COLORSAMPLES\n" ); }
        if( m_bUnjitterNeighborhood ) {     str.Append( "#define UNJITTER_NEIGHBORHOOD\n" ); }
        if( m_bUnjitterReprojection ) {     str.Append( "#define UNJITTER_REPROJECTION\n" ); }
        
        
        // SSTAA shader
        f_AssetBase->PrecacheShader( "temporal_aa", str.c_str(), COMPILE_BASIC );
        
        m_pShader = f_AssetBase->FindAssetByName<SGLShader>( "temporal_aa" );
        m_pShader->Use();
        
        // sampler info
        m_pShader->SetUniform( "renderedTexture", 0 );
        m_pShader->SetUniform( "PreviousFrame", 1 );
        m_pShader->SetUniform( "VelocityBuffer", 3 );
        m_pShader->SetUniform( "depthTexture", 4 );
        
        // texel size
        m_pShader->SetUniform( "vResolution", 1.0f / pRenderer->renderWidth, 1.0f / pRenderer->renderHeight );
        
        Update();
        
        m_pShader->Next();
        
        g_Core->p_Console->Print(LOG_INFO, "Creating framebuffer for SSTAA - %ix%i\n", pRenderer->renderWidth, pRenderer->renderHeight);
        
        // two targets for the frame history
        g_pReprojectionBuffer = (CFramebuffer *)pRenderer->pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pReprojectionBuffer->Create( pRenderer->renderWidth, pRenderer->renderHeight, true, true, true, true, false, false );
        
        g_Core->p_Camera->m_bJitter = true;
    }
    
    /**
     *  Update shader.
     */
    void CTemporalAAPost::Update()
    {
        float oneExtentY = tanf(0.5f * nkMath::Deg2Rad * GameView_FieldOfView->Get<float>());
        float oneExtentX = oneExtentY * (g_mainRenderer->renderAspectRatio);
        
        m_iUniforms[JitterOffset] = m_pShader->UniformLocation( "vJitterOffset" );
        m_iUniforms[Time] = m_pShader->UniformLocation( "fTime" );
        
        m_pShader->SetUniform( "vCorner", Vec4(oneExtentX, oneExtentY, 0.0f, 0.0f) );
        m_pShader->SetUniform( "fFeedbackMin", m_fFeedbackMin );
        m_pShader->SetUniform( "fFeedbackMax", m_fFeedbackMax );
        m_pShader->SetUniform( "fMotionScale", m_fMotionBlurStrength * (m_bMotionBlurIgnoreFF ? nkMath::Min(1.0f, 1.0f / 1.0f) : 1.0f) );
    }
    static float tempMatrix[16];
    /**
     *  Process temporal anti-aliasing.
     */
    void CTemporalAAPost::Render()
    {
        uint32_t firstPass = 0;
        if( m_iReprojectionIndex == -1 ) {   // bootstrap
            m_iReprojectionIndex = 0;
            
            // give it a first pass to process
            firstPass = m_pRenderer->m_pCompositeTex2->Image.GetId();
            m_reprojectionMatrix[m_iReprojectionIndex] = g_Core->p_Camera->ProjectionMatrix * g_Core->p_Camera->ViewMatrix;
            
        } else {
            firstPass = m_pRenderer->m_pFrameHistory[m_iReprojectionIndex]->Image.GetId();
        }
        
        int32_t indexRead = m_iReprojectionIndex;
        int32_t indexWrite = (m_iReprojectionIndex + 1) % 2;
        
        g_pReprojectionBuffer->BindForDrawing();
        m_pRenderer->ClearColorDepth();
        m_pRenderer->ResetViewport();
        
        m_pShader->Use();
        
        m_pRenderer->BindTexture( 0, m_pRenderer->g_compositeBuffer->GetUnitRT2() );    // pre-last pass from renderer, after we write to the final pass (already processed with SSTAA) and apply filmic tonemapping, etc..
        m_pRenderer->BindTexture( 1, firstPass );   // frame history
        m_pRenderer->BindTexture( 4, m_pRenderer->g_sceneBuffer[0]->GetUnitDepth() );
        m_pRenderer->BindTexture( 3, m_pRenderer->g_pCopyBuffer->GetUnitRT0() );   // .zw holds velocity buffer
        
        m_pShader->SetUniform("invProjView", 1, false, tempMatrix);
        m_pShader->SetUniform("prevViewProj", 1, false, g_Core->p_Camera->prevViewProj.m);
        
        const float mult = 1.0f;
        float dt = m_pRenderer->frameMsec * mult;
        
        m_pShader->SetUniform( m_iUniforms[Time], dt );
        m_pShader->SetUniform( m_iUniforms[JitterOffset], g_Core->p_Camera->m_vActiveSample );
        
        // reproject frame n-1 into output + history buffer
        {
            const static uint32_t restoreModess[2] = {
                GL_COLOR_ATTACHMENT0, // to screen
                GL_COLOR_ATTACHMENT1, // to buffer
            };
            
            glDrawBuffers( 2, (GLenum*)restoreModess );
            //m_pRenderer->ClearColorDepth();
            
            m_pRenderer->g_pEyeMesh[0]->DrawArrays();
            
            m_reprojectionMatrix[indexWrite] = g_Core->p_Camera->ViewProjectionMatrix;
            m_iReprojectionIndex = indexWrite;
        }
        
        m_pShader->Next();
        g_pReprojectionBuffer->UnbindDrawing();
        
        // @note - can't use the same framebuffer to copy (only use)
        
        glDrawBuffer( GL_COLOR_ATTACHMENT0 );
        
        // save buffer
        m_pRenderer->g_pFrameHistoryBuffer->BindForDrawing();
        {
            m_pRenderer->m_pCopyShader2->Use();
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   m_pRenderer->m_pFrameHistory[indexWrite]->Image.GetId(), 0 );
            
            m_pRenderer->BindTexture( 0, g_pReprojectionBuffer->GetUnitRT1() );
            
            m_pRenderer->g_pEyeMesh[0]->DrawArrays();
            
            m_pRenderer->m_pCopyShader2->Next();
        }
        m_pRenderer->g_pFrameHistoryBuffer->UnbindDrawing();
    }
    
    /**
     *  Disable current postprocess.
     */
    void CTemporalAAPost::Disable()
    {
        assert( g_pReprojectionBuffer == NEKO_NULL );
        
        g_pReprojectionBuffer->Delete();
        m_pRenderer->pAllocator->Dealloc( g_pReprojectionBuffer );
    }
    
    /// ============================================================================================
    
    /**
     *  Precache ( setup ) some stuff after pre-load.
     */
    void CRenderer::Precache()
    {
        g_Core->p_Console->Print( LOG_INFO, "Preparing renderer environment..\n" );
        
        // Rendering in HDR?
        m_bIsHDR = Render_IsHDR->Get<bool>();
        
        // Some variables.
        renderWidth = Render_Width->Get<int>();      // Renderer width.
        renderHeight = Render_Height->Get<int>();        // Renderer height.
        
        renderHalfWidth = renderWidth >> 1;     // Renderer half width.
        renderHalfHeight = renderHeight >> 1;   // Renderer half height.
        
        renderAspectRatio = renderWidth / this->renderHeight;     // Aspect ratio.
        
        renderAspectWidth = ( (float)renderWidth / renderAspectRatio );     // Renderer aspect width.
        renderAspectHeight = ( (float)renderHeight / renderAspectRatio );   // Renderer aspect height.
        
        
#   if defined( USES_OPENGL )
        // Scene shader.
        g_Core->p_Console->Print( LOG_INFO, "Looking for renderer shaders..\n" );
        // Load shaders.
        
        CStr parameters("");
        
        // ES, GLSL
        if( m_bRequiresPrecision == 1 ) {
            parameters.Append( "#define USE_PRECISION\n");
            parameters.Append( "#define PRECISION_TYPE highp\n" );
        }
        
        parameters.Append( "#define ZFAR %7.1f\n", g_Core->p_Camera->GetFarPlaneDistance() );
        parameters.Append( "#define ZNEAR %f\n", g_Core->p_Camera->GetNearPlaneDistance() );
        parameters.Append( "#define WIDTH %i.0\n", renderWidth );
        parameters.Append( "#define HEIGHT %i.0\n", renderHeight );
        parameters.Append( "#define ASPECTRATIO WIDTH/HEIGHT\n" );
        parameters.Append( "#define TEXELSIZEW %f\n", 1.0f / renderWidth );
        parameters.Append( "#define TEXELSIZEH %f\n", 1.0f / renderHeight );
        
        parameters.Append( "#define USE_GLOBALFOG\n" ); // Height fog.
        parameters.Append( "#define USE_SHADOWS\n" );
        
        if( GFX_UseLensFlare->Get<bool>() ) {
            parameters.Append( "#define USE_LENS\n" ); // Filmic lens.
            parameters.Append( "#define USE_LENSFLARE\n" );
        }  // Lens flare.
        
        if( GFX_Use2DSunShafts->Get<bool>() ) { parameters.Append( "#define USE_SUNRAYS\n" ); } // Basic 2d sunrays, not a volumetric lighting..
        
        f_AssetBase->PrecacheShader( "passthru", parameters.c_str(), COMPILE_BASIC); /* #define USE_SUNGLARE\n*/
        
        parameters.Clear();
        
        // Load composite shader.
        // ES, GLSL
        if( m_bRequiresPrecision == 1 ) {
            parameters.Append( "#define USE_PRECISION\n");
            parameters.Append( "#define PRECISION_TYPE highp\n" );
        }
        
        parameters.Append( "#define ZFAR %7.1f\n", g_Core->p_Camera->GetFarPlaneDistance() );
        parameters.Append( "#define ZNEAR %f\n", g_Core->p_Camera->GetNearPlaneDistance() );
        parameters.Append( "#define WIDTH %i.0\n", renderWidth );
        parameters.Append( "#define HEIGHT %i.0\n", renderHeight );
        parameters.Append( "#define ASPECTRATIO WIDTH/HEIGHT\n" );
        parameters.Append( "#define TEXELSIZEW %f\n", 1.0f / renderWidth );
        parameters.Append( "#define TEXELSIZEH %f\n", 1.0f / renderHeight );
        
        f_AssetBase->PrecacheShader( "composite", parameters.c_str(), COMPILE_BASIC );
        
        parameters.Clear();
        
        if( !strcmp( GFX_AntiAliasing->Get<const char*>(), "FXAA" ) ) {
            parameters.Append( "#define USE_FXAA\n" ) ;
        }
        
        // Load post-composite shaders.
        f_AssetBase->PrecacheShader( "composite_afterpass", parameters.c_str(), COMPILE_BASIC );
        f_AssetBase->PrecacheShader( "composite_multipass", COMPILE_BASIC );
        
        
        sceneShader = f_AssetBase->FindAssetByName<SGLShader>( "passthru" );
        compositeShader = f_AssetBase->FindAssetByName<SGLShader>( "composite" );
        compositeShader2 = f_AssetBase->FindAssetByName<SGLShader>( "composite_multipass" );
        compositeShader3 = f_AssetBase->FindAssetByName<SGLShader>( "composite_afterpass" );
        // A shader used to draw 2d stuff on screen.
        draw2dshader = f_AssetBase->FindAssetByName<SGLShader>( "draw2d" );
        // m_deferredSShader = f_AssetBase->FindShaderByName( "deferred ");
        hiZshader = f_AssetBase->FindAssetByName<SGLShader>( "hi-z" );
        downsampleShader = f_AssetBase->FindAssetByName<SGLShader>( "downsample_blur" );
        
        m_pBrdfShader = f_AssetBase->FindAssetByName<SGLShader>( "IntegrateBRDF" );
        
        m_pMiscShader = f_AssetBase->FindAssetByName<SGLShader>( "CopyShader" );
        m_pCopyShader2 = f_AssetBase->FindAssetByName<SGLShader>( "CopyShader2" );
        
//        m_pComputePhaseLut = f_AssetBase->FindAssetByName<SGLShader>("ComputePhaseLut" );
//        m_pComputeLightLut = f_AssetBase->FindAssetByName<SGLShader>("ComputeLightLut" );
//        m_pVolumetricLight = f_AssetBase->FindAssetByName<SGLShader>("RenderVolume" );
        
        // Indexed data would be better????
        // Used for render-to-texture method ( screen quds ).
        const static Vec3 g_ESceneEyeVertexData[3][6][3] =
        {
            // Full view.
            {
                Vec3( -1.0f, -1.0f, 0.0f ),
                Vec3(  1.0f, -1.0f, 0.0f ),
                Vec3( -1.0f,  1.0f, 0.0f ),
                Vec3( -1.0f,  1.0f, 0.0f ),
                Vec3( 1.0f,  -1.0f, 0.0f ),
                Vec3( 1.0f,   1.0f, 0.0f )
            },
            
            // Left eye.
            {
                Vec3( -1.0f, -1.0f, 0.0f ),
                Vec3(  0.0f, -1.0f, 0.0f ),
                Vec3( -1.0f, 1.0f, 0.0f ),
                Vec3( -1.0f, 1.0f, 0.0f ),
                Vec3( 0.0f,  1.0f, 0.0f ),
                Vec3( 0.0f, -1.0f, 0.0f ),
            },
            
            // Right eye.
            {
                Vec3(  0.0f, -1.0f, 0.0f ),
                Vec3(  1.0f, -1.0f, 0.0f ),
                Vec3(  0.0f,  1.0f, 0.0f ),
                Vec3(  0.0f,  1.0f, 0.0f ),
                Vec3(  1.0f, -1.0f, 0.0f ),
                Vec3(  1.0f,  1.0f, 0.0f ),
            },
        };
        
        //  UV coordinates.
        const static Vec2 g_ESceneEyeUVData[3][6][2] =
        {
            // Full view.
            {
                Vec2( 0.0f, 0.0f ),
                Vec2( 1.0f, 0.0f ),
                Vec2( 0.0f, 1.0f ),
                Vec2( 0.0f, 1.0f ),
                Vec2( 1.0f, 0.0f ),
                Vec2( 1.0f, 1.0f ),
            },
            
            // Left eye.
            {
                Vec2( 0.0f, 0.0f ),
                Vec2( 0.5f, 0.0f ),
                Vec2( 0.0f, 1.0f ),
                Vec2( 0.0f, 1.0f ),
                Vec2( 0.5f, 1.0f ),
                Vec2( 0.5f, 0.0f ),
            },
            
            // Right eye.
            {
                Vec2( 0.5f, 0.0f ),
                Vec2( 1.0f, 0.0f ),
                Vec2( 0.5f, 1.0f ),
                Vec2( 0.5f, 1.0f ),
                Vec2( 1.0f, 0.0f ),
                Vec2( 1.0f, 1.0f )
            }
        };
        
        sceneShader->Use();
        
        // Scene view "eyes". Full, Left, Right.
        g_Core->p_Console->Print( LOG_INFO, "Creating renderer viewports...\n" );
        const uint32_t toGenerate = Render_OVR->Get<bool>() ? 3 : 1;
        
        for( uint32_t i(0); i < toGenerate; ++i ) {
            g_pEyeMesh[i] = (CBasicMesh *)pAllocator->Alloc( sizeof(CBasicMesh) );
            g_pEyeMesh[i]->Create( &g_ESceneEyeVertexData[i][0][0], NEKO_NULL, &g_ESceneEyeUVData[i][0][0], NEKO_NULL, 6, 0 );
        }
        
        // Onscreen textures.
//        _lens_texture = f_AssetBase->p_MaterialBase->Find( "lensdirt" );
//        m_pLutSampler = f_AssetBase->p_MaterialBase->Find( "filmlut" );
        
        // Create singlepass mesh data.
        Vec3 Vertices[1];
        Vertices[0] = Vec3( 0.0f, 0.0f, 0.0f );
        
        m_pScreenSinglePass = (CBasicMesh *)pAllocator->Alloc( sizeof(CBasicMesh) );
        m_pScreenSinglePass->Create( Vertices, NEKO_NULL, NEKO_NULL, NEKO_NULL, 1, 0 );
        
        
        // Bloom settings.
        m_fBlurSize = 4.0f;
        m_fBloomIntensity = expf(0.4855f) - 1.0f;    // 0.0f..0.5f
        m_fLensDirtIntensity = expf(0.05f) - 1.0f;   // 0.0f..0.95f
        
        
        // Ids.
        sceneShader->SetUniform( "renderedTexture", 0 );    // Base texture.
        sceneShader->SetUniform( "depthTexture", 1 );       // Scene depth.
        sceneShader->SetUniform( "iblSampler", 2 );         // RMA maps.
        sceneShader->SetUniform( "normalTexture", 3 );      // Normal map.
        sceneShader->SetUniform( "emissiveSampler", 4 );    // Emissive (glow/shadow) sampler.
        sceneShader->SetUniform( "shadowSamplers", 5 ); // 2D Texture array!
        sceneShader->SetUniform( "noiseTex", 6 );
        sceneShader->SetUniform( "lightTexture", 7 );   // Static point light data.
        sceneShader->SetUniform( "particleSampler", 8 );    // Particles.
        sceneShader->SetUniform( "environmentSampler", 9 ); // Environment map sampler.
        sceneShader->SetUniform( "brdfSampler", 10 );   // Done on CPU.
        
        // Renderer first postprocess shading uniforms..
        _renderUniforms[RLightPosition_Time] = sceneShader->UniformLocation( "lightDirectionAndTime" );
        _renderUniforms[RSunWorldPos] = sceneShader->UniformLocation( "vSunPositionWS" );
        _renderUniforms[RSkyColor] = sceneShader->UniformLocation( "mSkyColor" );
        _renderUniforms[RGlobalFogColor] = sceneShader->UniformLocation( "mGlobalFogColor" );
        _renderUniforms[RLight2DPosition] = sceneShader->UniformLocation( "lightPositionOnScreen");
        _renderUniforms[RSunColor] = sceneShader->UniformLocation( "mSunColor" );
        _renderUniforms[RInvertedViewProj] = sceneShader->UniformLocation( "invProjView" );
        _renderUniforms[RCameraPosition] = sceneShader->UniformLocation( "cameraPosition" );
        _renderUniforms[RSunLuminance] = sceneShader->UniformLocation( "fSunLuminanceInLux" );
        
        sceneShader->Next();
        
        // Draw2D shader.
        draw2dshader->Use();
        
        _renderUniforms[RDraw2D_Color] = draw2dshader->UniformLocation( "colorMod" );
        _renderUniforms[RDraw2D_ScaleOrigin] = draw2dshader->UniformLocation( "scale_origin" );
        
        draw2dshader->Next();
        
        // Setup composite shader.
        compositeShader->Use();
        compositeShader->SetUniform( "renderedTexture", 0 );
        compositeShader->SetUniform( "depthTexture", 1 );
        compositeShader->SetUniform( "emissiveSampler", 2 );
        //        compositeShader->SetUniform( "uLensColor", 2 );
        compositeShader->SetUniform( "vResolution", float(renderWidth), float(renderHeight) );
        compositeShader->SetUniform( "vTexelSize", (float)(1.0 / renderWidth), (float)(1.0 / renderHeight) );
        
        // Bloom.
        compositeShader->SetUniform( "fBloomIntensity", m_fBloomIntensity );
        compositeShader->SetUniform( "fLensDirtIntensity", m_fLensDirtIntensity );
        
        _renderUniforms[RInvertedViewProj2] = compositeShader->UniformLocation( "invProjView" );
        _renderUniforms[RPreviousViewProj] = compositeShader->UniformLocation( "prevViewProj" );
        compositeShader->Next();
        
        // Setup composite shader.
        compositeShader2->Use();
        compositeShader2->SetUniform( "renderedTexture", 0 );
        compositeShader2->SetUniform( "eyeAdaptationLum", 1 );
        compositeShader2->Next();
        
        // Setup composite shader.
        compositeShader3->Use();
        compositeShader3->SetUniform( "renderedTexture", 0 );
        compositeShader3->SetUniform( "emissiveSampler", 1 );
        compositeShader3->SetUniform( "depthTexture", 2 );
        compositeShader3->SetUniform( "vTexelSize", (float)(1.0 / renderWidth), (float)(1.0 / renderHeight) );
        compositeShader3->Next();
        
        
        // Setup Hi-Z shader.
        hiZshader->Use();
        hiZshader->SetUniform( "LastMip", 0 );
        
        _renderUniforms[RLastMipSize] = hiZshader->UniformLocation("LastMipSize" );
        
        hiZshader->Next();
        
        // Downsample and blur shader.
        downsampleShader->Use();
        downsampleShader->SetUniform( "LastMip", 0 );
        
        // Setup subroutines.
        m_iDSRoutine[0] = downsampleShader->GetSubroutineIndex( "Downsample", EShaderType::Pixel);
        m_iDSRoutine[1] = downsampleShader->GetSubroutineIndex( "BlurV", EShaderType::Pixel);
        m_iDSRoutine[2] = downsampleShader->GetSubroutineIndex( "BlurH", EShaderType::Pixel);
        m_iDSRoutine[3] = downsampleShader->GetSubroutineIndex( "Final", EShaderType::Pixel);
        
        _renderUniforms[RBlurSize] = downsampleShader->UniformLocation( "fBlurSize" );
        _renderUniforms[RTexelSize] = downsampleShader->UniformLocation( "vTexelSize" );
        
        downsampleShader->SetUniform( _renderUniforms[RTexelSize], (float)(1.0 / renderWidth), (float)(1.0 / renderHeight) );
        downsampleShader->Next();
        
        // Setup 2d drawing shader.
        draw2dshader->Use();
        draw2dshader->SetUniform( "quad2Dtex", 0 );
        draw2dshader->Next();
        
        
        // Setup shader for a different sort of things..
        m_pMiscShader->Use();
        
        m_iMiscUniforms[0] = m_pMiscShader->UniformLocation("_PrevVP2");
        m_iMiscUniforms[1] = m_pMiscShader->UniformLocation("_InvVP");
        m_iMiscUniforms[2] = m_pMiscShader->UniformLocation("vCorner2");
        
        m_pMiscShader->SetUniform( "depthTexture", 0 );
        m_pMiscShader->SetUniform( "velocityTex", 1 );
        
        m_pMiscShader->Next();
        
        m_pCopyShader2->Use();
        m_pCopyShader2->SetUniform( "renderedTexture", 0 );
        m_pCopyShader2->Next();
        
//        m_pComputePhaseLut->Use();
        
        //        uniform uint g_uNumPhaseTerms = 1;
        //        uniform vec4 g_vPhaseParams[4];
        //        uniform uint g_uPhaseFunc[4];
        
        
        
//        m_pComputePhaseLut->Next();
        
        // Create postprocesses.
        
        m_pEyeAdaptationPost = (CEyeAdaptationPost* )pAllocator->Alloc( sizeof(CEyeAdaptationPost) );
        m_pEyeAdaptationPost->Init( GFX_UseEyeAdaptationQuality->Get<int>() == 0 ? true : false, this );
        
        if( !strcmp( GFX_AntiAliasing->Get<const char*>(), "TAA" ) ) {
            m_pTemporalAAPost = (CTemporalAAPost* )pAllocator->Alloc( sizeof(CTemporalAAPost) );
            m_pTemporalAAPost->Init( this );
        }
        
#   endif
        
    }
    
    /**
     *      Initialize renderer stuff here.
     */
    void CRenderer::Initialize( GraphicsInterface   * interface )
    {
        assert( interface != NEKO_NULL );
        
        int32_t t1, t2;
        
        g_Core->p_Console->Print( LOG_INFO, "Renderer init..\n" );
        
        t1 = g_Core->p_System->Milliseconds();   // Just for a record.
        
        // At this stage graphical API was initalized.
        g_Core->p_Console->Print( LOG_INFO, "Initializing graphics interface..\n" );
        m_pGraphicsInterface = interface;
        
        // Create renderer thread lock.
        m_pRenderLock = CCore::CreateLock();
        
        renderAspectRatio = ( float(renderWidth) / float(renderHeight) );
        
        // temp
        pAllocator = pMainAllocProxy;
        
        // Clear renderer tasks.
        m_pRenderQueue = (CQueue *)pAllocator->Alloc( sizeof(CQueue) );
        m_pRenderQueue->Create();
        
        // Initialize asset system and load assets.
        f_AssetBase = (CAssetManager *)pClassLinearAllocator->Alloc( sizeof(CAssetManager) );
        f_AssetBase->Initialize( pAllocator );
        
        // Initialize font renderer.
        p_CoreFont = (CCoreFontRenderer *)pClassLinearAllocator->Alloc( sizeof(CCoreFontRenderer) );
        p_CoreFont->Initialize();           // Load core font renderer.
        
        // Precache some stuff. :>
        Precache();
        
        
        // Create render target textures.
        m_pCompositeTex2 = f_AssetBase->p_MaterialBase->CreateMaterial( NEKO_NULL, renderWidth, renderHeight, TextureTarget::Texture2D, ETextureStorageType::Float, ETextureFormat::RGBA16F );
        
        // Frame history for: the current pass, past pass, past-past pass.
        for( int32_t i(0); i < 3; ++i ) {
            m_pFrameHistory[i] = f_AssetBase->p_MaterialBase->CreateMaterial( NEKO_NULL, renderWidth, renderHeight, TextureTarget::Texture2D, ETextureStorageType::Float, ETextureFormat::RGBA16F );
        }
        
        
        // Check for some errors or warnings.
#   if defined( USES_OPENGL )
        
        // Initialize framebuffer stuff.
        UpdateFramebufferObject( renderWidth, renderHeight );
        
        m_pGraphicsInterface->CheckAPIError( "Renderer load" );
#   endif
        
        t2 = g_Core->p_System->Milliseconds();
        g_Core->p_Console->Print( LOG_INFO, "Renderer and core asset load took %i msec.\n", t2 - t1 );
        
        g_pbEnv->MakePreparation( pAllocator );            // Prepare rendering world.
        
        
        
        // This gets called after camera initialization, so update scene shader.
        // Update shader stuff.
        //if( g_Core->p_Camera->IsInitialized() ) {
        sceneShader->Use();
        sceneShader->SetUniform( "perspectiveInvMatrix", 1, GL_FALSE, g_Core->p_Camera->ProjectionInvertMatrix.m );
        sceneShader->SetUniform( "projectionMatrix", 1, GL_FALSE, g_Core->p_Camera->ProjectionMatrix.m );
        
        //float fogBotHeight, fogTopHeight, fogBotInten, fogMaxDist, fogSlope = 1.0;
        sceneShader->SetUniform( "fogBotHeight", g_pbEnv->GetFogBottomHeight() );
        sceneShader->SetUniform( "fogTopHeight", g_pbEnv->GetFogTopHeight() );
        sceneShader->SetUniform( "fogBotInten", g_pbEnv->GetFogBottomIntensity() );
        sceneShader->SetUniform( "fogMaxDist", g_pbEnv->GetFogMaxDistance() );
        sceneShader->SetUniform( "fogSlope", g_pbEnv->GetFogSlope() );
        sceneShader->SetUniform( "shadow_ambient", g_pbEnv->m_pbEnvShadows->GetOpacity() );
        
        // Sun.
        sceneShader->SetUniform( _renderUniforms[RSunLuminance], g_pbEnv->m_fSunLuminance );
        
        // SSAO settings.
        // 'NoiseKernel' property is initialized only after material base init stuff.
        const int32_t kernelSize = GFX_SSAOKernelSize->Get<int32_t>();
        
        g_Core->p_Console->Print( LOG_INFO, "SSAO preferences:\n" );
        g_Core->p_Console->Print( LOG_INFO, "\tuKernelSize = %i\n", kernelSize );
        
        sceneShader->SetUniform( "uKernelSize", kernelSize );
        sceneShader->SetUniform3fv( "uKernelOffsets", kernelSize, (const GLfloat *)f_AssetBase->p_MaterialBase->noiseKernel );
        
        // Default threshold settings.
        sceneShader->SetUniform( "def_threshold", 0.64f );
        sceneShader->SetUniform( "def_gain", 1.2f );
        
        // Lens settings.
        sceneShader->SetUniform( "lens_samples", 4 );
        sceneShader->SetUniform( "lens_flare_dispersal", 0.4f );
        sceneShader->SetUniform( "lens_flare_halo_width", 0.45f );
        sceneShader->SetUniform( "lens_chroma_distortion", 0.01f, 0.03f, 0.05f );
        
        // @todo - spherical harmonics
        ncMatrix4 m;
        m.Identity();
        
        //        float w = 64.0;
        //        float minTextureSize = 32;
        //
        //        const float LN2 = logf(2);
        //
        //        float nbLod = logf( w ) / LN2;
        //        float maxLod = nbLod - logf( minTextureSize ) / LN2;
        //
        //        float coef0 = 1.0 / ( 2.0 * sqrtf( nkMath::PI ) );
        //        float coef1 = -( sqrtf( 3.0 / nkMath::PI ) * 0.5 );
        //        float coef2 = -coef1;
        //        float coef3 = coef1;
        //        float coef4 = sqrtf( 15.0 / nkMath::PI ) * 0.5;
        //        float coef5 = -coef4;
        //        float coef6 = sqrtf( 5.0 / nkMath::PI ) * 0.25;
        //        float coef7 = coef5;
        //        float coef8 = sqrtf( 15.0 / nkMath::PI ) * 0.25;
        //
        //        sceneShader->SetUniform( "uEnvironmentLodRange", nbLod, maxLod );
        //        sceneShader->SetUniform( "uEnvironmentSize", w, w );
        sceneShader->SetUniform( "environmentTransform", 1, false, m.m );
        
        sceneShader->Next();
        //}
        
        // Initialize user interface and menus now.
        p_UserInterface = (UserInterface *)pClassLinearAllocator->Alloc( sizeof(UserInterface) );
        
        p_UserInterface->Initialize( pAllocator );
        p_UserInterface->OpenMenu( "main" ); // Open main menu.
        
        
        g_Core->p_Console->WriteIni( "neko" );
        
        // Set renderer state to initialized.
        Initialized = true;
        
        // Some information.
        g_Core->p_Console->Print( LOG_INFO, "VRAM Texture memory used: %ld mb\n", 0 );
        g_Core->p_Console->Print( LOG_INFO, "Memory used: %llu mb\n", ByteInMegabyte(pMainAllocProxy->GetUsedMem()) );
        
    }
    
    /**
     *  Adjust element size.
     */
    const static Vec2 DEFAULT_SCREEN_SIZE = Vec2( 640.0f, 480.0f );
    void CRenderer::AdjustElemSize( float * x, float * y )
    {
        float xscale;
        float yscale;
        
        // @todo FIXME
        // Remember: these CVars aren't responsible for window size..
        // they're responsible for context ( view ) frame!
        xscale = 1.0f;//Window_Width.Get<float>() / DEFAULT_SCREEN_SIZE.x;
        yscale = 1.0f;//Window_Height.Get<float>() / DEFAULT_SCREEN_SIZE.y;
        
        if( x ) {
            *x *= xscale;
        }
        
        if( y ) {
            *y *= yscale;
        }
    }
    
    /**
     *  Same here. But with frame size now.
     */
    void CRenderer::AdjustElemSize( float *x, float *y, float *w, float *h )
    {
        AdjustElemSize( x, y );
        AdjustElemSize( w, h );
    }
    
    /**
     *  Fill GPU buffer with a data.
     */
    void CRenderer::BufferData( GPUBuffer *Buffer, const void *Data, unsigned int Offset, unsigned int Size )
    {
        if( !Data ) {
            g_Core->p_Console->Print( LOG_ERROR, "CRenderer::BufferData(): Attempt to pass empty data.\n" );
            return;
        }
        
        glBindBuffer( Buffer->Type, Buffer->Handle );
        
        if( Buffer->Size >= Size ) {
            glBufferSubData( Buffer->Type, Offset, Size, Data );
        } else {
            g_Core->p_Console->Print( LOG_ERROR, "BufferData(): Buffer with size %d bytes exceedes allocated buffer with %d bytes.\n", Size, Buffer->Size );
        }
        
        //        glBindBuffer( Buffer->Type, 0 );
        Buffer->Done = false;
    }
    
    /**
     *	Log created buffers?
     */
    void CRenderer::AllowBufferLogging( const bool allow ) {
        bAllowBufferLogging = allow;
    }
    
    /**
     *  Allocate GPU buffer.
     */
    GPUBuffer CRenderer::AllocGPUBuffer( uint32_t Size, EBufferStorageType Type, EBufferType Usage,
                                        EPrimitiveType Mode, uint32_t Complexity )
    {
        GPUBuffer newBuffer;
        
        if( Size == 0 ) {
            g_Core->p_Console->Print( LOG_WARN, "AllocGPUBuffer(): A buffer with null size!\n" );
        }
        
        memset( &newBuffer, 0x00, sizeof(GPUBuffer) );
        
        newBuffer.Count = 0;
        newBuffer.Size = Size;
        newBuffer.Complexity = Complexity;
        newBuffer.Type = g_pGraphicsManager->GetCurrentInterface()->GetAPIBufferStorageType( Type );
        newBuffer.VertexMode = g_pGraphicsManager->GetCurrentInterface()->GetAPIPrimitiveType( Mode );
        
        uint32_t BufferUsage = g_pGraphicsManager->GetCurrentInterface()->GetAPIBufferType( Usage );
        
#   if defined( USES_OPENGL )
        
        glGenBuffers( 1, &newBuffer.Handle );
        glBindBuffer( newBuffer.Type, newBuffer.Handle );
        glBufferData( newBuffer.Type, Size, NEKO_NULL, BufferUsage );
        glBindBuffer( newBuffer.Type, 0 );
        
#   else
        
#   endif
#define USAGE_STRING(s) s == GL_STREAM_DRAW ? "stream draw" : \
s == GL_STREAM_READ ? "stream read" : \
s == GL_STREAM_COPY ? "stream copy" : \
s == GL_STATIC_DRAW ? "static draw" : \
s == GL_STATIC_READ ? "static read" : \
s == GL_STATIC_COPY ? "static copy" : \
s == GL_DYNAMIC_DRAW ? "dynamic draw" : \
s == GL_DYNAMIC_READ ? "dynamic read" : \
s == GL_DYNAMIC_COPY ? "dynamic copy" : "Undefined" \

        ++m_iGPUBuffersAllocated;
        
        if( this->bAllowBufferLogging == true ) {
            g_Core->p_Console->Print( LOG_INFO, "AllocGPUBuffer(%i): A new GPU buffer was created with size of %d bytes and its \"%s\" usage\n", m_iGPUBuffersAllocated, Size, USAGE_STRING( BufferUsage ) );
        }
        
        return newBuffer;
    }
    
    /**
     *  Delete buffer.
     */
    void CRenderer::DeleteGPUBuffer( GPUBuffer *Buffer )
    {
        // Delete buffer now.
        glDeleteBuffers( 1, &Buffer->Handle );
        memset( Buffer, 0x00, sizeof( GPUBuffer ) );
        
        Buffer = NEKO_NULL;
        
        --m_iGPUBuffersAllocated;
    }
    
    /**
     *  Set GPU buffer vertex pointer.
     */
    void CRenderer::BufferPointer( GPUBuffer *Buffer, uint32_t Index, uint32_t Size, uint32_t Type, bool Normalized, uint32_t Stride, const void *Pointer )
    {
        glBindBuffer( Buffer->Type, Buffer->Handle );
        
        glEnableVertexAttribArray( Index );
        glVertexAttribPointer( Index, Size, Type, Normalized, Stride, Pointer );
        
        glBindBuffer( Buffer->Type, 0 );
    }
    
    /**
     *  Finish buffer creation.
     */
    void CRenderer::FinishBuffer( GPUBuffer *Buffer, uint32_t Offset, uint32_t Count )
    {
        Buffer->Count = Count;
        Buffer->Offset = Offset;
        Buffer->Done = true;
    }
    
    /**
     *  Set GPU buffer vertex pointer.
     */
    void CRenderer::BufferIPointer( GPUBuffer *Buffer, uint32_t Index, uint32_t Size, uint32_t Type, uint32_t Stride, const void *Pointer )
    {
        glBindBuffer( Buffer->Type, Buffer->Handle );
        
        glEnableVertexAttribArray( Index );
        glVertexAttribIPointer( Index, Size, Type, Stride, Pointer );
        
        glBindBuffer( Buffer->Type, 0 );
    }
    
    /**
     *  Check if we have errors in the graphical interface.
     */
    void CRenderer::CheckAPIErrors( const char * function, const int32_t line )
    {
#   if defined( USES_OPENGL )
        
#   endif
    }
    
    /**
     *  Create OpenGL vertex array.
     *
     *  Also binds vertex array.
     */
    const GLuint CRenderer::CreateVertexArray()
    {
        GLuint vertexArray;
        glGenVertexArrays( 1, &vertexArray );
        glBindVertexArray( vertexArray );
        
        return vertexArray;
    }
    
    /**
     *
     *  We need to call this to create FBOs.
     *
     */
    void CRenderer::UpdateFramebufferObject( const uint32_t w, const uint32_t h )
    {
        m_bIsHDR = true;
        const uint32_t mode = m_bIsHDR ? GL_RGBA16F : GL_RGBA8;
        
        // Advanced scenes.
        // Used for OVR. Three scene eyes, - Left, Right, Center ( full ).
        g_Core->p_Console->Print( LOG_INFO, "Creating renderer framebuffers..\n" );
        
        g_sceneBuffer[EYE_FULL] = (CFramebuffer*)pAllocator->Alloc(
                                                                   sizeof(CFramebuffer) );
        
        g_sceneBuffer[EYE_FULL]->Create( w, h, true, true, true, true, true, true, (GLenum)mode, (GLenum)mode, (GLenum)mode, (GLenum)mode );
        
        g_sceneBuffer[EYE_LEFT] = NEKO_NULL;
        g_sceneBuffer[EYE_RIGHT] = NEKO_NULL;
        
        g_pMiscFramebuffer = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pMiscFramebuffer->Create( 128, 128, true, true, true, false, false, false, GL_RG8 );
        
        // Final framebuffer used for drawing on screen.
        g_compositeBuffer = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        g_compositeBuffer->Create( w, h, true, true, true, true, true, true, (GLenum)mode, (GLenum)mode, (GLenum)mode, (GLenum)mode );
        
        g_pFrameHistoryBuffer = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pFrameHistoryBuffer->Create( w, h, true, true, true, false, false, false );
        
        // Screen copy buffer.
        g_pCopyBuffer = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        g_pCopyBuffer->Create( w, h, true, true, true, false, false, false );
        
        m_pPhaseLutTarget = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        m_pPhaseLutTarget->Create( 512, 512, true, true, true, false, false, false );
        
        m_pLightLutTarget = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        m_pLightLutTarget->Create( 512, 512, true, true, true, false, false, false );
        
        
        // Generate depth mipmaps for Hi-Z system.
        if( GFX_UseHiZCulling->Get<bool>() ) {
            // depth texture is gonna be a mipmap so we have to establish the mipmap chain
            m_pGraphicsInterface->GenerateMipmaps( g_sceneBuffer[0]->GetUnitDepth(), GL_TEXTURE_2D );
        }
        
        // FIXME
        // Emissive render target.
        if( GFX_UseHQCBloom->Get<bool>() ) {
            m_pGraphicsInterface->GenerateMipmaps( g_sceneBuffer[0]->GetUnitRT1(), GL_TEXTURE_2D );
        }
        
        // Currently required for eye adaptation.
        if( GFX_UseEyeAdaptation->Get<bool>() ) {
            m_pGraphicsInterface->GenerateMipmaps( m_pCompositeTex2->Image.GetId(), GL_TEXTURE_2D );
        }
        
        // Water reflection framebuffer.
        g_waterReflectionBuffer = (CFramebuffer*)pAllocator->Alloc( sizeof(CFramebuffer) );
        g_waterReflectionBuffer->Create( 160, 120, false, true, true, false, false, false, GL_RGBA8, GL_RGBA8, GL_RGBA8, GL_RGBA8 );
        g_waterReflectionBuffer->BindForDrawing();
        
        // Down-sampling.
        g_waterReflectionBuffer->Resize( w, h );
        
        SetViewportSize( /*0, 0,*/ 160, 120 );
        ClearColorDepthWithColor( 1.0, 1.0, 1.0, 1.0 ); // White color so water gamma keeps fine.
        
        g_waterReflectionBuffer->UnbindDrawing();
    }
    
    /**
     *  Beautiful water environment.
     */
    void CRenderer::PrepareFramebuffers( uint32_t msec )
    {
        //        if( !Render_UseReflections.Get<bool>() ) {
        return;
        //        }
        
        // Planar reflection.
        g_waterReflectionBuffer->BindForDrawing();
        
        SetViewportSize( /*0, 0,*/ 160, 120 );
        ClearWithColor( 0.0f, 0.0f, 0.0f, 1.0f );
        SetRenderTarget( ERenderTarget::RT0 );
        
        SetCulling( false, ECullMode::Back );
        
        g_pbEnv->RenderSky( msec, true /* reflected */ );
        
        SetCulling( true, ECullMode::Back );
        
        g_waterReflectionBuffer->UnbindDrawing();
        
    }
    
    /**
     *  Draw debug UI.
     */
    void CRenderer::RenderDebugUI()
    {
        if( p_CoreFont == NEKO_NULL ) {
            return;
        }
        
        const int32_t fntsz = 12;
        
        char cameraInfo[128];
        
        p_CoreFont->DrawString( COLOR_WHITE, 10, 10, fntsz, TEXTALIGN_DEFAULT, "Neko engine" );
        
        sprintf( cameraInfo, "camPos=%.2f %.2f %.2f camView=%.2f %.2f .%2f",
                g_Core->p_Camera->vEye.x, g_Core->p_Camera->vEye.y, g_Core->p_Camera->vEye.z,
                g_Core->p_Camera->vLook.x, g_Core->p_Camera->vLook.y, g_Core->p_Camera->vLook.z );
        p_CoreFont->DrawString( COLOR_WHITE, 10, 30, fntsz, TEXTALIGN_DEFAULT, cameraInfo );
        
        sprintf( cameraInfo, "worldTime: %i", (int32_t)g_pbEnv->m_fTime );
        p_CoreFont->DrawString( COLOR_WHITE, 10, 40, fntsz, TEXTALIGN_DEFAULT, cameraInfo );
        p_CoreFont->DrawString( COLOR_WHITE, 10, 60, fntsz, TEXTALIGN_DEFAULT, "hi-z culling: true" );
        
        sprintf( cameraInfo, "totalmem=%llu used=%llu",
                ByteInMegabyte( pMainAllocProxy->GetSize() ), ByteInMegabyte( pMainAllocProxy->GetUsedMem() ) );
        p_CoreFont->DrawString( COLOR_WHITE, 10, 80, fntsz, TEXTALIGN_DEFAULT, cameraInfo );
        
        
        sprintf( cameraInfo, "worldmem=%llu used=%llu",
                ByteInMegabyte( g_pbEnv->m_pWorldHandler->pChunkPoolAllocator->GetSize() ), ByteInMegabyte( g_pbEnv->m_pWorldHandler->pChunkPoolAllocator->GetUsedMem() ) );
        p_CoreFont->DrawString( COLOR_WHITE, 10, 90, fntsz, TEXTALIGN_DEFAULT, cameraInfo );
        
        
    }
    
    /**
     *  Render to texture.
     */
    void CRenderer::PreRender( uint32_t msec )
    {
        // We check collision in camera class.
        // It's getting called from Quadtree class, for the best performance
        // and because we got nice chunking in its Quadtree Chunk class.
        
        // Update view matrix.
        g_Core->p_Camera->UpdateMatrices();
        
        g_Core->p_Camera->OnPreCull();
        
        // Prepare framebuffer to render. Used for a Beautiful water and another stuff.
        PrepareFramebuffers( msec );
    }
    
    /**
     *  Clear screen color.
     */
    inline void CRenderer::ClearColor()
    {
#   if defined( USES_OPENGL )
        glClear ( GL_COLOR_BUFFER_BIT );
#   else
        
#   endif
    }
    
    /**
     *  Clear screen color with depth.
     */
    inline void CRenderer::ClearColorDepth()
    {
#   if defined( USES_OPENGL )
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
#   else
        
#   endif
    }
    
    /**
     *  Clear screen color & depth with a color.
     */
    inline void CRenderer::ClearColorDepthWithColor( const float &r, const float &g, const float &b, const float &a )
    {
#   if defined( USES_OPENGL )
        glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glClearColor( r, g, b, a );
#   else
        
#   endif
    }
    
    /**
     *  Set depth write mode.
     */
    void CRenderer::SetDepthWriteMode( const bool write )
    {
#   if defined( USES_OPENGL )
        glDepthMask( (GLboolean)write );
#   else
        
#   endif
    }
    
    const int mediumType_ = 1;
    
    const MediumDesc * getMediumDesc()
    {
        static MediumDesc mediumDesc;
        
        const float SCATTER_PARAM_SCALE = 0.0001f;
        mediumDesc.uNumPhaseTerms = 0;
        
        uint32_t t = 0;
        
        mediumDesc.PhaseTerms[t].ePhaseFunc = PhaseFunctionType::RAYLEIGH;
        mediumDesc.PhaseTerms[t].vDensity = (10.00f * SCATTER_PARAM_SCALE * Vec3(0.596f, 1.324f, 3.310f));
        t++;
        
        switch (mediumType_)
        {
            default:
            case 0:
                mediumDesc.PhaseTerms[t].ePhaseFunc =PhaseFunctionType::HENYEYGREENSTEIN;
                mediumDesc.PhaseTerms[t].vDensity = (10.00f * SCATTER_PARAM_SCALE * Vec3(1.00f, 1.00f, 1.00f));
                mediumDesc.PhaseTerms[t].fEccentricity = 0.85f;
                t++;
                mediumDesc.vAbsorption =(5.0f * SCATTER_PARAM_SCALE * Vec3(1, 1, 1));
                break;
                
            case 1:
                mediumDesc.PhaseTerms[t].ePhaseFunc = PhaseFunctionType::HENYEYGREENSTEIN;
                mediumDesc.PhaseTerms[t].vDensity = (15.00f * SCATTER_PARAM_SCALE * Vec3(1.00f, 1.00f, 1.00f));
                mediumDesc.PhaseTerms[t].fEccentricity = 0.60f;
                t++;
                mediumDesc.vAbsorption = (25.0f * SCATTER_PARAM_SCALE * Vec3(1, 1, 1));
                break;
                
            case 2:
                mediumDesc.PhaseTerms[t].ePhaseFunc = PhaseFunctionType::MIE_HAZY;
                mediumDesc.PhaseTerms[t].vDensity = (20.00f * SCATTER_PARAM_SCALE * Vec3(1.00f, 1.00f, 1.00f));
                t++;
                mediumDesc.vAbsorption = (25.0f * SCATTER_PARAM_SCALE * Vec3(1, 1, 1));
                break;
                
            case 3:
                mediumDesc.PhaseTerms[t].ePhaseFunc = PhaseFunctionType::MIE_MURKY;
                mediumDesc.PhaseTerms[t].vDensity = (30.00f * SCATTER_PARAM_SCALE * Vec3(1.00f, 1.00f, 1.00f));
                t++;
                mediumDesc.vAbsorption = (50.0f * SCATTER_PARAM_SCALE * Vec3(1, 1, 1));
                break;
        }
        
        mediumDesc.uNumPhaseTerms = t;
        
        return &mediumDesc;
    }
    
    static const uint LIGHT_LUT_DEPTH_RESOLUTION = 128;
    static const uint LIGHT_LUT_WDOTV_RESOLUTION = 512;
    // c16
    Vec3 vScatterPower;
    uint32_t uNumPhaseTerms;
    // c17
    Vec3 vSigmaExtinction;
    float pad2[1];
    // c18+MAX_PHASE_TERMS (4)
    uint32_t uPhaseFunc[MAX_PHASE_TERMS];
    // c22
    Vec4 vPhaseParams[MAX_PHASE_TERMS];
    
    static void PerFrame( MediumDesc const * pMediumDesc, SGLShader * shader )
    {
        
        // ComputePhaseLut
        const float SCATTER_EPSILON = 0.000001f;
        Vec3 total_scatter = Vec3(SCATTER_EPSILON, SCATTER_EPSILON, SCATTER_EPSILON);
        uNumPhaseTerms = pMediumDesc->uNumPhaseTerms;
        for (uint32_t p = 0; p < pMediumDesc->uNumPhaseTerms; ++p)
        {
            uPhaseFunc[p] = static_cast<uint32_t>(pMediumDesc->PhaseTerms[p].ePhaseFunc);
            Vec3 density = (pMediumDesc->PhaseTerms[p].vDensity);
            vPhaseParams[p] = Vec4(density.x, density.y, density.z, pMediumDesc->PhaseTerms[p].fEccentricity);
            total_scatter = total_scatter + density;
        }
        
        Vec3 absorption = (pMediumDesc->vAbsorption);
        vScatterPower.x = 1.0f - expf(-total_scatter.x);
        vScatterPower.y = 1.0f - expf(-total_scatter.y);
        vScatterPower.z = 1.0f - expf(-total_scatter.z);
        vSigmaExtinction = total_scatter + absorption;
        
        glDrawBuffer( GL_COLOR_ATTACHMENT0 );
        g_mainRenderer->m_pPhaseLutTarget->BindForDrawing();
        g_mainRenderer->ClearColor();
        g_mainRenderer->SetViewportSize( 1, LIGHT_LUT_WDOTV_RESOLUTION );
        shader->Use();
        
        //        uniform uint g_uNumPhaseTerms = 1;
        //        uniform vec4 g_vPhaseParams[4];
        //        uniform uint g_uPhaseFunc[4];
        shader->SetUniform( "g_uNumPhaseTerms", (int32_t)pMediumDesc->uNumPhaseTerms );
        glUniform4fv( glGetUniformLocation(shader->GetHandle(), "g_vPhaseParams"), pMediumDesc->uNumPhaseTerms, reinterpret_cast<GLfloat *>(&vPhaseParams[0]) );
        glUniform1iv( glGetUniformLocation(shader->GetHandle(), "g_uPhaseFunc"), pMediumDesc->uNumPhaseTerms, (const GLint*)&uPhaseFunc[0] );
        
        g_mainRenderer->g_pEyeMesh[0]->DrawArrays();
        shader->Next();
        g_mainRenderer->m_pPhaseLutTarget->UnbindDrawing();
        
        
        // ComputeLightLut
        g_mainRenderer->m_pLightLutTarget->BindForDrawing();
        g_mainRenderer->m_pComputeLightLut->Use();
        
        // Select subroutine.
        uint32_t indexes[1];
        indexes[0] = g_mainRenderer->m_pComputeLightLut->GetSubroutineIndex("Calculate", EShaderType::Pixel);
        glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &indexes[0] );
        
        
        glDrawBuffer( GL_COLOR_ATTACHMENT0 );
        g_mainRenderer->m_pLightLutTarget->BindForDrawing();
        g_mainRenderer->ClearColor();
        g_mainRenderer->SetViewportSize( LIGHT_LUT_DEPTH_RESOLUTION-1, LIGHT_LUT_WDOTV_RESOLUTION-1 );
        
        g_mainRenderer->m_pComputeLightLut->SetUniform( "phaseLutSampler", 0 );
        g_mainRenderer->BindTexture( 0, g_mainRenderer->m_pPhaseLutTarget->GetUnitRT0()  );
        
        const static float sunDist = 1e+05;
        const Vec3 sunPosW = Vec3::Normalize(g_pbEnv->m_vSunDirection) * sunDist;
        g_mainRenderer->m_pComputeLightLut->SetUniform( "g_vLightPos", sunPosW );
        g_mainRenderer->m_pComputeLightLut->SetUniform( "g_vEyePosition", g_Core->p_Camera->vEye );
        g_mainRenderer->m_pComputeLightLut->SetUniform( "g_vSigmaExtinction", vSigmaExtinction );
        g_mainRenderer->m_pComputeLightLut->SetUniform( "g_vScatterPower", vScatterPower * 1000.0f);
        
        g_mainRenderer->m_pComputeLightLut->SetUniform( "g_fLightZFar", 3000.0f );
        g_mainRenderer->m_pComputeLightLut->SetUniform( "g_fLightZNear", 0.1f );
        
        g_mainRenderer->g_pEyeMesh[0]->DrawArrays();
        g_mainRenderer->m_pComputeLightLut->Next();
        g_mainRenderer->m_pLightLutTarget->UnbindDrawing();
        
    }
    
    //! Light Source Description
    struct LightDesc
    {
        LightDesc() {
            
        }
        
        LightType eType;                //!< Type of light source
        ncMatrix4 mLightToWorld;        //!< Light clip-space to world-space transform
        Vec3 vIntensity;                //!< Color of light
        union
        {
            //! LightType = Directional
            struct {
                Vec3 vDirection;                        //!< Normalized light direction
            } Directional;
            
        };
    };
    
    
    const LightDesc * getLightDesc()
    {
        static LightDesc lightDesc;
        const float LIGHT_RANGE = 50.0f;
        
        Vec3 vLightPos;
        ncMatrix4 mLightViewProj;
        
        //        ncMatrix4 mLightViewProj_Inv = ncMatrix4(mLightViewProj);
        //        mLightViewProj_Inv.Inverse();
        
        lightDesc.vIntensity = 1.0;//(getLightIntensity());
        lightDesc.mLightToWorld = g_pbEnv->m_pbEnvShadows->m_mShadowViewMatrices[0];// (mLightViewProj_Inv);
        
        Vec3 vLightDirection = -g_pbEnv->GetOutdoorLightDirection();
        //        vLightDirection.Normalize();
        lightDesc.eType = LightType::DIRECTIONAL;
        lightDesc.Directional.vDirection = (vLightDirection);
        
        return &lightDesc;
    }
    
    float fGridSectionSize = 0.0f;
    float fTargetRaySize = 0.0f;
    
    void PerVolume( LightDesc const * pLightDesc )
    {
        VolumeDesc volumeDesc;
        {
            volumeDesc.fTargetRayResolution = 12.0f;
            volumeDesc.uMaxMeshResolution = 2048;
            volumeDesc.fDepthBias = 0.0f;
            volumeDesc.eTessQuality = ETessellationQuality::High;
        }
        
        Vec4 vw1 = Vec4(-1, -1,  1, 1);
        Vec4 vw2 = Vec4( 1,  1,  1, 1);
        
        vw1.TransformVector(pLightDesc->mLightToWorld);
        vw2.TransformVector(pLightDesc->mLightToWorld);
        vw1 = vw1 / vw1.w;
        vw2 = vw2 / vw2.w;
        float fCrossLength = ((vw1).xyz() - (vw2).xyz()).Length();
        float fSideLength = sqrtf(0.5f*fCrossLength*fCrossLength);
        fGridSectionSize = fSideLength / static_cast<float>(volumeDesc.uMaxMeshResolution / 64);
        fTargetRaySize = volumeDesc.fTargetRayResolution;
        
    }
    
    /**
     *  Render to shader.
     */
    void CRenderer::RenderToShader( uint32_t time, ESceneEye eye )
    {
#   if !defined( NEKO_SERVER )
        //! Mobile devices should not use this rendering method.
#   if !defined( iOS_BUILD )
        
        //  Offscreen rendering..
        /**********************************************/
        
        g_pbEnv->RenderShadows();        // Global sun shadow and light shadows.
        // Draw shadows.
        g_pbEnv->RenderCubemap( time );
        
//        PerFrame( getMediumDesc(), m_pComputePhaseLut );
//        PerVolume( getLightDesc() );
        // BeginAccumulation_UpdateMediumLUT
        
        
        //        uniform sampler2D lightLutSampler;
        //        uniform sampler2D phaseLutSampler;
        //
        //        uniform vec3 g_vLightPos, g_vEyePosition, g_vSigmaExtinction, g_vScatterPower;
        //        uniform float g_fLightZFar, g_fLightZNear;
        
        
        
        
        //  Rendering the world..
        /**********************************************/
        // Scene buffer.
        g_sceneBuffer[eye]->BindForDrawing();
        
        // Clear screen.
        ClearColorDepthWithColor( 0.0f, 0.0f, 0.0f, 1.0f );
        SetViewportSize( /*0, 0, */ renderWidth, renderHeight );
        
        // Render our great world!
        
        int32_t flags;
        flags = (int32_t)EWorldFlag::PlayerEye;
        
        // Render beautiful environment.
        g_pbEnv->Render( eye, flags, time );
        
 
        
        
        /**********************************************
         *  Hi-Z Render pass.
         **********************************************/
        if( GFX_UseHiZCulling->Get<bool>() ) {
            // Bind empty vertex array.
            glBindVertexArray( m_pScreenSinglePass->m_VAO );
            
            SetDepthTest( true );
            SetRenderTarget( ERenderTarget::None );
            
            // Use Hi-Z shader.
            hiZshader->Use();
            
            // Using scene depth..
            BindTexture( 0, g_sceneBuffer[0]->GetUnitDepth() );
            g_mainRenderer->SetDepthMode( ECompareMode::Always );
            
            // Calculate the number of mipmap levels for NPOT texture.
            int32_t numLevels = 1 + (int32_t)floorf(log2f(fmaxf((float)renderWidth, (float)renderHeight)));
            int32_t currentWidth = renderWidth;
            int32_t currentHeight = renderHeight;
            
            // Downsample depth texture.
            for( int32_t i(1); i < numLevels; ++i ) {
                hiZshader->SetUniform( _renderUniforms[RLastMipSize], currentWidth, currentHeight);
                
                // Calculate next viewport size
                currentWidth /= 2;
                currentHeight /= 2;
                // Ensure that the viewport size is always at least 1x1
                currentWidth = currentWidth > 0 ? currentWidth : 1;
                currentHeight = currentHeight > 0 ? currentHeight : 1;
                
                SetViewportSize(currentWidth, currentHeight);
                
                // Bind next level for rendering (first restrict fetches only to previous level)
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i - 1 );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i - 1 );
                
                glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_sceneBuffer[0]->m_maxTextures[0], i );
                
                // dummy draw command as the full screen quad is generated completely by a geometry shader
                glDrawArrays( GL_POINTS, 0, 1 );
            }
            
            // Reset mipmap level range for the depth image.
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numLevels - 1 );
            
            // Restore depth
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_sceneBuffer[0]->GetUnitDepth(), 0);
            
            // Reenable color buffer writes, reset viewport and reenable depth test.
            g_mainRenderer->SetDepthMode( ECompareMode::Lequal );
            g_mainRenderer->SetViewportSize(renderWidth, renderHeight);
            // Restore draw buffers ( were set to none ).
            const static GLenum restoreModes[4] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3
            };
            
            glDrawBuffers( 4, restoreModes );
            
            hiZshader->Next();
            SetDepthTest( false );
        }
        
        g_sceneBuffer[eye]->UnbindDrawing();
        
        //! Render offscreen things now.
        g_pbEnv->RenderOffscreen( time );      // Particles, pointlights, etc..
        
        
        
        
        
        
        //  Render pass 1.
        /**********************************************/
        //        glEnable(GL_FRAMEBUFFER_SRGB);
        
        //! Composite buffer drawing.
        g_compositeBuffer->BindForDrawing();
        SetViewportSize(renderWidth, renderHeight);
        // Color data of these will be written in shader itself!
        const static GLenum restoreModes3[3] = {
            GL_COLOR_ATTACHMENT0, //  Current color.
            GL_COLOR_ATTACHMENT1, //  Previous post processing sampler
            GL_COLOR_ATTACHMENT2, //  Emissive from the previous sampler.
            // Third render target will be used below.
        };
        
        glDrawBuffers( 3, restoreModes3 );
        
        // Below stuff is commented because we want to render in two targets.
        //        SetRenderTarget( ERenderTarget::RT2 );
        //        glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_pCompositeTex->Image.GetId(), 0 );
        
        ClearColorDepth();
        //!glReadBuffer( GL_COLOR_ATTACHMENT0 );
        
        //! Here comes shader.
        sceneShader->Use();
        
        /**  Scene texture.  **/
        BindTexture( 0, g_sceneBuffer[eye]->GetUnitRT0() );
        
        /**  Scene depth.    **/
        BindTexture( 1, g_sceneBuffer[eye]->GetUnitDepth() );
        
        /**  Scene PBR samplers.   **/
        BindTexture( 2, g_sceneBuffer[eye]->GetUnitRT2() );
        
        /**  Scene normal.   **/
        BindTexture( 3, g_sceneBuffer[eye]->GetUnitRT1() );
        
        /**  Scene emissive.   **/
        BindTexture( 4, g_sceneBuffer[eye]->GetUnitRT3() );
        
        // Render postprocess shadows ( 5th sampler ).
        g_pbEnv->m_pbEnvShadows->UpdatePostProcess(sceneShader);
        
        /**   Another nyashes.      **/
        
        /**     Noise sampler.      **/
        BindTexture( 6, f_AssetBase->p_MaterialBase->noiseMaterial->m_pDiffuse->Image.GetId() );
        
        /**     Pointlight map.     **/
        BindTexture( 7, g_pbEnv->m_pPointLightRenderer->pointLightFramebuffer.GetUnitRT0() );
        
        /**    Particle sampler.     **/
        BindTexture( 8, g_pbEnv->m_pParticleFramebuffer->GetUnitRT0() );
        
        /**     Environment map for PBR.        **/
        BindTexture( 9, g_pbEnv->m_pEnvironmentCubemap->Image.GetId(), TextureTarget::TextureCube  );
        
        /**     Bind BRDF preset material for PBR.     **/
        BindTexture( 10, g_pMiscFramebuffer->GetUnitRT0() );
        
        
        SetViewportSize( /*0.0f, 0.0f, */ renderWidth, renderHeight );
        
        static ncMatrix4 invProjView;
        
        sceneShader->SetUniform( _renderUniforms[RLightPosition_Time], g_pbEnv->GetCurrentLightDir(), g_Core->GetTime() / 10000.0f );
        sceneShader->SetUniform( _renderUniforms[RSkyColor], g_pbEnv->GetSkyLightColor() );
        sceneShader->SetUniform( _renderUniforms[RSunColor], g_pbEnv->GetSunLightColor() );
        
        sceneShader->SetUniform( _renderUniforms[RGlobalFogColor], g_pbEnv->GetGlobalFogColor() );
        
        const static float sunDist = 1e+05;
        const Vec3 sunPosW = Vec3::Normalize(g_pbEnv->m_vSunDirection) * sunDist;
        //        sunPosW.TransformVector( g_Core->p_Camera->ViewMatrix );
        
        sceneShader->SetUniform( _renderUniforms[RSunWorldPos], sunPosW );
        
        float uniformLightX;    // Projected light origin to 2d space.
        float uniformLightY;
        
        
        // *************************************************
        //        Invert view projection matrix.
        //   Note: used in shader to get world information.
        // *************************************************
        
        
        invProjView = g_Core->p_Camera->ViewProjectionMatrix;
        NekoUtils::gluInvertMatrix( invProjView.m, tempMatrix );
        
        // Inverted view projection.
        sceneShader->SetUniform( _renderUniforms[RInvertedViewProj], 1, false, tempMatrix );
        
        // Camera position.
        sceneShader->SetUniform( _renderUniforms[RCameraPosition], g_Core->p_Camera->vEye );
        
        // Update light direction.
        static GLfloat winX = 0.0f;
        static GLfloat winY = 0.0f;
        static GLfloat winZ = 0.0f;
        
        //! Get sun position on screen.
        NekoUtils::gluProject( g_pbEnv->GetSunLightPosition().x,
                              g_pbEnv->GetSunLightPosition().y,
                              g_pbEnv->GetSunLightPosition().z,
                              
                              g_Core->p_Camera->ViewMatrix.m,
                              g_Core->p_Camera->ProjectionMatrix.m,
                              g_pGraphicsManager->GetCurrentInterface()->GetViewport(),
                              
                              &winX,
                              &winY,
                              &winZ );
        
        uniformLightX = winX / renderAspectWidth;
        uniformLightY = winY / renderAspectHeight;
        
        //! Sun position.
        sceneShader->SetUniform( _renderUniforms[RLight2DPosition], uniformLightX, uniformLightY );
        
        //! Draw to screen now.
        g_pEyeMesh[eye]->DrawArrays();
        
        sceneShader->Next();
        
        //  Downsample/blur render pass.
        /**********************************************/
        if( GFX_UseHQCBloom->Get<bool>() ) {
            // Bind empty vertex array.
            glBindVertexArray( m_pScreenSinglePass->m_VAO );
            // Select emissive sampler.
            SetRenderTarget( ERenderTarget::RT1 );
            
            // Use downsample shader.
            downsampleShader->Use();
            // Using scene emissive..
            BindTexture( 0, g_compositeBuffer->GetUnitRT1() );
            
            // Calculate the number of mipmap levels for NPOT texture.
            //            int32_t numLevels = 1 + (int32_t)floorf(log2f(fmaxf((float)renderWidth, (float)renderHeight)));
            int32_t currentWidth = renderWidth;
            int32_t currentHeight = renderHeight;
            
            float fBloomSpread = 1.0;
            
            // Downsample depth texture.
            for( int32_t i(1); i < MAX_BLOOM_PASSES; ++i ) {
                // Downsample subroutine.
                glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_iDSRoutine[0] );
                
                if( i > 1 ) {
                    fBloomSpread = 1.0f;
                } else {
                    fBloomSpread = 0.5f;
                }
                
                if( i == 2 ) {
                    fBloomSpread = 0.75f;
                }
                
                // Calculate next viewport size
                currentWidth /= 2;
                currentHeight /= 2;
                // Ensure that the viewport size is always atleast 1x1
                currentWidth = currentWidth > 0 ? currentWidth : 1;
                currentHeight = currentHeight > 0 ? currentHeight : 1;
                
                SetViewportSize(currentWidth, currentHeight);
                
                downsampleShader->SetUniform( _renderUniforms[RTexelSize], (float)(1.0 / currentWidth) * 0.5f, (float)(1.0 / currentHeight) * 0.5f );
                // Bind next level for rendering
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i - 1 );
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i - 1 );
                
                glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_compositeBuffer->GetUnitRT1(), i );
                // dummy draw command as the full screen quad is generated completely by a geometry shader
                glDrawArrays( GL_POINTS, 0, 1 );
                
                // Vertical and horizontal blurs.
                for( int32_t j(0); j < 2; ++j ) {
                    // Downsampled texture.
                    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_compositeBuffer->GetUnitRT1(), i );
                    
                    // Select subroutine for blurring.
                    glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_iDSRoutine[j + 1] );
                    
                    downsampleShader->SetUniform( _renderUniforms[RBlurSize], (m_fBlurSize * 0.5f + j) * fBloomSpread );
                    glDrawArrays( GL_POINTS, 0, 1 );
                }
            }
            
            // Reset mipmap level range for the emissive image.
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MAX_BLOOM_PASSES - 1 );
            
            // Reset viewport.
            g_mainRenderer->SetViewportSize(renderWidth, renderHeight);
            
            // Final pass.
            glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_iDSRoutine[3] );
            // Restore sampler
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_compositeBuffer->GetUnitRT1(), 0);
            
            // dummy draw command as the full screen quad is generated completely by a geometry shader
            glDrawArrays( GL_POINTS, 0, 1 );
            
            downsampleShader->Next();
        }
        
        //        g_compositeBuffer->UnbindDrawing();
        
        //  Render pass 2.
        // This pass is used as additional stuff for deferred shading.
        /**********************************************/
        SetRenderTarget( ERenderTarget::RT2 );
        compositeShader3->Use();
        
        
        /**  Scene texture.  **/
        BindTexture( 0, g_compositeBuffer->GetUnitRT0() );
        /**  Shadow postprocessed texture.  **/
        BindTexture( 1, g_compositeBuffer->GetUnitRT1() );
        /**  Scene depth.  **/
        BindTexture( 2, g_sceneBuffer[0]->GetUnitDepth() );
        
        g_pEyeMesh[eye]->DrawArrays();
        
        compositeShader3->Next();
        g_compositeBuffer->UnbindDrawing();
        
        //  Pre-render pass 3.
        // Temporal anti-aliasing and velocity buffer creationism.
        /**********************************************/
        if( m_pTemporalAAPost != NEKO_NULL ) {
            g_pCopyBuffer->BindForDrawing();
            
            m_pMiscShader->Use();
            SetRenderTarget( ERenderTarget::RT0 );
            
            m_pMiscShader->SetUniform( m_iMiscUniforms[0], 1, false, g_Core->p_Camera->ViewProjectionMatrix.m  );
            m_pMiscShader->SetUniform( m_iMiscUniforms[1], 1, false, tempMatrix);//g_Core->p_Camera->ViewMatrix.m );
            //        m_pMiscShader->SetUniform( m_iMiscUniforms[2], g_Core->p_Camera->GetPerspectiveProjectionCornerRay(activeSample.x, activeSample.y));
            
            BindTexture( 0, g_sceneBuffer[0]->GetUnitDepth() );
            BindTexture( 1, g_sceneBuffer[0]->GetUnitRT1() );
            
            g_pEyeMesh[0]->DrawArrays();
            m_pMiscShader->Next();
            
            g_pCopyBuffer->UnbindDrawing();
            
            // Temporal anti-aliasing process (uses RT2!!!).
            m_pTemporalAAPost->Render();
        }
        
        //  Render pass 3.
        // This pass is used as pre-final postprocessing ( motion blur, dof and another thingies that could affect gamma ).
        /**********************************************/
        g_compositeBuffer->BindForDrawing();
        SetViewportSize(renderWidth, renderHeight);
        
        //! Draw final pass into the same framebuffer.
        SetRenderTarget( ERenderTarget::RT3 );
        glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_pCompositeTex2->Image.GetId(), 0 );
        
        compositeShader->Use();
        
        //        // Previous view projection for motion blur.
        //        compositeShader->SetUniform( _renderUniforms[RPreviousViewProj], 1, false, g_Core->p_Camera->prevViewProj.m );
        //        // Inverted view projection.
        //        compositeShader->SetUniform( _renderUniforms[RInvertedViewProj2], 1, false, tempMatrix );
        
        /**  Scene texture.  **/
        if( m_pTemporalAAPost != NEKO_NULL ) {
            BindTexture( 0, m_pTemporalAAPost->g_pReprojectionBuffer->GetUnitRT0() ); // follow the hierarchy! // m_pCompositeTex->Image.GetId() );
        } else {
            BindTexture( 0, g_compositeBuffer->GetUnitRT2() ); // follow the hierarchy!
        }
        
        /**  Scene depth.    **/
        BindTexture( 1, g_sceneBuffer[eye]->GetUnitDepth() );
        
        /**  Emissive sampler.  **/
        BindTexture( 2, g_compositeBuffer->GetUnitRT1() );
        
        //        /**  Scene lens texture. **/
        //        BindTexture( 2, _lens_texture->m_pDiffuse->GetId() );
        
        g_pEyeMesh[eye]->DrawArrays();
        
        // Unbind all textures.
        BindTexture( 0, 0 );
        
        compositeShader->Next();
        
        
//        m_pVolumetricLight->Use();
//        
//        //        uniform int g_uMeshResolution = 32;
//        //        uniform float g_fLightZFar = 3000.0;
//        //        uniform float g_fTargetRaySize = 12.0f;
//        //        uniform vec2 g_vOutputViewportSize = vec2(640.0, 480.0);
//        //
//        //        uniform mat4 g_mViewProj;
//        //        uniform mat4 g_mProj;
//        //
//        //        uniform mat4 g_mLightProj[4];   // viewproj!
//        //        uniform mat4 g_mLightProjInv[4];
//        //        uniform mat4 g_mLightToWorld;   // view matrix only
//        //
//        //        uniform float g_fGodrayBias = 0.5;
//        //        uniform vec3 g_vEyePosition;
//        //        uniform vec3 g_vLightPos;
//        
//        m_pVolumetricLight->SetUniform( "g_mViewProj", 1, false, g_Core->p_Camera->ViewProjectionMatrix.m );
//        m_pVolumetricLight->SetUniform( "g_mProj", 1, false, g_Core->p_Camera->ProjectionMatrix.m );
//        
//        m_pVolumetricLight->SetUniform( "g_vLightPos", g_pbEnv->GetCurrentLightDir() * 1000.0f );
//        m_pVolumetricLight->SetUniform( "g_vEyePosition", g_Core->p_Camera->vEye );
//        
//        for( int i = 0; i < 4; ++i ) {
//            float lightViewProjInv[16];
//            
//            NekoUtils::gluInvertMatrix( g_pbEnv->m_pbEnvShadows->m_mShadowViewMatrices[i].m, lightViewProjInv );
//            
//            m_pVolumetricLight->SetUniform( "g_mLightProj", 1, false, g_pbEnv->m_pbEnvShadows->m_mShadowViewMatrices[i].m );
//            m_pVolumetricLight->SetUniform( "g_mLightProjInv", 1, false, lightViewProjInv );
//        }
//        
//        static float lightViewProjInv[16];
//        ncMatrix4 lightViewProj = g_pbEnv->m_pbEnvShadows->m_lightProjMatrix * g_pbEnv->m_pbEnvShadows->m_lightViewMatrix;
//        NekoUtils::gluInvertMatrix( lightViewProj.m, lightViewProjInv );
//
//        m_pVolumetricLight->SetUniform( "g_mLightToWorld", 1, false, lightViewProjInv );
//        glPatchParameteri(GL_PATCH_VERTICES, 4);
//        m_pScreenSinglePass->BindVertexArray();
//        glDrawArrays( GL_PATCHES, 0, 4 * 32 * 32 );
//        glBindVertexArray(0);
//        m_pVolumetricLight->Next();
        
        
        
        // Draw UI.
        p_UserInterface->Render();
        
        // Draw debug UI.
        RenderDebugUI();
        
        
        // Bind empty vertex array.
        glBindVertexArray( m_pScreenSinglePass->m_VAO );
        
        // Use downsample shader.
        downsampleShader->Use();
        // Using scene emissive..
        BindTexture( 0, m_pCompositeTex2->Image.GetId() );
        
        // Calculate the number of mipmap levels for NPOT texture.
        int32_t currentWidth = renderWidth;
        int32_t currentHeight = renderHeight;
        
        // Downsample depth texture.
        for( int32_t i(1); i < 3; ++i ) {
            // Downsample subroutine.
            glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &m_iDSRoutine[0] );
            
            // Calculate next viewport size
            currentWidth /= 2;
            currentHeight /= 2;
            // Ensure that the viewport size is always atleast 1x1
            currentWidth = currentWidth > 0 ? currentWidth : 1;
            currentHeight = currentHeight > 0 ? currentHeight : 1;
            
            SetViewportSize(currentWidth, currentHeight);
            
            downsampleShader->SetUniform( _renderUniforms[RTexelSize], (float)(1.0 / currentWidth * 0.5f), (float)(1.0 / currentHeight * 0.5f) );
            // Bind next level for rendering
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i - 1 );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i - 1 );
            
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_pCompositeTex2->Image.GetId(), i );
            // dummy draw command as the full screen quad is generated completely by a geometry shader
            glDrawArrays( GL_POINTS, 0, 1 );
        }
        
        // Reset mipmap level range for the emissive image.
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2 );
        
        // Reset viewport.
        g_mainRenderer->SetViewportSize(renderWidth, renderHeight);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_pCompositeTex2->Image.GetId(), 0);
        // dummy draw command as the full screen quad is generated completely by a geometry shader
        glDrawArrays(GL_POINTS, 0, 1);
        downsampleShader->Next();
        

        
        g_compositeBuffer->UnbindDrawing(); // unbind framebuffer
        
        // Update occlusion world culling.
        g_pbEnv->m_pWorldHandler->UpdateCulling();
        
        if( m_pEyeAdaptationPost != NEKO_NULL ) {
            m_pEyeAdaptationPost->Render(frameMsec);
        }
        
        
        
        //  Render pass 4.
        // Final drawing pass, uses all passes rendered before and applies effects such as
        // gamma correction, filmic tonemapping, etc...
        /**********************************************/
        compositeShader2->Use();
        
        ClearColor();
        SetViewportSize( renderWidth, renderHeight );
        SetRenderTarget(ERenderTarget::RT0);
        
        /**  Scene texture.  **/
        BindTexture(0, m_pCompositeTex2->Image.GetId() );
        //        BindTexture( 2, m_pLutSampler->m_pDiffuse->Image.GetId() );
        BindTexture( 1, m_pEyeAdaptationPost->g_pBrightnessRTTemp->GetUnitRT0() );
        
        // Don't want to use it together with PBR.
        //        compositeShader2->SetUniform( "lutSampler", 4 );
        //        BindTexture( 4, f_AssetBase->prop3dLut->m_pDiffuse->Image.GetId(), TextureTarget::Texture3D );
        
        g_pEyeMesh[eye]->DrawArrays();
        
        compositeShader2->Next();
        
        //        glDisable(GL_FRAMEBUFFER_SRGB);
        
#   endif
        
        
        // Set previous camera view projection.
        g_Core->p_Camera->OnAfterFrame();
#   endif
    }
    
    /**
     *  Draw simple 2d quad.
     */
    void CRenderer::RenderQuadAt( Vec4 rect, Vec4 & color )
    {
        Vec4 adjustedRect( rect );
        // Adjust element size on interface.
        AdjustElemSize( &adjustedRect.x, &adjustedRect.y, &adjustedRect.z, &adjustedRect.w );
        
        // Use 2d shader.
        draw2dshader->Use();
        draw2dshader->SetUniform( _renderUniforms[RDraw2D_ScaleOrigin], adjustedRect.z, adjustedRect.w, adjustedRect.x, adjustedRect.y  );
        draw2dshader->SetUniform( _renderUniforms[RDraw2D_Color], color );
        
        g_pEyeMesh[0]->DrawArrays();
        
        draw2dshader->Next();
    }
    
    /**
     *  Read pixel using graphics API.
     */
    void CRenderer::ReadPixel( int32_t x, int32_t y, float & z )
    {
#   if defined( USES_OPENGL )
        // Unbind any bound framebuffer.
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
        glReadPixels( x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );
#   else
        
        // TODO
        
#   endif
    }
    
    /**
     *  Get an intersection point from mouse coordinates in the world space.
     *
     *  @param Intersection Intersection point.
     */
    void CRenderer::IntersectionPoint( Vec3 &Intersection )
    {
        int32_t mouseX = g_Core->p_Input->GetMouseX();
        int32_t mouseY = g_Core->p_Input->GetMouseY();
        mouseY = Window_Height->Get<int>() - mouseY;
        
        float z = 0.0f;
        
        // Read pixels.
        ReadPixel( (uint32_t)(mouseX + 0.5f), (uint32_t)(mouseY + 0.5f), z );
        
        NekoUtils::gluUnProject(mouseX, mouseY, z, g_Core->p_Camera->ViewMatrix.m, g_Core->p_Camera->ProjectionMatrix.m, g_pGraphicsManager->GetCurrentInterface()->GetViewport(), &Intersection.x, &Intersection.y, &Intersection.z );
    }
    
    /**
     *  Take a request off the queue.
     */
    SFunctionCallInfo * CRenderer::DequeueRequests()
    {

        return NEKO_NULL;
    }
    
    /**
     *  Lets see the world!
     */
    void CRenderer::Render( uint32_t msec )
    {
        if( !Initialized ) {
            return;
        }
        
        frameMsec = msec;
        
//        
//        // If we have requested events, process them.
//        if( m_pRenderQueue.isEmpty() == false ) {
//            //            m_pRenderLock->Lock();
//            
//            // Process request on this thread.
//            SFunctionCallInfo * found = DequeueRequests();
//            if( found != NEKO_NULL && found->m_pFunction != NEKO_NULL ) {
//                found->m_pFunction( found->m_pArg );
//                found->m_pFunction = NEKO_NULL;
//                
//                // Destroy function info created before.
//                delete found; // @todo - use Neko memory manager.
//            }
//            
//            //            m_pRenderLock->Unlock();
//        }
        
        SFunctionCallInfo * info = NEKO_NULL;
        if( m_pRenderQueue != NEKO_NULL && m_pRenderQueue->Get( (void**)&info ) == 0 ) {
            if( info != nullptr ) {
                info->m_pFunction( info->m_pArg );
                info->m_pFunction = NEKO_NULL;
                
                // Destroy function info created before.
                delete info; // @todo - use Neko memory manager.
            }
        }
        
        // Update user interface.
        p_UserInterface->Frame( msec );
        
        // Here we render the world.
        
        // Render some stuff first.
        PreRender( msec );
        
        // Here you can tell what you want to render.
#   if defined( OCULUSVR_BUILD )
        
        if( Render_OVR.Get<bool>() ) {
            RenderToShader( EYE_RIGHT );
            RenderToShader( EYE_LEFT );
        }
        else {
            RenderToShader( EYE_FULL );
        }
#   else
        RenderToShader( msec, EYE_FULL );
        
#   endif
        
        // Update player movement.
        g_Core->p_Camera->Frame( msec );
        
        /*      Update methods.     */
        
        // Update game environment.
        g_pbEnv->Update( msec );
        
        // Load assets which needs renderer context.
        f_AssetBase->Frame( msec, false );
        
        // SwapBuffer is called from main Windows class.
        // Context flush is called from MacWindow class.
    }
    
    /**
     *  Refresh graphical values.
     */
    void CRenderer::Refresh( void )
    {
        
    }
    
    /**
     *      Remove every active object.
     *      Don't touch sky shader.
     */
    void CRenderer::RemoveWorld( const char * msg )
    {
        g_Core->p_Console->Print( LOG_INFO, "Removing current world. Reason: \"%s\".\n", msg );
        g_Core->p_Console->Print( LOG_NONE, "\n");
        
        // Remove g_pbEnv.
        g_pbEnv->Destroy();
        
        // Remove BSP level if exists.
        //        g_staticWorld->visibleFaces.ClearAll();
        //        g_staticWorld->Unload();
        
        // Reset player camera.
        g_Core->p_Camera->Reset();
    }
    
    /**
     *  Renderer shutdown.
     */
    void CRenderer::Shutdown( void )
    {
        if( !Initialized ) {
            return;
        }
        
        RemoveWorld( "Renderer shutdown." );
        
        // Object cleanup.
        DeleteMainBuffers();
        
        // UI.
        p_UserInterface->Shutdown();
        
        // Font system.
        p_CoreFont->Shutdown();
        
        // Remove all shaders and assets.
        f_AssetBase->Shutdown();
        
        // Shutdown graphics interface.
        m_pGraphicsInterface->Shutdown();
        
#   if defined( USES_LUA )
        g_Core->p_ScriptBase->Shutdown();   // MOVE ME!
#   endif
        
        // Remove event watcher.
        m_pRenderQueue->DestroyComplete(NEKO_NULL);
        pAllocator->Dealloc(m_pRenderQueue);
    }
    
    /**
     *  Delete all available buffers.
     */
    void CRenderer::DeleteMainBuffers( void )
    {
        g_Core->p_Console->Print( LOG_INFO, "Cleaning up renderer..\n" );
        // Vertex buffers.
        
        uint32_t i;
        
        // Remove postprocess effects.
        if( m_pTemporalAAPost != NEKO_NULL ) {
            m_pTemporalAAPost->Disable();
            pAllocator->Dealloc( m_pTemporalAAPost );
        }
        
        if( m_pEyeAdaptationPost != NEKO_NULL ) {
            m_pEyeAdaptationPost->Disable();
            pAllocator->Dealloc( m_pEyeAdaptationPost );
        }
        
        m_pScreenSinglePass->Destroy();
        pAllocator->Dealloc( m_pScreenSinglePass );
        
        // delete framebuffers
        
        g_pCopyBuffer->Delete();
        pAllocator->Dealloc( g_pCopyBuffer );
        
        g_pFrameHistoryBuffer->Delete();
        pAllocator->Dealloc( g_pFrameHistoryBuffer );
        
        g_compositeBuffer->Delete();
        pAllocator->Dealloc( g_compositeBuffer );
        
        g_pMiscFramebuffer->Delete();
        pAllocator->Dealloc( g_pMiscFramebuffer );
        
        // Delete scene buffers.
        if( Render_OVR->Get<bool>() )
        {
            for( i = 0; i < 3; ++i ) {
                g_sceneBuffer[i]->Delete();
                g_pEyeMesh[i]->Destroy();
                
                pAllocator->Dealloc( g_pEyeMesh[i] );
            }
            
        } else {
            g_pEyeMesh[EYE_FULL]->Destroy();
            g_sceneBuffer[EYE_FULL]->Delete();
            
            pAllocator->Dealloc( g_pEyeMesh[EYE_FULL] );
        }
    }
    
    /**
     *  Take the game screenshot.
     *  FIX ME: Flipped image.
     */
    void CRenderer::MakeScreenshot( void )
    {
        const uint32_t screenW = Render_Width->Get<int>();
        const uint32_t screenH = Render_Height->Get<int>();
        
        SMemoryTempFrame * tempImageData;
        Byte * imageData;
        //g_sceneBuffer[0]->BindForReading(); // Don't need to bind current framebuffer to capture the screen.
        // Create temporary writer.
        tempImageData = _PushMemoryFrame( pLinearAllocator2 );
        // Byte data.
        imageData = (Byte*)PushMemory( tempImageData, sizeof(Byte) * screenW * screenH * 3 ); //new Byte[screenW * screenH * 3];
        
        // Read pixels now.
        glReadPixels( 0, 0, screenW, screenH, GL_RGB, GL_UNSIGNED_BYTE, imageData );
        
        //glGetTexImage( GL_TEXTURE_2D, GL_RGB, GL_RGB8, GL_UNSIGNED_BYTE, imageData );
        // Copy data to image data and create new file.
        CImageLoader::CreateImage( screenW, screenH, imageData, EImageInfoType::BMP_IMAGE, "screenshot" );
        
        _PopMemoryFrame( tempImageData );
        
    }
    
    CRenderer * g_mainRenderer;
    
}
#   endif // NEKO_SERVER
