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
//  ShaderLoader.cpp
//  Game shader loader.. :)
//
//  Created by Neko Vision on 06/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "ShaderFactory.h"
#include "../AssetBase.h"
#include "../FileSystem.h"
#include "../../Core/Core.h"
#include "../../Core/String/StringHelper.h"
#include "../../Platform/Shared/System.h"
#include "../../Graphics/OpenGL/OpenGLBase.h"
#include "../../Core/Streams/MemoryStream.h"

// replace this:
#define NEKO_LOG g_Core->p_Console->Print
#define NEKO_ERROR g_Core->p_Console->Error

//   Shader manager.
// @todo binary code support

#   if !defined( NEKO_SERVER )
namespace Neko {
    
    /*
     *  Structure of shader file (.nshdr)
     *
     *  #if _VERTEX_
     *  // store vertex here
     *  #endif
     *
     *  #if _FRAGMENT_
     *  // store fragment code here
     *  #endif
     *
     * .. etc for all types below.
     */
    
    //! We store all shader contents in one file so to group it we use defines.
    static const char VertexDefine[]     = "#define _VERTEX_  \n";
    static const char FragmentDefine[]   = "#define _FRAGMENT_\n";
    
#   if defined( GL_GEOMETRY_SHADER )
    static const char GeometryDefine[]   = "#define _GEOMETRY_\n";
#   endif
    
#   if defined( GL_COMPUTE_SHADER )
    static const char ComputeDefine[]       = "#define _COMPUTE_\n";
#   endif
    
#   if defined( GL_TESS_EVALUATION_SHADER ) && defined( GL_TESS_CONTROL_SHADER )
    static const char TessEvalDefine[]   = "#define _TESS_EVAL_\n";
    static const char TessControlDefine[]   = "#define _TESS_CONTROL_\n";
#   endif
    
    /**
     *  Constructor.
     */
    CShaderFactory::CShaderFactory()
    {
        
    }

    /**
     *  Precache shader stuff.
     */
    void CShaderFactory::Initialize( INekoAllocator * allocator )
    {
        pAllocatorHandle = allocator;
        pAllocator = NekoAllocator::newPoolAllocator( sizeof(SGLShader), __alignof(SGLShader), kShaderCacheSize, *pAllocatorHandle );
        
        // Create shader cache.
        m_pShaderCache.Create( MAX_SHADERS, pAllocator, pLinearAllocator2 );
        
        NEKO_LOG( LOG_INFO, "Shader resource manager loading... (I can load %i shaders!)\n", MAX_SHADERS );
        
        NEKO_LOG( LOG_INFO, "Looking for shader package file..\n" );
        
        // Package containing all our shaders.
        m_pDatafrom = g_Core->p_FileSystem->GetPak( "shared_data" );
    }
    
    /**
     *  Get shader cache.
     */
    SGLShader * CShaderFactory::GetShaderCache( const char * name )
    {
        return m_pShaderCache[name];   // will return NEKO_NULL if not found
    }

