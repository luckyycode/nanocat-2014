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
//  OpenGLBase.cpp
//  OpenGL manager..
//
//  Created by Neko Vision on 05/09/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//
// Edit on 12/27/15 - Added universal enumerator types and getters ( GetAPI* ).
// Edit on 02/19/16 - Added proper shutdown function.

#include "../../Core/Core.h"
#include "../../Core/Player/Camera/Camera.h"
#include "../../Core/Player/Input/Input.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Core/String/StringHelper.h"
#include "OpenGLBase.h"

namespace Neko {

    //! Shading language version.
    SConsoleVar  * GLSL_Version = 0;

    // yes
    // well...

    COpenGLAPI::COpenGLAPI() : m_majorVersion(0), m_minorVersion(0) {

    }

    /**
     *  Called when API has loaded.
     */
    void COpenGLAPI::OnLoad()
    {
        m_bInitialized = true;
        GraphicsInterface::OnLoad();
    }
    
    /**
     *  Get primitive type for the current API.
     */
    uint32_t COpenGLAPI::GetAPIPrimitiveType( EPrimitiveType type )
    {
        switch( type ) {
            case EPrimitiveType::Triangles:     return GL_TRIANGLES;
            case EPrimitiveType::TriangleStrip: return GL_TRIANGLE_STRIP;
            case EPrimitiveType::Lines:         return GL_LINES;
            case EPrimitiveType::Points:        return GL_POINTS;
            case EPrimitiveType::LineStrip:     return GL_LINE_STRIP;
                
            default:    return 0;
        }
        
        return 0;
    }
    
    /**
     *  Get buffer type for the current API.
     */
    uint32_t COpenGLAPI::GetAPIBufferType( EBufferType type )
    {
        switch( type ) {
            case EBufferType::Dynamic:  return GL_DYNAMIC_DRAW;
            case EBufferType::Static:   return GL_STATIC_DRAW;
            case EBufferType::Stream:   return GL_STREAM_DRAW;
                
            default:
                return 0;
        }
        
        return 0;
    }
    
    /**
     *  Get buffer type for the current API.
     */
    uint32_t COpenGLAPI::GetAPIBufferStorageType( EBufferStorageType type )
    {
        switch( type ) {
            case EBufferStorageType::Array:         return GL_ARRAY_BUFFER;
            case EBufferStorageType::IndexArray:    return GL_ELEMENT_ARRAY_BUFFER;
            case EBufferStorageType::Uniform:       return GL_UNIFORM_BUFFER;;
                
            default:
                return 0;
        }
        
        return 0;
    }
    
    /**
     *  Get texture target type.
     */
    uint32_t COpenGLAPI::GetAPITextureTarget( TextureTarget type )
    {
        // Order is very important! See "ETextureTarget".
        static const uint32_t valuesMap[StaticGraphicsInfo::MaxTextureTargets] =
        {
            GL_TEXTURE_2D,
            GL_TEXTURE_3D,
            
#if defined(GL_TEXTURE_2D_ARRAY)
            GL_TEXTURE_2D_ARRAY,
#else
            GL_TEXTURE_2D,
#endif
            
#if defined(GL_TEXTURE_RECTANGLE)
            GL_TEXTURE_RECTANGLE,
#else
            GL_TEXTURE_2D,
#endif
            
            GL_TEXTURE_CUBE_MAP,
        };
        
        return valuesMap[uint32_t(type)];
    }
    
    /**
     *  Get texture type for corresponding API.
     */
    uint32_t COpenGLAPI::GetAPITextureTile( ETextureTile type )
    {
        switch ( type ) {
            case ETextureTile::Repeat:          return GL_REPEAT;
            case ETextureTile::Clamp:           return GL_CLAMP_TO_BORDER;
            case ETextureTile::ClampToEdge:     return GL_CLAMP_TO_EDGE;
            case ETextureTile::MirrorRepeat:    return GL_MIRRORED_REPEAT;
                
            default:
                CCore::Assert( "GetAPITextureTile(OpenGL): Undefined type." );
                break;
        }
        
        return GL_REPEAT; // By default.
    }
    
