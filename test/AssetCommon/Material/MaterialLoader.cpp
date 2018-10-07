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
//  MaterialLoader.cpp
//  Material manager..
//
//  Created by Neko Vision on 18/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../../Core/Core.h"
#include "../../Core/Streams/BitMessage.h"
#include "../../Core/String/StringHelper.h"
#include "../../Core/Utilities/Utils.h"
#include "../../Graphics/GraphicsManager.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Graphics/Image.h"
#include "../../Platform/Shared/System.h"
#include "../AssetBase.h"
#include "../FileSystem.h"
#include "MaterialLoader.h"


#   if !defined( NEKO_SERVER )

namespace Neko {
    
    /**
     *   Material manager instance.
     */
    CMaterialFactory::CMaterialFactory()
    {
        
    }
    
    /**
     *  Destructor.
     */
    CMaterialFactory::~CMaterialFactory()
    {
        
    }
    
    /**
     *  Create a new material using API depended methods.
     *
     *  @param material  A material to create.
     *  @param imageData Image data.
     */
    SMaterial * CMaterialFactory::CreateMaterial( const uint8_t * imageData,
                                                 const uint32_t width,
                                                 const uint32_t height, TextureTarget textureType, ETextureStorageType textureDataMode, ETextureFormat internalFormat, ETextureFormat externalFormat, ETextureTile tiling, ETextureFilter filter, const uint32_t bitsPerPixel, bool hasMipmap, const int32_t maxLayers, const float LodBias )
    {
        SImageInfo  info;
        memset( &info, 0x00, sizeof(SImageInfo) );
        
        info.Width = width;
        info.Height = height;
        info.ImageData = (uint8_t *)imageData;
        info.BitsPerPixel = bitsPerPixel; // TODO: 32 for Metal API!!!
        info.HasMipmaps = hasMipmap;        // 'false' by default.
        // TextureArray, Cubemap.
        info.Tex3DLayerCount = maxLayers;
        info.LodBias = LodBias;
        
        // Tiling modes.
        // TODO: differences.
        info.TileWrapS = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTile( tiling );
        info.TileWrapT = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTile( tiling );
        
        
        // API requesters.
        info.DataType = g_pGraphicsManager->GetCurrentInterface()->GetAPIStorageType( textureDataMode );
        info.Type = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTarget( textureType );
        
        info.InternalFormat = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureFormat( internalFormat );
        info.ExternalFormat = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureFormat( externalFormat );
        
        info.Filter = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureFilteringMode( filter );
        
        //        // Calculate bits per pixel.
        //        const uint32_t bitsPerPixelN8 = info.BitsPerPixel / 8;
        //        const uint32_t mode = bitsPerPixel == 4 ? GL_RGBA : GL_RGB;
        
        // Create a new material entry.
        SMaterial * material = (SMaterial *) pMaterialAllocator->Alloc( sizeof(SMaterial) );
        // Never ever use TextureId.
        material->Image = info;
        material->m_iAPIIndex = g_pGraphicsManager->GetCurrentInterface()->CreateTexture( &material->Image );
        
        return material;
    }
   
    /**
     *  Unloads material.
     */
    void CMaterialFactory::UnloadMaterial( SMaterial * mat )
    {
        if( mat == NEKO_NULL ) {
            return;
        }
        
        mat->Image.Unload();
        memset( &mat->Image, 0x0, sizeof(SImageInfo) );
        pMaterialAllocator->Dealloc( mat );
        
        mat = NEKO_NULL;
    }
    
