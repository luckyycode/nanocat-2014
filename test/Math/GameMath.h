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
//  GameMath.h
//  GameMath library header.. :P
//
//  Created by Neko Vision on 10/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#ifndef gamemath_h
#define gamemath_h

#include <math.h>

#   if defined( _WIN32 ) // x64
    #include <cstdlib>
#   endif

#include "../Platform/Shared/SystemShared.h"

#if !defined( NEKO_NO_ASSIMP )
    #   if !defined( _WIN32 )
        #include "scene.h" // aiMatrix and another Assimp thingies..
    #   else
        #include <assimp/scene.h>
    #   endif
#   endif

namespace nkMath {
    
    static const int32_t POINT_ON_PLANE = 0;
    static const int32_t POINT_IN_FRONT_OF_PLANE = 1;
    static const int32_t POINT_BEHIND_PLANE = 2;


    // - Default constants.
    
    static const float PI              = 3.14159265358979323846f;
    static const float ANG2RAD         = PI / 180.0f;
    static const float PI2             = 2.0f * PI;
    static const float PIHALF          = 0.5f * PI;
    static const float EPSILON         = 2.71828182845904523536f;
    static const float INF             = 999999999.0f;
    static const float SEC2MS          = 1000.0f;
    static const float MS2SEC          = 0.001f;
    static const float PI2D            = PI / 2.0f;

    static const float Deg2Rad = (PI * 2.0f) / 360.0f;
    static const float Rad2Deg = 360.0f / (PI * 2.0f);
    
    /**
     *
     *  Tools and utilities.
     *
     */
    
    /**
     *  Make absolute(positive) number.
     */
    template <typename t>
    NEKO_FORCE_INLINE t Abs( const t a )
    {
        if( a < 0 ) {
            return 0 - a;
        }
        
        return a;
    }
    
    /**
     *  Degree to radian.
     */
    NEKO_FORCE_INLINE float DEGTORAD( float a )
    {
        return ( (a) * PI / 180.0f );
    }

    /**
     *  Radian to degree.
     */
    NEKO_FORCE_INLINE float RADTODEG( float radian )
    {
        return ((radian) * (180.0f / PI));
    }

    
    /**
     *  Return positive or negative number by x value.
     */
    NEKO_FORCE_INLINE int32_t Sign( int32_t x )
    {
        return ( x > 0 ? +1 : ( x < 0 ? -1 : 0 ) );
    }

    /**
     *  Cotangent.
     */
    NEKO_FORCE_INLINE float Cot( float angle )
    {
        return (float)(1.0 / tan(angle));
    }

    /**
     *  Round the float value.
     */
    NEKO_FORCE_INLINE int Roundf( float value )
    {
        return (int)(value + 0.5);
    }

    /**
     * Lerp.
     */
    NEKO_FORCE_INLINE float Lerpf( float a, float b, float t )
    {
        return a + (b - a) * t;
    }

    NEKO_FORCE_INLINE float LerpfAlt( float a, float b, float t )
    {
        return ( 1 - t ) * a + t * b;
    }

    /**
     *    Check if given value is float.
     */
    NEKO_FORCE_INLINE bool IsFloatValue( float value )
    {
        if( value - (int32_t)value == 0 ) {
            return true;
        } else {
            return false;
        }
    }

    /**
     *    Get random integer.
     */
    NEKO_FORCE_INLINE int RandInt32Seed( int *seed )
    {
        *seed = (69069 * *seed + 1);
        return *seed;
    }

    /**
     *    Random integer.
     */
    NEKO_FORCE_INLINE int Random( int max = RAND_MAX )
    {
        return rand() % ( max + 1 );
    }

    /**
     *  Get random float value.
     */
    NEKO_FORCE_INLINE float RandFloat( int *seed )
    {
        return ( RandInt32Seed( seed ) & 0xffff ) / (float)0x10000;
    }

    /**
     *  Get alternative random float value.
     */
    NEKO_FORCE_INLINE float RandFloatAlt(float min, float max)
    {
        return (max - min) * (/*static_cast<float>(*/(float)rand()/*)*/ / (float)RAND_MAX) + min;
    }

    /**
     *  Get random counting on RAND_MAX.
     */
    NEKO_FORCE_INLINE float UniformRandomf()
    {
        return (float)rand() / RAND_MAX;
    }

    /**
     *  Larger random.
     */
    NEKO_FORCE_INLINE unsigned long LargeRandom()
    {
        return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
    }

    /**
     *  Get max number.
     */
    template<class T>
    NEKO_FORCE_INLINE T Max( T min, T max )
    {
        return max > min ? max : min;
    }

    /**
     *  Get min number.
     */
    template<class T>
    NEKO_FORCE_INLINE T Min( T min, T max )
    {
        return max < min ? max : min;
    }

    /**
     *  Check if two values have almost the same number.
     */
    NEKO_FORCE_INLINE bool Close(float f1, float f2)
    {
        return fabsf((f1 - f2) / ((f2 == 0.0f) ? 1.0f : f2)) < EPSILON;
    }
    
    /**
     *  Clamp value. ( min > value < max )
     */
    template<typename T>
    NEKO_FORCE_INLINE T Clamp( T value, T min, T max )
    {
        if( value > max ) {
            return max;
        }

        if( value < min ) {
            return min;
        } else {
            return value;
        }
    }

    /**
     *  Mix two values.
     */
    template<typename T>
    NEKO_FORCE_INLINE static T Mix( const T a, const T b, float c )
    {
        return a * (1.0f - c) + b * c;
    }

