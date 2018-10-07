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
//  ParticleEngine.cpp
//  Particle engine. ;*
//
//  Created by Neko Code on 1/10/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//


#include "../Core/Core.h"

#include "../Core/Player/Camera/Camera.h"           // Camera view/origin.
#include "ParticleEngine.h"
#include "../AssetCommon/AssetBase.h"               // Material loading.
#include "../Graphics/Renderer/Renderer.h"          // Scene framebuffers.

#if !defined( NEKO_SERVER )

// GPU particle engine.

/*
 if( _canBounce ) {
 for( int ik = 0; ik < planesAdded; ++ik )
 {
 ncBasicPlaneStruct &plane = _planes[ik];
 
 // Computes distance to the plane
 float d = Vec3::Dot(plane.n, p.p) + plane.d;
 if( d < 0.0f ) {
 // Corrects any plane penetration.
 p.p = p.p + -d * plane.n;
 
 p.v = _bouncePower * p.v.Reflect( plane.n );
 }
 }
 }*/

namespace Neko {
    
    /**
     *  Init particle engine.
     */
    void CParticleEngine::Init()
    {
        // Linker for particles to be used on scene.
        SList::CreateList( &m_particleEnts );
   
        // Updating program
        particleUpdate = f_AssetBase->FindAssetByName<SGLShader>( "particleupdate" );
        particleUpdate->Use();
        
        // Set shader uniforms.
        m_particleUniforms[PPOSITION_TIME] = particleUpdate->UniformLocation( "vGenPositionAndTime" );
        m_particleUniforms[PVELOCITYMIN] = particleUpdate->UniformLocation( "vGenVelocityMin" );
        m_particleUniforms[PVELOCITYRANGE] = particleUpdate->UniformLocation( "vGenVelocityRange" );
        m_particleUniforms[PCOLOR] = particleUpdate->UniformLocation( "vGenColor" );
        m_particleUniforms[PGRAVITY] = particleUpdate->UniformLocation( "vGenGravityVector" );
        m_particleUniforms[PLIFEMIN_RANGE_SIZE] = particleUpdate->UniformLocation( "fGenLifeMinRangeAndSize" );
        m_particleUniforms[PNUM] = particleUpdate->UniformLocation( "iNumToGenerate" );
        m_particleUniforms[PSEED] = particleUpdate->UniformLocation( "vRandomSeed" );
        
        particleUpdate->Next();
        
        // Rendering program
        particleRender = f_AssetBase->FindAssetByName<SGLShader>( "particlerender" );
        particleRender->Use();
        
        // Shader uniforms.
        m_particleCommonUniforms[MVP] = particleRender->UniformLocation( "MVP" );
        m_particleCommonUniforms[QUAD0] = particleRender->UniformLocation( "vQuad1" ); // Quad used for billboarding.
        m_particleCommonUniforms[QUAD1] = particleRender->UniformLocation( "vQuad2" );
        
        // Samplers.
        particleRender->SetUniform( "gSampler", 0 );
        particleRender->SetUniform( "gDepth", 1 );
        
        particleRender->Next();
        
        // Notify us.
        g_Core->p_Console->Print( LOG_INFO, "Particle engine initialized.\n" );
    }

    /**
     *  Add a particle emitter.
     *
     *  @param particle GPU particle.
     */
    void CParticleEngine::AddParticle( SGFXParticle * particle )
    {
        if( particle == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_WARN, "AddParticleEmitter: particle = NEKO_NULL.\n" );
            return;
        }
        
