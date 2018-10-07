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
//  APIObject.h
//  Interface object ID.
//
//  Created by Kawaii Neko on 11/27/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef APIObject_h
#define APIObject_h

#include "../Platform/Shared/SystemShared.h"

namespace Neko {
    
    /// An interface object.
    class APIObject
    {
    public:
        
        /**
         *  Constructor.
         */
        APIObject() : m_APIhandle(0)
        {
            
        }
        
        /**
         *  Initialise API with the name.
         */
        APIObject( const char * Name ) : m_APIhandle(0)
        {
            
        }
        
        /**
         *  Is current handle valid?
         */
        NEKO_FORCE_INLINE bool                 IsValid() const  {   return (m_APIhandle != 0); }
        
        /**
         *  Set current API handle id.
         */
        NEKO_FORCE_INLINE void                 SetHandle( uint32_t Handle )    { m_APIhandle = Handle; }
        
        /**
         *  Get current API handle.
         */
        NEKO_FORCE_INLINE uint32_t                 GetHandle()     { return m_APIhandle;   }
        
//    private:
        
        //! Interface handle.
        uint32_t   m_APIhandle;
    };
}

#endif /* APIObject_h */
