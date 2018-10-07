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
//  MetalSupport.m
//  Metal support for Neko renderer. :P
//
//  Created by Kawaii Neko on 6/13/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "MetalExternal.h"
#include "MacWindow.h"

#include "Core.h"
#include "Console.h"
#include "Renderer.h"

#include "../../../World/Mesh.h"

#include "MetalReferences.h"

namespace Neko {
    
    /**
     *  New metal instance.
     */
    Metal::Metal()
    {
        NSLog( @"MetalContext initializing..\n" );
    }
   
    /**
     *  Get total material count.
     */
    const int32_t Metal::GetTexturesId()
    {
        return metalResource->GetTotalMTLTextures();
    }
    
    /**
     *  Get total mesh count.
     */
    const int32_t Metal::GetMeshesId()
    {
        return metalResource->GetTotalMTLBuffers();
    }

    /**
     *  Bind mesh attributes at index.
     */
    void Metal::BindMeshAtIndex( const int32_t index )
    {
        [MetalDataBase->commandEncoder setVertexBuffer:metalResource->metalMeshes[index].vertexBuffer offset:0 atIndex:0];
        [MetalDataBase->commandEncoder setVertexBuffer:metalResource->metalMeshes[index].normalBuffer offset:0 atIndex:1];
        [MetalDataBase->commandEncoder setVertexBuffer:metalResource->metalMeshes[index].uvBuffer offset:0 atIndex:2];
        
        [MetalDataBase->commandEncoder setVertexBuffer:MetalDataBase->uniformBuffer offset:0 atIndex:3];
        
        [MetalDataBase->commandEncoder setFragmentBuffer:MetalDataBase->uniformBuffer offset:0 atIndex:0];
    }
    
    /**
     *  Render mesh at index using index data.
     */
    void Metal::RenderMeshAtIndexWithIndiceData( const int32_t index, const uint32_t numIndices, int32_t BaseIndex, int32_t BaseVertex, int32_t textureId  )
    {
        [MetalDataBase->commandEncoder setFragmentTexture:metalResource->metalTextures[textureId].material atIndex:0];
        [MetalDataBase->commandEncoder setFragmentSamplerState:metalResource->metalTextures[textureId].metalSampler atIndex:0];
        
//        glDrawElementsBaseVertex( Topology,
//                                 m_Entries[i].NumIndices,
//                                 GL_UNSIGNED_INT,
//                                 (void*)(sizeof(unsigned int) * m_Entries[i].BaseIndex),
//                                 m_Entries[i].BaseVertex );
        

        
//        unsigned int count = [metalResource->metalMeshes[index].indexBuffer length] / sizeof(unsigned int);
        
        [MetalDataBase->commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:numIndices indexType:MTLIndexTypeUInt32 indexBuffer:metalResource->metalMeshes[index].indexBuffer indexBufferOffset:0];
    }
    
    /**
     *  Render mesh at index using vertex data.
     */
    void Metal::RenderMeshAtIndexWithVertexData( const int index, const unsigned int numVertices, int totalVertices, int textureId  ) {
        [MetalDataBase->commandEncoder setFragmentTexture:metalResource->metalTextures[textureId].material atIndex:0];
        [MetalDataBase->commandEncoder setFragmentSamplerState:metalResource->metalTextures[textureId].metalSampler atIndex:0];
        
        //[MetalDataBase->commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:numVertices vertexCount:totalVertices instanceCount:1 baseInstance:numVertices];
        [MetalDataBase->commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:numVertices];
    }
    
    uint32_t Metal::CreateTexture(Neko::SImageInfo *info)
    {
        return 0;
    }
    
    uint32_t Metal::GetAPICompareMode(Neko::ECompareMode type)
    {
        return 0;
    }
    
    /**
     *  Create basic mesh for Mesh loader.
     */
    unsigned int Metal::CreateMesh( Vec3 * Positions,
                    Vec3 * Normals,
                    Vec2 * TexCoords,
                    SVertexBoneData * Bones,
                        uint32_t * Indices,
                           const uint32_t iNumVertices,
                           const uint32_t iNumIndices ) {
        NSLog( @"Metal::CreateMesh called ( indexies: %i vertexies: %i ) ", iNumIndices, iNumVertices );

        
        MetalMesh mesh;
  
        mesh.vertexBuffer = [MetalDataBase->device newBufferWithBytes:Positions
                                                length:iNumVertices * sizeof(Vec3)
                                                              options:MTLResourceOptionCPUCacheModeDefault];
        
        mesh.normalBuffer = [MetalDataBase->device newBufferWithBytes:Normals
                                                     length:sizeof(Vec3) * iNumVertices
                                                    options:MTLResourceOptionCPUCacheModeDefault];
        
        mesh.uvBuffer = [MetalDataBase->device newBufferWithBytes:TexCoords
                                                     length:sizeof(Vec2) * iNumVertices
                                                    options:MTLResourceOptionCPUCacheModeDefault];
        
        // Add bones if we have any.
        if( Bones )
            mesh.boneBuffer = [MetalDataBase->device newBufferWithBytes:&Bones[0]
                                                       length:sizeof(SVertexBoneData) * iNumVertices
                                                      options:MTLResourceOptionCPUCacheModeDefault];
        
        mesh.indexBuffer = [MetalDataBase->device newBufferWithBytes:Indices
                                               length:iNumIndices * sizeof(GLuint)
                                              options:MTLResourceOptionCPUCacheModeDefault];
        
        // Mesh index.
        mesh.index = GetMeshesId();
        mesh.iVerticeNum = iNumVertices;
        mesh.iIndiceNum = iNumIndices;

        // Finally add mesh as Metal mesh.
        metalResource->AddBufferForMesh( mesh );
        
        return metalResource->GetTotalMTLBuffers() - 1; // uuuh
    }

