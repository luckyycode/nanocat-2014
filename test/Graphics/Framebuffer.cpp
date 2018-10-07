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
//  Framebuffer.cpp
//  OpenGL Framebuffer things. ;D
//
//  This file is a part of Neko engine.
//  Created by Neko Code on 1/14/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#include "Framebuffer.h"
#include "../Core/Core.h"
#include "Renderer/Renderer.h" // Rendering options!

#ifndef NEKO_SERVER

namespace Neko {
    
    const static float white_color[] = { 1.0f, 1.0f, 1.0f, 1.0f }; //! Used to paint texture borders.
    
    /**
     *
     *  Generate frame buffer.
     *
     */
    CFramebuffer::CFramebuffer( const int32_t x, const int32_t y,
                                 bool _usesDepth /* Uses Depth buffer? */,
                                 bool _usesCT0 /* Uses color target 0 ? */,
                                 bool _usesCT1 /* ..and soon .. */,
                                 bool _usesCT2,
                                 bool _usesCT3,
                                 /* Color targets. */
                                 const GLenum target0,
                                 const GLenum target1,
                                 const GLenum target2,
                                 const GLenum target3 ) : m_bUsesCT0(_usesCT0), m_bUsesCT1(_usesCT1), m_bUsesCT2(_usesCT2), m_bUsesCT3(_usesCT3), m_bUsesDepth(_usesDepth), m_bNeedsRenderBuffer(true), m_totalBuffers(0), m_totalTextures(0), depthInternalFormat(GL_RGB), depthInternalTarget(GL_RGBA),
                                     textureFormatCT0(target0),
                                     textureFormatCT1(target1),
                                     textureFormatCT2(target2),
                                     textureFormatCT3(target3)
    {
        CreateFramebuffer( x, y );
    }

    /**
     *  Constructor.
     */
    CFramebuffer::CFramebuffer()
    {
        // ( Use main settings instead )
        m_totalBuffers = 0;
        m_totalTextures = 0;
    }
    
    /**
     *  Set framebuffer properties.
     */
    void CFramebuffer::Create( const int32_t x, const int32_t y,
                                    bool _usesDepth /* Uses Depth buffer? */,
                                    bool _usesRB,
                                    bool _usesCT0 /* Uses color target 0 ? */,
                                    bool _usesCT1 /* ..and soon .. */,
                                    bool _usesCT2,
                                    bool _usesCT3,
                                    /* Color targets. */
                                    const GLenum target0,
                                    const GLenum target1,
                                    const GLenum target2,
                               const GLenum target3 )
    {
        //! Color targets.
        this->m_bUsesCT0 = _usesCT0;
        this->m_bUsesCT1 = _usesCT1;
        this->m_bUsesCT2 = _usesCT2;
        this->m_bUsesCT3 = _usesCT3;
        
        this->m_bUsesDepth = _usesDepth;
        this->m_bNeedsRenderBuffer = _usesRB;
        
        this->m_totalBuffers = 0;
        this->m_totalTextures = 0;
        
        this->depthInternalFormat = GL_RGB;
        this->depthInternalTarget = GL_RGBA;
        
        this->textureFormatCT0 = target0;
        this->textureFormatCT1 = target1;
        this->textureFormatCT2 = target2;
        this->textureFormatCT3 = target3;
        
        // Create framebuffer itself.
        CreateFramebuffer( x, y );
    }

    /**
     *  Create framebuffer.
     */
    void CFramebuffer::CreateFramebuffer( const int32_t x, const int32_t y )
    {
        uint32_t frameBuffer;
        int32_t totalReadBuffers;
        GLenum status;

        glEnable( GL_TEXTURE_2D );
        
        this->_x = x;
        this->_y = y;

        // Generate a new framebuffer.
        glGenFramebuffers( 1, &frameBuffer );
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, frameBuffer );

        SetHandle( frameBuffer );
        
        totalReadBuffers = 0;
        