    /**
     *  Initialize material system.
     */
    void CMaterialFactory::Initialize( INekoAllocator * allocator )
    {
        // DEVNOTE: MAKE SURE THAT ALL CUSTOM MATERIALS WERE ADDED TO CACHE LIST.
        
        SMemoryTempFrame  * tempMemory;
        Vec3   * noiseBuffer = NEKO_NULL;
        Byte    * ci_data1 = NEKO_NULL;
        Byte    * noise3dFloat = NEKO_NULL;
        
        int32_t  i, j, c;
        
        g_Core->p_Console->Print( LOG_INFO, "Material asset factory initializing..\n" );
        
        g_Core->p_Console->Print( LOG_INFO, "Creating material asset pool with size of %llu mb..\n", ByteInMegabyte(kMaterialAssetPoolSize) );
        
        pAllocatorHandle = allocator;
        
        // a new material pool to keep materials
        pAllocator = NekoAllocator::newPoolAllocator( sizeof(SMaterialProp), __alignof(SMaterialProp), kMaterialAssetPoolSize, *pAllocatorHandle );
        // Create a new material pool to keep SMaterial entries.
        pMaterialAllocator = NekoAllocator::newPoolAllocator( sizeof(SMaterial), __alignof(SMaterial), kMaterialAssetPoolSize, *pAllocatorHandle );
        
        
        g_Core->p_Console->Print( LOG_INFO, "Creating base materials\n" );
        
        // Create material cache to store our materials.
        m_materialCache.Create( MAX_MATERIALS, pAllocator, pLinearAllocator2 );
        
        tempMemory = _PushMemoryFrame( pLinearAllocator2 );
        //  Create checker texture ( null texture ).
        // Thanks to kimes for checker texture generate method.
        ci_data1 = (Byte *)PushMemory( tempMemory, sizeof(Byte) * NULL_TEXTURE_SIZE * NULL_TEXTURE_SIZE * 4 );
        
        for( i = 0; i < NULL_TEXTURE_SIZE; ++i ) {
            for( j = 0; j < NULL_TEXTURE_SIZE; ++j ) {
                c = ((( ( i & 0x8 ) == 0 ) ^ (( ( j & 0x8 )) == 0 ))) * 255;
                ci_data1[(i * 256) + (j * 4)] = 0;
                ci_data1[(i * 256) + (j * 4) + 1] = c;
                ci_data1[(i * 256) + (j * 4) + 2] = 0;
                ci_data1[(i * 256) + (j * 4) + 3] = 128;
            }
        }

        nullMaterial = (SMaterialProp *)pAllocator->Alloc( sizeof( SMaterialProp ) );
        nullMaterial->m_pDiffuse = CreateMaterial( (uint8_t*)ci_data1, NULL_TEXTURE_SIZE, NULL_TEXTURE_SIZE, TextureTarget::Texture2D );
        nullMaterial->m_pNormal = nullMaterial->m_pDiffuse; // -.-
        
        m_materialCache["null"] = nullMaterial;
        
        g_Core->p_Console->Print( LOG_INFO, "Generating procedural random noise texture..\n" );
        
        // Generate noise kernel.
        float   Scale;
        
        for( i = 0; i < KERNEL_SIZE; ++i ) {
            noiseKernel[i] = Vec3( 2.0f * (float)rand() / RAND_MAX - 1.0f,
                                   2.0f * (float)rand() / RAND_MAX - 1.0f,
                                   (float)rand() / RAND_MAX );
            noiseKernel[i].Normalize();
            
            Scale = (float)(i / KERNEL_SIZE);
            Scale = nkMath::Lerpf(0.1f, 1.0f, Scale * Scale);
            
            noiseKernel[i] = noiseKernel[i] * Scale;
        }
        
        // Generate noise texture.
        noiseBuffer = (Vec3 *)PushMemory( tempMemory, sizeof(Vec3) * NOISE_SIZE * NOISE_SIZE );
        
        for( i = 0; i < NOISE_SIZE * NOISE_SIZE; ++i ) {
            Vec3 NoiseValue = Vec3 ( 2.0f * (float)rand() / RAND_MAX - 1.0f,
                                      2.0f * (float)rand() / RAND_MAX - 1.0f, 0.0f );
            
            noiseBuffer[i] = NoiseValue;
            noiseBuffer[i].Normalize();
        }
        
        // Create a noise material.
        noiseMaterial = (SMaterialProp *)pAllocator->Alloc( sizeof( SMaterialProp ) );
        
        // Fill the material with image data.
        noiseMaterial->m_pDiffuse = CreateMaterial( (uint8_t *)noiseBuffer, NOISE_SIZE, NOISE_SIZE, TextureTarget::Texture2D, ETextureStorageType::Float, ETextureFormat::RGBA, ETextureFormat::RGBA, ETextureTile::Repeat, ETextureFilter::Linear, 32 );
        noiseMaterial->bAvailable = true;
        
        m_materialCache["noisemap"] = noiseMaterial;
        
        // Generate 3d noise.
        double * noise = (double *)PushMemory( tempMemory, sizeof(double) * noiseWidth * noiseHeight * noiseDepth );
        
        // Fill random pattern.
        for( i = 0; i < noiseWidth; ++i) {
            for( j = 0; j < noiseHeight; ++j) {
                for( c = 0; c < noiseDepth; ++c) {
                    noise[i + j + c] = (rand() % 32768) / 32768.0;
                }
            }
        }
        
        noise3dFloat = (Byte *)PushMemory( tempMemory, sizeof(Byte) * NOISE3D_TEXTURE_SIZE * NOISE3D_TEXTURE_SIZE * 4 );
        
        for( i = 0; i < NOISE3D_TEXTURE_SIZE; ++i ) {
            for( j = 0; j < NOISE3D_TEXTURE_SIZE; ++j ) {
                c = int32_t(256 * NekoUtils::NoiseOperations::turbulence(i, j, g_Core->GetTime() / 40, 128, noise));
                noise3dFloat[(i * 256) + (j * 4)] = c;
                noise3dFloat[(i * 256) + (j * 4) + 1] = c;
                noise3dFloat[(i * 256) + (j * 4) + 2] = c;
                noise3dFloat[(i * 256) + (j * 4) + 3] = 128;
            }
        }
        
        // Create a 3d noise material.
        noise3dMaterial = (SMaterialProp *)pAllocator->Alloc( sizeof( SMaterialProp ) );
        noise3dMaterial->m_pDiffuse = CreateMaterial( (uint8_t *)noise3dFloat, NOISE3D_TEXTURE_SIZE, NOISE3D_TEXTURE_SIZE, TextureTarget::Texture2D );
        noise3dMaterial->bAvailable = true;
        
        m_materialCache["noise3d"] = noise3dMaterial;
        // Everything went ok.
        
        _PopMemoryFrame( tempMemory );
    }
    
