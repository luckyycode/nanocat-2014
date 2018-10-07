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
//  LTerrainObject.h
//  Chunk-system terrain objects.
//
//  Created by Neko Code on 1/4/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __lterrain_object__
#define __lterrain_object__

#include "../Platform/Shared/SystemShared.h"
#include "../Math/GameMath.h"
#include "../Math/Vec4.h"
#include "Mesh.h"
#include "../Graphics/RendererMisc.h"
#include "../Core/Utilities/List.h"
#include "../AssetCommon/Material/MaterialLoader.h"

namespace Neko {
    
    class ncMesh;

    class ncActiveModel;
    class ncStaticModel;
    class nc2dCube; // Billboarding.
    
    class INekoAllocator;

    ///   Terrain chunk object.
    class ncLTerrainObject
    {
    public:

        
        /**
         *  Create new terrain object.
         */
        ncLTerrainObject( const char * meshName, const Vec4 & meshWorld, INekoAllocator * allocator, const float & scale = 1.0f );
        ncLTerrainObject();
        ~ncLTerrainObject();

        /**
         *  Render terrain object.
         */
        void                Draw( unsigned int id, float distance, bool isShadow = false );
        
        /**
         *  Delete terrain object.
         */
        void                Delete();

        /**
         *  Returns mesh at index.
         */
        inline ncMesh *             GetMesh( unsigned int id = 0 ) const {     return m_tMeshHolder; }

        /**
         *  Get object origin.
         */
        inline const Vec3 &                GetPosition() const {      return m_vPosition; }
        
        SLink   m_Link;

        //! Object allocator.
        INekoAllocator  * pAllocator = 0;
        
    private:

        //!  Object position.
        Vec3               m_vPosition;

        //!  Object angle ( 1.0, 1.0 * angle, 1.0 ).
        float                       m_fAngle;

        //!  Object mesh.
        ncMesh                      * m_tMeshHolder;
        
        //!  Billboard texture.
        SMaterialProp *   m_pMaterial;
    };
}

#endif /* defined(__lterrain_object__) */
