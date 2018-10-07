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
//  MetalReferences.h
//  Metal API references. ^.^
//
//  Created by Kawaii Neko on 6/13/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef MetalReferences_h
#define MetalReferences_h

#import <Metal/Metal.h>

namespace Neko {
    
    /**
     *  Metal mesh for ncMesh.
     */
    struct MetalMesh {
        __strong id<MTLBuffer> vertexBuffer;
        __strong id<MTLBuffer> normalBuffer;
        __strong id<MTLBuffer> uvBuffer;
        __strong id<MTLBuffer> boneBuffer;
        __strong id<MTLBuffer> indexBuffer;
        
        uint32_t diffuseTexture;
        uint32_t normalTexture;
        
        uint32_t index;
        
        uint32_t iVerticeNum;
        uint32_t iIndiceNum;
    };
    
    /**
     *  Metal texture for SMaterial.
     */
    struct MetalTexture {
        id<MTLTexture> material;
        id<MTLSamplerState> metalSampler;
        
        uint32_t index;
    };
    
    /**
     *  Metal resource base.
     */
    class MetalResources {
    public:
        
        MetalResources();
    
        
        MetalTexture metalTextures[128];
        MetalMesh metalMeshes[128];
        // etc...
        
        /**
         *  Add buffer for ncMesh.
         */
        void AddBufferForMesh( MetalMesh & mtlMesh );
        
        /**
         *  Add texture for SMaterial.
         */
        void AddTextureToBase( MetalTexture & mtlTexture );
//        
//        int hello() {
//            return 1;
//        }
        
        /**
         *  Get total Metal buffers created.
         */
        inline const int32_t GetTotalMTLBuffers() {
            return iNumBuffers;
        }
        
        /**
         *  Get total Metal textures created.
         */
        inline const int32_t GetTotalMTLTextures() {
            return iNumTextures;
        }
        
    private:
        /**
         *  Total buffers created for Metal.
         */
        uint32_t iNumBuffers;
        
        /**
         *  Total textures created for Metal.
         */
        uint32_t iNumTextures;
    };
    
    extern MetalResources * metalResource;
}

#endif /* MetalReferences_h */
