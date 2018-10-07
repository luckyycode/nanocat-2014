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
//  LChunk.h
//  Level of detail quadtree chunk.
//
//  This code is a part of Neko engine.
//  Created by Neko Vision on 12/22/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __terrain_chunk_h__
#define __terrain_chunk_h__

#include "../Core/CoreDef.h"
#include "../Core/Utilities/List.h"
#include "../Graphics/RendererMisc.h"
#include "../Math/GameMath.h"
#include "../Math/Vec2.h"
#include "../Platform/Shared/SystemShared.h"
#include "Foliage.h"
#include "LTerrainObject.h"
#include "Mesh.h"


namespace Neko {
    
    // Total chunks.
    static const int32_t TERRAIN_CHUNKS_LOD    = 7;

    // Terrain chunks.
    // TODO: make me customizable
    static const float TERRAIN_CHUNK_LOD0  = 170.0f;
    static const float TERRAIN_CHUNK_LOD1  = 230.0f;
    static const float TERRAIN_CHUNK_LOD2  = 250.0f;
    static const float TERRAIN_CHUNK_LOD3  = 470.0f;
    static const float TERRAIN_CHUNK_LOD4  = 910.0f;
    static const float TERRAIN_CHUNK_LOD5  = 1260.0f;

    // Ocean chunks.
    // TODO: same here
    static const float OCEAN_CHUNK_LOD0    = 30.0f;
    static const float OCEAN_CHUNK_LOD1    = 60.0f;
    static const float OCEAN_CHUNK_LOD2    = 90.0f;
    static const float OCEAN_CHUNK_LOD3    = 110.0f;
    static const float OCEAN_CHUNK_LOD4    = 120.0f;
    static const float OCEAN_CHUNK_LOD5    = 150.0f;

    //! Total biomes.
    static const int32_t BIOME_COUNT = 3;
    
    //! Maximum foliage objects per chunk.
    static const int32_t MAX_CHUNK_FOLIAGES = 8192;
    
    class ncLTerrainObject;
    struct ncCGrass;
    
    //!  Biome types.
    enum LandscapeChunkType
    {
        FOREST  = 0,
        SANDY   = 1,
        SNOWY   = 2,
    };

    ///  World node chunk.
    class CNodeChunk
    {
//        NEKO_NONCOPYABLE( CNodeChunk );
        
    public:
     
        /**
         *  Load new chunk.
         */
        void            Load( const uint32_t level,
                             const Vec2i & pos,
                             const Vec2i & size, const int16_t qtype, const bool hasFoliageData, int16_t flags, INekoAllocator * allocator, SMemoryTempFrame * temp );

        /**
         *   Delete chunk.
         */
        void                Delete();

        /**
         *   Render chunk.
         */
        int32_t                 DrawGround( uint32_t lod );

        /**
         *   Draw objects.
         */
        int32_t                 DrawObjects( int32_t lod, float distance, bool isShadow );
        
        /**
         *  Draw foliage.
         */
        int32_t                 DrawFoliage( uint32_t lod, float distance );

        /**
         *  Add object to chunk.
         */
        void                AddObject( ncLTerrainObject *object );

        /**
         *  Add foliage layer.
         */
        void                AddFoliageLayer( float x, float y, float z );
        
        CNodeChunk();
        ~CNodeChunk();
        
        /**
         *  Get biome type.
         */
        inline const LandscapeChunkType                 GetBiomeType() const    {       return m_tBiome;    }
        
        /**
         *  Foliage amount.
         */
        inline const int32_t                        GetFoliageCount() const    {       return m_foliageAmount; }
        
        /**
         *  Is it possible to render current chunk?
         *
         *  @return A corresponding value.
         */
        inline bool                 CanBeUsed() const   {       return m_bIsReady;  }

        /**
         *  Compute Levels of detail for chunk.
         */
        void                MakeLevelOfDetailIndices( const unsigned int lod, const unsigned int level,
                                                                const Vec2i & pos, const Vec2i & size );


        SMemoryTempFrame    * pTempMemory = 0;  //! Used to initialize huge data which will be removed after creation.
        INekoAllocator      * pAllocator = 0;   //! This comes from quadtree node.
        
        /**
         *  This holds terrain objects.
         */
        SList   m_terrainObjects;
        SList   m_foliageObjects;

        //!  Indice data.
        struct  IndiceData
        {
            uint32_t  * indices;
            uint32_t  count;
        };
        
        /**
         *  Quadtree type.
         */
        int32_t         m_qtType;
        
        /**
         *  Current lod value.
         */
        int32_t     m_currentLod;
        
        /**
         *  Can we render current chunk?
         */
        bool        m_bIsReady;
        
        /**
         *  Total grass cells ( per chunk ).
         */
        ncCGrass    **m_tCells;
        
        /**
         *  Total foliage amount.
         */
        int32_t         m_foliageAmount;
        
        /**
         *  Total terrain object amount.
         */
        GLuint      m_TerrainObjectCount;
        
        /**
         *  Chunk indexies.
         */
        IndiceData      m_tIndice[TERRAIN_CHUNKS_LOD];

        /**
         *  Index buffer for per level of detail.
         */
        GPUBuffer       m_iElementBuffer[TERRAIN_CHUNKS_LOD];

        /**
         *  Chunk sizes at per level ( WxH ).
         */
        GLuint      m_tIndOffsetW[TERRAIN_CHUNKS_LOD];
        GLuint      m_tIndOffsetH[TERRAIN_CHUNKS_LOD];
        
        //! Chunk flags.
        int16_t     m_iFlags;
        
        /**
         *  Chunk biome type.
         */
        LandscapeChunkType m_tBiome;
        
        float fAmbientRadius;
        bool bGotAmbient;
    };
}

#endif // __terrain_chunk_h__
