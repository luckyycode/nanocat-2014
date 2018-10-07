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

// Noise generation method by Ken Perlin.
/* (copyright Ken Perlin) */

#ifndef __noise_h__
#define __noise_h__

#include <math.h>

namespace Neko {
    static const int SAMPLE_SIZE = 1024;
    
    /**
     *  Perlin noise parameters.
     */
    struct NoisePerlinParams {
        int octave;
        int freq;
        int amp;
        long int seed;
    };
    
    /**
     *  Perlin noise generator.
     */
    class ncNoisePerlin
    {
    public:
        
        ncNoisePerlin( int octaves, float freq, float amp, int seed );
        
        void Set( int octaves, float freq, float amp, int seed );
        
        float Get( const float x, const float y );
        float Get( const float x, const float y, const float z );
        
        float noise1( float arg );
        float noise2( float vec[2] );
        float noise3( float vec[3] );
        
    private:
        
        void init_perlin( int n,float p );
        float perlin_noise_2D( float vec[2] );
        float perlin_noise_3D( float vec[3] );
        
        
        void normalize2( float v[2] );
        void normalize3( float v[3] );
        void init( void );
        
        int   mOctaves;
        float mFrequency;
        float mAmplitude;
        int   mSeed;
        
        int p[SAMPLE_SIZE + SAMPLE_SIZE + 2];
        float g3[SAMPLE_SIZE + SAMPLE_SIZE + 2][3];
        float g2[SAMPLE_SIZE + SAMPLE_SIZE + 2][2];
        float g1[SAMPLE_SIZE + SAMPLE_SIZE + 2];
        bool  mStart;
        
    };
}


#endif // __noise_h__
