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
//  Mesh.cpp
//  Game model loader & renderer..
//
//  Created by Neko Vision on 3/4/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../AssetCommon/AssetBase.h"
#include "../AssetCommon/FileSystem.h"
#include "../AssetCommon/Material/MaterialLoader.h"
#include "../Core/Console/ConsoleCommand.h"
#include "../Core/Core.h"
#include "../Core/Player/Camera/Camera.h"
#include "../Core/Player/Camera/Camera.h" // Frustum.
#include "../Core/ScriptSupport/Scripting.h"
#include "../Core/String/StringHelper.h"
#include "../Core/Utilities/Utils.h"
#include "../Graphics/Renderer/Renderer.h"
#include "BeautifulEnvironment.h"
#include "Mesh.h"

#   if !defined(NEKO_SERVER)
#   if defined(NEKO_APPLE_FAMILY)
        #include "cimport.h"
#   else
        #include <assimp\cimport.h>
#   endif
#   endif

#   if defined(USES_METAL)
        #include "MetalExternal.h"
#   endif

// @note somehow I formated this code to llvm convention and I am lazy to make it as it was
// @todo   own mesh format to replace assimp
// @todo   node linker for bones and whatever big
//
// Assimp notes:
// aiTextureType_NORMALS is a HEIGHT property for some reason ( and vice-versa ).
//
#   if !defined( NEKO_SERVER )
namespace Neko {
    
    /**
     *  Use this when current API requests that.
     *  Adds offset to indexes. Fixes rendering problem.
     */
#   if defined(USES_METAL) || defined(USES_D3D)
    #define INDEX_OFFSET_ADD
#   endif
    
    /**
     *  New mesh instance.
     */
    CMesh::CMesh()
    {
        Prepare();
    }
    
    /**
     *  Prepare a new mesh.
     */
    void CMesh::Prepare()
    {
        // Vertex array.
        m_VAO = 0;
        
        // Clear buffers.
        memset( &vertices, 0, sizeof(GPUBuffer) );
        memset( &uvs, 0, sizeof(GPUBuffer) );
        memset( &normals, 0, sizeof(GPUBuffer) );
        memset( &boneData, 0, sizeof(GPUBuffer) );
        memset( &indexBuffer, 0, sizeof(GPUBuffer) );
        
        // Default.
        m_Topology = GL_TRIANGLES;
        
        // Total bone info count ( id, weight ).
        m_iBoneInfoCount = 0;
        
        // Assimp scene.
        m_pScene = NEKO_NULL;
        // Mesh textures.
        m_pMaterial = NEKO_NULL;
        // Mesh data.
        m_meshEntries = NEKO_NULL;
        m_boneInfo = NEKO_NULL;
        m_pBSphere = NEKO_NULL;
        
        // Is animated?
        m_bIsAnimated = false;
    }
    
    /**
     *  Destructor.
     */
    CMesh::~CMesh()
    {
        Clear();
    }
    
    /**
     *  Remove all buffers and textures.
     */
    void CMesh::Clear()
    {
        // It's all is okay!
        
        // Delete textures.
        if( m_pMaterial != NEKO_NULL ) {
            pAllocator->Dealloc(m_pMaterial);
        }
        
        // Delete meshes.
        if( m_meshEntries ) {
            pAllocator->Dealloc(m_meshEntries);
        }
        // Delete bones.
        if( m_boneInfo != NEKO_NULL ) {
            pAllocator->Dealloc(m_boneInfo);
        }
        // Animation tags.
        if( m_animTags.GetCount() > 0 ) {
            
            int32_t * tag = NEKO_NULL;
            
            SLink * head;
            SLink * cur;
            
            head = &m_animTags.m_List.m_sList;
            
            for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
                tag = (int32_t *)cur->m_ptrData;
             
                pAllocator->Dealloc( tag );
            }
            
            m_animTags.Delete();
            memset( &m_animTags, 0x0, sizeof(m_animTags) );
        }
        
        // Neko allocator!!
        if( m_pBSphere != NEKO_NULL ) {
            delete m_pBSphere;
        }
        
        // Delete buffers on the GPU.
#   if defined( USES_OPENGL )
        if( vertices.Handle ) {
            g_mainRenderer->DeleteGPUBuffer(&vertices);
            g_mainRenderer->DeleteGPUBuffer(&uvs);
            g_mainRenderer->DeleteGPUBuffer(&normals);
            g_mainRenderer->DeleteGPUBuffer(&indexBuffer);
        }
        
        if( boneData.Handle ) {
            g_mainRenderer->DeleteGPUBuffer(&boneData);
        }
        
        // Delete vertex array.
        if( m_VAO ) {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
#   endif
    }
    
    /**
     *  Convert Assimp matrix to Neko matrix.
     *
     *  @note Probably I've made it temporarily, I am about to replace Assimp by my
     * own.
     */
    inline static ncMatrix4 aiMatrix4x4ToGlm(const aiMatrix4x4 *from)
    {
        ncMatrix4 to;
        
        to.m[0] = (GLfloat)from->a1;
        to.m[1] = (GLfloat)from->a2;
        to.m[2] = (GLfloat)from->a3;
        to.m[3] = (GLfloat)from->a4;
        to.m[4] = (GLfloat)from->b1;
        to.m[8] = (GLfloat)from->c1;
        to.m[12] = (GLfloat)from->d1;
        to.m[5] = (GLfloat)from->b2;
        to.m[9] = (GLfloat)from->c2;
        to.m[13] = (GLfloat)from->d2;
        to.m[6] = (GLfloat)from->b3;
        to.m[10] = (GLfloat)from->c3;
        to.m[14] = (GLfloat)from->d3;
        to.m[7] = (GLfloat)from->b4;
        to.m[11] = (GLfloat)from->c4;
        to.m[15] = (GLfloat)from->d4;
        
        return to;
    }
    
    /**
     *  Load our mesh!
     */
    void CMesh::LoadMesh( const char *filename, INekoAllocator *allocator,
                         bool fromPak, bool threaded )
    {
        const char *basepath = NekoCString::STR( "%s/models/%s", Filesystem_Path->Get<const char *>(), filename);
        uint32_t flags; // Assimp loader flags.
        
        // Release the previously loaded mesh (if it exists).
        Clear();
        
        pAllocator = allocator;
        
#   if defined( USES_METAL )
        flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_RemoveRedundantMaterials | aiProcess_FindInstances | aiProcess_GenUVCoords |
        /*aiProcess_Triangulate |*/ aiProcess_FindInvalidData | aiProcess_OptimizeMeshes | aiProcess_SplitLargeMeshes |  aiProcess_CalcTangentSpace | aiProcess_ImproveCacheLocality;
#   else
        flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_RemoveRedundantMaterials | aiProcess_FindInstances |  aiProcess_GenUVCoords |
        /*aiProcess_Triangulate |*/ aiProcess_FindInvalidData | aiProcess_OptimizeMeshes | aiProcess_SplitLargeMeshes | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality;
#   endif
        //
        m_pScene = NEKO_NULL;
        m_Importer = NekoAllocator::AllocateNew<Assimp::Importer>(pAllocator);// new Assimp::Importer(); // TODO: Use my allocator.
        
        // Let's see!
        m_pScene = m_Importer->ReadFile(basepath, flags);
        
        if( m_pScene != NEKO_NULL ) {
            InitMesh(m_pScene, filename, threaded);
        } else {
            g_Core->p_Console->Error(ERR_ASSET,  "Couldn't load %s model. Assimp error:\n%s\n",
                                     filename, m_Importer->GetErrorString());
        }
        
        m_Name = filename;
    }
    
