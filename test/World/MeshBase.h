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
//  MeshObject.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 10/15/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef MeshObject_hpp
#define MeshObject_hpp

#include "../Core/Physics/PlayerPhysics.h"
#include "../Graphics/RendererMisc.h"
#include "../Math/BoundingBox.h"
#include "../AssetCommon/AssetPtr.h"
#include "Mesh.h"

namespace Neko {
    
    //!  Maximum allocated models.
    static const int32_t MAX_MODELS = 128;

    /// Mesh entity.
    class ncMesh :  public TAssetPtr, public CPhysicsObject
    {
    public:
        
        /**
         *  Constructor.
         */
        ncMesh() : m_bActive(false), m_bInstanced(false)
        {
       
        }
        
        /**
         *  Destructor.
         */
        virtual ~ncMesh()
        {
            
        }

        /**
         *  Refresh model origin, rotation, scale, etc..
         */
        void                Refresh();
        
        /**
         *  Render a mesh.
         *
         *  @param isShadow Is it a shadow pass?
         */
        void                Render( bool isShadow = false, const int32_t instanceCount = 0 );
        
        /**
         *  Update a mesh.
         */
        void                Update();
        
        /**
         *  Get mesh owner.
         */
        NEKO_FORCE_INLINE CMesh *              GetHandle()   {   return m_hOwner;    }
        
        /**
         *  Set mesh owner.
         */
        void                SetOwner( CMesh * owner );
        
//    private:
        
        ncMatrix4   m_modelView;
        ncMatrix4   m_normalView;   // inversed and transposed model matrix
        ncMatrix4   m_projectionMatrix;
        ncMatrix4   m_localToWorldPrev;
        ncMatrix4   m_prevView;
        
        
        //!  Mesh handle.
        CMesh    * m_hOwner;
        
        //!  Is active?
        bool    m_bActive;
        
        //! Instanced draw?
        bool    m_bInstanced;
    };

    
    ///  Basic mesh.
    struct CBasicMesh : public ncMesh
    {
    public:
        
        
        /**
         *  Constructor.
         */
        CBasicMesh() : m_VAO(0), m_iFaceCount(0)
        {
            memset( &m_vVertex, 0x00, sizeof(GPUBuffer) );
            memset( &m_vNormals, 0x00, sizeof(GPUBuffer) );
            memset( &m_vUVs, 0x00, sizeof(GPUBuffer) );
            memset( &m_vIndexArray, 0x00, sizeof(GPUBuffer) );
        }
        
        ~CBasicMesh()
        {
            
        }
        
        uint32_t m_iPrimitiveType;
        
        GLuint m_VAO;
        
        GPUBuffer m_vVertex;
        GPUBuffer m_vNormals;
        GPUBuffer m_vIndexArray;
        GPUBuffer m_vUVs;
        
        uint32_t m_iFaceCount;
        uint32_t m_iVertexCount;
        
        /**
         *  Create a new mesh.
         */
        void                Create( const Vec3 * positions,
                                const Vec3 * normals,
                                const Vec2 * uvs,
                                const uint32_t * indices, const int32_t vertexCount, const int32_t indexCount, EPrimitiveType type = EPrimitiveType::Triangles );
        
        /**
         *  Destroy a mesh.
         */
        void                Destroy();
        
        /**
         *  Render a mesh.
         */
        void                DrawIndexed();
        
        /**
         *  Render a mesh.
         */
        void                DrawArrays();
        
        /**
         *  Bind index vertex array seperately.
         */
        NEKO_FORCE_INLINE void BindIndexData()
        {
            glBindVertexArray( m_VAO );
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vIndexArray.Handle );
        }
        
        
        /**
         *  Bind vertex array seperately.
         */
        NEKO_FORCE_INLINE void BindVertexData()
        {
            glBindVertexArray( m_VAO );
            glBindBuffer( GL_ARRAY_BUFFER, m_vVertex.Handle );
        }
        
        /**
         *  Bind vertex array.
         */
        NEKO_FORCE_INLINE void BindVertexArray()
        {
            glBindVertexArray( m_VAO );
        }
    };

    /// Model asset loader.
    class CMeshResource
    {
        NEKO_NONCOPYABLE( CMeshResource );
        
    public:
        
#define MPVM                0
#define MMODELMATRIX        1
#define MNORMALMATRIX       2
#define MTIME               3
#define MPREVMATRIX         4
        
        /**
         *  Constructor.
         */
        CMeshResource()
        {
            
        }
        
        /**
         *  Destructor.
         */
        virtual ~CMeshResource()
        {
            
        }
        
        /**
         *  Initialize a model manager.
         */
        void                Initialize( INekoAllocator  * allocator );
        
        /**
         *  Shutdown mesh manager.
         */
        void                Shutdown();
        
        /**
         *  Unload all meshes.
         */
        void                UnloadMeshes();
        
        /**
         *  Load the mesh.
         *
         *  @param filename Mesh filename.
         *  @param fromPak  Loading from a folder or the package file?
         */
        ncMesh *                Load( const char * filename, bool fromPak = false, bool threaded = false, void * ptr = NEKO_NULL, void * ptr2 = NEKO_NULL  );
        
        /**
         *  Find a mesh by name.
         *
         *  @param name Mesh filename.
         *
         *  @return Whatever you wanted.
         */
        CMesh   *               Find( const char * name );
        
        CMesh   *               Clone( const char * name );
        
        INekoAllocator      *   pAllocator;
        INekoAllocator      *   pMeshAllocator;
        INekoAllocator      *   pMeshEntAllocator;
        
        CHashMap<const char*, ncMesh*>      m_pMeshCache;
        
        
        /**
         *  Model shaders.
         */
        SGLShader            * fxmodel, * fxmodelAnim;
        
        //! Uniform buffer id.
        uint32_t        m_iMeshUniformBuffer;
        uint32_t        m_iMeshTransformUniform;    //! Transform uniform.
    
        GLuint          m_pShaderParams[6];
        GLuint          m_pShaderAnimParams[6];
        
        //! Simple quad mesh.
        CBasicMesh  *       m_pQuadMesh;
    };
    
    ///   Geometry sphere.
    class ncGeometrySphere : public CBasicMesh
    {
    public:
        ncGeometrySphere()
        {
            
        };
        
        ~ncGeometrySphere()
        
        {
            
        };
        
        /**
         *  Create a new geometric sphere.
         *
         *  @param color      Vertex color.
         */
        void                Create( float _radius = 1.0f, int _segmentsW = 16, int _segmentsH = 12 );
        
        Vec3 _position;
        ncMatrix4 objectMatrix;
    };
}

#endif /* MeshObject_hpp */
