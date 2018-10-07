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
//  Scripting.h
//  Script-engine implementation. :*
//
//  Created by Neko Code on 2/2/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__Scripting__
#define __Nanocat__Scripting__

#include "../Core.h" // non-copyable define
#include "../../Platform/Shared/SystemShared.h"
#include "../Utilities/Hashtable.h"
#include "../String/String.h"

#if defined( USES_LUA )

// Lua specific libraries.
extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
    #include "luaconf.h"
}

// Replace me with something... um, custom?
//#include <string>
//#include <map>
// Replaced by CHashMap and CStr.

// using namespace std;

//typedef int (*SCRIPT_API)( lua_State * L);
#define SCRIPT( N ) int N( lua_State * L )

namespace Neko {
  
    static const int MAX_LUA_STACK_FUNCTIONS = 512;
    static const int MAX_LUA_SCRIPTS = 512;
    
    /**
     *  Script functions to use in Lua files.
     **/
    namespace ncScriptFunction {
        // Core functions.
        SCRIPT( PrintLn );
        SCRIPT( Error );
        SCRIPT( Warn );
        SCRIPT( Assert );
        SCRIPT( MidError );
        SCRIPT( GetGameTime );
        SCRIPT( RegisterFunction ); // Allow user to add custom functions.
        
        // Interaction.
        SCRIPT( PlaySound );
        
        // Camera.
        SCRIPT( GetOriginX );
        SCRIPT( GetOriginY );
        SCRIPT( GetOriginZ );
        
        // Cvars.
        SCRIPT( GetCvarValue );
        SCRIPT( RegisterCVar );
        // CVar value setters.
        SCRIPT( SetCVarInt );
        SCRIPT( SetCVarBool );
        SCRIPT( SetCVarString );
        SCRIPT( SetCVarFloat );
        
        // Math.
        SCRIPT( FastSqrtf );
        SCRIPT( LargeRandom );
        SCRIPT( FastPow );
        SCRIPT( RandomFloatAlt );
        SCRIPT( Absf );
        SCRIPT( Absi );
        SCRIPT( Absd );
        SCRIPT( Deg2Rad );
        SCRIPT( Rad2Deg );
        SCRIPT( Signi );
        SCRIPT( LerpfAlt );
        SCRIPT( Maxi );
        SCRIPT( Mini );
        SCRIPT( Maxf );
        SCRIPT( Minf );
        
        // UI.
        SCRIPT( AddStaticText2D );
        
        // Controls & input.
        SCRIPT( GetLastPressedKey );
        SCRIPT( GetLastReleasedKey );
        
        SCRIPT( GetMouseX );
        SCRIPT( GetMouseY );
    }
   
    /**
     *  Lua function base in per file
     */
    class ncLuaFunctionPool {
        NEKO_NONCOPYABLE( ncLuaFunctionPool );
        
    public:
        ncLuaFunctionPool();

    private:
        int r_funcId, r_objId; // Function and object ids.
        CStr r_name; // Function name.
        
    public:
        
        /**
         *  Precache Lua function.
         */
        void AddFunction( lua_State *L, const char *functionName );
        /**
         *  Call Lua function.
         */
        template<typename t>
        bool Call( lua_State * L, t Lparams );
        
        /**
         *  Push arguments.
         */
        template<typename... Args>
        int PushLuaArgs( lua_State *L, bool next, Args... rest );
        
        template<typename... Args>
        int PushLuaArgs( lua_State *L, const char * next, Args... rest );
        
        template<typename... Args>
        int PushLuaArgs( lua_State *L, const int next, Args... rest );
        
        template<typename... Args>
        int PushLuaArgs( lua_State *L, const double next, Args... rest );
        
        template<typename... Args>
        int PushLuaArgs( lua_State *L, const float next, Args... rest );
        
        int PushLuaArgs( lua_State *L );
        
        template<typename... Args>
        int CallLuaFunction( lua_State * L, int retvals, Args... rest );
        
        /**
         *  Push key table value.
         */
        template<typename t>
        void PushTableString( lua_State * L, const char * key, const t value );
        
        /**
         *  Get function name.
         */
        inline const CStr & GetName() {
            return r_name;
        }
    };
    
    /**
     *  Lua script.
     */
    class ncLuaScript
    {
        NEKO_NONCOPYABLE( ncLuaScript );
        
    public:
        
        ncLuaScript();
        ncLuaScript( const int maxLuaFunctions, INekoAllocator * allocator );
        
        ~ncLuaScript();
        
        // Replace me, not sure about its performance. Buut must be good.
        // DONE: Replaced by CHashMap.
        CHashMap<const char*, ncLuaFunctionPool*> m_luaFunctions;
        
        /**
         *  Precache function.
         */
        void PrecacheFunction( lua_State * L, const char * N );
        