    /**
     *  Initialize mesh buffers.
     */
    void CMesh::InitMesh( const aiScene * pScene, const char *filename, bool threaded )
    {
        if( pScene->mNumMeshes < 1 ) {
            // Ignore "empty" models.
            g_Core->p_Console->Print( LOG_INFO, "LoadMesh(): Couldn't load \"%s\", no meshes found!\n", filename );
            return;
        }
        
        Vec3 * Positions = NEKO_NULL;
        Vec3 * Normals = NEKO_NULL;
        
        Vec2 * TexCoords = NEKO_NULL;
        
        SVertexBoneData * Bones = NEKO_NULL;
        
        uint32_t * Indices = NEKO_NULL;
        
        uint32_t NumVertices = 0;
        uint32_t NumIndices = 0;
        
        g_Core->p_Console->Print(LOG_INFO, "LoadMesh(): %s... ( %i meshes )\n", filename, pScene->mNumMeshes);
        
        // Allocate mesh entries.
        m_meshEntries =  (SMeshEntry *)pAllocator->Alloc(sizeof(SMeshEntry) * pScene->mNumMeshes);
        if( m_meshEntries == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "LoadMesh(): Couldn't allocate space for %i mesh entries.\n",
                                     pScene->mNumMeshes);
            return;
        }
        
        // Allocate material entries.
        m_pMaterial = (SMaterialProp **)pAllocator->Alloc(sizeof(SMaterialProp *) *  pScene->mNumMaterials);
        
        const uint32_t VerticesPerPrim = /*m_withAdjacencies ? 6 :*/ 3;
        
        uint32_t i;
        // Count the number of vertices and indices
        for( i = 0; i < pScene->mNumMeshes; ++i ) {
            m_meshEntries[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
            
#   if defined( INDEX_OFFSET_ADD )
            m_meshEntries[i].NumIndices =  (pScene->mMeshes[i]->mNumFaces * VerticesPerPrim) + NumVertices;
#   else
            m_meshEntries[i].NumIndices =  pScene->mMeshes[i]->mNumFaces * VerticesPerPrim;
#   endif
            
            m_meshEntries[i].BaseVertex = NumVertices;
            m_meshEntries[i].BaseIndex = NumIndices;
            
            NumVertices += pScene->mMeshes[i]->mNumVertices;
            NumIndices += m_meshEntries[i].NumIndices;
        }
        
        m_bIsAnimated = pScene->HasAnimations();
        
        SMemoryTempFrame * tempMem = _PushMemoryFrame(pLinearAllocator2);
        
        // Reserve space in the vectors for the vertex attributes and indices.
        Positions = (Vec3 *)PushMemory(tempMem, sizeof(Vec3) * NumVertices);
        Normals = (Vec3 *)PushMemory(tempMem, sizeof(Vec3) * NumVertices);
        TexCoords = (Vec2 *)PushMemory( tempMem, sizeof(Vec2) * NumVertices); // new Vec2[NumVertices];
        
        if( m_bIsAnimated ) {
            // Create bone data if mesh has any animation.
            Bones =  (SVertexBoneData *)pAllocator->Alloc( sizeof(SVertexBoneData) *  NumVertices );
            
            // Bone data (id, weight).
            m_boneInfo =  (SBoneInfo *)pAllocator->Alloc(sizeof(SBoneInfo) * NumVertices);
            
            // Animation tag data.
            m_animTags.Create( NumVertices * sizeof(uint32_t), pLinearAllocator2, pAllocator );
        }
        
        Indices = (uint32_t *)PushMemory(tempMem, sizeof(uint) * NumIndices);
        
        m_iVerticeNum = 0;
        m_iIndiceNum = 0;
        m_iBoneCount = 0;
        m_iBoneInfoCount = 0;
        
        // Initialize the meshes in the scene one by one
        for( i = 0; i < pScene->mNumMeshes; ++i ) {
            const aiMesh *paiMesh = pScene->mMeshes[i];
            
            LoadMesh(i, paiMesh, Positions, Normals, TexCoords, Bones, Indices, pScene);
        }
        
        // It's time to load mesh materials!
        InitMaterials(pScene, filename);
        
        // Create bounding sphere.
        m_pBSphere = BoundingSphere::Calculate(Positions, NumVertices);
        
        // Bounding box.
        memset(&m_BBox, 0x00, sizeof(SBoundingBox));
        
        m_BBox.min = Vec3(100000.0f, 100000.0f, 100000.0f);
        m_BBox.max = Vec3(-100000.0f, -100000.0f, -100000.0f);
        
        for( i = 0; i < NumVertices; ++i ) {
            m_BBox.Add(Positions[i]);
        }
        
        _PopMemoryFrame(tempMem);
        
        
        // Set data pointers
        m_vPositions = Positions;
        m_vNormals = Normals;
        m_vTexCoords = TexCoords;
        m_iBones = Bones;
        m_iIndices = Indices;
        
        
        if( !threaded ) {
#   if defined( USES_OPENGL )
            // Make buffers.
            MakeBuffers();
#   else
            // Metal.
            iMTLBuffer = g_mainRenderer->metalInstance->CreateMesh(  Positions, Normals, TexCoords, Bones, Indices, iVerticeNum, iIndiceNum);
#   endif
        }
    }
    
    /**
     *  Fill mesh buffer info.
     */
    void CMesh::LoadMesh(uint32_t MeshIndex, const aiMesh *paiMesh, Vec3 *Positions,
                         Vec3 *Normals, Vec2 *TexCoords, SVertexBoneData *Bones,
                         uint32_t *Indices, const aiScene *scene)
    {
        // Fill vertex properties.
        
        uint32_t i;
        
        for (i = 0; i < paiMesh->mNumVertices; ++i) {
            const aiVector3D *pPos = &(paiMesh->mVertices[i]);
            const aiVector3D *pNormal = &(paiMesh->mNormals[i]);
            const aiVector3D *pTexCoord = paiMesh->HasTextureCoords(0)
            ? &(paiMesh->mTextureCoords[0][i])
            : &AssimpVector3dZero;
            
            // Do not use 'i' here from current loop, breaks meshes ( dumbass ).
            Positions[m_iVerticeNum] = Vec3(pPos->x, pPos->y, pPos->z);
            Normals[m_iVerticeNum] = Vec3(pNormal->x, pNormal->y, pNormal->z);
            TexCoords[m_iVerticeNum] = Vec2(pTexCoord->x, pTexCoord->y);
            
            ++m_iVerticeNum;
        }
        
        // Let's check mesh animation availability.
        if (m_bIsAnimated == true) {
            LoadBones(MeshIndex, paiMesh, Bones);
        }
        
        // Set the topology.
        m_Topology = GL_TRIANGLES;
        
#   if defined( INDEX_OFFSET_ADD )
        int32_t offset;
        int32_t k;
        
        offset = 0;
        
        if (MeshIndex > 0) {
            for (k = 0; k < MeshIndex; k++) {
                offset += scene->mMeshes[k]->mNumVertices;
            }
        }
        
        for (i = 0; i < paiMesh->mNumFaces; ++i) {
            const aiFace &Face = paiMesh->mFaces[i];
            
            if (Face.mNumIndices == 3) {
                Indices[iIndiceNum] = Face.mIndices[0] + offset;
                ++iIndiceNum;
                Indices[iIndiceNum] = Face.mIndices[1] + offset;
                ++iIndiceNum;
                Indices[iIndiceNum] = Face.mIndices[2] + offset;
                ++iIndiceNum;
            }
        }
#   else
        for (i = 0; i < paiMesh->mNumFaces; ++i) {
            const aiFace &Face = paiMesh->mFaces[i];
            
            if (Face.mNumIndices == 3) {
                // Face must contain three indices.
                Indices[m_iIndiceNum] = Face.mIndices[0];
                ++m_iIndiceNum;
                Indices[m_iIndiceNum] = Face.mIndices[1];
                ++m_iIndiceNum;
                Indices[m_iIndiceNum] = Face.mIndices[2];
                ++m_iIndiceNum;
            }
        }
#   endif
    }

