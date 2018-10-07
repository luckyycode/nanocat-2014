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
//  AssetPtr.h
//  Neko engine
//
//  Created by Kawaii Neko on 4/18/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef AssetPtr_h
#define AssetPtr_h

#include "../Platform/Shared/SystemShared.h"
#include "../Core/Utilities/List.h"
#include "../Core/String/String.h"

namespace Neko {
    
    //!  Asset type.
    enum class EAssetType : int32_t
    {
        Mesh    = 0,
        Sound,
        Shader,
        Material
    };

    /// Asset handler struct.
    struct TAssetPtr
    {
        TAssetPtr() : ptr(NEKO_NULL), bNeedsLoad(true), iTime(0)
        {
            
        }
        
        //! Asset type.
        EAssetType  eType;
        //! Asset filename.
        CStr     sName;
        //! Check if asset needs load.
        int8_t    bNeedsLoad:1;
        
        //! Cache link.
        SLink   m_Link;
        //! Cache load link.
        SLink   m_Link2;
        
        //! Time when asset was loaded.
        int32_t    iTime;
        
        void * ptr;
        
        //! Is asset available yet? (used by threads).
        int8_t    bAvailable:1;
    };
    
}

#endif /* AssetPtr_h */