    /**
     *  Get compare type for corresponding API.
     */
    uint32_t COpenGLAPI::GetAPICompareMode( ECompareMode type )
    {
        static const uint32_t valuesMap[StaticGraphicsInfo::MaxCompareModes] =
        {
            GL_NONE,
            
            GL_EQUAL,
            GL_LESS,
            GL_LEQUAL,
            GL_GEQUAL,
            GL_GREATER,
            GL_ALWAYS
        };
        
        uint32_t intValue = static_cast<uint32_t>(type);
        return valuesMap[intValue];
    }

    /**
     *  Get culling mode for corresponding API.
     */
    uint32_t* COpenGLAPI::GetAPIBlendMode( EBlendMode mode )
    {
        //!  Blending modes.
        static const uint32_t blendModesMap[StaticGraphicsInfo::MaxBlendModes][2] =
        {
            { GL_ONE, GL_ZERO }, // Disabled,
            { GL_ONE, GL_ZERO }, // Current,
            { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }, // Default,
            { GL_ONE, GL_ONE_MINUS_SRC_ALPHA}, // AlphaPremultiplied,
            { GL_ONE, GL_ONE }, // Additive,
            { GL_SRC_ALPHA, GL_ONE }, // AlphaAdditive,
            { GL_ZERO, GL_SRC_ALPHA }, // AlphaMultiplicative,
            { GL_SRC_COLOR, GL_ONE}, // ColorAdditive,
            { GL_ZERO, GL_ONE_MINUS_SRC_ALPHA } // AlphaInverseMultiplicative
        };
        
        // API request.
        uint32_t intValue = static_cast<uint32_t>(mode);
        return (uint32_t*)blendModesMap[intValue];
    }
    
    
    /**
     *  Get culling mode for corresponding API.
     */
    uint32_t COpenGLAPI::GetAPICullMode( ECullMode mode )
    {
        static const uint32_t valuesMap[StaticGraphicsInfo::MaxCullModes] =
        {
            GL_FRONT,
            GL_BACK
        };
        
        uint32_t intValue = static_cast<uint32_t>(mode);
        return valuesMap[intValue];
    }
    
    /**
     *  Get color target.
     */
    uint32_t COpenGLAPI::GetAPIRenderTarget( ERenderTarget target )
    {
        static const uint32_t valuesMap[StaticGraphicsInfo::MaxRenderTargetsHW] =
        {
            GL_NONE,
            
            GL_COLOR_ATTACHMENT0,
            GL_COLOR_ATTACHMENT1,
            GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3,
#   if defined(GL_COLOR_ATTACHMENT4)
            GL_COLOR_ATTACHMENT4,
#   else
            0,
#   endif
#   if defined(GL_COLOR_ATTACHMENT5)
            GL_COLOR_ATTACHMENT5,
#   else
            0,
#   endif
#   if defined(GL_COLOR_ATTACHMENT6)
            GL_COLOR_ATTACHMENT6,
#   else
            0,
#   endif
#   if defined(GL_COLOR_ATTACHMENT7)
            GL_COLOR_ATTACHMENT7,
#   else
            0,
#   endif
        };
        
        uint32_t intValue = static_cast<uint32_t>(target);
        return valuesMap[intValue];
    }
    
    /**
     *  Get shader type for corresponding API.
     */
    uint32_t COpenGLAPI::GetAPIShaderType( EShaderType type )
    {
        static const uint32_t valuesMap[StaticGraphicsInfo::MaxShaderTypes] =
        {
            GL_VERTEX_SHADER,
            GL_FRAGMENT_SHADER,
            
#   if defined(GL_GEOMETRY_SHADER)
            GL_GEOMETRY_SHADER,
#   else
            0,
#   endif
            
#   if defined(GL_TESS_EVALUATION_SHADER)
            GL_TESS_EVALUATION_SHADER,
#   else
            0,
#   endif
            
#   if defined(GL_TESS_CONTROL_SHADER)
            GL_TESS_CONTROL_SHADER,
#   else
            0,
#   endif
            
#   if defined(GL_COMPUTE_SHADER)
            GL_COMPUTE_SHADER,
#   else
            0,
#   endif
        };
        
        uint32_t intValue = static_cast<uint32_t>(type);
        return valuesMap[intValue];
    }
    
