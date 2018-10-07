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
//  Console.cpp
//  Game console handle.. ^.^
//
//  This code is a part of Neko engine.
//  Created by Neko Vision on 03/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//


#include "../Core.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "../../AssetCommon/FileSystem.h"
//#include "../../Graphics/Renderer/FontRenderer.h"
#include "../Player/Camera/Camera.h" // Movement.
#include "../../Platform/Shared/System.h"
#include "../Player/Input/Input.h" // For console input.
#include "../../Math/GameMath.h" // Colors.
//#include "../../Graphics/Renderer/Renderer.h"
#include "../../World/BeautifulEnvironment.h" // LoadsWorld?
#include "../../Graphics/OpenGL/OpenGLBase.h" // Initialized?
#include "../Streams/MemoryStream.h"

#   if defined( _WIN32 )
        #include "../../Platform/Windows/Main32.h"
#   endif // _WIN32

namespace Neko {

    /**
     *  Create new console instance.
     */
    CGameConsole::CGameConsole()
    {
        logFile = NEKO_NULL;

        logWarnings = 0;
        logErrors = 0;
        logCount = 0;
        logFill = 0;

        isShown = false;
    }

    /**
     *  Destructor.
     */
    CGameConsole::~CGameConsole()
    {
    
    }

    /**
     *  Shutdown the game console.
     */
    void CGameConsole::Shutdown()
    {
        g_Core->p_Console->Print( LOG_DEVELOPER, "Game console quit called..\n" );
        
        // delete all cvars
        SConsoleVar * var = NEKO_NULL;
        
        SLink   * head;
        SLink   * cur;
        
        const char    * cvarName = NEKO_NULL;
        
        head = &m_CVarCache.m_List.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            cvarName = (const char *) cur->m_ptrData;
            // cvarName can't be null
            var = m_CVarCache[cvarName];
            
            m_pCvarAllocator->Dealloc( var );
            var = NEKO_NULL;
        }
        
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)m_pCvarAllocator, pAllocatorHandle );
        m_CVarCache.Delete();
    }

    
    