        SList::AddHead( &m_particleEnts, &particle->m_Link, particle );
    }

    /**
     *  Update particle physics on GPU.
     */
    void CParticleEngine::Update( float msec, SGFXParticle * _particle )
    {
        float fTimePassed = msec * _particle->m_fSpeed;
        Vec3 vRandomSeed;
        
        //! Particle visibility.
        // @FIXME: particle size ( physical ).
        if( g_Core->p_Camera->m_CommonFrustum.ContainsObject( _particle->m_vPosition, 32.0 ) ) {
            _particle->m_bVisible = true;
        } else {
            _particle->m_bVisible = false;
        }
        
        // Use particle update shader.
        particleUpdate->SetUniform( m_particleUniforms[PPOSITION_TIME], _particle->m_vPosition.x, _particle->m_vPosition.y, _particle->m_vPosition.z, fTimePassed );
        particleUpdate->SetUniform( m_particleUniforms[PVELOCITYMIN], _particle->m_vVelocityMin );
        particleUpdate->SetUniform( m_particleUniforms[PVELOCITYRANGE], _particle->m_vVelocityRange );
        particleUpdate->SetUniform( m_particleUniforms[PCOLOR], _particle->m_vColor );
        particleUpdate->SetUniform( m_particleUniforms[PGRAVITY], _particle->m_vGravityVector );
        
        particleUpdate->SetUniform( m_particleUniforms[PLIFEMIN_RANGE_SIZE], _particle->m_fLifeMin, _particle->m_fLifeRange, _particle->m_fSize );
        particleUpdate->SetUniform( m_particleUniforms[PNUM], 0 );
        
        // Increase life time.
        _particle->m_fElapsedTime += fTimePassed;
        
        // Generate particles if particle timout'd before.
        if( _particle->m_fElapsedTime > _particle->m_fNextGenerationTime ) {
            // Yet to spawn..
            particleUpdate->SetUniform( m_particleUniforms[PNUM], _particle->m_toSpawn );
            _particle->m_fElapsedTime -= _particle->m_fNextGenerationTime;
            
            vRandomSeed = Vec3( nkMath::grandf(-10.0f, 20.0f),
                                                      nkMath::grandf(-10.0f, 20.0f),
                                                      nkMath::grandf(-10.0f, 20.0f) );
            
            particleUpdate->SetUniform( m_particleUniforms[PSEED], vRandomSeed );
        }
        
        _particle->UpdateParticles( fTimePassed );

    }
    
    /**
     *  Render particles.
     */
    void CParticleEngine::Render( float msec )
    {
        SLink   * head;
        SLink   * cur;
        
        SGFXParticle * particle;
        
        glEnable( GL_RASTERIZER_DISCARD ); // Disable drawing.
        
        // Enable particle update shader ( no drawing ).
        particleUpdate->Use();

        head = &m_particleEnts.m_sList;
        
        // Update particles on a GPU.
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            particle = (SGFXParticle *) cur->m_ptrData;
            
            Update( msec, particle );
        }
        
        particleUpdate->Next();

   
        // Enable drawing.
        glDisable( GL_RASTERIZER_DISCARD );

        //! Use renderer shader.
        particleRender->Use();

        particleRender->SetUniform( m_particleCommonUniforms[MVP], 1, GL_FALSE, g_Core->p_Camera->m_CommonFrustum.GetModelView().m );
        particleRender->SetUniform( m_particleCommonUniforms[QUAD0], vQuad1.x, vQuad1.y, vQuad1.z );
        particleRender->SetUniform( m_particleCommonUniforms[QUAD1], vQuad2.x, vQuad2.y, vQuad2.z );

        // Render particles.
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            particle = (SGFXParticle *) cur->m_ptrData;
            
            particle->RenderParticles();
        }
        
        particleRender->Next();
    }
    
    /**
     *  Destroy all particles.
     */
    void CParticleEngine::DestroyAllParticles()
    {
        SLink * head;
        SLink * cur;
        
        SGFXParticle * particle;
        
        head = &m_particleEnts.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            particle = (SGFXParticle *) cur->m_ptrData;
            
            particle->Delete();
            
            delete particle;
        }
        
        // Recreate the list.
        SList::CreateList( &m_particleEnts );
    }
    
    /**
     *  Shut down.
     */
    void CParticleEngine::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "ParticleEngine: unitializing..\n" );
        
        DestroyAllParticles();
    }
    
    /**
     *  Constructor.
     */
    SGFXParticle::SGFXParticle()
    {

    }

    /**
     *  Init GPU particle.
     */
    void SGFXParticle::Init()
    {
        // Default values.
        m_bCanBeRendered = false;
        m_readBuffer = 0;
        m_fElapsedTime = 0.8f;
        
        // Particle system uses transform feedback and queries to 'query' results
        // back to engine core from the geometry shader.
        glGenTransformFeedbacks( 1, &m_transformFeedbackBuffer );
        glGenQueries( 1, &m_queryId );
        
        // Generate vertex arrays.
        glGenVertexArrays( 2, m_vertexArrays );
        
        // Particle initialization time.
        SParticle partInitialization;
        partInitialization.m_iType = GPUParticleType::Generator;
        
        // Generate vertex arrays and create particle buffers.
        /********************************************
                    (Particle elements)
                    _______________________
                    | 0 | 1 | 2 | 3 | etc..
                    ------------|----------
                                |
                    Particle buffer object
        *********************************************/
        
        int32_t     k, i;
        for( i = 0; i < 2; ++i ) {
            // Bind vertex array.
            glBindVertexArray( m_vertexArrays[i] );

            // Create GPU buffer.
            m_particleBuffer[i] = g_mainRenderer->AllocGPUBuffer( sizeof(SParticle) * MAX_PARTICLES_ON_SCENE, EBufferStorageType::Array, EBufferType::Static );
            g_mainRenderer->BufferData( &m_particleBuffer[i], &partInitialization, 0, sizeof(SParticle) );
            g_mainRenderer->FinishBuffer( &m_particleBuffer[i], 0, MAX_PARTICLES_ON_SCENE );
            
            g_mainRenderer->BindBuffer( &m_particleBuffer[i] );
            
            for( k = 0; k < MAX_PARTICLE_ATTRS; ++k ) {
                glEnableVertexAttribArray( k );
            }
            
            // vec3 ( 3 * 4 )
            glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(SParticle), (const GLvoid*)0 );    // Position
            glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(SParticle), (const GLvoid*)12 );   // Velocity
            glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(SParticle), (const GLvoid*)24 );   // Color
            // float ( 4 )
            glVertexAttribPointer( 3, 1, GL_FLOAT, GL_FALSE, sizeof(SParticle), (const GLvoid*)36 );   // Lifetime
            glVertexAttribPointer( 4, 1, GL_FLOAT, GL_FALSE, sizeof(SParticle), (const GLvoid*)40 );   // Size
            // int ( 4 )
            glVertexAttribPointer( 5, 1, GL_INT, GL_FALSE, sizeof(SParticle), (const GLvoid*)44 );     // Type
            
            glBindVertexArray( 0 );
        }
        
        // Initial values.
        m_readBuffer = 0;
        m_particlesNum = 1;
        m_bCanBeRendered = true;
        
        // Particle material.
        m_pMaterial = f_AssetBase->p_MaterialBase->Find( "doge" );
        
        m_bVisible = false;
        
