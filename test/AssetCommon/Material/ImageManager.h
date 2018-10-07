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
//  ImageManager.h
//  Image loader.. :V
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef image_h
#define image_h

#include "../../Core/Core.h" // Non-copyable define.
#include "../../Platform/Shared/SystemShared.h"
#include "../../Graphics/Image.h"
#include "../../Core/GameMemory.h" // Memory pools.

namespace Neko {

    /**
     *  Classic bmp header.
     */
    struct ncBMPHeader
    {
        char bfType[2];       // "BM"
        
        int bfSize;           // Size of file in bytes
        int bfReserved;       // skip
        int bfOffBits;        // Byte offset to bitmap data; 54
        int biSize;           // Size of BITMAPINFOHEADER; 40
        int biWidth;          // Width of image
        int biHeight;         // Height of image
        
        short biPlanes;       // Number of planes in target device
        short biBitCount;     // Bits per pixel
        
        int biCompression;    // Type of compression (0 if no compression)
        int biSizeImage;      // Image size, in bytes (0 if no compression)
        int biXPelsPerMeter;  // Resolution in pixels/meter of display device.
        int biYPelsPerMeter;  // Resolution in pixels/meter of display device.
        int biClrUsed;        // Number of colors in the color table (if 0, use maximum allowed by biBitCount)
        int biClrImportant;   // Number of important colors. If 0, all colors are important.
    };
    
    /**
     *  Uncompressed Targa header.
     */
    typedef struct
    {
        Byte                header[6];          // First six headers.
        unsigned int		bytesPerPixel;      // Bytes per pixels in image.
        unsigned int		imageSize;          // Image size.
        unsigned int		temp;               // Temp variable.
        unsigned int		type;               // Type of image, RGB or RGBA.
        unsigned int		Height;             // Height of image.
        unsigned int		Width;              // Width of Image.
        unsigned int		Bpp;					// Bits Per Pixel.
    } ncTGAImage;

    
    /**
     *  Image loader.
     */
    class CImageLoader
    {
        NEKO_NONCOPYABLE( CImageLoader );
        
    public:
        
        // Default constructor/destructor.
        CImageLoader() { }
        ~CImageLoader() { }
        
        /**
         *  Create image.
         */
        static bool                 CreateImage( const unsigned int width, const unsigned int height, const Byte * data, EImageInfoType type, const char * filename );
        
        /**
         *  Unload image.
         */
        void                Unload( SImageInfo &tex );
        
        /**
         *  Create BMP file.
         */
        static              const int WriteBmp( const char *filename, const int width, const int height, const Byte * rgb );
        
        /**
         *  Load TGA file.
         */
        bool                LoadTGA( const char *fileName, SImageInfo * out, SMemoryTempFrame * memory_area );
        
        /**
         *  Load BMP file.
         */
        bool                LoadBMP( const char *fileName, SImageInfo * out, SMemoryTempFrame * memory_area );
        
        /**
         *  Load image file with a type.
         */
        bool                LoadImage( const char *filename, EImageInfoType type, SImageInfo * out, SMemoryTempFrame * memory_area );
        
        bool                LoadUncompressedTGA( SImageInfo* texture, const char *  filename, FILE * fTGA );
        bool                LoadCompressedTGA( SImageInfo * texture, const char *  filename, FILE * fTGA );
        
        void            MakeSeamlessTGA( SImageInfo *img );
        void            MakeBlurry( SImageInfo * img );
    };
    
    extern CImageLoader *g_imageManager;
}

#endif