    /**
     *  Load mesh animation bones.
     */
    void CMesh::LoadBones(uint32_t MeshIndex, const aiMesh *pMesh,
                          SVertexBoneData *Bones)
    {
        uint32_t i;
        
        uint32_t BoneIndex;
        const char *BoneName;
        
        // Look for an existing bones.
        int32_t * curAnim;
        
        g_Core->p_Console->Print(LOG_INFO, "Mesh(%i) has %i bones.\n", MeshIndex,
                                 pMesh->mNumBones);
        
        for( i = 0; i < pMesh->mNumBones; ++i ) {
            BoneIndex = 0;
            BoneName = pMesh->mBones[i]->mName.data;
            curAnim = m_animTags[BoneName];
            
            if (curAnim == NEKO_NULL /* Not found */) {
                // Allocate an index for a new bone.
                BoneIndex = m_iBoneInfoCount;
                
                SBoneInfo bi = SBoneInfo();
                m_boneInfo[m_iBoneInfoCount] = bi;
                
                m_animTags[BoneName] = (int32_t *)pAllocator->Alloc (sizeof(int32_t));
                *m_animTags[BoneName] = BoneIndex;
                
//                NekoCString::Copy(m_animTags[BoneName].name, BoneName);
                
                ++m_iBoneInfoCount;
            } else {
                BoneIndex = *curAnim;
            }
            
            // Set bone offset matrix.
            m_boneInfo[BoneIndex].BoneOffset = pMesh->mBones[i]->mOffsetMatrix; //
            
            uint32_t j;
            uint32_t VertexID;
            
            float Weight;
            
            for (j = 0; j < pMesh->mBones[i]->mNumWeights; ++j) {
                VertexID = m_meshEntries[MeshIndex].BaseVertex +
                pMesh->mBones[i]->mWeights[j].mVertexId;
                Weight = pMesh->mBones[i]->mWeights[j].mWeight;
                
//                if(Weight == .0f )
//                    continue;
                
                Bones[VertexID].AddBoneData(BoneIndex, Weight);
                ++m_iBoneCount;
            }
        }
    }
    
    /**
     *  Make buffers and another cool stuff.
     */
    void CMesh::MakeBuffers()
    {
        // Create the buffers for the vertices attributes.
        
        // Generate and populate the buffers with vertex attributes and the indices.
      
        // Create the VAO.
        m_VAO = g_mainRenderer->CreateVertexArray();
        
        // Vertice data.
        vertices = g_mainRenderer->AllocGPUBuffer( sizeof(m_vPositions[0]) * m_iVerticeNum, EBufferStorageType::Array, EBufferType::Static);
        g_mainRenderer->BufferData(&vertices, &m_vPositions[0], 0, sizeof(m_vPositions[0]) * m_iVerticeNum);
        g_mainRenderer->BufferPointer(&vertices, POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
        g_mainRenderer->FinishBuffer(&vertices, 0, m_iVerticeNum);
        
        // UV data.
        uvs = g_mainRenderer->AllocGPUBuffer(sizeof(m_vTexCoords[0]) * m_iVerticeNum, EBufferStorageType::Array, EBufferType::Static);
        g_mainRenderer->BufferData(&uvs, &m_vTexCoords[0], 0, sizeof(m_vTexCoords[0]) * m_iVerticeNum);
        g_mainRenderer->BufferPointer(&uvs, UV_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);
        g_mainRenderer->FinishBuffer(&vertices, 0, m_iVerticeNum);
        
        // Normal data.
        normals = g_mainRenderer->AllocGPUBuffer(sizeof(m_vNormals[0]) * m_iVerticeNum, EBufferStorageType::Array, EBufferType::Static);
        g_mainRenderer->BufferData(&normals, &m_vNormals[0], 0, sizeof(m_vNormals[0]) * m_iVerticeNum);
        g_mainRenderer->BufferPointer(&normals, NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);
        g_mainRenderer->FinishBuffer(&normals, 0, m_iVerticeNum);
        
        // So if it's animated add some bones!
        if (m_bIsAnimated) {
            // Bone buffer.
            boneData = g_mainRenderer->AllocGPUBuffer(sizeof(m_iBones[0]) * m_iBoneCount, EBufferStorageType::Array, EBufferType::Static);
            g_mainRenderer->BufferData(&boneData, &m_iBones[0], 0, sizeof(m_iBones[0]) * m_iBoneCount);
            g_mainRenderer->BufferIPointer(&boneData, BONE_ID_LOCATION, 4, GL_INT, sizeof(SVertexBoneData), (const GLvoid *)0);
            g_mainRenderer->BufferPointer(&boneData, BONE_WEIGHT_LOCATION, 4, GL_FLOAT,  GL_FALSE, sizeof(SVertexBoneData),  (const GLvoid *)16);
            g_mainRenderer->FinishBuffer(&boneData, 0, m_iVerticeNum);
            
            // Set up bone locations. We're using animation shader to animate our
            // models.
            f_AssetBase->p_MeshBase->fxmodelAnim->Use();
            
            uint32_t i;
            
            for (i = 0; i < MAX_BONES; ++i) {
                char Name[128];
                memset(Name, 0, sizeof(Name));
                snprintf(Name, sizeof(Name), "gBones[%i]", i);
                
                m_boneLocation[i] = f_AssetBase->p_MeshBase->fxmodelAnim->UniformLocation(Name);
            }
            
            f_AssetBase->p_MeshBase->fxmodelAnim->Next();
            
            pAllocator->Dealloc( (void *)m_iBones );
        }
        
        // Fill index buffer.
        indexBuffer = g_mainRenderer->AllocGPUBuffer( sizeof(m_iIndices[0]) * m_iIndiceNum, EBufferStorageType::IndexArray, EBufferType::Static);
        g_mainRenderer->BufferData(&indexBuffer, &m_iIndices[0], 0, sizeof(m_iIndices[0]) * m_iIndiceNum);
        g_mainRenderer->FinishBuffer(&indexBuffer, 0, m_iIndiceNum);
        // Make sure the VAO is not changed from the outside.
        g_mainRenderer->UnbindVertexArray();
    }
    
    /**
     *  Load mesh materials.
     */
    void CMesh::InitMaterials(const aiScene *pScene, const char *Filename)
    {
        if (pScene->HasMaterials())
        {
            uint32_t i;
            
            for (i = 0; i < pScene->mNumMaterials; ++i) {
                const aiMaterial *pMaterial = pScene->mMaterials[i];
                
                m_pMaterial[i] = NEKO_NULL;
                
                // Check if we have textures.
                if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
                    aiString Path;
                    
                    // Diffuse map..
                    if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path,
                                              NEKO_NULL, NEKO_NULL, NEKO_NULL, NEKO_NULL, NEKO_NULL) == AI_SUCCESS) {
                        const char *res = CFileSystem::GetFileNameWithoutExtension(Path.data);
                        m_pMaterial[i] = f_AssetBase->p_MaterialBase->Find("coupe");
                        
 //                        printf( "%s\n", res  );
                    }
                    
                } else {
                    // Else set it to default one.
                    m_pMaterial[i] = f_AssetBase->p_MaterialBase->Find("null");
                }
            }
        } else {
            g_Core->p_Console->Print(
                                     LOG_INFO, "Mesh::InitMaterials() - No materials for %s\n", Filename);
        }
    }
    
    /**
     *  Render meshes.
     */
    void CMesh::Render(const int32_t instanceCount, const int32_t lod)
    {
        unsigned int i;
#   if defined( USES_OPENGL )
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.Handle);
        
        int32_t lodNum = m_pScene->mNumMeshes;
        
        for (i = lod; i < lodNum; ++i) {
            const uint32_t MaterialIndex = m_meshEntries[i].MaterialIndex;
            
            if( m_pMaterial[MaterialIndex] == NEKO_NULL ) {
                continue;
            }
            
            if (m_pMaterial[MaterialIndex] ->m_pDiffuse) {
                g_mainRenderer->BindTexture( 0, m_pMaterial[MaterialIndex]->m_pDiffuse->Image.GetId());
            }
            
            if (m_pMaterial[MaterialIndex]->m_pNormal) {
                g_mainRenderer->BindTexture( 1, m_pMaterial[MaterialIndex]->m_pNormal->Image.GetId());
            }
            
            if (m_pMaterial[MaterialIndex]->m_pSpecular) {
                g_mainRenderer->BindTexture( 2, m_pMaterial[MaterialIndex]->m_pSpecular->Image.GetId());
            }
            
            if (instanceCount == 0) {
                glDrawElementsBaseVertex( m_Topology, m_meshEntries[i].NumIndices, GL_UNSIGNED_INT,
                                         (void *)(sizeof(uint32_t) * m_meshEntries[i].BaseIndex),
                                         m_meshEntries[i].BaseVertex);
            } else { // Instanced drawing.
//                glDrawElementsInstancedBaseVertex( m_Topology,
//                                         m_meshEntries[i].NumIndices,
//                                         GL_UNSIGNED_INT,
//                                         (void*)(sizeof(uint32_t) *
//                                         m_meshEntries[i].BaseIndex),
//                                         instanceCount,
//                                         m_meshEntries[i].BaseVertex );
                glDrawElementsInstanced(m_Topology, m_meshEntries[i].NumIndices,
                                        GL_UNSIGNED_INT, NEKO_NULL, instanceCount);
            }
            
            g_mainRenderer->UnbindTexture(0);
        }
        
        // Make sure the VAO is not changed from the outside
        glBindVertexArray(0);
        
