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
//  MaterialLoader.h
//  Material manager..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef material_h
#define material_h

#include "../../Core/String/String.h"
#include "../../Core/Utilities/Hashtable.h"
#include "../../Graphics/GraphicsManager.h"
#include "../AssetPtr.h"
#include "ImageManager.h"


#ifndef NEKO_SERVER

namespace Neko {

    //!  Checker texture size. ( Used when no texture found.. whatever ).
    static const int32_t NULL_TEXTURE_SIZE  = 64;      //! Checker texture ( no texture ) size.
    static const int32_t NOISE_TEXTURE_SIZE = 512;      //! Noise texture.
    //!  3D noise texture.
    static const int32_t NOISE3D_TEXTURE_SIZE = 64;     //! 3D Noise texture size.
    //!  Material files to be loaded per cache.
    static const int32_t MAX_MATERIALS = 64;
    //!  Maximum texture layers in material array.
    static const int32_t    MAX_ARRAY_TEXTURES = 8;
    
    /// Material flags.
    enum class EMaterialFlag : int32_t
    {
        Array       = 1 << 0,
        Available   = 1 << 1    // for async load
    };
    
    /// Material type flags.
    enum class EMaterialTypeFlag : int32_t
    {
        Diffuse     = 1 << 0,
        Normal      = 1 << 1,
        RMA         = 1 << 2    // Roughness, Metallic, Ambient occlusion
    };
    
    // Noise.
#define KERNEL_SIZE     32
#define NOISE_SIZE      16

    ///      Material.
    struct SMaterial
    {
    public:
        
        /**
         *  Constructor.
         */
        SMaterial() : m_iAPIIndex(0)
        {
            memset( &Image, 0x00, sizeof(SImageInfo) );
        }
        
        ~SMaterial()
        {
            
        }
     
        //!  Image handler.
        SImageInfo          Image;
        //!  API image index.
        int32_t         m_iAPIIndex;
        
        /**
         *  Get material Id.
         */
        inline uint32_t                 GetId() const   {       return Image.GetId();   }

        /**
         *  Set tile mode.
         */
        void                SetTileMode( ETextureTile tile );
        
        /**
         *  Set comparesment mode for.
         */
        void                CompareRefToTexture( bool enable, ECompareMode mode );
        
        /**
         *  Set texture anisotropy level.
         */
        void                SetAnisotropy( const int32_t value );
        
        /**
         *  Generate mipmaps for this material.
         */
        void                MakeMipmaps();
    };
    
    ///     Material property.
    struct SMaterialProp : public TAssetPtr
    {
    public:
        SMaterialProp() : m_pDiffuse(NEKO_NULL), m_pNormal(NEKO_NULL), m_pSpecular(NEKO_NULL), m_iFlags(0)
        {
            
        }
        
        //! Diffuse map.
        SMaterial   * m_pDiffuse;
        //! Normal(bump) map.
        SMaterial   * m_pNormal;
        //! Specular map.
        SMaterial   * m_pSpecular;
        
        //!  Material name.
        CStr           m_Name;
        
        //! Material flags.
        int32_t     m_iFlags;
        //! Temp memory.
        SMemoryTempFrame  * m_pTempMemory;
        
        /**
         *  Unload all materials.
         */
        void            Unload();
    };
    
    /// Material array.
    struct SMaterialPropArray
    {
    public:
      
        SMaterialPropArray() : m_iLayers(0), m_bUnloadOriginals(true), m_bMipmapped(false)
        {
            m_iFlags = (int32_t)EMaterialTypeFlag::Diffuse;
        }
        
        //! Materials for texture array.
        const char   * props[MAX_ARRAY_TEXTURES];
        //! Total layers.
        int32_t     m_iLayers;
        //! Unload used materials?
        bool            m_bUnloadOriginals;
        //! Mipmapped materials?
        bool            m_bMipmapped;
        //! Texture formats.
        ETextureFormat  internalFormat, externalFormat;
        //! Common texture size.
        int32_t     m_iWidth, m_iHeight;
        //! Preferences..
        int32_t     m_iFlags;
    };
    
    ///  Material loader.
    class CMaterialFactory
    {
        NEKO_NONCOPYABLE( CMaterialFactory );
        
    public:
    
        /**
         *  Constructor.
         */
        CMaterialFactory();
        
        /**
         *  Destructor.
         */
        ~CMaterialFactory();
        
        /**
         *  Initialize material manager.
         */
        void                Initialize( INekoAllocator * allocator );

        /**
         *  Unload all materials.
         */
        void                UnloadAllMaterials();
        
        /**
         *  Shutdown material manager.
         */
        void                Shutdown();
        
        /**
         *  Unloads material.
         *
         *  @param material <#material description#>
         */
        void            UnloadMaterial( SMaterial * material );
        
        /**
         *  Load image.
         */
        SMaterial               * LoadImage( const char * material_name, const int16_t tileFlag, const int16_t miscFlags, bool threaded = false, SMemoryTempFrame * memoryPool = NEKO_NULL );
        
        /**
         *  Find material by name.
         *  No const because we can change just found material.
         */
        SMaterialProp               * Find( const char * entry );
        
        /**
         *  Load material file.
         *
         *  @note   If you don't free memory then you MUST do it later after your decisions.
         */
        SMaterialProp               * LoadFromFile( const char * name, const bool freesMemory = true, const bool threaded = false, void * ptr = NEKO_NULL );
        
        /**
         *  Create texture array.
         */
        SMaterialProp *                    CreateTextureArray( const char * name, SMaterialPropArray  * info, SMemoryTempFrame  * textureMemory );
        
        /**
         *  Create a new material.
         *
         *  @param material  Material to create.
         *  @param imageData Image data.
         */
        SMaterial               *    CreateMaterial( const uint8_t * imageData, const uint32_t width, const uint32_t height, TextureTarget textureType, ETextureStorageType textureDataMode = ETextureStorageType::Uchar, ETextureFormat internalFormat = ETextureFormat::RGBA, ETextureFormat externalFormat = ETextureFormat::RGBA, ETextureTile tiling = ETextureTile::Repeat, ETextureFilter filter = ETextureFilter::Linear, const uint32_t bitsPerPixel = 32, bool hasMipmap = false, const int32_t maxLayers = 0, const float LodBias = 0.0f );
       
    public:
        
        INekoAllocator  * pMaterialAllocator = 0;
        INekoAllocator  * pAllocator = 0;
        INekoAllocator  * pAllocatorHandle = 0;
        
        //!   Null ( checker ).     Autogenerated.
        SMaterialProp *nullMaterial;
        //!  Noise material.
        SMaterialProp *noiseMaterial, *noise3dMaterial;
        //!  Noise kernel data.
        Vec3 noiseKernel[KERNEL_SIZE];
        
//    private:
        //!  Material cache.
        CHashMap<const char*, SMaterialProp*>     m_materialCache;

    };

}

#endif // NEKO_SERVER

#endif
