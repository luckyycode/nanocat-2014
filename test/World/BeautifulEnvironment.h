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
//  BeautifulEnvironment.h
//  Beautiful environment creator.
//
//  Created by Neko Code on 11/30/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__BeautifulEnvironment__
#define __Nanocat__BeautifulEnvironment__

#include "../Platform/Shared/SystemShared.h"
#include "Landscape/LandscapeChunk.h"
#include "Landscape/ChunkHandler.h"
#include "Ocean.h"
#include "ParticleEngine.h"
#include "Foliage.h"
#include "MeshBase.h"
#include "../Graphics/Renderer/ShadowRenderer.h"

namespace Neko {

    //!  For the whole world scene.
    static const int32_t MAX_DEFERRED_POINT_LIGHTS = 2048;
  
    /// Time of day mode.
    enum class ETimeMode : int32_t {
        Realistic = 0,
        Manual
    };
    
    enum class EFogType : int32_t {
        None = 0,
        Height,
        Directional,
    };

    enum class EColorRangeType : int32_t {
        Auto = 0,
        HDR,
        LDR
    };
    
    enum class EWeatherType : int32_t {
        Clear = 0,
        Storm,
        Dust,
        Fog,
        Rainy
    };
    
    ///  Flags used to show what we can currently render.
    enum EWorldRenderingFlags {
        CreatedLandscape     = 0x1,  // Can render landscape.
        CreatedOcean         = 0x2,  // Can render ocean.
        CreatedParticles     = 0x4,  // Can render fx.
        CreatedSky           = 0x8,  // Can render sky.
        CreatedEnvironment   = 0x16  // Can render environment.
    };
   
    /***
     *
     *      Beautiful environment.
     *
     **/
    
#ifndef NEKO_SERVER
    
    ///  Deferred point lights.
    struct COffscreenPointlight
    {
    public:
        
        /**
         *  Constructor.
         */
        COffscreenPointlight() { };
        
        /**
         *  Destructor.
         */
        ~COffscreenPointlight() { };
        
        /**
         *  Create point light.
         */
        void                Create( const float radius, const float r, const float g, const float b, const float radiusDraw = 0.0f );
        
        /**
         *  Set light position.
         */
        void                SetPosition( const Vec3 & _position );
        
        //! Directional sphere.
        ncGeometrySphere    * dSphere;
        
        //! Radius sphere.
        ncGeometrySphere    * rSphere;

        /**
         *  Set light size.
         */
        inline void             SetLightSize( const float& _size ) {     fLightSize = _size; }
        
        /**
         *  Get light size.
         */
        inline const float          & GetLightSize() const   {       return fLightSize;  }
        
    public:
        
        //!  Light size.
        float   fLightSize;
        //!  Light color.
        Vec3   m_vColor;
    };
    
    ///  Instanced point lights.
    class COffscreenMultiPointLight
    {
    public:
        
        /**
         *  Constructor.
         */
        COffscreenMultiPointLight( COffscreenPointlight * _pLight )
        {
            pLight = _pLight;
        };
        
        //!  Point light handle.
        COffscreenPointlight   *pLight;
        //!  Object matrices.
        ncMatrix4       objectMatrices[MAX_DEFERRED_POINT_LIGHTS];
        //!  Object positions.
        Vec3       objectCenters[MAX_DEFERRED_POINT_LIGHTS];
    };

	//!  Light sizes.
	static float LIGHTRADIUS_TINY = 4.0f;
	static float LIGHTRADIUS_SMALL = 8.0f;
	static float LIGHTRADIUS_MEDIUM = 16.0f;
	static float LIGHTRADIUS_BIG = 32.0f;
	static float LIGHTRADIUS_VERYBIG = 64.0f;

    ///  Offscreen light renderer.
    class COffscreenLightRenderer
    {
        NEKO_NONCOPYABLE( COffscreenLightRenderer );
        
    public:

        /**
         *  Constructor.
         */
        COffscreenLightRenderer()
        {
            
        }
        
        /**
         *  Destructor.
         */
        ~COffscreenLightRenderer()
        {
            
        }
        
        /**
         *  Initialize point light renderer.
         */
        void                Initialize();
        