//        return true; // It's all okay!
    }
    
    /**
     *     Particle updating on GPU.
     */
    void SGFXParticle::UpdateParticles( float timePassed )
    {
#   if defined( USES_OPENGL )
        // Bind transform feedback.
        glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, m_transformFeedbackBuffer );
        
        // Bind vertex array of currently read buffer.
        glBindVertexArray( m_vertexArrays[m_readBuffer] );
        glEnableVertexAttribArray( 1 ); // Re-enable velocity.
        
        glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[1 - m_readBuffer].Handle );
        
        // Get query information.
        glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_queryId );
        glBeginTransformFeedback( GL_POINTS );
        
        glDrawArrays( GL_POINTS, 0, m_particlesNum );
        
        glEndTransformFeedback();
        
        glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );
        glGetQueryObjectiv( m_queryId, GL_QUERY_RESULT, &m_particlesNum );
        
        glBindTransformFeedback( GL_TRANSFORM_FEEDBACK, 0 );
#   endif
        
        m_readBuffer = 1 - m_readBuffer;
    }
    
    /**
     *  Delete particle.
     */
    void SGFXParticle::Delete()
    {
#   if defined( USES_OPENGL )
        glDeleteQueries( 1, &m_queryId );
        glDeleteTransformFeedbacks( 1, &m_transformFeedbackBuffer );
        glDeleteVertexArrays( 2, m_vertexArrays );
#   else
        
#   endif
        
        g_mainRenderer->DeleteGPUBuffer( &m_particleBuffer[0] );
        g_mainRenderer->DeleteGPUBuffer( &m_particleBuffer[1] );
        
        m_bCanBeRendered = false;
    }

    /**
     *  Render particles on GPU.
     */
    void SGFXParticle::RenderParticles()
    {
        if( !m_bVisible ) {
            return;
        }
        
        // Bind particle texture.
        g_mainRenderer->BindTexture( 0, m_pMaterial->m_pDiffuse->GetId() );
        
        // Bind scene depth.
        g_mainRenderer->BindTexture( 1, g_mainRenderer->g_sceneBuffer[0]->GetUnitDepth() );
        
   
        
        // Vertex array.
        glBindVertexArray( m_vertexArrays[m_readBuffer] );
        glDisableVertexAttribArray( 1 ); // Disable velocity, because we don't need it for rendering
        
        glDrawArrays( GL_POINTS, 0, m_particlesNum );
        
        glBindVertexArray( 0 );
        
        g_mainRenderer->BindTexture( 0, 0 );
    }
    
    /**
     *  Set particle properties.
     */
    void SGFXParticle::SetProperties( const Vec3 & a_vGenPosition,
                                         const Vec3 & a_vGenVelocityMin,
                                         const Vec3 & a_vGenVelocityMax,
                                         const Vec3 & a_vGenGravityVector,
                                         const Vec3 & a_vGenColor,
                                         const float & a_fGenLifeMin,
                                         const float & a_fGenLifeMax,
                                         const float & a_fGenSize,
                                         const float & fEvery,
                                      const int32_t a_iNumToGenerate )
    {
        m_fSpeed = 0.5f;
        m_vPosition = a_vGenPosition;
        m_vVelocityMin = a_vGenVelocityMin;
        m_vVelocityRange = a_vGenVelocityMax - a_vGenVelocityMin;
        
        m_vGravityVector = a_vGenGravityVector;
        m_vColor = a_vGenColor;
        m_fSize = a_fGenSize;
        
        m_fLifeMin = a_fGenLifeMin;
        m_fLifeRange = a_fGenLifeMax - a_fGenLifeMin;
        
        m_fNextGenerationTime = fEvery;
        m_fElapsedTime = 0.8f;
        
        m_toSpawn = a_iNumToGenerate;
    }
    
    
    /**
     *   Add particle.
     */
    void CParticleEngine::AddParticle( const Vec3 & pos,
                                       const Vec3 & minVelocitiy,
                                       const Vec3 & maxVelocity,
                                       const Vec3 & gravity,
                                       const Vec3 & color,
                                       const float & minLifeTime,
                                       const float & maxLifeTime,
                                       const float & size,
                                       const float & period,
                                       const int toSpawn,
                                       const char * name  )
    {
        SGFXParticle * particle = new SGFXParticle;
        
        particle->Init();
        particle->SetProperties(
                                                   pos,            // Where the particles are generated.
                                                   minVelocitiy,   // Minimal velocity.
                                                   maxVelocity,    // Maximal velocity.
                                                   gravity,        // Gravity force applied to particles.
                                                   color,          // Color (light blue).
                                                   minLifeTime,    // Minimum lifetime in seconds.
                                                   maxLifeTime,    // Maximum lifetime in seconds.
                                                   size,           // Rendered size.
                                                   period,         // Spawn every 0.05 seconds.
                                                   toSpawn );      // And spawn 30 particles.
        particle->SetMaterial( name );
        
        SList::AddHead( &m_particleEnts, &particle->m_Link, particle );
        
        g_Core->p_Console->Print( LOG_INFO, "A new particle created at (%4.2f %4.2f %4.2f)\n", pos.x, pos.y, pos.z );
        
    }
    
}
#endif 