        //! Depth & stencil buffer.
        if( m_bUsesDepth && m_bNeedsRenderBuffer )
        {
            glGenRenderbuffers( 1, &m_maxBuffers[m_totalBuffers] );
            glBindRenderbuffer( GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, x, y );
            glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            m_totalBuffers++;
        }
        
        //! Render buffer.
        if( m_bUsesCT0 && m_bNeedsRenderBuffer )
        {
            glGenRenderbuffers( 1, &m_maxBuffers[m_totalBuffers] );
            glBindRenderbuffer( GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            glRenderbufferStorage( GL_RENDERBUFFER, textureFormatCT0, x, y );
            glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            m_totalBuffers++;
            totalReadBuffers++;
        }
        
        //! Bind the normal render target.
        if( m_bUsesCT1 && m_bNeedsRenderBuffer )
        {
            glGenRenderbuffers( 1, &m_maxBuffers[m_totalBuffers] );
            glBindRenderbuffer( GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            glRenderbufferStorage( GL_RENDERBUFFER, textureFormatCT1, x, y );
            glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            m_totalBuffers++;
            totalReadBuffers++;
        }

        //! Bind the no-light-color render target.
        if( m_bUsesCT2 && m_bNeedsRenderBuffer )
        {
            glGenRenderbuffers( 1, &m_maxBuffers[m_totalBuffers] );
            glBindRenderbuffer( GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            glRenderbufferStorage( GL_RENDERBUFFER, textureFormatCT2, x, y );
            glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            m_totalBuffers++;
            totalReadBuffers++;
        }
        
        //! Bind position mapping buffer data
        if( m_bUsesCT3 && m_bNeedsRenderBuffer )
        {
            glGenRenderbuffers( 1, &m_maxBuffers[m_totalBuffers] );
            glBindRenderbuffer( GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            glRenderbufferStorage( GL_RENDERBUFFER, textureFormatCT3, x, y );
            glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER, m_maxBuffers[m_totalBuffers] );
            m_totalBuffers++;
            totalReadBuffers++;
        }
        //g_Core->p_Console->Print( LOG_INFO, "Framebuffer buffers: %i\n", m_totalBuffers );
        
        
        //! Depth texture.
        if( m_bUsesDepth )
        {
            glGenTextures( 1, &m_maxTextures[m_totalTextures] );
            glBindTexture( GL_TEXTURE_2D, m_maxTextures[m_totalTextures] );
            glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, x, y, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NEKO_NULL );
            
            // Make a texture border color to white so it's good for things such as
            // shadow mapping, etc..
//            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, white_color);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
//            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
//            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//            // Depth buffer error on iOS.
//#if !defined(iOS_BUILD) || !defined(_WIN32) || !defined(_WIN64) // Nvidia card problem with color targets, resulting a non-lit scene.
//            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
//            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
//#endif
            
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_maxTextures[m_totalTextures], 0 );
            ++m_totalTextures;
            // depth texture is gonna be a mipmap so we have to establish the mipmap chain
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture( GL_TEXTURE_2D, 0 );
        } else {
            ++m_totalTextures;  // So we can still use GetUnitRTX
        }
        
        
        //! Generate texture.
        if( m_bUsesCT0 )
        {
            glGenTextures( 1, &m_maxTextures[m_totalTextures] );
            glBindTexture( GL_TEXTURE_2D, m_maxTextures[m_totalTextures] );
            glTexImage2D( GL_TEXTURE_2D, 0, textureFormatCT0,  x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NEKO_NULL );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_maxTextures[m_totalTextures], 0 );
            m_totalTextures++;
        }
        
        //! Normal map.
        if( m_bUsesCT1 )
        {
            glGenTextures( 1, &m_maxTextures[m_totalTextures] );
            glBindTexture( GL_TEXTURE_2D, m_maxTextures[m_totalTextures] );
            glTexImage2D( GL_TEXTURE_2D, 0, textureFormatCT1, x, y, 0, GL_RGBA, GL_FLOAT, 0 );
//            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_maxTextures[m_totalTextures], 0 );
            m_totalTextures++;
        }
        
