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
//  BeautifulEnvironment.cpp
//  Beautiful environment. ;p
//
//  This code is a part of Neko engine.
//  Created by Neko Code on 11/30/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../AssetCommon/AssetBase.h"
#include "../AssetCommon/Material/MaterialLoader.h"
#include "../AssetCommon/Sound/SoundManager.h"
#include "../Core/Player/Camera/Camera.h"
#include "../Core/ScriptSupport/Scripting.h"
#include "../Core/Utilities/BRDF_Preset.h"  // BRDF lut material for PBR rendering.
#include "../Core/Utilities/Utils.h"
#include "../Graphics/OpenGL/GLShader.h"
#include "../Math/GameMath.h"
#include "../Platform/Shared/System.h" // Game time.
#include "BeautifulEnvironment.h"
#include "Landscape/LandscapeChunk.h"
#include "ParticleEngine.h"


// @todo: Postprocess water
// @todo: Water projection grid
// @todo: Light propagation volumes

namespace Neko {
    
    /**
     *  Initial Beautiful Environment settings.
     */
    CBeautifulEnvironment::CBeautifulEnvironment()
            : m_pWorldOcean(NEKO_NULL), m_bIsOceanThreadActive(true), grassShader(NEKO_NULL), skyShader(NEKO_NULL)
    {
        Reset();
    }
    
    /**
     *  Destructor.
     */
    CBeautifulEnvironment::~CBeautifulEnvironment()
    {
        
    }
    
    /**
     *  Reset beautiful environment.
     */
    void CBeautifulEnvironment::Reset()
    {
        m_fRainAmount = 0.0f;
        m_fTime = 12.0f; // 24 hours/0:00, start of the new day, assuming it's day time ( not am/pm ).
        m_fTimeIncrease = m_fTime;
        
        // Fog settings.
        fSkyDistance = 1000.0f;//0
        fFogMaxDistance = 512000.0f;
        fFogBottomHeight = 1000.0f;         // TODO: customizable
        fFogBottomIntensity = 0.532f;
        fFogTopHeight = 400000.0f;
        fFogTopIntensity = 0.600f;
        
        // Sky settings.
        fWaveInvSunColorRed = 1.0f / powf( 0.660f, 4.0f ); // 635 // 0.595f
        fWaveInvSunColorGreen = 1.0f / powf( 0.570f, 4.0f ); // 0.5202f
        fWaveInvSunColorBlue = 1.0f / powf( 0.475f, 4.0f ); // 0.492f
        
        // Correct fog preferences;
        fFogBottomHeight = fFogBottomHeight / fSkyDistance;
        fFogTopHeight = fFogTopHeight / fSkyDistance;
        fFogMaxDistance = fFogMaxDistance / fSkyDistance;
        fFogSlope = (fFogTopIntensity - fFogBottomIntensity) / (fFogTopHeight - fFogBottomHeight);        // Fog slope.
        
        m_fWaterMinLevel = 10.0f;       // @todo: world depended
        m_iEnvFlags = 0;
        m_bLoadsWorld = false;
        
        m_fSkyUpdateInterval = 1.0f;
        
        // Sunlight direction.
        m_vSunDirection = Vec3( 1.0f );
        
        m_fGrassLod2 = 50.0f;
        m_fGrassLod1 = 100.0f;
        
        CloudUV = Vec3( nkMath::RandFloatAlt( 1.0f, 128.0f ) );
        
        // Basic daytime information.
        m_timeInfo.m_fEndTime = 24.0f;
        m_timeInfo.m_fStartTime = 0.0f;
        m_timeInfo.m_fAnimSpeed = 0.00825f;
        
        fLastTimeChange = 0.0f;
        
        // TODO: Customizable.
        m_fDawnTime = 355.0f;
        m_fDawnDuration = 410.0f;
        m_fDuskTime = 540.0f;
        m_fDuskDuration = 490.0f;
        
        // to hours..
        m_dawnStart = (m_fDawnTime - m_fDawnDuration * 0.5f) / 60.0f;
        m_dawnEnd = (m_fDawnTime + m_fDawnDuration * 0.5f) / 60.0f;
        m_duskStart = 12.0f + (m_fDuskTime - m_fDuskDuration * 0.5f) / 60.0f;
        m_duskEnd = 12.0f + (m_fDuskTime + m_fDuskDuration * 0.5f) / 60.0f;
        
        if( m_dawnEnd > m_duskStart ) { // sanity check
            m_duskEnd += m_dawnEnd - m_duskStart;
            m_duskStart = m_dawnEnd;
        }
        
        m_bRenderToCubemap = false;
        
        UpdateWind();
    }
    
    /**
     *  Destroy the beautiful environment.
     */
    void CBeautifulEnvironment::Destroy()
    {
        g_Core->p_Console->Print( LOG_INFO, "Removing world environment..\n" );
        
        m_iEnvFlags = 0;
        
        // Remove particle engine.
        m_pParticleEngine->DestroyAllParticles();
        // Remove its particle engine framebuffer.
        m_pParticleFramebuffer->Delete();
        pAllocator->Dealloc(m_pParticleFramebuffer);
        
        f_AssetBase->p_MaterialBase->UnloadMaterial( m_pEnvironmentCubemap );
        
        m_pCubemapFBO->Delete();
        pAllocator->Dealloc( m_pCubemapFBO );
        m_pCubemapFBO = NEKO_NULL;
        
        // TODO: m_pPointLightRenderer 
        
        m_pWorldHandler->Destroy();
        
        m_oceanFFT->CallbackEnd( 200.0f );
        g_Core->KillThread( m_oceanFFT );
        
        m_pbEnvShadows->Destroy();
        
        pAllocator->Dealloc( m_pMoonImposter );
    }
    
    /**
     *  Render sky.
     */
    void CBeautifulEnvironment::RenderSky( uint32_t msec, bool isReflection )
    {
        //! Check if we can render the sky.
        if( !(m_iEnvFlags & CreatedSky) ) {
            return;
        }
        
        ncMatrix4   pos;
        ncMatrix4   projectionModelView;
        
        Vec3       skyPos;
        float       coef;
        
        // Inverse Y if needed ( for planar reflections ).
        coef = isReflection ? -1.0f : 1.0f;
        
        // Sky dome position.
        // Follows the player camera origin.
        skyPos = Vec3( g_Core->p_Camera->vEye.x, g_Core->p_Camera->vEye.y * coef /* planar */,
                       g_Core->p_Camera->vEye.z );
        
        pos.Translate( skyPos );
        pos.ScaleY( coef );
        
        // Calculate model view projection.
        projectionModelView = g_Core->p_Camera->ViewProjectionMatrix * pos;
        
        skyShader->Use();
        skyShader->SetUniform( skyUniforms[SModelViewProj] , 1, GL_FALSE, projectionModelView.m );
        
        skyShader->SetUniform( skyUniforms[SSunLightPos], m_vSunDirection );
        skyShader->SetUniform( skyUniforms[SMoonLightPos], m_vMoonDirection );
        
        // Light colors.
        skyShader->SetUniform( skyUniforms[SAmbientColor], GetSunLightColor(), msec * 0.00001f  );
        //        skyShader->SetUniform( skyUniforms[SCLOUDCOLOR], GetCloudColor() );
        
        skyShader->SetUniform( skyUniforms[SMoonCloudColor], m_vMoonCloudColor );
        skyShader->SetUniform( skyUniforms[SSunCloudColor], m_vSunCloudColor );
        skyShader->SetUniform( skyUniforms[SMoonSkyColor], m_vMoonSkyColor );
        skyShader->SetUniform( skyUniforms[SMoonHaloColor], m_vMoonHaloColor );
        skyShader->SetUniform( skyUniforms[SFogginess], m_fFogginess );
        skyShader->SetUniform( skyUniforms[SFogColor], m_vSunColor );
        skyShader->SetUniform( skyUniforms[SSunSkyColor], m_vSunSkyColor );
        
        // 3D noise texture to generate procedural clouds.
        g_mainRenderer->BindTexture( 0, skyNoiseTexture->m_pDiffuse->Image.GetId() );
        
        // Draw sky dome mesh.
        m_pSkyDomeMesh->m_hOwner->Render( 0, 2 );
        
        skyShader->Next();
    }
    
    /**
     *  Render offscreen things.
     */
    void CBeautifulEnvironment::RenderOffscreen( uint32_t msec )
    {
        // Render particles.
        RenderParticles( msec );
        
        // Render deferred point lights.
        g_mainRenderer->SetBlending( true, EBlendMode::Additive, EBlendMode::Additive  );
        g_mainRenderer->SetCulling( true, ECullMode::Front );
        
        // Render pointlights to a framebuffer.
        m_pPointLightRenderer->StartRender();
        m_pPointLightRenderer->Render( _pointLights );
        m_pPointLightRenderer->StopRender();
        
        // Restore..
        g_mainRenderer->SetCulling( false, ECullMode::Back );
        g_mainRenderer->SetBlending( false, EBlendMode::Disabled, EBlendMode::Disabled );
    }
    
    /**
     *  Render particles.
     */
    void CBeautifulEnvironment::RenderParticles( uint32_t msec )
    {
        if( !(m_iEnvFlags & CreatedParticles) ) {
            return;
        }
        
        // Update billboarding for particle.
        m_pParticleEngine->SetMatrices( g_Core->p_Camera->vLook );
        
        //glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
        
        // Bind particle framebuffer.
        m_pParticleFramebuffer->BindForDrawing();
        
        g_mainRenderer->ClearColorDepthWithColor( 0.0f, 0.0f, 0.0f, 1.0f );
        g_mainRenderer->SetViewportSize( g_mainRenderer->particleOffscreenWidth, g_mainRenderer->particleOffscreenHeight );
        
        // Set blending.
        g_mainRenderer->SetBlending( true, EBlendMode::Additive, EBlendMode::AlphaAdditive );
        
        // Disable writing to depth.
        g_mainRenderer->SetDepthWriteMode( false );
        
        // Render particles.
        m_pParticleEngine->Render( (float)msec / 1000.0f );
        
        // Restore settings.
        g_mainRenderer->SetDepthWriteMode( true );
        
        g_mainRenderer->SetBlending( false, EBlendMode::Disabled, EBlendMode::Disabled );
        
        m_pParticleFramebuffer->UnbindDrawing();
        
        //glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
    }
    
