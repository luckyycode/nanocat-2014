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
//  Texture.h
//  Texture creationism..
//
//  Created by Neko Code on 2/9/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__Texture__
#define __Nanocat__Texture__

#include "../Platform/Shared/SystemShared.h"
#include "../Math/Vec3.h"
#include "RendererMisc.h"

namespace Neko {
    
    ///  Image file types.
    enum EImageInfoType
    {
        TGA_IMAGE   = 0,                        // TGA
        BMP_IMAGE                             // BMP
    };
    
    ///  Image.
    struct SImageInfo
    {
    public:
        
        /**
         *  Returns pixel color at pixel position.
         */
        Vec3i              GetPixelColorAt( const uint32_t x, const uint32_t y ) const;
        
        SImageInfo();
        
        /**
         *  Texture width.
         */
        inline const uint32_t               GetWidth() const  {       return Width;   }
        
        /**
         *   Texture height.
         */
        inline const uint32_t                   GetHeight() const {       return Height;  }
        
        /**
         *  Texture Id.
         */
        inline const uint32_t               GetId() const {       return TextureID;   }
        
        /**
         *  Get type.
         */
        inline const GLuint                 GetType() const {       return Type;    }
        
        /**
         *   Bits per pixel.
         */
        inline const GLuint                 GetBitsPerPixel() const {       return BitsPerPixel;    }
        
        /**
         *  Get image data.
         *
         *  @note Can be NEKO_NULL.
         */
        inline Byte *               GetData() const   {       return ImageData;   }
        
        /**
         *  Make seamless image.
         */
        void                MakeSeamless();
        
        
        /**
         *  Remove texture and its data.
         */
        void Unload()
        {
            glDeleteTextures( 1, (GLuint*)&TextureID );
        }
        
        
        uint8_t        *ImageData;             //! Image Data (Up To 32 Bits).
        int32_t     TextureID;				//! Texture ID Used To Select A Texture.
        
        uint32_t 	Width;					//! Image Width.
        uint32_t    Height;					//! Image Height.
        uint32_t    BitsPerPixel;			//! Image Color Depth In Bits Per Pixel.
        uint32_t    InternalFormat;					//! Image Type (GL_RGB, GL_RGBA).
        uint32_t    ExternalFormat;
        
        bool    HasMipmaps;
        
        uint32_t    DataType;   //! Data storage types ( FLOAT, INT, BYTE, etc.. )
        
        uint32_t    Type;   //! Texture2D, TextureRect, etc.. depends what we have picked.
        uint32_t    Filter;
        uint32_t    TileWrapS;
        uint32_t    TileWrapT;
        
        int32_t     Tex3DLayerCount;     //! Amount of 3D texture layers.
        float   LodBias;        //! Level of Detail Bias for Mipmapping.
    };

}

#endif /* defined(__Nanocat__Texture__) */