        /**
         *  Render point lights.
         */
        void                Render( const COffscreenPointlight * pLights );
        void                Render( const COffscreenMultiPointLight & pLights );
        
        /**
         *  Begin point light renderer.
         */
        void                StartRender();
        
        /**
         *  End point light renderer.
         */
        void                StopRender();
        
        //!  Point light spheres.
        ncGeometrySphere    m_pLightSpheres[5];
        //!  Point light shader.
        SGLShader      * pointLightShader;
        //!  Point light offscreen framebuffer.
        CFramebuffer   pointLightFramebuffer; // Offscreen.
 
        
        //! Shader uniforms.
        enum LightUniforms
        {
            PerspectiveMatrix       = 0,
            PerspectiveInvMatrix    = 1,
            WorldMatrix             = 2,
            ObjectMatrix            = 3,
            Origin                  = 4,
            LightSize               = 5,
            LightColor              = 6,
            Dummy                   = 7
        };
        
        //! Light shader uniform passes.
        int32_t     m_uLightPasses[7];
        
        int32_t     uNormalTexture;
        int32_t     uDepthTexture;
        
        //!  Current amount of scene lights.
        uint32_t    m_iSceneLights;
        uint32_t    m_iSceneMultiLights;
    };
#endif
    
    ///  Main world environment.
    class CBeautifulEnvironment
    {
        NEKO_NONCOPYABLE( CBeautifulEnvironment );
        
    public:
        
        /**
         *  Constructor & destructor.
         */
        CBeautifulEnvironment();
        ~CBeautifulEnvironment();

        /**
         *  Add pointlight to the scene!
         */
        void                AddPointLight( const Vec3 & vPos, const Vec3 & vColor, const float & fSize );

        /**
         *  Render everything in our world!
         */
        void                Render( CRenderer::ESceneEye eye, int32_t flags, uint32_t msec );
 
        /**
         *  Update environment.
         */
        void                Update( uint32_t msec );
        
        /**
         *  Create environment..
         */
        void                Makeup( const NoisePerlinParams & largeNoise, const NoisePerlinParams & mediumNoise, const NoisePerlinParams & smallNoise );
        
        /**
         *  Prepare environment.
         */
        void                MakePreparation( INekoAllocator * alllocator );
        
        //  Rendering methods.
        void                RenderTerrain( CRenderer::ESceneEye eye, int32_t flags ); //! Render terrain.
        void                RenderGrass( CRenderer::ESceneEye eye );   //! Render grass.
        void                RenderSky( uint32_t msec, bool isReflection );            //! Render sky.
        void                RenderDeferredLights();                    //! Render deferred lights.
        void                RenderParticles( uint32_t msec );          //! Render particles.
        void                RenderOffscreen( uint32_t msec );          //! Render offscreen things.

        /**
         *  Renders all the world into the cubemap.
         */
        void                RenderCubemap( uint32_t msec );
        
        /**
         *  Render shadows.
         */
        void                RenderShadows();
        
        /**
         *  Reset beautiful environment.
         */
        void                Reset();
       
        /**
         *  Cool sky.
         */
        void                    MakeSky();

        
        //! Chunk handler for great landscapes.
        CWorldHandler           * m_pWorldHandler;
        //! Environment flags.
        uint32_t    m_iEnvFlags;
    
        //!  Ocean.
        CWorldOcean     * m_pWorldOcean;
        
        
        //! BRDF lut material.
        SMaterial       * m_pBRDFMaterial;
        
        
        //  Environment mapping!
        
        //! Environment cubemap.
        SMaterial   * m_pEnvironmentCubemap;
        //! Environment cubemap size.
        int32_t     m_iEnvironmentCubeSize;
        //! Environment framebuffer.
        CFramebuffer    * m_pCubemapFBO;
        //! Should we update environment mapping?
        bool    m_bRenderToCubemap;
        //! Cubemap projection matrix.
        ncMatrix4   m_mCubemapProjection;
        
        // temp
        ncMesh      * m_pIndoorScene;
        
        
        ncMesh      * m_pSkyDomeMesh = 0;
        
