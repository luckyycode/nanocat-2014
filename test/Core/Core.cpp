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
//  Core.cpp
//  Game core.. :^)
//
//  This code is a part of Neko engine.
//  Created by Neko Vision on 10/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "../AssetCommon/AssetBase.h"
#include "../AssetCommon/FileSystem.h"
#include "../AssetCommon/Material/ImageManager.h"
#include "../Graphics/OpenGL/OpenGLBase.h"
#include "../Graphics/Renderer/Renderer.h"
#include "../Platform/Shared/System.h"
#include "Console/Console.h"
#include "Console/ConsoleCommand.h"
#include "Core.h"
#include "EventHandler.h"
#include "GameMemory.h"
#include "Joystick.h"
#include "Network/MultiplayerClient.h"
#include "Network/MultiplayerServer.h"
#include "Network/Network.h"
#include "Player/Camera/Camera.h"
#include "Player/Input/Input.h"
#include "ScriptSupport/Scripting.h"
#include "String/StringHelper.h"
#include "Utilities/Hashtable.h"
#include "Utilities/Utils.h"
#include "XmlParser.h"
#include "../Platform/Shared/SystemShared.h"

#include "Queue.h"
/**
 *  Game engine core.
 *  Rendering loop is called from another thread.
 *
 *  'Preload' should be ALWAYS called before all system initialization.
 */

namespace Neko {

    //!  Engine version.
    SConsoleVar     * Core_Version = 0;
    //!  Maximum framerate.
    SConsoleVar     * Core_MaxFPS = 0;
    //!  Time multiplier.
    SConsoleVar     * Core_TimeMultiplier = 0;
    //!  Heap memory size.
    SConsoleVar     * Com_HeapMemory = 0;
    //! Enable game controllers?
    SConsoleVar     * Com_Gamepad = 0;
    
    //! Current gamepad preset.
    SConsoleVar     * Gamepad_Preset = 0;
    
    //! Skip sound system load?
    SConsoleVar     * Skip_AudioEngine = 0;
    
    /// ============================================================================================
    
    /**
     *  Constructor.
     */
    CCore::CCore() : Time( -1 ), LastTime( 0 ), FrameTime( 0 ),
                bInitialized( false ), bUseGraphics( true )
    {

    }

    /**
     *  Get Unique ID.
     */
    void CCore::CreateGuid( nekoguid_t & Result )
    {
#   if defined( NEKO_APPLE_FAMILY ) // x64
        uuid_t UUID;
        uuid_generate( UUID );
        
        uint32_t * Values = (uint32_t *)(&UUID[0]);
        
        Result.a = Values[0];
        Result.b = Values[1];
        Result.c = Values[2];
        Result.d = Values[3];
#   elif defined( NEKO_WINDOWS_FAMILY )
        // @todo
        
#   endif
    }
    
    /// ============================================================================================
    
    /**
     *  Add game controller device.
     */
    bool CCore::AddGameController( const int32_t deviceIndex )
    {
        IGamepadController * controller = NEKO_NULL;
        
        // Check if requested device is a controller.
        if( p_GameControllerSys->IsGameController( deviceIndex ) ) {
            controller = new IGamepadController();
            controller->pSysJoystick = g_Core->m_pSysJoystick;
            controller->OpenController( deviceIndex );
            
            p_Console->Print( LOG_INFO, "AddGameController: %s\n", controller->joystick->name );
            
            m_pGamepad[deviceIndex] = controller;
            
            return true;
        }
        
        return false;
    }
    
    /**
     *  Remove game controller device.
     */
    bool CCore::RemoveGameController( const int32_t deviceIndex )
    {
        IGamepadController * controller = NEKO_NULL;
        
//        if( p_GameControllerSys->IsGameController( deviceIndex ) ) {
            controller = CheckControllerExistance( deviceIndex );
            
            if( controller != NEKO_NULL ) {
                p_Console->Print( LOG_INFO, "RemoveGameController: %s\n", controller->joystick->name );
                
                controller->CloseController();
                
                g_Core->m_pGamepad[deviceIndex] = NEKO_NULL;
                
                delete controller;
                return true;
            }
//        }
        
        return false;
    }
    
