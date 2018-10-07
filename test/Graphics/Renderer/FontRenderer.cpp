//
//              *                  *
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
//  CoreFont.cpp
//  Font loader and renderer.. >.>
//
//  Created by Neko Vision on 02/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../../Core/Core.h"
#include "../../Platform/Shared/SystemShared.h"
#include "../../AssetCommon/AssetBase.h"
#include "../../AssetCommon/Material/MaterialLoader.h"
#include "FontRenderer.h"
#include "Renderer.h"
#include "../OpenGL/OpenGLBase.h"
#include "../../Math/GameMath.h"

#ifndef NEKO_SERVER

namespace Neko {

    /**
     *  Font renderer instance.
     */
    CCoreFontRenderer::CCoreFontRenderer()
    {
        fVertexArray = 0;
        
        fShaderColorID = 0;
        fShaderTextureID = 0;
        
        iFontsLoaded = 0;
        
        fShader = NEKO_NULL;
    }
    
    /**
     *  Destructor.
     */
    CCoreFontRenderer::~CCoreFontRenderer() {
        
    }
    
    /**
     *  Precache font.
     */
    uintptr_t CCoreFontRenderer::PrecacheFont( const char *fontName )
    {
        // Find a font map.
        fTextureMap[iFontsLoaded] = f_AssetBase->p_MaterialBase->Find( fontName/*ui/font_pixelated*/ );
        
        uintptr_t texId = fTextureMap[iFontsLoaded]->m_pDiffuse->Image.GetId();
        
        // Change filtering to linear so font map appears smooth.
        g_mainRenderer->BindTexture( 0, (GLuint)texId );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); // Just for sure.
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
        g_mainRenderer->UnbindTexture( 0 );
        
        ++iFontsLoaded;
        
        g_Core->p_Console->Print( LOG_INFO, "PrecacheFont(): \"%s\" loaded.\n", fontName );
        
        m_iCharSkip = 32;
        
