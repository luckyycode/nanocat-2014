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
//  Foliage.h
//  Neko
//
//  Created by Neko Code on 5/1/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__Foliage__
#define __Nanocat__Foliage__

#ifndef NEKO_SERVER

#include "Quadtree/Quadtree.h"
#include "../Math/GameMath.h"
#include "../Core/Utilities/List.h"
#include "../Math/Vec3.h"

namespace Neko {
    

    ///  Grass(foliage) cell.
    struct ncCGrass
    {
    public:
        //friend class CBeautifulEnvironment;
        
        //! Bush types.
        //! Flowers and another foliages.
        /**
         *  Bush types.
         */
        enum ncCGrassTypes
        {
            GRASS_BRUSH = 0,
            GRASS_LEAF,
        };
        
        //!  Cell position.
        Vec3    vPos;    
        
        //!  Cell rotation.
        float fRotation;
        
        ncCGrass();
        
//        void SetType( ncCGrassTypes type );
        
        /**
         *  New bush type.
         */
        ncCGrass( const float& mPositionX, const float& mPositionY, const float& mPositionZ );
        
        //! Rendering methods.
        
        /**
         *  Render cell.
         */
        void                Render();
        
        /**
         *  Set foliage properties.
         */
        void                Set( const float& mPositionX, const float& mPositionY, const float& mPositionZ );
        
        SLink   m_Link;
        
    private:
        
        /**
         *  Render foliage cell.
         */
        inline void                 RenderCell();
        
        /**
         *  Render foliage lower level cell.
         */
        inline void                 RenderLowLodCell();
    };

}

#endif // NEKO_SERVER

#endif /* defined(__Nanocat__Foliage__) */