        //!  Particle handler.
        CParticleEngine    * m_pParticleEngine;
        //!  Offscreen particle framebuffer.
        CFramebuffer       * m_pParticleFramebuffer;
    
        //! Dev.
        void                Feedback();

        /**
         *  Create object.
         */
        ncMesh *                CreateObject( const char * name, const Vec3 & origin, const Vec3 & rotation, const float & scale, const bool hasSphere = false );
        
        /**
         *  Destroy the beautiful environment.
         */
        void                Destroy();
  
        /**
         *  Is Ocean animated?
         */
        void                    SetOceanUsesFFTT( const bool usesFFT );
        
        /**
         *  Get world time.
         */
        inline const float  &               GetTime()   {   return m_fTime;     }
        
        /**
         *  Water level.
         */
        inline const float      &           GetMinWaterLevel() {   return m_fWaterMinLevel;  }

        /**
         *  Sun light position.
         */
        inline const Vec3&                 GetSunLightPosition()  {   return m_vSunPosition;    }
        
        /**
         *  Sun light direction.
         */
        inline const Vec3&                 GetSunLightDirection() {   return m_vSunDirection;   }
        
        /**
         *  Sun current light direction.
         */
        inline const Vec3&                     GetCurrentLightDir() {   return m_vCurrentLightDir;   }
        
        /**
         *  Get sun light color.
         */
        inline const Vec3&                     GetSunLightColor() {   return m_vSunColor;   }
        
        /**
         *  Get sky light color.
         */
        inline const Vec3&                         GetSkyLightColor() {   return m_vSkyColor;   }
        
        inline const Vec3&                     GetGlobalFogColor()     { return m_vGlobalFogColor; }
        
//        inline const Vec3&                 GetCloudColor() {   return m_vCloudColor;   }

        /**
         *  Outdoor light direction.
         */
        inline const Vec3      &           GetOutdoorLightDirection()  {        return m_vCurrentLightDir;  }
        
        //! Bind terrain textures.
        void                BindTerrainSummerTextures();
        
        //! Refresh ambient settings.
        void                UpdateEnviroment( const uint32_t msec );
    
        /// Billboard imposter information.
        struct  SImposterInfo {
            //! 3d billboard sprite origin.
            Vec3   origin;
            //! Billboard size.
            float   size;
            //! Billboard sampler.
            SMaterialProp * sampler;
            //! Ignores light pass by deferred shading?
            float    lightFlag;
            //! Billboard transparency.
            float       alpha;
        };
        
        
        float   m_fShadowStrengthNight ;
        float   m_fShadowStrengthDay ;
        
        // Sky parameters
        float   m_fRayleighMultiplier;
        float   m_fMieMultiplier;
        float   m_fSkyBrightness ;
        float   m_fSkyContrast ;
        float   m_fSkyFogginess ;
        
        float   m_fkMie;
        float   m_fkRayleigh;
        
        float   m_fKr4PI;
        float   m_fKm4PI;
        
         float m_kSunBrightness;
        
        //! Sky beta mie.
        Vec3    m_vkBetaMie;
        
        //! Controls how low the light source is allowed to go.
        //!< -1 light source can go as low as it wants.
        //!< 0 light source will never go below the horizon.
        //!< +1 light source will never leave zenith.
        float   m_fLightMinimumHeight;
        
        float   m_fSkyUpdateInterval;
        
        float       m_fHaloSize ;// HaloPower = 1.0 / HaloSize
        float       m_fHaloBrightness ;
        
        //! 0 fog is atmosphere color at horizon. 1 fog is atmosphere color at zenith.
        float       m_fHeightBias;
        
        /**
         *  Draw billboard sprite!
         */
        void                DrawBillboard( const SImposterInfo    * info );
        
        //! Billboarding shader.
        SGLShader * m_billboardShader;
        //! Billboard shader uniforms.
        GLuint  m_billboardShaderIds[3];
        
        //! Billboard shader pass ids
        enum BillboardShaderId {
            BBSHADER_MVP = 0,
            BBSHADER_MATRIX,
            BBSHADER_PROPERTIES
        };

