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

//
//  Perlin Noise generation by Ken Perlin.
//

#include "Noise.h"
#include "../../Platform/Shared/SystemShared.h"
#include "../../Math/GameMath.h"

#define B SAMPLE_SIZE
#define BM (SAMPLE_SIZE - 1)

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

namespace Neko {
    inline static float lerp( float t, float a, float b ) {
        return a + t * ( b - a );
    }
    
    inline static float s_curve( const float t ) {
        return t * t * ( 3.0f - 2.0f * t );
    }
    
    inline static float at2( float rx, float ry, float qx, float qy ) {
        return rx * qx + ry * qy;
    }
    
#define setup(i,b0,b1,r0,r1) \
    t = vec[i] + N; \
    b0 = ((int)t) & BM; \
    b1 = (b0+1) & BM; \
    r0 = t - (int)t; \
    r1 = r0 - 1.0f;
    
    
    float ncNoisePerlin::Get( const float x, const float y ) {
        float vec[2];
        vec[0] = x;
        vec[1] = y;
        
        return perlin_noise_2D( vec );
    };
    
    float ncNoisePerlin::Get( const float x, const float y, const float z ) {
        float vec[3];
        vec[0] = x;
        vec[1] = y;
        vec[2] = z;
        
        return perlin_noise_3D( vec );
    };
    
    float ncNoisePerlin::noise1( float arg ) {
        int bx0, bx1;
        float rx0, rx1, sx, t, u, v, vec[1];
        
        vec[0] = arg;
        
        if( mStart ) {
            srand( mSeed );
            mStart = false;
            init();
        }
        
        setup( 0, bx0, bx1, rx0, rx1 );
        
        sx = s_curve(rx0);
        
        u = rx0 * g1[ p[ bx0 ] ];
        v = rx1 * g1[ p[ bx1 ] ];
        
        return lerp( sx, u, v );
    }
    
    float ncNoisePerlin::noise2( float vec[2] ) {
        int bx0, bx1, by0, by1, b00, b10, b01, b11;
        float rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
        int i, j;
        
        if( mStart ) {
            srand( mSeed );
            mStart = false;
            init();
        }
        
        setup( 0, bx0, bx1, rx0, rx1 );
        setup( 1, by0, by1, ry0, ry1 );
        
        i = p[bx0];
        j = p[bx1];
        
        b00 = p[i + by0];
        b10 = p[j + by0];
        b01 = p[i + by1];
        b11 = p[j + by1];
        
        sx = s_curve( rx0 );
        sy = s_curve( ry0 );
        
        q = g2[b00];
        u = at2( rx0, ry0, q[0], q[1] );
        q = g2[b10];
        v = at2( rx1, ry0, q[0], q[1] );
        a = lerp( sx, u, v );
        
        q = g2[b01];
        u = at2( rx0, ry1, q[0], q[1] );
        q = g2[b11];
        v = at2( rx1, ry1, q[0], q[1] );
        b = lerp( sx, u, v );
        
        return lerp( sy, a, b );
    }
    
    float ncNoisePerlin::noise3( float vec[3] )
    {
        int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
        float rx0, rx1, ry0, ry1, rz0, rz1, *q, sy, sz, a, b, c, d, t, u, v;
        int i, j;
        
        if( mStart )
        {
            srand(mSeed);
            mStart = false;
            init();
        }
        
        setup( 0, bx0,bx1, rx0,rx1 );
        setup( 1, by0,by1, ry0,ry1 );
        setup( 2, bz0,bz1, rz0,rz1 );
        
        i = p[ bx0 ];
        j = p[ bx1 ];
        
        b00 = p[ i + by0 ];
        b10 = p[ j + by0 ];
        b01 = p[ i + by1 ];
        b11 = p[ j + by1 ];
        
        t  = s_curve(rx0);
        sy = s_curve(ry0);
        sz = s_curve(rz0);
        
#define at3( rx, ry, rz ) ( rx * q[0] + ry * q[1] + rz * q[2] )
        
        q = g3[ b00 + bz0 ] ; u = at3(rx0,ry0,rz0);
        q = g3[ b10 + bz0 ] ; v = at3(rx1,ry0,rz0);
        a = lerp(t, u, v);
        
        q = g3[ b01 + bz0 ] ; u = at3(rx0,ry1,rz0);
        q = g3[ b11 + bz0 ] ; v = at3(rx1,ry1,rz0);
        b = lerp(t, u, v);
        
        c = lerp(sy, a, b);
        
        q = g3[ b00 + bz1 ] ; u = at3(rx0,ry0,rz1);
        q = g3[ b10 + bz1 ] ; v = at3(rx1,ry0,rz1);
        a = lerp(t, u, v);
        
        q = g3[ b01 + bz1 ] ; u = at3(rx0,ry1,rz1);
        q = g3[ b11 + bz1 ] ; v = at3(rx1,ry1,rz1);
        b = lerp(t, u, v);
        
        d = lerp(sy, a, b);
        
        return lerp( sz, c, d );
    }
    
