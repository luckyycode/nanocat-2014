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
//  ShadowRenderer.cpp
//  Shadow map renderers and technologies.
//
//  Created by Neko Code on 12/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "ShadowRenderer.h"
#include "../OpenGL/OpenGLBase.h" // Near, Far distances
#include "../../World/BeautifulEnvironment.h"
#include "../../Core/Player/Camera/Camera.h"
#include "../../World/Mesh.h"
#include "../../Math/Frustum.h"
#include "../../Core/Utilities/Utils.h"
#include "../../Core/Player/Camera/Camera.h"
#include "../../Math/BoundingBox.h"

///
///     Cascaded shadow map renderer.
///
// TODO: time interval update and caching.

namespace Neko {

    //!  Render shadows?
    SConsoleVar    * Shadow_Enabled = 0;
    //!  Amount of CSM splits.
    SConsoleVar     * Shadow_Splits = 0;
    //!  CSM split weight.
    SConsoleVar    * Shadow_Distance = 0;
    //!  Shadow map size.
    SConsoleVar     * Shadow_Size = 0;
    //!  Shadow map filtering.
    SConsoleVar     * Shadow_Filtering = 0;
    
    /**
     *  Calculate frustum bounding boxes.
     */
    void CCascadedShadow::frustumBoundingBoxLightViewSpace( const int32_t currentSplit, float nearPlane, float farPlane, Vec4 & min, Vec4 & max )
    {
        Vec4 frustumMin(FLT_MAX);
        Vec4 frustumMax(FLT_MIN);
        
        const float & m_fov = 45.0f;// GameView_FieldOfView->Get<float>();
        
        const float & m_width(Render_Width->Get<float>());
        const float & m_height(Render_Height->Get<float>());
        
        const float nearHeight = 2.0f * tan(m_fov * 0.5f) * nearPlane;
        const float nearWidth = nearHeight * m_width / m_height;
        const float farHeight = 2.0f * tan(m_fov * 0.5f) * farPlane;
        const float farWidth = farHeight * m_width / m_height;
        const Vec4 cameraPos = Vec4(g_Core->p_Camera->vEye, 1.0f);

        const Vec4 viewDir = Vec4(0.0f, 0.0f, -1.0f, 0.0f);
        const Vec4 upDir = Vec4(0.0f, 1.0f, 0.0f, 0.0f);
        const Vec4 rightDir = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
        const Vec4 nc = cameraPos + viewDir * nearPlane; // near center
        const Vec4 fc = cameraPos + viewDir * farPlane; // far center
        
        // Vertices in a world space.
        Vec4 vertices[8] = {
            nc - upDir * nearHeight * 0.5f - rightDir * nearWidth * 0.5f, // nbl (near, bottom, left)
            nc - upDir * nearHeight * 0.5f + rightDir * nearWidth * 0.5f, // nbr
            nc + upDir * nearHeight * 0.5f + rightDir * nearWidth * 0.5f, // ntr
            nc + upDir * nearHeight * 0.5f - rightDir * nearWidth * 0.5f, // ntl
            fc - upDir * farHeight  * 0.5f - rightDir * farWidth * 0.5f, // fbl (far, bottom, left)
            fc - upDir * farHeight  * 0.5f + rightDir * farWidth * 0.5f, // fbr
            fc + upDir * farHeight  * 0.5f + rightDir * farWidth * 0.5f, // ftr
            fc + upDir * farHeight  * 0.5f - rightDir * farWidth * 0.5f, // ftl
        };
        
        for( uint32_t vertId = 0; vertId < 8; ++vertId ) {
//            m_frustumSplits[currentSplit].m_vPlane[vertId] = vertices[vertId];
            
            // Light view space.
            vertices[vertId] = m_lightViewMatrix * vertices[vertId];
            // Update bounding box.
            frustumMin.x = nkMath::Min(frustumMin.x, vertices[vertId].x);
            frustumMin.y = nkMath::Min(frustumMin.y, vertices[vertId].y);
            frustumMin.z = nkMath::Min(frustumMin.z, vertices[vertId].z);
            frustumMin.w = nkMath::Min(frustumMin.w, vertices[vertId].w);
            
            frustumMax.x = nkMath::Max(frustumMax.x, vertices[vertId].x);
            frustumMax.y = nkMath::Max(frustumMax.y, vertices[vertId].y);
            frustumMax.z = nkMath::Max(frustumMax.z, vertices[vertId].z);
            frustumMax.w = nkMath::Max(frustumMax.w, vertices[vertId].w);
        }
        
        min = frustumMin;
        max = frustumMax;
    }