    void Metal::Initialize() {
        g_Core->p_Console->Print( LOG_INFO, "Initializing Metal support..\n" );
      //  g_mainRenderer->Initialize();
        
        GraphicsInterface::Initialize();
    }
    
    void Metal::OnResize( const int32_t w, const int32_t h )
    {
        GraphicsInterface::OnResize( w, h );
    }
    
    void Metal::OnLoad()
    {
        GraphicsInterface::OnLoad();
    }
    
    static uintptr_t CreateMetalTexture( void * data, unsigned int w, unsigned int h ) {

        MTLTextureDescriptor* texDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:w height:h mipmapped:NO];
        
        id<MTLTexture> tex = [MetalDataBase->device newTextureWithDescriptor:texDesc];
        
        MTLRegion r = MTLRegionMake3D( 0, 0, 0, w, h, 1 );
        [tex replaceRegion:r mipmapLevel:0 withBytes:data bytesPerRow:w * 4];
        
        return (uintptr_t)(__bridge_retained void*)tex;
    }
    
    void DestroyMetalTexture( uintptr_t tex ) {
        id<MTLTexture> mtltex = (__bridge_transfer id<MTLTexture>)(void*)tex;
        mtltex = nil;
    }
    
    extern "C" intptr_t CreateNativeTexture(SImageInfo * image)
    {

        uintptr_t ret = 0;
        ret = CreateMetalTexture( image->GetData(), image->GetWidth(), image->GetHeight() );
        
        // Image will be deleted after its loading.

        return ret;
    }
    
    /**
     *  Create basic texture for material manager 
     *  and return it's index in base.
     */
    int Metal::CreateTexture2DFromImage( SImageInfo * image ) {
//        printf( "HELLO! CLAP!\n" );
  
        //NSLog( @"Metal::CreateTexture2D - width: %i height %i bitsPerPixel: %i\n", w, h, bytesPerPixel );
        CreateTexture2D( image->GetWidth(), image->GetHeight(), image->GetData(), image->GetBitsPerPixel() );
 
        return metalResource->GetTotalMTLTextures() - 1;
    }

    int Metal::CreateTexture2D( const int w, const int h, const Byte *rawData, const int BitsPerPixel ) {
//        printf( "HELLO! CLAP!\n" );
        
        // Check for data.
        if( !rawData ) {
            g_Core->p_Console->Print( LOG_ERROR, "Metal::CreateTexture2D - missing data.\n" );
            return 0;
        }
        
        //NSLog( @"Metal::CreateTexture2D - width: %i height %i bitsPerPixel: %i\n", w, h, bytesPerPixel );
        
        MTLPixelFormat pixelFormat = MTLPixelFormatInvalid;
        NSUInteger bytesPerRow = 32 / 8 * w;
        
        // Find our format for texture.
        if( BitsPerPixel <= 24 ) {
            pixelFormat = MTLPixelFormatRGB10A2Unorm;
        } else { /* 32 BitsPerPixel */
            pixelFormat = MTLPixelFormatRGBA8Unorm;
        }
        
        // New texture descriptor.
        MTLTextureDescriptor * textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                                                      width:w
                                                                                                     height:h
                                                                                                  mipmapped:YES];
        // Create new texture.
        id<MTLTexture> texture = [MetalDataBase->device  newTextureWithDescriptor:textureDescriptor];
        
        if( !texture )
            return 0;

        
        // Fill texture with data.
        [texture replaceRegion:MTLRegionMake3D( 0, 0, 0, w, h, 1.0 )
                   mipmapLevel:0
                     withBytes:rawData
                   bytesPerRow:bytesPerRow];
        
        MetalTexture dummyTexture;
        dummyTexture.material = texture;
        dummyTexture.index = GetTexturesId();

        // Add texture to Metal base.
        metalResource->AddTextureToBase( dummyTexture );
        
        texture = nil;
        
        return metalResource->GetTotalMTLTextures() - 1;
    }
}
