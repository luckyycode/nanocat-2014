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
//  Neko engine.
//
//  System manager..
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "System.h"
#include "../../Core/Network/Network.h"
#include "../../Core/Console/Console.h"
#include "../../AssetCommon/FileSystem.h"
#include "../../Core/String/StringHelper.h"
#include "../../Core/Core.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Platform/Shared/SystemShared.h"

// Apple OS-depend utilities.
#   if defined( NEKO_APPLE_FAMILY )
    #   if !defined( NEKO_IOS )
            #include "MacUtilities.h"
    #   else
            #include "iOSUtilities.h"
    #   endif
#   endif

// System class. Not an interface.

namespace Neko {

    /**
     *  System manager.
     */
    CSystem::CSystem() : _systemTime(0)
    {

    }

    /**
     *  Update stats.
     */
    void CSystem::UpdateStats( GameMemory * game_memory )
    {

#   if defined( NEKO_APPLE_FAMILY )
        vm_statistics Stats;
        mach_msg_type_number_t StatsSize = sizeof(Stats);
        host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&Stats, &StatsSize);
        
        // Get page size.
        vm_size_t PageSize;
        host_page_size(mach_host_self(), &PageSize);

        // Stats.
        uint64_t FreeMem = Stats.free_count * PageSize;
        uint64_t UsedMem = (Stats.active_count + Stats.inactive_count + Stats.wire_count) * PageSize;
        
        // Get task related statitics.
        task_basic_info_64_data_t TaskInfo;
        mach_msg_type_number_t TaskInfoCount = TASK_BASIC_INFO_COUNT;
        task_info( mach_task_self(), TASK_BASIC_INFO, (task_info_t)&TaskInfo, &TaskInfoCount );
        
        game_memory->iSystemFreeMemory = FreeMem;
        game_memory->iSystemUsedMem = UsedMem;
        
        game_memory->iProcessUsedMemory = TaskInfo.resident_size;
#   else
		// MemoryStatusEx stuff...
        
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof( statex );
        
        if( GlobalMemoryStatusEx( &statex ) ) {
            System_PhysMemory.Set( (int32_t)(statex.ullTotalPhys / MEGABYTE) );
            System_VirtMemory.Set( (int32_t)(statex.ullTotalVirtual / MEGABYTE) );
            
//            g_Core->p_Console->Print( LOG_INFO, "Installed physical memory: %u mb\n", statex.ullTotalPhys / MEGABYTE );
//            g_Core->p_Console->Print( LOG_INFO, "Available virtual memory: %u mb\n", statex.ullTotalVirtual / MEGABYTE );
        } else {
            g_Core->p_Console->Print( LOG_WARN, "Could not get system memory information.\n" );
        }
        
        //g_Core->p_Console->Print( LOG_INFO, "Central processor unit: %s\n", GetProcessorName() );

#   endif
    }
    
    /**
     *  System initialization.
     */
    void CSystem::Initialize( void )
    {
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "System initializing..\n" );
        
        Milliseconds();
        
        PrintSystemTime();
        
        _systemTime = 0;
        
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        // Non-blocking stdin.
        //fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
        //fcntl( 0, F_SETFL, fcntl( 0, F_GETFL, 0 ) | FNDELAY );

#	if defined( NEKO_APPLE_FAMILY )

        uint64_t ull;
        unsigned long ul;
        
        int32_t _cpu_family;
        int32_t _cpu_type;
        int32_t _cpu_sub;

        bool error;
        
#   if !defined( NEKO_IOS  )
        g_Core->p_Console->Print( LOG_INFO, "\tOSX Version: %s\n", ncMacUtilities::GetSystemVersion() );
#   endif
        // Memory size.
        error = GetSysCTLValue("hw.memsize", &ull);
        if( error ) {
            g_Core->p_Console->Print( LOG_WARN, "\tCouldn't get physical memory size.\n" );
        } else {
            g_Core->p_Console->Print( LOG_INFO, "\tPhysical memory installed: %d mb\n", (ull >> 20) );
            m_iPhysMemory = ( (int32_t)( ull >> 20 ) );
        }

//        g_Core->p_Console->Print( LOG_INFO, "\t%i mb free memory available.\n", GetSysFreeMemory() );

        // User memory.
        error = GetSysCTLValue("hw.usermem", &ul);
        if( error ) {
            g_Core->p_Console->Print( LOG_WARN, "\tCouldn't get user memory size.\n" );
        } else{
            g_Core->p_Console->Print( LOG_INFO, "\tUser memory: %d mb\n", ByteInMegabyte(ul >> 20) );
        }

        // CPU clock frequency.
        error = GetSysCTLValue("hw.cpufrequency", &ull);
        if( error ) {
            ull = 0;
            g_Core->p_Console->Print( LOG_WARN, "\tCouldn't determine CPU frequency.\n" );
        } else {
            ull /= 1000000;
            g_Core->p_Console->Print( LOG_INFO, "\tProcessor clock frequency: %d mhz\n", ull );
            m_iCPUSpeed = (int32_t)ull;
        }

        error = GetSysCTLValue("hw.cpufamily", &ul);
        
        _cpu_sub = -1;
        _cpu_family = -1;
        _cpu_type = -1;
        
        if( error ) {
            ul = 0;
            g_Core->p_Console->Print( LOG_WARN, "\tCouldn't determine CPU family.\n" );
        } else {
            _cpu_family = static_cast<int32_t>( ul );
            error = GetSysCTLValue("hw.cputype", &ul);
            if( error ) {
                g_Core->p_Console->Print( LOG_WARN, "\tCouldn't determine CPU type.\n" );
            } else {
                _cpu_type = static_cast<int32_t>( ul );
             
                error = GetSysCTLValue( "hw.cpusubtype", &ul );
                if( error )
                    g_Core->p_Console->Print( LOG_INFO, "\tCouldn't determine CPU sub-type.\n" );
                else
                    _cpu_sub = static_cast<int32_t>( ul );
                
                g_Core->p_Console->Print( LOG_INFO, "\tCPU( Family: 0x%08x Type: %d Subtype: %d )\n", _cpu_family, _cpu_type, _cpu_sub );
            }
        }
        
        // Obtain processor name.
        char   * name = NEKO_NULL;
        size_t size;
        int32_t ret;
        
        ret = sysctlbyname( "machdep.cpu.brand_string", NEKO_NULL, &size, NEKO_NULL, 0 );
        if( ret != 0 ) {
            return;
        }
        
        SMemoryTempFrame * tempStr = _PushMemoryFrame( pLinearAllocator2 );
        name = (char*)PushMemory( tempStr, sizeof(char) * size );
        
        ret = sysctlbyname( "machdep.cpu.brand_string", name, &size, NEKO_NULL, 0 );
        
        if( ret == 0 ) {
            printf( "Processor name: \"%s\"\n",  NekoCString::STR( "%s", name )  );
        }
        
        _PopMemoryFrame( tempStr );
        