    /**
     *  Add pointlight to the scene!
     */
    void CBeautifulEnvironment::AddPointLight( const Vec3 & vPos, const Vec3 & vColor, const float & fSize )
    {
        g_Core->p_Console->Print( LOG_INFO, "AddPointLight(): A new pointlight(%i) was created at (%4.2f, %4.2f, %4.2f)..\n", m_pPointLightRenderer->m_iSceneLights, vPos.x, vPos.y, vPos.z );
        
        _pointLights[m_pPointLightRenderer->m_iSceneLights].Create( fSize, vColor.x, vColor.y, vColor.z );
        _pointLights[m_pPointLightRenderer->m_iSceneLights].SetPosition( vPos );
        
        ++m_pPointLightRenderer->m_iSceneLights;
    }
    
    /**
     *  Update environment.
     *
     *  @param msec Frame time.
     */
    void CBeautifulEnvironment::Update( uint32_t msec )
    {
        if( !(m_iEnvFlags & CreatedEnvironment) ) {
            return;
        }
        
        // Update chunk handler.
        if( m_iEnvFlags & CreatedLandscape ) {
            m_pWorldHandler->Update();
        }
        
        // dev workaround
        if( g_Core->m_pGamepad[0] != NEKO_NULL && g_Core->p_Camera->m_bIsGamepad ) {
            int32_t axis = g_Core->m_pGamepad[0]->GetAxisMapped( EGameControllerAxis::TriggerLeft );
            int32_t axis2 = g_Core->m_pGamepad[0]->GetAxisMapped( EGameControllerAxis::TriggerRight );
            
            if( axis != 0 ) {
                m_fTime+=0.1f;
            }
            
            if( axis2 != 0 ) {
                m_fTime-=0.1f;
            }
            
            if( g_Core->m_pGamepad[0]->GetButtonMapped( EGameControllerButton::ButtonY) == 1 )
            {
                LandscapeChunk * c = m_pWorldHandler->FindNodeAt( g_Core->p_Camera->vEye );
                if( c != NEKO_NULL ) {
                    printf( "node origin=<%4.2f, %4.2f>\n", (float)c->m_fPositionX, (float)c->m_fPositionY );
                }
                
                if( c != NEKO_NULL )
                {
                    CQuadtree * cqt = c->GetQuadtree();
                    if( cqt != NEKO_NULL )
                    {
                        CQuadtreeNode * cqtn = cqt->FindNode( Vec2( static_cast<int32_t>(g_Core->p_Camera->vEye.x), static_cast<int32_t>(g_Core->p_Camera->vEye.z) ) );
                        if( cqtn != NEKO_NULL ) {
                            CNodeChunk * cqtnc = cqtn->GetChunk();
                            
                            cqtnc->AddObject( new ncLTerrainObject( "coupe.obj",
                                                                   Vec4(g_Core->p_Camera->vEye, (float)nkMath::Random(360) ), pMainAllocProxy, 7.2f
                                                                   ) );
                        }
                    }
                }
                
            }
        }
        
        if( Render_Secret) {
			m_fTime += 0.1f;
            Render_Secret = false;
            //            m_pWorldHandler->CreatePathNodeAt( Vec2( g_Core->p_Camera->vEye.x, g_Core->p_Camera->vEye.z ) );
            
            f_AssetBase->LoadAssetAsynchronously( EAssetType::Mesh, "coupe.obj" );
            
            
            
        }
        
        if( Render_Secret2 ) {
            Render_Secret2 = false ;
//
//            
//            LandscapeChunk * c = m_pWorldHandler->FindNodeAt( g_Core->p_Camera->vEye );
//            if( c != NEKO_NULL ) {
//                printf( "node origin=<%4.2f, %4.2f>\n", (float)c->m_fPositionX, (float)c->m_fPositionY );
//            }
//            
//            if( c != NEKO_NULL )
//            {
//                CQuadtree * cqt = c->GetQuadtree();
//                if( cqt != NEKO_NULL )
//                {
//                    CQuadtreeNode * cqtn = cqt->FindNode( Vec2( g_Core->p_Camera->vEye.x, g_Core->p_Camera->vEye.z ) );
//                    if( cqtn != NEKO_NULL ) {
//                        CNodeChunk * cqtnc = cqtn->GetChunk();
//                        
//                        cqtnc->AddObject( new ncLTerrainObject( "coupe.obj",
//                                                               Vec4(g_Core->p_Camera->vEye, (float)nkMath::Random(360) ), pMainAllocProxy, 7.2f
//                                                               ) );
//                    }
//                }
//            }
//            AddPointLight( g_Core->p_Camera->vEye, Vec3(1.0, 0.0, 0.0), 16.0f );

            m_bRenderToCubemap = true;
//            m_pWorldHandler->CreateInstancedObjects();
        }
        
        UpdateEnviroment( msec );
        
        
        // Update clouds.
        float u = sinf(nkMath::Deg2Rad * WindDegrees);
        float v = cosf(nkMath::Deg2Rad * WindDegrees);
        
        float time = 1e-3f * g_Core->frameMsec;
        float wind = WindSpeed * time;
        
        float x = CloudUV.x;
        float y = CloudUV.y;
        float z = CloudUV.z;
        
        y += time * 0.1f;
        x -= wind * u;
        z -= wind * v;
        
        x -= floorf(x);
        y -= floorf(y);
        z -= floorf(z);
        
        CloudUV = Vec3(x, y, z);
        
        //        localRotation = Euler(0, y * 360, 0);
        
        // CloudOffset = OffsetUV
        // CloudWind = CloudUV
        // CloudSize = (Size * 2, Size, Size *2)
        
    }
    
    /**
     *  Render our Beautiful Environment!
     */
    void CBeautifulEnvironment::Render( CRenderer::ESceneEye eye, int32_t flags, uint32_t msec )
    {
        // Check if we can render...
        if( !(m_iEnvFlags & CreatedEnvironment) ) {
            return;
        }
        
        const bool isReflection = (flags & (int32_t)EWorldFlag::Reflection);
        
        // Render the sky.
        RenderSky( msec, isReflection );
        
        g_mainRenderer->SetDepthTest(true);
        g_mainRenderer->SetCulling(true, ECullMode::Back);
        
        //  Is it reflected?
        if( !isReflection ) {
            
            // Update moon position and render it.
            m_pMoonImposter->origin = g_Core->p_Camera->vEye + ( m_vMoonDirection * 2000.0f );
            
            g_mainRenderer->SetDepthWriteMode(false);
            g_mainRenderer->SetBlending( true, EBlendMode::AlphaAdditive, EBlendMode::AlphaAdditive );
            // Render Moon!
            DrawBillboard( m_pMoonImposter );
            // Restore states.
            g_mainRenderer->SetBlending( false, EBlendMode::Disabled, EBlendMode::Disabled );
            g_mainRenderer->SetDepthWriteMode(true);
            
            // Draw the landscape.
            RenderTerrain( eye, flags );
            if(m_pIndoorScene!=NEKO_NULL){
                m_pIndoorScene->Render();
            }
         
            if( !(flags & (int32_t)EWorldFlag::Cubemap ) ) {
// Render beautiful water.
//                if( m_pWorldOcean != NEKO_NULL ) {
//                    m_pWorldOcean->Render();
//                }
            }
        } else {
            g_mainRenderer->SetCulling( true, ECullMode::Front );
            
            RenderTerrain( eye, flags );
            
            g_mainRenderer->SetCulling( false, ECullMode::Back );
        }
        
        g_mainRenderer->SetDepthTest(false);
        g_mainRenderer->SetCulling(false, ECullMode::Back);
    }
    
    /**
     *  Render all shadows in the world.
     */
    void CBeautifulEnvironment::RenderShadows()
    {
        if( m_pbEnvShadows != NEKO_NULL ) {
            m_pbEnvShadows->RenderShadow();
        }
    }

