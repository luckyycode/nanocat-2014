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
//  Ocean.cpp
//  Beautiful ocean renderer
//
//  Created by Neko Vision on 2/8/14.
//  Copyright (c) 2013-2014 Neko Vision. All rights reserved.
//

#include "../AssetCommon/AssetBase.h"
#include "../AssetCommon/Sound/SoundManager.h"
#include "../Core/Core.h"
#include "../Core/Utilities/Utils.h"
#include "../Graphics/OpenGL/GLShader.h"
#include "../Graphics/OpenGL/OpenGLBase.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Math/BoundingBox.h"
#include "BeautifulEnvironment.h"
#include "Ocean.h"
#include "Quadtree/Quadtree.h"

// CPU generated and culled ocean.
// @todo GPU implementation

#if !defined( NEKO_SERVER )

namespace Neko {

    //!  Ocean wave gravity.
    static const int32_t OCEAN_WAVE_GRAVITY        = 10.41f;

    //!  UV coordinate scale.
    static const float OCEAN_TEXTURE_SCALE     = 4.0f;
    
    //!  Dispersion factor.
    static const float OCEAN_DISPERSION_FACTOR = 200.0f;
    
    SConsoleVar * Ocean_Waves = 0;

    /**
     *  Set water fog color.
     */
    void CWorldOcean::SetWaterColor( const float & _r, const float & _g, const float & _b, const float & _a )
    {
        m_waterColor.x = _r; // red
        m_waterColor.y = _g; // green
        m_waterColor.z = _b; // blue
        m_waterColor.w = _a; // hdr constant
    }
    
    /**
     *  Set stuff.
     */
    void CWorldOcean::SetParameters()
    {
        oShader = f_AssetBase->FindAssetByName<SGLShader>( "ocean" );
        oNormalMap = f_AssetBase->FindAssetByName<SMaterialProp>( "water1_n" );
        oSeaFoam = f_AssetBase->FindAssetByName<SMaterialProp>( "sea_foam" );
    }
    
