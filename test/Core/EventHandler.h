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
//  EventHandler.hpp
//  Neko engine
//
//  Created by Kawaii Neko on 9/25/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef EventHandler_cpp
#define EventHandler_cpp

#include "../Platform/Shared/SystemShared.h"
#include "Core.h"

namespace Neko {
    
    //! Function pointer caller
    typedef void (*FunctionPtr)( void * );
    
    /// Function call.
    struct SFunctionCallInfo
    {
        //! Function call.
        FunctionPtr     m_pFunction;
        
        //! Flags.
        uint16_t    m_iFlag;
        
        //! Call information.
        void        * m_pArg;
        
        //! Is thread active.
        bool    m_bActive;
        
        SFunctionCallInfo()
        {
            m_pArg = NEKO_NULL;
            m_iFlag = 0;
            m_pFunction = NEKO_NULL;
            
            m_bActive = false;
        }
    };
    
    /// Node info.
    struct SListNode
    {
    public:
        
        SListNode * m_next;
        SListNode * m_prev;
        
        SFunctionCallInfo * m_pFunctionInfo;
        
        // constructor
        SListNode()
        {
            m_next = m_prev = NEKO_NULL;
            m_pFunctionInfo = NEKO_NULL;
        }
        
        // destructor
        virtual ~SListNode()
        {
        
        }
    };

    /**
     *  A queue event.
     */
    struct Event {
        uint32_t Time;
        virtual void Handle() = 0;
    };

    ///   Event handler system.
    class EventSystem
    {
        NEKO_NONCOPYABLE( EventSystem );
        
    public:
        
        /**
         *  Constructor.
         */
        EventSystem();
        ~EventSystem();
        
        /**
         *  Initialize event handler.
         */
        void                Initialize();
        
        /**
         *  Handle events.
         */
        uint32_t                HandleEvents();

    private:
        //!  Current events.
        Event * m_pEvents;
        
        //!  Amount of events.
        uint32_t m_iEventsCount;
    };
}

#endif /* EventHandler_cpp */