        //! Foliage shader.
        SGLShader               * grassShader;
        //! Light renderer.
        COffscreenLightRenderer      * m_pPointLightRenderer;
        
        /**
         *  Is Ocean animation currently turned on?
         */
        inline bool                 IsOceanThreadActive() const {       return !m_bIsOceanThreadActive; }
        
        //!  Is it busy?
        bool    m_bLoadsWorld;
        
        //!  Ocean CPU thread.
        INekoThread  * m_oceanFFT;
        
        //!  Environment shadow renderer.
        CCascadedShadow * m_pbEnvShadows;
        
        //!  Moon imposter.
        SImposterInfo   * m_pMoonImposter;
        
        /**
         *  Get grass uniforms.
         */
        inline GLuint       &               GetGrassUniforms( const GLuint uniform )    {       return _grassUniformArray[uniform]; }
        
        /**
         *  Get grass LoDs.
         */
        inline const float      &               GetGrassLod1() const   {       return m_fGrassLod1;   }
        inline const float      &               GetGrassLod2() const   {       return m_fGrassLod2;   }

//        // Wind direction and speed calculation
//        Vector2 v1 = new Vector2(cosf(Mathf.Deg2Rad * (WindDegrees + 15)),
//                                 sinf(Mathf.Deg2Rad * (WindDegrees + 15)));
//        Vector2 v2 = new Vector2(cosf(Mathf.Deg2Rad * (WindDegrees - 15)),
//                                 sinf(Mathf.Deg2Rad * (WindDegrees - 15)));
//        Vector4 wind = WindSpeed / 100f * new Vector4(v1.x, v1.y, v2.x, v2.y);

        /**
         *  Update wind settings.
         */
        void                UpdateWind();

        /**
         *  Set grass settings
         */
        inline void                 BeginGrassRender();
        
        /**
         *  Set grass settings
         */
        inline void                 EndGrassRender();

        /**
         *  Set asset handle pointers.
         */
        void                    SetAssetHandlers();
        
        /**
         *  Set grass position and rotation uniforms.
         */
        inline void                 PassGrassUniforms( const float& x, const float& y, const float& z, const float& rot );
        
        //! Get fog params.
        //! - ----------------------------------------
        inline const float  &               GetSkyDistanceForFog() const {  return fSkyDistance;    }

        inline const float  &               GetFogMaxDistance() const { return fFogMaxDistance; }
        
        inline const float  &               GetFogBottomHeight() const {    return fFogBottomHeight;    }
        
        inline const float  &               GetFogBottomIntensity() const { return fFogBottomIntensity; }
        
        inline const float  &               GetFogTopHeight() const {   return fFogTopHeight;   }
        
        inline const float  &               GetFogTopIntensity() const {    return fFogTopIntensity;    }
        
        inline const float  &               GetFogSlope() const {   return fFogSlope;   }
     
        /**
         *  Get sun and sky color relation.
         */
        inline const float GetSunSkyColorRelation( const Vec3 & skyColor, const Vec3 & sunColor )
        {
            const float cSunMax = nkMath::Max(0.1f, nkMath::Max(sunColor.x, nkMath::Max(sunColor.y, sunColor.z)));
            const float cSkyMax = nkMath::Max(0.1f, nkMath::Max(skyColor.x, nkMath::Max(skyColor.y, skyColor.z)));
            
            return cSunMax / (cSkyMax + cSunMax);
        }
        
//    private:
        
        INekoAllocator      * pAllocator = 0;
        
        //!          - GRASS -
        
        ///  Shader uniforms.
        enum GrassUniforms {
            VIEWPROJECTION = 0,
            GRASSLOD,
            CAMERAPOSITION_TIME,
            CELLPOSITION,
            CELLROTATION,
            WINDDIRECTION,
            WINDSTREN,
            MOVEFACTOR,
            
            GRASS_UNI,
        };
        
        ///  Grass materials.
        enum GrassMaterials {
            GRASS_GREEN = 0,
            GRASS_GREEN_N, // Normal.
            GRASS_UNIMAT,
        };
        
        //! Grass shader passes.
        GLuint _grassUniformArray[GRASS_UNI];
        