    /**
     *  Create ocean surface.
     */
    void CWorldOcean::Create( SBoundingBox &boundingBox, const int32_t N, const float A, const Vec2 &w, const float length, bool hasAnimation, OceanGenType genType )
    {
        // Ocean properties.
        this->oGravity = OCEAN_WAVE_GRAVITY;
        
        // ( ・∀・)・・・--------☆
        
        this->N = N;
        this->Nplus1 = N + 1;
        this->N2 = this->Nplus1 * this->Nplus1;
        this->Nplus1N = this->Nplus1 * this->N;
        
        this->A = A;
        this->oWind = w;
        this->Length = length;

        this->h_tilde = NEKO_NULL;
        this->h_tilde_slopex = NEKO_NULL;
        this->h_tilde_slopez = NEKO_NULL;
        this->h_tilde_dx = NEKO_NULL;
        this->h_tilde_dz = NEKO_NULL;
        
        this->fft = NEKO_NULL;
        
        this->bHasAnimation = hasAnimation;
        this->oceanType = genType;

        bRenderOcean = false;
        
        
        g_Core->p_Console->Print( LOG_INFO, "Creating great ocean..\n" );
        
        m_pQuadtree     = NEKO_NULL; // Will be initialized later.
        oNormalMap      = NEKO_NULL;
        oShader         = NEKO_NULL;
        
        vertices        = (Vec3*)pAllocator->Alloc( sizeof(Vec3) * N2 );// new Vec3[N2];
        uvCoords        = (Vec2*)pAllocator->Alloc( sizeof(Vec2) * N2 );// new Vec2[N2];

        normals         = (Vec3*)pAllocator->Alloc( sizeof(Vec3) * N2 );// new Vec3[N2];
        
        // Create properties for CPU driven ocean.
        if( this->bHasAnimation && genType == CPU_OCEAN ) {
            h_tilde         = (ncFFTComplex*)pAllocator->Alloc( sizeof(ncFFTComplex) * N * N );// new ncFFTComplex[N * N];
            h_tilde_slopex  = (ncFFTComplex*)pAllocator->Alloc( sizeof(ncFFTComplex) * N * N );//new ncFFTComplex[N * N];
            h_tilde_slopez  = (ncFFTComplex*)pAllocator->Alloc( sizeof(ncFFTComplex) * N * N );// new ncFFTComplex[N * N];
            h_tilde_dx      = (ncFFTComplex*)pAllocator->Alloc( sizeof(ncFFTComplex) * N * N );// new ncFFTComplex[N * N];
            h_tilde_dz      = (ncFFTComplex*)pAllocator->Alloc( sizeof(ncFFTComplex) * N * N );// new ncFFTComplex[N * N];
            
            
            fft             = (ncFFT*)pAllocator->Alloc( sizeof(ncFFT) );//new ncFFT(N);
            fft->Create( N );
            
            hTilde0         = (Vec3*)pAllocator->Alloc( sizeof(Vec3) * N2 );// new Vec3[N2];
            hTilde0mk       = (Vec3*)pAllocator->Alloc( sizeof(Vec3) * N2 );// new Vec3[N2];
        }
        
        firstOrigin     = (Vec3*)pAllocator->Alloc( sizeof(Vec3) * N2 );// new Vec3[N2];
        
        uint32_t index = 0;
        int32_t     i;
        int32_t     j;
        
        m_BoundingBox = boundingBox;
        
        if( genType == CPU_OCEAN && this->bHasAnimation ) {
            g_Core->p_Console->Print( LOG_INFO, "Ocean will generate CPU waves.\n" );
            
            ncFFTComplex htilde0, htilde0mk_conj;
            
            for( i = 0; i < Nplus1; ++i ) {
                for( j = 0; j < Nplus1; ++j ) {
                    index = i * Nplus1 + j;
                    
                    htilde0        = hTilde_0( j,  i);
                    htilde0mk_conj = hTilde_0(-j, -i).Conjugate();
                    
                    hTilde0[index].x  = htilde0.a;
                    hTilde0[index].y  = htilde0.b;
                    hTilde0mk[index].x = htilde0mk_conj.a;
                    hTilde0mk[index].y = htilde0mk_conj.b;
                    
                    // Calculate vertices with bounding box coordinates.
                    firstOrigin[index].x =
                    vertices[index].x =
                    m_BoundingBox.min.x + ((float)j) * (m_BoundingBox.max.x - m_BoundingBox.min.x) / N;
                    
                    firstOrigin[index].y =
                    vertices[index].y =
                    m_BoundingBox.min.y + ((float)1.0f) * (m_BoundingBox.max.y - m_BoundingBox.min.y) / N;
                    
                    firstOrigin[index].z =
                    vertices[index].z =
                    m_BoundingBox.min.z + ((float)i) * (m_BoundingBox.max.z - m_BoundingBox.min.z) / N;
                    
                    // Normals.
                    normals[index].x = 0.0f;
                    normals[index].y = 1.0f;
                    normals[index].z = 0.0f;
                    
                    // UV.
                    uvCoords[index].x = (float)( (float)j / Nplus1 ) * OCEAN_TEXTURE_SCALE;
                    uvCoords[index].y = (float)( (float)i / Nplus1 ) * OCEAN_TEXTURE_SCALE;
                }
            }
        } else if( bHasAnimation && genType == GPU_OCEAN ) {
            g_Core->p_Console->Print( LOG_INFO, "Ocean will generate GPU waves.\n" );
            
            // TODO.
        } else {
            g_Core->p_Console->Print( LOG_INFO, "Ocean will not generate any wave.\n" );
            for( i = 0; i < Nplus1; ++i ) {
                for( j = 0; j < Nplus1; ++j ) {
                    index = i * Nplus1 + j;
                    
                    // Calculate vertices with bounding box coordinates.
                    firstOrigin[index].x =
                    vertices[index].x =
                    m_BoundingBox.min.x + ((float)j) * (m_BoundingBox.max.x - m_BoundingBox.min.x) / N;
                    
                    firstOrigin[index].y =
                    vertices[index].y =
                    m_BoundingBox.min.y + ((float)1.0f) * (m_BoundingBox.max.y - m_BoundingBox.min.y) / N;
                    
                    firstOrigin[index].z =
                    vertices[index].z =
                    m_BoundingBox.min.z + ((float)i) * (m_BoundingBox.max.z - m_BoundingBox.min.z) / N;
                    
                    // Normals.
                    normals[index].x = 0.0f;
                    normals[index].y = 1.0f;
                    normals[index].z = 0.0f;
                    
                    // UV.
                    uvCoords[index].x = (float)( (float)j / Nplus1 ) * OCEAN_TEXTURE_SCALE;
                    uvCoords[index].y = (float)( (float)i / Nplus1 ) * OCEAN_TEXTURE_SCALE;
                }
            }
            
        }
        
        Ocean_Waves = g_Core->p_Console->RegisterCVar( ECvarGroup::World, "ocean_waves", "Generate ocean waves?", true, CVFlag::None, ECvarType::Int );
        
        // Setup assets and another thingies.
        SetParameters();
        
//        ncALSound *oceanFx = g_soundSystem->GetSoundByName( "oceanwave" );
//        oceanFx->Play();

    }
    