    /**
     *  Load textures for material.
     */
    SMaterial   * CMaterialFactory::LoadImage( const char * material_name, const int16_t tileFlag, const int16_t miscFlags, bool threaded, SMemoryTempFrame * memoryPool )
    {
        SMaterial      * tempImage = NEKO_NULL;
        
        // Allocate an image.
        tempImage = (SMaterial *)pMaterialAllocator->Alloc( sizeof(SMaterial) );
        
        // Load an Image.
        memset( &tempImage->Image, 0x00, sizeof(SImageInfo) );
        
        // Load image from file.
        if( g_imageManager->LoadImage( material_name, TGA_IMAGE, &tempImage->Image, memoryPool ) ) {
            // Update renderer information.
//            g_mainRenderer->videoMemoryInfo.textureMemoryUsed += materialPool->Used;
            
            // Gene
            // Upload to the GPU.
            
            // By this stage, tempImage already got all properties about texture.
            
            tempImage->Image.HasMipmaps = (bool)(miscFlags & 0x1);
            
            // API requesters.
            tempImage->Image.DataType = g_pGraphicsManager->GetCurrentInterface()->GetAPIStorageType( ETextureStorageType::Uchar );
            tempImage->Image.Type = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTarget( TextureTarget::Texture2D );
            tempImage->Image.Filter = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureFilteringMode( ETextureFilter::Linear );
            
            // Calculate bits per pixel.
            const uint32_t bitsPerPixelN8 = tempImage->Image.BitsPerPixel / 8;
            const ETextureFormat mode = bitsPerPixelN8 == 4 ? ETextureFormat::RGBA : ETextureFormat::RGB;
            
            // Color mode.
            tempImage->Image.InternalFormat = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureFormat( mode );
            tempImage->Image.ExternalFormat = tempImage->Image.InternalFormat;  // TEMP!
            
            // Tiling modes.
            // TODO: differences.
            tempImage->Image.TileWrapS = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTile( (ETextureTile)tileFlag );
            tempImage->Image.TileWrapT = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTile( (ETextureTile)tileFlag );
            
            if( !threaded ) {
                // Create the image using graphical API.
                tempImage->m_iAPIIndex = g_pGraphicsManager->GetCurrentInterface()->CreateTexture( &tempImage->Image );
            } else {
                tempImage->m_iAPIIndex = 0;
            }
            
            // 32/8 - 4 chan ( alpha ) - RGBA
            // 24/8 - 3 chan - RGB
            
            // Remove all loaded images here.
            // Unload image data only!
            
            //            m_materialCache[material_name] = tempImage;
        } else {
            // Error.
            g_Core->p_Console->Print( LOG_ERROR, "MaterialManager::LoadImage() - couldn't load \"%s\" material, using default instead.\n", material_name );
            m_materialCache[material_name] = nullMaterial;
        }
        
        return tempImage;
    }
    
