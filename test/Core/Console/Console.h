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
//  Console.h
//  Console manager.. ^.^
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef console_h
#define console_h

#include "../../Platform/Shared/SystemShared.h"
#include "../Core.h" // Non-copyable define.
#include "../CoreDef.h"
#include "../String/String.h"
#include "../String/StringHelper.h"
#include "../Utilities/Hashtable.h"
#include "ConsoleVariable.h"
#include "GameMemory.h"


namespace Neko {

    class SConsoleVar;

    //! Maximum *in-game* console log lines to render.
    static const int32_t MAX_CONSOLE_LOG_LINES = 36;
    static const int32_t CONSOLE_LINE_SKIP  = 11;

    static const int32_t CONSOLELOG_LINES = 512;
    static const int32_t CONSOLELOG_LINE_CHAR = 512;

    static const int32_t MAX_COMMAND_SIZE = 32;        // Max console command length.

    // Maximum console command tokens.
    static const int32_t MAX_COMMAND_TOKENS = 5;
    
    static const int32_t MAX_ERROR_TEXT_LENGTH = 1024;
    
    ///  Error types.
    enum EErrorType
    {
        ERR_FATAL   = 0,            // Fatal error ( probably unknown ).
        ERR_NETWORK,                // Network errors.
        ERR_SCRIPT,                 // Script errors.
        ERR_OPENGL,                 // Graphics system error.
        ERR_FILESYSTEM,             // File system error.
        ERR_ASSET,                  // Missing asset, corrupted asset etc.
        ERR_SERVER,                 // Server faults.
        ERR_CLIENT,                 // Client error.
        ERR_SYSTEM                  // System stuff error.
    };

    /// Log types.
    enum ELogType
    {
        // No prefix to use. Used for next line print.
        LOG_NONE    = 0,
        // Script call.
        LOG_SCRIPT,
        // Warning.
        LOG_WARN,
        // Error.
        LOG_ERROR,
        // Debug, shows only when 'log_verbose' is '1'.
        LOG_DEVELOPER,
        // NC_ASSERT
        LOG_ASSERT,
        LOG_INFO                // Just an info.
    };

    ///  Game console.
    class CGameConsole
    {
        NEKO_NONCOPYABLE( CGameConsole );
        
    public:
        CGameConsole();
        ~CGameConsole();

        /**
         *  Execute a command or CVar.
         */
        void                    Execute( const char * msg, ... );     // Execute a command.
        
        /**
         *  Render console interface.
         */
        void                    Render( void );                        // Render console UI.

        /**
         *  Clear external and internal consoles.
         */
        void                    Clear( void );                         // Clear *both* consoles.
        
        /**
         *  Key handler.
         */
        void                    KeyInput( char key );                  // Key input for input line.

        /**
         *  Initialize game console.
         */
        void                    Initialize( void );                    // Initialize console.
        
        /**
         *  Create CVar cache.
         */
        void                    CreateCache();
        
        /**
         *  Print to external console.
         */
        void                    PrintExternal( const char * msg );    // Print to external console.

        /**
         *  Register console variable.
         */
        SConsoleVar *                    RegisterCVar( ECvarGroup group, const char * name, const char * desc, const char * value, CVFlag flag, ECvarType type );
        
        /**
         *  Register console variable.
         */
        SConsoleVar *                    RegisterCVar( ECvarGroup group, const char * name, const char * desc, const float value, CVFlag flag, ECvarType type );
        
        
        /**
         *  Register console variable.
         */
        SConsoleVar *                    RegisterCVar( ECvarGroup group, const char * name, const char * desc, const int32_t value, CVFlag flag, ECvarType type );
        
        
        /**
         *  Load Ini file.
         */
        void                    LoadIni( const char *name );
        
        /**
         *  Write cvars to Ini file.
         */
        void                WriteIni( const char * name );
        
        /**
         *  Shutdown the game console.
         */
        void                    Shutdown();
        
        
        int                     GetFramerate();

        /**
         *  Is console shown?
         */
        inline bool                 IsShown();
        
        /**
         *  Switch internal console visibility.
         */
        inline void                 SwitchVisibility();
        
        /**
         *  Set console visibility.
         */
        inline void                 SetVisible( bool visible );

        /**
         *  Fatal error.
         */
        void                Error( EErrorType err, const char * msg, ... );
        void                Error( const char * msg, ... );

        /**
         *  Print to console(s).
         */
        void                Print( const char * msg );
        
        /**
         *  Print to console(s) with log type ( none, info, error, warning, etc.. ).
         */
        void                    Print( ELogType type, char const *msg, ... );

        /**
         *  Find console variable by name.
         */
        SConsoleVar *                   FindCVar( const char *c_name );

        /**
         *  Write log to file ( if allowed to ).
         */
        void                    WriteLogToFile( const char * msg );

        inline const unsigned int                   GetLogLineCount() const;
        inline const unsigned int                   GetLogFillCount() const;

        void                    CheckForLogging();

        
        /**
         *  Get log file.
         */
        FILE    *               GetLogFile( void );

        // Fix me: to do more?
        char        Buffer[MAX_COMMAND_SIZE];

        // Internal console log. [lines][characters]
        char        Log[CONSOLELOG_LINES][CONSOLELOG_LINE_CHAR]; // Characters per line.

        // Last executed buffer ( without the command itself ).
        char        lastBuffer[MAX_COMMAND_TOKENS - 1][MAX_COMMAND_SIZE];

        int32_t         logWarnings;    // Log warnings count.
        int32_t         logErrors;      // Log errors count.
        int32_t         logCount;       // Log total lines count.
        int32_t         logFill;

        /**
         *   Add text to log data.
         */
        inline void             AppendText( const char * text );
 
        
        //! Console variable memory pool.
        INekoAllocator      * pAllocatorHandle;
        INekoAllocator      * m_pCvarAllocator;
        
    private:
        //! Is console visible?
        bool        isShown;

        //! Log file to write our log.
        FILE        *logFile;           // Log is written while *Clean* command is called
        // ( what means we write log part not per line ).

        //! Usually it's "prefix > command buffer".
        CStr       _Prefix;

        //! Hashmap to store console variables.
        CHashMap<const char*, SConsoleVar*>   m_CVarCache;
        
    protected:
    };

}
#endif
