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
//  GLShader.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 12/2/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef GLShader_hpp
#define GLShader_hpp

#include "../../Core/Core.h"
#include "../../Core/APIObject.h"
#include "..//RendererMisc.h"
#include "../../Math/Vec4.h"
#include "../../Math/Vec3.h"
#include "../../Math/Vec2.h"

namespace Neko {
    
    ///  GLSL shader.
    class SGLShader : public APIObject
    {
    public:
        
        /**
         *  Constructors.
         */
        SGLShader();
        SGLShader( GLint _program, const char *_name );
        
        ~SGLShader();
        
        /**
         *  Set shader properties.
         */
        void                SetProperties( GLint _program, const char *_name );
        
        /**
         *  Delete shader.
         */
        void                Delete();
        
        /**
         *  Use shader.
         */
        NEKO_FORCE_INLINE void                 Use() const {   glUseProgram( m_APIhandle ); }
        
        /**
         *  Get uniform location.
         */
        NEKO_FORCE_INLINE GLint UniformLocation( const char * uniname )
        {
            return glGetUniformLocation( m_APIhandle, uniname );
        }
        
        /**
         *  Get uniform block index.
         */
        NEKO_FORCE_INLINE GLint UniformBlockIndex( const char * uniname )
        {
            GLint result = 0;
            GLint uniform_block_index = glGetUniformBlockIndex( m_APIhandle, uniname );
            // assign the block binding
            glUniformBlockBinding( m_APIhandle, uniform_block_index, result);
            return result;
        }
        
        /**
         *  Get shader subroutine index by name.
         */
        GLint               GetSubroutineIndex( const char * name, EShaderType type );
        
        // glGetUniformLocation included.
        
