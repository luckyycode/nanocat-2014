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
//  ImageManager.cpp
//  Image loader.. :V
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//


#include "../../Core/Core.h"
#include "../../Core/Streams/MemoryStream.h"
#include "../../Core/Streams/Streams.h"
#include "../../Core/String/StringHelper.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../FileSystem.h"
#include "ImageManager.h"


namespace Neko {
    
    CImageLoader local_imageManager;
    CImageLoader *g_imageManager = &local_imageManager;
    
    /**
     *  New image instance.
     */
    SImageInfo::SImageInfo() : Tex3DLayerCount(1)
    {
        //ImageData = NEKO_NULL;
        
        //BitsPerPixel = -1;
        //Width = -1;
        //Height = -1;
        
        //Type = -1;
        //TextureID = -1;
        LodBias = 0.0f;
    }
    
    
    /**
     *  Get color at pixel.
     */
    vec3_template<int> SImageInfo::GetPixelColorAt( unsigned int x, unsigned int y ) const
    {
        unsigned int idx = (y * Width + x) * BitsPerPixel;
        return vec3_template<int>( ImageData[idx+0], ImageData[idx+1], ImageData[idx+2] );
    }
    
    /**
     *  Load Targa uncompressed image.
     */
    bool CImageLoader::LoadTGA( const char * fileName, SImageInfo * out, SMemoryTempFrame * memory_area ) {

        uint32_t        imageSize;
        
        Byte            ucharBad;
        Byte            colorSwap;
        
        int16_t         sintBad;

        int32_t         colorMode;
        int32_t         imageIdx;
        
        ncMemoryStream  * stream;
        SPackFile      * dataFile;
        AssetDataPool   data;
        
        // Load file from a package and create
        // a memory stream to read contents of the file.
        dataFile = g_Core->p_FileSystem->GetPak( "texturedata" );
        data = dataFile->GetData( NC_TEXT( "%s.tga", fileName ), memory_area );
        
        if( data.tempData == NEKO_NULL ) {
            return false; // lol why
        }
        
        stream = (ncMemoryStream *)PushMemory( data.tempPool, sizeof(ncMemoryStream) );
        
        // Set image data to our memory stream.
        stream->SetSourceBuffer( &data.tempData );
        
        if( data.tempData == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "ImageLoad(): Couldn't find %s image.\n", fileName );
            _PopMemoryFrame( data.tempPool );
            return false;
        }
        
        stream->Read( &ucharBad, sizeof(unsigned char), 1 );// fread( &ucharBad, sizeof(unsigned char), 1, imgfile );
        stream->Read( &ucharBad, sizeof(unsigned char), 1 );//fread( &ucharBad, sizeof(unsigned char), 1, imgfile );
        stream->Read( &out->Type, sizeof(unsigned char), 1 );//fread( &Type, sizeof(unsigned char), 1, imgfile );
        
        if( out->Type != 2 && out->Type != 3 ) {
            g_Core->p_Console->Print( LOG_ERROR, "ImageLoad(): %s.tga is a corrupted.\n", fileName );
//            PopMemoryFrame( data.tempPool );
            
            return false;
        }
        
        // Ignore useless bytes.
        stream->Read( &sintBad, sizeof(short int), 1 );//fread( &sintBad, sizeof(short int), 1, imgfile );
        stream->Read( &sintBad, sizeof(short int), 1 );//fread( &sintBad, sizeof(short int), 1, imgfile );
        stream->Read( &ucharBad, sizeof(unsigned char), 1 );//fread( &ucharBad, sizeof(unsigned char), 1, imgfile );
        stream->Read( &sintBad, sizeof(short int), 1 );//fread( &sintBad, sizeof(short int), 1, imgfile );
        stream->Read( &sintBad, sizeof(short int), 1 );//fread( &sintBad, sizeof(short int), 1, imgfile );
        
        // Get width and height.
        stream->Read( &out->Width, sizeof(short int), 1 );//fread( &Width, sizeof(short int), 1, imgfile );
        stream->Read( &out->Height, sizeof(short int), 1 );//fread( &Height, sizeof(short int), 1, imgfile );
        
        // Depth.
        stream->Read( &out->BitsPerPixel, sizeof(unsigned char), 1 );//fread( &BitsPerPixel, sizeof(unsigned char), 1, imgfile );
        
        // Skip one byte.
        stream->Read( &ucharBad, sizeof(unsigned char), 1 );//fread( &ucharBad, sizeof(unsigned char), 1, imgfile );
        
        // Color mode.
        colorMode = out->BitsPerPixel / 8;
        imageSize = out->Width * out->Height * colorMode;
        
        // Image data.
        out->ImageData = (Byte *)PushMemory( data.tempPool, sizeof(Byte) * imageSize );
//        memset( out->ImageData, 0x00, imageSize );
        
        // Read the image data.
        stream->Read( out->ImageData, sizeof(Byte), imageSize );
        