    /**
     *  Prepare some things.
     */
    void CBeautifulEnvironment::MakePreparation( INekoAllocator * allocator )
    {
        // Sun position.
        m_vSunPosition = Vec3( -3100.0f, 4000.5f, 2700.0f );
        
        // Sun luminance in Lux.
        m_fSunLuminance = 120000.0f;    // Sun light strength in Lumens.

        m_fSunAngularRadius = nkMath::PI * 75.0f /* sun_angle */ / 180.0f;
        
        m_fTimeSinceLightUpdate = 0.0f;
        
        m_fLatitude = 0.0f;
        m_fLongitude = 0.0f;
        
        m_iYear = 2077;
        m_iMonth = 10;
        m_iDay = 30;
        m_iHour = 15;
        
        m_fUTC = 3.0f;
        
        m_fLightIntensityDay = 1.0f;
        m_fLightIntensityNight = 0.125f;
        
        m_fFogginess = 0.0f;
        
        g_Core->p_Console->Print( LOG_INFO, "World preparation..\n" );

        pAllocator = allocator;
        
        // Update assets.
        SetAssetHandlers();
        
        // Initialize chunk handler.
        m_pWorldHandler = (CWorldHandler *)pClassLinearAllocator->Alloc(sizeof(CWorldHandler) );
        m_pWorldHandler->Initialize( pAllocator );
        // Point lights.
        m_pPointLightRenderer = (COffscreenLightRenderer* )pClassLinearAllocator->Alloc(  sizeof(COffscreenLightRenderer) );
        m_pPointLightRenderer->Initialize();
        // Initialize particle system.
        m_pParticleEngine = (CParticleEngine *)pClassLinearAllocator->Alloc( sizeof(CParticleEngine) );
        m_pParticleEngine->Init();
        
        // Particle framebuffer.
        g_mainRenderer->particleOffscreenWidth = 640;
        g_mainRenderer->particleOffscreenHeight = 480;
        
        // Create the framebuffer.
        m_pParticleFramebuffer = (CFramebuffer *)pAllocator->Alloc( sizeof(CFramebuffer) );
        m_pParticleFramebuffer->Create ( g_mainRenderer->particleOffscreenWidth, g_mainRenderer->particleOffscreenHeight, true, true, true, false, false );
        
        m_pParticleFramebuffer->BindForDrawing();
        m_pParticleFramebuffer->Resize( g_mainRenderer->particleOffscreenWidth, g_mainRenderer->particleOffscreenHeight );
        
        m_pParticleFramebuffer->UnbindDrawing();
        
        // Create environment cubemap.
        m_iEnvironmentCubeSize = 64;
        g_Core->p_Console->Print( LOG_INFO, "Creating environment map sampler ( %ix%i )..\n", m_iEnvironmentCubeSize, m_iEnvironmentCubeSize );
        
        m_pEnvironmentCubemap = f_AssetBase->p_MaterialBase->CreateMaterial( NEKO_NULL, m_iEnvironmentCubeSize, m_iEnvironmentCubeSize, TextureTarget::TextureCube, ETextureStorageType::Float, ETextureFormat::RGBA16F, ETextureFormat::RGBA, ETextureTile::Repeat );
        // Cubemap environment framebuffer.
        m_pCubemapFBO = (CFramebuffer *)pAllocator->Alloc(  sizeof(CFramebuffer) );
        m_pCubemapFBO->Create( m_iEnvironmentCubeSize, m_iEnvironmentCubeSize, false, true, true );
        
        // Create BRDF material which is going to be used for PBR.
        g_Core->p_Console->Print( LOG_INFO, "Creating BRDF lookup table..\n" );
        
        // Generate BRDF sampler.
        g_mainRenderer->g_pMiscFramebuffer->BindForDrawing();
        g_mainRenderer->ClearColor();
        g_mainRenderer->SetViewportSize( 128, 128 );
        g_mainRenderer->m_pBrdfShader->Use();
        g_mainRenderer->g_pEyeMesh[0]->DrawArrays();
        g_mainRenderer->m_pBrdfShader->Next();
        g_mainRenderer->g_pMiscFramebuffer->UnbindDrawing();
        
        
        // Cubemap projection matrix.
        m_mCubemapProjection.CreatePerspective( 90.0f, 1.0f, g_Core->p_Camera->GetNearPlaneDistance(), g_Core->p_Camera->GetFarPlaneDistance() );
        
        // Landscape.
        m_pLandscapeMaterial[0] = f_AssetBase->p_MaterialBase->Find( "landscapeMat" );
        
        //particleHandler->AddParticle( NEKO_NULL );
        // Grass parameters.
        grassShader = f_AssetBase->FindAssetByName<SGLShader>( "beautifulgrass" );
        grassShader->Use();
        grassShader->SetUniform( "diffuseTex", 0 );
        grassShader->SetUniform( "normalTex", 1 );
        
        _grassUniformArray[VIEWPROJECTION] = grassShader->UniformLocation( "MVP" );
        _grassUniformArray[GRASSLOD] = grassShader->UniformLocation( "mGrassLod" );
        _grassUniformArray[CAMERAPOSITION_TIME] = grassShader->UniformLocation( "mCameraPos_Time" );
        _grassUniformArray[CELLPOSITION] = grassShader->UniformLocation( "mPosition" );
        _grassUniformArray[CELLROTATION] = grassShader->UniformLocation( "mRotation" );
        _grassUniformArray[WINDDIRECTION] = grassShader->UniformLocation( "mWindDirection" );
        _grassUniformArray[WINDSTREN] = grassShader->UniformLocation( "mWindStrength" );
        _grassUniformArray[MOVEFACTOR] = grassShader->UniformLocation( "mMoveFactor" );
        
        grassShader->Next();
        
        // Set grass materials.
//        m_pGrassMaterial = f_AssetBase->p_MaterialBase->Find("greengrass");
        
        // Billboard stuff.
        m_billboardShader->Use();
        
        // Setup samplers.
        m_billboardShader->SetUniform( "samplerTex", 0 );
        m_billboardShader->SetUniform( "normalTex", 1 );
        
        // Setup ids.
        m_billboardShaderIds[BBSHADER_MVP] =  m_billboardShader->UniformLocation( "MVP");
        m_billboardShaderIds[BBSHADER_MATRIX] =  m_billboardShader->UniformLocation( "rotationMatrix" );
        m_billboardShaderIds[BBSHADER_PROPERTIES] = m_billboardShader->UniformLocation( "fProperties" );
        
        m_billboardShader->Next();
        
        // Create sky mesh and apply shader on it.
        MakeSky();
        
        // Create shadow maps and projections.
        m_pbEnvShadows = (CCascadedShadow *)pClassLinearAllocator->Alloc(  sizeof(CCascadedShadow) );
        m_pbEnvShadows->Init( pAllocator );
        
        g_Core->p_Console->Print( LOG_INFO, "Environment parameters:\n" );
        g_Core->p_Console->Print( LOG_INFO, "\tsunIlluminanceInLux=%4.0f\n", m_fSunLuminance );
        g_Core->p_Console->Print( LOG_INFO, "\tsunAngularRadius=%4.0f\n", m_fSunAngularRadius );
        g_Core->p_Console->Print( LOG_INFO, "\tfogSlope=%4.2f\n", fFogSlope );
        g_Core->p_Console->Print( LOG_INFO, "\tfogSkyDistance=%4.2f\n", fSkyDistance );
        g_Core->p_Console->Print( LOG_INFO, "\toceanLevel=%4.2f\n", m_fWaterMinLevel );
    }
    
    /**
     *  Create threaded ocean movement on CPU.
     */
    static void CreateOceanThread( INekoThread * thread, void * arg1, void * args )
    {
        CWorldOcean * pOcean = (CWorldOcean *) arg1;
        
        g_Core->p_Console->Print( LOG_INFO, "Creating ocean CPU thread..\n" );
        
        // Generating waves is too expensive method. That's why there are thread stuff.
        while( (g_pbEnv->m_iEnvFlags & CreatedOcean) /*&& _oceanUsesFFT*/ ) {
            pOcean->EvaluateWavesFFT( g_Core->GetTime() * 0.0009f );
        }
        
    }
    
    /**
     *  Use ocean FFT thread?
     */
    void CBeautifulEnvironment::SetOceanUsesFFTT( const bool usesFFT )
    {
        m_bIsOceanThreadActive = usesFFT;
    }
    
    /**
     *  "Setup" our Beautiful Environment!
     */
    void CBeautifulEnvironment::Makeup( const NoisePerlinParams & largeNoise,
                                       const NoisePerlinParams & mediumNoise,
                                       const NoisePerlinParams & smallNoise )
    {
        m_bLoadsWorld = true;
        
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "BeautifulEnvironment(): Creating a world..\n" );
        
        // Create landscape.
        m_pWorldHandler->SetNoiseParameters( largeNoise, mediumNoise, smallNoise );
        m_pWorldHandler->Create();
        
        m_iEnvFlags |= CreatedLandscape;
        
        m_pIndoorScene = CreateObject( "Bridge.obj", g_Core->p_Camera->vEye, Vec3(0.0f), 20.125f );
        
        // Beautiful ocean!
        if( m_pWorldOcean == NEKO_NULL ) {
            const float o_size = 1024.0f;
            SBoundingBox bbox( Vec3(-o_size, -2.0f, -o_size), Vec3(o_size, 32.0f, o_size) );
            
            // Has nothing to graphics API. Generates vertex data and precaches some assets.
            m_pWorldOcean = new CWorldOcean();//(CWorldOcean *)AllocMemory( &pClassLinearAllocator, sizeof(CWorldOcean) );
            m_pWorldOcean->pAllocator = pClassLinearAllocator;
            m_pWorldOcean->Create( bbox, 72, 0.0005f, Vec2( 3.0f, 2.5f ), 256, true, CPU_OCEAN );
            //64, 0.0005f, vector2(32.0f,32.0f), 64, false
            
            bool _oceanUsesFFT = true;//g_Core->p_ScriptBase->GetScriptVar<bool>( "env", "environment.bOcean.usesFFT" );
            
            SetOceanUsesFFTT( _oceanUsesFFT );
            
            // Set the environment flag.
            m_iEnvFlags |= CreatedOcean;
            
            const Vec3 oceanColor = Vec3( 29.0f, 122.0f, 141.0f ) / 255.0f;
            m_pWorldOcean->SetWaterColor( oceanColor.x, oceanColor.y, oceanColor.z, 1.0f );
        
            
            // Here we go. ;p
            if( m_oceanFFT == NEKO_NULL )
            {
                m_oceanFFT = CCore::CreateThread( INekoThread::PRIORITY_NORMAL, 1,
                                              CreateOceanThread, NEKO_NULL, NEKO_NULL );
            }
        }
        
        // -  -- Let's setup grass now.
        
        g_Core->p_Console->Print( LOG_INFO, "Generating grass data..\n" );
        
        //  Get grass Level of Detail values.
        m_fGrassLod1 = 100.0f;//CGrass_Lod1.Get<float>();
        m_fGrassLod2 = 50.0f;//CGrass_Lod2.Get<float>();
        
        g_Core->p_Console->Print( LOG_INFO, "Grass level of detail: %4.2f/%4.2f\n", this->m_fGrassLod1, this->m_fGrassLod2 );
        
        fGrassMoveFactor = 0.09f;
        
        // Setup grass shader.
        grassShader->Use();
        
        // Initial parameters.
        grassShader->SetUniform( _grassUniformArray[GRASSLOD], (float)m_fGrassLod2, (float)m_fGrassLod1 );
//        grassShader->SetUniform( _grassUniformArray[WINDSTREN], fWindStrength );
//        grassShader->SetUniform( _grassUniformArray[WINDDIRECTION], windDirection.x, windDirection.y );
        grassShader->SetUniform( _grassUniformArray[MOVEFACTOR], fGrassMoveFactor );
        
        grassShader->Next();
        
        // Grass cell vertex buffer.
        
        // Get mesh.
        m_pQuadMesh = f_AssetBase->p_MeshBase->m_pQuadMesh;
        
        // Update wind settings.
        UpdateWind();
        // Update assets we need.
//        SetAssetHandlers();
        
        // temp
        g_Core->p_Camera->Reset();
        
        // Set environment flags to render the world.
        m_iEnvFlags |= CreatedSky;
        m_iEnvFlags |= CreatedParticles;
        m_iEnvFlags |= CreatedEnvironment;
        
        m_bLoadsWorld = false;
        m_bRenderToCubemap = true;
        
