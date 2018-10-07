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
//  Mesh.h
//  Model loader and manager..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef model_h
#define model_h

#include "../AssetCommon/Material/MaterialLoader.h"
#include "../Core/Utilities/Hashtable.h"
#include "../Core/Physics/PlayerPhysics.h"
#include "../Graphics/OpenGL/GLShader.h"
#include "../Graphics/RendererMisc.h"
#include "../Math/BoundingBox.h"
#include "../Math/Mat4.h"


// Assimp includes.
// TODO: Make a new library!
#   if !defined( NEKO_SERVER )
#   if !defined( NEKO_APPLE_FAMILY ) // x64
        #include <assimp/scene.h>
        #include <assimp/Importer.hpp>
        #include <assimp/postprocess.h>
    #   else
        #include "scene.h"
        #include "Importer.hpp"
        #include "postprocess.h"
    #   endif
#   endif

#   if !defined( NEKO_SERVER )
namespace Neko {
    
    //!  Max mesh animation bones.
    static const uint32_t MAX_BONES = 100;
    
    //!  Max bone count per vertex.
    static const uint32_t NUM_BONES_PER_VERTEX = 4;
    
    //!  Vertex bone data.
    struct SVertexBoneData
    {
        uint32_t    m_IDs[NUM_BONES_PER_VERTEX];
        float   m_fWeights[NUM_BONES_PER_VERTEX];
        
        SVertexBoneData()
        {
            Reset();
        };
        
        /**
         *  Reset bone IDs and Weight data.
         */
        void Reset()
        {
            memset( m_IDs, 0, sizeof(uint32_t) * NUM_BONES_PER_VERTEX );
            memset( m_fWeights, /*0.0f*/0x00, sizeof(float) * NUM_BONES_PER_VERTEX );
        }
        
        /**
         *  Add bone data.
         */
        void AddBoneData( uint32_t BoneID, float Weight );
    };
    
    
    const static aiVector3D AssimpVector3dZero( 0.0f, 0.0f, 0.0f );
    
    //!  Bone mapping.
    struct MeshAnimationTag
    {
        int32_t     id;
        char    name[32];
    };
    
    /// Mesh transform info.
    struct MeshTransform
    {
        ncMatrix4 ModelView;
        ncMatrix4 NormalMatrix;
        ncMatrix4 ModelViewProjection;
        ncMatrix4 PrevMatrix;
        
        float time;
    };
    
    ///   Main mesh loader.
    class CMesh
    {
 
        
    public:
        CMesh();
        ~CMesh();
        
        /**
         *  Prepare mesh.
         */
        void                Prepare();
        
        /**
         *  Load mesh.
         */
        void                LoadMesh( const char * filename, INekoAllocator * allocator, bool fromPak = false, bool threaded = false  );
        
        /**
         *  Render mesh.
         */
        void                Render( const int32_t instanceCount = 0, const int32_t lod = 0 );
        
        /**
         *  Get bone count.
         */
        inline const uint32_t &                     NumBones() const    {       return m_iBoneInfoCount;    }

        /**
         *  Get vertex array.
         */
        inline GLuint &                 GetVertexArray() {     return m_VAO; }
        
        /**
         *  Get model name.
         */
        inline const CStr &                 GetName() const {     return m_Name; }
        
        /**
         *  Get mesh bounding box.
         */
        inline SBoundingBox &           GetBoundingBox() {     return m_BBox; }
        
        /**
         *  Get mesh bounding sphere.
         */
        inline BoundingSphere *                     GetBoundingSphere() {       return m_pBSphere;  }
        
        /**
         *  Bone transform for animation.
         */
        void                    BoneTransform( float TimeInSeconds  );
       
        /**
         *  Are there any bone in our mesh?
         */
        inline bool                 IsAnimated() const {     return m_bIsAnimated; }

        //! Bone locations in the shaders.
        GLuint  m_boneLocation[MAX_BONES];

        //! Mesh transform info.
        MeshTransform transform;
        
        INekoAllocator  * pAllocator;
        
//    private:
        
#   if defined( USES_METAL )
        int32_t     iMTLBuffer;
#   endif
        
        //!  Bounding box.
        SBoundingBox   m_BBox;
        //!  Bounding sphere.
        BoundingSphere * m_pBSphere;
        
        //!  Name.
        CStr   m_Name;
        
        uint32_t    m_iVerticeNum;
        uint32_t    m_iIndiceNum;
        uint32_t    m_iBoneCount, m_iBoneInfoCount;
        