        return texId; // Return a texture index.
    }
    
    /**
     *  Print text somewhere on screen.
     */
    void CCoreFontRenderer::DrawString( const Vec4& color, float x, float y, int32_t size, const Vec2i & align,  const char * msg )
    {
        // That was made for inverted 'y' in windowed mode, mostly for OSX to skip
        // upper and bottom spaces, such as Dock, etc..
//        y = g_mainRenderer->displayFixedHeight - y ;
        // Done in its shader!
        
//        y /= 2;
        char    character;
        
        size_t  length;
        uint32_t    c;
        
        uint32_t    i;
        
        float   nx;
//        float ny = ( length * fHeight / 3 );
        
        length = strlen( msg );
        nx = ( length * fWidth / 3 );
        c = 0;
        
        // Text align.
        switch( (int32_t)align.x )
        {
            case 2:
                x -= nx;
                break;
            case 1:
                x -= (int)(nx / 2);
                break;
            case 0:
            default:
                break;
        }
        
        
        // Fix y-origin a bit.
        y = y - fHeight / 1.5f;
        
        float fAdjustedWidth( fWidth ), fAdjustedHeight( fHeight );
        float fAdjustedX( x ), fAdjustedY( y );
        
        // Fix font position.
        g_mainRenderer->AdjustElemSize( &fAdjustedX, &fAdjustedY );
        g_mainRenderer->AdjustElemSize( &fAdjustedWidth, &fAdjustedHeight );
        
        // Scroll thru every character.
        // TODO: indexed characters.
        for ( i = 0; i < length; ++i )
        {
            // Find character byte.
            character = msg[i] - m_iCharSkip;
            
            // Find uvs for character.
            float uv_x = ( character % 16 ) / 16.0f;
            float uv_y = ( character / 16 ) / 16.0f;
            
            // Make a small character quad.
            Vec2 vertex_up_left    = Vec2(i * size , size - fAdjustedHeight );
            Vec2 vertex_up_right   = Vec2(  i * size + size + fAdjustedWidth , size - fAdjustedHeight );
            Vec2 vertex_down_right = Vec2(  i * size + size + fAdjustedWidth , 0.0f );
            Vec2 vertex_down_left  = Vec2( i * size, 0.0f );
            
            
            // Build character map.
            fFontBufferData[c].x = vertex_up_left.x; fFontBufferData[c].z = uv_x;
            fFontBufferData[c].y = vertex_up_left.y; fFontBufferData[c].w = 1 - uv_y - fSize;
            ++c;
            
            fFontBufferData[c].x = vertex_down_left.x; fFontBufferData[c].z = uv_x;
            fFontBufferData[c].y = vertex_down_left.y; fFontBufferData[c].w = 1 - uv_y;
            ++c;
            
            fFontBufferData[c].x = vertex_up_right.x; fFontBufferData[c].z = uv_x + fSize;
            fFontBufferData[c].y = vertex_up_right.y; fFontBufferData[c].w = 1 - uv_y - fSize; // Right
            ++c;
            
            fFontBufferData[c].x = vertex_down_right.x; fFontBufferData[c].z = uv_x + fSize;
            fFontBufferData[c].y = vertex_down_right.y; fFontBufferData[c].w = 1 - uv_y;
            ++c;
            
            fFontBufferData[c].x = vertex_up_right.x; fFontBufferData[c].z = uv_x + fSize;
            fFontBufferData[c].y = vertex_up_right.y; fFontBufferData[c].w = 1 - uv_y - fSize;
            ++c;
            
            fFontBufferData[c].x = vertex_down_left.x; fFontBufferData[c].z = uv_x;
            fFontBufferData[c].y = vertex_down_left.y; fFontBufferData[c].w = 1 - uv_y;
            ++c;
        }
        
        //! So we made our string, render it once and nice.
        
        //  Real time text rendering.
        fShader->Use();
        
        glBindVertexArray( fVertexArray );
        
        // Set buffer.
        glBindBuffer( GL_ARRAY_BUFFER, fVertexBuffer.Handle );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(Vec4) * c, &fFontBufferData[0] );
        
        // Bind font texture.
        g_mainRenderer->BindTexture( 0, fTextureMap[0]->m_pDiffuse->GetId() );
        
        // Apply color.
        fShader->SetUniform( (GLuint)fShaderColorID, color.x, color.y, color.z );
        fShader->SetUniform( (GLuint)fShaderOriginScaleID, 1.0f, 1.0f, fAdjustedX, fAdjustedY );
        
        // Bind vertex object and render font.
        glDrawArrays( GL_TRIANGLES, 0, c );

        glBindVertexArray( 0 );
        fShader->Next();
    }
    
    /**
     *  Load font system.
     */
    void CCoreFontRenderer::Initialize( void )
    {
        // Font preferences.
        fHeight = 0.0175f;
        fWidth = 0.0125f;
        fSize = 0.0625f;
        
        // Font vertex array.
        fVertexArray = g_mainRenderer->CreateVertexArray();
        
        // Create buffers.
        fVertexBuffer = g_mainRenderer->AllocGPUBuffer( fFontCharacterSize * 6 * sizeof(Vec4), EBufferStorageType::Array, EBufferType::Stream );
        
        // Just keep font buffer empty at beginning.
        g_mainRenderer->BufferPointer( &fVertexBuffer, 0, 4, GL_FLOAT, GL_FALSE, sizeof(Vec4), 0 );
        g_mainRenderer->FinishBuffer( &fVertexBuffer, 0, fFontCharacterSize * 6 );
        
        glBindVertexArray( 0 );

        // Font rendering shader.
        fShader = f_AssetBase->FindAssetByName<SGLShader>( "font" );
        
        // Load the main font.
        PrecacheFont( "ui/font" );
        
        // Setup some preferences.
        fShader->Use();
        
        // Font color.
        fShaderColorID = fShader->UniformLocation( "colorModifier" );
        // Font map.
        fShaderTextureID = fShader->UniformLocation( "font_texture" );
        // Font size.
        fShaderOriginScaleID = fShader->UniformLocation( "scale_origin" );
        
        fShader->SetUniform( (GLuint)fShaderTextureID, 0 );
        fShader->SetUniform( (GLuint)fShaderColorID, 1.0f, 1.0f, 1.0f );
        
        fShader->Next();
    }
    
    /**
     *  Destroy the font system.
     */
    void CCoreFontRenderer::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "Font system shutting down..\n" );
        
#   if defined( USES_OPENGL )
        glDeleteVertexArrays( 1, &fVertexArray );
#   endif
        
        g_mainRenderer->DeleteGPUBuffer( &fVertexBuffer );
    }
}
#endif // NEKO_SERVER