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
//  Quadtree.h
//  Level of detail quadtree implementation. :OO
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.

#ifndef __quadtree_node_h__
#define __quadtree_node_h__

#include "../../Platform/Shared/SystemShared.h"
#include "../LChunk.h"
#include "../../Math/BoundingBox.h"
#include "../../Math/Frustum.h"
#include "QuadtreeNode.h"

namespace Neko {

    /**
     *  Quadtree node location.
     */
    enum QuadtreeNodeSide
    {
        CHILD_NW    = 0,
        CHILD_NE    = 1,
        CHILD_SW    = 2,
        CHILD_SE    = 3
    };

    /**
     *  World node properties.
     */
    enum class EWorldFlag : int32_t
    {
        PlayerEye       = 1 << 0,
        Reflection      = 1 << 1,
        Shadow          = 1 << 2,  // Our new buddy!
        Cubemap         = 1 << 3,      // Whoa!
        
        Dummy   = 1 << 4
    };

    /// Quadtree flags.
    enum QuadtreeFlags
    {
        WORLD_THREADED  = 0x1,
        WORLD_DUMMY     = 0x2
    };
    
    class CNodeChunk;

    ///  CPU Quadtree LOD.
    class CQuadtree
    {
    public:
        
        /**
         *  Create new quadtree.
         */
        void                Create( EQuadtreeType type, SBoundingBox * pBBox,
                                        const Vec2i & mSize, uint32_t minmSize, const int16_t flags, INekoAllocator * allocator );
        
        /**
         *  Calculate bounding boxes.
         */
        void                CalculateBoundingBoxAndCollisionData( const Vec3 *vertices,
                                                     bool needsCollision, bool needsBoundingBox  );
        
        /**
         *  Delete indices.
         */
        void                DeleteIndices();
        
        /**
         *  Destroy quadtree.
         */
        void                    Destroy();

        
        /**
         *  Draw surface.
         */
        int32_t                 Draw( /*bool bReflection, bool bShadow = false*/ int32_t flags );
        
        /**
         *  Draw foliage.
         */
        int32_t                 DrawFoliage();
        
        /**
         *  Draw objects.
         */
        int32_t                 DrawObjects( /*bool isReflection, bool isShadow*/ int16_t flags );
        
        /**
         *  Find quadtree node leaf at position( x;z ).
         */
        CQuadtreeNode  *            FindNode( const Vec2 & pos );

        INekoAllocator   * pAllocator;
        
        /**
         *  Constructor.
         */
        CQuadtree()
        {
            nodeRoot = NEKO_NULL;
        }

        /**
         *  Destructor.
         */
        ~CQuadtree()
        {
            Destroy();
        }

        /**
         *  Quadtree node with it's children.
         */
        CQuadtreeNode  * nodeRoot;
    };

}

#endif // __quadtree_node_h__