#   if !defined( NEKO_IOS )

        // Number of cores.
        error = GetSysCTLValue("hw.ncpu", &ul);
        if( error ) {
            g_Core->p_Console->Print( LOG_WARN, "\tCouldn't determine CPU number of cores.\n" );
        } else {
            g_Core->p_Console->Print( LOG_INFO, "\tAvailable processor cores: %i\n", ul );
            m_iCPUCores = (int32_t)ul;
        }

#   endif

#	endif 

        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Not bad at all!\n" );
    }

    /**
     *  Get CPU name.
     */
    const char * CSystem::GetProcessorName()
    {
        return "Implement me!";
    }


    /**
     *  Get current user name.
     */
    const char * CSystem::GetCurrentUsername( void )
    {

#   if defined( _WIN64 )

        char username[MAX_USERNAME_LENGTH/*512*/ + 1];
        long unsigned int username_len = MAX_USERNAME_LENGTH/*512*/ + 1;
        GetUserName( username, &username_len );

        return username;

#   else

        struct passwd *u;

        if ( !( u = getpwuid( getuid() ) ) ) {
            return "User";
        }

        return u->pw_name;

#   endif

        return "Unknown";
    }

    /**
     *  Shutdown everything and quit safety.
     */
    void CSystem::Quit()
    {
        // Now just quit
        exit( EXIT_SUCCESS );
    }

    /**
     *  Switch mouse.
     */
    void CSystem::ToggleMouse( bool mEnable )
    {
#   if defined( NEKO_APPLE_FAMILY )

    #   if !defined( NEKO_IOS )
        ncMacUtilities::ShowMouseCursor();
    #   endif

#   else

#   endif
    }

    /**
     *  Message box ( system dependent ).
     */
    void CSystem::MessageBox( ncSysMessageBoxType mType, const char *mTitle, const char *mMessage )
    {
        // Message box for Windows
#   if defined( _WIN64 )

        ::MessageBox( NEKO_NULL, mMessage, mTitle, MB_OK | MB_ICONINFORMATION );

        HANDLE hOut;

        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hOut, FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY );

#   elif NEKO_APPLE_FAMILY
#   if !defined( NEKO_IOS )
        ncMacUtilities::MassageBox( mTitle, mMessage );
#   else
        //nciOSUtilities::MessageBox( mTitle, mMessage );
#   endif
#   endif // _WIN32
    }

    /**
     *  Hide window.
     */
    void CSystem::HideWindow()
    {

    }


#   if defined( NEKO_APPLE_FAMILY )
    const int32_t CSystem::GetSysCTLValue( const char key[], void *dest )
    {
        size_t len = 0;
        int32_t err;

        err = sysctlbyname( key, NEKO_NULL, &len, NEKO_NULL, 0 );
        if( !err ) {
            err = sysctlbyname( key, dest, &len, NEKO_NULL, 0 );
        }

        return err;
    }
//
//    const unsigned long CSystem::GetSysFreeMemory( void ) {
//        // Get free memory.
//        vm_size_t g_pageSize;
//        host_page_size( mach_host_self(), &g_pageSize );
//
//        // Get memory stats.
//        vm_statistics g_stats;
//        mach_msg_type_number_t g_statsSize = sizeof( g_stats );
//        host_statistics( mach_host_self(), HOST_VM_INFO, (host_info_t)&g_stats, &g_statsSize );
//
//        return ( g_stats.free_count * g_pageSize ) / MEGABYTE;
//    }

#   endif

    /**
     *  Print system time.
     */
    void CSystem::PrintSystemTime( void )
    {
        struct tm *tmp;
        time_t s;


        s = time( NEKO_NULL );
        tmp = localtime( &s );

        if( !tmp ) {
            g_Core->p_Console->Print( LOG_ERROR, "Unable to get system time.\n" );
            return;
        }

        g_Core->p_Console->Print( LOG_INFO, "Local time: %i:%i:%i\n", tmp->tm_hour, tmp->tm_min, tmp->tm_sec );

    }

    /**
     *  System frame function.
     */
    void CSystem::Frame( void )
    {

    }
}