        g_Core->p_Camera->SetUpdateCamera( true );
    }
    
    /**
     *  Set asset handles.
     */
    void CBeautifulEnvironment::SetAssetHandlers()
    {
        // Terrain textures.
        
        
        // Setup shader and shader uniforms.
        m_billboardShader = f_AssetBase->FindAssetByName<SGLShader>( "billboard" );
        // Sky noise texture used for cloud generation.
        skyNoiseTexture = f_AssetBase->p_MaterialBase->noise3dMaterial; // Procedural texture!
        moonTexture = f_AssetBase->p_MaterialBase->Find( "moon_albedo" );
        
        
        // Moon imposter.
        m_pMoonImposter = (SImposterInfo *)pAllocator->Alloc( sizeof(SImposterInfo) ) ;
        m_pMoonImposter->sampler = moonTexture;
        m_pMoonImposter->size = 128.0f;
        m_pMoonImposter->lightFlag = 0.0f;
        m_pMoonImposter->alpha = 1.0f;
        
    }
    
    /**
     *   Update wind settings.
     */
    void CBeautifulEnvironment::UpdateWind()
    {
        
    }
    
    // Preferences.
    const float Samples = 4.0f;
    
    const float fDirectionality = 0.7f;
    const float g = -fDirectionality;
    const float g2 = g * g;
    const float InnerRadius = 1.0f;
    const float OuterRadius = 1.025f;
    const float Scale = 1.0f / (OuterRadius - 1.0f);
    const float ScaleDepth = 0.25f;
    const float ScaleOverScaleDepth = Scale / ScaleDepth;
    
    const float kCameraHeight = 0.0001f;
    const float kInnerRadius2 = InnerRadius * InnerRadius;
    const float kOuterRadius2 = OuterRadius * OuterRadius;
    
    /**
     *  Make good-looking sky.
     */
    void CBeautifulEnvironment::MakeSky()
    {
#   if  !defined( NEKO_SERVER )
        
        g_Core->p_Console->Print( LOG_INFO, "Creating atmospheric sky..\n" );
        
        m_fShadowStrengthNight = 1.0f;
        m_fShadowStrengthDay = 1.0f;
        
        m_fRayleighMultiplier = 1.0f;
        m_fMieMultiplier = 1.25f;
        m_fSkyBrightness = 1.5f;
        m_fSkyContrast = 1.5f;
        m_fSkyFogginess = 0.0f;
        
        m_fkMie      = 0.0020f * m_fMieMultiplier;
        m_fkRayleigh = 0.0020f * m_fRayleighMultiplier;
        
        m_fKr4PI = m_fkRayleigh * 4.0f * nkMath::PI;
        m_fKm4PI = m_fkMie * 4.0f * nkMath::PI;
        
        m_kSunBrightness = 40.0f;
        
        m_fHaloSize = 0.1f;// HaloPower = 1.0 / HaloSize
        m_fHaloBrightness = 1.0f;
        
        // 0 fog is atmosphere color at horizon. 1 fog is atmosphere color at zenith.
        m_fHeightBias = 0.0f;
        
        // Clouds.
        float Opacity = 1.0f;
        float Coverage = 0.535f;
        float Density = 0.0f;
        float Sharpness = 0.305f;
        float Attenuation = 0.5f;
        float Saturation = 0.5f;
        float Scattering = 1.0f;
        float CloudBrightness = 1.0f;
        
        
        // Precache sky shader.
        skyShader = f_AssetBase->FindAssetByName<SGLShader>( "skyscatter" );
        skyShader->Use();
        
        // Shader paramters
        m_vkBetaMie.x = 1.5f * ((1.0f - g2) / (2.0f + g2));
        m_vkBetaMie.y = 1.0f + g2;
        m_vkBetaMie.z = 2.0f * g;
        
        
        // Set shader parameters.
        
        // Moon.
        skyShader->SetUniform( "fMoonHaloPower", 1.0f / m_fHaloSize );
        
        // Clouds.
        
        Coverage = nkMath::Mix( Coverage, 0.97f, m_fRainAmount );
        Coverage = nkMath::LerpfAlt( 0.8f, 0.0f, Coverage );
        
        Density = nkMath::LerpfAlt( 0.0f, 1.75f, Sharpness );
        
        skyShader->SetUniform( "fCoverage", Coverage );
        skyShader->SetUniform( "fDensity", Density );
        
        skyShader->SetUniform( "kBetaMie", m_vkBetaMie );
        skyShader->SetUniform( "fContrast", m_fSkyContrast );
        skyShader->SetUniform( "fBrightness", m_fSkyBrightness );
        
        skyShader->SetUniform( "fCloudBrightness", CloudBrightness );
        
        skyShader->SetUniform( "v3CameraPos", 0.0f, kCameraHeight, 0.0f );
        skyShader->SetUniform( "v3InvWavelength", fWaveInvSunColorRed, fWaveInvSunColorGreen, fWaveInvSunColorBlue );
        skyShader->SetUniform( "fCameraHeight", kCameraHeight );
        skyShader->SetUniform( "fInnerRadius", InnerRadius );
        skyShader->SetUniform( "fInnerRadius2", kInnerRadius2 );
        skyShader->SetUniform( "fOuterRadius", OuterRadius );
        skyShader->SetUniform( "fOuterRadius2", kOuterRadius2 );
        skyShader->SetUniform( "fKrESun", Vec3( fWaveInvSunColorRed, fWaveInvSunColorGreen, fWaveInvSunColorBlue ) * m_fkRayleigh * m_kSunBrightness );
        skyShader->SetUniform( "fKmESun", m_fkMie * m_kSunBrightness );
        skyShader->SetUniform( "fKr4PI", m_fKr4PI );
        skyShader->SetUniform( "fKm4PI", m_fKm4PI );
        skyShader->SetUniform( "fScale", Scale );
        skyShader->SetUniform( "fScaleDepth", ScaleDepth );
        skyShader->SetUniform( "fScaleOverScaleDepth", ScaleOverScaleDepth );
        skyShader->SetUniform( "g", g );
        skyShader->SetUniform( "g2", g * g );
        skyShader->SetUniform( "Samples", Samples );
        skyShader->SetUniform( "fNoiseTex", 0 );    // 3D noise!
        
        // Uniforms.
        skyUniforms[SModelViewProj] = skyShader->UniformLocation( "MVP" );
        skyUniforms[SSunLightPos] = skyShader->UniformLocation( "v3LocalSunLightDir" );
        skyUniforms[SMoonLightPos] = skyShader->UniformLocation( "v3LocalMoonLightDir" );
        skyUniforms[SRainAmount] = skyShader->UniformLocation( "fRainAmount");
        skyUniforms[SInvColor] = skyShader->UniformLocation(  "v3InvWavelength");
        skyUniforms[SSunRadius] = skyShader->UniformLocation(  "fKrESun");
        skyUniforms[SSunMRadius] = skyShader->UniformLocation(  "fKmESun");
        skyUniforms[SAmbientColor] = skyShader->UniformLocation( "ambientColorAndfTime" );
        skyUniforms[SCloudColor] = skyShader->UniformLocation( "cloudColor" );
        
        skyUniforms[SMoonCloudColor] = skyShader->UniformLocation("vMoonCloudColor" );
        skyUniforms[SSunCloudColor] = skyShader->UniformLocation("vSunCloudColor" );
        skyUniforms[SMoonSkyColor] = skyShader->UniformLocation("vMoonSkyColor" );
        skyUniforms[SMoonHaloColor] = skyShader->UniformLocation("vMoonHaloColor" );
        skyUniforms[SFogginess] = skyShader->UniformLocation("fFogginess" );
        skyUniforms[SFogColor] = skyShader->UniformLocation("vFogColor" );
        skyUniforms[SSunSkyColor] = skyShader->UniformLocation("vSunSkyColor" );
        
        skyShader->Next();
        
        // Create a sky dome mesh.
        m_pSkyDomeMesh = CreateObject( "geosphere.obj", Vec3(0.0f), Vec3(0.0f), 0.95f, false );
        
        m_iEnvFlags |= CreatedSky;
#   endif
    }
    
    /**
     *  Draw billboard at origin.
     */
    void CBeautifulEnvironment::DrawBillboard( const SImposterInfo    * info )
    {
        if( info->sampler == NEKO_NULL ) {
            return;
        }
        
        ncMatrix4   rotationMatrix;
        
        ncMatrix4::MakeBillboardPoint( &rotationMatrix, info->origin, g_Core->p_Camera->vEye, g_Core->p_Camera->vUp );
        
        // Use billboard shader.
        m_billboardShader->Use();
        
        // View projection.
        m_billboardShader->SetUniform( m_billboardShaderIds[BBSHADER_MVP], 1, GL_FALSE, g_Core->p_Camera->ViewProjectionMatrix.m );
        // Rotation ( billboard ) matrix.
        m_billboardShader->SetUniform( m_billboardShaderIds[BBSHADER_MATRIX], 1, GL_FALSE, rotationMatrix.m );
        // Size and another properties.
        m_billboardShader->SetUniform( m_billboardShaderIds[BBSHADER_PROPERTIES], info->size, info->lightFlag, info->alpha );
        
        // Bind a texture!
        g_mainRenderer->BindTexture( 0, info->sampler->m_pDiffuse->GetId() );
        
        if ( info->sampler->m_pNormal != NEKO_NULL ) {
            g_mainRenderer->BindTexture( 1, info->sampler->m_pNormal->GetId() );
        }
        
        // Render now!
        m_pQuadMesh->DrawIndexed();
        
        m_billboardShader->Next();
    }
    
    /**
     *  Renders all the world into cubemap.
     */
    void CBeautifulEnvironment::RenderCubemap( uint32_t msec )
    {
        if( !m_bRenderToCubemap ) {
            return;
        }
        
        int32_t     i;
        
        ncMatrix4   lookMatrix;
        ncMatrix4   matrixToRestore;
        
        
        Vec3 OZZY_COME_BACK = g_Core->p_Camera->vEye;
        const Vec3 & pos = g_Core->p_Camera->vEye;
        
        matrixToRestore = g_Core->p_Camera->ViewProjectionMatrix;
        
        // Standard view that will be overridden below.
        Vec3 vLookatPt, vUpVec;
        
        m_pCubemapFBO->BindForDrawing();
        
        for ( i = 0; i < 6; ++i ) {
            switch(i) {
                case 0: // Positive X
                    vLookatPt = Vec3(1.0f, 0.0f, 0.0f);
                    vUpVec    = Vec3(0.0f, -1.0f, 0.0f);
                    break;
                case 1: // Negative X
                    vLookatPt = Vec3(-1.0f, 0.0f, 0.0f);
                    vUpVec    = Vec3( 0.0f, -1.0f, 0.0f);
                    break;
                case 2: // Positive Y
                    vLookatPt = Vec3(0.0f, 1.0f, 0.0f);
                    vUpVec    = Vec3(0.0f, 0.0f,1.0f);
                    break;
                case 3: // Negative Y
                    vLookatPt = Vec3(0.0f,-1.0f, 0.0f);
                    vUpVec    = Vec3(0.0f, 0.0f, -1.0f);
                    break;
                case 4: // Positive Z
                    vLookatPt = Vec3( 0.0f, 0.0f, 1.0f);
                    vUpVec    = Vec3( 0.0f, -1.0f, 0.0f);
                    break;
                case 5: // Negative Y
                    vLookatPt = Vec3(0.0f, 0.0f,-1.0f);
                    vUpVec    = Vec3(0.0f, -1.0f, 0.0f);
                    break;
            }
            
            // Build a matrix.
            lookMatrix = ncMatrix4::LookAt( pos, pos + vLookatPt, vUpVec );
            
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_pEnvironmentCubemap->Image.GetId(), 0 );
            
            g_mainRenderer->SetViewportSize( m_iEnvironmentCubeSize, m_iEnvironmentCubeSize );
            g_mainRenderer->ClearWithColor( m_vSkyColor.x, m_vSkyColor.y, m_vSkyColor.z, 1.0f );
            
            int32_t flags = 0x0;
            // Render the whole visible world.
            flags |= (int32_t)EWorldFlag::Cubemap;
            
            g_Core->p_Camera->ViewProjectionMatrix = m_mCubemapProjection * lookMatrix;
            Render( CRenderer::ESceneEye::EYE_FULL, flags, msec );
        }
        // Restore previous matrix.
        g_Core->p_Camera->ViewProjectionMatrix = matrixToRestore;
        
        m_pCubemapFBO->UnbindDrawing();
        
        // Tell it that it don't need to do environment mapping since we have that in cache now.
        m_bRenderToCubemap = false;
    }
    
    /**
     *  Render terrain.
     */
    void CBeautifulEnvironment::RenderTerrain( CRenderer::ESceneEye eye, int32_t flags  )
    {
        if( !(m_iEnvFlags & CreatedLandscape) ) {
            return;
        }
        
        // Draw now.
        m_pWorldHandler->Render( eye, flags );
    }

    /**
     *  Apply alpha component on RGB.
     */
    static Vec3 ApplyAlpha( const Vec4 & color )
    {
        return Vec3( color.x * color.w, color.y * color.w, color.z * color.w );
    }
    
    /**
     *  Normalize color for HDR/LDR.
     */
    void NormalizeColor( Vec4 & vLightColor ) {
        float fLength = vLightColor.LengthSq();
        
        if( fLength > 1.0f ) {
            float len = sqrtf( fLength );
            
            vLightColor.x = vLightColor.x / len;
            vLightColor.y = vLightColor.y / len;
            vLightColor.z = vLightColor.z / len;
        }
        
        float fLightAmount = ( vLightColor.x + vLightColor.y + vLightColor.z ) / 3.0;
        
        vLightColor.x = vLightColor.x * fLightAmount;
        vLightColor.y = vLightColor.y * fLightAmount;
        vLightColor.z = vLightColor.z * fLightAmount;
    }
    
    /**
     *  Get color gradient.
     */
    Vec4 GetColor( const Vec4 &col0, const Vec4 &col1, const Vec4 &col2,
                   const float alpha0, const float alpha1, const float alpha2,
                   float a, const bool applyAlpha = true, const float middleAlpha = 0.5f ) {
        
        Vec4 res;
        float alpha;
        
        if( a <= middleAlpha ) {
            res = col1 * a * 2.0 + col0 * (middleAlpha - a) * 2.0;
            alpha = alpha1 * a * 2.0 + alpha0 * (middleAlpha - a) * 2.0;
        } else {
            res = col2 * (a - middleAlpha) * 2.0 + col1 * (1.0 - a) * 2.0;
            alpha = alpha2 * (a - middleAlpha) * 2.0 + alpha1 * (1.0 - a) * 2.0;
        }
        
        return Vec4( res.x, res.y, res.z, applyAlpha ? alpha : 1.0f );
    }
    
    /**
     *  Update atmospheric stuff.
     */
    void CBeautifulEnvironment::UpdateEnviroment( const uint32_t msec )
    {
        Vec4   vSunColor;
        Vec4   vLightColor;
        Vec4   vGlobalFogColor, vMoonFogColor;
        Vec4   vMoonColor;
        
        float   sunMultiplier = 0.15f;
        float   dayTimeToggle = 1.0f;
        float   sunIntensityMultiplier = 0.625f;
        
        // Sun information.
        
        // Calculate game day-night time.
        float fTime = m_fTime;//  + (g_Core->GetFrameTime() * m_fAnimSpeed);
        
        float lst_rad, sun_zenith_rad, sun_altitude_rad, sun_azimuth_rad,
        moon_zenith_rad, moon_altitude_rad, moon_azimuth_rad;
        
        // Celestial computations
        {
            // Local latitude
            float lat_rad = nkMath::Deg2Rad * m_fLatitude;
            float lat_sin = sinf(lat_rad);
            float lat_cos = cosf(lat_rad);
            
            // Local longitude
            float lon_deg = m_fLongitude;
            
            // Horizon angle
            float horizon_rad = nkMath::Deg2Rad * 90.0f ;
            
            // Date
            int32_t   year  = m_iYear;
            int32_t   month = m_iMonth;
            int32_t   day   = m_iDay;
            float   hour  = fTime - m_fUTC; // m_iHour

            // Time scale
            float d = 367 * year - 7 * (year + (month + 9) / 12) / 4 + 275 * month / 9 + day - 730530 + hour / 24.0f;
            float d_noon = 367 * year - 7 * (year + (month + 9) / 12) / 4 + 275 * month / 9 + day - 730530 + 12.0f / 24.0f;
            
            // Tilt of earth's axis of rotation
            float ecl = 23.4393f - 3.563E-7f * d;
            float ecl_rad = nkMath::Deg2Rad * ecl ;
            float ecl_sin = sinf(ecl_rad);
            float ecl_cos = cosf(ecl_rad);
            
            // Sunset and sunrise
            {
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#4
                
                float w = 282.9404f + 4.70935E-5f * d_noon;
                float e = 0.016709f - 1.151E-9f * d_noon;
                float M = 356.0470f + 0.9856002585f * d_noon;
                
                float M_rad = nkMath::Deg2Rad * M ;
                float M_sin = sinf(M_rad);
                float M_cos = cosf(M_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#5
                
                float E_rad = M_rad + e * M_sin * (1.0f + e * M_cos);
                float E_sin = sinf(E_rad);
                float E_cos = cosf(E_rad);
                
                float xv = E_cos - e;
                float yv = sqrtf(1.0f - e*e) * E_sin;
                
                float v = nkMath::Rad2Deg * atan2f(yv, xv);
                float r = sqrtf(xv*xv + yv*yv);
                
                float l_deg = v + w;
                float l_rad = nkMath::Deg2Rad * l_deg;
                float l_sin = sinf(l_rad);
                float l_cos = cosf(l_rad);
                
                float xs = r * l_cos;
                float ys = r * l_sin;
                
                float xe = xs;
                float ye = ys * ecl_cos;
                float ze = ys * ecl_sin;
                
                float rasc_rad = atan2f(ye, xe);
                float rasc_deg = nkMath::Rad2Deg * rasc_rad;
                float decl_rad = atan2f(ze, sqrtf(xe*xe + ye*ye));
                float decl_sin = sinf(decl_rad);
                float decl_cos = cosf(decl_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#5b
                
                float Ls = v + w;
                
                float GMST0_deg = Ls + 180.0f;
                
                // See http://www.stjarnhimlen.se/comp/riset.html#2
                
                float ut_deg = rasc_deg - GMST0_deg - lon_deg;
                
                float twilight_deg = -6.0f;
                float twilight_rad = nkMath::Deg2Rad * twilight_deg;
                float twilight_sin = sinf(twilight_rad);
                
                float lha_cos = (twilight_sin - lat_sin * decl_sin) / (lat_cos * decl_cos);
                float lha_rad = acosf(lha_cos);
                float lha_deg = nkMath::Rad2Deg * lha_rad;
                
                SunsetTime  = (24.0f + fmodf(fmodf(((ut_deg + lha_deg) / 15.0f), 24.0f), 24.0f));
                SunriseTime = (24.0f + fmodf(fmodf(((ut_deg - lha_deg) / 15.0f), 24.0f), 24.0f));
            }
            
            // Sun position
            {
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#4
                
                float w = 282.9404f + 4.70935E-5f * d;
                float e = 0.016709f - 1.151E-9f * d;
                float M = 356.0470f + 0.9856002585f * d;
                
                float M_rad = nkMath::Deg2Rad * M;
                float M_sin = sinf(M_rad);
                float M_cos = cosf(M_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#5
                
                float E_rad = M_rad + e * M_sin * (1.0f + e * M_cos);
                float E_sin = sinf(E_rad);
                float E_cos = cosf(E_rad);
                
                float xv = E_cos - e;
                float yv = sqrtf(1.0f - e*e) * E_sin;
                
                float v = nkMath::Rad2Deg * atan2f(yv, xv);
                float r = sqrtf(xv*xv + yv*yv);
                
                float l_deg = v + w;
                float l_rad = nkMath::Deg2Rad * l_deg;
                float l_sin = sinf(l_rad);
                float l_cos = cosf(l_rad);
                
                float xs = r * l_cos;
                float ys = r * l_sin;
                
                float xe = xs;
                float ye = ys * ecl_cos;
                float ze = ys * ecl_sin;
                
                float rasc_rad = atan2f(ye, xe);
                float decl_rad = atan2f(ze, sqrtf(xe*xe + ye*ye));
                float decl_sin = sinf(decl_rad);
                float decl_cos = cosf(decl_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#5b
                
                float Ls = v + w;
                
                float GMST0_deg = Ls + 180.0f;
                float GMST_deg  = GMST0_deg + 15.0f * hour;
                
                lst_rad = nkMath::Deg2Rad * (GMST_deg + lon_deg);
                
                LocalSiderealTime = (GMST_deg + lon_deg) / 15.0f;
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#12b
                
                float HA_rad = lst_rad - rasc_rad;
                float HA_sin = sinf(HA_rad);
                float HA_cos = cosf(HA_rad);
                
                float x = HA_cos * decl_cos;
                float y = HA_sin * decl_cos;
                float z = decl_sin;
                
                float xhor = x * lat_sin - z * lat_cos;
                float yhor = y;
                float zhor = x * lat_cos + z * lat_sin;
                
                float azimuth  = atan2f(yhor, xhor) + nkMath::Deg2Rad * 180.0f;
                float altitude = atan2f(zhor, sqrtf(xhor*xhor + yhor*yhor));
                
                sun_zenith_rad   = horizon_rad - altitude;
                sun_altitude_rad = altitude;
                sun_azimuth_rad  = azimuth;
            }
            
            SunZenith   = nkMath::Rad2Deg * sun_zenith_rad;
            SunAltitude = nkMath::Rad2Deg * sun_altitude_rad;
            SunAzimuth  = nkMath::Rad2Deg * sun_azimuth_rad;
            
            // Moon position
            if( m_bTimeOfDayManual == false )
            {
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#4
                
                float N = 125.1228f - 0.0529538083f * d;
                float i = 5.1454f;
                float w = 318.0634f + 0.1643573223f * d;
                float a = 60.2666f;
                float e = 0.054900f;
                float M = 115.3654f + 13.0649929509f * d;
                
                float N_rad = nkMath::Deg2Rad * N;
                float N_sin = sinf(N_rad);
                float N_cos = cosf(N_rad);
                
                float i_rad = nkMath::Deg2Rad * i;
                float i_sin = sinf(i_rad);
                float i_cos = cosf(i_rad);
                
                float M_rad = nkMath::Deg2Rad * M;
                float M_sin = sinf(M_rad);
                float M_cos = cosf(M_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#6
                
                float E_rad = M_rad + e * M_sin * (1.0f + e * M_cos);
                float E_sin = sinf(E_rad);
                float E_cos = cosf(E_rad);
                
                float xv = a * (E_cos - e);
                float yv = a * (sqrtf(1.0f - e*e) * E_sin);
                
                float v = nkMath::Rad2Deg * atan2f(yv, xv);
                float r = sqrtf(xv*xv + yv*yv);
                
                float l_deg = v + w;
                float l_rad = nkMath::Deg2Rad * l_deg;
                float l_sin = sinf(l_rad);
                float l_cos = cosf(l_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#7
                
                float xh = r * (N_cos * l_cos - N_sin * l_sin * i_cos);
                float yh = r * (N_sin * l_cos + N_cos * l_sin * i_cos);
                float zh = r * (l_sin * i_sin);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#11
                
                float xg = xh;
                float yg = yh;
                float zg = zh;
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#12
                
                float xe = xg;
                float ye = yg * ecl_cos - zg * ecl_sin;
                float ze = yg * ecl_sin + zg * ecl_cos;
                
                float rasc_rad = atan2f(ye, xe);
                float decl_rad = atan2f(ze, sqrtf(xe*xe + ye*ye));
                float decl_sin = sinf(decl_rad);
                float decl_cos = cosf(decl_rad);
                
                // See http://www.stjarnhimlen.se/comp/ppcomp.html#12b
                
                float HA_rad = lst_rad - rasc_rad;
                float HA_sin = sinf(HA_rad);
                float HA_cos = cosf(HA_rad);
                
                float x = HA_cos * decl_cos;
                float y = HA_sin * decl_cos;
                float z = decl_sin;
                
                float xhor = x * lat_sin - z * lat_cos;
                float yhor = y;
                float zhor = x * lat_cos + z * lat_sin;
                
                float azimuth  = atan2f(yhor, xhor) + nkMath::Deg2Rad * 180.0f;
                float altitude = atan2f(zhor, sqrtf(xhor*xhor + yhor*yhor));
                
                moon_zenith_rad   = horizon_rad - altitude;
                moon_altitude_rad = altitude;
                moon_azimuth_rad  = azimuth;
            }
            else  // Manual.
            {
                moon_zenith_rad   = sun_zenith_rad - nkMath::PI;
                moon_altitude_rad = sun_altitude_rad - nkMath::PI;
                moon_azimuth_rad  = sun_azimuth_rad;
            }
            
            MoonZenith   = nkMath::Rad2Deg * moon_zenith_rad;
            MoonAltitude = nkMath::Rad2Deg * moon_altitude_rad;
            MoonAzimuth  = nkMath::Rad2Deg * moon_azimuth_rad;
        }
        
        // Sun position.
        m_vSunCenterPos = Vec3::OrbitalToLocal(sun_zenith_rad, sun_azimuth_rad);
        m_vMoonCenterPos = Vec3::OrbitalToLocal(moon_zenith_rad, moon_azimuth_rad);
        
        // Sunlight direction.
        m_vSunDirection = Vec3::Normalize( m_vSunCenterPos );
        m_vSunPosition = g_Core->p_Camera->vEye + m_vSunCenterPos;
        // Moon light direction.
        m_vMoonDirection = Vec3::Normalize( m_vMoonCenterPos );
        
        if( m_fTime < m_dawnStart || m_fTime >= m_duskEnd ) {
            // Night.
            dayTimeToggle = 0.0f;
            sunIntensityMultiplier = 0.0;
        } else if( m_fTime < m_dawnEnd ) {
            // Dawn.
            float b = 0.5f * (m_dawnStart + m_dawnEnd);
            
            if( m_fTime < b ) {
                // Fade out moon.
                sunMultiplier *= (b - m_fTime) / (b - m_dawnStart);
                sunIntensityMultiplier = 0.0;
            } else {
                // Fade in sun.
                float t = (m_fTime - b) / (m_dawnEnd - b);
                sunMultiplier *= t;
                sunIntensityMultiplier = t;
            }
            
            dayTimeToggle = (m_fTime - m_dawnStart) / (m_dawnEnd - m_dawnStart);
        } else if( m_fTime < m_duskStart ) {
            // day
            dayTimeToggle = 1.0;
        } else if( m_fTime < m_duskEnd ) {
            // dusk
            float b = 0.5f * (m_duskStart + m_duskEnd);
            if (m_fTime < b) {
                // fade out sun
                
                float t((b - m_fTime) / (b - m_duskStart));
                sunIntensityMultiplier = t;
                sunMultiplier *= t;
            } else {
                // fade in moon
                
                sunMultiplier *= (m_fTime - b) / (m_duskEnd - b);
                sunIntensityMultiplier = 0.0;
            }
            
            dayTimeToggle = (m_duskEnd - m_fTime) / (m_duskEnd - m_duskStart);
        }
        
        // Calculate sun, fog colors.
        
        // Falls off the darker the sunlight gets.
        // +1 at day
        // 0 at night
        float mix = nkMath::InverseLerp( 105.0f, 90.0f, SunZenith );
        
        float dayTime = nkMath::Clamp01( SunZenith / 90.0f );
        float nightTime = nkMath::Clamp01( (SunZenith - 90.0f) / 90.0f );
        
        const float lerpThreshold = 0.1f;
        const float falloffAngle = 5.0f;
        
        float sunLerpValue = nkMath::Clamp01( (mix - lerpThreshold) / (1.0f - lerpThreshold) );
        float moonLerpValue = nkMath::Clamp01( (lerpThreshold - mix) / lerpThreshold );
        float moonAboveHorizon = nkMath::Clamp01( (90.0f - moon_zenith_rad * nkMath::Rad2Deg) / falloffAngle );
        
        // TODO: color key pairs
        Vec4 sunLightCol;
        {
            Vec4 col0 = Vec4(255,243,234,255);
            Vec4 col1 = Vec4(255,243,234,255);
            Vec4 col2 = Vec4(255,154,0,255);
            
            float alpha0 = 1.0f;
            float alpha1 = 1.0f;
            float alpha2 = 1.0f;
            
            sunLightCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, dayTime );
        }
        
        Vec4 moonLightCol;
        {
            Vec4 col0 = Vec4(25,40,65,255);
            Vec4 col1 = Vec4(25,40,65,255);
            Vec4 col2 = Vec4(25,40,65,255);
            
            float alpha0 = 1.0f;
            float alpha1 = 1.0f;
            float alpha2 = 1.0f;
            
            moonLightCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, nightTime );
        }
        
        Vec4 sunSkyCol;
        {
            Vec4 col0 = Vec4(255,243,234,255);
            Vec4 col1 = Vec4(255,243,234,255);
            Vec4 col2 = Vec4(255,243,234,255);
            
            float alpha0 = 1.0f;
            float alpha1 = 1.0f;
            float alpha2 = 1.0f;
            
            sunSkyCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, dayTime, false );
        }
        
        Vec4 moonSkyCol;
        {
            Vec4 col0 = Vec4(25,40,65,255);
            Vec4 col1 = Vec4(25,40,65,255);
            Vec4 col2 = Vec4(25,40,65,255);
            
            float alpha0 = 0.5f;
            float alpha1 = 1.0f;
            float alpha2 = 0.5f;
            
            moonSkyCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, nightTime );
        }
        
        Vec4 sunCloudCol;
        {
            Vec4 col0 = Vec4(224,235,255,255);
            Vec4 col1 = Vec4(224,235,255,255);
            Vec4 col2 = Vec4(255,195,145,255);
            
            float alpha0 = 1.0f;
            float alpha1 = 1.0f;
            float alpha2 = 1.0f;
            
            sunCloudCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, dayTime );
        }
        
        Vec4 moonCloudCol;
        {
            Vec4 col0 = Vec4(25,40,65,255);
            Vec4 col1 = Vec4(25,40,65,255);
            Vec4 col2 = Vec4(25,40,65,255);
            
            float alpha0 = 0.5f;
            float alpha1 = 1.0f;
            float alpha2 = 0.5f;
            
            moonCloudCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2,nightTime );
        }
        
        Vec4 sunFogCol;
        {
            Vec4 col0 = Vec4(191,191,191,255);
            Vec4 col1 = Vec4(191,191,191,255);
            Vec4 col2 = Vec4(127,127,127,255);
            
            float alpha0 = 1.0f;
            float alpha1 = 1.0f;
            float alpha2 = 1.0f;
            
            sunFogCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2,dayTime );
        }
        
        Vec4 moonFogCol;
        {
            Vec4 col0 = Vec4(25,40,65,255);
            Vec4 col1 = Vec4(25,40,65,255);
            Vec4 col2 = Vec4(25,40,65,255);
            
            float alpha0 = 0.2f;
            float alpha1 = 1.0f;
            float alpha2 = 0.2f;
            
            moonFogCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2,nightTime );
        }
        
        Vec4 sunAmbientCol;
        {
            Vec4 col0 = Vec4(94,89,87,255);
            Vec4 col1 = Vec4(94,89,87,255);
            Vec4 col2 = Vec4(94,89,87,255);
            
            float alpha0 = 1.0f;
            float alpha1 = 1.0f;
            float alpha2 = 1.0f;
            
            sunAmbientCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, dayTime );
        }
        
        Vec4 moonAmbientCol;
        {
            Vec4 col0 = Vec4(25,40,65,255);
            Vec4 col1 = Vec4(25,40,65,255);
            Vec4 col2 = Vec4(25,40,65,255);
            
            float alpha0 = 0.2f;
            float alpha1 = 1.0f;
            float alpha2 = 0.2f;
            
            moonAmbientCol = GetColor( col0, col1, col2, alpha0, alpha1, alpha2, nightTime );
        }
        
        // Sun and moon visibility
        const float SunVisibility  = (1.0f - m_fFogginess) * sunLerpValue;
        const float MoonVisibility = (1.0f - m_fFogginess) * moonLerpValue * moonAboveHorizon;
    
        // Intensity of the light source.
        float lightIntensity;
        
        // Opacity of the shadows dropped by the light source.
        float lightShadowStrength;
        Vec4 lightColor;
        
        bool isDay, isNight;
        
        if( mix > lerpThreshold ) {
            isDay = true;
            isNight = false;
            
            // day parameters
            lightShadowStrength = m_fShadowStrengthDay;
            lightIntensity = nkMath::Mix(0.0f, m_fLightIntensityDay, SunVisibility);
            lightColor = sunLightCol;
        } else {
            isDay = false;
            isNight = true;
            
            // night parameters
            lightShadowStrength = m_fShadowStrengthNight;
            lightIntensity = nkMath::Mix(0.0f, m_fLightIntensityNight, MoonVisibility);
            lightColor = moonLightCol;
        }

        if( m_fTimeSinceLightUpdate >= m_fSkyUpdateInterval ) {
            m_vCurrentLightDir = isNight ? Vec3::OrbitalToLocal( nkMath::Min(moon_zenith_rad, (1.0f - m_fLightMinimumHeight) * nkMath::PI / 2.0f), moon_azimuth_rad) :
            Vec3::OrbitalToLocal( nkMath::Min(sun_zenith_rad, (1.0f - m_fLightMinimumHeight) * nkMath::PI / 2.0f), sun_azimuth_rad );
        } else {
            m_fTimeSinceLightUpdate += g_Core->frameMsec / 1000.0f;
        }
        
        vSunColor = lightColor;
        vLightColor = Vec4::Mix( moonSkyCol, sunSkyCol, mix );
        vGlobalFogColor = Vec4::Mix( moonFogCol, sunFogCol, mix );
        
        
        // Normalize colors (for HDR/LDR)
        // Take into account lighting first
        NormalizeColor( vLightColor );
        // Normalize sun color;
        NormalizeColor( vSunColor );
        // Normalize fog color;
        NormalizeColor( vGlobalFogColor );
        // Normalize sun cloud color;
        NormalizeColor( sunCloudCol );
        // Normalize moon cloud color;
        NormalizeColor( moonCloudCol );
        // Normalize sun and moon sky cloud colors;
        NormalizeColor( moonSkyCol );
        NormalizeColor( sunSkyCol );
        
        /**     Set light colors.       **/
        
        // Final color combination.
        m_vSunColor = ApplyAlpha( vSunColor );
        m_vSkyColor = ApplyAlpha( vLightColor );
        m_vGlobalFogColor = SampleFogColor().xyz();// ApplyAlpha( vGlobalFogColor );
        m_vSunCloudColor = ApplyAlpha( sunCloudCol );
        m_vMoonCloudColor = ApplyAlpha( moonCloudCol );
        m_vMoonSkyColor = ApplyAlpha( moonSkyCol );
        m_vSunSkyColor = ApplyAlpha( sunSkyCol );
        
        m_vMoonHaloColor = m_vMoonCloudColor * 1.0f /*HaloBrightness*/ * moonAboveHorizon;
        
        m_pbEnvShadows->SetOpacity( lightShadowStrength );  // set global shadow opacity ( light strength )
        
        // Compute this on CPU, no need to waste GPU.
        m_pMoonImposter->alpha = 0.1f + MoonVisibility; // Moon alpha.
        
        //        fSkyLightAmbientIntensity = 1.0;  // removed since LPV/PBR
        //        m_bRenderToCubemap = false;
    }
    
    float ShaderScale(float inCos)
    {
        float x = 1.0f - inCos;
        return 0.25f * expf(-0.00287f + x*(0.459f + x*(3.83f + x*(-6.80f + x*5.25f))));
    }
    
    float ShaderRayleighPhase(float eyeCos2)
    {
        return 0.75f + 0.75f * eyeCos2;
    }
    
    float ShaderMiePhase(float eyeCos, float eyeCos2)
    {
        return g_pbEnv->m_vkBetaMie.x * (1.0f + eyeCos2) / powf(g_pbEnv->m_vkBetaMie.y + g_pbEnv->m_vkBetaMie.z * eyeCos, 1.5f);
    }
    
    Vec3 ShaderNightSkyColor(Vec3 dir, const Vec3 & moonCol)
    {
        dir.y = nkMath::Max(0.0f, dir.y);
        return moonCol * (1.0f - 0.75f * dir.y);
    }
    
    Vec3 ShaderMoonHaloColor(const Vec3 & dir, const Vec3 & moonHaloCol, const Vec3 & moonDir)
    {
        return moonHaloCol * powf(fmax(0.0f, Vec3::Dot(dir, moonDir)), g_pbEnv->m_fHaloSize);
    }
    
    Vec4 HDR2LDR(Vec4 & color, const float Brightness = 1.0f)
    {
        return Vec4(1.0f - powf(2.0f, -Brightness * color.x), 1.0f - powf(2.0f, -Brightness * color.y), 1.0f - powf(2.0f, -Brightness * color.z), color.w);
    }
    
    Vec4  Linear2Gamma(Vec4 & color)
    {
        return  Vec4(sqrtf(color.x), sqrtf(color.y), sqrtf(color.z), color.w);
    }
    
    /**
     *  Calculate the equator color. 
     */
    Vec4   CBeautifulEnvironment::SampleEquatorColor()
    {
        Vec3 sample = m_vSunDirection;
        sample.y = 0.0f;
        
        Vec4 color = SampleAtmosphere( sample, false );
        
        return Vec4( color.xyz(), 1.0f );
    }
    
    /**
     *  Calculate sky color.
     */
    Vec4   CBeautifulEnvironment::SampleSkyColor()
    {
        Vec3 sample = m_vSunDirection;
        sample.y = nkMath::Abs( sample.y );
        
        Vec4 color = SampleAtmosphere( sample, false );
        
        return Vec4( color.xyz(), 1.0f );
    }
    
    /**
     *  Sample fog color.
     */
    Vec4   CBeautifulEnvironment::SampleFogColor()
    {
        Vec3 dir = Vec3::Mix( g_Core->p_Camera->vLook * Vec3( 1.0f, 0.0f, 0.0f ), VECTOR_UP, m_fHeightBias );
        Vec4 color = SampleAtmosphere( dir, false );
        
        return Vec4( color.xyz(), 1.0f );
    }
    
    /**
     *  Sample atmosphere at direction.
     */
    Vec4 CBeautifulEnvironment::SampleAtmosphere(Vec3 dir, const bool directLight)
    {
        Vec4 color;
        
        dir.y = nkMath::Max( 0.0f, dir.y );
        color = SkyScatterColor( dir, directLight );
        
        color = HDR2LDR( color );   // convert to LDR
        color = Linear2Gamma( color );
        
        return color;
    }
    
    /**
     *  Scattering color.
     *
     *  @param dir         Direction to sample.
     *  @param directLight Include direct light ( sun/moon )?
     *
     *  @return     Color in HDR.
     */
    Vec4 CBeautifulEnvironment::SkyScatterColor(Vec3 & dir, const bool directLight)
    {
        float fFar =  sqrt(kOuterRadius2 + kInnerRadius2 * dir.y * dir.y - kInnerRadius2) - InnerRadius * dir.y;
        
        // Ray starting position.
        Vec3 v3Start = Vec3(0.0, InnerRadius + kCameraHeight, 0.0);
        float fDepth = exp( ScaleOverScaleDepth * ( - kCameraHeight) );
        
        float fStartAngle = Vec3::Dot( dir, v3Start ) / (InnerRadius + kCameraHeight);
        float fStartOffset = fDepth * ShaderScale( fStartAngle );
        
        // Scattering.
        float fSampleLength = fFar / Samples;
        float fScaledLength = fSampleLength * Scale;
        
        Vec3 v3SampleRay = dir * fSampleLength;
        Vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
        
        // Now loop through the sample rays
        Vec3 sunColor = Vec3(0.0, 0.0, 0.0);
        
        Vec3 waveInv = Vec3(fWaveInvSunColorRed, fWaveInvSunColorGreen, fWaveInvSunColorBlue);
        for( int32_t i( 0 ); i < int32_t( Samples ); ++i ) {
            float fHeight = v3SamplePoint.Length();
            float invHeight = 1.0 / fHeight;
            
            float fDepth = expf( ScaleOverScaleDepth * ( InnerRadius - fHeight ) );
            
            float fLightAngle = Vec3::Dot( m_vSunDirection, v3SamplePoint ) * invHeight;
            float fCameraAngle = Vec3::Dot( dir, v3SamplePoint ) * invHeight;
            float fScatter = ( fStartOffset + fDepth * ( ShaderScale( fLightAngle ) - ShaderScale( fCameraAngle ) ) );
            
            Vec3 exp;
            
            exp.x = expf( -fScatter * ( waveInv.x * m_fKr4PI + m_fKm4PI ) );
            exp.y = expf( -fScatter * ( waveInv.y * m_fKr4PI + m_fKm4PI ) );
            exp.z = expf( -fScatter * ( waveInv.z * m_fKr4PI + m_fKm4PI ) );
            
            Vec3 v3Attenuate = exp;
            
            sunColor = sunColor + (v3Attenuate * (fDepth * fScaledLength));
            v3SamplePoint = v3SamplePoint + v3SampleRay;
        }
        
        
        // Mie and Rayleigh colors.
        Vec3 vOutScatter = m_vSunSkyColor * sunColor * m_fkMie * m_kSunBrightness;
        Vec3 vInScatter = m_vSunSkyColor * sunColor * (waveInv * (m_fkRayleigh * m_kSunBrightness));
        
        // Rayleigh and Mie calculations.
        
        Vec3 resultColor = Vec3( 0.0f );
        
        // Sun angle
        float sunCos  = Vec3::Dot(m_vSunDirection, dir);
        float sunCos2 = sunCos * sunCos;
        
        // Add sun light
        float rayleighPhase = ShaderRayleighPhase(sunCos2);
        
        resultColor = resultColor + (rayleighPhase * vInScatter);
        
        if( directLight ) {
            float miePhase = ShaderMiePhase(sunCos, sunCos2);
            
            resultColor = resultColor + (miePhase * vOutScatter);
        }
        
        // Add moon light
        Vec3 nightSkyColor = ShaderNightSkyColor(dir, m_vMoonSkyColor);
        
        resultColor = resultColor + nightSkyColor;
        
        if( directLight ) {
            Vec3 moonHaloColor = ShaderMoonHaloColor(dir, m_vMoonHaloColor, m_vMoonDirection);
            
            resultColor = resultColor + moonHaloColor;
        }
        
        // Lerp to fog color
        resultColor = Vec3::Mix(resultColor, m_vGlobalFogColor, m_fFogginess);
        
        // Adjust output color
        resultColor.x = powf(resultColor.x * m_fSkyBrightness, m_fSkyContrast);
        resultColor.y = powf(resultColor.y * m_fSkyBrightness, m_fSkyContrast);
        resultColor.z = powf(resultColor.z * m_fSkyBrightness, m_fSkyContrast);
        
        return Vec4(resultColor , 1.0);
    }
    
    /**
     *  Create object.
     */
    ncMesh * CBeautifulEnvironment::CreateObject( const char *name, const Vec3 & origin, const Vec3 & rotation, const float & scale, const bool hasSphere )
    {
        ncMesh * tMeshHolder = NEKO_NULL;
        // Create a mesh alias with its handle.
        tMeshHolder = (ncMesh *)pAllocator->Alloc( sizeof( ncMesh )  );
        // Set object properties.
        tMeshHolder->SetObjectPos( origin );
        tMeshHolder->SetRotation( rotation );
        // Default scale.
        tMeshHolder->SetScale( scale );
        // Set the mesh.
        tMeshHolder->SetOwner( f_AssetBase->p_MeshBase->Find( name ) );
        // Setup modelview matrix and another stuff.
        tMeshHolder->Refresh();
        
        // Create a bounding sphere.
        if( hasSphere ) {
            tMeshHolder->m_pbSphere = *tMeshHolder->GetHandle()->GetBoundingSphere();
            tMeshHolder->m_pbSphere.SetScale( scale );
            tMeshHolder->m_pbSphere.SetCenter( origin );
        }
        
        // Set the object flag.
        tMeshHolder->SetObjectFlag( CPhysicsObject::PHYS_NONMOVABLE );
        
        return tMeshHolder;
    }
    
    /**
     *  Test g_pbEnvironment feedback.
     */
    void CBeautifulEnvironment::Feedback()
    {
        
    }
    
    /**
     *  Bind terrain textures.
     */
    void CBeautifulEnvironment::BindTerrainSummerTextures()
    {
        // Bind textures.
        
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D_ARRAY, m_pLandscapeMaterial[0]->m_pDiffuse->Image.GetId() );
        
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D_ARRAY, m_pLandscapeMaterial[0]->m_pNormal->Image.GetId() );
        
        glActiveTexture( GL_TEXTURE2 );
        glBindTexture( GL_TEXTURE_2D_ARRAY, m_pLandscapeMaterial[0]->m_pSpecular->Image.GetId() );
    }
    