    /**
     *  Generate ocean.
     */
    CWorldOcean::CWorldOcean( SBoundingBox& boundingBox, const int N, const float A, const Vec2 & windDir, const float length, bool hasAnimation, OceanGenType genType ) : oGravity( OCEAN_WAVE_GRAVITY ), N( N ), Nplus1( N + 1 ), N2( 0 ), Nplus1N( 0 ), A( A ), oWind( windDir ), Length( length ), vertices( NEKO_NULL ), h_tilde( NEKO_NULL ), h_tilde_slopex( NEKO_NULL ), h_tilde_slopez( NEKO_NULL ), h_tilde_dx( NEKO_NULL ), h_tilde_dz( NEKO_NULL ), fft( NEKO_NULL ), bHasAnimation( hasAnimation ), oceanType( genType )
    {
        Create( boundingBox, N, A, windDir, length, hasAnimation, genType );
    }
    
    /**
     *  Default constructor.
     */
    CWorldOcean::CWorldOcean()
    {
        
    }

    CWorldOcean::~CWorldOcean()
    {
        Release();
    }

    /**
     *  Remove FFT.
     */
    void CWorldOcean::RemoveFFTStuff()
    {
        if( h_tilde ) { pAllocator->Dealloc( h_tilde ) ; }
        if( h_tilde_slopex ) { pAllocator->Dealloc( h_tilde_slopex ) ; }
        if( h_tilde_slopez ) { pAllocator->Dealloc( h_tilde_slopez ) ; }
        if( h_tilde_dx ) { pAllocator->Dealloc( h_tilde_dx ) ; }
        if( h_tilde_dz ) { pAllocator->Dealloc( h_tilde_dz ) ; }

        if( fft != NEKO_NULL ) {
            delete fft;
            fft = NEKO_NULL;
        }
    }

    /**
     *  Remove the ocean.
     */
    void CWorldOcean::Release()
    {
        if( !bRenderOcean ) {
            return;
        }

        bRenderOcean = false;
        
        m_pQuadtree->Destroy();
        
        pAllocator->Dealloc( m_pQuadtree );// m_pQuadtree;
        pAllocator->Dealloc( vertices );// [] vertices;

        RemoveFFTStuff();
    }

    /**
     *  FFT dispersion.
     */
    float CWorldOcean::Dispersion( int32_t n_prime, int32_t m_prime )
    {
        float w_0 = 2.0f * nkMath::PI / OCEAN_DISPERSION_FACTOR;
        
        float kx = nkMath::PI * (2.0f * n_prime - N) / Length;
        float kz = nkMath::PI * (2.0f * m_prime - N) / Length;

        return floor( nkMath::FastSqrty(oGravity * nkMath::FastSqrty(kx * kx + kz * kz)) / w_0 ) * w_0;
    }

