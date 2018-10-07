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
//  ParticleEngine.h
//  Particle engine. ;*
//
//  Created by Neko Code on 1/10/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__CParticleEngine__
#define __Nanocat__CParticleEngine__

#include "../Core/Core.h"
#include "../Platform/Shared/SystemShared.h"
#include "../Math/GameMath.h"
#include "../Graphics/OpenGL/GLShader.h"
#include "../AssetCommon/AssetBase.h"
#include "../AssetCommon/Sound/SoundManager.h"
#include "../Core/Player/Camera/Camera.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Core/Utilities/List.h"

#ifndef NEKO_SERVER
namespace Neko {

    //!  Max scene particle emitters.
    static const uint32_t MAX_PARTICLES = 128;
    
    //!  Max scene GPU particles ( not emitters! ).
    static const uint32_t MAX_PARTICLES_ON_SCENE = 128000;
    
    //!  Particle object attributes.
    static const uint32_t MAX_PARTICLE_ATTRS = 6;
    
    
    /// GPU particle emitter type.
    enum class GPUParticleType : int32_t
    {
        Generator = 0,
        Normal
    };
    
    ///  Particle piece.
    struct SParticle
    {
        Vec3   m_vPosition;        //! Particle origin.
        Vec3   m_vVelocity;        //! Particle velocity.
        Vec3   m_vColor;           //! Particle color.
        
        float   m_fLifeTime;        //! Life time in seconds.
        float   m_fSize;            //! Size.
        
        GPUParticleType     m_iType;        //! Generation type.
    };
    
    ///  Particle emitter!
    class SGFXParticle
    {
    public:
        
        /**
         *  Initialize.
         */
        void                    Init();
        
        /**
         *  Render particles.
         */
        void                    RenderParticles();
        
        /**
         *  Update particle.
         *
         *  @param timePassed   Renderer frame time.
         */
        void                    UpdateParticles( float timePassed );
        
        /**
         *  Set properties.
         */
        void                    SetProperties( const Vec3 & a_vGenPosition, const Vec3 & a_vGenVelocityMin, const Vec3 & a_vGenVelocityMax, const Vec3 & a_vGenGravityVector, const Vec3 & a_vGenColor, const float &a_fGenLifeMin, const float & a_fGenLifeMax, const float & a_fGenSize, const float & fEvery, const int a_iNumToGenerate);
        
        /**
         *  Clear all particles.
         */
        void                    ClearAllParticles();
        
        /**
         *  Remove this particle.
         */
        void                    Delete();
        
        /**
         *  Get particle count.
         */
        inline const uint32_t               GetNumParticles() const   {       return m_particlesNum;  }
        
        /**
         * -------------------------------------
         *      Set properties individually.
         * -------------------------------------
         */
        
        /**
         *  Set particle spawn origin.
         */
        inline void                 SetOrigin( const Vec3 & pos )  {       m_vPosition = pos;  }
        
        /**
         *  Set particle velocity.
         */
        inline void SetVelocity( const Vec3 & min, const Vec3 & range )
        {
            m_vVelocityMin = min;
            m_vVelocityRange = range;
        }
        
        /**
         *  Set particle gravity.
         */
        inline void                 SetGravity( const Vec3 & gravity ) {       m_vGravityVector = gravity; }
        
        /**
         *  Set particle color.
         */
        inline void                 SetColor( const Vec3 & color ) {       m_vColor = color;   }
        
        /**
         *  Set particle size.
         */
        inline void                 SetSize( const float & size )   {       m_fSize = size; }
        
        /**
         *  Set life time.
         */
        inline void SetLifeTime( const float & min, const float & range )
        {
            m_fLifeMin = min;
            m_fLifeRange = range;
        }
        
        /**
         *  Generation ( spawn ) time.
         */
        inline void SetGenTime( const float & time )
        {
            m_fNextGenerationTime = time;
            m_fElapsedTime = 0.8f;
        }
        
        /**
         *  Set particle amount.
         */
        inline void                 SetAmount( const int32_t count )    {       m_toSpawn = count;  }
        
        /**
         *  Set particle material.
         */
        inline void SetMaterial( const char * materialName )
        {
            m_pMaterial = f_AssetBase->p_MaterialBase->Find( materialName );
        }
        
        /**
         *  Set life speed multiplier.
         */
        inline void                 SetSpeed( const float & _speed )    {       m_fSpeed = _speed;  }
        
        
        /**
         *  Get alive particles.
         */
        inline const int32_t                GetAliveParticles() const  {       return m_particlesNum;  }
        
        /**
         *  Is particle visible.
         *
         *  @return A corresponding value.
         */
        inline bool                 IsVisible() const   {       return m_bVisible;  }
        
        /**
         *  Constructor.
         */
        SGFXParticle();
 
