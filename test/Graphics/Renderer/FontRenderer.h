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
//  CoreFont.h
//  Game font renderer..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef corefont_h
#define corefont_h

#include "../../Math/GameMath.h" // 2d vector stuff.
#include "../OpenGL/GLShader.h"
#include "../../AssetCommon/Material/MaterialLoader.h" // Font loading.
#include "Renderer.h"

#ifndef NEKO_SERVER

namespace Neko {

    //!  Max fonts allowed to load.
    static const int32_t MAX_FONTS = 8;
    
    //!  Bitmap font character skip.
    static const int32_t fFontCharacterSize = 64;
    
    ///   Font renderer.
    class CCoreFontRenderer
    {
        NEKO_NONCOPYABLE( CCoreFontRenderer );
        
    public:
        
        /**
         *  Constructor.
         */
        CCoreFontRenderer();
        
        /**
         *  Destructor.
         */
        ~CCoreFontRenderer();
        
        /**
         *  Initialize.
         */
        void                Initialize( void );
        
        /**
         *  Print 2D text.
         */
        void                DrawString( const Vec4 & color, float x, float y, int32_t size, const Vec2i & align,  const char * msg ) ;
        
        /**
         *  Destroy the font system.
         */
        void                Shutdown();
        
        /**
         *  Precache font.
         */
        uintptr_t               PrecacheFont( const char * fontName );
        
        
        //! Vertex buffers.
        GPUBuffer       fVertexBuffer;
        
        //! Vertex array.
        VertexArray         fVertexArray;
        
    private:
        
        //! Font buffer data.
        Vec4       fFontBufferData[6 * fFontCharacterSize * sizeof(Vec4)];
        
        //! Font character width.
        float           fWidth;
        
        //! Font character height.
        float           fHeight;
        
        //! String size.
        float           fSize;
        
        //! Total fonts loaded.
        uint32_t        iFontsLoaded;
        
        //! Character skip on its fontmap.
        int32_t         m_iCharSkip;
        
        
        //! Shader stuff.
        uintptr_t       fShaderColorID;
        uintptr_t       fShaderTextureID;
        uintptr_t       fShaderOriginScaleID;
//        uintptr_t fShaderOriginID;
        
        //! Texture maps.
        SMaterialProp *         fTextureMap[MAX_FONTS];
        
        //! Font shader.
        SGLShader *         fShader;  // Font shader.
    };
 
}

#endif // NEKO_SERVER

#endif