    /**
     *  Get storage type.
     */
    uint32_t COpenGLAPI::GetAPIStorageType( ETextureStorageType type )
    {
        switch ( type ) {
            case ETextureStorageType::Uchar :   return GL_UNSIGNED_BYTE;
            case ETextureStorageType::Char:     return GL_BYTE;
            case ETextureStorageType::Float:    return GL_FLOAT;
            case ETextureStorageType::Double:   return GL_DOUBLE;
            // 32.
            case ETextureStorageType::Int:  return GL_INT;
            case ETextureStorageType::Uint: return GL_UNSIGNED_INT;
            // 16.
            case ETextureStorageType::Short:    return GL_SHORT;
            case ETextureStorageType::Ushort:   return GL_UNSIGNED_SHORT;
              
            default:
                CCore::Assert( "GetAPIStorageType: Undefined type.\n" );
                break;
        }
        
        return GL_UNSIGNED_BYTE; // By default.
    }
    
    /**
     *  Get color type for this API.
     */
    uint32_t COpenGLAPI::GetAPITextureFormat( ETextureFormat type )
    {
        static const uint32_t valuesMap[StaticGraphicsInfo::MaxTextureFormats] =
        {
            0, // Invalid,
            
            GL_RED, // R,
            GL_R8, //R8,
            
#	if defined(GL_RG16)
            GL_R16, //R16,
#	else
            0,
#	endif
            
            GL_R16F, //R16F,
            GL_R32F, //R32F,
            
            GL_RG, //RG,
            GL_RG8, //RG8,
            
#	if defined(GL_RG16)
            GL_RG16, //RG16,
#	else
            0,
#	endif
            
            GL_RG16F, //RG16F,
            GL_RG32F, //RG32F,
            GL_RGB, //RGB,
            GL_RGB8, //RGB8,
            
#	if defined(GL_RGBA16)
            GL_RGB16, //RGB16,
#	else
            0,
#	endif
            
            GL_RGB16F, //RGB16F,
            GL_RGB32F, //RGB32F,
            
#	if defined(GL_BGR)
            GL_BGR, //BGR,
#	else
            GL_RGB, // for BGR,
#	endif
            
            GL_RGBA, //RGBA,
            GL_RGBA8, //RGBA8,
            
#	if defined(GL_RGBA16)
            GL_RGBA16, //RGBA16,
#	else
            0,
#	endif
            
            GL_RGBA16F, //RGBA16F,
            GL_RGBA32F, //RGBA32F,
            GL_BGRA, //BGRA,
            
#	if defined(GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
            GL_COMPRESSED_RGB_S3TC_DXT1_EXT, //DXT1_RGB,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
            GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, //DXT1_RGBA,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT)
            GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, //DXT3,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
            GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, //DXT5,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RG_RGTC2)
            GL_COMPRESSED_RG_RGTC2, //RGTC2,
#	else
            0,
#	endif
            
            GL_DEPTH_COMPONENT, //Depth,
            GL_DEPTH_COMPONENT16, //Depth16,
            GL_DEPTH_COMPONENT24, //Depth24,
            
#	if defined(GL_DEPTH_COMPONENT32)
            GL_DEPTH_COMPONENT32, //Depth32,
#	else
            0,
#	endif
            
#	if defined(GL_DEPTH_COMPONENT32F)
            GL_DEPTH_COMPONENT32F, //Depth32F,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG)
            GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, // PVR_2bpp_RGB,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT)
            GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT, // PVR_2bpp_sRGB,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
            GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, // PVR_2bpp_RGBA,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT)
            GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, // PVR_2bpp_sRGBA,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG)
            GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, // PVR_4bpp_RGB,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT)
            GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT, // PVR_4bpp_sRGB,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
            GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, // PVR_4bpp_RGBA,
#	else
            0,
#	endif
            
#	if defined(GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT)
            GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, // PVR_4bpp_sRGBA,
#	else
            0,
#	endif
            
#if defined(GL_R11F_G11F_B10F)
            GL_R11F_G11F_B10F
#	else
            0
#	endif
        };
        
        uint32_t intValue = static_cast<uint32_t>(type);
        return valuesMap[intValue]; 
    }
    