    /**
     *  Shutdown and delete all materials. Must be called only once.
     */
    void CMaterialFactory::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "Material base unitializing..\n" );
        
        UnloadAllMaterials();
        
        g_Core->p_Console->Print( LOG_INFO, "Deleting material asset pools..\n" );;
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pAllocator, pAllocatorHandle );
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pMaterialAllocator, pAllocatorHandle );
    }
    
    /**
     *  Unload and delete all materials.
     */
    void CMaterialFactory::UnloadAllMaterials()
    {
        g_Core->p_Console->Print( LOG_INFO, "Unloading all materials...\n" );
        
        SMaterialProp * mat = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_materialCache.m_List.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            mat = (SMaterialProp *)cur->m_ptrData;
            
            mat->Unload(); // Delete texture.
            
            // No checks here, if there's no diffuse map on load then it could throw error.
            pMaterialAllocator->Dealloc( mat->m_pDiffuse );
            mat->m_pDiffuse = NEKO_NULL;
            
            if( mat->m_pNormal ) {
                pMaterialAllocator->Dealloc( mat->m_pNormal );
                mat->m_pNormal = NEKO_NULL;
            }
            
            if( mat->m_pSpecular ) {
                pMaterialAllocator->Dealloc( mat->m_pSpecular );
                mat->m_pSpecular = NEKO_NULL;
            }
            
            pAllocator->Dealloc( mat );
            mat = NEKO_NULL;
        }
    
        // Delete material cache.
        // @note - Never ever delete this material cache from the memory, it should stay here until application quit.
        m_materialCache.Delete();
    }
    
    /**
     *  Create texture array.
     *
     *  Uses first(null) index in info properties as the MAIN material information.
     *  Writes information to 'diffuse' channel.
     */
    SMaterialProp * CMaterialFactory::CreateTextureArray( const char * name, SMaterialPropArray *info, SMemoryTempFrame  * textureMemory )
    {
        // Sanity checks.
        if( info->props[0] == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_INFO, "CreateTextureArray: Empty material properties.\n" );
            
            return NEKO_NULL;
        }

        int32_t     i;
        
        SMaterial   * materialDiffuse = NEKO_NULL;
        SMaterial   * materialNormal = NEKO_NULL;    // some of materials can be missing ( or just not used )
        SMaterial   * materialPBR = NEKO_NULL;
        
        SMaterialProp * materialprop = NEKO_NULL;
        SMaterialProp * prop = NEKO_NULL;
        
        // Use original width and height.
        const int32_t iWidth = info->m_iWidth;
        const int32_t iHeight = info->m_iHeight;
        
        // Create material base.
        materialDiffuse = CreateMaterial( NEKO_NULL, iWidth, iHeight, TextureTarget::Texture2DArray, ETextureStorageType::Uchar, ETextureFormat::RGBA, ETextureFormat::RGBA, ETextureTile::Repeat, ETextureFilter::Linear, 32, false, info->m_iLayers );
        
        // Normal maps..
        if( (info->m_iFlags & (int32_t)EMaterialTypeFlag::Normal) ) {
            materialNormal = CreateMaterial( NEKO_NULL, iWidth, iHeight, TextureTarget::Texture2DArray, ETextureStorageType::Uchar, ETextureFormat::RGBA, ETextureFormat::RGBA, ETextureTile::Repeat, ETextureFilter::Linear, 32, false, info->m_iLayers );
        }
        
        // PBR maps..
        if( (info->m_iFlags & (int32_t)EMaterialTypeFlag::RMA) ) {
            materialPBR = CreateMaterial( NEKO_NULL, iWidth, iHeight, TextureTarget::Texture2DArray, ETextureStorageType::Uchar, ETextureFormat::RGBA, ETextureFormat::RGBA, ETextureTile::Repeat, ETextureFilter::Linear, 32, false, info->m_iLayers );
        }
        