        /***
         *
         *   Floats.
         *
         **/
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLfloat value )
        {
            GLint location = UniformLocation( uniname );
            glUniform1f( location, value );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLfloat v0, GLfloat v1 )
        {
            GLint location = UniformLocation( uniname );
            glUniform2f( location, v0, v1 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLfloat v0, GLfloat v1, GLfloat v2 )
        {
            GLint location = UniformLocation( uniname );
            glUniform3f( location, v0, v1, v2 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
        {
            GLint location = UniformLocation( uniname );
            glUniform4f( location, v0, v1, v2, v3 );
        }
        
        NEKO_FORCE_INLINE void SetUniform1fv( const char * uniname, GLsizei count, const GLfloat *values )
        {
            GLint location = UniformLocation( uniname );
            glUniform1fv( location, count, values );
        }
        
        NEKO_FORCE_INLINE void SetUniform2fv( const char * uniname, GLsizei count, const GLfloat *values )
        {
            GLint location = UniformLocation( uniname );
            glUniform2fv( location, count, values );
        }
        
        NEKO_FORCE_INLINE void SetUniform3fv( const char * uniname, GLsizei count, const GLfloat *values )
        {
            GLint location = UniformLocation( uniname );
            glUniform3fv( location, count, values );
        }
        
        
        /***
         *
         *   Integers.
         *
         **/
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLint value )
        {
            GLint location = UniformLocation( uniname );
            glUniform1i( location, value );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLint v0, GLint v1 )
        {
            GLint location = UniformLocation( uniname );
            glUniform2i( location, v0, v1 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLint v0, GLint v1, GLint v2 )
        {
            GLint location = UniformLocation( uniname );
            glUniform3i( location, v0, v1, v2 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLint v0, GLint v1, GLint v2, GLint v3 )
        {
            GLint location = UniformLocation( uniname );
            glUniform4i( location, v0, v1, v2, v3 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLsizei count, const GLint *values )
        {
            GLint location = UniformLocation( uniname );
            glUniform1iv( location, count, values );
        }
        
        
        /***
         *
         *   Matrices.
         *
         **/
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, GLsizei count, GLboolean transpose, const GLfloat *values )
        {
            GLint location = UniformLocation( uniname );
            glUniformMatrix4fv( location, count, transpose, values );
        }
        
        //! Advanced.
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, const Vec3& to )
        {
            GLint location = UniformLocation( uniname );
            glUniform3f( location, to.x, to.y, to.z );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, const vec3_template<int>& to )
        {
            GLint location = UniformLocation( uniname );
            glUniform3i( location, to.x, to.y, to.z );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, const vec2_template<float>& to ) {
            GLint location = UniformLocation( uniname );
            glUniform2f( location, to.x, to.y );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, const vec2_template<int>& to )
        {
            GLint location = UniformLocation( uniname );
            glUniform2i( location, to.x, to.y );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, const Vec4& to )
        {
            GLint location = UniformLocation( uniname );
            glUniform4f( location, to.x, to.y, to.z, to.w );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * uniname, const Vec4i& to )
        {
            GLint location = UniformLocation( uniname );
            glUniform4i( location, to.x, to.y, to.z, to.w );
        }
        
        // glGetUniformLocation not included.
        // Floats.
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLfloat value )
        {
            glUniform1f( loc, value );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLfloat v0, GLfloat v1 )
        {
            glUniform2f( loc, v0, v1 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLfloat v0, GLfloat v1, GLfloat v2 )
        {
            glUniform3f( loc, v0, v1, v2 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 )
        {
            glUniform4f( loc, v0, v1, v2, v3 );
        }
        NEKO_FORCE_INLINE void SetUniform1fv( GLint loc, GLsizei count, const GLfloat *values )
        {
            glUniform1fv( loc, count, values );
        }
        
        NEKO_FORCE_INLINE void SetUniform2fv( GLint loc, GLsizei count, const GLfloat *values )
        {
            glUniform2fv( loc, count, values );
        }
        
        NEKO_FORCE_INLINE void SetUniform3fv( GLint loc, GLsizei count, const GLfloat *values )
        {
            glUniform3fv( loc, count, values );
        }
        
        // Integers.
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLint value )
        {
            glUniform1i( loc, value );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLint v0, GLint v1 )
        {
            glUniform2i( loc, v0, v1 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLint v0, GLint v1, GLint v2 )
        {
            glUniform3i( loc, v0, v1, v2 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLint v0, GLint v1, GLint v2, GLint v3 )
        {
            glUniform4i( loc, v0, v1, v2, v3 );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLsizei count, const GLint *values )
        {
            glUniform1iv( loc, count, values );
        }
        
        
        // Matrices.
        NEKO_FORCE_INLINE void SetUniform( GLint loc, GLsizei count, GLboolean transpose, const GLfloat *values )
        {
            glUniformMatrix4fv( loc, count, transpose, values );
        }
        
        
        // Custom.
        NEKO_FORCE_INLINE void SetUniform( GLint loc, const Vec3 &to )
        {
            glUniform3f( loc, to.x, to.y, to.z );
        }

        NEKO_FORCE_INLINE void SetUniform( GLint loc, const Vec3 &to, const GLfloat& w )
        {
            glUniform4f( loc, to.x, to.y, to.z, w );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, const vec3_template<int> &to )
        {
            glUniform3i( loc, to.x, to.y, to.z );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, const Vec4 &to )
        {
            glUniform4f( loc, to.x, to.y, to.z, to.w );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, const Vec4i &to )
        {
            glUniform4i( loc, to.x, to.y, to.z, to.w );
        }

        NEKO_FORCE_INLINE void SetUniform( GLint loc, const vec2_template<float> &to )
        {
            glUniform2f( loc, to.x, to.y );
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, const vec2_template<int> &to )
        {
            glUniform2i( loc, to.x, to.y );
        }

        NEKO_FORCE_INLINE void SetUniform( GLint loc, Vec3 * vectors, int count = 1)
        {
            glUniform2fv( loc, count, (GLfloat*)vectors );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * name, Vec3 * vectors, int count = 1 )
        {
            int iLoc = glGetUniformLocation( GetHandle(), name );
            glUniform2fv(iLoc, count, (GLfloat*)vectors);
        }
        
        NEKO_FORCE_INLINE void SetUniform( GLint loc, ncMatrix4 * matrices, int count = 1 )
        {
            glUniformMatrix4fv( loc, count, false, (const GLfloat*)matrices );
        }
        
        NEKO_FORCE_INLINE void SetUniform( const char * name, ncMatrix4 * matrices, int count = 1 )
        {
            int iLoc = glGetUniformLocation( GetHandle(), name );
            glUniformMatrix4fv(iLoc, count, false, (const GLfloat*)matrices);
        }
        
        
        /**
         *  Stop using shader.
         */
        NEKO_FORCE_INLINE void             Next() const    {       glUseProgram( 0 );  }
        
        /**
         *  Get shader id.
         */
        NEKO_FORCE_INLINE const GLuint               GetId() const {       return m_APIhandle;  }
        
        /**
         *  Get shader name.
         */
        NEKO_FORCE_INLINE const CStr &                 GetName() const {       return Name;    }
        
//    private:
        
        GLuint Vertex, Fragment, Geom, Compute, TessEval, TessControl;
        
        //!  Shader name.
        CStr Name;
    };

}

#endif /* GLShader_hpp */