        //! Particle link.
        SLink   m_Link;
        
    private:
        
        //!  Can be rendered? ( initialized? )
        bool        m_bCanBeRendered;
        
        //!  Particle material.
        SMaterialProp    * m_pMaterial;
        
        //!  Transform feedback buffer for particle update on GPU.
        uint32_t    m_transformFeedbackBuffer;
        
        //!  Particle vertex buffers.
        GPUBuffer   m_particleBuffer[2];
        
        //!  Particle vertex arrays.
        uint32_t    m_vertexArrays[2];
        
        //!  Query buffer. By this we are getting information about particles from GPU.
        uint32_t    m_queryId;
        
        //! Which vertex buffer it needs to read.
        int32_t     m_readBuffer;     // Can be negative!
        
        //! Total fx particles.
        int32_t     m_particlesNum;

        
    public:
        
        //! Elapsed time since particle was created.
        float   m_fElapsedTime;
        
        //! Wait till.
        float   m_fNextGenerationTime;
        
        //! Generation origin.
        Vec3   m_vPosition;
        
        //! Velocity.
        Vec3   m_vVelocityMin, m_vVelocityRange;
        
        //! Gravity vector.
        Vec3   m_vGravityVector;
        
        //! Particle color. TODO: random for EACH fx particle.
        Vec3   m_vColor;
        
        //! Life time.
        float   m_fLifeMin, m_fLifeRange;
        
        float   m_fSize;    //! Particle size.
        float   m_fSpeed;   //! Particle life speed.
        
        int32_t     m_toSpawn;
        
        //!  Is particle visible?
        bool        m_bVisible;
      
        //! Casts shadow?
        bool        m_bCastShadow;
        
        //! Recieves shadow?
        bool        m_bRecievesShadow;
    };
    
    
    ///   Particle engine.
    class CParticleEngine
    {
        NEKO_NONCOPYABLE( CParticleEngine );
        
    public:
        
        /**
         *  Default constructor.
         */
        CParticleEngine() : particleUpdate(NEKO_NULL), particleRender(NEKO_NULL)
        {
            
        }
        
        /**
         *  Destructor.
         */
        ~CParticleEngine()
        {
            
        }
        
        /**
         *  Initialize particle engine.
         */
        void                Init();
        
        /**
         *  Update billboards.
         */
        inline void SetMatrices( const Vec3 & vLook )
        {
            vQuad1.Cross( vLook, g_Core->p_Camera->vUp );
            vQuad1.Normalize();
            
            vQuad2.Cross( vLook, vQuad1 );
            vQuad2.Normalize();
        }
        
        /**
         *  Render particles.
         */
        void                    Render( float msec );
        
        /**
         *  Update particles.
         */
        void                Update( float timePassed, SGFXParticle * _particle );
        
        /**
         *  Destroy all particles.
         */
        void                DestroyAllParticles();
        
        /**
         *  Shut down particle engine.
         */
        void                Shutdown();
        
        /**
         *  Add particle.
         */
        void                AddParticle( const Vec3 & pos,
                                           const Vec3 & minVelocitiy,
                                           const Vec3 & maxVelocity,
                                           const Vec3 & gravity,
                                           const Vec3 & color,
                                           const float & minLifeTime,
                                           const float & maxLifeTime,
                                           const float & size,
                                           const float & period,
                                           const int toSpawn,
                                            const char * name );
        void                AddParticle( SGFXParticle * particle );
        
        
        /**
         *  Get active particle count.
         */
        inline const int32_t GetActiveParticleCount() const
        {
            return (const int32_t)m_particleEnts.m_iCount;
        }
        
    private:

        //!  Particle common uniforms.
        enum ParticleSystemUniforms
        {
            MVP = 0,
            QUAD0, QUAD1,
        };
        
        //!  Particle uniforms.
        enum ParticleUniforms
        {
            PPOSITION_TIME  = 0,      // Position & time.
            PVELOCITYMIN,           // Minimal velocity time.
            PVELOCITYRANGE,         // Velocity range.
            PCOLOR,                 // Particle color.
            PGRAVITY,               // Particle gravity vector.
            PLIFEMIN_RANGE_SIZE,       // Minimal life time, life range & particle size.
            PNUM,                   // Amount of particles.
            PSEED                   // Random vector seed.
        };
        
        //! Total particles.
        SList   m_particleEnts;
        
        //! GPU particle uniforms.
        GLuint m_particleUniforms[11];
        
        //! Global particle uniforms.
        GLuint m_particleCommonUniforms[3];
        
    public:
        //! Billboard quads.
        Vec3 vQuad1, vQuad2;
        
        //! Shaders.
        SGLShader * particleUpdate, * particleRender;
    };
}
#endif // NEKO_SERVER
#endif /* defined(__Nanocat__CParticleEngine__) */
