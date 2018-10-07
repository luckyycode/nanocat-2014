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
//  GLShader.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 12/2/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "GLShader.h"
#include "../GraphicsManager.h"

namespace Neko {
   
    /**
     *  Empty shader.
     */
    SGLShader::SGLShader()
    {
 
    }
    
    /**
     *  New shader.
     */
    SGLShader::SGLShader( GLint _program, const char *_name )
    {
        SetProperties( _program, _name );
    }
    
    /**
     *  Get shader subroutine index by name.
     */
    GLint SGLShader::GetSubroutineIndex( const char *name, EShaderType type )
    {
        GLint result = 0;
        GLint shdr = (GLint)g_pGraphicsManager->GetCurrentInterface()->GetAPIShaderType( type );
        
        result = glGetSubroutineIndex( GetHandle(), shdr, name );
        
        return result;
    }

    /**
     *  Set new shader properties.
     */
    void SGLShader::SetProperties( GLint _program, const char *_name )
    {
        SetHandle( _program );
        
        this->Name = _name;
    }
    
    /**
     *  Delete shader.
     */
    SGLShader::~SGLShader() {
        Delete();
    }
    
    /**
     *  Delete shader.
     */
    void SGLShader::Delete()
    {
        GLint handle;
        
        handle = GetHandle();
        
        // Delete shader.
        glDeleteProgram( handle );
        
        if( Vertex ) {
            glDeleteShader( Vertex );
        }
        
        if( Fragment ) {
            glDeleteShader( Fragment );
        }
        
        if( Geom ) {
            glDeleteShader( Geom );
        }
        
        if( Compute ) {
            glDeleteShader( Compute );
        }
        
        if( TessEval ) {
            glDeleteShader( TessEval );
        }
        
        if( TessControl ) {
            glDeleteShader( TessControl );
        }
    }
}