        struct SBoneInfo
        {
            aiMatrix4x4t<float> BoneOffset;
            aiMatrix4x4t<float> FinalTransformation;
            
            SBoneInfo()
            {

            }
        };

        /**
         *  Calculate scale for animation node.
         */
        void                CalculateScaling( aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim );
        
        /**
         *  Calculate rotation for animation node.
         */
        void                CalculateRotation( aiQuaternion & Out, float AnimationTime, const aiNodeAnim * pNodeAnim );
        
        /**
         *  Calculate position for animation node.
         */
        void                CalculatePosition( aiVector3D & Out, float AnimationTime, const aiNodeAnim * pNodeAnim );
  
        
        /**
         *  Make buffer objects and another misc. cool stuff.
         */
        void                MakeBuffers();
        
        
        /**
         *  Find scale value for animation node.
         */
        uint32_t                FindScaling( float AnimationTime, const aiNodeAnim * pNodeAnim );
        
        /**
         *  Find rotation value for animation node.
         */
        uint32_t                FindRotation( float AnimationTime, const aiNodeAnim * pNodeAnim );
        
        /**
         *  Find position value for animation node.
         */
        uint32_t                FindPosition( float AnimationTime, const aiNodeAnim * pNodeAnim );
        
        
        
        /**
         *  Find node animation by its name.
         */
        const aiNodeAnim *              FindNodeAnim( const aiAnimation * pAnimation, const char * NodeName );
        
        /**
         *  Recursive node heirarchy reading.
         */
        void                    ReadNodeHeirarchy( float AnimationTime, const aiNode * pNode, const aiMatrix4x4t<float> & ParentTransform );
        
        
        /**
         *  Load mesh.
         */
        void                InitMesh( const aiScene* pScene, const char * filename, bool threaded = false );

        /**
         *  Load mesh vertice, normal, bone, uv, etc data.
         */
        void                LoadMesh( uint32_t MeshIndex,
                                    const aiMesh* paiMesh,
                                    Vec3 * Positions,
                                    Vec3 * Normals,
                                    Vec2 * TexCoords,
                                    SVertexBoneData * Bones,
                                    uint32_t * Indices, const aiScene * scene );
        
        /**
         *  Load bones ( if animation is present ).
         */
        void                LoadBones( uint32_t MeshIndex, const aiMesh * paiMesh, SVertexBoneData * Bones );
        
        /**
         *  Load mesh materials.
         */
        void                InitMaterials( const aiScene * pScene, const char * Filename );
        
        /**
         *  Clear all buffers and delete textures.
         */
        void            Clear();
        
#define INVALID_MATERIAL 0xFFFFFFFF
  
#define POSITION_LOCATION    0
#define UV_LOCATION          1
#define NORMAL_LOCATION      2
#define BONE_ID_LOCATION     3
#define BONE_WEIGHT_LOCATION 4
        
        //         GPU Buffers.        
        
        //! Vertex array
        GLuint  m_VAO;
        
        //! Vertices
        GPUBuffer   vertices;
        //! Mesh normals
        GPUBuffer   normals;
        //! UV coords
        GPUBuffer   uvs;
        //! Bone data
        GPUBuffer   boneData;
        //! Index buffer data
        GPUBuffer   indexBuffer;
        
        // this will be nulled as mesh will be created
        Vec3 * m_vPositions;
        Vec3 * m_vNormals;
        Vec2 * m_vTexCoords;
        SVertexBoneData * m_iBones;
        uint * m_iIndices;
        
        //!  Mesh structure for every model group.
        struct SMeshEntry
        {
            SMeshEntry()
            {
                NumIndices    = 0;
                BaseVertex    = 0;
                BaseIndex     = 0;
                MaterialIndex = INVALID_MATERIAL;
            }
            
            uint32_t    NumIndices;
            uint32_t    BaseVertex;
            uint32_t    BaseIndex;
            uint32_t    MaterialIndex;
        };
        
        //! Meshes.
        SMeshEntry   * m_meshEntries;
        
        // Textures.
        SMaterialProp    ** m_pMaterial;
        
        uint32_t    m_iNumBones;
        uint8_t    m_Topology; // GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;
        
        SBoneInfo    * m_boneInfo;
        CHashMap<const char*, int32_t*>  m_animTags;    // Maps a bone name to its index.
        
        uint8_t    m_bIsAnimated:1;
        
        //!  Assimp scene and importer.
        const aiScene   * m_pScene;
        Assimp::Importer    * m_Importer; //!<
        
    };

}

#endif
#endif