#define MAX_FILESIZE_INI    128

    /**
     *  Get section name by group index.
     */
    static const char * GetSectionName( const ECvarGroup group )
    {
        switch( group )
        {
            case ECvarGroup::Display:     return "Display";
            case ECvarGroup::Input:     return "Input";
            case ECvarGroup::Server:     return "Server";
            case ECvarGroup::Client:     return "Client";
            case ECvarGroup::Network:     return "Network";
            case ECvarGroup::Engine:     return "Engine";
            case ECvarGroup::Player:     return "Player";
            case ECvarGroup::World:     return "World";
            default:
                return "None";
        }
    }
    
    /**
     *  Write Cvars to Ini file.
     *
     *  @note dang, this is complex
     */
    void CGameConsole::WriteIni( const char *name )
    {
        FILE *  ini = CFileSystem::OpenWrite( NC_TEXT( "%s/%s.ini", Filesystem_Path->Get<const char*>(), name ) );
        
        // Get sections.
        SConsoleVar * var = NEKO_NULL;
        
        SLink   * head;
        SLink   * cur;
        
        const char    * cvarName = NEKO_NULL;

        head = &m_CVarCache.m_List.m_sList;
        
        // Write cvars by sections.
        for( int32_t i(0); i < (int32_t)ECvarGroup::Max; ++i ) {
            // Print a section.
            fprintf( ini, "\n[%s]\n", GetSectionName( (Neko::ECvarGroup)i ) );
            
            for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
                cvarName = (const char *) cur->m_ptrData;
                // cvarName can't be null
                var = m_CVarCache[cvarName];
                if( var->eGroup == (ECvarGroup)i && var->eFlag != (uint8_t)CVFlag::Hidden ) {
                    if( var->eType == ECvarType::Float ) {
                        fprintf( ini, "%s=%f\n", var->GetName(), var->Get<float>() );
                    } else if( var->eType == ECvarType::String ) {
                        fprintf( ini, "%s=%s\n", var->GetName(), var->Get<const char*>() );
                    }if( var->eType == ECvarType::Int ) {   // same as boolean
                        fprintf( ini, "%s=%i\n", var->GetName(), var->Get<int32_t>() );
                    }
                }
            }
        }
        
        fflush( ini );
        fclose( ini );
    }
    
    /**
     *  Load Ini file.
     */
    void CGameConsole::LoadIni( const char *name )
    {
        char    * line;
        int32_t     charsRead;
        SMemoryTempFrame    *  tempMemory = NEKO_NULL; // file loading
        
        char * readBuffer = NEKO_NULL;
        tempMemory = _PushMemoryFrame( pLinearAllocator2 );
        
        CFileSystem::Load( NekoCString::STR("%s/%s.ini", Filesystem_Path->Get<const char*>(), name), (void**)&readBuffer, tempMemory );
        
        // Parse asset list now.
        line = (char* )PushMemory( tempMemory, sizeof(char) * MAX_FILESIZE_INI );  ;
        
        charsRead = 0;
        
        char * end = NEKO_NULL;
        char * start = NEKO_NULL;
        char * value = NEKO_NULL;
        
        char section[64] = "";
        char prev_name[64] = "";
        
        int32_t lineno = 0;
        int32_t error = 0;
       
        // Read line by line.
        while( str_readline( line, MAX_FILESIZE_INI, (const char *)readBuffer, charsRead ) && line != NEKO_NULL ) {
            start = line;
            start = NekoCString::lskip(NekoCString::rstrip(start)) ;
            
            ++lineno;
            
            if( *start == '#' || *start == ';' || *start == '\n' ) { // comment
                
            } else if( *start == '[' ) {
                // "[section]"
                end = NekoCString::FindChars(start + 1, "]");
                if( *end == ']' ) {
                    *end = '\0';
                    NekoCString::strncpy0( section, start + 1, sizeof(section) );
                    *prev_name = '\0';
                } else if( !error ) {
                    // Syntax error. No ']' found
                    error = lineno;
                }
            } else if( *start ) {
                // Find a valid pair.
                end = NekoCString::FindChars(start, "=:");
                if( *end == '=' || *end == ':' ) {
                    *end = '\0';
                    name = NekoCString::rstrip( start );
                    value = NekoCString::lskip( NekoCString::rstrip( end + 1 ) );
                    
                    NekoCString::strncpy0( prev_name, name, sizeof(prev_name) );
//                    //section, name, value
//                    printf( "section=%s, name=%s, value=%s\n", section, name, value );
                    
                    SConsoleVar * var = m_CVarCache[name];
                    if( var != NEKO_NULL ) {
                        // Command does exist
                        if( !strcmp( GetSectionName( var->eGroup ), section ) ){
                            var->Set( (const char*)value );
                        }
                    }
                    
                }
            }
        }
        
        _PopMemoryFrame( tempMemory );
    }
    
    
    
    /**
     *  Console input from external/internal sources.
     */
    void CGameConsole::Execute( const char * msg, ... )
    {
        va_list             argptr;
        static char         g_message[MAX_COMMAND_SIZE];
        
        // Check command length.
        if( strlen( msg ) > MAX_COMMAND_SIZE )
        {
            g_Core->p_Console->Print( LOG_DEVELOPER, "Command length is too long.\n" );
            return;
        }

        // Command parameters.
        va_start( argptr, msg );
        vsnprintf( (char*)g_message, sizeof(g_message), msg, argptr );
        va_end( argptr );

        // Make me to look sexier.

        // We need to make our input string as char array.
        NekoCString::TokenizeString( (char*)g_message );

        // Execute our commands now.
    }
   
    /**
     *  Render console and text.
     */
    void CGameConsole::Render( void )
    {
        
    }

    /**
     *  Write to the log file.
     */
    void CGameConsole::WriteLogToFile( const char * msg )
    {
        if ( !logFile ) {
            return;
        }

        // Gosh.
        if( !msg ) {
            return;
        }

        fprintf( logFile, "%s\n", msg );
    }


    /**
     *  Clear console, external and internal.
     */
    void CGameConsole::Clear( void )
    {
        uint32_t h = 0, j = 0;

        // Write a log piece to the log file.
        if( Filesystem_Logging->Get<bool>() == true ) {
            for( j = 0; j < logFill; j++ ) {
                WriteLogToFile( Log[j] );
            }
        }

        // Clean the console buffer.
        for( h = 0; h < 256; ++h ) {
            for( j = 0; j < 512; ++j ) {
                Log[h][j] = '\0';
            }
        }

        logCount = 1;
        logFill = 0;

        //if( c_CommandManager->ArgCount() > 1 ) {
        //    #ifdef _WIN32 // Clear Windows console.
        //        SendMessage( wConsole->hwndBuffer, EM_SETSEL, 0, -1 );
        //        SendMessage( wConsole->hwndBuffer, EM_REPLACESEL, FALSE, ( LPARAM ) "" );
        //        UpdateWindow( wConsole->hwndBuffer );
        //    #endif // _WIN32
        //}
    }

    /**
     *  Find a console variable.
     */
    SConsoleVar * CGameConsole::FindCVar( const char *c_name )
    {
        SConsoleVar *tmpVar = m_CVarCache[c_name];
        
        if( tmpVar == NEKO_NULL ) {
            return NEKO_NULL;
        } else {
            return tmpVar;
        }
    }

    /**
     *  Register a console variable.
     */
    SConsoleVar * CGameConsole::RegisterCVar( ECvarGroup group, const char * name, const char * desc, const char * value, CVFlag flag, ECvarType type )
    {
        SConsoleVar * var = (SConsoleVar *)m_pCvarAllocator->Alloc( sizeof(SConsoleVar) );
        var->RegisterCVar( group, name, desc, value, flag, type );
        m_CVarCache[name] = var;
        
        return var;
    }
    
    /**
     *  Register a console variable.
     */
    SConsoleVar * CGameConsole::RegisterCVar( ECvarGroup group, const char * name, const char * desc, const float value, CVFlag flag, ECvarType type )
    {
        SConsoleVar * var = (SConsoleVar *)m_pCvarAllocator->Alloc( sizeof(SConsoleVar) );
        var->RegisterCVar( group, name, desc, value, flag, type );
        m_CVarCache[name] = var;
        
        return var;
    }
    
    /**
     *  Register a console variable.
     */
    SConsoleVar * CGameConsole::RegisterCVar( ECvarGroup group, const char * name, const char * desc, const int32_t value, CVFlag flag, ECvarType type )
    {
        SConsoleVar * var = (SConsoleVar *)m_pCvarAllocator->Alloc( sizeof(SConsoleVar) );
        var->RegisterCVar( group, name, desc, value, flag, type );
        m_CVarCache[name] = var;
        
        return var;
    }

    const static uint64_t kCvarCacheSize = Megabyte( 4 );
    
    /**
     *  Initialize console.
     */
    void CGameConsole::Initialize( void )
    {
        uint32_t h, j;
        
        // Initialize console buffer - looks ugly but it's okaaay.
        for(  h = 0; h < CONSOLELOG_LINES; h++ ) {
            for( j = 0; j < CONSOLELOG_LINE_CHAR; j++ ) {
                Log[h][j] = '\0'; // Clear console text ( [line][character] )
            }
        }
        
        // Some initial values.
        logCount    = 0;
        logFill     = 0;
        logErrors   = 0;
        logWarnings = 0;
        
        Print( LOG_NONE, "\n" );
        Print( LOG_INFO, "Game console initialized...\n" );
    }

    /**
     *  Create CVar cache.
     */
    void CGameConsole::CreateCache()
    {
        pAllocatorHandle = pMainAllocProxy;
        m_pCvarAllocator = NekoAllocator::newPoolAllocator( sizeof(SConsoleVar), __alignof(SConsoleVar), kCvarCacheSize, *pMainAllocProxy );
        
        
        if( !m_CVarCache.Create( MAX_CONSOLEVARIABLES, m_pCvarAllocator, pMainAllocProxy ) ) {
            Error( ERR_FATAL, "Could not create console variable cache with %i CVars.\n", MAX_CONSOLEVARIABLES );
            return;
        }
    }

    /**
     *  Console key listener.
     */
    void CGameConsole::KeyInput( char key )
    {

    }

    /**
     *  Check if logging enabled.
     */
    void CGameConsole::CheckForLogging()
    {
        /* Check file logging. */
        if( Filesystem_Logging->Get<bool>() == true ) {
            //g_Core->p_Console->Print( LOG_INFO, "Checking log file availability..\n" );
            if( !( logFile = CFileSystem::OpenWrite( NC_TEXT("%s/%s", Filesystem_Path->Get<const char*>(), Filesystem_Log->Get<const char*>() ) ) ) ) {
                g_Core->p_Console->Print( LOG_ERROR, "Couldn't write or create file for logging. Please check permissions.\n" );
                g_Core->p_Console->Print( LOG_INFO, "Logging to file is disabled.\n");
                g_Core->p_Console->Print( LOG_NONE, "\n");

                Filesystem_Logging->Set( 0 );
                Filesystem_Logging->Lock();
            } else {
                g_Core->p_Console->Print( LOG_INFO, "Logging to \"%s\".\n", Filesystem_Log->Get<const char*>() );
            }
        } else {
            g_Core->p_Console->Print( LOG_INFO, "File logging is disabled.\n" );
        }
    }

    /**
     *
     *  Some another methods.
     *
     */
    
    /**
     *  Get current console log line count.
     */
    const uint32_t CGameConsole::GetLogLineCount( void ) const
    {
        return logCount;
    }

    /**
     *  Log line character fill.
     */
    const uint32_t CGameConsole::GetLogFillCount( void ) const
    {
        return logFill;
    }

    /**
     *  Log file handle.
     */
    FILE * CGameConsole::GetLogFile( void )
    {
        return logFile;
    }

    /**
     * Get frame rates per second.
     */
