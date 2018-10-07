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
//  QuadtreeNode.h
//  Neko engine
//
//  Created by Kawaii Neko on 12/2/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef QuadtreeNode_h
#define QuadtreeNode_h

#include "../../Core/Core.h"
#include "../LChunk.h"

namespace Neko {
    
    class CNodeChunk;
    
    //!   Quadtree types.
    enum EQuadtreeType
    {
        TERRAIN = 0,     // Terrain surface.
        OCEAN,           // Ocean.
        DUMMY,           // Dummy cell.
    };

    ///  Quadtree world node.
    class CQuadtreeNode
    {
        NEKO_NONCOPYABLE( CQuadtreeNode );
        
    public:
        
        /**
         *  Create quadtree node and its children.
         */
        void                Create( uint32_t depth, const Vec2i & pos, const Vec2i & mSize,
                                            uint32_t minmSize, int16_t flags, INekoAllocator * allocator );
        
        /**
         *  Calculate bounding boxes.
         */
        void                CalculateBoundingBoxAndCollisionData( const Vec3 *vertices,
                                                                            bool needsCollision, bool needsBoundingBox );
        
        /**
         *  Delete indices.
         */
        void                DeleteIndices();
        
        /**
         *  Destroy chunk.
         */
        void                Destroy();
        
        /**
         *  Draw chunk.
         */
        int32_t                 Draw( CFrustum * frustumum, int32_t options /* drawing parameters. */ );
        
        /**
         *  Draw chunk foliage.
         */
        int32_t                 DrawFoliage();
        
        /**
         *  Draw chunk objects.
         */
        int32_t                 DrawObjects( /*bool isReflection, bool isShadow*/ int16_t flags );
        
        /**
         *  Check if current node don't have any children.
         */
        inline bool                 isChild() const {       return (m_nodeChildren == 0);   }
        
        /**
         *  Constructor.
         */
        CQuadtreeNode() : m_nodeChildren( NEKO_NULL ), m_terrainChunk( NEKO_NULL ), m_iChunkLOD( 0 )
        {
            
        }
        
        ~CQuadtreeNode()
        {
            Destroy();
        }
        
        /**
         *  Get terrain chunk.
         */
        inline CNodeChunk *                 GetChunk() const  {       return m_terrainChunk;  }
        
        INekoAllocator      * pAllocator = 0;
        
        /**
         *  Current node plane normal.
         */
        Vec3               m_planeNormal;
        
        /**
         *  Get chunk level of detail & total collision vertices.
         */
        int32_t             m_iChunkLOD;    //! Can be negative sometimes.
        uint32_t            m_iColNum;      //! Collision block vertex count.
        
        /**
         *  Distance from chunk to its children.
         */
        float               m_fDistance;
        
        /**
         *  Level of detail offset.
         */
        float               m_fLodOffset;
        
        /**
         *  Is chunk visible?
         */
        bool                m_bCanBeSeen;
        
        /**
         *  Chunk bounding box.
         */
        SBoundingBox       m_boundingBox;
        
        /**
         *  Get chunk children.
         */
        CQuadtreeNode      * m_nodeChildren;
        
        /**
         *  Get chunk.
         */
        CNodeChunk            * m_terrainChunk;
        
        /**
         *  Quadtree node type.
         */
        EQuadtreeType          m_qType;
        
        // Collision chunks. Mostly filled by the most detailed chunk data.
        Vec3           *   m_pCollisionChunks;
    };

}

#endif /* QuadtreeNode_h */
