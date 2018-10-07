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
//  Terrain.h
//  Beautiful terrain creator and renderer..
//
//  Created by Neko Code on 11/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __Terrain__
#define __Terrain__

#include "../Quadtree/Quadtree.h"
#include "../../Math/BoundingBox.h"
#include "../../AssetCommon/Material/MaterialLoader.h"
#include "../../Core/Utilities/Noise.h"
#include "../../Graphics/Renderer/Renderer.h"

namespace Neko {
   
    const static uint32_t MAX_PATH_NODES = 4;   //! Maximum path nodes.
    const static uint32_t MAX_PATH_NODE_PIECES = 256; //! Maximum pieces of each path node.
    
#define MAX_MESH_LOD 3
#define MAX_MESH_INSTANCES 8
    
    ///  Landscape type.
    enum class ELandscapeType : int32_t
    {
        Procedural   = 0,   //! Random noise generated landscape.
        Heightmap,          //! Load landscape from an image.
        LoadDisk,           //! Load landscape file.
        
        Dummy
    };

    ///  Path node object data.
    struct SPathNodeObjects
    {
        SPathNodeObjects()
        {
            Position.MakeNull();
            Width = 6.0f;
        }
        
        //! Node start origin.
        Vec3   Position;
        //! Node mesh width.
        float   Width;
    };
    
    ///  Path chunk.
    struct SChunkPathCell
    {
        SChunkPathCell()
        {
            HeightAt = 1.0f;
            Position.MakeNull();
        }
        
        //! Height at origin.
        float   HeightAt;
        //! Origin.
        Vec2   Position;
        //! Added to world?
        bool    IsAdded;
    };
    
    /**
     *  Vertex data to edit.
     *  Allows us to edit only selected vertex pieces instead of the whole chunk.
     */
    struct SEditVertices
    {
        //!  Indexed vertex data.
        int32_t     * m_iIndexData;
        //!  Vertex flags ( used to mark if it's been used ).
        int16_t     * m_iFlags;
        //!  Vertex surface type flags.
        int32_t     * m_iTypeFlags;
        //!  Vertices count.
        int32_t     m_iVerticeCount;
        //!  Indexed vertices count.
        int32_t     m_iVerticeIndiceCount;
        
        /**
         *  Basic constructor.
         */
        SEditVertices() : m_iTypeFlags( 0 ), m_iFlags( 0 )
        {
            
        };

        ~SEditVertices()
        {
            
        };
        
        /**
         *  Add vertex index.
         *
         *  @note 'vertexFlag' value can be any option.
         */
        void AddVertexIndex( int32_t vertexId, const int8_t vertexFlag )
        {
            // Check if we can use current vertex.
            if( m_iFlags[vertexId] == 0 ) {
                // Add a new vertex.
                m_iIndexData[m_iVerticeIndiceCount++] = vertexId;
                m_iFlags[vertexId] = 1;
                m_iTypeFlags[vertexId] = (int32_t)vertexFlag;
            }
        }
        
        /**
         *  Create a new vertex index array to edit.
         */
        void Create( const int32_t vertexCount )
        {
            // Initialize data.
            m_iIndexData = new int32_t[vertexCount];
            m_iFlags = new int16_t[vertexCount];
            m_iTypeFlags = new int32_t[vertexCount];
            
            memset( m_iFlags, 0x00, vertexCount );
            
            m_iVerticeCount = vertexCount;
            m_iVerticeIndiceCount = 0;
        }
        
        /**
         *  Clear indexed data.
         */
        void Empty()
        {
            memset( m_iFlags, 0x00, m_iVerticeCount );
            m_iVerticeIndiceCount = 0;
        }
        
        /**
         *  Delete indexed data.
         */
        void Destroy()
        {
            delete [] m_iIndexData;
            delete [] m_iFlags;
            delete [] m_iTypeFlags;
        }
        
        /**
         *  Get vertex index.
         */
        inline const int32_t                GetVertexIndex( int32_t index ) const  {       return m_iIndexData[index]; }
        
        /**
         *  Get vertex type flag.
         */
        inline const int16_t                GetTypeFlag( int32_t index ) const {       return m_iTypeFlags[index]; }
        
        /**
         *  Get index vertices count.
         */
        inline const int32_t                GetVerticesIndicesCount() const    {       return m_iVerticeIndiceCount;   }
    };
    
    struct CBasicMesh;
    
    ///  Path info.
    struct SPathInfo
    {
        /**
         *  Constructor.
         */
        SPathInfo() : m_pNodeMesh(NEKO_NULL), m_pNodeObjects(NEKO_NULL), m_iNodeFlags(0), m_iPathCount(0), m_iNodePaths(0)
        {
            
        }
        
        //!  Node mesh.
        CBasicMesh         * m_pNodeMesh;
        //!  Node objects.
        SPathNodeObjects    * m_pNodeObjects;
        //!  Node flags.
        uint16_t            m_iNodeFlags;
        //!  Node path count.
        uint32_t            m_iPathCount;
        //!  Path width.
        float               m_fPathWidth;
        //!  Path cells.
        SChunkPathCell  * m_pPathCells;
        //!  Node paths count.
        int32_t         m_iNodePaths;
        //!  Node path vertices.
        Vec3       * m_nodeObjectVerts;
        //! Node materials.
        SMaterialProp   * m_pMaterial;
    };
    