    /// ============================================================================================
    
    /**                                     **/
    /**             Game memory.            **/
    /**                                     **/
    
    const static uint64_t   kMemorySize = Megabyte( 1024 );
    const static uint64_t   kClassAllocatorSize = Megabyte(48); // holds only classes, not its "contents"
    const static uint64_t   kBaseLinearAllocatorSize = Megabyte(256);   // linear allocator used for things such as temporary pools, etc..
    
    GameMemory    mGameMemory;
    
    CFreeNodeAllocator      * pMainAlloc = 0;
    CProxyAllocator         * pMainAllocProxy = 0;
    CLinearAllocator        * pClassLinearAllocator = 0;   // Allocated using proxy.
    CLinearAllocator        * pLinearAllocator2 = 0;   // Allocated using proxy.
    
    
    /**
     *    Pre-load stuff, even before context & graphic stuff load.
     *    Initializes the most important stuff.
     *
     *    @param execpath   Execution directory path.
     */
    void CCore::Preload( const char * execpath )
    {
        // O
        srand( (uint32_t)time( NEKO_NULL ) );
        
        // Initialize console and things like console variables.
        p_Console = new CGameConsole(); // we have to initialize console handler using default allocator
        p_Console->Initialize();
        p_Console->Print( LOG_NONE, "\n" );
        p_Console->Print( LOG_INFO, "Preparing memory..\n" );
        
        InitMem( &mGameMemory, kMemorySize, false );
        // Create linked-list allocator.
        pMainAlloc = new CFreeNodeAllocator( kMemorySize, mGameMemory.pMemoryBase );

        // Create proxy allocator to watch allocations happening in main allocator.
        pMainAllocProxy = new CProxyAllocator( pMainAlloc );
        pClassLinearAllocator = NekoAllocator::newLinearAllocator( kClassAllocatorSize, *pMainAllocProxy );
        pLinearAllocator2 = NekoAllocator::newLinearAllocator( kBaseLinearAllocatorSize, *pMainAllocProxy );
        
        // Reset game time.
        Time = 0;
        LastTime = 0;
        FrameTime = 0;

        bInitialized = false;
        bUseGraphics = true;

        // Create console variable cache.
        p_Console->CreateCache();
        Core_Version = p_Console->RegisterCVar( ECvarGroup::Engine, "iCoreVersion", "Game engine version.", "7.1k", CVFlag::Readonly, ECvarType::String );    // Register some commands.
        Core_TimeMultiplier = p_Console->RegisterCVar( ECvarGroup::Engine, "fCoreTimeMult", "Game time multiplier.", 1.0f, CVFlag::None, ECvarType::Float );
        Core_MaxFPS = p_Console->RegisterCVar( ECvarGroup::Engine, "iCoreMaxFps", "Game maximum framerate.", 60, CVFlag::None, ECvarType::Int );

        Com_Gamepad = p_Console->RegisterCVar( ECvarGroup::Engine, "bGameControllers", "Enable game controllers?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        Com_HeapMemory = p_Console->RegisterCVar( ECvarGroup::Engine, "iMemorySize", "Heap memory size", 1024, CVFlag::Readonly, ECvarType::Int );
        Gamepad_Preset = p_Console->RegisterCVar( ECvarGroup::Engine, "sGamepadPreset", "Current gamepad preset", "mac", CVFlag::NeedsRefresh, ECvarType::String );
        Skip_AudioEngine = p_Console->RegisterCVar( ECvarGroup::Engine, "bSkipAudioLoad", "Skips sound system load", false, CVFlag::NeedsRefresh, ECvarType::Int );
        
        // Initialize file system.
        p_FileSystem = (CFileSystem *)pClassLinearAllocator->Alloc( sizeof(CFileSystem) );
        if( p_FileSystem == NEKO_NULL ) {
            p_Console->Error( ERR_FATAL, "Not enough memory to create file system\n" );
        }
        
        p_FileSystem->Initialize( execpath, pMainAllocProxy );

        // System stuff.
        p_System = (CSystem *)pClassLinearAllocator->Alloc( sizeof(CSystem) );
        if( p_System == NEKO_NULL ) {
            p_Console->Error( ERR_FATAL, "Not enough memory to create system interface\n" );
        }
        
        p_System->UpdateStats( &mGameMemory ); // Computer RAM usage.
        p_System->Initialize();
        
        // Initialize the event system.
        p_EventSystem = (EventSystem *)pClassLinearAllocator->Alloc( sizeof(EventSystem) );
        if( p_EventSystem == NEKO_NULL ) {
            p_Console->Error( ERR_FATAL, "Not enough memory to create event system\n" );
        }
        
        p_EventSystem->Initialize();
        
#   if defined( USES_LUA )
        // Prepare game scripting.
        p_ScriptBase = (ncScriptBase *)pClassLinearAllocator->Alloc( sizeof(ncScriptBase) );
        p_ScriptBase->Init( pMainAllocProxy );
#   endif
        
        // Preload Neko renderer properties.
        g_mainRenderer = (CRenderer *)pClassLinearAllocator->Alloc( sizeof(CRenderer) );
        if( g_mainRenderer == NEKO_NULL ) {
            p_Console->Error( ERR_FATAL, "Not enough memory to create renderer\n" );
        }
        
        g_mainRenderer->Preload();
        // Itself, Neko renderer initializes in another thread!
        
        if( Com_Gamepad->Get<bool>() ) {
            // Reset gamepads.
            for( int32_t i = 0; i < MAX_CONTROLLERS; ++i ) {
                m_pGamepad[i] = NEKO_NULL;
            }
            // Init
            p_GameControllerSys = (CGameControllerBase *)pClassLinearAllocator->Alloc( sizeof(CGameControllerBase) );
            if( p_GameControllerSys == NEKO_NULL ) {
                p_Console->Error( ERR_FATAL, "Not enough memory to create game controller interface\n" );
            }
            
            p_GameControllerSys->Init();
            
            for( int32_t i(0); i < p_GameControllerSys->GetNumJoysticks(); ++i ) {
                
                // Check if we can add the controller.
                if( AddGameController( i ) ) {
                    break;
                };
            }
        }
        

        // Initialize Neko engine.
        Initialize();
    }

    /**
     *  Warning message.
     */
    void CCore::Assert( const char * msg )
    {
        g_Core->p_Console->Print( LOG_WARN, "Assert: \"%s\" \n" );
    }

    /**
     *   Game core, load only on application launch!
     */
    void CCore::Initialize()
    {
        p_System->Milliseconds();
        
        int32_t  t1, t2;
        
        t1 = p_System->Milliseconds();

        // Build version info.
        Core_Version->Set<const char*>( NEKO_BUILD_VERSION );

        // Notify us.
        p_Console->Print( LOG_NONE, "\n" );
        p_Console->Print( LOG_INFO, "Loading Neko engine core, %s\n", Core_Version->Get<const char*>() );
        p_Console->Print( LOG_INFO, "Build: %s, %s\n", __TIME__, __DATE__ );
        p_Console->Print( LOG_INFO, "%d mb memory heap.\n", ByteInMegabyte(mGameMemory.iMemorySize) );

        // Initialize network.
        p_Network = (CNetwork *)pClassLinearAllocator->Alloc( sizeof(CNetwork) );
        p_Network->Initialize( pMainAllocProxy );

        g_pWorldPhysics = (CWorldPhysics *)pClassLinearAllocator->Alloc( sizeof(CWorldPhysics) );
        
        // Initialize client system.
        p_Client = (ncClient *)pClassLinearAllocator->Alloc( sizeof(ncClient) );
        p_Client->Initialize( pMainAllocProxy );
        // Create server.
        p_Server = (CServer *)pClassLinearAllocator->Alloc( sizeof(CServer) );
        p_Server->Initialize( pMainAllocProxy );

        // Input and camera.
        // User input.
        p_Input = (CInputInterface *)pClassLinearAllocator->Alloc( sizeof(CInputInterface) );
        p_Input->Initialize();

        // Camera.
        p_Camera = (CCamera *)pClassLinearAllocator->Alloc( sizeof(CCamera) );
        p_Camera->Initialize();

        // OpenAL manager.
#   if !defined( NEKO_SERVER )
        p_SoundSystem = (CSoundManager *)pClassLinearAllocator->Alloc( sizeof(CSoundManager) );
        p_SoundSystem->Initialize( pMainAllocProxy );
#   endif
        
        // Ok!
        bInitialized = true; t2 = g_Core->p_System->Milliseconds();

        p_Console->Print( LOG_INFO, "Neko engine core took %i msecs to load.\n", t2 - t1 );
        p_Console->Print( LOG_NONE, "\n" );
    }

    /**
     *  All systems got loaded, so do something after it.
     */
    void CCore::Loaded( void )
    {
        // Keep console clean.
//        g_Core->p_Console->Execute( "clear" );

        g_Core->p_Console->Print( LOG_INFO, "All systems were loaded.\n" );
        g_Core->p_Console->Print( LOG_DEVELOPER, "%i errors and %i warnings found while initializing.\n", g_Core->p_Console->logErrors, g_Core->p_Console->logWarnings );

        if( Server_Dedicated->Get<bool>() )  {
            g_Core->p_Console->Execute("launch");
            g_Core->p_Console->Print( LOG_INFO, "See server configuration file to edit parameters.\n" );
            g_Core->p_Console->Print( LOG_INFO, "Server has been successfully created.\n" );
        }

    }
    
    /// ============================================================================================
    
    // using standart allocator here
    
    /**
     *  Create a new thread.
     *
     *  @param priority    Thread priority.
     *  @param threadCount Thread count.
     *  @param arg1        An argument to pass.
     *  @param args        An argument list.
     *
     *  @return A new thread.
     */
    INekoThread * CCore::CreateThread( int32_t priority, int32_t threadCount, ThreadCallback callback, void *arg1, void **args )
    {
#   if defined( NEKO_UNIX_FAMILY ) // Linux also
        return new UnixThread( priority, threadCount, callback, arg1, args );
#   elif defined( NEKO_WINDOWS_FAMILY )
		return new Win64Thread(threadCount, priority, callback, arg1, args);
#   endif
    }
    
    /**l
     *  Create a new thread mutex.
     */
    INekoThreadLock * CCore::CreateLock()
    {
#   if defined( NEKO_UNIX_FAMILY )
        return new UnixThreadLock();
#   elif defined( NEKO_WINDOWS_FAMILY )
        return new Win64Lock();
#   endif
    }
    
    /**
     *  Create a new thread event ( condition variable ).
     */
    INekoThreadEvent * CCore::CreateEvent( INekoThreadLock * mutex )
    {
#   if defined( NEKO_UNIX_FAMILY )
        if( mutex != NEKO_NULL ) {
            return new UnixThreadEventLockless( (UnixThreadLock *)mutex );
        } else {
            return new UnixThreadEvent();
        }
#   elif defined( NEKO_WINDOWS_FAMILY )
        return new Win64Event();
#   endif
    }
    
    /// ============================================================================================
    
    CCore  * g_Core = 0;

    /**
     *  Game process.
     *  Let the everything create/think/move/load/do/read/write etc..
     */
    void CCore::Frame()
    {
        int32_t msec, minMsec;
        float   scale;

        // Main game time count.
        if ( Core_MaxFPS->Get<int32_t>() > 1 ) {
            minMsec = 1000 / Core_MaxFPS->Get<int32_t>();
        } else {
            minMsec = 1;
        }

        do {
            // Do queued events.
            FrameTime = p_EventSystem->HandleEvents();

            if ( LastTime > FrameTime ) {
                LastTime = FrameTime;
            }

            msec = FrameTime - LastTime;
        } while ( msec < minMsec );
        
        // Don't go further until we don't get initialized.
        if( !bInitialized ) {
            return;
        }
        
        LastTime = FrameTime;

//        scale = Core_TimeMultiplier.Get<float>();
//        if( scale < 1.0f )
//        {
//            Core_TimeMultiplier.Set<float>( 1.0f );
//        }
        
        
        scale = 1.0f;
        msec = (uint32_t)(msec * scale);

        if( msec < 1 ) {
            msec = 1;
        } else if( msec > 5000 ) {
            msec = 5000;
        }

        if( msec > 500 ) {
            g_Core->p_Console->Print( LOG_INFO, "Neko engine overloaded.. or just stuck for %d ms\n", msec );
        }

        Time += msec;
        
        frameMsec = msec;
   
#   if !defined( NEKO_EDITOR )
      
        // Network manager.
        p_Network->Frame();

        // Server and client framing.
        p_Client->Frame( msec );
        p_Server->Frame( msec );

#   else // Renderer in Editor mode is running on the main thread.
        g_mainRenderer->Render( msec );
#   endif
        
        // Update sound.
#   if !defined( NEKO_SERVER )
        p_SoundSystem->UpdateListener( Time );
#   endif
        //  g_Core->p_Camera->Frame( g_Core->GetTime() );      // Should you move me to core thread? Not now, you make strange stuff!
        // Again, renderer drawing is called from its thread.
       
        // Update frustum.
        //g_Core->p_Camera->mCommonFrustum.Extract( g_pbEnv->GetSunLightPosition() );
        if( p_Camera->UpdateFrustum() ) {
            p_Camera->m_CommonFrustum.Extract( p_Camera->vEye );
        }
    }

    /**
     *  Called after system events.
     */
    void CCore::AfterPostEvent()
    {
        // Update gamecontrollers.
        if( p_GameControllerSys != NEKO_NULL ) {
            p_GameControllerSys->UpdateGamepads();
        }
    }
    
    /**
     *  "Smart" disconnect.
     *  Disconnect from server or remove server if we are host.
     */
    void CCore::Disconnect( void )
    {
        if( p_Client->m_bRunning ) {
            p_Client->Disconnect();
        }

        if( p_Server->m_bRunning ) {
            p_Server->Disconnect();
        }
    }
    
    //! Temp.
    static void InitMemoryTest()
    {
        static const uint32_t _randomWords[] = {
            0xfeedc0de,
            0x1337d05e,
            0xdeadbeef,
            0xd3adb33f,
            0x1337c0de,
        };

    }

    /**
     *  Calls only on application quit.
     *
     *  @param quitApp  Should we totally quit application?
     */
    void CCore::Quit( const char * reason, bool quitApp )
    {
        // Notify the user.
        p_Console->Print( LOG_NONE, "\n" );
        p_Console->Print( LOG_INFO, "Application quit. %s\n", reason );
        
        bInitialized = false;
        
        // Disconnect from server. ( If connected )
        Disconnect();

        // Disconnect server.
        p_Console->Execute( "lazykillserver" );
        
        // Shutdown the networking.
        p_Network->Shutdown();
        
        // Client..
        p_Client->Shutdown();

        // Server..
        p_Server->Shutdown( "Game quit\n" );
        
        // Some information.
        p_Console->Print( LOG_INFO, "Game was launched for %i minute(s)\n", ( Time / 60000 ) );
        p_Console->Print( LOG_INFO, "Seems like everything is okay.. quitting!\n" );

        // Destroy the camera entity..
        p_Camera->Destroy();
        
        // Remove controllers.
        p_GameControllerSys->RemoveControllers();
        
        // Shutdown the sound system.
#   if !defined( NEKO_SERVER )
        p_SoundSystem->Shutdown();
#   endif
        // Shutdown the file system.
        p_FileSystem->Shutdown();

        // Renderer shutdown.
        //g_mainRenderer->Shutdown();

        // Clear the console buffer & write to the file.
        p_Console->Execute( "clear" );
        p_Console->Shutdown();
        delete p_Console ;
        
        pLinearAllocator2->Reset();
        NekoAllocator::deleteLinearAllocator( *pLinearAllocator2, *pMainAllocProxy );
        NekoAllocator::deleteLinearAllocator( *pClassLinearAllocator, *pMainAllocProxy );
        
        delete pMainAllocProxy;
        delete pMainAlloc;
        
        printf( "Bye, %s!", p_System->GetCurrentUsername() );

        // Quit application?
        if( quitApp )
        {
            p_System->Quit();
        }
    }
    
    /// ============================================================================================
}