//        glPixelStorei( GL_PACK_ALIGNMENT, 1 );
        
        for( i = 0; i < info->m_iLayers; ++i ) {
            // Materials can be big enough, so create separated memory pools.
            prop = LoadFromFile( info->props[i], false );

            // Get texture image data using graphical API.
#   if defined( USES_OPENGL )
   
            // Diffuse.
            glBindTexture( GL_TEXTURE_2D_ARRAY, materialDiffuse->Image.GetId() );
            glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, prop->m_pDiffuse->Image.Width, prop->m_pDiffuse->Image.Height, 1, GL_RGB, GL_UNSIGNED_BYTE, prop->m_pDiffuse->Image.ImageData );
            
            // Normal.
            if( (info->m_iFlags & (int32_t)EMaterialTypeFlag::Normal) ) {
                if( prop->m_pNormal != NEKO_NULL ) {
                    glBindTexture( GL_TEXTURE_2D_ARRAY, materialNormal->Image.GetId() );
                    glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, prop->m_pNormal->Image.Width, prop->m_pNormal->Image.Height, 1, GL_RGB, GL_UNSIGNED_BYTE, prop->m_pNormal->Image.ImageData );
                } else {
                    UnloadMaterial( materialNormal );
                }
            }
            
            // PBR map.
            if( (info->m_iFlags & (int32_t)EMaterialTypeFlag::RMA) ) {
                if( prop->m_pSpecular != NEKO_NULL ) {
                    glBindTexture( GL_TEXTURE_2D_ARRAY, materialPBR->Image.GetId() );
                    glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, prop->m_pSpecular->Image.Width, prop->m_pSpecular->Image.Height, 1, GL_RGB, GL_UNSIGNED_BYTE, prop->m_pSpecular->Image.ImageData );
                } else {
                    UnloadMaterial( materialPBR );
                }
            }
#   else
            // Implement me!
