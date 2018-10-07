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
//  AssetManager.h
//  Asset manager and loader.. :>
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef assetmanager_h
#define assetmanager_h

#include "../Core/Queue.h"
#include "../Core/Utilities/VectorList.h"
#include "../Platform/Shared/SystemShared.h"
#include "../World/MeshBase.h"
#include "Material/MaterialLoader.h"
#include "Shader/ShaderFactory.h" // SGLShader
#include "AssetPtr.h"


namespace Neko {
    
#define COMPILE_BASIC   ((int32_t)EShaderCompileCaps::Vertex | (int32_t)EShaderCompileCaps::Fragment)
#define COMPILE_BASIC_GEOMETRY   ((int32_t)EShaderCompileCaps::Vertex | (int32_t)EShaderCompileCaps::Fragment | (int32_t)EShaderCompileCaps::Geometry)
    
#define COMPILE_VERTEX_GEOMETRY   ((int32_t)EShaderCompileCaps::Vertex | (int32_t)EShaderCompileCaps::Geometry)
#define COMPILE_FRAGMENT_GEOMETRY   ((int32_t)EShaderCompileCaps::Fragment | (int32_t)EShaderCompileCaps::Geometry)
    
    
    static const int32_t kMaterialAssetPoolSize = Megabyte( 32 );
    static const uint32_t kShaderCacheSize = Megabyte( 8 );
    static const uint32_t kSoundCachePoolSize = Megabyte( 2 );
    
    
    //!  Maximum CSV parameters.
    static const int32_t MAX_CSV_PARAMETERS = 11;
    //!  Maximum loader threads to initialize.
    const static int32_t MAX_LOADER_THREADS = 16;

    ///  Asset manager.
    class CAssetManager
    {
        NEKO_NONCOPYABLE( CAssetManager );
        
    public:
        
        /**
         *  Constructor.
         */
        CAssetManager()
        {
            
        }
        
        /**
         *  Destructor.
         */
        ~CAssetManager()
        {
        
        }

        /**
         *  Load core shaders.
         */
        void                    LoadShaders();
        
        /**
         *  Load asset on its loader thread.
         */
        void *                    LoadAssetAsynchronously( EAssetType type, const char * name );
        
        /**
         *  Update asset manager on main thread!
         */
        void                    Frame( uint32_t msec, bool isMainThread = true );
        
        /**
         *  Re-load asset.
         */
        bool                    ReloadThreadedAsset();
        
        /**
         *  Load CSV asset list.
         */
        void                LoadStaticAssetList( const char * assetListName );
        
        /**
         *  Initialize asset manager.
         */
        void                Initialize( INekoAllocator * allocator );
        
        /**
         *  Unload all assets.
         */
        void                UnloadAll();
        
        /**
         *  Shutdown the asset system.
         */
        void                Shutdown();
        
        /**
         *  Find asset by name.
         */
        template<class t> t               * FindAssetByName( const char * name );
        
        /**
         *  Precache shader.
         */
        void                PrecacheShader( const char * name, uint32_t flags );
        void                PrecacheShader( const char * name, const char * params, uint32_t flags );

        INekoAllocator      * pAllocator = 0;
        INekoAllocator      * pPoolAllocator = 0;
        
        // Nice stuff.
        class CShaderFactory   * p_ShaderBase;
        class CMaterialFactory * p_MaterialBase;
        class CMeshResource     * p_MeshBase;

        //!     Asset loader threads.
        INekoThread          * m_pLoaderThread;
        //!     Asset loader condition variables.
        INekoThreadEvent     * m_pLoaderEvent;
        //!     Asset loader thread lock.
        INekoThreadLock      * m_pLoaderLock;

        //! Asset streaming queue.
        CQueue      * m_pAssetQueue;
        
        //! Loaded assets cache list.
        SList   m_pLoadedAssets;
        
        SList       m_assetsToLoad;
        
        //! Is loader thread active?
        int8_t    LoadCache:1;
    };
    
#   if !defined( NEKO_SERVER )
    template<> SGLShader * CAssetManager::FindAssetByName( const char *name );
    template<> SMaterialProp * CAssetManager::FindAssetByName( const char *name );
#   endif
    
    extern CAssetManager *f_AssetBase;

}
#endif
