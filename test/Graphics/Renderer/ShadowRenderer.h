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
//  ShadowRenderer.h
//  Shadow renderer.
//
//  Created by Neko Code on 12/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__ShadowRenderer__
#define __Nanocat__ShadowRenderer__

#include "../../Platform/Shared/SystemShared.h"
#include "../../Math/GameMath.h"
#include "../OpenGL/GLShader.h"
#include "Renderer.h"

// /*Un*/finished!.

#ifndef NEKO_SERVER
namespace Neko {

    //! Maximum shadow splits.
    static const int32_t MAX_SPLITS = 4;
    
    /// Shadow map types.
    enum class EShadowType : int32_t
    {
        None,
        
        Cascaded,
        Single,         // fades on distance
    };

    ///  Cascaded Shadow Map renderer.
    class CCascadedShadow
    {
        NEKO_NONCOPYABLE( CCascadedShadow );
        
    public:

        /**
         *  Constructor.
         */
        CCascadedShadow();
        
        /**
         *  Destructor.
         */
        ~CCascadedShadow();
        
        /**
         *  Create framebuffer and projection matrices.
         */
        void                Init( INekoAllocator * allocator );
        
        /**
         *  Destroy the shadow renderer.
         */
        void                Destroy();
        
        /**
         *  Render shadow to framebuffer.
         */
        void                RenderShadow();

        /**
         *  Update rendering postprocess fx.
         */
        void                UpdatePostProcess( SGLShader * shader );
        
        /**
         *  Shadow global transparency ( opacity ).
         */
        inline  const float                 GetOpacity()    const   {   return m_fGlobalOpacity;    }
        /**
         *  Set global shadow opacity.
         */
        inline  void                SetOpacity( const float opacity )    { m_fGlobalOpacity = opacity;   }
        
        /**
         *  Update frustum splits.
         */
        void                updateFrustumSegmentFarPlanes();
        
        /**
         *  Update frustum bounding box and matrices.
         */
        void                frustumBoundingBoxLightViewSpace( const int32_t currentSplit, float nearPlane, float farPlane, Vec4 & min, Vec4 & max)
        ;
        
//    private:
        
        INekoAllocator  * pAllocator;
        
        //! CSM shader uniforms.
        enum CSMUniforms : int32_t {
            Matrix0 = 0, Matrix1, Matrix2, Matrix3,
            NormalMat0, NormalMat1, NormalMat2, NormalMat3,
            FarDistanceVec, LightRadiusUV, Max
        };
        
        float   m_farPlanes[4];
        float           m_normalizedFarPlanes[4]; // Normalized to [0, 1] depth range.
        
        float m_viewportDims[2];
        
        ncMatrix4   m_lightViewMatrix, m_lightViewMatrix2;
        ncMatrix4   m_lightProjMatrix;
        
        Vec4           m_lightViewports[MAX_SPLITS];
        ncMatrix4       m_lightSegmentVPSBMatrices[MAX_SPLITS];
        
        //! Shader uniforms.
        uint32_t            m_iCSMUniforms[CSMUniforms::Max];
        //! Shadow splits.
        int32_t             m_iShadowSplits;
        //! Shadow sizes.
        int32_t            m_iShadowSizeX;
        int32_t            m_iShadowSizeY;
        
        Vec2       m_vFrustumSize;
        
        //! Shadow map near distance.
        float               m_nearPlane = 0.1f;
        //! Shadow map far distance.
        float               m_farPlane= 3000.0f;
        float               m_frustumSplitCorrection = 0.8f;    // size
        
        SBoundingBox        m_bBoundingBox[MAX_SPLITS];
        
        CFrustum            m_frustumSplits[MAX_SPLITS];
        
        //! Shadow opacity.
        float               m_fGlobalOpacity;
        //! Light radius.
        float           m_fLightRadiusWorld;
        
        //! Shadow map filter.
        ETextureFormat      m_eShadowFilter = ETextureFormat::Depth24;
        
        //! Shadow map array.
        SMaterial           *m_pShadowMap;
        //! Framebuffer.
        CFramebuffer        * m_pDepthBuffer;
        
        //! Shadow light view projection matrices.
        ncMatrix4               m_mShadowViewMatrices[MAX_SPLITS];
        //! Shadow light view matrices.
        ncMatrix4               m_mLightViewMatrices[MAX_SPLITS];
    };
    
    
    extern SConsoleVar      * Shadow_Enabled;
}
#endif // NEKO_SERVER

#endif /* defined(__Nanocat__ShadowRenderer__) */
