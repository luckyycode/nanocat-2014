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
//  ShaderLoader.h
//  Shader manager.. :)
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef shaderloader_h
#define shaderloader_h

#include "../../Platform/Shared/SystemShared.h"
#include "../../Math/GameMath.h"
#include "../../Core/String/String.h"
#include "../../Core/Utilities/Hashtable.h"
#include "../FileSystem.h" // File loading.
#include "../../Graphics/OpenGL/GLShader.h"

//  Shader loader and manager.

#   if !defined( NEKO_SERVER )
namespace Neko {
    
    //!  Maximum shader amount allowed to load.
    static const int32_t MAX_SHADERS = 24;

    /// Shader compile flags.
    enum class EShaderCompileCaps : int32_t
    {
        Fragment        = 1 << 0,
        Vertex          = 1 << 1,
        Geometry        = 1 << 2,
        TessEval        = 1 << 3,
        TessControl     = 1 << 4,
        Compute         = 1 << 5,
        
        Dummy   = 1 << 6
    };
    
    ///   Shader manager.
    class CShaderFactory
    {
        NEKO_NONCOPYABLE( CShaderFactory );
        
    public:
        
        /**
         *  Constructor.
         */
        CShaderFactory();
        
        /**
         *  Destructor.
         */
        ~CShaderFactory() { }

        /**
         *  Initialize shader manager.
         */
        void                Initialize( INekoAllocator * allocator );

        /**
         *  Compile teh shader.
         */
        GLuint                  Compile( const char * shadername, AssetDataPool *data, GLenum type, const char *params = NEKO_NULL );

        /**
         *  Compile shader from file.
         */
        void                    CompileFromFile( const char * file, GLuint &vs, GLuint &fs, GLuint &gs, GLuint &cs, GLuint &tesseval,GLuint &tesscontrol, uint32_t flags, const char *params = NEKO_NULL );
        
        /**
         *  Load shader with parameters.
         */
        void                    Load( const char *file, const char *params, uint32_t flags );
        
        /**
         *  Load shader.
         */
        void                    Load( const char *file, uint16_t flags );
        
        /**
         *  Delete shader.
         */
        void                    Delete( SGLShader *shader );
        
        /**
         *  Shutdown teh shader.
         */
        void                    Shutdown( void );
        
        /**
         *  Unload shaders.
         */
        void                    UnloadShaders( void );
        
        /**
         *  Get shader cache.
         */
        SGLShader   *               GetShaderCache( const char * name );
        
//    private:
        
        INekoAllocator  * pAllocator;
        INekoAllocator  * pAllocatorHandle;
        
        //!  Shader cache.
        CHashMap<const char*, SGLShader*>   m_pShaderCache;
        
        //!  Shader package file.
        SPackFile  * m_pDatafrom;
    };
}

#   endif // NEKO_SERVER

#endif
