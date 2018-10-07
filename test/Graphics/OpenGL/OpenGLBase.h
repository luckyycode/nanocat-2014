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
//  OpenGLBase.h
//  OpenGL manager..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef _neko_opengl_h
#define _neko_opengl_h

#define BUFFER_OFFSET(a) ((char*)NEKO_NULL + (a))
#define IBO_OFFSET(a) BUFFER_OFFSET((a) * sizeof(GLuint))

#include "../../Core/Console/ConsoleVariable.h"
#include "../../Core/Core.h"
#include "../GraphicsInterface.h"

//#ifdef DARWIN
//    #ifndef GL_TESS_CONTROL_SHADER
//        #define GL_TESS_CONTROL_SHADER 0x00008e88
//    #endif
//    #ifndef GL_TESS_EVALUATION_SHADER
//        #define GL_TESS_EVALUATION_SHADER 0x00008e87
//    #endif
//    #ifndef GL_PATCHES
//        #define GL_PATCHES 0x0000000e
//    #endif
//#endif

namespace Neko {

    ///  OpenGL context & helper methods.
    class COpenGLAPI : public GraphicsInterface
    {
        NEKO_NONCOPYABLE( COpenGLAPI );
        
    public:
        
        /**
         *  Constructor.
         */
        COpenGLAPI();
        
        /**
         *  Destructor.
         */
        virtual ~COpenGLAPI() {
            
        }

        /**
         *  Initialize OpenGL base.
         */
        virtual void                Initialize( void );
        
        /**
         *  Action on resize.
         *
         *  @param w A new width value.
         *  @param h A new height value.
         */
        virtual void                OnResize( const int32_t w, const int32_t h );
        
        /**
         *  When graphics interface got loaded..
         */
        virtual void                OnLoad();
        
        /**
         *  Show driver information.
         */
        void                ShowInfo( void );
        
        /**
         *  Shutdown the interface.
         */
        virtual void                Shutdown();
        
        /**
         *  Check API reported errors.
         */
        virtual void                CheckAPIError( const char * where );
        
        /**
         *  Get shader type.
         */
        virtual uint32_t                GetAPIShaderType( EShaderType type );
        
        /**
         *  Get primitive type for current API.
         */
        virtual uint32_t                GetAPIPrimitiveType( EPrimitiveType type );
        
        /**
         *  Get buffer type for current API.
         */
        virtual uint32_t                GetAPIBufferType( EBufferType type );
        
        /**
         *  Get buffer storage type for current API.
         */
        virtual uint32_t                    GetAPIBufferStorageType( EBufferStorageType type );
        
        /**
         *  Get texture target type for current API.
         */
        virtual uint32_t                    GetAPITextureTarget( TextureTarget type );
        
        /**
         *  Get API corresponding tile.
         */
        virtual uint32_t                    GetAPITextureTile( ETextureTile type );
        
        /**
         *  Create texture.
         */
        virtual uint32_t                    CreateTexture( SImageInfo * info );
        
        /**
         *  Generate mipmaps for texture.
         */
        virtual void                        GenerateMipmaps( const uint32_t texId, const uint32_t target );
        
        /**
         *  Get storage type.
         */
        virtual uint32_t                    GetAPIStorageType( ETextureStorageType type );
        
        /**
         *  Get color type.
         */
        virtual uint32_t                    GetAPITextureFormat( ETextureFormat type );
        
        /**
         *  Get texture filtering mode.
         */
        virtual uint32_t                    GetAPITextureFilteringMode( ETextureFilter type );
        
        /**
         *  Get compare mode.
         */
        virtual uint32_t                GetAPICompareMode( ECompareMode type );
        
        /**
         *  Get color target.
         */
        virtual uint32_t                GetAPIRenderTarget( ERenderTarget target );
        
        /**
         *  Get cull mode.
         */
        virtual uint32_t                GetAPICullMode( ECullMode type );
        
        /**
         *  Get blend mode.
         */
        virtual uint32_t *                GetAPIBlendMode( EBlendMode type );
        
        
        const uint32_t              GetMajorVersion( void ) const;
        const uint32_t              GetMinorVersion( void ) const;

    private:
        
        int32_t     m_majorVersion;
        int32_t     m_minorVersion;
        
        int32_t     m_iMaxAnisotropy;
    };
    
    // Console variables.
    extern SConsoleVar      * GLSL_Version;                              // Game GLSL version to use.

	// Windows OpenGL extensions.
	// ( Since MS are sillylazy )
#ifdef _WIN32
	typedef BOOL(WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats,
		int *piFormats, UINT *nNumFormats);
	typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
	typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);

	extern PFNWGLCHOOSEPIXELFORMATARBPROC       wglChoosePixelFormatARB;
	extern PFNWGLCREATECONTEXTATTRIBSARBPROC    wglCreateContextAttribsARB;
	extern PFNWGLSWAPINTERVALEXTPROC            wglSwapIntervalEXT;
#endif
}

#endif
