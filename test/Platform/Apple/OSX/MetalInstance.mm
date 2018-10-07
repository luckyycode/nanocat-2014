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
//  MetalInstance.m
//  Metal instance used to create buffers, textures, etc.. ;o
//
//  Created by Kawaii Neko on 6/15/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#import "MetalReferences.h"
#import "MetalView.h"
#import "Core.h"

namespace Neko {
    
    MetalResources local_MTLResources;
    MetalResources * metalResource = &local_MTLResources;
    
    /**
     *  Metal instance.
     */
    MetalResources::MetalResources() : iNumBuffers(0),
    iNumTextures(0) {
        
    }
    
    /**
     *  Add buffers for basic mesh from Mesh loader.
     */
    void MetalResources::AddBufferForMesh( MetalMesh & mesh ) {
        ++iNumBuffers;
        
        const int index = mesh.index;
        
        metalMeshes[index] = mesh;
        
        g_Core->p_Console->Print( LOG_INFO, "Metal - Added mesh at %i index\n", index );
    }
    
    /**
     *  Add texture to its base at index.
     */
    void MetalResources::AddTextureToBase( MetalTexture & mtlTexture ) {
        ++iNumTextures;

        // Create new sampler descriptor.
        MTLSamplerDescriptor *samplerDesc = [MTLSamplerDescriptor new];
        samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
        samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
        samplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
        samplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
        
        // Assign it to the texture.
        mtlTexture.metalSampler = [MetalDataBase->device newSamplerStateWithDescriptor:samplerDesc];
        metalTextures[mtlTexture.index] = mtlTexture;
        
        g_Core->p_Console->Print( LOG_INFO, "Metal - Added texture at %i index\n", iNumTextures );
    }
}

