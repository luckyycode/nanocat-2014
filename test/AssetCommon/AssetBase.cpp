//
//          *                  *
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
//  AssetBase.cpp
//  Main game asset manager.. :>
//
//  This code is a part of Neko engine.
//  Created by Neko Vision on 1/22/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../Core/Core.h"
#include "../Core/Player/Camera/Camera.h"   // Shader defines.
#include "../Core/Streams/MemoryStream.h"
#include "../Core/Streams/Streams.h"
#include "../Core/Utilities/Utils.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Platform/Shared/System.h"
#include "../World/Mesh.h"
#include "AssetBase.h"
#include "Material/MaterialLoader.h"
#include "Sound/SoundManager.h"


// TODO: asset timeout
// TODO: hunk asset loadout

namespace Neko {
    
    CAssetManager * f_AssetBase = 0;
    
    static const char * GetAssetTypeName( EAssetType type );
    
    /**
     *  Load asset using asset streamer.
     *
     *  @param type Asset type.
     *  @param name Asset name.
     */
    void *CAssetManager::LoadAssetAsynchronously( EAssetType type, const char *name )
    {
        // Check if asset is already in memory or loading..
        TAssetPtr * asset = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_assetsToLoad.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            asset = (TAssetPtr *)cur->m_ptrData;
            
            // Check if assets needs load and if that even exists.
            if( !strcmp( asset->sName.c_str(), name ) ) {    // not needed to check if asset was fully loaded, we already gave it a pointer
                g_Core->p_Console->Print( LOG_INFO, "AssetStream: Requested asset \"%s\" of type \"%s\" is in the progress\n", name, GetAssetTypeName(type) );
                return asset;
            }
        }
        
        // Now check if asset was loaded before
        head = &m_pLoadedAssets.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            asset = (TAssetPtr *)cur->m_ptrData;
            
