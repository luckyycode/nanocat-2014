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
//  MetalExternal.h
//  Metal API to use in Neko renderer core. :D
//
//  Created by Kawaii Neko on 6/13/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef MetalExternal_h
#define MetalExternal_h

#include "MaterialLoader.h"
#include "Mesh.h" // SVertexBoneData
#include "../../../Graphics/GraphicsInterface.h"
#include "../../../Graphics/Image.h"

namespace Neko {
    
    struct SVertexBoneData;
    
    /**
     *  Metal instance for еру engine core.
     *  Since we can't directly use Metal in C++ code,
     *  I've decided to make something like this.
     *
     *  DO NOT write Objective-C stuff in MetalExternal.h ( this file )
     */
    class Metal : public GraphicsInterface
    {
        NEKO_NONCOPYABLE( Metal );
        
    public:
        
        /**
         *  Constructor.
         */
        Metal();
        
        /**
         *  Destructor.
         */
        virtual ~Metal()
        {
            
        }
        
        /**
         *  Initialize Metal instance.
         */
        virtual void Initialize();
        
        /**
         *  Action on resize.
         *
         *  @param w A new width value.
         *  @param h A new height value.
         */
        virtual void OnResize( const int32_t w, const int32_t h );
        
        /**
         *  When graphics interface got loaded..
         */
        virtual void OnLoad();
        
        /**
         *  Create texture from material image.
         */
        int32_t CreateTexture2DFromImage( SImageInfo * image );
        
        /**
         *  Create texture with raw data.
         */
        int32_t CreateTexture2D( const int32_t w, const int32_t h, const Byte * rawData, const int32_t BitsPerPixel );
        
//        void Blalbla( const int indexCount, unsigned int * Indices );
        
        /**
         *  Render ncMesh using index data.
         */
        void RenderMeshAtIndexWithIndiceData( const int32_t index, const uint32_t numIndices, int32_t BaseIndex, int32_t BaseVertex, int32_t textureId  );
        
        /**
         *  Render ncMesh using vertex data.
         */
        void RenderMeshAtIndexWithVertexData( const int32_t index, const uint32_t numVertices, int32_t totalVertices, int32_t textureId );
        
        /**
         *  Bind ncMesh.
         */
        void BindMeshAtIndex( const int32_t index );
        
        /**
         *  Create mesh.
         */
        //( Positions, Normals, TexCoords, Bones, Indices );
        uint32_t CreateMesh( Vec3 * Positions,
                                Vec3 * Normals,
                                Vec2 * TexCoords,
                                SVertexBoneData * Bones,
                                unsigned int * Indices,
                                const unsigned int iNumVertices,
                                const unsigned int iNumIndices );
        
        virtual uint32_t CreateTexture( SImageInfo * info );
        
        /**
         *  Get Metal compare mode.
         */
        virtual uint32_t GetAPICompareMode( ECompareMode type ) = 0;
        
        /**
         *  Get textures count.
         */
        const int32_t GetTexturesId();
        
        /**
         *  Get meshes count.
         */
        const int32_t GetMeshesId();
    };
}
#endif /* MetalExternal_h */