    /**
     *  Wave phillips.
     */
    float CWorldOcean::PhillipsSpectrum( int32_t n_prime, int32_t m_prime )
    {
        // Wave vector.
        Vec2 k( nkMath::PI * (2.0f * n_prime - N) / Length,
                 nkMath::PI * (2.0f * m_prime - N) / Length );

        // Magnitude.
        float k_length  = k.Length();
        
        // Check if wave vector is very small, if it is then
        // no need to calculate.
        if( k_length < 0.000001f ){ // Give it a little slope.
            return 0.0f;
        }

        float k_length2 = k_length  * k_length;
        float k_length4 = k_length2 * k_length2;

        float k_dot_w   = k.Normalize() * oWind.Normalize();
        float k_dot_w2  = k_dot_w * k_dot_w;

        float w_length  = oWind.Length();
        float L         = w_length * w_length / oGravity;
        float L2        = L * L;

        float damping   = 0.001;
        float l2        = L2 * damping * damping;

        return A * exp( -1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2 * exp( -k_length2 * l2 );
    }

    inline ncFFTComplex RandomGaussianVariable()
    {
        float x1, x2, w;

        do {
            x1 = 2.f * nkMath::UniformRandomf() - 1.f;
            x2 = 2.f * nkMath::UniformRandomf() - 1.f;
            w = x1 * x1 + x2 * x2;
        } while ( w >= 1.f );
        
        w = nkMath::FastSqrty( (-2.f * log(w)) / w );

        return ncFFTComplex( x1 * w, x2 * w );
    }

    ncFFTComplex CWorldOcean::hTilde_0( int32_t n_prime, int32_t m_prime )
    {
        ncFFTComplex r = RandomGaussianVariable();
        return r * nkMath::FastSqrty(PhillipsSpectrum(n_prime, m_prime) / 2.0f);
    }

    ncFFTComplex CWorldOcean::hTilde( float t, int32_t n_prime, int32_t m_prime )
    {
        int32_t index = m_prime * Nplus1 + n_prime;

        ncFFTComplex htilde0( hTilde0[index].x,  hTilde0[index].y );
        ncFFTComplex htilde0mkconj( hTilde0mk[index].x, hTilde0mk[index].x );

        float omegat = Dispersion( n_prime, m_prime ) * t;

        float cos_ = cos( omegat );
        float sin_ = sin( omegat );

        ncFFTComplex c0( cos_,  sin_ );
        ncFFTComplex c1( cos_, -sin_ );

        //ncFFTComplex res = htilde0 * c0 + htilde0mkconj * c1;

        return htilde0 * c0 + htilde0mkconj * c1;
    }

    /**
     *  Check if ocean chunk visible.
     *  Gosh, saves a lot performance.
     */
    bool CWorldOcean::IsOceanVerticeVisible( int32_t index2 )
    {
        Vec2 point = Vec2( vertices[index2].x, vertices[index2].z );
        CQuadtreeNode * node = m_pQuadtree->FindNode( point );

        if( node == NEKO_NULL ) {
            return false;
        }

        // Don't animate unless it's too far away.
        if( node->m_iChunkLOD >= 2 ) {
            return false;
        }

        // Is visible?
        return node->m_bCanBeSeen ;
    }

    static const float signs[] = { 1.0f, -1.0f };
    
    // THREADED!
    
    /**
     *  Make some waves using Fast Fourier Transform.
     */
    void CWorldOcean::EvaluateWavesFFT( float t )
    {
        if( m_pQuadtree == NEKO_NULL ) {
            return;
        }

        if( fft == NEKO_NULL ) {
            return;
        }

        float   kx, kz, len, lambda = -1.0f;
        int32_t     index, index1;
        int32_t     i;
        int32_t     j;

        for( i = 0; i < N; ++i )  {
            kz = nkMath::PI * (2.0f * i - N) / Length;
            for ( j = 0; j < N; ++j ) {
                kx = nkMath::PI * (2 * j - N) / Length;
                len = nkMath::FastSqrty( kx * kx + kz * kz );
                index = i * N + j;

                h_tilde[index] = hTilde( t, j, i );

                h_tilde_slopex[index] = h_tilde[index] * ncFFTComplex( 0, kx );
                h_tilde_slopez[index] = h_tilde[index] * ncFFTComplex( 0, kz );

                if( len < 0.000001f /* give it a little slope */ ) {
                    h_tilde_dx[index]     = ncFFTComplex( 0.0f, 0.0f );
                    h_tilde_dz[index]     = ncFFTComplex( 0.0f, 0.0f );
                } else {
                    h_tilde_dx[index]     = h_tilde[index] * ncFFTComplex( 0, -kx / len );
                    h_tilde_dz[index]     = h_tilde[index] * ncFFTComplex( 0, -kz / len );
                }
            }
        }

        
        for( i = 0; i < N; ++i ) {
            //if( h_tilde[(m_prime * 1) + (m_prime * N)].visible )
            fft->MakeFFT( h_tilde, h_tilde, 1, i * N );
            
            if( h_tilde_slopex[(i * 1) + (i * N)].visible ) {
                fft->MakeFFT( h_tilde_slopex, h_tilde_slopex, 1, i * N );
            }
            
            if( h_tilde_slopez[(i * 1) + (i * N)].visible ) {
                fft->MakeFFT( h_tilde_slopez, h_tilde_slopez, 1, i * N );
            }
            
            if( h_tilde_dx[(i * 1) + (i * N)].visible ) {
                fft->MakeFFT( h_tilde_dx, h_tilde_dx, 1, i * N );
            }
            
            if( h_tilde_dz[(i * 1) + (i * N)].visible ) {
                fft->MakeFFT( h_tilde_dz, h_tilde_dz, 1, i * N );
            }
        }
        
        for( j = 0; j < N; ++j ) {
            //if( h_tilde[(n_prime * N + n_prime)].visible )
            fft->MakeFFT( h_tilde, h_tilde, N, j );
            
            if( h_tilde_slopex[(j * N + j)].visible ) {
                fft->MakeFFT( h_tilde_slopex, h_tilde_slopex, N, j );
            }
            
            if( h_tilde_slopex[(j * N + j)].visible ) {
                fft->MakeFFT( h_tilde_slopez, h_tilde_slopez, N, j );
            }
            
            if( h_tilde_dx[(j * N + j)].visible ) {
                fft->MakeFFT( h_tilde_dx, h_tilde_dx, N, j );
            }
            
            if( h_tilde_dz[(j * N + j)].visible ) {
                fft->MakeFFT( h_tilde_dz, h_tilde_dz, N, j );
            }
        }

        
        int32_t sign;

        Vec3 n;
        for( i = 0; i < N; ++i ) {
            for( j = 0; j < N; ++j ) {
                index  = i * N + j;		// index into h_tilde..
                index1 = i * Nplus1 + j;	// index into vertices

                // Check if we can animate.
                if( !IsOceanVerticeVisible( index1 ) ) {
                    h_tilde[index].visible = false;
                    h_tilde_slopex[index].visible = false;
                    h_tilde_slopez[index].visible = false;
                    h_tilde_dx[index].visible = false;
                    h_tilde_dz[index].visible = false;
                } else {
                    h_tilde[index].visible = true;
                    h_tilde_slopex[index].visible = true;
                    h_tilde_slopez[index].visible = true;
                    h_tilde_dx[index].visible = true;
                    h_tilde_dz[index].visible = true;
                }

                sign = signs[(j + i) & 1];

                h_tilde[index]     = h_tilde[index] * sign;

                // height
                vertices[index1].y = h_tilde[index].a;

                // displacement
                h_tilde_dx[index] = h_tilde_dx[index] * sign;
                h_tilde_dz[index] = h_tilde_dz[index] * sign;

                vertices[index1].x = firstOrigin[index1].x + h_tilde_dx[index].a * lambda + 8.0f;
                vertices[index1].z = firstOrigin[index1].z + h_tilde_dz[index].a * lambda + 8.0f;

                // Normal.
                h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
                h_tilde_slopez[index] = h_tilde_slopez[index] * sign;

                n = Vec3( 0.0f - fabs(h_tilde_slopex[index].a), 1.0f, 0.0f - fabs(h_tilde_slopez[index].a) );
                n.Normalize();

                normals[index1] =  n;

                // For tiling.
                if( j == 0 && i == 0 ) {
                    vertices[index1 + N + Nplus1N].y = fabs(h_tilde[index].a);

                    vertices[index1 + N + Nplus1N].x = firstOrigin[index1 + N + Nplus1N].x + fabs(h_tilde_dx[index].a) * lambda;
                    vertices[index1 + N + Nplus1N].z = firstOrigin[index1 + N + Nplus1N].z + fabs(h_tilde_dz[index].a) * lambda;

                    normals[index1 + N + Nplus1N] =  n;
                }

                if( j == 0 ) {
                    vertices[index1 + N].y = fabs(h_tilde[index].a);

                    vertices[index1 + N].x = firstOrigin[index1 + N].x + fabs(h_tilde_dx[index].a) * lambda;
                    vertices[index1 + N].z = firstOrigin[index1 + N].z + fabs(h_tilde_dz[index].a) * lambda;

                    normals[index1 + N] =  n;
                }

                if( i == 0 ) {
                    vertices[index1 + Nplus1N].y = fabs(h_tilde[index].a);

                    vertices[index1 + Nplus1N].x = firstOrigin[index1 + Nplus1N].x + fabs(h_tilde_dx[index].a) * lambda;
                    vertices[index1 + Nplus1N].z = firstOrigin[index1 + Nplus1N].z + fabs(h_tilde_dz[index].a) * lambda;

                    normals[index1 + Nplus1N] =  n;
                }
            }
        }
    }

    /**
     *  Called from renderer thread!
     *  We are generating waves in ocean thread.
     */
    void CWorldOcean::Render()
    {
        if( !bRenderOcean ) {
            // Create buffers.
            vao_buffer = g_mainRenderer->CreateVertexArray();

            // Vertices.
            vbo_vertices = g_mainRenderer->AllocGPUBuffer( sizeof(Vec3) * N2, EBufferStorageType::Array, EBufferType::Stream );
            g_mainRenderer->BufferPointer( &vbo_vertices, 0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (char*)NEKO_NULL );
            g_mainRenderer->FinishBuffer( &vbo_vertices, 0, N2 );
            
            // Normals.
            vbo_normals = g_mainRenderer->AllocGPUBuffer( sizeof(Vec3) * N2, EBufferStorageType::Array, EBufferType::Stream );
            g_mainRenderer->BufferPointer( &vbo_normals, 2, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)NEKO_NULL );
            g_mainRenderer->FinishBuffer( &vbo_normals, 0, N2 );
            
            // Texture coordinates.
            vbo_uv = g_mainRenderer->AllocGPUBuffer( sizeof(Vec2) * N2, EBufferStorageType::Array, EBufferType::Static );
            g_mainRenderer->BufferData( &vbo_uv, &uvCoords[0], 0, sizeof(Vec2) * N2 );
            g_mainRenderer->BufferPointer( &vbo_uv, 1, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)NEKO_NULL );
            g_mainRenderer->FinishBuffer( &vbo_uv, 0, N2 );
            
            glBindVertexArray( 0 );

            // Create ocean quadtree.
            m_pQuadtree = (CQuadtree*)pAllocator->Alloc( sizeof(CQuadtree) );// new CQuadtree();
            m_pQuadtree->Create( OCEAN, &m_BoundingBox, Vec2i( Nplus1, Nplus1 ), 32, 0, NEKO_NULL );
            m_pQuadtree->CalculateBoundingBoxAndCollisionData( vertices, false /* do not generate collision */, true /* has bounding box */ );
            //m_pQuadtree->CalculateCollision( mSeparatedVertices );
            m_pQuadtree->DeleteIndices();
            
//            FreeMemory( sizeof(uvCoords), uvCoords );// delete [] uvCoords;
            
            // Refresh settings.
            Refresh();

            // Now we can render our ocean.
            bRenderOcean = true;
            
            return;
        }