#ifndef NEKO_SERVER
    
    /**
     *  Initialize point light renderer.
     */
    void COffscreenLightRenderer::Initialize()
    {
        // Offscreen light renderer size.
        g_mainRenderer->pointlightOffscreenWidth = 640;
        g_mainRenderer->pointlightOffscreenHeight = 480;
        
        
        m_iSceneLights = 0;
        m_iSceneMultiLights = 0;
        
        uNormalTexture = 0;
        uDepthTexture = 0;
        
        glEnable( GL_TEXTURE_2D );
        
        // Setup point light shader.
        pointLightShader = f_AssetBase->FindAssetByName<SGLShader>( "pointlight" );
        pointLightShader->Use();
        
        uNormalTexture = pointLightShader->UniformLocation( "normalTexture" );
        uDepthTexture = pointLightShader->UniformLocation( "depthTexture" );
        
        pointLightShader->SetUniform( uNormalTexture, 0 );
        pointLightShader->SetUniform( uDepthTexture, 1 );
        
        m_uLightPasses[LightUniforms::Origin] = pointLightShader->UniformLocation( "center" );
        m_uLightPasses[LightUniforms::LightSize]= pointLightShader->UniformLocation( "lightSize" );
        m_uLightPasses[LightUniforms::LightColor]= pointLightShader->UniformLocation( "lightColor" );
        m_uLightPasses[LightUniforms::ObjectMatrix] = pointLightShader->UniformLocation( "objectMatrix" );
        
        m_uLightPasses[LightUniforms::WorldMatrix] = pointLightShader->UniformLocation( "worldMatrix" );
        m_uLightPasses[LightUniforms::PerspectiveMatrix] = pointLightShader->UniformLocation( "perspectiveMatrix" );
        m_uLightPasses[LightUniforms::PerspectiveInvMatrix] = pointLightShader->UniformLocation( "perspectiveInvMatrix" );
        
        // Set properties.
        pointLightShader->SetUniform( m_uLightPasses[LightUniforms::PerspectiveMatrix], 1, GL_FALSE, g_Core->p_Camera->ProjectionMatrix.m );
        pointLightShader->SetUniform( m_uLightPasses[LightUniforms::PerspectiveInvMatrix], 1, GL_FALSE, g_Core->p_Camera->ProjectionInvertMatrix.m );
        
        pointLightShader->Next();
        
        //! Create framebuffer for offscreen pointlights.
        pointLightFramebuffer.Create( g_mainRenderer->pointlightOffscreenWidth, g_mainRenderer->pointlightOffscreenHeight, true, true, true, false, false, false, GL_RGBA16F );
        
        pointLightFramebuffer.BindForDrawing();
        pointLightFramebuffer.Resize( g_mainRenderer->pointlightOffscreenWidth,
                                     g_mainRenderer->pointlightOffscreenHeight );
        pointLightFramebuffer.UnbindDrawing();
        
        
        m_pLightSpheres[0] = ncGeometrySphere();
        m_pLightSpheres[0].Create( LIGHTRADIUS_MEDIUM * 1.1f, 8, 6 );
        
        glDisable( GL_TEXTURE_2D );
    }
    
    /**
     *  Create an offscreen point light!
     */
    void COffscreenPointlight::Create( float radius, float r, float g, float b, float radiusDraw )
    {
        fLightSize = radius;
        m_vColor = Vec3( r, g, b );
        
        //        dSphere.Create( Vec4( r, g, b, 1.0f ), radius * 1.1f, 4, 3 );
        rSphere = dSphere = new ncGeometrySphere( g_pbEnv->m_pPointLightRenderer->m_pLightSpheres[0] );
        //        rSphere->Create( radius * 1.1f, 8, 6 );
    }
    
    /**
     *  Set light position.
     */
    void COffscreenPointlight::SetPosition( const Vec3 &position )
    {
        dSphere->_position = position;
        rSphere->_position = position;
        
        dSphere->objectMatrix.Translate( dSphere->_position );
        rSphere->objectMatrix.Translate( rSphere->_position );
    }
    
    /**
     *  Begin rendering pointlights.
     */
    void COffscreenLightRenderer::StartRender()
    {
        glEnable( GL_DEPTH_TEST );
        
        pointLightFramebuffer.BindForDrawing();
        
        //! Clear color.
        g_mainRenderer->ClearColorDepthWithColor( 0.0f, 0.0f, 0.0f, 1.0f );
        g_mainRenderer->SetViewportSize( g_mainRenderer->pointlightOffscreenWidth, g_mainRenderer->pointlightOffscreenHeight );
        
        //! Use pointlight shader for normal applying.
        pointLightShader->Use();
        
        //! Disable depth writes.
        glDepthMask( GL_FALSE );
        
        //! Bind scene textures.
        g_mainRenderer->BindTexture( 1, g_mainRenderer->g_sceneBuffer[CRenderer::EYE_FULL]->GetUnitDepth() );
        g_mainRenderer->BindTexture( 0, g_mainRenderer->g_sceneBuffer[CRenderer::EYE_FULL]->GetUnitRT1() );
        
        //! Bind view matrix.
        pointLightShader->SetUniform( m_uLightPasses[LightUniforms::WorldMatrix], 1, GL_FALSE, g_Core->p_Camera->ViewMatrix.m );
    }
    
    /**
     *  Stop rendering lights.
     */
    void COffscreenLightRenderer::StopRender()
    {
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        
        pointLightShader->Next();
        pointLightFramebuffer.UnbindDrawing();
        
        //! Enable depth writes again.
        g_mainRenderer->SetDepthWriteMode( true );
        
        glDisable( GL_DEPTH_TEST );
        
    }
    
    /**
     *  Render pointlights1!!!!:D
     */
    void COffscreenLightRenderer::Render( const COffscreenMultiPointLight &pLights )
    {
        uint32_t i;
        
        pointLightShader->SetUniform( m_uLightPasses[LightUniforms::LightSize], pLights.pLight->GetLightSize() );
        
        for( i = 0; i < m_iSceneMultiLights; ++i )
        {
            // Bind vertex array.
            glBindVertexArray( pLights.pLight->rSphere->m_VAO );
            
            pointLightShader->SetUniform( m_uLightPasses[LightUniforms::Origin], pLights.objectCenters[i].x, pLights.objectCenters[i].y, pLights.objectCenters[i].z );
            
            pointLightShader->SetUniform( m_uLightPasses[LightUniforms::ObjectMatrix], 1, GL_FALSE, pLights.objectMatrices[i].m );
            
            // Draw light geometry.
            glDrawElements( GL_TRIANGLES, pLights.pLight->rSphere->m_iFaceCount , GL_UNSIGNED_INT, (void*)0 );
            
            glBindVertexArray( 0 );
        }
    }
    
    /**
     *  Render pointlight!!11! :DD 
     */
    void COffscreenLightRenderer::Render( const COffscreenPointlight *pLights )
    {
        uint32_t    i;
        
        for( i = 0; i < m_iSceneLights; ++i )
        {
            glBindVertexArray( pLights[i].rSphere->m_VAO );
            
            //! Pass shader uniforms.
            pointLightShader->SetUniform( m_uLightPasses[LightUniforms::ObjectMatrix], 1, false, pLights[i].rSphere->objectMatrix.m );
            pointLightShader->SetUniform( m_uLightPasses[LightUniforms::Origin], pLights[i].rSphere->_position.x, pLights[i].rSphere->_position.y, pLights[i].rSphere->_position.z );
            pointLightShader->SetUniform( m_uLightPasses[LightUniforms::LightColor], pLights[i].m_vColor.x, pLights[i].m_vColor.y, pLights[i].m_vColor.z );
            pointLightShader->SetUniform( m_uLightPasses[LightUniforms::LightSize], pLights[i].GetLightSize() );
            
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, pLights[i].rSphere->m_vIndexArray.Handle );
            
            //! Draw light geometry.
            glDrawElements( GL_TRIANGLES, pLights[i].rSphere->m_iFaceCount, GL_UNSIGNED_INT, (void*)0 );
            
            glBindVertexArray( 0 );
        }
    }
    
#endif // NEKO_SERVER
    // Grass is moved to it's class - ncGrass.
    
    CBeautifulEnvironment local_benv;
    CBeautifulEnvironment *g_pbEnv = &local_benv;
}