#   endif
            if( prop->m_pTempMemory != NEKO_NULL ) {
                _PopMemoryFrame( prop->m_pTempMemory );
            }
            
            // Unload previous material data if requested.
            if( info->m_bUnloadOriginals ) {
                prop->Unload();
                
                pAllocator->Dealloc( prop );
                prop = NEKO_NULL;
            }
            
        }
        
        
        // Note: glTexSubImage3D requires texture completeness. That's why we generate mipmaps now.
        if( info->m_bMipmapped ) {
            materialDiffuse->MakeMipmaps();
            materialNormal->MakeMipmaps();
            materialPBR->MakeMipmaps();
        }
        
        // Create the material.
        materialprop = (SMaterialProp *)pAllocator->Alloc( sizeof(SMaterialProp) );
        materialprop->m_pNormal = materialNormal;
        materialprop->m_pSpecular = materialPBR;
        materialprop->m_pDiffuse = materialDiffuse;
        
        materialprop->bAvailable = true;
        // Add material to cache.
        m_materialCache[name] = materialprop;
        
        return materialprop;
    }
    
    /**
     *  Load material.
     */
    SMaterialProp * CMaterialFactory::LoadFromFile( const char * name, const bool freesMemory, const bool threaded, void * ptr )
    {
#   if defined( NEKO_DEBUG )
        int32_t     t1, t2;
        t1 = g_Core->p_System->Milliseconds();
#   endif

        int16_t     materialFlags, materialTile;
        
        SMaterial   * prop0 = NEKO_NULL, * prop1 = NEKO_NULL, * prop2 = NEKO_NULL;
        SMaterialProp   * materialInfo = NEKO_NULL;
        
        // Package file which contains asset list file.
        SPackFile  * assetShared = NEKO_NULL;
        // Asset data.
        AssetDataPool   assetListData;
        SMemoryTempFrame * texturePool = NEKO_NULL;
        
        // Get a package with scripts including our asset list as *.csv file.
        assetShared = g_Core->p_FileSystem->GetPak( "shared_data" );
        // If we got something..
        assetListData = assetShared->GetData( NC_TEXT( "materials/%s", name ), NEKO_NULL );
        if( assetListData.tempData == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_INFO, "Couldn't find \"%s\" material.\n", name );
            
            _PopMemoryFrame(assetListData.tempPool);
            
            return nullMaterial;
        }
        
//        // Close previous memory pool to prevent leaks.
//        _PopMemoryFrame( assetListData.tempPool );
        
        /**
             Material file architecture:
             
             [header: mat]
             [string: diffusemap]
             [string: normalmap]
             [string: RMA(PBR)map]
             
             [bool: hasmipmaps?]
             [enum: tile mode]
             [end header: 0xff]
         
         **/
        
        // Read material contents.
        SBitMessage bitReader( assetListData.tempData, 2048, true );
        bitReader.BeginReading();
        
        // Check header ( sanity check ).
        if( bitReader.ReadByte() != 0x07 ) {
            g_Core->p_Console->Print( LOG_WARN, "Asset of type material \"%s\" has wrong header.\n", name );
            return NEKO_NULL;
        }
        
        texturePool = assetListData.tempPool;// ??
  
        materialFlags = 0x1;
        materialTile = 0x0;
        
        Byte  cmd;
        do {
            cmd = bitReader.ReadByte();
            
            switch( cmd ) {
                    
                    // Diffuse map.
                case 0x02:
                    // Diffuse.
                    prop0 = LoadImage( bitReader.ReadString(), materialTile, materialFlags, threaded, texturePool );
                    
                    break;
                    // Normal map.
                case 0x04:
                    // Normal.
                    prop1 = LoadImage( bitReader.ReadString(), materialTile, materialFlags, threaded, texturePool );
                    
                    break;
                    // PBR map.
                case 0x08:
                    // PBR.
                    prop2 = LoadImage( bitReader.ReadString(), materialTile, materialFlags, threaded, texturePool );
                    
                    break;
                    
                    
                    // Material tile mode.
                case 0x16:
                    materialTile   = (int16_t)bitReader.ReadByte();
                    break;
                    // Material mipmap mode.
                case 0x32:
                    materialFlags   |= (int16_t)bitReader.ReadByte();
                    break;
            }
            
        } while( cmd != 0xff );
   
        // Create a new material.
        if( threaded && ptr != NEKO_NULL ) {
            materialInfo = (SMaterialProp *)ptr;
        } else {
            materialInfo = (SMaterialProp *)pAllocator->Alloc( sizeof(SMaterialProp) );
        }
        
        materialInfo->m_Name = name;
        materialInfo->m_pDiffuse = prop0;
        materialInfo->m_pNormal = prop1;
        materialInfo->m_pSpecular = prop2;
        materialInfo->bAvailable = !threaded;  // Will be set later.
        
        if( freesMemory ) {
            _PopMemoryFrame(assetListData.tempPool);
            materialInfo->m_pTempMemory = NEKO_NULL;
        } else {
            materialInfo->m_pTempMemory = texturePool;
        }
        
        if( !threaded ) {
            // Set a new material cache entry.
            m_materialCache[name] = materialInfo;
        }
        