        //! Foliage billboard.
        GPUBuffer billboardIndexArray;
        
        //! Foliage textures.
        SMaterialProp * m_pGrassMaterial;
        
        float   m_fGrassLod1, m_fGrassLod2;
        
        //!          - SKY -
        
        ///  Sky shader uniforms.
        enum SkyUniforms
        {
            SModelViewProj = 0,
            SSunLightPos,
            SMoonLightPos,
            SRainAmount,
            SInvColor,
            SSunRadius,
            SSunMRadius,
            SCloudColor,
            SAmbientColor,
            
            SMoonCloudColor,
            SSunCloudColor,
            SMoonSkyColor,
            SMoonHaloColor,
            SFogginess,
            SFogColor,
            SSunSkyColor,
        };
        
        //! Last time of daytime hour increase.
        float   fLastTimeChange;
        
        float   fWaveInvSunColorRed;
        float   fWaveInvSunColorGreen;
        float   fWaveInvSunColorBlue;

        //! Sky shader pass ids.
        GLuint skyUniforms[18];
        //! Noise.
        SMaterialProp * skyNoiseTexture, * moonTexture;
        
        //! Scene point lights.
        COffscreenPointlight _pointLights[MAX_DEFERRED_POINT_LIGHTS];

        //! Sun position ( follows the main camera ).
        Vec3 m_vSunPosition;
        //! Sun direction ( position normalized ).
        Vec3 m_vSunDirection;
        Vec3 m_vSunCenterPos;     //! Sun center origin ( * does NOT follow the player camera! * )
        
        //! Moon position.
        Vec3 m_vMoonCenterPos;
        //! Moon light direction.
        Vec3   m_vMoonDirection;
        //! Current light direction.
        Vec3   m_vCurrentLightDir;
        
        //! Global fog color.
        Vec3       m_vGlobalFogColor;
        //! Cloud color.
        Vec3       m_vSunCloudColor;
        Vec3       m_vMoonCloudColor;
        
        Vec3       m_vSunSkyColor;
        Vec3       m_vMoonSkyColor;
        Vec3       m_vMoonHaloColor;
        
        //! Sun color.
        Vec3       m_vSunColor;
        Vec3       m_vSkyColor;  //! Sky color.
    
        //! Time of day mode.
        //!< [Manual, Realistic]
        bool        m_bTimeOfDayManual;
        
        float       m_fTimeSinceLightUpdate;
        
        //! [-90, +90]
        //!< Latitude of the current location in degrees.
        float   m_fLatitude;
        
        //! [-180, +180]
        //!< Longitude of the current location in degrees.
        float   m_fLongitude;
        
        int32_t     m_iYear;
        int32_t     m_iMonth;
        int32_t     m_iDay;
        int32_t     m_iHour;
        
        //! [-14, +14]
        //!< UTC/GMT time zone of the current location in hours.
        float   m_fUTC;

        /**
         *  Sample atmosphere at direction.
         */
        Vec4               SampleAtmosphere( Vec3 dir, const bool directLight = true );
        
        /**
         *  Sky scattering color.
         */
        Vec4               SkyScatterColor( Vec3 & dir, const bool directLight = true );
        
        /**
         *  Sample fog color.
         */
        Vec4               SampleFogColor();
        
        /**
         *  Calculate the sky color.
         */
        Vec4               SampleSkyColor();
        
        /**
         *  Calculate equator color.
         */
        Vec4               SampleEquatorColor();
        
        //! Environment settings.
        float           m_dawnStart;
        float           m_dawnEnd;
        float           m_duskStart;
        float           m_duskEnd;
        
        //! Time settings in minutes!
        float               m_fDawnTime;
        float               m_fDawnDuration;
        float               m_fDuskTime;
        float               m_fDuskDuration;
        
        float           m_fSunLuminance;    //! In Lux.
        float           m_fSunAngularRadius;    
        
        float           m_fLightIntensityDay;
        float           m_fLightIntensityNight;
        
        
        //! Sun zenith angle in degrees.
        //!< \n = 0   if the sun is exactly at zenith.
        //!< \n = 180 if the sun is exactly below the ground.
        float SunZenith;
        