    void ncNoisePerlin::normalize2( float v[2] ) {
        
        float s;
        
        s = (float)nkMath::FastSqrty(v[0] * v[0] + v[1] * v[1]);
        s = 1.0f/s;
        
        v[0] = v[0] * s;
        v[1] = v[1] * s;
    }
    
    void ncNoisePerlin::normalize3( float v[3] ) {
        
        float s;
        
        s = (float)nkMath::FastSqrty(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
        s = 1.0f/s;
        
        v[0] = v[0] * s;
        v[1] = v[1] * s;
        v[2] = v[2] * s;
    }
    
    void ncNoisePerlin::init(void)
    {
        int i, j, k;
        
        for (i = 0 ; i < B ; i++)
        {
            p[i] = i;
            g1[i] = (float)((rand() % (B + B)) - B) / B;
            
            for (j = 0 ; j < 2 ; j++)
                g2[i][j] = (float)((rand() % (B + B)) - B) / B;
            normalize2(g2[i]);
            for (j = 0 ; j < 3 ; j++)
                g3[i][j] = (float)((rand() % (B + B)) - B) / B;
            normalize3(g3[i]);
        }
        
        while (--i)
        {
            k = p[i];
            p[i] = p[j = rand() % B];
            p[j] = k;
        }
        
        for (i = 0 ; i < B + 2 ; i++)
        {
            p[B + i] = p[i];
            g1[B + i] = g1[i];
            
            for (j = 0 ; j < 2 ; j++)
                g2[B + i][j] = g2[i][j];
            for (j = 0 ; j < 3 ; j++)
                g3[B + i][j] = g3[i][j];
        }
        
    }
    
    
    float ncNoisePerlin::perlin_noise_2D( float vec[2] ) {
        const int terms    = mOctaves;
        //float freq   = mFrequency;
        float result = 0.0f;
        float amp = mAmplitude;
        
        vec[0] *= mFrequency;
        vec[1] *= mFrequency;
        
        for( int i = 0; i < terms; ++i ) {
            result += noise2(vec) * amp;
            vec[0] *= 2.0f;
            vec[1] *= 2.0f;
            amp *= 0.5f;
        }
        
        
        return result;
    }
    
    float ncNoisePerlin::perlin_noise_3D( float vec[3] ) {
        const int terms    = mOctaves;
        //float freq   = mFrequency;
        float result = 0.0f;
        float amp = mAmplitude;
        
        vec[0] *= mFrequency;
        vec[1] *= mFrequency;
        vec[2] *= mFrequency;
        
        for( int i = 0; i < terms; ++i ) {
            result += noise3(vec) * amp;
            vec[0] *= 2.0f;
            vec[1] *= 2.0f;
            vec[2] *= 2.0f;
            amp *= 0.5f;
        }
        
        
        return result;
    }
    
    void ncNoisePerlin::Set( int octaves, float freq, float amp, int seed )
    {
        mOctaves = octaves;
        mFrequency = freq;
        mAmplitude = amp;
        mSeed = seed;
        mStart = true;
    }
    
    ncNoisePerlin::ncNoisePerlin( int octaves, float freq, float amp, int seed )
    {
        mOctaves = octaves;
        mFrequency = freq;
        mAmplitude = amp;
        mSeed = seed;
        mStart = true;
    }
}