        //MakeSeamlessTGA( img );
        for( imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode ) {
            colorSwap = out->ImageData[imageIdx];
            
            out->ImageData[imageIdx] = out->ImageData[imageIdx + 2];
            out->ImageData[imageIdx + 2] = colorSwap;
        }
        
        // Don't free imageData here.
        // Will be freed in material manager.

        // Remove memory frame if a new one was created before.
        if( memory_area == NEKO_NULL ) {
            _PopMemoryFrame( data.tempPool );
        }

        return true;
    }
    
    /**
     *  Load BMP.
     */
    bool CImageLoader::LoadBMP( const char *fileName, SImageInfo * out, SMemoryTempFrame * memory_area ) {
        
        Byte            header[54];
        
        uint32_t        dataPos;
        uint32_t        imageSize;
        
        ncMemoryStream  * stream;
        SPackFile      * dataFile;
        AssetDataPool   data;
        
        
        // Same here, get a file from the package and create
        // a memory stream to get information from it.
        dataFile = g_Core->p_FileSystem->GetPak( "texturedata" );
        data = dataFile->GetData( NC_TEXT( "%s.bmp", fileName ) );
        stream = (ncMemoryStream*)PushMemory( data.tempPool, sizeof(ncMemoryStream) );
        
        // Set image data to our memory stream.
        stream->SetSourceBuffer( &data.tempData );
        // BMP image.

        if( stream == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "ImageLoad(): couldn't find %s image.\n", fileName );
            _PopMemoryFrame( data.tempPool );
            
            return false;
        }
        
        if( stream->Read( header, 1, 54 ) != 54 ) {
            g_Core->p_Console->Print( LOG_ERROR, "ImageLoad(): %s.bmp is a corrupted.\n", fileName );
            _PopMemoryFrame( data.tempPool );
            
            return false;
        }
        
        if( header[0] != 'B' || header[1] != 'M' ) {
            g_Core->p_Console->Print( LOG_ERROR, "ImageLoad(): image %s is not a BMP.\n", fileName );
            _PopMemoryFrame( data.tempPool );
            
            return false;
        }
        
        // Read data from header.
        dataPos = *(int*) & (header[0x0A]);
        imageSize = *(int*) & (header[0x22]);
        out->Width = *(int*) & (header[0x12]);
        out->Height = *(int*) & (header[0x16]);
        
        // Reset to default.
        if( imageSize == 0 )
            imageSize = out->Width * out->Height * 3 /* RGB */;
        