            if( !strcmp( asset->sName.c_str(), name ) ) {
                g_Core->p_Console->Print( LOG_INFO, "AssetStream: Requested asset \"%s\" of type \"%s\" is already loaded\n", name, GetAssetTypeName(type) );
                return asset;
            }
        }
        
        // Precache empty asset, just create an entry and add to its bases
        if( type == EAssetType::Material ) {
            asset = (SMaterialProp *)p_MaterialBase->pAllocator->Alloc( sizeof(SMaterialProp) );
            asset->bAvailable = false;
            asset->ptr = NEKO_NULL;
            
            // Create cache entry.
            p_MaterialBase->m_materialCache[name] = (SMaterialProp *)asset;
        } else if( type == EAssetType::Mesh ) {
            asset = (ncMesh *)p_MeshBase->pMeshEntAllocator->Alloc( sizeof(ncMesh) );
            asset->bAvailable = false;
            ncMesh * fuckyou = (ncMesh *)asset;
            
            CMesh * entry = (CMesh *)p_MeshBase->pMeshAllocator->Alloc(sizeof(CMesh));
            asset->ptr = (void *)entry;
            fuckyou->m_hOwner = entry;
            
            
            // Create cache entry.
            p_MeshBase->m_pMeshCache[name] = (ncMesh *)asset;
        }
        
        asset->sName = name;
        asset->eType = type;
        asset->bNeedsLoad = true;    // Set asset flag.
        asset->iTime = g_Core->GetTime();
        
        m_pAssetQueue->Put( (void *)asset );
        
        SList::AddTail( &m_assetsToLoad, &asset->m_Link2, (void *)asset );
        
        return (void *)asset;     //so we can use this address later where we need to
    }
    
    /**
     *  Update asset manager on the main ( base ) thread!
     */
    void CAssetManager::Frame( uint32_t msec, bool isMainThread )
    {
        SLink   * head;
        SLink   * cur;
        SLink   * next;
        
        TAssetPtr    * asset = NEKO_NULL;
        
//        m_pLoaderLock->Lock();
        
        head = &m_pLoadedAssets.m_sList;
        cur = head->m_pNext;
        
        // Delete objects.
        while( cur != head ) {
            next = cur->m_pNext;
            
            asset = (TAssetPtr *)cur->m_ptrData;
            
            // Check if assets needs load and if that even exists.
            if( asset->bNeedsLoad ) {
                
                switch( asset->eType ) {
                        
                    case EAssetType::Mesh: {
                        g_Core->p_Console->Print( LOG_INFO, "AssetStream: Creating \"%s\" (type: mesh)\n", asset->sName.c_str() );
                        
                        ncMesh * mesh = (ncMesh *)asset;
                        mesh->m_hOwner->MakeBuffers();
                        
                        asset->bNeedsLoad = false;
                        asset->bAvailable = true;
                    }
                        break;
                        
                    case EAssetType::Sound:
                        
                        break;
                        
                    case EAssetType::Shader:
                        // should be loaded at start
                        break;
                        
                    case EAssetType::Material: {
                        g_Core->p_Console->Print( LOG_INFO, "AssetStream: Creating \"%s\" (type: material)\n", asset->sName.c_str() );
                        
                        SMaterialProp * assetInfo = (SMaterialProp * ) asset;
                        
                        // GraphicsInterface is only available on the renderer thread.
                        assetInfo->m_pDiffuse->m_iAPIIndex = g_pGraphicsManager->GetCurrentInterface()->CreateTexture( &assetInfo->m_pDiffuse->Image );
                        
                        // Load normal map.
                        if( assetInfo->m_pNormal ) {
                            assetInfo->m_pNormal->m_iAPIIndex = g_pGraphicsManager->GetCurrentInterface()->CreateTexture( &assetInfo->m_pNormal->Image );
                        }
                        
                        // Load specular map.
                        if( assetInfo->m_pSpecular ) {
                            assetInfo->m_pSpecular->m_iAPIIndex = g_pGraphicsManager->GetCurrentInterface()->CreateTexture( &assetInfo->m_pSpecular->Image );
                        }
                        
                        asset->bAvailable = true;
                        asset->bNeedsLoad = false;
                    }
                        break;
                }
                
                SList::RemoveAt( &m_assetsToLoad, &asset->m_Link2 );
                
            }
            
            
            cur = next;
        }
        
        
        // Check if we have something in the asset list..
        if( m_pAssetQueue->GetCount() > 0 ) {
            // Call the build event.
            m_pLoaderEvent->Signal();
        }
        
//        m_pLoaderLock->Unlock();
    }
    
    /**
     *  Main asset loader thread.
     */
    static void AssetLoadThread( INekoThread * owner, void * arg, void * args )
    {
        CAssetManager * base = (CAssetManager *) arg;
        bool continueLoad = false;
        
        while( base->LoadCache /* Is asset manager active yet? */ ) {
            base->m_pLoaderEvent->Wait( 100.0 );
            
            while( true ) {
                base->m_pLoaderLock->Lock();
                
                continueLoad = base->ReloadThreadedAsset();
                
                base->m_pLoaderLock->Unlock();
                
                // Are there any assets to load left?
                if( continueLoad == true ) {
                    break;
                }
            }
        }
    }
    
    /**
     *  Reload threaded asset.
     */
    bool CAssetManager::ReloadThreadedAsset()
    {
        bool reload = false;
        uint32_t     timeStart, timeEnd;
        
        // Check if we have something to build.
        if( m_pAssetQueue->GetCount() > 0 ) {
            TAssetPtr * asset = NEKO_NULL;
            
            // Request last asset in the list.
            if(m_pAssetQueue->Get( (void **)&asset ) != 0) {
                return true;
            }
        
            // Check if current asset still needs to be initialized..
            if( asset && asset->bNeedsLoad ) {
                timeStart = g_Core->p_System->Milliseconds();
                
                // That's the place where we should load asset from disk/whatever.
                switch( asset->eType ) {
                    case EAssetType::Material:
                        // Load the material from source.
                        p_MaterialBase->LoadFromFile( asset->sName, true, true, asset );
                        // Add to cache list.
                        SList::AddHead( &m_pLoadedAssets, &asset->m_Link, asset );
                        timeEnd = g_Core->p_System->Milliseconds();
                        
                        g_Core->p_Console->Print( LOG_INFO, "Waited %i msec for asset load of type \"material\"\n", timeEnd - timeStart );
                        break;
                        
                    case EAssetType::Mesh:
                        p_MeshBase->Load( asset->sName, true, true, asset, asset->ptr );
                        // Add to cache list.
                        SList::AddHead( &m_pLoadedAssets, &asset->m_Link, (void *)asset );
                        timeEnd = g_Core->p_System->Milliseconds();
                        
                        g_Core->p_Console->Print( LOG_INFO, "Waited %i msec for asset load of type \"mesh\"\n", timeEnd - timeStart );
                        break;
                        
                    case EAssetType::Sound:
                        
                        break;
                        
                    case EAssetType::Shader:
                        // shaders should be loaded at engine launch
                        break;
                }
            }
            
            
            // If we still have something to build..
            reload = (m_pAssetQueue->GetCount() > 0);
        }
        
        
        return reload;
    }
    
    /**
     *  Asset type to string, eh.
     */
    static const char * GetAssetTypeName( EAssetType type )
    {
        switch( type ) {
            case EAssetType::Material:
                return "material";
            case EAssetType::Mesh:
                return "mesh";
            case EAssetType::Sound:
                return "sfx";
        }
        
        return "(unknown)";
    }
    
    static const uint64_t   kAssetPoolPreloadSize   = Megabyte(24);
    
    /**
     *  Initialize asset system & load some assets..
     */
    void CAssetManager::Initialize( INekoAllocator  * allocator )
    {
        SMemoryTempFrame  * memoryPool = NEKO_NULL;
        SMaterialPropArray  materialArrayInfo;
        
        // Some information..
        g_Core->p_Console->Print( LOG_INFO, "Asset manager initializing...\n" );
        g_Core->p_Console->Print( LOG_INFO, "Creating asset loader thread..\n" );
        
        LoadCache = true;
        pAllocator = allocator;
        pPoolAllocator = NekoAllocator::newPoolAllocator( sizeof(TAssetPtr), __alignof(TAssetPtr), kAssetPoolPreloadSize, *allocator );
        
        // Create loader properties.
        m_pLoaderEvent = CCore::CreateEvent();
        m_pLoaderLock = CCore::CreateLock();
        m_pLoaderThread = CCore::CreateThread( INekoThread::PRIORITY_NORMAL, 2, AssetLoadThread, this, NEKO_NULL );
        
        // Initialize asset streaming queue.
        m_pAssetQueue = (CQueue *)pAllocator->Alloc( sizeof(CQueue) );
        m_pAssetQueue->Create();
        
        // Loaded assets cache.
        SList::CreateList( &m_pLoadedAssets );
        SList::CreateList( &m_assetsToLoad );
        
        // Initialize material manager.
        p_MaterialBase = (CMaterialFactory *)pClassLinearAllocator->Alloc( sizeof(CMaterialFactory) );
        p_MaterialBase->Initialize( pMainAllocProxy );
        
        // Initialize shader manager.
        p_ShaderBase = (CShaderFactory *)pClassLinearAllocator->Alloc( sizeof(CShaderFactory) );
        p_ShaderBase->Initialize( pMainAllocProxy );
        
        CRenderer::CheckAPIErrors( "Asset load", __LINE__ );
        
        LoadShaders();
        
        // Precache some materials.
        p_MaterialBase->LoadFromFile( "ui/font" );
//        p_MaterialBase->LoadFromFile( "lensdirt" );
        p_MaterialBase->LoadFromFile( "moon_albedo" );
//        p_MaterialBase->LoadFromFile( "road_def" );
//        p_MaterialBase->LoadFromFile( "sea_foam" );
//        p_MaterialBase->LoadFromFile( "water1_n" );
//        p_MaterialBase->LoadFromFile( "filmlut" );
        p_MaterialBase->LoadFromFile("panelorange");
//        p_MaterialBase->LoadFromFile("road01");
//        p_MaterialBase->LoadFromFile("coupe");
        
        
        // Should be enough for several materials.
        memoryPool = _PushMemoryFrame( pLinearAllocator2 );
        // Load and pack materials.
        materialArrayInfo.props[0] = "sandrocks01";
        materialArrayInfo.props[1] = "grasssand01";
        materialArrayInfo.props[2] = "greyrock01";
        materialArrayInfo.props[3] = "panelorange";
        
        materialArrayInfo.m_iLayers = 4;
        materialArrayInfo.m_bMipmapped = true;
        materialArrayInfo.m_bUnloadOriginals = true;
        materialArrayInfo.internalFormat = ETextureFormat::RGBA;
        materialArrayInfo.externalFormat = ETextureFormat::RGBA;
        materialArrayInfo.m_iWidth = 2048;
        materialArrayInfo.m_iHeight = 2048;
        materialArrayInfo.m_iFlags = ((int32_t)EMaterialTypeFlag::Diffuse | (int32_t)EMaterialTypeFlag::Normal | (int32_t)EMaterialTypeFlag::RMA);
        
        // Create material.
        p_MaterialBase->CreateTextureArray("landscapeMat", &materialArrayInfo, memoryPool);
        
        _PopMemoryFrame( memoryPool );
        
        // Load models now.
        p_MeshBase = (CMeshResource *)pClassLinearAllocator->Alloc( sizeof(CMeshResource) ) ;
        p_MeshBase->Initialize( pAllocator );
        
//        g_Core->p_SoundSystem->LoadSound("scissors");
    }

    /**
     *  Unload all assets.
     */
    void CAssetManager::UnloadAll()
    {
        g_Core->p_Console->Print( LOG_INFO, "Unloading assets..\n" );
        
        // Unload materials.
        f_AssetBase->p_MaterialBase->UnloadAllMaterials();
        
        // Delete sounds.
        g_Core->p_SoundSystem->UnloadAllSounds();
        
        // Remove all shaders.
        p_ShaderBase->UnloadShaders();
    }
    
    /**
     *  Shutdown the asset system.
     */
    void CAssetManager::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "Asset system shutting down..\n" );
        
        UnloadAll();
        
        p_ShaderBase->Shutdown();
        p_MeshBase->Shutdown();
        
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pPoolAllocator, pAllocator );
        
        m_pLoaderThread->CallbackEnd( 200.0f );
        m_pAssetQueue->DestroyComplete( NEKO_NULL );
        pAllocator->Dealloc(m_pAssetQueue);
    }
    
    /**
     *  Precache shader.
     */
    void CAssetManager::PrecacheShader( const char * name, uint32_t flags )
    {
        p_ShaderBase->Load( name, NEKO_NULL, flags );
    }
    
    void CAssetManager::PrecacheShader( const char * name, const char * params, uint32_t flags )
    {
        p_ShaderBase->Load( name, params, flags );
    }
    
    const static uint32_t  MAX_ASSETLIST_LINE_LENGTH = 512;
    
    /**
     *  Load asset list file.
     */
    void CAssetManager::LoadStaticAssetList( const char *assetListName )
    {
        // A timer to get asset load time.
        float   t1, t2;
        
        // Total assets in one csv file.
        uint32_t    totalAssets;
        
        // Package file which contains asset list file.
        SPackFile  * assetShared;
        
        // Asset data.
        AssetDataPool   assetListData;
        
        // Get a package with scripts including our asset list as *.csv file.
        assetShared = g_Core->p_FileSystem->GetPak( "shared_data" );
        if( assetShared == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "Couldn't find shared_data package.\n" );
            
            return;
        }
        
        // If we got something..
        assetListData = assetShared->GetData( NC_TEXT( "scripts/asset_loadout/%s.csv", assetListName ) );
        
        totalAssets = 0;
        if( assetListData.tempData == NEKO_NULL ) {
            // Throw error if file wasn't found or wasn't loaded.
            g_Core->p_Console->Error( ERR_ASSET, "LoadStaticAssetList(): Could not read '%s' asset list.\n", assetListName );
            
            return;
        } else {
            g_Core->p_Console->Print( LOG_INFO, "Loading \"%s\" asset list contents..\n", assetListName );
            t1 = (float)g_Core->p_System->Milliseconds();
            
            char    * line;
            int32_t charsRead;
            
            // Parse asset list now.
            line = (char*)PushMemory( assetListData.tempPool, sizeof(char) * (MAX_ASSETLIST_LINE_LENGTH + 1) ); // Temporary line for per line parsing.
            
            charsRead = 0;
            
            char * csv_params[MAX_CSV_PARAMETERS];
            static char tmp[512];
            
            // TODO: Tokenizer.
            
            // Read line by line.
            // @note: it doesn't write to the memory so it's safe for temporary pools.
            while( str_readline( line, MAX_ASSETLIST_LINE_LENGTH, (const char*)assetListData.tempData, charsRead ) )
            {
                // Allow comments.
                if( (line[0] == '/' && line[1] == '/') || line[0] == '#' ) {
                    continue;
                }
                
                // Allow empty lines.
                if( line[0] == '\n' || line[0] == '\r' || line[0] == ' ' ) {
                    continue;
                }
                
                // Copy string.
                strcpy( tmp, line );
                memset( csv_params, 0x00, sizeof(csv_params) );
                
                // Nicely parse CSV file into tokens.
                NekoUtils::ParseCSV( tmp, csv_params, sizeof( csv_params ) ); // 'csv_params' value is rewritten every asset load.
                
#ifndef NEKO_SERVER
                // Parse contents.
                // Image asset.
                if( !strcmp( csv_params[0], "image" ) ) {
                    // Load materials.
                    
                    // Parse file formats.
                    if( !strcmp( csv_params[1], "tgau" ) ) {
                        // Uncompressed TGA.
                        f_AssetBase->p_MaterialBase->LoadImage( csv_params[2], atoi(csv_params[3]), true, TGA_IMAGE );
                    } else if( !strcmp( csv_params[1], "bmp" ) ) {
                        // Classic BMP.
                        f_AssetBase->p_MaterialBase->LoadImage( csv_params[2], atoi(csv_params[3]), true, BMP_IMAGE );
                    }
                    
                    ++totalAssets;
                } else if( !strcmp( csv_params[0], "sound" ) ) {
                    // Sounds.
                    // E.g. 'sound,wav,ocean,'
                    
                    // Load a sound.
                    g_Core->p_SoundSystem->LoadSound( (const char *)csv_params[2] );
                    ++totalAssets;
                } else {
                    g_Core->p_Console->Print( LOG_WARN, "Unknown asset type \"%s\" in asset list found.\n", csv_params[0] );
                }
                
#endif  // NEKO_SERVER
            }
            
            // Don't forget to..
            _PopMemoryFrame( assetListData.tempPool );
            
            t2 = (float)g_Core->p_System->Milliseconds();
            g_Core->p_Console->Print( LOG_INFO, "Asset list \"%s\" with %i assets took %4.2f msec to load.\n", assetListName, totalAssets, (t2 - t1) );
        }
    }
    
    /**
     *  Load core shaders.
     */
    void CAssetManager::LoadShaders()
    {
        g_Core->p_Console->Print( LOG_INFO, "Loading shaders..\n" );
//        
//        PrecacheShader( "ComputePhaseLut", COMPILE_BASIC );
//        PrecacheShader( "ComputeLightLut", COMPILE_BASIC );
//        // Volumetric light
//        PrecacheShader( "RenderVolume", (int32_t)EShaderCompileCaps::Vertex |
//                       (int32_t)EShaderCompileCaps::Fragment |
//                       (int32_t)EShaderCompileCaps::TessEval |
//                       (int32_t)EShaderCompileCaps::TessControl );
        
        const char * particleRenderParams =
        NekoCString::STR("#define ZFAR %f\n \
                         #define ZNEAR %f\n \
                         //#define SOFT_PARTICLES\n \
                         #define Z_CLIP\n",
                         g_Core->p_Camera->GetFarPlaneDistance(),
                         g_Core->p_Camera->GetNearPlaneDistance() );
        
        PrecacheShader( "hi-z", COMPILE_BASIC_GEOMETRY );
        PrecacheShader( "cull", COMPILE_VERTEX_GEOMETRY );
        PrecacheShader( "downsample_blur", COMPILE_BASIC_GEOMETRY );
        // PrecacheShader( "blur", COMPILE_BASIC_GEOMETRY );
        PrecacheShader( "IntegrateBRDF", COMPILE_BASIC );
        PrecacheShader( "CopyShader", COMPILE_BASIC );
        PrecacheShader( "CopyShader2", COMPILE_BASIC );
        PrecacheShader( "particleupdate", COMPILE_VERTEX_GEOMETRY );
        PrecacheShader( "particlerender", particleRenderParams, COMPILE_BASIC_GEOMETRY );
        
        const char *fontParams = NekoCString::STR( "#define USE_PRECISION\n \
                                                  #define PRECISION_TYPE mediump\n \
                                                  #define HALF_WIDTH %f\n \
                                                  #define HALF_HEIGHT %f\n",
                                                  
                                                  Window_Width->Get<float>() / 2.0f,
                                                  Window_Height->Get<float>() / 2.0f );
        
        PrecacheShader( "font", fontParams, COMPILE_BASIC );
        
        const char *draw2dParams = NekoCString::STR( "#define USE_PRECISION\n \
                                                    #define PRECISION_TYPE mediump\n \
                                                    #define HALF_WIDTH %f\n \
                                                    #define HALF_HEIGHT %f\n",
                                                    
                                                    Window_Width->Get<float>() / 2.0f,
                                                    Window_Height->Get<float>() / 2.0f );
        
        //PrecacheShader( "particle", "#define USE_PRECISION\n#define PRECISION_TYPE mediump\n" );
        PrecacheShader( "draw2d", draw2dParams, COMPILE_BASIC );
        
        const char *waterParams = NekoCString::STR( "#define USE_PRECISION\n \
                                                   #define PRECISION_TYPE mediump\n \
                                                   #define ZNEAR %f\n#define ZFAR %7.1f\n",
                                                   
                                                   g_Core->p_Camera->GetNearPlaneDistance(),
                                                   g_Core->p_Camera->GetFarPlaneDistance() );
        
        PrecacheShader( "ocean", waterParams, COMPILE_BASIC );
        const char *modelShading = NekoCString::STR( "#define USE_PRECISION\n \
                                                    #define PRECISION_TYPE mediump\n \
                                                    #define %s\n",
                                                    Use_UniformBuffers->Get<bool>() ? "USE_UNIFORM_OBJECTS" : "NO_UNIFORM_OBJECTS" );;
        
        //        PrecacheShader( "level" );
        PrecacheShader( "model", modelShading, COMPILE_BASIC );
        // Here's your new friend.
        PrecacheShader( "animmodel", modelShading, COMPILE_BASIC );
        
        
        PrecacheShader( "skyscatter", "#define USE_CLOUDS\n \
                       #define USE_PRECISION\n \
                       #define PRECISION_TYPE highp\n", COMPILE_BASIC );
        
        //f_AssetBase->p_ShaderBase->Load( "beautifulgrass" );
        PrecacheShader( "landscape", "#define USE_PRECISION\n \
                       #define PRECISION_TYPE mediump\n", COMPILE_BASIC );//, "#define USE_NORMAL_MAPPING\n" );
        
        //f_AssetBase->p_ShaderBase->Load( "envmap", "#define USE_PRECISION\n#define PRECISION_TYPE mediump\n#define USE_ANIMATION\n" );
        PrecacheShader( "beautifulgrass", "#define USE_ANIMATION\n \
                       #define USE_NORMALMAPPING\n", COMPILE_BASIC );
        //PrecacheShader( "dynamic_cubemap" );
        // OH.. MY... AAAAA
        // PrecacheShader( "deferred" );
        //PrecacheShader( "instanced" );
        PrecacheShader( "road", COMPILE_BASIC );
        PrecacheShader( "billboard", COMPILE_BASIC );
        PrecacheShader( "pointlight", COMPILE_BASIC );
    }
    
    
#ifndef NEKO_SERVER
    
    /**
     *  Find shader by the name.
     */
    template<> SGLShader * CAssetManager::FindAssetByName( const char * name )
    {
        SGLShader *cachedShader = f_AssetBase->p_ShaderBase->GetShaderCache( name );
        if( cachedShader == NEKO_NULL ) {
            // Well.
            g_Core->p_Console->Error( ERR_ASSET, "Asset manager couldn't find %s shader.\n", name );
            
            return NEKO_NULL;
        } else {
            return cachedShader;
        }
        
    }
    
    /**
     *  Find material by the name.
     */
    template<> SMaterialProp * CAssetManager::FindAssetByName( const char * name )
    {
        SMaterialProp  *cachedMaterial = f_AssetBase->p_MaterialBase->Find( name );
        return cachedMaterial;
        
    }
#endif // NEKO_SERVER
    
    
}