        //! No light color map.
        if( m_bUsesCT2 )
        {
            glGenTextures( 1, &m_maxTextures[m_totalTextures] );
            glBindTexture( GL_TEXTURE_2D, m_maxTextures[m_totalTextures] );
            glTexImage2D( GL_TEXTURE_2D, 0, textureFormatCT2, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_maxTextures[m_totalTextures], 0 );
            m_totalTextures++;
        }

        //! Position mapping.
        if( m_bUsesCT3 )
        {
            glGenTextures( 1, &m_maxTextures[m_totalTextures] );
            glBindTexture( GL_TEXTURE_2D, m_maxTextures[m_totalTextures] );
            glTexImage2D( GL_TEXTURE_2D, 0, textureFormatCT3, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
            glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_maxTextures[m_totalTextures], 0 );
            m_totalTextures++;
        }
    
        //! Draw buffers.
        GLenum DrawBuffers[] =
        {
            static_cast<GLenum>(m_bUsesCT0 ? GL_COLOR_ATTACHMENT0 : GL_NONE),
            static_cast<GLenum>(m_bUsesCT1 ? GL_COLOR_ATTACHMENT1 : GL_NONE),
            static_cast<GLenum>(m_bUsesCT2 ? GL_COLOR_ATTACHMENT2 : GL_NONE),
            static_cast<GLenum>(m_bUsesCT3 ? GL_COLOR_ATTACHMENT3 : GL_NONE)
        };
        
        if( totalReadBuffers != 0 ) {
            glDrawBuffers( totalReadBuffers, DrawBuffers );
        } else {
            glDrawBuffer( GL_NONE );
            glReadBuffer( GL_NONE );
        }

        /**
            Note to myself.

         * Do not edit, it wasn't working on Mac before
         now it's perfect.
         */

        // Check for existing errors and its completeness.
        status = glCheckFramebufferStatus( GL_FRAMEBUFFER ); // TODO: Full string messages.
        if( status != GL_FRAMEBUFFER_COMPLETE ) {
            g_Core->p_Console->Error( ERR_OPENGL, "Could not initialize frame buffer object. Code %i\n", status );
            return;
        }

        glDisable( GL_TEXTURE_2D );
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
        
        g_Core->p_Console->Print( LOG_INFO, "Created framebuffer with %i render targets.\n", totalReadBuffers );
    }
    
    /**
     *  Resize a framebuffer.
     */
    void CFramebuffer::Resize( const uint32_t newWidth, const uint32_t newHeight, const GLbitfield target, GLenum filter )
    {
        glBlitFramebuffer( 0, 0, this->_x, this->_y, 0, 0, newWidth, newHeight, target, filter );
    }

    /**
     *  Set depth internal format.
     */
    void CFramebuffer::SetDepthFormat( const GLenum internalFormat )
    {
        depthInternalFormat = internalFormat;
    }
    
    /**
     *  Delete framebuffer object.
     */
    void CFramebuffer::Delete()
    {
        uint32_t frameBuffer = GetHandle();
        glDeleteFramebuffers( 1, &frameBuffer );

        glDeleteRenderbuffers( m_totalBuffers, m_maxBuffers );
        glDeleteTextures( m_totalTextures, m_maxTextures );
        
    }
}
#endif
//
//                ,MMM8&&&.            *
//               MMMM88&&&&&    .
//              MMMM88&&&&&&&
// *            MMM88&&&&&&&&
//              MMM88&&&&&&&&
//              'MMM88&&&&&&'
//                'MMM8&&&'      *
//    *    |\___/|
//         )     (             .              '
//        >\     /<
//          )===(       *
//         /     \
//         |     |
//        /       \           *
//        \       /
// _/\_/\_/\__  _/_/\_/\_/\_/\_/\_/\_/\_/\_/\_
// |  |  |  |( (  |  |  |  |  |  |  |  |  |  |
// |  |  |  | ) ) |  |  |  |  |  |  |  |  |  |
// |  |  |  |(_(  |  |  |  |  |  |  |  |  |  |
// |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
// |                    |                    |