    /**
     *  Compile shader.
     */
    GLuint CShaderFactory::Compile( const char * shadername, AssetDataPool *data,
                                   GLenum type, const char *params )
    {
        GLuint      shader;
        
        int32_t     iStatus, iInfoLog = 0;
        uint32_t    iFilesToRead;

        //  1. Defines.
        //  2. Includes ( virtual file systems guesses that we needto load include file and put it into buffer ).
        //  3. Shader code ( vertex, fragment, geometry, etc ).
        char * ShaderContents[8] = {  };
        
        // Create a shader.
        shader = glCreateShader( type );

        // Sanity check.
        if( !shader ) {
            NEKO_ERROR( ERR_OPENGL, "Could not compile \"%s\" shader file.", shadername );
            return 0;
        }
        
        iFilesToRead = 0;
        
        // Define GLSL version.
        const char * ShaderParams = NC_TEXT( "#version %s\n", GLSL_Version->Get<const char*>() );   // 400 core
        if( params && (strlen( params ) > 0) ) {    // Got params to include.
            ShaderParams = NC_TEXT( "#version %s\n%s", GLSL_Version->Get<const char*>(), params );  // 400 core
        }
        
        // Final data.
        ShaderContents[iFilesToRead++] = (char*)ShaderParams;
        
        // We store our all shader code in one file, so check for a "groups".
        const char *ShaderData = "";
        
        switch ( type )
        {
            case GL_VERTEX_SHADER:
                ShaderData = VertexDefine;
                break;
            case GL_FRAGMENT_SHADER:
                ShaderData = FragmentDefine;
                break;
                
#   if defined( GL_GEOMETRY_SHADER )
            case GL_GEOMETRY_SHADER:
                ShaderData = GeometryDefine;
                break;
#   endif
                
#   if defined( GL_COMPUTE_SHADER )
            case GL_COMPUTE_SHADER:
                ShaderData = ComputeDefine;
                break;
#   endif
                
#   if defined( GL_TESS_EVALUATION_SHADER )
            case GL_TESS_EVALUATION_SHADER:
                ShaderData = TessEvalDefine;
                break;
#   endif
                
#   if defined( GL_TESS_CONTROL_SHADER )
            case GL_TESS_CONTROL_SHADER:
                ShaderData = TessControlDefine;
                break;
#   endif
        }
        
        // Shader group define.
        ShaderContents[iFilesToRead++] = (char*)ShaderData;
        
        char    * line;

        // '#include' support, only works in pixel shader now
        
        int32_t iCharsRead = 0, i;
        AssetDataPool pool;
        // Parse shader now.
        // Temporary line for per line parsing.
        line = (char*)PushMemory( data->tempPool, sizeof(char) * 128 );
        
        // Read line by line.
        if( type == GL_FRAGMENT_SHADER ) {  // TODO: include support for all shader types.
            while( str_readline( line, 256, (const char*)data->tempData, iCharsRead ) ) {
                // Look for include directives.
                if( NekoCString::CompareRange( line, "#include", 8 ) ) {
                    NekoCString::TokenizeString( line );
                    
                    // Remove the line from stream for further parsing without errors.
                    for( i = iCharsRead - (uint32_t)strlen(line); i < iCharsRead; ++i  ) {
                        data->tempData[i] = ' ';
                    }
                    
                    pool = m_pDatafrom->GetData( NC_TEXT( "shaders/%s", NekoCString::lastCommandArguments[1] ), NEKO_NULL, true );
                    
                    if( pool.tempData == NEKO_NULL ) {
                        NEKO_ERROR( ERR_ASSET, "Couldn't find \"%s\" include file in \"%s\"\n", NekoCString::lastCommandArguments[1], shadername );
                        // ...
                        return 0;
                    }
                    
                    ShaderContents[iFilesToRead++] = (char *)pool.tempData;
                    
                    // Don't forget to!
                    _PopMemoryFrame( pool.tempPool );
                }
            }
        }
        
        // Actual shader data.
        ShaderContents[iFilesToRead++] = (char*)data->tempData;
    
        // - ------------
        
        // Set shader source.
        glShaderSource( shader, iFilesToRead, ShaderContents, NEKO_NULL );

        // Try to compile shader now.
        glCompileShader( shader );

        // Query shader compile status.
        glGetShaderiv( shader, GL_COMPILE_STATUS, &iStatus );

        // Something went wrong.
        if( !iStatus ) {
            glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &iInfoLog );
            
            if( iInfoLog > 1 ) {
                char *error_log = (char*)PushMemory( data->tempPool, sizeof(char) * (iInfoLog + 1) );

                glGetProgramInfoLog( shader, 1024, NEKO_NULL, error_log );

                NEKO_ERROR( ERR_FATAL, "Could not compile %s shader. Here's log:\n", shadername, error_log );

//                Neko::PopMemoryFrame( data->tempPool );
                
                glDeleteShader( shader );

                return 0;
            } else {
                NEKO_ERROR( ERR_ASSET, "Could not compile shader %s.\n No log available.\n", shadername );

//                Neko::PopMemoryFrame( data->tempPool );
                
                glDeleteShader( shader );
                
                return 0;

            }
        }