#   if defined( NEKO_DEBUG )
        t2 = g_Core->p_System->Milliseconds();
        
        g_Core->p_Console->Print( LOG_INFO, "Material asset \"%s\" loaded in %i ms\n ", name, t2 - t1 );
#   endif
        
        return materialInfo;
    }
    
    /**
     *  Find material.
     */
    SMaterialProp * CMaterialFactory::Find( const char * entry )
    {
        // Hash will return 0 if material is not found in hashmap.
        // '0' stands for empty checker texture.
        SMaterialProp *whatIGet = m_materialCache[entry];
        
        if( whatIGet == NEKO_NULL ) {
            return (SMaterialProp *)f_AssetBase->LoadAssetAsynchronously( EAssetType::Material, entry );
            
//            return nullMaterial;
        }
        
        
        return whatIGet; // Just to be sure.
        
        // Texture wasn't found, so use checker texture instead.
        //return &m_Materials[0];
    }
    
    /**
     *  Set tile mode.
     */
    void SMaterial::SetTileMode( ETextureTile tile )
    {
        uint32_t tileAPI = g_pGraphicsManager->GetCurrentInterface()->GetAPITextureTile( tile );
        
#   if defined( USES_OPENGL )
        glTexParameterf( (GLenum)Image.Type, GL_TEXTURE_WRAP_S, tileAPI );
        glTexParameterf( (GLenum)Image.Type, GL_TEXTURE_WRAP_T, tileAPI );
#   endif
    }
    
    /**
     *  Set texture comparesment mode.
     */
    void SMaterial::CompareRefToTexture( bool enable, Neko::ECompareMode mode )
    {
        uint32_t modeAPI = g_pGraphicsManager->GetCurrentInterface()->GetAPICompareMode( mode );
        
        glBindTexture( (GLenum)Image.Type, Image.TextureID );
        
        if( enable ) {
            glTexParameteri( (GLenum)Image.Type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
            glTexParameteri( (GLenum)Image.Type, GL_TEXTURE_COMPARE_FUNC, modeAPI );
        } else {
            glTexParameteri( (GLenum)Image.Type, GL_TEXTURE_COMPARE_MODE, GL_NONE );
        }
        
        glBindTexture( (GLenum)Image.Type, 0 );
    }
    
    /**
     *  Set texture comparesment mode.
     */
    void SMaterial::SetAnisotropy( const int32_t value )
    {
        glBindTexture( (GLenum)Image.Type, Image.TextureID );
        
        // Set anisotropy level.
        glTexParameteri( (GLenum)Image.Type, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, value );
        glBindTexture( (GLenum)Image.Type, 0 );
    }
    
    /**
     *  Generate mipmaps.
     */
    void SMaterial::MakeMipmaps()
    {
        // Bind material and change filtering.
        g_pGraphicsManager->GetCurrentInterface()->GenerateMipmaps( (GLuint)Image.TextureID, (GLenum)Image.Type );
    }
    
    /**
     *  Unload all materials.
     */
    void  SMaterialProp::Unload()
    {
        if( m_pDiffuse ) {
            m_pDiffuse->Image.Unload();
        }
        
        if( m_pNormal ) {
            m_pNormal->Image.Unload();
        }
        
        if( m_pSpecular ) {
            m_pSpecular->Image.Unload();
        }
    }
}
#endif // NEKO_SERVER