    /**
     *  Get OpenGL texture filter mode.
     */
    uint32_t COpenGLAPI::GetAPITextureFilteringMode( ETextureFilter type )
    {
        switch( type ) {
            case ETextureFilter::Linear:    return GL_LINEAR;
            case ETextureFilter::Nearest:   return GL_NEAREST;

            default:
                break;
        }
        
        return GL_LINEAR; // By default.
    }
    
    /**
     *  OpenGL stuff initialization.
     */
    void COpenGLAPI::Initialize( void )
    {
        // Check if we can use graphics interface.
        if( !g_Core->UsesGraphics() ) {
            return;
        }

        const char * version = "410";//"300 es";
        
        GLSL_Version =  g_Core->p_Console->RegisterCVar( ECvarGroup::Display, "sGlslVersion", "GLSL version", version, CVFlag::NeedsRefresh, ECvarType::String );
        
        glGetIntegerv( GL_MAJOR_VERSION, &m_majorVersion );
        glGetIntegerv( GL_MINOR_VERSION, &m_minorVersion );

        // Set the values.
        const char * glGpu = (const char *)glGetString( GL_RENDERER );
        const char * glGlsl = (const char *)glGetString( GL_SHADING_LANGUAGE_VERSION );
        const char * glVersion = (const char *)glGetString( GL_VERSION );
        const char * glVendor = (const char *)glGetString( GL_VENDOR );

        g_Core->p_Console->Print( LOG_INFO, "Initializing OpenGL system..\n" );

        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClearDepthf( 1.0f );
        //glClearStencil( 1.0f );

//        glEnable( GL_DEPTH_TEST ); // Shouldn't leave some caps enabled..
//        glEnable( GL_CULL_FACE );
        
        // Fill up Viewport.
        glViewport( 0, 0, Render_Width->Get<int>(), Render_Height->Get<int>() );
        glGetIntegerv( GL_VIEWPORT, m_displayViewport );

        m_bInitialized = true;

        
        GLint     maxColorAttachments = 0;
        GLint       maxUniformBlockSize,
                     maxUniformBlocksVertex,
                     maxUniformOffsetAlignment = 0;
        
        m_iMaxAnisotropy = 0;
        
        // maximum texture anisotropy
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_iMaxAnisotropy);
        // maximum framebuffer rendertargets
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);
        // maximum size of uniform block
        glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
        // maximum number of uniform blocks in each shader (vertex, frag, etc)
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxUniformBlocksVertex);
        // alignment for multiple uniform blocks in one UBO - glBindBufferRange()
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &maxUniformOffsetAlignment);
        
        g_Core->p_Console->Print( LOG_INFO, "*****************************************\n" );
        g_Core->p_Console->Print( LOG_INFO, "|\tOpenGL information - \n" );
        g_Core->p_Console->Print( LOG_INFO, "|\tOpenGL version: %s\n", glVersion );
        g_Core->p_Console->Print( LOG_INFO, "|\tGPU: %s\n", glGpu );
        g_Core->p_Console->Print( LOG_INFO, "|\tVendor: %s\n", glVendor );
        g_Core->p_Console->Print( LOG_INFO, "|\tShader model version: %s\n", glGlsl );
        
        g_Core->p_Console->Print( LOG_INFO, "|\tMaximum framebuffer color attachments: %i\n", maxColorAttachments );
        g_Core->p_Console->Print( LOG_INFO, "|\tMaximum texture anisotropy level: %i\n", m_iMaxAnisotropy );
        g_Core->p_Console->Print( LOG_INFO, "|\tMaximum uniform block size: %i\n", maxUniformBlockSize );
        g_Core->p_Console->Print( LOG_INFO, "|\tMaximum vertex uniform blocks: %i\n", maxUniformBlocksVertex );
        g_Core->p_Console->Print( LOG_INFO, "|\tMaximum offset alignment: %i\n", maxUniformOffsetAlignment );
        
        
        g_Core->p_Console->Print( LOG_INFO, "|\tAvailable extensions on this system:\n" );
        
        // Request extension list.
        glGetIntegerv( GL_NUM_EXTENSIONS, &maxColorAttachments );
        for( GLint i(0); i < maxColorAttachments; ++i ) {
            const char * ext = (const char *)glGetStringi( GL_EXTENSIONS, i );
            printf( "\t\t%s\n", ext );
        }
        
        g_Core->p_Console->Print( LOG_INFO, "*****************************************\n" );
        
        g_Core->p_Console->Print( LOG_NONE, "\nOpenGL preferences initialized\n" );

        // Call the interface.
        GraphicsInterface::Initialize();
    }

    /**
     *  Called on window resize.
     */
    void COpenGLAPI::OnResize( const int32_t w, const int32_t h )
    {
        // Don't resize if we're not initialized.
        if( !m_bInitialized ) {
            return;
        }

        if( !g_Core->UsesGraphics() ) {
            return;
        }

        // TODO: w > h = w / h
        g_mainRenderer->windowWidth   = w;
        g_mainRenderer->windowHeight  = h;
    }

    /**
     *  Get OpenGL major version.
     */
    const uint32_t COpenGLAPI::GetMajorVersion( void ) const    {       return m_majorVersion;  }

    /**
     *  Get OpenGL minor version.
     */
    const uint32_t COpenGLAPI::GetMinorVersion( void ) const    {       return m_minorVersion;  }

    /**
     *  Generate mipmaps for a texture.
     */
    void COpenGLAPI::GenerateMipmaps(const uint32_t texId, const uint32_t Type)
    {
        glBindTexture( (GLenum)Type, (GLuint)texId );
        
//        glTexParameteri((GLenum)Type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri((GLenum)Type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        
        glGenerateMipmap((GLenum)Type);
        // Optionally here, already set.
//        glTexParameteri((GLenum)Image.Type, GL_TEXTURE_MAX_LEVEL, 6);
        glBindTexture( (GLenum)Type, 0 );
    }
    
    /**
     *  Create OpenGL texture.
     */
    uint32_t COpenGLAPI::CreateTexture( SImageInfo *info )
    {
        // Sanity check.
        if( info == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "CreateTexture(OpenGL): Null image information.\n" );
            return 0;
        }
        
        // Probably current texture already exists.
        if( info->TextureID ) {
            glDeleteTextures( 1, (GLuint*)&info->TextureID );
        }
        
        GLenum textureType = (GLenum)info->Type;
        
        // Create OpenGL texture.
        glGenTextures( 1, (GLuint*)&info->TextureID );
        glBindTexture( textureType, info->TextureID );
        
        // Decide what to use.
        switch( textureType ) {
            case GL_TEXTURE_2D:
                // Request 2D texture.
                glTexImage2D( textureType, 0, info->InternalFormat, info->Width, info->Height, 0, info->ExternalFormat, info->DataType, info->ImageData );
                
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY:
                // Request 3D texture.
                glTexImage3D( textureType, 0, info->InternalFormat, info->Width, info->Height, info->Tex3DLayerCount, 0, info->ExternalFormat, info->DataType, info->ImageData );
                break;
        }
        
        