        g_mainRenderer->SetDepthWriteMode(false);
        
        // Apply shader.
        oShader->Use();

        glBindVertexArray( vao_buffer );

        // Update GPU buffer data.
        glBindBuffer( GL_ARRAY_BUFFER, vbo_vertices.Handle );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Vec3) * N2, vertices );

        glBindBuffer( GL_ARRAY_BUFFER, vbo_normals.Handle );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Vec3) * N2, normals );

        // Set shader uniforms.
        oShader->SetUniform( oShaderUniforms[WMVP_UNIFORM], 1, false, g_Core->p_Camera->ViewProjectionMatrix.m );
        oShader->SetUniform( oShaderUniforms[WLIGHTPOSITION_UNIFORM], g_pbEnv->GetCurrentLightDir() * 1000.0f );
        oShader->SetUniform( oShaderUniforms[WCAMERAPOSITION_UNIFORM], g_Core->p_Camera->vEye );
        oShader->SetUniform( oShaderUniforms[WLIGHTCOLOR_UNIFORM], g_pbEnv->GetSkyLightColor() );
        
        //vec3f(10.f / 255.f, 40.f / 255.f, 120.f / 255.f) * 0.1f;
        oShader->SetUniform( oShaderUniforms[WWATERCOLOR_UNIFORM], m_waterColor );
        oShader->SetUniform( oShaderUniforms[WTIMEDENSITYCLIP_UNIFORM], g_Core->GetTime() * -0.0001f, 0.216 );