    ///  Beautiful game landscape chunk.
    class LandscapeChunk
    {
        NEKO_NONCOPYABLE( LandscapeChunk );
        
    public:
 
        /**
         *  Constructor.
         */
        LandscapeChunk();
        
        /**
         *  Destructor.
         */
        ~LandscapeChunk();
        
        /**
         *  Create a new path node.
         */
        void                CreatePathNode( SChunkPathCell * nodeCell );
        
        /**
         *  Add node position.
         */
        void                    AddNode( const Vec3 position, const float width );
        
        /**
         *  Create a path mesh.
         */
        void                    CreatePath( int32_t smoothingLevel, bool flatten, bool road );
        
        /**
         *  Called when node paths have been created.
         */
        bool                    FinalizePath();
        
        /**
         *  Get current path node.
         *
         *  @note   Can return NEKO_NULL.
         */
        inline SPathInfo   * GetPathNode() const
        {
            if( m_iNodePaths == 0 ) {
                return NEKO_NULL;
            }
            
            return m_pPathNodes[m_iNodePaths - 1];
        }
        
        /**
         *  Render path nodes ( roads, ways, etc.. ).
         */
        void                    RenderPathNodes();
       
        /**
         *  Create landscape, params set in "Load".
         */
        void                    Create( INekoAllocator * allocator, SMemoryTempFrame * tempMemory, bool threaded = true ) ;
        
        /**
         *  Create path node cache.
         */
        void                    CreateNodeCache( SPathInfo * pathInfo, const uint32_t maxNodes );
        
        /**
         *  Clear landscape properties.
         */
        void                    Clear();

        /**
         *  Refresh terrain settings and shader stuff.
         */
        void                    Refresh();

        /**
         *  Delete terrain.
         */
        void                    Destroy();
    
        /**
         *  Draw terrain objects.
         */
        void                    RenderObjects( CRenderer::ESceneEye eye, /*bool isReflection, bool isShadow*/int16_t flags );
        
        /**
         *  Load terrain.
         */
        void                    Load( SGLShader *g_pbEnvShader, SBoundingBox& mbox, const uint32_t chunkObjectsNum, const uint32_t chunkSize, const int32_t startX, const int32_t startY );
        
        /**
         *  Render terrain.
         */
        void                    Render( CRenderer::ESceneEye eye, /*bool isReflection, bool isShadow = false*/int32_t flags );

        /**
         *  Update chunk on renderer frame.
         */
        void                    UpdateFrame( SGLShader * cullShader );
      
        
        //! Vertex data to edit.
        SEditVertices       m_verticesToEdit;
        //! Everything is going to be freed after it's use.
        Vec3			*   m_Normals;
        Vec3			*   m_Positions;
        
        /**
         *  Create buffers and misc. stuff.
         */
        void                    CreateBuffers( const int32_t maxLod = 3);
        
        /**
         *  Render foliage.
         */
        void                    DrawFoliage();

        /**
         *  Render objects, foliages and misc. thingies ^:^.
         */
        void                    RenderEntities( CRenderer::ESceneEye eye, int16_t flags );
        
        /**
         *  Make terrain objects.
         */
        void                    MakeObjects( const int32_t maxLod = 0 );
     
        /**
         *  Create terrain vertex array/buffer objects.
         */
        void                    CreateVertexObjects();
       
        /**
         *  Create quadtree Level of detail.
         */
        void                    CreateQuadtree( const uint32_t chunking = 64 ) ;

        /**
         *  Finish quadtree creation.
         */
        void                    FinishQuadtree( const int32_t maxLod = 3 );
        
        /**
         *  Get random position on chunk.
         */
        const Vec3 &               GetRandomPosition();

        /**
         *  Get terrain size.
         */
        inline const uint32_t               GetSize() const   {       return m_iSize; }
     
        /**
         *   Get terrain common size.
         */
        inline const uint32_t               GetChunkSize() const  {       return m_iSize * m_iSize;   }
        
        /**
         *  Get object count.
         */
        inline const uint32_t               GetActiveObjectCount() const  {       return m_iObjectsDrawn; }
        
        /**
         *  Get drawn faces count/amount/groups etc.
         */
        inline uint32_t                 GetFacesDrawn() const   {       return m_iFacesDrawn;   }

        /**
         *  Get terrain quadtree.
         */
        inline CQuadtree *              GetQuadtree() const  {       return m_tQuadtree; }

        /**
         *  Rebuild current chunk?
         */
        inline bool                 NeedsRebuild() const    {       return bNeedsRebuild;   }
        
        /**
         *  Do we need to recreate GPU buffers?
         */
        inline bool                 NeedsBufferRecreate() const {       return bRecreateBuffers;    }