//        glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
//        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        
//        glTexParameteri( textureType, GL_TEXTURE_MAX_LEVEL, 4 );
//        glTexParameteri( textureType, GL_TEXTURE_MAX_LOD, 2 );
//        glTexParameteri( textureType, GL_TEXTURE_MIN_LOD, 1 );
        
        
        glTexParameteri( textureType, GL_TEXTURE_MAG_FILTER, info->Filter );
        glTexParameteri( textureType, GL_TEXTURE_MIN_FILTER, info->Filter );
        
        glTexParameteri( textureType, GL_TEXTURE_WRAP_S, info->TileWrapS );
        glTexParameteri( textureType, GL_TEXTURE_WRAP_T, info->TileWrapT );
        
        // Anisotropy..
//        glTexParameteri( textureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_iMaxAnisotropy );
        
        if( info->HasMipmaps == true ) {
            glGenerateMipmap( textureType );
            // Override.
            
            // 'GL_TEXTURE_MAG_FILTER' can only take two values: LINEAR/NEAREST.
//            glTexParameteri( textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
            glTexParameteri( textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

            glTexParameterf( textureType, GL_TEXTURE_LOD_BIAS, info->LodBias );
        }
        else {  // Use defaults.
            glTexParameteri( textureType, GL_TEXTURE_MAG_FILTER, info->Filter );
            glTexParameteri( textureType, GL_TEXTURE_MIN_FILTER, info->Filter );
        }
        
        // If it's cubemap, then prepare the faces and add one more wrap mode.
        if( textureType == GL_TEXTURE_CUBE_MAP ) {
            
            glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, info->TileWrapT);
            
            for( int32_t face = 0; face < 6; ++face ) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, info->InternalFormat, info->Width, info->Height, 0, info->ExternalFormat, info->DataType, 0);
            }
        }
        
        glTexParameteri(textureType, GL_TEXTURE_BASE_LEVEL, 0);