//        oShader->SetUniform( "perspectiveInverse", 1, GL_FALSE, g_Core->p_Camera->ProjectionInvertMatrix.m );

        // Attach textures.
        
        // Reflection map.
        g_mainRenderer->BindTexture( 0, g_mainRenderer->g_waterReflectionBuffer->GetUnitRT0() );
        
        // Refraction map.
        g_mainRenderer->BindTexture( 1, g_mainRenderer->g_sceneBuffer[CRenderer::EYE_FULL]->GetUnitRT0() );

        // Normal map.
        g_mainRenderer->BindTexture( 2, oNormalMap->m_pDiffuse->GetId() );

        // Scene depth used for water depth.
        g_mainRenderer->BindTexture( 3, g_mainRenderer->g_sceneBuffer[CRenderer::EYE_FULL]->GetUnitDepth() );

        // Foam sampler.
        g_mainRenderer->BindTexture( 4, oSeaFoam->m_pDiffuse->GetId() );
 
        // Draw ocean!
        m_pQuadtree->Draw( false );

        glBindVertexArray( 0 );

        oShader->Next();
        
        g_mainRenderer->SetDepthWriteMode(true);
    }

    /**
     *  Update water graphic settings.
     */
    void CWorldOcean::Refresh( void )
    {
        // Set up parameters
        oShader->Use();

        oShaderUniforms[WTIMEDENSITYCLIP_UNIFORM] = oShader->UniformLocation( "time_density" );
        oShaderUniforms[WMODELMATRIX_UNIFORM] = oShader->UniformLocation( "ModelMatrix" );
        oShaderUniforms[WMVP_UNIFORM] = oShader->UniformLocation( "MVP" );
        oShaderUniforms[WWATERCOLOR_UNIFORM] = oShader->UniformLocation( "mWaterColor" );
        oShaderUniforms[WLIGHTPOSITION_UNIFORM] = oShader->UniformLocation( "mLightPosition" );
        oShaderUniforms[WCAMERAPOSITION_UNIFORM] = oShader->UniformLocation( "cameraPos" );
        oShaderUniforms[WLIGHTCOLOR_UNIFORM] = oShader->UniformLocation( "fogColor" );
        
        m_modelViewPos.Identity();
        m_modelViewPos.Translate( Vec3(0.0f, 250.0f, 0.0f) );
        oShader->SetUniform( oShaderUniforms[WMODELMATRIX_UNIFORM], 1, false, m_modelViewPos.m );
        
//        oShader->SetUniform( "projectionMatrixInverse", 1, GL_FALSE, g_Core->p_Camera->ProjectionInvertMatrix.m );
        
        // Water samplers.
        oShader->SetUniform( "reflection_texture", 0 );
        oShader->SetUniform( "refraction_texture", 1 );
        oShader->SetUniform( "normal_texture", 2 );
        oShader->SetUniform( "depth_texture", 3 );
        oShader->SetUniform( "foamSampler", 4 ); // That's how we're going to make it look even cooler and realistic.
        oShader->SetUniform( "wavesSampler", 5 );
        
        oShader->Next();
    }
}

#endif