#   else // Metal
        
        // "Bind buffer"
        g_mainRenderer->metalInstance->BindMeshAtIndex(iMTLBuffer);
        
        for (i = 0; i < m_pScene->mNumMeshes; ++i) {
            const uint32_t MaterialIndex = m_meshEntries[i].MaterialIndex;
            
            g_mainRenderer->metalInstance->RenderMeshAtIndexWithIndiceData( iMTLBuffer, m_meshEntries[i].NumIndices, m_meshEntries[i].BaseIndex,
                                                                           m_meshEntries[i].BaseVertex, m_Textures[MaterialIndex]);
            // g_mainRenderer->metalInstance->RenderMeshAtIndexWithVertexData(
            // iMTLBuffer, m_Entries[i].BaseVertex, iVerticeNum,
            // m_Textures[MaterialIndex] );
        }
        
#   endif
    }
    
    /**
     *  Update mesh.
     */
    void ncMesh::Update()
    {
        m_projectionMatrix = g_Core->p_Camera->ViewProjectionMatrix * m_modelView;
        
        if (Use_UniformBuffers->Get<bool>()) {
            m_prevView = g_Core->p_Camera->prevViewProj * m_localToWorldPrev;
            m_hOwner->transform.ModelView = m_modelView;
            m_hOwner->transform.NormalMatrix = m_normalView;
            m_hOwner->transform.ModelViewProjection = m_projectionMatrix;
            m_hOwner->transform.PrevMatrix = m_prevView;
            m_hOwner->transform.time = g_Core->GetTime() * 0.001f;
            
            glBindBuffer( GL_UNIFORM_BUFFER, f_AssetBase->p_MeshBase->m_iMeshUniformBuffer );
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(m_hOwner->transform), &m_hOwner->transform);
            glBindBufferRange(GL_UNIFORM_BUFFER, f_AssetBase->p_MeshBase->m_iMeshTransformUniform, f_AssetBase->p_MeshBase->m_iMeshUniformBuffer, 0, sizeof(m_hOwner->transform));
        }
    }
    
    /**
     *  Render a mesh.
     */
    void ncMesh::Render(bool isShadow, const int32_t instanceCount /* = 0 */)
    {
        bool meshIsAnimated;
        
        if( m_hOwner == NEKO_NULL ) {
            return;
        }
        
        // Use shader.
        meshIsAnimated = m_hOwner->IsAnimated();
        
        if (instanceCount == 0) {
            Update();
        }
        
        if (meshIsAnimated) {
            
            if (instanceCount == 0) {
                f_AssetBase->p_MeshBase->fxmodelAnim->Use();
            }
            
            if (isShadow == false) {
                m_hOwner->BoneTransform(g_Core->GetTime());
            }
            
            // If not using uniform objects, then just send shader uniforms.
            if (!Use_UniformBuffers->Get<bool>() && instanceCount == 0) {
                // Set properties.
                f_AssetBase->p_MeshBase->fxmodelAnim->SetUniform( f_AssetBase->p_MeshBase->m_pShaderAnimParams[MPVM], 1, false, m_projectionMatrix.m);
                f_AssetBase->p_MeshBase->fxmodelAnim->SetUniform( f_AssetBase->p_MeshBase->m_pShaderAnimParams[MMODELMATRIX], 1, false, m_modelView.m);
                f_AssetBase->p_MeshBase->fxmodelAnim->SetUniform( f_AssetBase->p_MeshBase->m_pShaderAnimParams[MNORMALMATRIX], 1, false, m_normalView.m);
                m_prevView = g_Core->p_Camera->prevViewProj * m_localToWorldPrev;
                f_AssetBase->p_MeshBase->fxmodelAnim->SetUniform( f_AssetBase->p_MeshBase->m_pShaderAnimParams[MPREVMATRIX], 1, false, m_prevView.m);
                f_AssetBase->p_MeshBase->fxmodelAnim->SetUniform( f_AssetBase->p_MeshBase->m_pShaderAnimParams[MTIME], g_Core->GetTime() * 0.001f);
            }
            
            // Render the mesh.
            m_hOwner->Render(instanceCount);
            
        } else {
            
            if (instanceCount == 0) {
                f_AssetBase->p_MeshBase->fxmodel->Use();
            }
            
            if (!Use_UniformBuffers->Get<bool>() && instanceCount == 0) {
                // Set properties.
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MPVM], 1, false, m_projectionMatrix.m);
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MMODELMATRIX], 1, false, m_modelView.m);
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MNORMALMATRIX], 1, false, m_normalView.m);
                m_prevView = g_Core->p_Camera->prevViewProj * m_localToWorldPrev;
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MPREVMATRIX], 1, false, m_prevView.m);
                f_AssetBase->p_MeshBase->fxmodel->SetUniform( f_AssetBase->p_MeshBase->m_pShaderParams[MTIME],  g_Core->GetTime() * 0.001f);
            }
            
            // Render the mesh.
            m_hOwner->Render(instanceCount);
        }
        
        // Disable shader.
        f_AssetBase->p_MeshBase->fxmodel->Next();
    }
    
    /**
     *  Refresh values for the mesh.
     */
    void ncMesh::Refresh()
    {
        m_modelView.Identity();
        
        // m_modelView.Rotate( m_Rotation.y, Vec3( 0.0f, 1.0f, 0.0f ) );
        
        const Vec3 &Rotation = GetRotation();
        m_modelView.Rotate(Rotation.x, Vec3(1.0f, 0.0f, 0.0f));
        m_modelView.Rotate(Rotation.y, Vec3(0.0f, 1.0f, 0.0f));
        m_modelView.Rotate(Rotation.z, Vec3(0.0f, 0.0f, 1.0f));
        
        m_modelView.Scale(GetScale());
        m_modelView.Translate(GetObjectPos());
        
        // TODO: update dynamic model normal matrix
        m_normalView = ncMatrix4(m_modelView);
        m_normalView.Transpose();
        
        float matrix[16];
        NekoUtils::gluInvertMatrix(m_normalView.m, matrix);
        m_normalView = ncMatrix4(matrix);
        
        m_localToWorldPrev = m_modelView;
    }
    
    /**
     *  Find anim position for given node.
     */
    unsigned int CMesh::FindPosition(float AnimationTime,
                                     const aiNodeAnim *pNodeAnim)
    {
        uint32_t i;
        
        for (i = 0; i < pNodeAnim->mNumPositionKeys - 1; ++i) {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
                return i;
            }
        }
        
        g_Core->p_Console->Print( LOG_DEVELOPER, "Mesh::FindPosition() - couldn't find animkey for %4.2f\n", AnimationTime);
        
        return 0;
    }
    
    /**
     *  Find anim rotation for given node.
     */
    uint32_t CMesh::FindRotation(float AnimationTime, const aiNodeAnim *pNodeAnim)
    {
        uint32_t i;
        
        for (i = 0; i < pNodeAnim->mNumRotationKeys - 1; ++i) {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
                return i;
            }
        }
        
        g_Core->p_Console->Print( LOG_DEVELOPER, "Mesh::FindRotation() - couldn't find animkey for %4.2f\n", AnimationTime);
        
        return 0;
    }
    
    /**
     *  Find anim scale for given node.
     */
    uint32_t CMesh::FindScaling(float AnimationTime, const aiNodeAnim *pNodeAnim)
    {
        uint32_t i;
        
        for (i = 0; i < pNodeAnim->mNumScalingKeys - 1; ++i) {
            if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
                return i;
            }
        }
        
        g_Core->p_Console->Print( LOG_DEVELOPER, "Mesh::FindScale() - couldn't find animkey for %4.2f\n", AnimationTime);
        
        return 0;
    }
    
    /**
     *  Lerp between two vectors.
     */
    aiVector3t<float> lerp(aiVector3t<float> _a, aiVector3t<float> _b, float _t)
    {
        aiVector3t<float> p;
        p = _a + (_b - _a) * _t;
        return p;
    }
    
    /**
     *  Calculate interpolatated positions.
     */
    void CMesh::CalculatePosition(aiVector3D &Out, float AnimationTime,
                                  const aiNodeAnim *pNodeAnim)
    {
        if (pNodeAnim->mNumPositionKeys == 1) {
            Out = pNodeAnim->mPositionKeys[0].mValue;
            return;
        }
        
        uint32_t PositionIndex;
        uint32_t NextPositionIndex;
        
        float fDeltaTime;
        float fFactor;
        
        PositionIndex = FindPosition(AnimationTime, pNodeAnim);
        NextPositionIndex = (PositionIndex + 1);

        //        if( NextPositionIndex < pNodeAnim->mNumPositionKeys ) {
        fDeltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime -
                             pNodeAnim->mPositionKeys[PositionIndex].mTime);
        fFactor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / fDeltaTime;
        
