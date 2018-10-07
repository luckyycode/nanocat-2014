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
//  Utils.h
//  Utilities for application.
//
//  Created by Neko Code on 9/12/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef Utils_h
#define Utils_h

#include "../../Math/GameMath.h"

using namespace Neko;
namespace NekoUtils {

#define noiseWidth 192
#define noiseHeight 192
#define noiseDepth 64
    
    namespace NoiseOperations
    {
        double turbulence( double x, double y, double z, double size, double * noise );
        double smoothNoise(double x, double y, double z, double * noise);
    }
    
    /**
     *  Spline cubic expression.
     */
    struct Cubic
    {
    public:
        float a, b, c, d;
        
        Cubic() : a(0.0f), b(0.0f), c(0.0f), d(0.0f)
        {
            
        }
        
        inline Cubic( float a, float b, float c, float d )
        {
            this->a = a;
            this->b = b;
            this->c = c;
            this->d = d;
        }
        
        inline float eval( float u )
        {
            return (((d*u) + c)*u + b)*u + a;
        }
    };
    
    
    unsigned char Nibble(char c);

    
    /**
     *  Calculate natural cubic expression.
     */
    Cubic * calcNaturalCubic(int n, float * x);
    
    
    /**
     * Parse CSV...
     */
    size_t ParseCSV( char *line, char *list[], size_t size, const char delimiter = ',' );
    // ------------

    /**
     *  Standart Hex encode/decode.
     */
    char * HexEncode( const char * data, unsigned int data_size );
    bool HexCharDecode( char hexdigit, Byte &value );

    const Vec3& RayFromMousePos( ncMatrix4 m_modelView, int x, int y );

    /**
     *  GLU library functions.
     */
    GLint gluUnProject( GLfloat winx, GLfloat winy, GLfloat winz, const GLfloat * modelMatrix, const GLfloat * projMatrix, const GLint * viewport, GLfloat *objx, GLfloat *objy, GLfloat *objz);
    GLint gluProject( GLfloat objx, GLfloat objy, GLfloat objz, const GLfloat modelMatrix[16], const GLfloat projMatrix[16], const GLint viewport[4], GLfloat *winx, GLfloat *winy, GLfloat *winz);
    
    
    bool		LineIntersect1f( float start, float axisdir, float min, float max, float &enter, float &exit );
    
    /**
     *  Invert matrix.
     */
    bool gluInvertMatrix( const GLfloat * m, GLfloat * invOut );
    
    /**
     *  Little long.
     */
    int LittleLong( int l );
};


#endif
