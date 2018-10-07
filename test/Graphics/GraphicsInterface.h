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
//  GraphicsInterface.h
//  Graphics API. :)
//
//  Created by Kawaii Neko on 10/12/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef GraphicsInterface_hpp
#define GraphicsInterface_hpp

#include "../Core/Core.h"
#include "RendererMisc.h"
#include "Image.h"

namespace Neko {
    
    /// Graphics interface used by APIs such as OpenGL, Metal, DirectX, etc..
    class GraphicsInterface
    {
        NEKO_NONCOPYABLE( GraphicsInterface );
        
    public:
        
        GraphicsInterface() : m_bInitialized(false) {

        }
        
        /**
         *  Destructor.
         */
        virtual ~GraphicsInterface() {
            
        }
        
        /**
         *  Initialize graphics interface.
         */
        virtual void                Initialize() = 0;
        
        /**
         *  Action on context resize.
         */
        virtual void                OnResize( const int32_t w, const int32_t h ) = 0;
        
        /**
         *  When graphics interface got loaded..
         */
        virtual void                OnLoad() = 0;
        
        /**
         *  Unload all resources that API requested and remove the interface.
         */
        virtual void                Shutdown() = 0;
        
        /**
         *  Check API reported errors.
         */
        virtual void                CheckAPIError( const char * where ) = 0;
        
        /**
         *  Get current display viewport.
         */
        inline virtual const int32_t *          GetViewport()    {       return (int32_t*)m_displayViewport; }
        
        /**
         *  Get shader type.
         */
        virtual uint32_t                GetAPIShaderType( EShaderType type ) = 0;
        
        /**
         *  Get primitive type for current API.
         */
        virtual uint32_t                GetAPIPrimitiveType( EPrimitiveType type ) = 0;
        
        /**
         *  Get buffer type for current API.
         */
        virtual uint32_t                GetAPIBufferType( EBufferType type ) = 0;
        
        /**
         *  Get buffer storage type for current API.
         */
        virtual uint32_t                GetAPIBufferStorageType( EBufferStorageType type ) = 0;
        
        /**
         *  Get texture target type for current API.
         */
        virtual uint32_t                GetAPITextureTarget( TextureTarget type ) = 0;
        
        /**
         *  Get API corresponding tile.
         */
        virtual uint32_t                GetAPITextureTile( ETextureTile type ) = 0;
        
        /**
         *  Create texture.
         */
        virtual uint32_t                CreateTexture( SImageInfo * info ) = 0;
        
        /**
         *  Generate mipmaps for a texture.
         */
        virtual void                    GenerateMipmaps( const uint32_t texId, const uint32_t target ) = 0;
        
        /**
         *  Get storage type.
         */
        virtual uint32_t                GetAPIStorageType( ETextureStorageType type ) = 0;
        
        /**
         *  Get color type.
         */
        virtual uint32_t                GetAPITextureFormat( ETextureFormat type ) = 0;
        
        /**
         *  Get texture filtering mode.
         */
        virtual uint32_t                GetAPITextureFilteringMode( ETextureFilter type ) = 0;
        
        /**
         *  Get compare mode.
         */
        virtual uint32_t                GetAPICompareMode( ECompareMode type ) = 0;
        
        /**
         *  Get cull mode.
         */
        virtual uint32_t                GetAPICullMode( ECullMode type ) = 0;
        
        /**
         *  Get blend mode.
         */
        virtual uint32_t *               GetAPIBlendMode( EBlendMode type ) = 0;
        
        /**
         *  Get color target.
         */
        virtual uint32_t                GetAPIRenderTarget( ERenderTarget target ) = 0;
        
        /**
         *  Initialized?
         */
        bool        m_bInitialized;
        
        /**
         *  Current viewport.
         */
        int32_t     m_displayViewport[4];
        
    protected:
    private:
        
    };
}

#endif /* GraphicsInterface_hpp */