        /**
         *  Get vertex height at the point.
         */
        inline float GetHeightAt( const float x, const float z, const float maxHeight = 2048.0f, const float maxDistance = 8092.0f )
        {
            SRaycastInfo rayHit;
            
            CQuadtreeNode * qtreenode = m_tQuadtree->FindNode( Vec2( x, z ) );
            
            // Will be inverted if source is under the node.
            rayHit.m_vDirection = Vec3( 0.0f, -1.0f, 0.0f );
            rayHit.m_vOrigin = Vec3( x, maxHeight, z );
            rayHit.m_fMaxDistance = maxDistance;
            rayHit.m_pGroundNode = qtreenode;
            
            g_pWorldPhysics->RayHitIntersection( &rayHit );
            
            return rayHit.m_vHitOrigin.y;
        }
        
        /**
         *  Get height at noise.
         *
         *  @note THIS will NOT work on non-procedural surface!
         */
        float               GetNoiseHeightAt( int32_t x, int32_t z, bool localCoords = false );
        
        /**
         *  Get landscape chunk's bounding box.
         */
        inline const SBoundingBox  *                GetBoundingBox() const    {       return &m_tBoundingBox; }
      
        /**
         *  Smoothen landscape edges.
         */
        void                Modify( bool smoothAll = false );
        
        //!  Terrain objects drawn.
        uint32_t                 m_iObjectsDrawn;
        
        void        RenderGPUCulledObjects();
        
        void        UpdateGPUCulledObjects();
        
        
        BoundingSphere*  m_pBoundingSphere;
        
    private:
        
        INekoAllocator      * pAllocator = 0;
        INekoAllocator      * pAllocatorHandle = 0;
        
        //!  Main terrain shader ( should be moved to g_pbEnv ).
        SGLShader  * m_pChunkShader, * m_pRoadShader;
        
        //! Road uniforms for shader.
#   define LOC_MODELVIEW   0
#   define LOC_MODELMATRIX 1
        
        //! Road uniforms location data.
        int32_t     m_iRoadLoc[3];
        
        //!   Is terrain active?
        bool        bActive;
        
        //!  Do we need to rebuild current chunk?
        bool        bNeedsRebuild;
        
        //!  Do we need to recreate buffers?
        bool        bRecreateBuffers;
        
        //!  Bounding box.
        SBoundingBox        m_tBoundingBox;
        
        //!  Quad tree level of detail.
        CQuadtree          *m_tQuadtree;
        
        //!  Terrain faces draw.
        uint32_t        m_iFacesDrawn;
        
        uint32_t            m_map_size; // W * H
        uint32_t            m_index_size;
        
        //! Foliage count.
        uint32_t            m_foliageObjectCount;
        
        //! Landscape Chunk size.
        uint32_t            m_iSize;
        uint32_t            m_iSizeM1; //! \ m_iSize - 1
        
        bool                m_bRebuildNormals;  //! Recalculate normals for lighting?
        
        /// Instanced object draw batch.
        struct  SInstancedObjectDrawInfo
        {
            struct InstanceData
            {
                Vec3 position;					// position of the instance
                //                Vec3 scale;
            };
            
            //! A mesh to render ( with its LOD meshes ).
            ncMesh *     m_pMesh[MAX_MESH_LOD];
            //! Object instance data buffer.
            uint32_t    m_iInstanceObjectData;
            //! Objects drawn in this batch.
            uint32_t    m_iVisibleObjects[MAX_MESH_LOD];		// number of visible trees
            //! Instanced buffer objects.
            uint32_t    m_iInstanceBufferObjects[MAX_MESH_LOD];		// instance buffer objects
            //! Vertex array with instanced data.
            uint32_t    m_iCullVertexArray;					// vertex array used for culling
            //! Culling query. Helps us to get information from GPU.
            GLuint m_iCullQuery[3];		// query object used for the culling
            
            LandscapeChunk  * m_pNode;
        };
        
        SInstancedObjectDrawInfo    * m_pMeshInstances[MAX_MESH_INSTANCES];
        //! How many mesh base instances were created?
        int32_t     m_iMeshInstanceBaseCount;
        
    public:
        
        //! Path nodes.
        SPathInfo       ** m_pPathNodes;
        uint32_t        m_iNodePaths;       //! Total path count.
        
        //! Was it created on the different thread?
        bool        m_bThreaded;
        
        
        //!  Landscape chunk X origin.
        int32_t       m_fPositionX;
        
        //!  Landscape chunk Z origin.
        int32_t       m_fPositionY;
        
        //!  Noise start X.
        int32_t     m_fNoiseStartX;
        
        //!  Noise start Z.
        int32_t     m_fNoiseStartZ;

        /*      GPU elements.       */
        CBasicMesh     * m_pMeshData;
        
        /**
         *  Calculate normals for chunk.
         *
         *  @param diameter    Normal diameter.
         *  @param index       Index number.
         *  @param normalFaces Out value.
         *  @param cheap       Fast generated normals?
         */
        void		CalculateNormals( int32_t diameter, uint32_t index, Vec3 * normalFaces, bool cheap = false );
    };

}

#endif