    /**
     *  Initialize CSM renderer.
     *
     *  @note   Must be called after renderer initialisation.
     */
    void CCascadedShadow::Init( INekoAllocator * allocator )
    {
//        // Create orthogonal projection for a directional shadow.
//        m_pShadowMapLayers[0]->LightProjectionMatrix.CreateOrtho( -75, 75, -75, 75, 0.01f, 10000.0f );
        
        m_fGlobalOpacity = 1.0f;  // changes, depends on weather type
   
        m_nearPlane = 0.1f;
        m_farPlane= 1150.0f;
        m_frustumSplitCorrection = 0.99625f;
        
        m_fLightRadiusWorld = 0.005f; // sun can be a very small disk area value
        
        m_eShadowFilter = ETextureFormat::Depth32F;
        
        pAllocator= allocator;
        
        // Register cvars.
        Shadow_Enabled = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "bShadows", "Turn on shadow rendering?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        Shadow_Splits = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "iShadowSplits", "Amount of cascaded shadow map splits", 4, CVFlag::NeedsRefresh, ECvarType::Int );
        Shadow_Distance = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "fSplitDist", "Shadow map split weight", 0.85f, CVFlag::NeedsRefresh, ECvarType::Float );
        Shadow_Size = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "iShadowSize", "Shadow map size", 2048, CVFlag::NeedsRefresh, ECvarType::Int );
        Shadow_Filtering = g_Core->p_Console->RegisterCVar( ECvarGroup::Display,  "sShadowMapFiltering", "Shadow map filtering", 0 /* PCSS */, CVFlag::NeedsRefresh, ECvarType::Int );
        
        g_Core->p_Console->LoadIni( "neko" );
        
        m_iShadowSplits = Shadow_Splits->Get<int32_t>();
        
        
        m_iShadowSizeX = Shadow_Size->Get<int32_t>();
        m_iShadowSizeY = Shadow_Size->Get<int32_t>();
        
        
        // Set postprocess shader uniforms.
        SGLShader * sceneShader = g_mainRenderer->GetSceneShader();
        sceneShader->Use();
        
        // Prepare shader uniforms.
        m_iCSMUniforms[CSMUniforms::FarDistanceVec] = sceneShader->UniformLocation( "far_d" );
        
        // Light view projection matrices.
        m_iCSMUniforms[CSMUniforms::Matrix0] = sceneShader->UniformLocation( "g_texMatrix[0]" );
        m_iCSMUniforms[CSMUniforms::Matrix1] = sceneShader->UniformLocation( "g_texMatrix[1]" );
        m_iCSMUniforms[CSMUniforms::Matrix2] = sceneShader->UniformLocation( "g_texMatrix[2]" );
        m_iCSMUniforms[CSMUniforms::Matrix3] = sceneShader->UniformLocation( "g_texMatrix[3]" );
        // Light view matrices.
        m_iCSMUniforms[CSMUniforms::NormalMat0] = sceneShader->UniformLocation( "g_texMatrix[4]" );
        m_iCSMUniforms[CSMUniforms::NormalMat1] = sceneShader->UniformLocation( "g_texMatrix[5]" );
        m_iCSMUniforms[CSMUniforms::NormalMat2] = sceneShader->UniformLocation( "g_texMatrix[6]" );
        m_iCSMUniforms[CSMUniforms::NormalMat3] = sceneShader->UniformLocation( "g_texMatrix[7]" );
        
        m_iCSMUniforms[CSMUniforms::LightRadiusUV] = sceneShader->UniformLocation("g_lightRadiusUV");
        
        // Light settings.
        sceneShader->SetUniform( "g_lightZNear", m_nearPlane );
        sceneShader->SetUniform( "g_lightZFar", m_farPlane );
        
        sceneShader->Next();
        
        // Shadow map framebuffers.
        m_pDepthBuffer = (CFramebuffer *)pAllocator->Alloc( sizeof(CFramebuffer) );
        m_pDepthBuffer->Create( m_iShadowSizeX, m_iShadowSizeY, true, false /* only depth, no colors */ );
        
        // Create shadow map.
        m_pShadowMap = f_AssetBase->p_MaterialBase->CreateMaterial( NEKO_NULL, m_iShadowSizeX, m_iShadowSizeY, TextureTarget::Texture2DArray, ETextureStorageType::Float, m_eShadowFilter, ETextureFormat::Depth, ETextureTile::ClampToEdge, ETextureFilter::Nearest, 32, false, MAX_SPLITS, -2.4f );
        
        // PCF.
        // NOTE: PCF only.
        m_pShadowMap->CompareRefToTexture( true, ECompareMode::Lequal );
        m_pShadowMap->SetAnisotropy( 16 );
        
        glGetFloatv( GL_MAX_VIEWPORT_DIMS, m_viewportDims );
    }

    /**
     *  Constructor.
     */
    CCascadedShadow::CCascadedShadow() {
        
    }

    /**
     *  Destructor.
     */
    CCascadedShadow::~CCascadedShadow() {
        
    }
    
    /**
     *  Render shadow as post process.
     *
     *  @param shader Postprocessing shader.
     *  @note           Called in renderer while the main g-buffer post process shader is in use.
     */
    void CCascadedShadow::UpdatePostProcess( SGLShader * shader )
    {
        const static ncMatrix4 bias( 0.5f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 0.5f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 0.5f, 0.0f,
                                    0.5f, 0.5f, 0.5f, 1.0f );
        
        // Use shadow map array.
        g_mainRenderer->BindTexture( 5, m_pShadowMap->Image.GetId(), TextureTarget::Texture2DArray );

        for( uint32_t i = 1; i <= m_iShadowSplits; ++i )
        {
            const float distFactor = static_cast<float>(i) / m_iShadowSplits;
            const float stdTerm = m_nearPlane * powf(m_farPlane / m_nearPlane, distFactor);
            const float corrTerm = m_nearPlane + distFactor * (m_farPlane - m_nearPlane);
            
            // UpdateSplitDist
            const float viewDepth = m_frustumSplitCorrection * stdTerm + (1.0f - m_frustumSplitCorrection) * corrTerm;
            m_farPlanes[i - 1] = viewDepth;
            
            const Vec4 projectedDepth = g_Core->p_Camera->ProjectionMatrix * Vec4(0.0f, 0.0f, - viewDepth, 1.0f);
            // Normalized to [0, 1] depth range.
            m_normalizedFarPlanes[i - 1] = (projectedDepth.z / projectedDepth.w) * 0.5f + 0.5f;
            
            m_mShadowViewMatrices[i - 1] = bias * m_mShadowViewMatrices[i - 1];
            
            // Upload matrices.
            shader->SetUniform( m_iCSMUniforms[CSMUniforms::Matrix0 + i - 1], 1, false, m_mShadowViewMatrices[i - 1].m );
            shader->SetUniform( m_iCSMUniforms[CSMUniforms::NormalMat0 + i - 1], 1, false, m_mLightViewMatrices[i - 1].m );
        }
        
        shader->SetUniform( m_iCSMUniforms[CSMUniforms::LightRadiusUV], m_fLightRadiusWorld / m_vFrustumSize.x, m_fLightRadiusWorld / m_vFrustumSize.y );
        // Upload far bound values to GPU shader.
        shader->SetUniform( m_iCSMUniforms[CSMUniforms::FarDistanceVec],  m_normalizedFarPlanes[0], m_normalizedFarPlanes[1], m_normalizedFarPlanes[2], 1.0f  );
    }
    
    /**
     *      Shadow renderer.
     */
    void CCascadedShadow::RenderShadow()
    {
        // Wait for world until it will be created.
        if( !(g_pbEnv->m_iEnvFlags & CreatedEnvironment) ) {
            return;
        }
        
        int16_t     iFlags;  // World flags.
        
        ncMatrix4   mPrevViewProj; // Use this to restore previous viewprojection matrix.
        ncMatrix4   lightProj;
        
        Vec4   frustumMin( FLT_MIN );
        Vec4   frustumMax( FLT_MAX );
        
        float nearSegmentPlane = 0.0f;
        
        // Shadows are depth values.
        g_mainRenderer->SetDepthTest(true);
        g_mainRenderer->SetDepthMode( ECompareMode::Lequal );

        // Save a previous world matrix.
        mPrevViewProj = g_Core->p_Camera->ViewProjectionMatrix;
        
        // Notice the renderer that we're drawing shadows.
        iFlags = (int32_t)EWorldFlag::Shadow;
        
        // Rendering to the depth texture.
        m_pDepthBuffer->BindForDrawing();
        // Shadow map size.
        g_mainRenderer->SetViewportSize( m_iShadowSizeX, m_iShadowSizeY );
        g_mainRenderer->SetRenderTarget( ERenderTarget::None );
        
        // Eliminate artifacts caused by shadow mapping.
        // This turns of the polygon offset functionality to fix artifacts.
        glEnable( GL_POLYGON_OFFSET_FILL );
        glPolygonOffset( 8.0f, 4.0f );
        
        // Clock-wise mode.
        g_mainRenderer->SetCulling( true, ECullMode::Front );
        
        const Vec3 & DirL = g_pbEnv->m_vSunDirection;
        m_lightViewMatrix = ncMatrix4::LookAt( DirL + g_Core->p_Camera->vEye, g_Core->p_Camera->vEye, Vec3(-1.0f, 0.0f, 0.0f) );
        m_lightViewMatrix2 = ncMatrix4::LookAt( Vec3(0.0f), Vec3::Normalize(Vec3(-DirL.x, -DirL.y, -DirL.z)) , Vec3(-1.0f, 0.0f, 0.0f) );
        // Find a bounding box of whole camera frustum in light view space.
        frustumBoundingBoxLightViewSpace( 0, m_nearPlane, m_farPlane, frustumMin, frustumMax );
        

        // Update light projection matrix to only cover the area viewable by the camera
        m_lightProjMatrix.CreateOrtho3D( frustumMin.x, frustumMax.x, frustumMin.y, frustumMax.y, 0.0f, frustumMin.z);
        
        // Update viewports.
        m_vFrustumSize = Vec2(frustumMax.x - frustumMin.x, frustumMax.y - frustumMin.y);

        // Find a bounding box of segment in light view space.
        for( uint32_t i( 0 ); i < m_iShadowSplits; ++i ) {
            Vec4 segmentMin(FLT_MAX);
            Vec4 segmentMax(FLT_MAX);
            
            frustumBoundingBoxLightViewSpace( i, nearSegmentPlane, m_farPlanes[i], segmentMin, segmentMax);

            const float segmentSizeX = segmentMax.x - segmentMin.x;
            const float segmentSizeY = segmentMax.y - segmentMin.y;
            const float segmentSize = segmentSizeX < segmentSizeY ? segmentSizeY : segmentSizeX;
            
            const Vec2 offsetBottomLeft(segmentMin.x - frustumMin.x, segmentMin.y - frustumMin.y);
            const Vec2 offsetSegmentSizeRatio(offsetBottomLeft.x / segmentSize, offsetBottomLeft.y / segmentSize);
            const Vec2 frustumSegmentSizeRatio(m_vFrustumSize.x / segmentSize, m_vFrustumSize.y / segmentSize);
            
            m_bBoundingBox[i] = SBoundingBox( segmentMin.xyz(), segmentMax.xyz() );
            
            Vec2 pixelOffsetTopLeft(offsetSegmentSizeRatio * m_iShadowSizeX);
            Vec2 pixelFrustumSize(frustumSegmentSizeRatio * m_iShadowSizeY);
            
            // Scale factor that helps if frustum size is supposed to be bigger
            // than maximum viewport size.
            Vec2 scaleFactor(  m_viewportDims[0] < pixelFrustumSize.x ? m_viewportDims[0] / pixelFrustumSize.x : 1.0f,
                                  m_viewportDims[1] < pixelFrustumSize.y ? m_viewportDims[1] / pixelFrustumSize.y : 1.0f);
            
            pixelOffsetTopLeft = pixelOffsetTopLeft * scaleFactor;
            pixelFrustumSize = pixelFrustumSize * scaleFactor;
            
            m_lightViewports[i] = Vec4(-pixelOffsetTopLeft.x, -pixelOffsetTopLeft.y, pixelFrustumSize.x, pixelFrustumSize.y);
            
            float w[] = {m_lightViewports[i].x, m_lightViewports[i].y, m_lightViewports[i].z, m_lightViewports[i].w};
//            glViewportIndexedfv(i, w);
            
            // Update light view-projection matrices per segment.
            lightProj.CreateOrtho3D( segmentMin.x, segmentMin.x + segmentSize, segmentMin.y, segmentMin.y + segmentSize, 0.0f, frustumMin.z);
            
            // Clip to texture coordinates.
            ncMatrix4 lightScale;
            lightScale.Identity();
            lightScale.Scale(Vec3(0.5f * scaleFactor.x, 0.5f * scaleFactor.y, 0.5f));
            ncMatrix4 lightBias;
            lightBias.Identity();
            lightBias.Translate(Vec3(0.5f * scaleFactor.x, 0.5f * scaleFactor.y, 0.5f));
            
            // Final light matrix.
            m_lightSegmentVPSBMatrices[i] = lightBias * lightScale * lightProj * m_lightViewMatrix;
            
            // Shadow Map Flickering
            Vec4   ptOriginShadow( 0.0f, 0.0f, 0.0f, 1.0f );
            ptOriginShadow.TransformVector(  m_lightSegmentVPSBMatrices[i] );
            
            // Find a texel we need. 0.5f, x,y is in -1..1 range
            // but we need 0..1 range
            float texCoordX = ptOriginShadow.x * (float)m_iShadowSizeX * 0.5f;
            float texCoordY = ptOriginShadow.y * (float)m_iShadowSizeY * 0.5f;
            
            // Round texels.
            float texCoordRoundedX = roundf( texCoordX );
            float texCoordRoundedY = roundf( texCoordY );
            // Find a difference between original and rounded texels to prevent sub-texel movement.
            float dx = texCoordRoundedX - texCoordX;
            float dy = texCoordRoundedY - texCoordY;
            
            // Make dx, dy into homogeneous space.
            dx /= (float)m_iShadowSizeX * 0.5f;
            dy /= (float)m_iShadowSizeY * 0.5f;
            
            ncMatrix4 vRounding;
            vRounding.Identity();
            vRounding.Translate( Vec3( dx, dy, 0.0f ) );
            
            m_lightSegmentVPSBMatrices[i] =  m_lightSegmentVPSBMatrices[i] * vRounding;
            
            nearSegmentPlane = m_normalizedFarPlanes[i];
            
            // Make the current depth map a rendering target.
            glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_pShadowMap->Image.GetId(), 0, i );
            
            // Clear render target.
            g_mainRenderer->ClearDepth();
            
            // Set the main camera viewprojection matrix to the sun matrix.
            m_mShadowViewMatrices[i] = m_lightSegmentVPSBMatrices[i] * vRounding;
            
//            float dafell[16];
//            NekoUtils::gluInvertMatrix( m_lightViewMatrix.m, dafell );
            
            m_mLightViewMatrices[i] = m_lightViewMatrix2;
            
            g_Core->p_Camera->ViewProjectionMatrix = m_mShadowViewMatrices[i];
            
            // Render the world.
            g_pbEnv->RenderTerrain( CRenderer::ESceneEye::EYE_FULL, iFlags );
            if( g_pbEnv->m_pIndoorScene != NEKO_NULL ){
                g_pbEnv->m_pIndoorScene->Render();
            }
        }
        
        
