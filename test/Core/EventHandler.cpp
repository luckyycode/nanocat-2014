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
//  EventHandler.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 9/25/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "EventHandler.h"
#include "../Platform/Shared/System.h"
// - External headers.
#include "Network/MultiplayerClient.h"
#include "Network/MultiplayerServer.h"

#   if defined( NEKO_APPLE_FAMILY )
    #include "../Platform/Apple/OSX/OSX_External.h"
#   endif

namespace Neko {

//    SListNode * CQueue::g_freeNode = NEKO_NULL;

    /****       Event system.        ****/
    
    /**
     *  Init event handler system.
     */
    void EventSystem::Initialize()
    {
        g_Core->p_Console->Print( LOG_INFO, "Event system initializing\n" );
        
        m_pEvents = NEKO_NULL;
        m_iEventsCount = 0;
    }
    
    /**
     *  Constructor
     */
    EventSystem::EventSystem()
    {
        
    }
    
    /**
     *  Destructor.
     */
    EventSystem::~EventSystem()
    {
    
    }
    
    /**
     *  Handle system events.
     *
     *  @note - called from the main loop on the main thread.
     */
    uint32_t EventSystem::HandleEvents()
    {
        // Process network events.
        Byte * packet = NEKO_NULL;

        if( g_Core->p_Client->m_isHost == true ) {
            // Loopback client packets.
            while( (packet = g_Core->p_Network->GetLoopbackPacket( ENetworkChannelType::ClientServer ) ) != NEKO_NULL ) {
                g_Core->p_Server->Process( &g_Core->p_Client->CurrentServer, packet );
            }
            
            // Loopback server packets.
            while( (packet = g_Core->p_Network->GetLoopbackPacket( ENetworkChannelType::ServerClient ) ) != NEKO_NULL ) {
                g_Core->p_Client->Process( &g_Core->p_Client->CurrentServer, packet );
            }
        }
        
        
        
        // wtf
        return g_Core->p_System->Milliseconds();;
    }
}