//        glTexParameteri(textureType, GL_TEXTURE_MAX_LEVEL, 6);
//        glTexParameteri(textureType, GL_TEXTURE_MAX_LEVEL, 6);
        
        glBindTexture( textureType, 0 );
        
        // OpenGL returns its own indexes.
        return info->TextureID;
    }
    
    /**
     *  Check last API error.
     */
    void COpenGLAPI::CheckAPIError( const char * where )
    {
        GLenum err = glGetError();
        
        while( err != GL_NO_ERROR ) {
            const char * error;
            
            switch( err ) {
                case GL_INVALID_OPERATION:
                    error = "INVALID_OPERATION";
                    break;
                case GL_INVALID_ENUM:
                    error = "INVALID_ENUM";
                    break;
                case GL_INVALID_VALUE:
                    error = "INVALID_VALUE";
                    break;
                case GL_OUT_OF_MEMORY:
                    error = "OUT_OF_MEMORY";
                    break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    error = "INVALID_FRAMEBUFFER_OPERATION";
                    break;
                default:
                    error = "Unexpected OpenGL error";
                    break;
            }
            
            g_Core->p_Console->Print( LOG_DEVELOPER, "CheckAPIErrors(): label: \"%s\", error: \"%s\"\n", where, error );
            
            err = glGetError();
        }
    }
    
    /**
     *  Shutdown OpenGL API.
     */
    void COpenGLAPI::Shutdown()
    {
        glFinish();
        GraphicsInterface::Shutdown();
    }
    
#ifdef _WIN32AAA
    ncGL32API local_gl;
    ncGL32API *m_OpenGL = &local_gl;

    bool ncGL32API::LoadExtensionList()
    {

/*
        glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");

        glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");

        glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");
        glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");

        glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
        glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");

        glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
        glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
        glDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");


        glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
        glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
        glDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");

        glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
        glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");

        glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
        glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)wglGetProcAddress("glGetAttribLocation");

        glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
        glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");

        glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
        glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
        glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
        glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
        glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");

        glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
        glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");

        glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");

        glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");

        glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");


        glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");

        glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");

        glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glDisableVertexAttribArray");

        glUniform3fv = (PFNGLUNIFORM3FVPROC)wglGetProcAddress("glUniform3fv");

        glUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");

        glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
        glUniform2f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform2f");
        glUniform3f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform3f");
        glUniform4f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform4f");

        glUniform1d = (PFNGLUNIFORM1DPROC)wglGetProcAddress("glUniform1d");
        glUniform1fv = (PFNGLUNIFORM1FVPROC)wglGetProcAddress("glUniform1fv");
        glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1i");

        glUniform1iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1i");
        glUniform2iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform2i");
        glUniform3iv = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform3i");

        glUniform1d = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform1d");
        glUniform2d = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform2d");
        glUniform3d = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform3d");
        glUniform4d = (PFNGLUNIFORM1IVPROC)wglGetProcAddress("glUniform4d");

*/
        return true;
    }
#endif

}