//            if(Factor >= 0.0f && Factor <= 1.0f) {
        
        const aiVector3D &Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D &End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        
        // aiVector3D Delta = End - Start;
        Out = lerp(Start, End, fFactor);
//            }
//        }
    }
    
    /**
     *  Calculate interpolated rotations.
     */
    void CMesh::CalculateRotation(aiQuaternion &Out, float AnimationTime,
                                  const aiNodeAnim *pNodeAnim)
    {
        // We need at least two values to interpolate...
        if( pNodeAnim->mNumRotationKeys == 1 ) {
            Out = pNodeAnim->mRotationKeys[0].mValue;
            return;
        }
        
        uint32_t RotationIndex;
        uint32_t NextRotationIndex;
        
        float fDeltaTime;
        float fFactor;
        
        // Find a rotation value for current time.
        RotationIndex = FindRotation(AnimationTime, pNodeAnim);
        NextRotationIndex = (RotationIndex + 1);
        
        fDeltaTime = pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime;
        fFactor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / fDeltaTime;
        
        // Quaternion rotation.
        const aiQuaternion &StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion &EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
        
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, fFactor);
        
        Out = Out.Normalize();
    }
    
    /**
     *  Calculate interpolated scalings.
     */
    void CMesh::CalculateScaling(aiVector3D &Out, float AnimationTime,
                                 const aiNodeAnim *pNodeAnim)
    {
        if (pNodeAnim->mNumScalingKeys == 1) {
            Out = pNodeAnim->mScalingKeys[0].mValue;
            return;
        }
        
        uint32_t ScalingIndex;
        uint32_t NextScalingIndex;
        
        float fDeltaTime;
        float fFactor;
        
        aiVector3D vDelta;
        
        // Find a scale value for current time.
        ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
        NextScalingIndex = (ScalingIndex + 1);
        
        // g_Core->p_Console->Print( LOG_DEVELOPER, "Mesh::CalculateInterpolatedScale
        // - nextScaleIndex > mNumScaleKeys!\n" );
        
        fDeltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime -
                             pNodeAnim->mScalingKeys[ScalingIndex].mTime);
        fFactor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / fDeltaTime;
        
        // Values to interpolate between.
        const aiVector3D &Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D &End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        
        vDelta = End - Start;
        Out = Start + fFactor * vDelta;
    }
    
    /**
     *  Read node heirarchy.
     */
    void CMesh::ReadNodeHeirarchy(float AnimationTime, const aiNode *pNode,
                                  const aiMatrix4x4t<float> &ParentTransform)
    {
        uint32_t i;
        int32_t * BoneIndex;
        
        const char *NodeName = pNode->mName.data;
        
        const aiAnimation *pAnimation = m_pScene->mAnimations[0];
        aiMatrix4x4t<float> NodeTransformation = pNode->mTransformation;
        
        const aiNodeAnim *pNodeAnim = FindNodeAnim(pAnimation, pNode->mName.data);
        
        // Current tag exists. Animation, do ya thing!
        if( pNodeAnim != NEKO_NULL ) {
            aiVector3D Scaling;
            CalculateScaling(Scaling, AnimationTime, pNodeAnim);
            aiMatrix4x4 ScalingM;
            
            ScalingM[0][0] = Scaling.x;
            ScalingM[0][1] = 0.0f;
            ScalingM[0][2] = 0.0f;
            ScalingM[0][3] = 0.0f;
            ScalingM[1][0] = 0.0f;
            ScalingM[1][1] = Scaling.y;
            ScalingM[1][2] = 0.0f;
            ScalingM[1][3] = 0.0f;
            ScalingM[2][0] = 0.0f;
            ScalingM[2][1] = 0.0f;
            ScalingM[2][2] = Scaling.z;
            ScalingM[2][3] = 0.0f;
            ScalingM[3][0] = 0.0f;
            ScalingM[3][1] = 0.0f;
            ScalingM[3][2] = 0.0f;
            ScalingM[3][3] = 1.0f;
            
            aiQuaternion RotationQ;
            CalculateRotation(RotationQ, AnimationTime, pNodeAnim);
            aiMatrix4x4 RotationM = aiMatrix4x4(RotationQ.GetMatrix());
            
            aiVector3D Translation;
            CalculatePosition(Translation, AnimationTime, pNodeAnim);
            
            NodeTransformation = RotationM * ScalingM;
            
            NodeTransformation[0][3] = Translation.x;
            NodeTransformation[1][3] = Translation.y;
            NodeTransformation[2][3] = Translation.z;
        }
        
        aiMatrix4x4t<float> GlobalTransformation;
        GlobalTransformation = ParentTransform * NodeTransformation;
        
        // Look for animation index.
        BoneIndex = m_animTags[NodeName];// GetAnimTagByName(NodeName);
        if (BoneIndex != NEKO_NULL) {
            m_boneInfo[*BoneIndex].FinalTransformation =
            GlobalTransformation * m_boneInfo[*BoneIndex].BoneOffset;
        }
        
        // Recurse to the node children.
        for (i = 0; i < pNode->mNumChildren; ++i) {
            ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
        }
    }
    
    /**
     *  Bone transformation.
     */
    void CMesh::BoneTransform(float fTimeInSeconds)
    {
        uint32_t i;
        
        float fTicksPerSecond;
        float fTimeInTicks;
        float fAnimationTime;
        
        fTimeInSeconds /= 1000.0f; // One second.
        
        aiMatrix4x4t<float> Identity;
        
        fTicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0
                                  ? m_pScene->mAnimations[0]->mTicksPerSecond
                                  : 25.0f);
        fTimeInTicks = fTimeInSeconds * fTicksPerSecond;
        fAnimationTime = fmod(fTimeInTicks, (float)m_pScene->mAnimations[0]->mDuration);
        
        // Get animation nodes.
        ReadNodeHeirarchy(fAnimationTime, m_pScene->mRootNode, Identity);
        
        ncMatrix4 Transforms[100]; // previous transformation matrices will be
        // rewritten or not even used
        
        // Outside of 'ReadNodeHeirarchy', because it uses recursion tree, don't put
        // it to here.
        for (i = 0; i < m_iBoneInfoCount; ++i) {
            Transforms[i] = aiMatrix4x4ToGlm(&m_boneInfo[i].FinalTransformation);
            
            glUniformMatrix4fv(m_boneLocation[i], 1, GL_TRUE, &Transforms[i].m[0]);
        }
        
        //        printf( "#bone transform node#\n" );
    }
    
    /**
     *  Find anim for node.
     */
    const aiNodeAnim *CMesh::FindNodeAnim(const aiAnimation *pAnimation,
                                          const char *NodeName) {
        uint32_t i;
        
        for (i = 0; i < pAnimation->mNumChannels; ++i) {
            const aiNodeAnim *pNodeAnim = pAnimation->mChannels[i];
            
            if (!strcmp(pNodeAnim->mNodeName.data, NodeName)) {
                return pNodeAnim;
            }
        }
        
        return NEKO_NULL;
    }
    
    // static void gg() {
    //
    //}
    
    // temp workaround
    CMesh *CMeshResource::Clone(const char *name)
    {
        CMesh *ent = Find(name);
        if (ent == NEKO_NULL) {
            return NEKO_NULL;
        }
        
        CMesh *clone = new CMesh(*ent);
        //        glDeleteVertexArrays( 1, &clone->m_VAO );
        
        // Create a new vertex array and bind existing data.
        uint32_t vao = g_mainRenderer->CreateVertexArray();
        glBindVertexArray(vao);
        
        // Vertice data.
        glBindBuffer(GL_ARRAY_BUFFER, ent->vertices.Handle);
        g_mainRenderer->BufferPointer(&ent->vertices, POSITION_LOCATION, 3, GL_FLOAT,
                                      GL_FALSE, 0, 0);
        g_mainRenderer->FinishBuffer(&ent->vertices, 0, ent->m_iVerticeNum);
        
        // UV data.
        glBindBuffer(GL_ARRAY_BUFFER, ent->uvs.Handle);
        g_mainRenderer->BufferPointer(&ent->uvs, UV_LOCATION, 2, GL_FLOAT, GL_FALSE,
                                      0, 0);
        g_mainRenderer->FinishBuffer(&ent->uvs, 0, ent->m_iVerticeNum);
        
        // Normal data.
        glBindBuffer(GL_ARRAY_BUFFER, ent->normals.Handle);
        g_mainRenderer->BufferPointer(&ent->normals, NORMAL_LOCATION, 3, GL_FLOAT,
                                      GL_FALSE, 0, 0);
        g_mainRenderer->FinishBuffer(&ent->normals, 0, ent->m_iVerticeNum);
        
        // So if it's animated add some bones!
        if (ent->m_bIsAnimated) {
            // Bone buffer.
            glBindBuffer(GL_ARRAY_BUFFER, ent->boneData.Handle);
            g_mainRenderer->BufferIPointer(&ent->boneData, BONE_ID_LOCATION, 4, GL_INT,
                                           sizeof(SVertexBoneData), (const GLvoid *)0);
            g_mainRenderer->BufferPointer(&ent->boneData, BONE_WEIGHT_LOCATION, 4,
                                          GL_FLOAT, GL_FALSE, sizeof(SVertexBoneData),
                                          (const GLvoid *)16);
            g_mainRenderer->FinishBuffer(&ent->boneData, 0, ent->m_iVerticeNum);
            
            // Set up bone locations. We're using animation shader to animate our
            // models.
            f_AssetBase->p_MeshBase->fxmodelAnim->Use();
            
            uint32_t i;
            
            for (i = 0; i < MAX_BONES; ++i) {
                char Name[128];
                memset(Name, 0, sizeof(Name));
                snprintf(Name, sizeof(Name), "gBones[%i]", i);
                
                ent->m_boneLocation[i] =
                f_AssetBase->p_MeshBase->fxmodelAnim->UniformLocation(Name);
            }
            
            f_AssetBase->p_MeshBase->fxmodelAnim->Next();
        }
        
        // Fill index buffer.
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ent->indexBuffer.Handle);
        g_mainRenderer->FinishBuffer(&ent->indexBuffer, 0, ent->m_iIndiceNum);
        
        glBindVertexArray(0);
        clone->m_VAO = vao;
        
        return clone;
    }
    
    /**
     *  Unload all meshes.
     */
    void CMeshResource::UnloadMeshes()
    {
        ncMesh * mesh = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_pMeshCache.m_List.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ){
            mesh = (ncMesh *)cur->m_ptrData;
            
            mesh->m_hOwner->Clear();
            
            pMeshAllocator->Dealloc( mesh->m_hOwner );
            pMeshEntAllocator->Dealloc( mesh );
            
            mesh = NEKO_NULL;
        }
        
    }
    
    /**
     *  Shutdown mesh manager.
     */
    void CMeshResource::Shutdown()
    {
        g_Core->p_Console->Print(LOG_INFO, "Mesh resource manager shutting down..\n");
        
        UnloadMeshes();
        
        if( m_pQuadMesh != NEKO_NULL ) {
            pAllocator->Dealloc(m_pQuadMesh);
        }
        
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pMeshEntAllocator, pAllocator );
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pMeshAllocator, pAllocator );
        
        m_pMeshCache.Delete();
        memset( &m_pMeshCache, 0x00, sizeof(m_pMeshCache) );
    }
    
    /**
     *  Initialize and load teh models.
     */
    void CMeshResource::Initialize(INekoAllocator *allocator)
    {
        pAllocator = allocator;
        g_Core->p_Console->Print( LOG_INFO,  "Initializing mesh resource manager....\n" );
        
        const uint64_t kMeshCacheSize = Megabyte( 32 );
        const uint64_t kMeshEntCacheSize = Megabyte( 8 );
        
        pMeshAllocator = NekoAllocator::newPoolAllocator( sizeof(CMesh), __alignof(CMesh), kMeshCacheSize, *pAllocator );
        pMeshEntAllocator = NekoAllocator::newPoolAllocator( sizeof(ncMesh), __alignof(ncMesh), kMeshEntCacheSize, *pAllocator );
        
        //     Load mesh shaders.
        
        // Create uniform buffer objects..
        if( Use_UniformBuffers->Get<bool>() ) {
            g_Core->p_Console->Print( LOG_INFO, "Creating uniform buffer objects..\n" );
            
            MeshTransform transform;
            
            transform.ModelView.Identity();
            transform.NormalMatrix.Identity();
            transform.ModelViewProjection.Identity();
            transform.PrevMatrix.Identity();
            
            glGenBuffers(1, &m_iMeshUniformBuffer);
            
            glBindBuffer(GL_UNIFORM_BUFFER, m_iMeshUniformBuffer);
            glBufferData(GL_UNIFORM_BUFFER, sizeof(transform), &transform,  GL_STREAM_DRAW);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_iMeshUniformBuffer);
            
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }
        
        g_Core->p_Console->Print(LOG_INFO, "Precaching mesh shaders..\n");
        
        ncMatrix4 Identity;
        Identity.Identity();
        
        fxmodel = f_AssetBase->FindAssetByName<SGLShader>("model");
        fxmodel->Use();
        
        fxmodel->SetUniform( "decal", 0 );        // diffuse
        fxmodel->SetUniform( "normalMap", 1 );    // normal
        fxmodel->SetUniform( "rgbMap", 2 );       // RMA
        
        // Uniforms.
        m_pShaderParams[MPVM] = fxmodel->UniformLocation("Transform.MVP");
        m_pShaderParams[MMODELMATRIX] = fxmodel->UniformLocation("Transform.ModelMatrix");
        m_pShaderParams[MNORMALMATRIX] = fxmodel->UniformLocation("Transform.NormalMatrix");
        m_pShaderParams[MTIME] = fxmodel->UniformLocation("Transform.time");
        m_pShaderParams[MPREVMATRIX] = fxmodel->UniformLocation("Transform.prevModelView");
        
        m_iMeshTransformUniform = fxmodel->UniformBlockIndex("Transform");
        
        fxmodel->SetUniform(m_pShaderParams[MMODELMATRIX], 1, GL_FALSE, Identity.m);
        fxmodel->SetUniform(m_pShaderParams[MNORMALMATRIX], 1, GL_FALSE, Identity.m);
        fxmodel->Next();
        
        //**     Load animated mesh shader.      **
        
        fxmodelAnim = f_AssetBase->FindAssetByName<SGLShader>("animmodel");
        fxmodelAnim->Use();
        
        fxmodelAnim->SetUniform("decal", 0);
        fxmodelAnim->SetUniform("normalMap", 1);
        fxmodelAnim->SetUniform("rgbMap", 2);
        
        m_pShaderAnimParams[MPVM] = fxmodelAnim->UniformLocation("Transform.MVP");
        m_pShaderAnimParams[MMODELMATRIX] = fxmodelAnim->UniformLocation("Transform.ModelMatrix");
        m_pShaderAnimParams[MNORMALMATRIX] = fxmodelAnim->UniformLocation("Transform.NormalMatrix");
        m_pShaderAnimParams[MTIME] = fxmodelAnim->UniformLocation("Transform.time");
        m_pShaderAnimParams[MPREVMATRIX] = fxmodelAnim->UniformLocation("Transform.prevModelView");
        
        m_iMeshTransformUniform = fxmodelAnim->UniformBlockIndex("Transform");
        
        fxmodelAnim->SetUniform(m_pShaderAnimParams[MMODELMATRIX], 1, GL_FALSE, Identity.m);
        fxmodelAnim->SetUniform(m_pShaderAnimParams[MNORMALMATRIX], 1, GL_FALSE, Identity.m);
        fxmodelAnim->Next();

        
        
        
        // Create mesh cache.
        m_pMeshCache.Create( MAX_MODELS, pLinearAllocator2, pAllocator );
        
        // No spammish console..
        g_mainRenderer->AllowBufferLogging(false);
        
        // Load models now
        
        Load("geosphere.obj");
        Load("Bridge.obj");
