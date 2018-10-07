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
//  Core.h
//  Game core.. :^)
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef core_h
#define core_h

#include "CoreDef.h"

#include "Console/ConsoleVariable.h"
#include "String/StringHelper.h"
#include "Console/Console.h"
//#include "../../Libraries/tinythread.h"
#include "GameMemory.h"
#include "Threading.h"
#include "../AssetCommon/Sound/SoundManager.h"
#include "Console/Console.h"
#include "Joystick.h"

namespace Neko {

    //! Maximum engine game joysticks.
    static const int32_t    MAX_CONTROLLERS = 4;
    
    //! Neko engine version.
    static const char * NEKO_BUILD_VERSION = "7.1";

    //! System stuff.
    static const int32_t MAX_SPRINTF_BUFFER     = 16384;

    //! Maximum server world entities ( clients, etc )
    static const int32_t   MAX_SERVER_ENTITIES = 32;
    
    /// ============================================================================================
    ///  Unique ID.
    struct nekoguid_t
    {
        int32_t a;
        int32_t b;
        int32_t c;
        int32_t d;
        
        /**
         *  Create a GUID.
         */
        nekoguid_t( int32_t first, int32_t second, int32_t third, int32_t fourth ) {
            a = first; b = second; c = third; d = fourth;
        }
        
        const int32_t operator [] ( int32_t & i ) {
            switch( i ) {
                case 0: return a;
                case 1: return b;
                case 2: return c;
                case 3: return d;
            }
            
            return 0;
        }
    };
    
    /// ============================================================================================
    //! --- Main game memory section.
    extern GameMemory    mGameMemory;
    
    extern CLinearAllocator * pMainLinearAllocator;
    
    extern CFreeNodeAllocator      * pMainAlloc;
    extern CProxyAllocator         * pMainAllocProxy;
    extern CLinearAllocator        * pClassLinearAllocator;   // Allocated using proxy.
    
    extern CLinearAllocator        * pLinearAllocator2;   // Allocated using proxy.
    
    /// ============================================================================================
    ///  Game engine core.
    class CCore
    {
        NEKO_NONCOPYABLE( CCore );
        
    public:
        
        /**
         *  Constructor..
         */
        CCore();
        
        /**
         *  Destructor.
         */
        ~CCore() {
            
        }

        /**
         *  Preload game core ( initializes CVar, file systems ).
         *  @execpath - current game directory.
         */
        void                Preload( const char * execpath );

        /**
         *  Initialize game core.
         */
        void                Initialize();

        /**
         *  Called when we got fully initialized.
         */
        void                Loaded();

        /**
         *  Game loop.
         *  Rendering loop is in its thread!
         */
        void                Frame();

        /**
         *  Called after system events.
         */
        void                AfterPostEvent();
        
        /**
         *  Disconnect client or kill server if exists.
         */
        void                Disconnect();

        /**
         *  Quit the game.
         */
        void                Quit( const char * reason, bool quitApp = true );

        /**
         *  Get game time ( in milliseconds ).
         */
        inline uint32_t                 & GetTime() {       return Time;    }
        
        /**
         *  Get frame time ( in msecs ).
         */
        inline uint32_t                 GetFrameTime()  {       return frameMsec;   }
        
        /**
         *  Uses graphics?
         */
        inline bool                 UsesGraphics() const    {       return bUseGraphics;    }

        /**
         *  Initialized?
         */
        inline bool                 Initialized() const {       return bInitialized;    }

        /**
         *  Set graphics usage flag.
         */
        inline void                 SetUsesGraphics( bool _usesGraphics )   {       bUseGraphics = _usesGraphics;   }

        /**
         *  Catch errors.
         */
        static void                 Assert( const char * );
        
        /**
         *  Create an unique id.
         */
        static void                 CreateGuid( nekoguid_t & Result );

        /**
         *  Create a new thread.
         */
        static INekoThread               * CreateThread( int32_t priority, int32_t threadCount, ThreadCallback callback, void * arg1, void ** args );
        
        /**
         *  Create a thread lock mutex.
         */
        static INekoThreadLock               * CreateLock();
        
        /**
         *  Create a thread event ( condition variable ).
         */
        static INekoThreadEvent              * CreateEvent( INekoThreadLock * mutex = NEKO_NULL );
        
        /**
         *  Destroy thread.
         */
        static void KillThread( INekoThread * thread ) {
            if( thread != NEKO_NULL ) {
#   if defined( _WIN32 )
                delete (Win64Thread*)thread;
#   else
                delete (UnixThread*)thread;
#   endif
            }
        }
        
        /**
         *  Destroy lock.
         */
        static void KillLock( INekoThreadLock * lock ) {
            if( lock != NEKO_NULL ) {
                delete lock;
                
//                while( (error = pthread_mutex_destroy( mutex )) == EBUSY ) {
//                    usleep(100000);
//                }
            }
        }
        
        /**
         *  Destroy event.
         */
        static void KillEvent( INekoThreadEvent * event ) {
            if( event != NEKO_NULL ) {
                delete event;
            }
        }
        
        /// Sound manager instance.
        class CSoundManager * p_SoundSystem;
        
        /// Game console instance.
        class CGameConsole * p_Console;
        
        /// Server instance.
        class CServer      * p_Server;
        
        /// Client instance.
        class ncClient      * p_Client;
        /// Client world interface.
        class ClientWorld   * p_ClientWorld;
       
        /// Network interface.
        class CNetwork     * p_Network;
        
        /// Game input.
        class CInputInterface       * p_Input;
        
        /// Game main camera.
        class CCamera      * p_Camera;
        
        /// File system.
        class CFileSystem  * p_FileSystem;
        
        /// Computer system interface.
        class CSystem      * p_System;
        
        /// Script support interface.
        class ncScriptBase  * p_ScriptBase;
        
        /// Event system.
        class EventSystem   * p_EventSystem;
        
        /// Game controller system.
        class CGameControllerBase   * p_GameControllerSys;
        
        /**
         *  Add game controller device.
         */
        bool                AddGameController( const int32_t deviceIndex );
        
        /**
         *  Remove game controller device.
         */
        bool                RemoveGameController( const int32_t deviceIndex );
        
        //! System gamepad interface.
        IGamepadSystemInterface *       m_pSysJoystick;
        
        //! Game controllers connected.
        IGamepadController          * m_pGamepad[MAX_CONTROLLERS];
        
        //! Frame per msec.
        uint32_t                frameMsec;
        
    private:

        //! Variables used in game loop.
        uint32_t                LastTime;
        uint32_t                FrameTime;

        //! Game time.
        uint32_t                    Time;

        //!  Game core initialized?
        bool                bInitialized;

        //!  Use Graphical API and load assets?
        bool                bUseGraphics;   // Mostly used for dedicated server.

    protected:
    };

    extern SConsoleVar      * Skip_AudioEngine;
    
    extern CCore   *g_Core;
    /// ============================================================================================
}


#endif
