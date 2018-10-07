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
//  Framebuffer.h
//  OpenGL Framebuffer things. ;D
//
//  Created by Neko Code on 1/14/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__Framebuffer__
#define __Nanocat__Framebuffer__

#include "../Platform/Shared/SystemShared.h"
#include "../Core/APIObject.h"
#include "../Core/CoreDef.h"

#ifndef NEKO_SERVER
namespace Neko {

    //!  Maximum framebuffer render buffer objects.
    static const int32_t MAX_FRAMEBUFFER_RENDERBUFFERS = 7;
    
    /// Framebuffer object class.
    class CFramebuffer : public APIObject
    {
        // Noncopyable.
        NEKO_NONCOPYABLE( CFramebuffer );
        
    public:
        
        /**
         *  Do not call, has empty properties and will call an error.
         */
        CFramebuffer();
        
        /**
         *  Setup new framebuffer.
         */
        CFramebuffer( const int32_t x, const int32_t y,
                      bool _usesDepth,
                      bool _usesCT0,
                      bool _usesCT1,
                      bool _usesCT2,
                      bool _usesCT3,
                      
                      /* Color targets. */
        
                      const GLenum target0 = GL_RGBA16F,
                      const GLenum target1 = GL_RGBA16F,
                      const GLenum target2 = GL_RGBA16F,
                      const GLenum target3 = GL_RGBA16F );
       
        /**
         *  Set framebuffer properties.
         */
        void                Create( const int32_t x, const int32_t y,
                                    bool _usesDepth = false,
                                    bool _usesRB = false,
                                    bool _usesCT0 = false,
                                    bool _usesCT1 = false,
                                    bool _usesCT2 = false,
                                    bool _usesCT3 = false,
                                    
                                    /* Color targets. */
                                    
                                    const GLenum target0 = GL_RGBA16F,
                                    const GLenum target1 = GL_RGBA16F,
                                    const GLenum target2 = GL_RGBA16F,
                                    const GLenum target3 = GL_RGBA16F );
        
        /**
         *  Resize a framebuffer.
         *  @default values: target = GL_COLOR_BUFFER_BIT, filter = GL_LINEAR
         */
        void                Resize( const uint32_t newWidth, const uint32_t newHeight, const GLbitfield target = GL_COLOR_BUFFER_BIT, GLenum filter = GL_LINEAR );
        
        /**
         *  Uses depth buffer?
         */
        void                SetUsesDepth( bool bUsesDepth );

        void                SetDepthFormat( const GLenum internalFormat );
        void                SetDepthTarget( const GLenum internalTarget );
        
        /**
         *  Bind framebuffer.
         */
        inline void BindForDrawing()    {
            glBindFramebuffer( GL_DRAW_FRAMEBUFFER, GetHandle() );
        }
        
        /**
         *  Bind framebuffer.
         */
        inline void BindForReading() {
            glBindFramebuffer( GL_READ_FRAMEBUFFER, GetHandle() );
        }
        
        
        /**
         *  Unbind buffer.
         */
        inline void UnbindDrawing() {
            glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
        }
        
        /**
         *  Unbind buffer.
         */
        inline void UnbindReading() {
            glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );
        }
        
        /**
         *  Delete framebuffer.
         */
        void                Delete();

        /**
         *  Get depth texture.
         */
        inline GLuint               & GetUnitDepth()   {       return m_maxTextures[0];    }

        /**
         *  Get render target #3 texture.
         */
        inline GLuint               & GetUnitRT2() {       return m_maxTextures[3];    }
        
        /**
         *  Get render target #1 texture.
         */
        inline GLuint               & GetUnitRT0() {       return m_maxTextures[1];    }
        
        /**
         *  Get render target #2 texture.
         */
        inline GLuint               & GetUnitRT1()  {       return m_maxTextures[2];    }
        
        /**
         *  Get render target #4 texture.
         */
        inline GLuint               & GetUnitRT3()  {       return m_maxTextures[4];    }

        /**
         *  Get framebuffer width.
         */
        uint32_t                    GetWidth();
        
        /**
         *  Get framebuffer height.
         */
        uint32_t                    GetHeight();
        
        /**
         *  Add buffer to framebuffer object.
         */
        void                AddBuffer( GLenum internalFormat, GLenum _attachment, GLenum _renderbuffertarget );

//    private:
        
        /**
         *  Created framebuffer with properties ( which are already defined ).
         */
        void                    CreateFramebuffer( const int32_t x, const int32_t y );

        uint32_t    _x;
        uint32_t    _y;
        
        int32_t     m_totalBuffers;
        int32_t     m_totalTextures;
        
        // Depth buffer.
        bool m_bUsesDepth;
        bool m_bNeedsRenderBuffer;
        
        // Color targets.
        bool m_bUsesCT0;
        bool m_bUsesCT1;
        bool m_bUsesCT2;
        bool m_bUsesCT3;
        
        GLenum textureFormatCT0;
        GLenum textureFormatCT1;
        GLenum textureFormatCT2;
        GLenum textureFormatCT3;
        
        GLenum depthInternalFormat, depthInternalTarget;
        
        GLuint  m_maxBuffers[MAX_FRAMEBUFFER_RENDERBUFFERS];
        GLuint  m_maxTextures[MAX_FRAMEBUFFER_RENDERBUFFERS];
    };
}
#endif // NEKO_SERVER
#endif /* defined(__Nanocat__Framebuffer__) */