        return shader;
    }

    /**
     *  Load shader from file.
     */
    void CShaderFactory::CompileFromFile( const char * file, GLuint &vs, GLuint &fs, GLuint &gs, GLuint &cs, GLuint &tesseval, GLuint &tesscontrol, uint32_t flags, const char *params )
    {
        AssetDataPool data;

        // Look up for shader data (simply load file string data).
        data = m_pDatafrom->GetData( NC_TEXT( "shaders/%s.nshdr", file), NEKO_NULL, true );
        if( data.tempData == NEKO_NULL ) {
            NEKO_ERROR( ERR_FILESYSTEM, "No \"%s\" shader found.\n", file );
            return;
        }
        
        // Compile vertex shader.
        if( flags & (uint32_t)EShaderCompileCaps::Vertex ) {
            vs = Compile( file, &data, GL_VERTEX_SHADER, params );
        } else {
            vs = 0;
        }
        
#   if defined( GL_TESS_EVALUATION_SHADER ) && defined( GL_TESS_CONTROL_SHADER )
        // Compile tesselation evaluation shader.
        if( flags & (uint32_t)EShaderCompileCaps::TessEval ) {
            tesseval = Compile( file, &data, GL_TESS_EVALUATION_SHADER, params );
        } else {
            tesseval = 0;
        }
        
        // Compile tesselation control shader.
        if( flags & (uint32_t)EShaderCompileCaps::TessControl ) {
            tesscontrol = Compile( file, &data, GL_TESS_CONTROL_SHADER, params );
        } else {
            tesscontrol = 0;
        }
#   endif
        
        
        // Compile fragment shader.
        if( flags & (uint32_t)EShaderCompileCaps::Fragment ) {
            fs = Compile( file, &data, GL_FRAGMENT_SHADER, params );
            
        } else {
            fs = 0;
        }
   
#   if defined( GL_GEOMETRY_SHADER )
        
        // Compile geometry shader.
        if( flags & (uint32_t)EShaderCompileCaps::Geometry ) {
            gs = Compile( file, &data, GL_GEOMETRY_SHADER, params );
            
        } else {
            gs = 0;
        }
#   else
        gs = 0;
        
#   endif
        
        
#   if defined( GL_COMPUTE_SHADER )
        
        // Compile compute shader.
        if( flags & (uint32_t)EShaderCompileCaps::Compute ) {
            cs = Compile( file, &data, GL_COMPUTE_SHADER, params );
            
        } else {
            cs = 0;
        }
#   else
        cs = 0;
        
#   endif
        
        // Close the temp memory now!
        _PopMemoryFrame( data.tempPool );
        data.tempData = NEKO_NULL;
        data.tempPool = NEKO_NULL;
    }

    /**
     *  Simply load shader.
     */
    void CShaderFactory::Load( const char *file, uint16_t flags )
    {
        Load( file, NEKO_NULL, flags );
    }

    /**
     *  Simply load shader with parameters.
     */
    void CShaderFactory::Load( const char *file, const char *params, uint32_t flags )
    {
        GLuint      program, vs = 0, fs = 0, gs = 0, cs = 0, tesseval = 0, tesscontrol = 0;
        int32_t     err, infolen;

        CompileFromFile( file, vs, fs, gs, cs, tesseval, tesscontrol, flags, params );

        program = glCreateProgram();

        if( vs /* Vertex shader. */ ) {
            glAttachShader(program, vs);
        }
        
        if( tesseval /* Tesselation evaluation shader. */ ) {
            glAttachShader(program, tesseval);
        }
        
        if( tesscontrol /* Tesselation control shader. */ ) {
            glAttachShader(program, tesscontrol);
        }
        
        if( gs /* Geometry shader. */ ) {
            glAttachShader(program, gs);
        }
        
        if( fs /* Fragment(pixel) shader. */ ) {
            glAttachShader(program, fs);
        }
        
        if( cs /* Compute shader. */ ) {
            glAttachShader(program, cs);
        }
        
        // Should be moved.
        // TODO: Ignore list.
        if( !strcmp( file, "particleupdate" ) ) {
            uint32_t    i;
            
            const static char* sVaryings[6] = {
                "vPositionOut",
                "vVelocityOut",
                "vColorOut",
                "fLifeTimeOut",
                "fSizeOut",
                "iTypeOut",
            };
            
            for( i = 0; i < 6; ++i ) {
                glTransformFeedbackVaryings( program, 6, sVaryings, GL_INTERLEAVED_ATTRIBS );
            }
        }
        
        if( !strcmp( file, "cull" ) ) {
            NEKO_LOG( LOG_INFO, "cull shader-> registering feedback varyings..\n" );
            
            // bind output varyings
            const char *vars[] = {
                "InstPosLOD0",
                "gl_NextBuffer",
                "InstPosLOD1",
                "gl_NextBuffer",
                "InstPosLOD2"
            };
            glTransformFeedbackVaryings( program, 5, vars, GL_INTERLEAVED_ATTRIBS );
        }
        
        // - -------
        
        // Try to link the shader program.
        glLinkProgram( program );
        glGetProgramiv( program, GL_LINK_STATUS, &err );

        if( !err ) {
            infolen = 0;
            glGetProgramiv( program, GL_INFO_LOG_LENGTH, &infolen );

            if( infolen > 1 ) {
                // lazy to use Neko allocator here
                char * log_string = new char[infolen + 1];

                glGetProgramInfoLog( program, infolen, NEKO_NULL, log_string );

                // Tell us.
                g_Core->p_Console->Error( ERR_FATAL, "Could not link \"%s\" shader. Error: %s\n", file, log_string );

                delete [] log_string;

                return;
            }
        } else {
            // Create a new shader and add it to shader factory!
            SGLShader * s_temp = (SGLShader *)pAllocator->Alloc( sizeof( SGLShader ) );
            s_temp->SetProperties( program, file );
            
            s_temp->Vertex = vs;
            s_temp->Fragment = fs;
            s_temp->Geom = gs;
            s_temp->Compute = cs;
            s_temp->TessEval = tesseval;
            s_temp->TessControl = tesscontrol;

            m_pShaderCache[file] = s_temp;
            
            NEKO_LOG( LOG_INFO, "LoadShader(): %s loaded..  \n", file/*, t2-t1*/ );
        }
    }

    /**
     *  Remove the shader.
     */
    void CShaderFactory::Delete( SGLShader *shader )
    {
        shader->~SGLShader();
    }

    /**
     *  Unload shaders.
     */
    void CShaderFactory::UnloadShaders()
    {
        NEKO_LOG( "Unloading all shaders...\n" );
        
        SGLShader * shader = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_pShaderCache.m_List.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            shader = (SGLShader *)cur->m_ptrData;
            // shader can't be null here
            
            // Delete texture.
            shader->Delete();
            
            pAllocator->Dealloc( shader );
        }
        
        // Delete shader cache.
        m_pShaderCache.Delete();
    }
    
    /**
     *  Mostly called on application shutdown.
     */
    void CShaderFactory::Shutdown( void )
    {
        NEKO_LOG( "Shader factory shutting down..\n" );
        
        UnloadShaders();
        
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pAllocator, pAllocatorHandle );
    }
}
#   endif