#define	FPS_FRAMES	4
    int CGameConsole::GetFramerate()
    {
        static int32_t	previousTimes[FPS_FRAMES];
        static int32_t	index;
        static int32_t	previous;
        int32_t         t, frameTime;
        int32_t         i, total;
        int32_t         fps = 0;

        t = (float)g_Core->p_System->Milliseconds();
        frameTime = t - previous;
        previous = t;

        previousTimes[index % FPS_FRAMES] = frameTime;
        ++index;
        
        if ( index > FPS_FRAMES )
        {
            // average multiple frames together to smooth changes out a bit
            total = 0;
            for ( i = 0 ; i < FPS_FRAMES ; ++i )
            {
                total += previousTimes[i];
            }
            
            if ( !total )
            {
                total = 1;
            }
            
            fps = 1000 * FPS_FRAMES / total;
            return fps;
        }

        return fps;
    }

    /**
     *  Add text to console window.
     */
    void CGameConsole::PrintExternal( const char * msg )
    {
        static char buffer[MAX_SPRINTF_BUFFER * 2];
        char * b = buffer;

        int32_t i = 0;

        // Fix next line symbols.
        while ( msg[i] && ( ( b - buffer ) < (sizeof( buffer ) - 1) ) )
        {
            if ( msg[i] == '\n' )
            {
                b[0] = '\r'; b[1] = '\n';
                b += 2;
            }
            else if ( msg[i] == '\r' )
            {
                b[0] = '\r';
                b[1] = '\n';
                b += 2;
            }
            else if ( msg[i] == '\n' )
            {
                b[0] = '\r';
                b[1] = '\n';
                b += 2;
            }
            else
            {
                *b= msg[i];
                b++;
            }
            ++i;
        }
        *b = 0;
        
        /* this is slooow */
#ifdef _WIN32
        SendMessage( wConsole->hwndBuffer, EM_LINESCROLL, 0, 0xffff );
        SendMessage( wConsole->hwndBuffer, EM_SCROLLCARET, 0, 0 );
        SendMessage( wConsole->hwndBuffer, EM_REPLACESEL, 0, (LPARAM) buffer );
#else
        /*
         TODO something.
        */
#endif
    }

    /**
     *
     *      Common stuff.
     *
     */
    

    /**
     *  Error.
     */
    void CGameConsole::Error( EErrorType err, const char * msg, ... )
    {
        if( ( strlen(msg) < 1 || !msg ) )   // Check for message data.
        {
            g_Core->p_Console->Print( LOG_DEVELOPER, "No error message given.\n" );
            msg = "No reason";
        }

        va_list argptr;
        static char text[MAX_ERROR_TEXT_LENGTH];

        // Argument parsing.
        va_start( argptr, msg );
        vsnprintf( (char*)text, sizeof(text), msg, argptr );
        va_end( argptr );

        // Hide window and show the message box.
        CSystem::HideWindow();
        CSystem::MessageBox( MB_ERROR, "Error ;(", /*NC_TEXT( "%s(errorcode: 0x%x)\n%s", */text/*, err, ERROR_REPORT )*/ );

        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_NONE, "Oops, errors were made:\n" );
        g_Core->p_Console->Print( LOG_NONE, "%s (error code: 0x%x) :(\n", text, err );
        g_Core->p_Console->Print( LOG_NONE, "\n" );

        g_Core->p_Console->Print( LOG_NONE, "\n" );

        /***************
         * I love arts. :P
         ***************/
        const static int32_t lines = 12;
        const char * oh_no[lines]=
        {

            "                                           \n",
            "        :\\     /;                _        \n",
            "       ;  \\___/  ;              ; ; Oops..\n",
            "      ,:-\"'   `\"-:.            / ;       \n",
            " _   /,---.   ,---.\\   _     _; /         \n",
            " _:>((  |  ) (  |  ))<:_ ,-""_,\"          \n",
            "     \\`````   `````/""""\",-\"            \n",
            "      '-.._ v _..-'      )                 \n",
            "        / ___   ____,..  \\                \n",
            "       / /   | |   | ( \\. \\              \n",
            "      / /    | |    | |  \\ \\             \n",
            "      `\"     `\"     `\"    `\"           \n"
        };

        uint32_t i;
        for( i = 0; i < lines; ++i )
        {
            g_Core->p_Console->Print( LOG_NONE, oh_no[i] );
        }

        g_Core->p_Console->Print( LOG_NONE, "Bye! :>\n" );

        // Shutdown the game now.
        g_Core->Quit( NC_TEXT( "Error - 0x%i\n", err ) );
    }

    void CGameConsole::Error( const char * msg, ... ) {
        va_list argptr;
        static char text[MAX_ERROR_TEXT_LENGTH];
        
        // Argument parsing.
        va_start( argptr, msg );
        vsnprintf( (char*)text, sizeof(text), msg, argptr );
        va_end( argptr );
        
        Error( ERR_FATAL, text );
    }
    
    /**
     *  Append text to log.
     */
    void CGameConsole::AppendText( const char * text )
    {
        ++logCount;
        
        if( logCount > MAX_CONSOLE_LOG_LINES )
        {
//            Execute( "clear ext" );
        }

        // External console.
#ifdef SHOW_CONSOLE_LINE_PREFIX
        fputs( text /* NC_TEXT("%s%s", logtype, text) */ , stdout );
        PrintExternal( /*NC_TEXT("%s%s", logtype,*/ text /*)*/ );
#else
        fputs( text, stdout );
        PrintExternal( text );
#endif
        
        // Log will be written to the file on console clear.
        WriteLogToFile( text );
        
        fflush( stdout );
    }

    
    /**
     *  Print to console and to the log file if necessary.
     */
    void CGameConsole::Print( const char * message )
    {
        g_Core->p_Console->Print( LOG_INFO, message );
    }

    void CGameConsole::Print( ELogType type, const char * msg, ... )
    {
        if( msg == NEKO_NULL )
        {
            return;
        }

        va_list argptr;
        static char text[MAX_SPRINTF_BUFFER];

        // Argument parsing.
        va_start( argptr, msg );
        vsnprintf( text, sizeof(text), msg, argptr );
        va_end( argptr );

        /*
         const char * logtype;

         switch( type ) {
         case LOG_DEVELOPER:
         logtype = "[DEBUG]: ";
         break;
         case LOG_ERROR:
         logtype = "[ERROR]: ";
         ++g_Core->p_Console->logErrors;
         break;
         case LOG_INFO:
         logtype = "[INFO]: ";
         break;
         case LOG_NONE:
         logtype = "";
         break;
         case LOG_WARN:
         logtype = "[WARNING]: ";
         ++g_Core->p_Console->logWarnings;
         break;
         default:
         logtype = "[UNKNOWN]: ";
         break;
         }
         */

        AppendText( text );
    }
}