//        if( dataPos == 0 )
//            dataPos = 54;

        // Image data.
        out->ImageData = (Byte*)PushMemory( data.tempPool, sizeof(Byte) * imageSize );
        if( !out->ImageData ) {
            g_Core->p_Console->Error( ERR_FATAL, "Failed to allocate %i bytes for %s image.\n", imageSize, fileName );
            _PopMemoryFrame( data.tempPool );
            
            return false;
        }
        
        // Read image data.
        stream->Read( out->ImageData, 1, imageSize );

        _PopMemoryFrame( data.tempPool );
        
        return true;
    }
    
    /**
     *  Load image.
     */
    bool CImageLoader::LoadImage( const char *filename, EImageInfoType type, SImageInfo * out, SMemoryTempFrame * memory_area )
    {
        switch( type ) {
            case TGA_IMAGE:
               return LoadTGA( filename, out, memory_area );
            case BMP_IMAGE:
                return LoadBMP( filename, out, memory_area );
            default:
                g_Core->p_Console->Error( ERR_ASSET, "LoadImage(): Unknown image type requested.\n" );
                return false;
        }

        return false;
    }
    
    /**
     *  Make image seamless on.
     */
    void SImageInfo::MakeSeamless() {
        Byte * image = (Byte*)ImageData;
        Byte a[4], b[4];
        const char Bpp = BitsPerPixel / 8;
        
        for( unsigned int i = 0; i < Width; ++i ) {
            memcpy( a, &image[i * Bpp], Bpp );
            memcpy( b, &image[( i + ( Height - 1) * Width) * Bpp], Bpp );
            
            for( unsigned int j = 0; j < Bpp; ++j )
                a[j] = ((int)a[j] + (int)b[j]) / 2;
            
            memcpy( &image[i * Bpp], a, Bpp);
            memcpy( &image[ ( i + ( Height - 1 ) * Width ) * Bpp], a, Bpp );
        }
        
        for( unsigned int i = 0; i < Height; ++i ) {
            memcpy( a, &image[i * Width * Bpp], Bpp );
            memcpy( b, &image[(i * Width + Width - 1) * Bpp], Bpp );
            
            for( unsigned int j = 0; j < Bpp; ++j )
                a[j] = ((int)a[j] + (int)b[j]) / 2;
            
            memcpy( &image[i * Width * Bpp], a, Bpp );
            memcpy( &image[( i* Width + Width - 1 ) * Bpp], a, Bpp );
        }
    }
    
    
    /**
     *  Write BMP file.
     */
    const int CImageLoader::WriteBmp( const char *filename, const int width, const int height, const Byte * rgb )
    {
        int i, j, ipos;
        unsigned int bytesPerLine;
        
        Byte * line;
        
        FILE * file;
        struct ncBMPHeader bmph;
        
        // The length of each line must be a multiple of 4 bytes.
        bytesPerLine = (3 * (width + 1) / 4) * 4;
        
        strcpy( bmph.bfType, "BM" );
        
        bmph.bfOffBits = 54;
        bmph.bfSize = bmph.bfOffBits + bytesPerLine * height;
        bmph.bfReserved = 0;
        bmph.biSize = 40;
        bmph.biWidth = width;
        bmph.biHeight = height;
        bmph.biPlanes = 1;
        bmph.biBitCount = 24;
        bmph.biCompression = 0;
        bmph.biSizeImage = bytesPerLine * height;
        bmph.biXPelsPerMeter = 0;
        bmph.biYPelsPerMeter = 0;
        bmph.biClrUsed = 0;
        bmph.biClrImportant = 0;
        
        file = fopen( filename, "wb" );
        if( !file )
            return 0;
        
        fwrite( &bmph.bfType, 2, 1, file );
        fwrite( &bmph.bfSize, 4, 1, file );
        fwrite( &bmph.bfReserved, 4, 1, file );
        fwrite( &bmph.bfOffBits, 4, 1, file );
        fwrite( &bmph.biSize, 4, 1, file );
        fwrite( &bmph.biWidth, 4, 1, file );
        fwrite( &bmph.biHeight, 4, 1, file );
        fwrite( &bmph.biPlanes, 2, 1, file );
        fwrite( &bmph.biBitCount, 2, 1, file );
        fwrite( &bmph.biCompression, 4, 1, file );
        fwrite( &bmph.biSizeImage, 4, 1, file );
        fwrite( &bmph.biXPelsPerMeter, 4, 1, file );
        fwrite( &bmph.biYPelsPerMeter, 4, 1, file );
        fwrite( &bmph.biClrUsed, 4, 1, file );
        fwrite( &bmph.biClrImportant, 4, 1, file );
        
        SMemoryTempFrame * tmp_write = _PushMemoryFrame( pLinearAllocator2 );
        
        line = (Byte*)PushMemory( tmp_write, sizeof(Byte) * bytesPerLine );
        if( !line ) {
            return 0;
        }
        
        for( i = height - 1; i >= 0; --i ) {
            for( j = 0; j < width; ++j ) {
                ipos = 3 * (width * i + j);
                line[3 * j] = rgb[ipos + 2];
                line[3 * j + 1] = rgb[ipos + 1];
                line[3 * j + 2] = rgb[ipos];
            }
            
            fwrite( line, bytesPerLine, 1, file );
        }
        
        _PopMemoryFrame( tmp_write );
//        delete [] line;
        fclose( file );
        
        return(1);
    }
    
    /**
     *  Make image blurry.
     */
    void CImageLoader::MakeBlurry( SImageInfo * img )
    {
        long a, d, n,
        m00, m10, i32;
        m00 = 0;
        
        long int depth = img->GetBitsPerPixel();
        Byte * array = (Byte*)img->GetData();
        
        for( d = 0; d < depth; d++ ) {
            for( a = 0; a < img->GetHeight() - 1; a++ ) {
                for( n = 0; img->GetWidth() - 1; n++ ) {
                    //  m01= m00+1;
                    //  m11= m00+ image_width+1;
                    m10= m00 + img->GetWidth();
                    i32 = ((long)array[m00] + (long)array[m00+1] + (long)array[m10 + 1] + (long)array[m10]) >> 2;
                    
                    if( i32 > 0xFF )
                        array[m00]= (Byte)0xFF;
                    else
                        array[m00]=(Byte)i32;
                    
                    m00 += 1;
                }
            }
        }
    }
    
    /**
     *   Write image.
     */
    bool CImageLoader::CreateImage( const unsigned int width, const unsigned int height, const Byte * data, EImageInfoType type, const char * filename ) {
        
        switch( type ){
            case BMP_IMAGE: {
                FILE *f;
                
                f = fopen( NC_TEXT( "%s/%s.bmp", Filesystem_Path->Get<const char*>(), filename), "wb" );
                if( !f ) {
                    g_Core->p_Console->Error( ERR_ASSET, "CImageLoader::CreateImage - Couldn't create %s image.\n", filename );
                    return false;
                }
                
                // Create new BMP image.
                if( !WriteBmp( filename, width, height, data ) ) {
                    g_Core->p_Console->Print( LOG_ERROR, "CreateImage failed to create %s.bmp\n", filename );
                    return false;
                };
                
                g_Core->p_Console->Print( LOG_INFO, "%s.bmp successfully created.\n", filename );
            }
                break;
            case TGA_IMAGE:
                g_Core->p_Console->Print( LOG_WARN, "CImageLoader::CreateImage(TGA_IMAGE_UNCOMPRESSED) - Implement me!\n" );
                break;
            default:
                g_Core->p_Console->Error( ERR_ASSET, "CImageLoader::CreateImage - Undefined image type given while creating %s.\n", filename );
                break;
                
        }
        
        return true;
    }
    

}