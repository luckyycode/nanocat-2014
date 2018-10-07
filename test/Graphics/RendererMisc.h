//
//  RendererMisc.h
//  Nanocat
//
//  Created by Kawaii Neko on 10/12/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef RendererMisc_h
#define RendererMisc_h

#include "../Platform/Shared/SystemShared.h"

namespace Neko {
    
    ///  Blend states.
    enum class EBlendMode : int32_t
    {
        Disabled,
        Current,
        Default,
        AlphaPremultiplied,
        Additive,
        AlphaAdditive,
        AlphaMultiplicative,
        ColorAdditive,
        AlphaInverseMultiplicative,
        
        Dummy
    };
    
    ///  Culled face states.
    enum class ECullMode : int32_t
    {
        Front,
        Back,
        
        Dummy,
    };
    
    ///  Color types.
    enum class ETextureFormat : uint32_t
    {
        Invalid,
        
        R,
        R8,
        R16,
        R16F,
        R32F,
        
        RG,
        RG8,
        RG16,
        RG16F,
        RG32F,
        
        RGB,
        RGB8,
        RGB16,
        RGB16F,
        RGB32F,
        
        BGR,
        
        RGBA,
        RGBA8,
        RGBA16,
        RGBA16F,
        RGBA32F,
        
        BGRA,
        
        DXT1_RGB,
        DXT1_RGBA,
        DXT3,
        DXT5,
        
        RGTC2,
        
        Depth,
        Depth16,
        Depth24,
        Depth32,
        Depth32F,
        
        PVR_2bpp_RGB,
        PVR_2bpp_sRGB,
        PVR_2bpp_RGBA,
        PVR_2bpp_sRGBA,
        PVR_4bpp_RGB,
        PVR_4bpp_sRGB,
        PVR_4bpp_RGBA,
        PVR_4bpp_sRGBA,
        
        R11G11B10F,
        
        Dummy
    };
    
    
    /// Texture storage types.
    enum class ETextureStorageType : int32_t
    {
        Char,
        Uchar,
        
        Short,
        Ushort,
        
        Int,
        Uint,
        
        Float,
        Double,
        
        Dummy
    };
    
    /// Texture wrap modes.
    enum class ETextureTile : int32_t
    {
        Repeat = 0, // Keep '0' here.
        MirrorRepeat,
        Clamp,
        ClampToEdge,
        
        Dummy
    };
    
    /// Texture filters.
    enum class ETextureFilter : int32_t
    {
        Linear,
        Nearest,
        
        Dummy
    };
    
    /// Basic data types.
    enum class EDataType : int32_t
    {
        Char8,
        UnsignedChar,
        Int16,
        UnsignedShort,
        Int32,
        UnsignedInt,
        Half,
        Float,
        Double,
        
        UnsignedShort_4444,
        UnsignedShort_5551,
        UnsignedShort_565,
        
        UnsignedInt_8888_Rev,
        
        Dummy,
    };
    
    /// Primitive types.
    enum class EPrimitiveType : int32_t
    {
        Points,
        Triangles,
        TriangleStrip,
        Lines,
        LineStrip,
        
        Dummy,
    };
    
    /// GPU Index buffer data type.
    enum class EIndexArrayType : int32_t
    {
        Index_8 = 1,
        Index_16 = 2,
        Index_32 = 4,
        
        Dummy,
    };
    
    /**
     *  GPU buffer data.
     */
    struct GPUBuffer
    {
        unsigned int Handle;
        unsigned int VertexMode;
        unsigned int Complexity;
        unsigned int Offset;
        unsigned int Count;
        unsigned int Size;
        unsigned int Type;
        
        bool Done;
    };
    
    /// GPU buffer types.
    enum class EBufferType : int32_t
    {
        Static,
        Dynamic,
        Stream,
        
        Dummy,
    };
    
    /// GPU buffer storage type.
    enum class EBufferStorageType : int32_t
    {
        Array,
        IndexArray,
        Uniform,
        
        Dummy,
    };
    
    /// Texture target.
    enum class TextureTarget : uint32_t
    {
        Texture2D,
        Texture3D,
        Texture2DArray,
        TextureRectangle,
        TextureCube,
        
        Dummy,
        None, // ??
    };
    
    /// Compare modes.
    enum class ECompareMode : int32_t
    {
        None,
        Equal,
        Less,
        Lequal,
        Gequal,
        Greater,
        Always,
        
        Dummy,
    };
    
    /// Render targets ( color attachments ).
    enum class ERenderTarget : int32_t
    {
        None,
        
        RT0,
        RT1,
        RT2,
        RT3,
        
#   if defined(GL_COLOR_ATTACHMENT4)
        RT4,
#   else
        0,
#   endif
        
#   if defined(GL_COLOR_ATTACHMENT5)
        RT5,
#   else
        0,
#   endif
        
#   if defined(GL_COLOR_ATTACHMENT6)
        RT6,
#   else
        0,
#   endif
        
#   if defined(GL_COLOR_ATTACHMENT7)
        RT7,
#   else
        0,
#   endif
        
        Dummy,
    };
    
    /// Shader types.
    enum class EShaderType : int32_t
    {
        Vertex,
        Pixel, // fragment
        Geometry,
        TessellationEval,
        TessellationControl,
        Compute,
        
        Dummy
    };
    
    enum class EDownsampleMode
    {
        Unknown = -1,
        Full,
        Half,
        Quarter,
        
        Dummy
    };
    
    //! Phase function to use for this media term
    enum class PhaseFunctionType
    {
        UNKNOWN = -1,
        ISOTROPIC,	        //!< Isotropic scattering (equivalent to HG with 0 eccentricity, but more efficient)
        RAYLEIGH,	        //!< Rayleigh scattering term (air/small molecules)
        HENYEYGREENSTEIN,	//!< Scattering term with variable anisotropy
        MIE_HAZY,	        //!< Slightly forward-scattering
        MIE_MURKY,	        //!< Densely forward-scattering
        COUNT
    };

    //! Amount of tessellation to use
    enum class ETessellationQuality
    {
        Unknown = -1,
        Low,        //!< Low amount of tessellation (16x)
        Medium,     //!< Medium amount of tessellation (32x)
        High,       //!< High amount of tessellation (64x)
        
        Dummy
    };

    
    //! Quality of upsampling
    enum class UpsampleQuality
    {
        Unknown = -1,
        Point,		//!< Point sampling (no filter)
        Bilinear,	//!< Bilinear Filtering
        Bilateral,	//!< Bilateral Filtering (using depth)
        
        Dummy
    };

    typedef GPUBuffer IndexBuffer;
    typedef GPUBuffer VertexBuffer;
    
#ifdef USES_OPENGL
    typedef GLuint VertexArray;
#else
    typedef GLuint VertexArray;
#endif

}

#endif /* RendererMisc_h */