    /**
     *  Integer float.
     */
    NEKO_FORCE_INLINE int IntFloor( double x )
    {
        int i = (int)x; /* truncate */
        return i - ( i > x ); /* convert trunc to floor */
    }

    /**
     *  Faster than standart sqrt implementation by 4.7x.
     */
    NEKO_FORCE_INLINE float FastSqrty( float val )
    {
        // float check.
        union
        {
            int tmp;
            float val;
        } vals;

        vals.val = val;
        vals.tmp -= 1 << 23; // Remove last bit so 1.0 gives 1.0.
        // tmp is now an approximation to logbase2(val)
        vals.tmp >>= 1; // Divide by 2.
        vals.tmp += 1 << 29; // Add 64 to exponent: ( e + 127 ) / 2 = ( e / 2 ) + 63;
        // vals.tmp = (1<<29) + (vals.tmp >> 1) - (1<<22) + 0x4C000;  // I am working on a way to improve this value.
        // that represents (e/2)-64 but want e/2
        return vals.val;
    }

    /**
     *  Faster pow ( I hope so ).
     */
    NEKO_FORCE_INLINE long long FastPow(int a,int n)
    {
        long long result = 1;
        long long power = n;
        long long value = a;

        while( power > 0)
        {
            if( power & 1) {
                result = result * value;
                result = result % 1000000007;
            }
            value = value * value;
            value = value % 1000000007;
            power /= 2;
            // or..
            //power >>= 1;
        }
        return result;
    }
    
    /**
     *  Random float.
     */
    NEKO_FORCE_INLINE float grandf( float fMin, float fAdd )
    {
        float fRandom = float( rand() % (RAND_MAX + 1)) / float( RAND_MAX );
        return fMin + fAdd * fRandom;
    }
    
    /**
     *  Cubical expression.
     */
    template<typename t>
    NEKO_FORCE_INLINE t CubicSmooth( t x)
    {
        return x * x * (3 - 2 * x);
    }
    
    /**
     *  Clamp the number between 0 and 1.
     */
    template<typename t>
    NEKO_FORCE_INLINE t Clamp01(const t x)
    {
        return Clamp(x, (t)0, (t)1);
    }
    
    /**
     *  Inverse lerp.
     */
    NEKO_FORCE_INLINE float InverseLerp( float a, float b, float value )
    {
        float c = b - a;
        if( c == 0.0f ) {
            return 0.5f;
        } else {
            return (value - a) / (b - a);
        }
    }
    
}

namespace Neko {


    /**
     * Fast Fourier Transform.

     * http://www.keithlantz.net/
     * http://www.keithlantz.net/2011/11/ocean-simulation-part-two-using-the-fast-fourier-transform/
     */

    /**
     *  FFT Complex.
     */
    class ncFFTComplex
    {
    private:
    protected:
    public:
        float a, b;

        static uint32_t     additions, multiplications;

        ncFFTComplex();
        ncFFTComplex( float a, float b );
        ncFFTComplex        Conjugate();

        ncFFTComplex        operator*( const ncFFTComplex& c ) const;
        ncFFTComplex        operator+( const ncFFTComplex& c ) const;
        ncFFTComplex        operator-( const ncFFTComplex& c ) const;
        ncFFTComplex        operator-() const;
        ncFFTComplex        operator*( const float c ) const;
        ncFFTComplex&       operator=( const ncFFTComplex& c );

        inline static void         Reset();

        bool visible;   // move me
        
    };

    class INekoAllocator;
    
    /**
     *   Fast Fourier Transform implementation.
     */
    class ncFFT
    {
    private:
        uint32_t N, which;
        uint32_t log_2_N;

        uint32_t *reversed; // Bit reversal.

        ncFFTComplex **T;
        ncFFTComplex *c[2];
        
        INekoAllocator  * pAllocator;
    protected:
    public:
        ncFFT();
        ncFFT( uint32_t N );
        ~ncFFT();

        /**
         *  Create a new fast fourier transform pattern.=
         */
        void Create( uint32_t N );
        uint32_t Reverse( uint32_t i );
        ncFFTComplex t( uint32_t x, uint32_t N );
        inline void MakeFFT( ncFFTComplex* input, ncFFTComplex* output, int32_t stride, int32_t offset )
        {
            uint32_t i;
            uint32_t j;
            uint32_t k;
            
            for( i = 0; i < N; ++i ) {
                // Loop through input.
                c[which][i] = input[reversed[i] * stride + offset];
            }
            
            uint32_t loops       = N >> 1; // bit operations
            uint32_t size        = 1 << 1;
            uint32_t size_over_2 = 1;
            uint32_t w_          = 0;
            
            for( i = 1; i <= log_2_N; ++i ) {
                which ^= 1;
                for( j = 0; j < loops; ++j ) {
                    for( k = 0; k < size_over_2; ++k ) {
                        c[which][size * j + k] =  c[which ^ 1][size * j + k] +
                        c[which^ 1][size * j + size_over_2 + k] * T[w_][k];
                    }
                    
                    for( k = size_over_2; k < size; ++k ) {
                        c[which][size * j + k] =  c[which ^ 1][size * j - size_over_2 + k] -
                        c[which ^ 1][size * j + k] * T[w_][k - size_over_2];
                    }
                }
                
                loops       >>= 1;
                size        <<= 1;
                size_over_2 <<= 1;
                w_++;
            }
            
            // FFT output.
            for( i = 0; i < N; ++i ) {
                output[i * stride + offset] = c[which][i];
            }
        }
    };

}

#endif