        float SunAltitude, SunAzimuth;
        
        //! Moon zenith angle in degrees.
        //!< \n = 0   if the moon is exactly at zenith.
        //!< \n = 180 if the moon is exactly below the ground.
        float MoonZenith;
        
        float MoonAltitude, MoonAzimuth;
        
        float SunsetTime, SunriseTime;
        float LocalSiderealTime;

        
        //!< Currently active light source zenith angle in degrees.
        //!< \n = 0  if the currently active light source (sun or moon) is exactly at zenith.
        //!< \n = 90 if the currently active light source (sun or moon) is exactly at the horizon.
        inline const float              LightZenith() const  { return nkMath::Min(SunZenith, MoonZenith); }

        
        float       m_fFogginess;
        
        //!          - WIND -
        
        //! Wind direction in degrees.
        //!< 0 for wind blowing in northern direction.
        //!< 90 for wind blowing in eastern direction.
        //!< 180 for wind blowing in southern direction.
        //!< 270 for wind blowing in western direction.
        float   WindDegrees = 0.0f;
        
        //! Speed of the wind that is acting on the clouds.
        float   WindSpeed = 1.0f;
        
        Vec3   CloudUV;
        
        float fGrassMoveFactor;
        
        //!          - OCEAN -
        
        float   m_fWaterMinLevel;
        bool    m_bIsOceanThreadActive;
        
//        tthread::thread _oceanFFTThread;
        
        //! Sky shader.
        SGLShader  *skyShader;
        
        //! Billboard mesh.
        CBasicMesh * m_pQuadMesh;
        
//        //! Sky mesh.
//        CBasicMesh * m_pSkyDomeMesh;

        //!          - WEATHER -
        float   m_fRainAmount;
        
        //! Daynight time.
        float   m_fTime, m_fTimeIncrease;
        
        //! Timing information.
        struct STimeInfo
        {
            float   m_fStartTime;
            float   m_fEndTime;
            
            float   m_fAnimSpeed;
        };
        
        //! Global daytime information.
        STimeInfo   m_timeInfo;
        
        //!          - FOG -
        float   fSkyDistance;
        float   fFogMaxDistance;
        float   fFogBottomHeight;
        float   fFogBottomIntensity;
        float   fFogTopHeight;
        float   fFogTopIntensity;
        float   fFogSlope; // Resulting color.
     
        //! Landscape material array.
        SMaterialProp   * m_pLandscapeMaterial[1];
    };
    
    /**
     *  Set grass settings
     */
    inline void CBeautifulEnvironment::BeginGrassRender()
    {
        // Grass shader.
        grassShader->Use();
        
        // Samplers.
        g_mainRenderer->BindTexture( 0, m_pGrassMaterial->m_pDiffuse->GetId() );
        g_mainRenderer->BindTexture( 1, m_pGrassMaterial->m_pNormal->GetId() );
        
        // Foliage common properties.
        grassShader->SetUniform( _grassUniformArray[VIEWPROJECTION], 1, false, g_Core->p_Camera->ViewProjectionMatrix.m );
        grassShader->SetUniform( _grassUniformArray[CAMERAPOSITION_TIME], g_Core->p_Camera->vEye.x, g_Core->p_Camera->vEye.y, g_Core->p_Camera->vEye.z, g_Core->GetTime() * 0.002f  );
        
        // Bind vertex array at begin.
        m_pQuadMesh->BindIndexData();
    }
    
    
    /**
     *  End grass rendering.
     */
    inline void CBeautifulEnvironment::EndGrassRender()
    {
        g_mainRenderer->UnbindVertexArray();
        grassShader->Next();
    }
    
    
    /**
     *  Set grass position and rotation uniforms.
     */
    inline void CBeautifulEnvironment::PassGrassUniforms( const float& x, const float& y, const float& z, const float& rot )
    {
        // Pass positions to shader.
        grassShader->SetUniform( _grassUniformArray[CELLPOSITION], x, y, z, rot );
    }
    
    
    
    extern CBeautifulEnvironment   *g_pbEnv;

}

#endif /* defined(__Nanocat__BeautifulEnvironment__) */