//        // Set remaining viewports to some kind of standard state.
//        for (unsigned int i = m_iShadowSplits; i < MAX_SPLITS; ++i) {
//            glViewportIndexedf(i, 0, 0, m_iShadowSizeX, m_iShadowSizeY);
//        }
        
        g_mainRenderer->SetCulling( true, ECullMode::Back );
        g_mainRenderer->SetDepthMode( ECompareMode::Less );
//        glFrontFace( GL_CCW );
        // Restore previous states & modes.
//        g_mainRenderer->SetCulling( true, ECullMode::Back );
        
        glDisable( GL_POLYGON_OFFSET_FILL );

        m_pDepthBuffer->UnbindDrawing();
        
        // Restore matrix.
        g_Core->p_Camera->ViewProjectionMatrix = mPrevViewProj;
        
//        g_mainRenderer->SetDepthMode( ECompareMode::Less  );
        g_mainRenderer->SetDepthTest(false);
        

    }
    
    /**
     *  Destroy the shadow renderer.
     */
    void CCascadedShadow::Destroy()
    {
        m_pDepthBuffer->Delete();
        pAllocator->Dealloc( m_pDepthBuffer );
        f_AssetBase->p_MaterialBase->UnloadMaterial( m_pShadowMap );
        
        // TODO: material destroy
        // 'm_pShadowMap'
    }
 
}
