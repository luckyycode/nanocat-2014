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
//  Utils.h
//  Utilities..
//
//  Created by Neko Code on 4/25/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../Core.h"
#include "../Console/ConsoleCommand.h"
#include "../../AssetCommon/FileSystem.h"
#include "../String/StringHelper.h"

#include "Utils.h"

/*inline*/ namespace NekoUtils {
    
    namespace NoiseOperations
    {
        /**
         *  Smooth noise generation.
         */
        double smoothNoise(double x, double y, double z, double * noise)
        {
            //get fractional part of x and y
            double fractX = x - int(x);
            double fractY = y - int(y);
            double fractZ = z - int(z);
            
            //wrap around
            int x1 = (int(x) + noiseWidth) % noiseWidth;
            int y1 = (int(y) + noiseHeight) % noiseHeight;
            int z1 = (int(z) + noiseDepth) % noiseDepth;
            
            //neighbor values
            int x2 = (x1 + noiseWidth - 1) % noiseWidth;
            int y2 = (y1 + noiseHeight - 1) % noiseHeight;
            int z2 = (z1 + noiseDepth - 1) % noiseDepth;
            
            //smooth the noise with bilinear interpolation
            double value = 0.0;
            value += fractX       * fractY       * fractZ       * noise[x1+y1+z1];
            value += fractX       * (1 - fractY) * fractZ       * noise[x1+y2+z1];
            value += (1 - fractX) * fractY       * fractZ       * noise[x2+y1+z1];
            value += (1 - fractX) * (1 - fractY) * fractZ       * noise[x2+y2+z1];
            
            value += fractX       * fractY       * (1 - fractZ) * noise[x1+y1+z2];
            value += fractX       * (1 - fractY) * (1 - fractZ) * noise[x1+y2+z2];
            value += (1 - fractX) * fractY       * (1 - fractZ) * noise[x2+y1+z2];
            value += (1 - fractX) * (1 - fractY) * (1 - fractZ) * noise[x2+y2+z2];
            
            return value;
        }
        
        /**
         *  Noise turbulence.
         */
        double turbulence( double x, double y, double z, double size, double * noise )
        {
            double value = 0.0, initialSize = size;
            
            while(size >= 1) {
                value += smoothNoise(x / size, y / size, z / size, noise) * size;
                size /= 2.0;
            }
            
            return(128.0 * value / initialSize);
        }
    }
    
    /**
     *  Check line intersection.
     */
    bool LineIntersect1f( float start, float axisdir, float min, float max, float &enter, float &exit )
    {
        // Intersection params.
        float t0, t1;
        t0 = (min - start) / axisdir;
        t1 = (max - start) / axisdir;
        
        // sort intersections
        if( t0 > t1 ) {
            float t = t0;
            t0 = t1;
            t1 = t;
        }
        
        // Reduce interval.
        if( t0 > enter )
            enter = t0;
        
        if( t1 < exit )
            exit = t1;
        
        // Ray misses the box.
        if( exit < enter )
            return false;
        
        return true;
    }

    
    Cubic * calcNaturalCubic(int n, float * x)
    {
        float * gamma = new float[n+1];
        float * delta = new float[n+1];
        float * D = new float[n+1];
        int i;
        
        gamma[0] = 1.0f/2.0f;
        
        for ( i = 1; i < n; i++)
        {
            gamma[i] = 1/(4-gamma[i-1]);
        }
        
        gamma[n] = 1/(2-gamma[n-1]);
        
        delta[0] = 3*(x[1]-x[0])*gamma[0];
        
        for ( i = 1; i < n; i++)
        {
            delta[i] = (3*(x[i+1]-x[i-1])-delta[i-1])*gamma[i];
        }
        
        delta[n] = (3*(x[n]-x[n-1])-delta[n-1])*gamma[n];
        
        D[n] = delta[n];
        
        for ( i = n-1; i >= 0; i--)
        {
            D[i] = delta[i] - gamma[i]*D[i+1];
        }
        
        Cubic * C = new Cubic[n+1];
        for ( i = 0; i < n; i++) {
            C[i] = Cubic((float)x[i], D[i], 3*(x[i+1] - x[i]) - 2*D[i] - D[i+1],
                         2*(x[i] - x[i+1]) + D[i] + D[i+1]);
        }
        
        delete [] delta;
        delete [] gamma;
        delete [] D;
        
        return C;
    }
    
    unsigned char Nibble(char c)
    {
        if( (c >= '0') && (c <= '9') ) {
            return (unsigned char)(c - '0');
        }
        
        if( (c >= 'A') && (c <= 'F') ) {
            return (unsigned char)(c - 'A' + 0x0a);
        }
        
        if( (c >= 'a') && (c <= 'f') ) {
            return (unsigned char)(c - 'a' + 0x0a);
        }
        
        return 0;
    }
    
    int    LittleLong (int l)
    {
        byte    b1,b2,b3,b4;
        
        b1 = l & 255;
        b2 = (l >> 8) & 255;
        b3 = (l >> 16) & 255;
        b4 = (l >> 24) & 255;
        
        return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
    }
    
    /**
     *  Parse CSV file.
     */
    size_t ParseCSV( char *line, char *list[], size_t size, const char delimiter )
    {
        char   *p;
        size_t n;
        
        p = line;
        n = 0;
        for ( ; ; )
        {
            /* Ditch leading commas */
            while ( *p == delimiter )
                p++;
            /* Nothing of use */
            if ( *p == '\0' )
                return n;
            /* Save the string */
            list[n++] = p;
            /* Find the next field */
            while ( *p != delimiter && *p != '\0' )
                p++;
            /* Nothing else of use or too many fields */
            if ( *p == '\0' || n >= size )
                return n;
            /* Split the field */
            *p++ = '\0';
        }
    }
    
    
    //
    //  GL projection function.
    //  From GLU library.
    //
    static void __gluMultMatrixVecf(const GLfloat matrix[16], const GLfloat in[4],
                                    GLfloat out[4])
    {
        int i;
        
        for( i = 0; i < 4; ++i ) {
            out[i] =
            in[0] * matrix[0*4+i] +
            in[1] * matrix[1*4+i] +
            in[2] * matrix[2*4+i] +
            in[3] * matrix[3*4+i];
        }
    }
    
    /*
     ** Invert 4x4 matrix.
     ** Contributed by David Moore (See Mesa bug #6748)
     */
    static int __gluInvertMatrixf(const GLfloat m[16], GLfloat invOut[16])
    {
        double inv[16], det;
        int i;
        
        inv[0] = m[5]  * m[10] * m[15] -
        m[5]  * m[11] * m[14] -
        m[9]  * m[6]  * m[15] +
        m[9]  * m[7]  * m[14] +
        m[13] * m[6]  * m[11] -
        m[13] * m[7]  * m[10];
        
        inv[4] = -m[4]  * m[10] * m[15] +
        m[4]  * m[11] * m[14] +
        m[8]  * m[6]  * m[15] -
        m[8]  * m[7]  * m[14] -
        m[12] * m[6]  * m[11] +
        m[12] * m[7]  * m[10];
        
        inv[8] = m[4]  * m[9] * m[15] -
        m[4]  * m[11] * m[13] -
        m[8]  * m[5] * m[15] +
        m[8]  * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];
        
        inv[12] = -m[4]  * m[9] * m[14] +
        m[4]  * m[10] * m[13] +
        m[8]  * m[5] * m[14] -
        m[8]  * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];
        
        inv[1] = -m[1]  * m[10] * m[15] +
        m[1]  * m[11] * m[14] +
        m[9]  * m[2] * m[15] -
        m[9]  * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];
        
        inv[5] = m[0]  * m[10] * m[15] -
        m[0]  * m[11] * m[14] -
        m[8]  * m[2] * m[15] +
        m[8]  * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];
        
        inv[9] = -m[0]  * m[9] * m[15] +
        m[0]  * m[11] * m[13] +
        m[8]  * m[1] * m[15] -
        m[8]  * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];
        
        inv[13] = m[0]  * m[9] * m[14] -
        m[0]  * m[10] * m[13] -
        m[8]  * m[1] * m[14] +
        m[8]  * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];
        
        inv[2] = m[1]  * m[6] * m[15] -
        m[1]  * m[7] * m[14] -
        m[5]  * m[2] * m[15] +
        m[5]  * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];
        
        inv[6] = -m[0]  * m[6] * m[15] +
        m[0]  * m[7] * m[14] +
        m[4]  * m[2] * m[15] -
        m[4]  * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];
        
        inv[10] = m[0]  * m[5] * m[15] -
        m[0]  * m[7] * m[13] -
        m[4]  * m[1] * m[15] +
        m[4]  * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];
        
        inv[14] = -m[0]  * m[5] * m[14] +
        m[0]  * m[6] * m[13] +
        m[4]  * m[1] * m[14] -
        m[4]  * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];
        
        inv[3] = -m[1] * m[6] * m[11] +
        m[1] * m[7] * m[10] +
        m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] -
        m[9] * m[2] * m[7] +
        m[9] * m[3] * m[6];
        
        inv[7] = m[0] * m[6] * m[11] -
        m[0] * m[7] * m[10] -
        m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] +
        m[8] * m[2] * m[7] -
        m[8] * m[3] * m[6];
        
        inv[11] = -m[0] * m[5] * m[11] +
        m[0] * m[7] * m[9] +
        m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -
        m[8] * m[1] * m[7] +
        m[8] * m[3] * m[5];
        
        inv[15] = m[0] * m[5] * m[10] -
        m[0] * m[6] * m[9] -
        m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] +
        m[8] * m[1] * m[6] -
        m[8] * m[2] * m[5];
        
        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        
        if (det == 0)
            return false;
        
        det = 1.0 / det;
        
        for (i = 0; i < 16; i++)
            invOut[i] = inv[i] * det;
        
        return true;
    }
    
    static void __gluMultMatricesf(const GLfloat a[16], const GLfloat b[16],
                                   GLfloat r[16])
    {
        int i, j;
        
        for (i = 0; i < 4; i++) {
            for (j = 0; j < 4; j++) {
                r[i*4+j] =
                a[i*4+0]*b[0*4+j] +
                a[i*4+1]*b[1*4+j] +
                a[i*4+2]*b[2*4+j] +
                a[i*4+3]*b[3*4+j];
            }
        }
    }
    
    GLint gluProject(GLfloat objx, GLfloat objy, GLfloat objz,
                     const GLfloat modelMatrix[16],
                     const GLfloat projMatrix[16],
                     const GLint viewport[4],
                     GLfloat *winx, GLfloat *winy, GLfloat *winz)
    {
        float in[4];
        float out[4];
        
        in[0]=objx;
        in[1]=objy;
        in[2]=objz;
        in[3]=1.0;
        __gluMultMatrixVecf(modelMatrix, in, out);
        __gluMultMatrixVecf(projMatrix, out, in);
        if (in[3] == 0.0) return(GL_FALSE);
        in[0] /= in[3];
        in[1] /= in[3];
        in[2] /= in[3];
        /* Map x, y and z to range 0-1 */
        in[0] = in[0] * 0.5 + 0.5;
        in[1] = in[1] * 0.5 + 0.5;
        in[2] = in[2] * 0.5 + 0.5;
        
        /* Map x,y to viewport */
        in[0] = in[0] * viewport[2] + viewport[0];
        in[1] = in[1] * viewport[3] + viewport[1];
        
        *winx=in[0];
        *winy=in[1];
        *winz=in[2];
        
        
        return(GL_TRUE);
    }
    
    GLint gluUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
                       const GLfloat * modelMatrix,
                       const GLfloat * projMatrix,
                       const GLint * viewport,
                       GLfloat *objx, GLfloat *objy, GLfloat *objz)
    {
        float finalMatrix[16];
        float in[4];
        float out[4];
        
        __gluMultMatricesf(modelMatrix, projMatrix, finalMatrix);
        if (!__gluInvertMatrixf(finalMatrix, finalMatrix)) return(GL_FALSE);
        
        in[0]=winx;
        in[1]=winy;
        in[2]=winz;
        in[3]=1.0;
        
        /* Map x and y from window coordinates */
        in[0] = (in[0] - viewport[0]) / viewport[2];
        in[1] = (in[1] - viewport[1]) / viewport[3];
        
        /* Map to range -1 to 1 */
        in[0] = in[0] * 2 - 1;
        in[1] = in[1] * 2 - 1;
        in[2] = in[2] * 2 - 1;
        
        __gluMultMatrixVecf(finalMatrix, in, out);
        if (out[3] == 0.0) return(GL_FALSE);
        out[0] /= out[3];
        out[1] /= out[3];
        out[2] /= out[3];
        *objx = out[0];
        *objy = out[1];
        *objz = out[2];
        
        
        return(GL_TRUE);
    }
    
    bool gluInvertMatrix( const GLfloat * m, GLfloat * invOut )
    {
        double inv[16], det;
        int i;
        
        inv[0] = m[5]  * m[10] * m[15] -
        m[5]  * m[11] * m[14] -
        m[9]  * m[6]  * m[15] +
        m[9]  * m[7]  * m[14] +
        m[13] * m[6]  * m[11] -
        m[13] * m[7]  * m[10];
        
        inv[4] = -m[4]  * m[10] * m[15] +
        m[4]  * m[11] * m[14] +
        m[8]  * m[6]  * m[15] -
        m[8]  * m[7]  * m[14] -
        m[12] * m[6]  * m[11] +
        m[12] * m[7]  * m[10];
        
        inv[8] = m[4]  * m[9] * m[15] -
        m[4]  * m[11] * m[13] -
        m[8]  * m[5] * m[15] +
        m[8]  * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];
        
        inv[12] = -m[4]  * m[9] * m[14] +
        m[4]  * m[10] * m[13] +
        m[8]  * m[5] * m[14] -
        m[8]  * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];
        
        inv[1] = -m[1]  * m[10] * m[15] +
        m[1]  * m[11] * m[14] +
        m[9]  * m[2] * m[15] -
        m[9]  * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];
        
        inv[5] = m[0]  * m[10] * m[15] -
        m[0]  * m[11] * m[14] -
        m[8]  * m[2] * m[15] +
        m[8]  * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];
        
        inv[9] = -m[0]  * m[9] * m[15] +
        m[0]  * m[11] * m[13] +
        m[8]  * m[1] * m[15] -
        m[8]  * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];
        
        inv[13] = m[0]  * m[9] * m[14] -
        m[0]  * m[10] * m[13] -
        m[8]  * m[1] * m[14] +
        m[8]  * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];
        
        inv[2] = m[1]  * m[6] * m[15] -
        m[1]  * m[7] * m[14] -
        m[5]  * m[2] * m[15] +
        m[5]  * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];
        
        inv[6] = -m[0]  * m[6] * m[15] +
        m[0]  * m[7] * m[14] +
        m[4]  * m[2] * m[15] -
        m[4]  * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];
        
        inv[10] = m[0]  * m[5] * m[15] -
        m[0]  * m[7] * m[13] -
        m[4]  * m[1] * m[15] +
        m[4]  * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];
        
        inv[14] = -m[0]  * m[5] * m[14] +
        m[0]  * m[6] * m[13] +
        m[4]  * m[1] * m[14] -
        m[4]  * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];
        
        inv[3] = -m[1] * m[6] * m[11] +
        m[1] * m[7] * m[10] +
        m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] -
        m[9] * m[2] * m[7] +
        m[9] * m[3] * m[6];
        
        inv[7] = m[0] * m[6] * m[11] -
        m[0] * m[7] * m[10] -
        m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] +
        m[8] * m[2] * m[7] -
        m[8] * m[3] * m[6];
        
        inv[11] = -m[0] * m[5] * m[11] +
        m[0] * m[7] * m[9] +
        m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -
        m[8] * m[1] * m[7] +
        m[8] * m[3] * m[5];
        
        inv[15] = m[0] * m[5] * m[10] -
        m[0] * m[6] * m[9] -
        m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] +
        m[8] * m[1] * m[6] -
        m[8] * m[2] * m[5];
        
        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
        
        if (det == 0)
            return false;
        
        det = 1.0 / det;
        
        for (i = 0; i < 16; ++i)
            invOut[i] = inv[i] * det;
        
        return true;
    }
    
    
    //
    //  Model convertation tool.
    //
    void model_convert( void )
    {

    }
    
};