//        Load("coupe.obj");
//        Load("Pilot.dae");
//			Load("tower.obj");
        
        // Spam now.
        g_mainRenderer->AllowBufferLogging(true);
        g_Core->p_Console->Print( LOG_INFO, "%ul models loaded.\n", m_pMeshCache.GetCount() );
        
        // c_CommandManager->Add( "sm", gg );
        
        Vec3 *vertices = NEKO_NULL;
        Vec2 *uvs = NEKO_NULL;
        uint32_t *indexes = NEKO_NULL;
        
        // Create a quad vertex data.
        vertices = new Vec3[4];
        
        vertices[0] = Vec3(-1.0f, -1.0f, 0.0f); // Bottom left corner.
        vertices[1] = Vec3(-1.0f, 1.0f, 0.0f);  // Top left corner.
        vertices[2] = Vec3(1.0f, 1.0f, 0.0f);   // Top right corner.
        vertices[3] = Vec3(1.0f, -1.0f, 0.0f);  // Bottom right corner.
        
        // Mapping coordinates for the vertices.
        uvs = new Vec2[4];
        
        uvs[0] = Vec2(0.0f, 1.0f);
        uvs[1] = Vec2(0.0f, 0.0f);
        uvs[2] = Vec2(1.0f, 1.0f);
        uvs[3] = Vec2(1.0f, 0.0f);
        
        // Index data.
        indexes = new uint32_t[6];
        
        // First triangle (bottom left - top left - top right).
        indexes[0] = 0;
        indexes[1] = 1;
        indexes[2] = 2;
        
        // Second triangle (bottom left - top right - bottom right).
        indexes[3] = 0;
        indexes[4] = 3;
        indexes[5] = 2;
        
        m_pQuadMesh = (CBasicMesh *)pAllocator->Alloc(sizeof(CBasicMesh));
        m_pQuadMesh->Create(vertices, NEKO_NULL, uvs, indexes, 4, 6,
                            EPrimitiveType::TriangleStrip);
        
        delete[] vertices;
        delete[] uvs;
        delete[] indexes;
    }
    
    /**
     *  Find model by the name.
     */
    CMesh *CMeshResource::Find(const char *name)
    {
        ncMesh * mesh = m_pMeshCache[name];
        if( mesh == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "CMeshFactory: \"%s\" not found\n", name );
            return NEKO_NULL;
        }
        
        return mesh->GetHandle();
    }
    
    /**
     *  Load the model file.
     */
    ncMesh * CMeshResource::Load( const char * filename, bool fromPak, bool threaded, void * ptr, void * ptr2 )
    {
        CMesh * tmp = NEKO_NULL;
        
        if( threaded && ptr != NEKO_NULL ) {
            tmp = (CMesh *)ptr2;
        } else {
            tmp = (CMesh *)pMeshAllocator->Alloc(sizeof(CMesh));
        }
        
        tmp->Prepare();
        tmp->LoadMesh(filename, pAllocator, fromPak, threaded);
        
        ncMesh * meshHandle = NEKO_NULL;
        if( threaded && ptr != NEKO_NULL ) {
            meshHandle = (ncMesh *)ptr;
        } else {
            meshHandle = (ncMesh *)pMeshEntAllocator->Alloc(sizeof(ncMesh));
            
            meshHandle->SetOwner( tmp );
        }
    
        if( !threaded ) {
            m_pMeshCache[filename] = meshHandle;
        }
        
        return meshHandle;
    }
    
    /**
     *
     *
     *  Geometric figures.
     *
     *
     */
    
    // Sphere.
    void ncGeometrySphere::Create(float _radius, int _segmentsW, int _segmentsH)
    {
        SMemoryTempFrame *tempMemory;
        const int32_t numVertices = (_segmentsH + 1) * (_segmentsW + 1);
        
        tempMemory = _PushMemoryFrame(pLinearAllocator2);
        
        Vec3 *mVertices = (Vec3 *)PushMemory(tempMemory, sizeof(Vec3) * numVertices);
        Vec3 *mNormals = (Vec3 *)PushMemory(tempMemory, sizeof(Vec3) * numVertices);
        
        const int32_t numIndices = (_segmentsH - 1) * _segmentsW * 6;
        uint32_t *mFaceIndexArray = (uint32_t *)PushMemory(
                                                           tempMemory, sizeof(uint32_t) * numIndices); // new uint32_t[numIndices ];
        
        int triIndex = 0;
        int vertcount = 0;
        
        for (int j = 0; j <= _segmentsH; ++j) {
            float horangle = nkMath::PI * j / _segmentsH;
            float z = -_radius * cos(horangle);
            float ringradius = _radius * sin(horangle);
            
            for (int i = 0; i <= _segmentsW; ++i) {
                float verangle = 2 * nkMath::PI * i / _segmentsW;
                float x = ringradius * cos(verangle);
                float y = ringradius * sin(verangle);
                float normLen = 1 / nkMath::FastSqrty(x * x + y * y + z * z);
                
                mVertices[vertcount].x = x;
                mVertices[vertcount].y = y;
                mVertices[vertcount].z = z;
                
                mNormals[vertcount].x = x * normLen;
                mNormals[vertcount].y = y * normLen;
                mNormals[vertcount].z = z * normLen;
                
                vertcount++;
                
                if (i > 0 && j > 0) {
                    int a = (_segmentsW + 1) * j + i;
                    int b = (_segmentsW + 1) * j + i - 1;
                    int c = (_segmentsW + 1) * (j - 1) + i - 1;
                    int d = (_segmentsW + 1) * (j - 1) + i;
                    
                    if (j == _segmentsH) {
                        mFaceIndexArray[triIndex++] = a;
                        mFaceIndexArray[triIndex++] = c;
                        mFaceIndexArray[triIndex++] = d;
                    } else if (j == 1) {
                        mFaceIndexArray[triIndex++] = a;
                        mFaceIndexArray[triIndex++] = b;
                        mFaceIndexArray[triIndex++] = c;
                    } else {
                        mFaceIndexArray[triIndex++] = a;
                        mFaceIndexArray[triIndex++] = b;
                        mFaceIndexArray[triIndex++] = c;
                        mFaceIndexArray[triIndex++] = a;
                        mFaceIndexArray[triIndex++] = c;
                        mFaceIndexArray[triIndex++] = d;
                    }
                }
            }
        }
        
        m_iFaceCount = numIndices;
        
        // Generate buffers.
        m_VAO = g_mainRenderer->CreateVertexArray();
        
        // Vertex buffer.
        m_vVertex = g_mainRenderer->AllocGPUBuffer(numVertices * sizeof(Vec3),
                                                   EBufferStorageType::Array,
                                                   EBufferType::Static);
        g_mainRenderer->BufferData(&m_vVertex, &mVertices[0], 0,
                                   numVertices * sizeof(Vec3));
        g_mainRenderer->BufferPointer(&m_vVertex, 0, 3, GL_FLOAT, GL_FALSE,
                                      sizeof(Vec3), 0);
        g_mainRenderer->FinishBuffer(&m_vVertex, 0, numVertices);
        
        // Normal buffer.
        m_vNormals = g_mainRenderer->AllocGPUBuffer(numVertices * sizeof(Vec3),
                                                    EBufferStorageType::Array,
                                                    EBufferType::Static);
        g_mainRenderer->BufferData(&m_vNormals, &mNormals[0], 0,
                                   numVertices * sizeof(Vec3));
        g_mainRenderer->BufferPointer(&m_vNormals, 1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        g_mainRenderer->FinishBuffer(&m_vNormals, 0, numVertices);
        
        // Index buffer.
        m_vIndexArray = g_mainRenderer->AllocGPUBuffer(
                                                       numIndices * sizeof(unsigned int), EBufferStorageType::IndexArray,
                                                       EBufferType::Static);
        g_mainRenderer->BufferData(&m_vIndexArray, &mFaceIndexArray[0], 0,
                                   numIndices * sizeof(unsigned int));
        g_mainRenderer->FinishBuffer(&m_vIndexArray, 0, numIndices);
        
        glBindVertexArray(0);
        
        Neko::_PopMemoryFrame(tempMemory);
        
        mFaceIndexArray = NEKO_NULL;
        mVertices = NEKO_NULL;
        mNormals = NEKO_NULL;
    }
    
    /**
     *  Add bone data.
     */
    void SVertexBoneData::AddBoneData(unsigned int BoneID, float Weight)
    {
        //        printf( "AddBoneData: %i %f\n", BoneID, Weight );
        
        uint32_t i;
        
        for (i = 0; i < (sizeof(m_IDs) / sizeof(m_IDs[0])); ++i) {
            if (m_fWeights[i] == 0.0f) {
                m_IDs[i] = BoneID;
                m_fWeights[i] = Weight;
                
                return;
            }
        }
    }
}
#endif