        /**
         *  Register metatable.
         */
        void RegisterMeta( const char * metaname, const luaL_Reg * regs );
        
        /**
         *  Register constant for script.
         */
        template<typename T> void RegisterConstant( T value, const char *classname, const char *constantname );
        
        /**
         *  Get argument at index.
         */
        template<typename T> T GetArgument( int index );
        
        /**
         *  Get constant from script.
         */
        template<typename T> T GetConstant( const char *N );
        
        /**
         *  Create class table.
         */
        bool CreateClassTable( const char *classname );
        
        /**
         *  Set script Lua VM.
         */
        void SetState( lua_State * _L );
        
        /**
         *  Get current Lua VM.
         */
        inline lua_State * GetState() const {
            return L;
        }
        
        /**
         *  Get argument count.
         */
        int GetArgumentCount();
        
        /**
         *  Destroy script and properties.
         */
        void Kill();
        
    private:
        
        INekoAllocator  * pAllocator;
        
        /**
         *  Lua VM. POINTS TO THE MAIN VM!!!! IT DOES NOT CREATE A NEW ONE!!!!!!!
         */
        lua_State *L;
    };

    // - Function definitions.
    template<> void ncLuaScript::RegisterConstant<int>( const int value, const char *classname, const char *constantname );
    template<> void ncLuaScript::RegisterConstant<const char*>( const char *value, const char *classname, const char *constantname );
    template<> void ncLuaScript::RegisterConstant<bool>( bool value, const char *classname, const char *constantname );
    template<> void ncLuaScript::RegisterConstant<double>( const double value, const char *classname, const char *constantname );
    template<> void ncLuaScript::RegisterConstant<lua_CFunction>( lua_CFunction value, const char *classname, const char *constantname );
    template<> void ncLuaScript::RegisterConstant<float>( const float value, const char *classname, const char *constantname );
    
    template<> float ncLuaScript::GetConstant<float>( const char *N );
    template<> int ncLuaScript::GetConstant<int>( const char *N );
    template<> const char *ncLuaScript::GetConstant<const char*>( const char *N );
    template<> bool ncLuaScript::GetConstant<bool>(const char *N );
    template<> double ncLuaScript::GetConstant<double>( const char *N );
    // -
    
    /**
     *  Script engine.
     */
    class ncScriptBase {
        NEKO_NONCOPYABLE( ncScriptBase );
        
    public:

        ncScriptBase();
        ~ncScriptBase();
    
        /**
         *  Initialize script engine.
         */
        void                Init( INekoAllocator * allocator );
        
        /**
         *  Load some important stuff at engine load.
         */
        void LoadScripts();
        
        /**
         *  Load Lua script.
         *  Error will be thrown if there's no Lua VM initialized.
         */
        bool LoadLuaScript( const char *filename, const bool hasScriptCore = true );

        /**
         *  Get script variable.
         */
        template<typename t>
        t GetScriptVar( const char *filename, const char *variable );

        /**
         *  Execute script function without parameters.
         */
        bool ExecuteScript( lua_State *L, const char *filename, const char *function );
        bool ScriptFunction( const char *filename, const char *variable ); // Probably does the same.
        
        /**
         *  Execute function with parameters.
         */
        template<typename... Args>
        bool ExecuteScript( lua_State *L, const char *pScriptName, const char *pFunctionName, const int pArgCount, Args... rest );
        
        template<typename... Args>
        bool ScriptFunction( const char * pScriptName, const char * pFunctionName, const int pArgCount, Args... rest );
        
        /**
         *  Error handler.
         */
        int ErrorHandler( lua_State *L );
        
        /**
         *  Returns common Lua state.
         */
        inline lua_State *GetGlobalLuaState() {
            return _global_state;
        }
        
        /**
         *  Shutdown script system.
         */
        void Shutdown();
        
        /**
         *  Register new class object.
         */
        void RegisterClassObject( lua_State * L, const char *metaname, const luaL_Reg * regs );
        
    private:
        
        INekoAllocator      * pAllocator;
        
        /**
         *  Global Lua common state.
         */
        lua_State   *_global_state;
        
        /**
         *  Loaded Lua files.
         */
        CHashMap<const char*, ncLuaScript*> m_luaCache;
        
    protected:
    };
    
    // - Function definitions.
    template<> int ncScriptBase::GetScriptVar<int>( const char *filename, const char *variable );
    template<> float ncScriptBase::GetScriptVar<float>( const char *filename, const char *variable );
    template<> bool ncScriptBase::GetScriptVar<bool>( const char *filename, const char *variable );
    template<> const char * ncScriptBase::GetScriptVar<const char*>( const char *filename, const char *variable );
    // - 
    
}

#endif

#endif /* defined(__Nanocat__Scripting__) */
