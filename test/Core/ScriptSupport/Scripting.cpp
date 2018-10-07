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
//  Scripting.cpp
//  Script engine implementation. :*
//
//  Created by Neko Code on 2/2/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#include "Scripting.h"
#include "../String/StringHelper.h"
#include "../Core.h"
#include "../Player/Camera/Camera.h" // API
#include "../Console/Console.h"
#include "../UserInterface/UserMenu.h" // API
#include "../Player/Input/Input.h"      // API
#include "../../AssetCommon/FileSystem.h" // File loading.
#include "../../World/BeautifulEnvironment.h" // API
#include "../../AssetCommon/Sound/SoundManager.h"   // API
#include "../../World/Mesh.h"       // API
#include "../../World/ParticleEngine.h" // API
#include "../Network/MultiplayerServer.h"

#if defined( USES_LUA )

#include "../../../Libraries/LuaUtilities.h"   // lua_tonumber
// Use luamath, luastring, luaio etc?
//#define _USE_LUAOPEN_

// This system opens more lands to this game engine, literally.
// User is able to create everything in Lua files without editing the game engine itself.

// TODO: Neko language interpreter ( currently in the progress, standalone ).
// Only Lua is supported now.

namespace Neko {

    /**
      * This namespace holds Lua Script
      * functions to be integrated in its script.
      * SCRIPT method returns Integer as Lua requests.
     **/
    namespace ncScriptFunction {
        /**
         *  Print simple console text message.
         */
        SCRIPT( PrintLn ) {
            g_Core->p_Console->Print( LOG_SCRIPT, "%s\n", lua_tostring( L, -1 ) );
            return 1;
        }

        /**
         *  Get gametime.
         */
        SCRIPT( GetGameTime ) {
            const int _time = g_Core->GetTime();
            lua_pushinteger( L, _time );

            return 1;
        }

        /**
         *  Error.
         */
        SCRIPT( Error ) {
            const char *_errorText = lua_tostring( L, -1 );
            g_Core->p_Console->Error( ERR_SCRIPT, "Script initiated error: %s.", _errorText );
            return 1;
        }

        /**
         *  Register function.
         */
        SCRIPT( RegisterFunction ) {

            return 1;
        }

        /**
         *  Print all function.
         */
        SCRIPT( PrintAllFunctionNames ) {
            //int i;

            //g_Core->p_ScriptBase->DumpAllFunctions();
            return 1;
        }

        /**
         *  Console variable stuff.
         */
        SCRIPT( GetCvarValue ) {
            const char *m_cmdName = lua_tostring( L, -1 );
            lua_pushstring( L, g_Core->p_Console->FindCVar( m_cmdName )->Get<const char*>() );

            return 1;
        }

        /**
         *  Create and register CVar.
         */
        SCRIPT( RegisterCVar ) {
            const char *m_cmdName = lua_tostring( L, -1 );
            const char *m_cmdDesc = lua_tostring( L, -2 );
            const char *m_defaultValue = lua_tostring( L, -3 );

            SConsoleVar *tmp = new SConsoleVar( m_cmdName, m_cmdDesc, m_defaultValue, CVFlag::None );
            g_Core->p_Console->RegisterCVar( *tmp );

            return 1;
        }

        /**
         *
         *  Set CVar value.
         *
         */
        SCRIPT( SetCVarInt ) { // Integer.
            const char *m_cmdName = lua_tostring( L, -2 );
            const int m_cmdValue = (int)lua_tointeger( L, -1 );

            SConsoleVar *var = g_Core->p_Console->FindCVar( m_cmdName );
            if( var ) {
                var->Set( m_cmdValue );
            }

            return 1;
        }

        SCRIPT( SetCVarString ) { // String.
            const char *m_cmdName = lua_tostring( L, -2 );
            const char *m_cmdValue = lua_tostring( L, -1 );

            SConsoleVar *var = g_Core->p_Console->FindCVar( m_cmdName );
            if( var ) {
                var->Set( m_cmdValue );
            }

            return 1;
        }

        SCRIPT( SetCVarBool ) { // Boolean.
            const char *m_cmdName = lua_tostring( L, -2 );
            const bool m_cmdValue = lua_toboolean( L, -1 );

            SConsoleVar *var = g_Core->p_Console->FindCVar( m_cmdName );
            if( var ) {
                var->Set( m_cmdValue );
            }


            return 1;
        }

        SCRIPT( SetCVarFloat ) { // Float.
            const char *m_cmdName = lua_tostring( L, -2 );
            const float m_cmdValue = (float)lua_tonumber( L, -1 );

            SConsoleVar *var = g_Core->p_Console->FindCVar( m_cmdName );
            if( var ) {
                var->Set( m_cmdValue );
            }

            return 1;
        }

        /**
         *
         * Client stuff.
         *
         */
        SCRIPT( GetOriginX ){
            lua_pushnumber( L, static_cast<double>( g_Core->p_Camera->vEye.x ) );

            return 1;
        }

        SCRIPT( GetOriginY ){
            lua_pushnumber( L, static_cast<double>( g_Core->p_Camera->vEye.y ) );

            return 1;
        }

        SCRIPT( GetOriginZ ){
            lua_pushnumber( L, static_cast<double>( g_Core->p_Camera->vEye.z ) );

            return 1;
        }

        /**
         * 
         *  Math.
         *
         */
        SCRIPT( FastSqrtf ) {
            const float m_argParam = static_cast<float>( lua_tonumber( L, -1 ) );
            lua_pushnumber( L, (float)nkMath::FastSqrty(m_argParam) );

            return 1;
        }

        SCRIPT( LargeRandom ) {
            const unsigned long m_argParam = nkMath::LargeRandom();

            lua_pushnumber( L, static_cast<unsigned long>(m_argParam) );

            return 1;
        }

        SCRIPT( FastPow ) {
            const float m_argParam01 = static_cast<int>( lua_tonumber( L, -1 ) );
            const float m_argParam02 = static_cast<int>( lua_tonumber( L, -2 ) );

            const float m_argVal = nkMath::FastPow( m_argParam01, m_argParam02 );

            lua_pushnumber( L, static_cast<int>( m_argVal ) );

            return 1;
        }

        SCRIPT( Absf ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            
            const float m_argVal = nkMath::Abs( m_argParam01 );
            
            lua_pushnumber( L, static_cast<float>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Absi ) {
            int m_argParam01 = static_cast<int>( lua_tonumber( L, -1 ) );
            
            const int m_argVal = nkMath::Abs( m_argParam01 );
            
            lua_pushnumber( L, static_cast<int>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Deg2Rad ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            
            const float m_argVal = nkMath::DEGTORAD( m_argParam01 );
            
            lua_pushnumber( L, static_cast<float>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Rad2Deg ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            
            const float m_argVal = nkMath::RADTODEG( m_argParam01 );
            
            lua_pushnumber( L, static_cast<float>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Signi ) {
            int m_argParam01 = static_cast<int>( lua_tonumber( L, -1 ) );
            
            const int m_argVal = nkMath::Sign( m_argParam01 );
            
            lua_pushnumber( L, static_cast<int>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( LerpfAlt ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            float m_argParam02 = static_cast<float>( lua_tonumber( L, -2 ) );
            float m_argParam03 = static_cast<float>( lua_tonumber( L, -3 ) );
            
            const float m_argVal = nkMath::LerpfAlt( m_argParam01, m_argParam02, m_argParam03 );
            
            lua_pushnumber( L, static_cast<float>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Maxf ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            float m_argParam02 = static_cast<float>( lua_tonumber( L, -2 ) );
            
            const float m_argVal = nkMath::Max( m_argParam01, m_argParam02 );
            
            lua_pushnumber( L, static_cast<float>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Minf ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            float m_argParam02 = static_cast<float>( lua_tonumber( L, -2 ) );
            
            const float m_argVal = nkMath::Min( m_argParam01, m_argParam02 );
            
            lua_pushnumber( L, static_cast<float>( m_argVal ) );
            
            return 1;
        }
        
        
        SCRIPT( Maxi ) {
            int m_argParam01 = static_cast<int>( lua_tonumber( L, -1 ) );
            int m_argParam02 = static_cast<int>( lua_tonumber( L, -2 ) );
            
            const int m_argVal = nkMath::Max( m_argParam01, m_argParam02 );
            
            lua_pushnumber( L, static_cast<int>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( Mini ) {
            int m_argParam01 = static_cast<int>( lua_tonumber( L, -1 ) );
            int m_argParam02 = static_cast<int>( lua_tonumber( L, -2 ) );
            
            const int m_argVal = nkMath::Min( m_argParam01, m_argParam02 );
            
            lua_pushnumber( L, static_cast<int>( m_argVal ) );
            
            return 1;
        }
        
        SCRIPT( RandomFloatAlt ) {
            float m_argParam01 = static_cast<float>( lua_tonumber( L, -1 ) );
            float m_argParam02 = static_cast<float>( lua_tonumber( L, -2 ) );

            const float m_argVal = nkMath::RandFloatAlt( m_argParam01, m_argParam02 );

            lua_pushnumber( L, static_cast<float>( m_argVal ) );

            return 1;
        }

        /**
         *  UI.
         */
        SCRIPT( AddStaticText2D ) {

            /*
            const int _tX = (int)lua_tointeger( L, -3 );
            const int _tY = (int)lua_tointeger( L, -2 );
            const int _tScale = (int)lua_tointeger( L, -1 );

            static UIText2D _t( _tX, _tY, _tScale );

            ui_Base->Add( t )
            */

            return 1;
        }

        /**
         *
         *  Input & controls.
         *
         */
        SCRIPT( GetLastPressedKey ) {
            lua_pushstring( L, NC_TEXT( "%c", g_Core->p_Input->GetLastPressedKey() ) );

            return 1;
        }

        SCRIPT( GetLastReleasedKey ) {
            lua_pushstring( L, NC_TEXT( "%c", g_Core->p_Input->GetLastReleasedKey() ) );

            return 1;
        }

        // Get mouse coordinates.
        SCRIPT( GetMouseX ) {
            lua_pushinteger( L, g_Core->p_Input->GetMouseX() );

            return 1;
        }

        SCRIPT( GetMouseY ) {
            lua_pushinteger( L, g_Core->p_Input->GetMouseY() );

            return 1;
        }
    }

    /**
     *  This namespace holds all Lua class objects.
     */
    namespace ncScriptClassObjects {

#ifndef NEKO_SERVER
        /**
         * 
         *  PARTICLE MANAGER
         *
         */
        SGFXParticle * fx_getstruct( lua_State * l, int n )
        {
            // This checks that the argument is a userdata
            // with the metatable "luaL_Foo"
            return *(SGFXParticle **)luaL_checkudata( l, n, "luaL_fx" );
        }
        
        int fx_new( lua_State * L ) {
            
//            const char *soundName = lua_tostring( L, 1 );
//            
//            // Allocate data.
//            SGFXParticle ** udata = (SGFXParticle **)lua_newuserdata(L, sizeof(SGFXParticle *));
//            *udata = (SGFXParticle*)AllocMemory( &scriptMemoryPool, sizeof(SGFXParticle) );// new ncGFXParticle();
//            
//            // Effectively, this metatable is not accessible by Lua by default.
//            luaL_getmetatable(L, "luaL_fx");
//            
//            lua_setmetatable(L, -2);
//            
//            // We return 1 so Lua callsite will get the user data and
//            // Lua will clean the stack after that.
//            g_Core->p_Console->Print( LOG_INFO, "fx:prepare - created new particle fx \"%s\"\n", soundName );
            
            return 1;
        }
        
        int fx_gc( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            delete s;
            
            return 0;
        }
        
        int fx_setpos( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float x = static_cast<float>( lua_tonumber( L, 2 ) );
            float y = static_cast<float>( lua_tonumber( L, 3 ) );
            float z = static_cast<float>( lua_tonumber( L, 4 ) );
            
            s->SetOrigin( Vec3(x, y, z ) );
            
            return 1;
        }
        
        int fx_setgravity( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float x = static_cast<float>( lua_tonumber( L, 2 ) );
            float y = static_cast<float>( lua_tonumber( L, 3 ) );
            float z = static_cast<float>( lua_tonumber( L, 4 ) );
            
            s->SetGravity( Vec3(x, y, z ) );
            
            return 1;
        }
        
        int fx_setvelocity( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float x = static_cast<float>( lua_tonumber( L, 2 ) );
            float y = static_cast<float>( lua_tonumber( L, 3 ) );
            float z = static_cast<float>( lua_tonumber( L, 4 ) );
            
            float x1 = static_cast<float>( lua_tonumber( L, 5 ) );
            float y1 = static_cast<float>( lua_tonumber( L, 6 ) );
            float z1 = static_cast<float>( lua_tonumber( L, 7 ) );
            
            s->SetVelocity( Vec3( x, y, z ), Vec3( x1, y1, z1 ) );
            
            return 1;
        }
        
        int fx_setsize( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float size = static_cast<float>( lua_tonumber( L, 2 ) );
            
            s->SetSize( size );
            
            return  1;
        }
        
        int fx_setlifetime( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float t1 = static_cast<float>( lua_tonumber( L, 2 ) );
            float t2 = static_cast<float>( lua_tonumber( L, 3 ) );
            
            s->SetLifeTime( t1, t2 );
            
            return 1;
        }
        
        int fx_setcolor( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float r = static_cast<float>( lua_tonumber( L, 2 ) );
            float g = static_cast<float>( lua_tonumber( L, 3 ) );
            float b = static_cast<float>( lua_tonumber( L, 4 ) );
            
            s->SetColor( Vec3( r, g, b ) );
            
            return 1;
        }
        
        int fx_settime( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            float time = static_cast<float>( lua_tonumber( L, 3 ) );
            
            s->SetGenTime( time );
            
            return 1;
        }
        
        int fx_setamount( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            
            int amount = static_cast<int>( lua_tonumber( L, 2 ) );

            s->SetAmount( amount );
            
            return 1;
        }
        
        int fx_spawn( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            if( !s ) {
                g_Core->p_Console->Error( ERR_FATAL, "Scripting::fx_spawn error, struct was not set." );
                return 0;
            }
            
            g_pbEnv->m_pParticleEngine->AddParticle( s );
            
            return 1;
        }
        
        int fx_init( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            s->Init();
            
            return 1;
        }
        
        int fx_initspawn( lua_State * L ) {
            SGFXParticle * s = fx_getstruct( L, 1 );
            s->Init();
            // Should look cutier somehow later.
            float px = static_cast<float>( lua_tonumber( L, 2 ) );
            float py = static_cast<float>( lua_tonumber( L, 3 ) );
            float pz = static_cast<float>( lua_tonumber( L, 4 ) );
            
            float vxm = static_cast<float>( lua_tonumber( L, 5 ) );
            float vym = static_cast<float>( lua_tonumber( L, 6 ) );
            float vzm = static_cast<float>( lua_tonumber( L, 7 ) );
            
            float vxa = static_cast<float>( lua_tonumber( L, 8 ) );
            float vya = static_cast<float>( lua_tonumber( L, 9 ) );
            float vza = static_cast<float>( lua_tonumber( L, 10 ) );
            
            float gx = static_cast<float>( lua_tonumber( L, 11 ) );
            float gy = static_cast<float>( lua_tonumber( L, 12 ) );
            float gz = static_cast<float>( lua_tonumber( L, 13 ) );
            
            float cx = static_cast<float>( lua_tonumber( L, 14 ) );
            float cy = static_cast<float>( lua_tonumber( L, 15 ) );
            float cz = static_cast<float>( lua_tonumber( L, 16 ) );
            
            float lifeMin = static_cast<float>( lua_tonumber( L, 17 ) );
            float lifeMax = static_cast<float>( lua_tonumber( L, 18 ) );
            
            float size = static_cast<float>( lua_tonumber( L, 19 ) );
            
            float period = static_cast<float>( lua_tonumber( L, 20 ) );
            
            int togenerate = static_cast<int>( lua_tonumber( L, 21 ) );
            
            s->SetProperties( Vec3( px, py, pz ),
                             Vec3( vxm, vym, vzm ),
                             Vec3( vxa, vya, vza ),
                             Vec3( gx, gy, gz ),
                             Vec3( cx, cy, cz ),
                             lifeMin, lifeMax, size, period, togenerate );

            return 1;
        }

        /**
         *  
         *  Sound class object.
         *
         */
        CSoundEnt * soundfx_getstruct( lua_State * l, int n )
        {
            // This checks that the argument is a userdata
            // with the metatable "luaL_Foo"
            return *(CSoundEnt **)luaL_checkudata( l, n, "luaL_sfx" );
        }

        int soundfx_new( lua_State * L ) {

//            const char *soundName = lua_tostring( L, 1 );
//            CSound *soundWeNeed = g_Core->p_SoundSystem->FindSound( soundName );
//            if( !soundWeNeed ) {
//                g_Core->p_Console->Print( LOG_WARN, "sfx:prepare() - Couldn't find %s sound.\n", soundName );
//                return 1;
//            }
//
//            // Allocate data.
//            CSoundEnt ** udata = (CSoundEnt **)lua_newuserdata(L, sizeof(CSoundEnt *));
//            *udata = (CSoundEnt*)AllocMemory( &scriptMemoryPool, sizeof(CSoundEnt) );// new ncALSound( *soundWeNeed );
//            
//            // Effectively, this metatable is not accessible by Lua by default.
//            luaL_getmetatable(L, "luaL_sfx");
//
//            lua_setmetatable(L, -2);
//
//            // We return 1 so Lua callsite will get the user data and
//            // Lua will clean the stack after that.
//            g_Core->p_Console->Print( LOG_INFO, "sfx:prepare - created new sfx \"%s\"\n", soundName );

            return 1;
        }

        int soundfx_gc( lua_State * L ) {
            CSoundEnt * s = soundfx_getstruct( L, 1 );
            delete s;

            return 0;
        }

        int soundfx_setorigin( lua_State * L ) {
            CSoundEnt * s = soundfx_getstruct( L, 1 );

            float x = static_cast<float>( lua_tonumber( L, 2 ) );
            float y = static_cast<float>( lua_tonumber( L, 3 ) );
            float z = static_cast<float>( lua_tonumber( L, 4 ) );

            s->m_vOrigin = Vec3( x, y, z );
//            s->SetPosition( Vec3( x, y,  z ) );

            return 1;
        }

        int soundfx_setvelocity( lua_State * L ) {
//            ncALSound * s = soundfx_getstruct( L, 1 );
//
//            float x = static_cast<float>( lua_tonumber( L, 2 ) );
//            float y = static_cast<float>( lua_tonumber( L, 3 ) );
//            float z = static_cast<float>( lua_tonumber( L, 4 ) );
//
//            s->SetVelocity( Vec3( x, y,  z ) );

            return 1;
        }

        int soundfx_setlooping( lua_State * L ) {
//            CSoundEnt * s = soundfx_getstruct( L, 1 );
//            bool isLOOPING = lua_toboolean( L, 2 );
//
//            s->SetLooping( isLOOPING );

            return 1;
        }

        int soundfx_set2d( lua_State * L ) {
//            ncALSound * s = soundfx_getstruct( L, 1 );
//            bool is2D = lua_toboolean( L, 2 );
//
//            s->SetDistanceNotCare( is2D );

            return 1;
        }

        int soundfx_play( lua_State * L ) {
//            ncALSound * s = soundfx_getstruct( L, 1 );
//
//            g_Core->p_Console->Print( LOG_INFO, "sfx:play() - playing \"%s\"\n", s->sName.c_str() );
//
//            s->Play();

            return 1;
        }

        int soundfx_stop( lua_State * L ) {
//            ncALSound * s = soundfx_getstruct( L, 1 );
//            s->Stop();

            return 1;
        }

        int soundfx_setgain( lua_State * L ) {
//            ncALSound * s = soundfx_getstruct( L, 1 );
//            float gainVal = static_cast<float>( lua_tonumber( L, 2 ) );
//
//            s->SetGain( gainVal );

            return 1;
        }

        int soundfx_setradius( lua_State * L ) {
//            ncALSound * s = soundfx_getstruct( L, 1 );
//            float distVal = static_cast<float>( lua_tonumber( L, 2 ) );
//            float maxdistVal = static_cast<float>( lua_tonumber( L, 3 ) );
//            
//            s->SetRadius( distVal, maxdistVal );

            return 1;
        }
#endif
        /**
         * 
         *  Vector 3d class object.
         *
         */
        Vec3 * vec3c_getstruct( lua_State * l, int n )
        {
            // This checks that the argument is a userdata
            // with the metatable "luaL_Foo"
            return *(Vec3 **)luaL_checkudata( l, n, "luaL_sfx" );
        }
        
        int vec3c_new( lua_State * L ) {
            
//            const float x = lua_tonumber( L, 1 );
//            const float y = lua_tonumber( L, 2 );
//            const float z = lua_tonumber( L, 3 );
//            
//            // Allocate data.
//            Vec3 ** udata = (Vec3 **)lua_newuserdata(L, sizeof(Vec3 *));
//            *udata = (Vec3*)AllocMemory( &scriptMemoryPool, sizeof(Vec3) );// new Vec3();
//            
//            // Effectively, this metatable is not accessible by Lua by default.
//            luaL_getmetatable(L, "luaL_sfx");
//            
//            lua_setmetatable(L, -2);
//            
//            // We return 1 so Lua callsite will get the user data and
//            // Lua will clean the stack after that.
//            g_Core->p_Console->Print( LOG_INFO, "vec3:new - new with (%4.2f %4.2f %4.2f)\n", x, y ,  z );
            
            return 1;
        }
        
        int vec3c_gc( lua_State * L ) {
            Vec3 *strc = vec3c_getstruct( L, 1 );
            delete strc;
    
            return 0;
        }
        
#ifndef NEKO_SERVER
        
        /**
         *  
         *  Entity class object.
         *
         */
        ncMesh * entity_getstruct( lua_State * l, int n )
        {
            // This checks that the argument is a userdata
            // with the metatable "luaL_Foo"
            return *(ncMesh **)luaL_checkudata( l, n, "luaL_ent" );
        }

        int32_t entity_new( lua_State * L ) {
            
//            const char *_entType = lua_tostring( L, 1 );
//            
//            // Allocate data.
//            ncMesh ** udata = (ncMesh **)lua_newuserdata(L, sizeof(ncMesh *));
//            *udata = (ncMesh*)AllocMemory( &scriptMemoryPool, sizeof(ncMesh) );// new ncMesh();
//            
//            // Effectively, this metatable is not accessible by Lua by default.
//            luaL_getmetatable(L, "luaL_ent");
//            
//            lua_setmetatable(L, -2);
//            
//            // We return 1 so Lua callsite will get the user data and
//            // Lua will clean the stack after that.
//            g_Core->p_Console->Print( LOG_INFO, "ent:create - new entity with type \"%s\"\n", _entType);
            
            
            return 1;
        }
        
        int entity_gc( lua_State * L ) {
            ncMesh *_mesh = entity_getstruct( L, 1 );
            delete _mesh;
            
            return 0;
        }
#endif
        
        /**
         *
         *
         *      Lua class objects.
         *   Accessed by it's namespace!
         *
         *
         */

        
#ifndef NEKO_SERVER
        const luaL_Reg SFXClass[] = {
            { "prepare", soundfx_new },
            { "setpos", soundfx_setorigin },
            { "setvelocity", soundfx_setvelocity },
            { "setgain", soundfx_setgain },
            { "setlooping", soundfx_setlooping },
            { "setradius", soundfx_setradius },
            { "set2d", soundfx_set2d }, // 3D sound?
            { "play", soundfx_play },
            { "stop", soundfx_stop },

            { "__gc", soundfx_gc },
            { NEKO_NULL, NEKO_NULL }
        };

        const luaL_Reg FXClass[] = {
            { "create", fx_new },
            { "setorigin", fx_setpos },
            { "setvelocity", fx_setvelocity },
            { "setsize", fx_setsize },
            { "setlifetime", fx_setlifetime },
            { "setcolor", fx_setcolor },
            { "setgravity", fx_setgravity },
            { "setperiod", fx_settime },
            { "setamount", fx_setamount },
            { "spawn", fx_spawn },
            { "init", fx_init },
            { "newone", fx_initspawn },
  
            { NEKO_NULL, NEKO_NULL }
        };

        const luaL_Reg EntityClass[] = {
            { "create", entity_new },
            //{ "setpos", entity_setpos },
            { "__gc", entity_gc },
            { NEKO_NULL, NEKO_NULL }
        };
#endif
        
//        const luaL_Reg Vec3Class[] = {
//            { "new", vec3c_new },
//            { "__gc", vec3c_gc },
//            { NEKO_NULL, NEKO_NULL }
//        };
    }

    /**
     *  Lua function C wrapper.
     */
    ncLuaFunctionPool::ncLuaFunctionPool() {
        r_funcId = -1;
        r_objId = -1;

        r_name = "";
    }

    /**
     *  Precache function.
     */
    void ncLuaFunctionPool::AddFunction( lua_State *L, const char *functionName ) {

        // Push this function to stack.
        r_name = functionName;

        lua_pushvalue( L, 1 );
        r_objId = luaL_ref( L, LUA_REGISTRYINDEX );
        // Get the object instance.
        lua_rawgeti( L, LUA_REGISTRYINDEX, r_objId );

        // No, these are just functions without classes.
        
        // Get the function.
        // Using tables to create function groups.
        //lua_getfield(L, -1, "core");

        lua_getglobal( L, r_name.c_str() );
        r_funcId = luaL_ref( L, LUA_REGISTRYINDEX );

        lua_pop( L, 1 );
    }

    /**
     *  Call Lua function with arguments.
     */
    template<typename... Args>
    int ncLuaFunctionPool::CallLuaFunction( lua_State * L, int retvals, Args... rest ) {

        const int old_stack_height = lua_gettop( L );
        lua_rawgeti( L, LUA_REGISTRYINDEX, r_funcId );

        // Push the arguments.
        int pushed = PushLuaArgs( L, rest... );
        //lua_call( L, pushed, retvals );
        lua_pcall( L, pushed, retvals, 0 );

        return lua_gettop( L ) - old_stack_height;
    }

    /**
     *  Call function.
     */
    template<>
    bool ncLuaFunctionPool::Call<const char*>( lua_State * L, const char * Lparam )  {
        lua_rawgeti( L, LUA_REGISTRYINDEX, r_funcId );


        lua_pushstring( L, Lparam );

        // Call function.
        if( lua_pcall( L, 1, 0, 0 ) != 0 ) {
            g_Core->p_Console->Print( LOG_WARN, "Script error while running function '%s' - %s\n", r_name.c_str(), lua_tostring( L, -1 ) );
            return false;
        }

        return true;
    }

    /**
     *  New script base instance.
     */
    ncScriptBase::ncScriptBase() {
        _global_state = NEKO_NULL;
    }

    /**
     *  Destructor.
     */
    ncScriptBase::~ncScriptBase() {
        //Shutdown();
    }

    /**
     *  Error handler.
     */
    int ncScriptBase::ErrorHandler( lua_State *L ) {
        const char *errorText = lua_tostring( L, -1 );

        lua_getglobal( L, "debug" ); // stack: err debug
        lua_getfield( L, -1, "traceback" ); // stack: err debug debug.traceback

        // debug.traceback() returns one argument.
        if( lua_pcall( L, 0, 1, 0 ) )
        {
            const char * err = lua_tostring( L, -1 );
            g_Core->p_Console->Print( LOG_ERROR, "Error in debug.traceback() call: %s\n", err ) ;
        }
        else
        {
            const char * stackTrace = lua_tostring( L, -1 );
            g_Core->p_Console->Print( LOG_ERROR, "C++ stack traceback: %s\n", errorText );
            g_Core->p_Console->Print( LOG_ERROR, "Possibly the following error will be empty: %s\n", stackTrace );
        }

        g_Core->p_Console->Error( ERR_FATAL, "Script error. \n\n%s", errorText );
        return 1;
    }

    /**
     *  Load Lua script file.
     */
    bool ncScriptBase::LoadLuaScript( const char *filename, const bool hasScriptCore ) {

        if( !_global_state ) {
            g_Core->p_Console->Print( LOG_WARN, "LoadLuaScript \"%s\" - No Lua virtual machine initialized! Ignoring\n", filename );
            return false;
        }

        int ret;

        g_Core->p_Console->Print( LOG_INFO, "LoadLuaScript(): Loading %s\n", filename );

        // Load file.
//        if( !isConfig ) {
            const char * l_filePath = NekoCString::STR( "scripts/%s.lua", filename );
            
            SPackFile * luaScriptPack = g_Core->p_FileSystem->GetPak( "shared_data" );
            AssetDataPool scriptInfo = luaScriptPack->GetData( l_filePath, NEKO_NULL, true );
            
            ret = luaL_loadstring( _global_state, reinterpret_cast<const char*>( scriptInfo.tempData ) );
//            delete [] scriptInfo;
            _PopMemoryFrame( scriptInfo.tempPool );
//        } else {
//            const char * l_filePath = NekoCString::STR( "%s/scripts/%s.lua", Filesystem_Path.Get<const char*>(), filename );
//            ret = luaL_dofile( _global_state, l_filePath );
//        }
        
        if( ret != 0 )
        {
            g_Core->p_Console->Print( LOG_INFO, "Error occurs when calling luaL_dofile or luaL_loadstring, hint: 0x%x\n", ret );
            ErrorHandler( _global_state );
            return false;
        }

        if( lua_pcall( _global_state, 0, 0, 0 ) )
        {
            ErrorHandler( _global_state );
            return false;
        }

        ncLuaScript * tempScript = new ncLuaScript( MAX_LUA_STACK_FUNCTIONS, pAllocator );


        tempScript->SetState( _global_state );

        
        /**      Add native function wrappers.     **/
        
        //  Create a new function environment and store it in the registry.
        if( !hasScriptCore )
        {
            m_luaCache[filename] = tempScript;
            g_Core->p_Console->Print( LOG_DEVELOPER, "Ignoring scripting core functions and and namespaces for %s.lua\n", filename );
            return true;
        }
        

        //tempScript.RegisterMeta( "vec3", ncScriptClassObjects::Vec3Class );

        g_Core->p_Console->Print( LOG_INFO, "Registering function callbacks for %s\n", filename );

        lua_State * luaState = tempScript->GetState();
        
        tempScript->PrecacheFunction( luaState, "OnLoad" );
        tempScript->PrecacheFunction( luaState, "OnWorldCreate" );
        tempScript->PrecacheFunction( luaState, "OnSettingsRefresh" );
        tempScript->PrecacheFunction( luaState, "OnKeyPress" );

        //tempScript.PrecacheFunction( _global_state, "OnAssetLoad" );
        //m_luaCache[filename].PrecacheFunction( L, "what" );

        // Register functions and some things.
        // .. so we can do cool stuff in scripts later on.
        g_Core->p_Console->Print( LOG_INFO, "Registering namespaces with functions for %s\n", filename );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::PrintLn, "game", "println" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetGameTime, "game", "gettime" );

        // Console.
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetCvarValue, "con", "getcvar" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::SetCVarInt, "con", "setcvarint" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::SetCVarBool, "con", "setcvarbool" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::SetCVarFloat, "con", "setcvarfloat" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::SetCVarString, "con", "setcvarstring" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::RegisterCVar, "con", "registercvar" );

        // Client.
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetOriginX, "player", "getoriginx" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetOriginY, "player", "getoriginy" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetOriginZ, "player", "getoriginz" );

        // Core.
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Error, "core", "error" );

        // File constants.
        tempScript->RegisterConstant<const char*>( filename, "main", "_filename" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::PrintAllFunctionNames, "main", "dumpall" ); // Print all function names.

        // Math.
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::FastSqrtf, "gmath", "fastsqrtf" );

        tempScript->RegisterConstant<float>( nkMath::PI, "gmath", "pi" );
        tempScript->RegisterConstant<float>( nkMath::PI2, "gmath", "pi2" );
        tempScript->RegisterConstant<float>( nkMath::PI2D, "gmath", "pid2" );
        tempScript->RegisterConstant<float>( nkMath::INF, "gmath", "infinite" );
        tempScript->RegisterConstant<float>( nkMath::EPSILON, "gmath", "eps" );
        tempScript->RegisterConstant<float>( nkMath::PIHALF, "gmath", "pihalf" );
        tempScript->RegisterConstant<float>( nkMath::Rad2Deg, "gmath", "rad2deg" );
        tempScript->RegisterConstant<float>( nkMath::SEC2MS, "gmath", "sec2ms" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::LargeRandom, "gmath", "largerand" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::FastPow, "gmath", "fastpow" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::RandomFloatAlt, "gmath", "randfloatalt" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Absf, "gmath", "absf" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Absi, "gmath", "absi" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Deg2Rad, "gmath", "deg2rad" /* degree to radians */ );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Rad2Deg, "gmath", "rad2deg" /* radians to degree */ );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Signi, "gmath", "signi" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::LerpfAlt, "gmath", "lerpfalt" /* a, b, time */ );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Maxi, "gmath", "maxint" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Mini, "gmath", "minint" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Maxf, "gmath", "maxf" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::Minf, "gmath", "minf" );
        
        // UI. ( hell yeah )
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::AddStaticText2D, "ui", "addtext2d" );

        // Input & controls.
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetLastPressedKey, "input", "getlastpressedkey" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetLastReleasedKey, "input", "getlastreleasedkey" );
        tempScript->RegisterConstant<lua_CFunction>( ncScriptFunction::GetMouseX, "input", "getmousex" );
        tempScript->ncLuaScript::RegisterConstant<lua_CFunction>( ncScriptFunction::GetMouseY, "input", "getmousey" );


        m_luaCache[filename] = tempScript;
        
        // Script was loaded, so we call OnLoad trigger function.
        //m_luaCache[filename].RegisterConstant<int>( 13, "println" );
        // In case if function call failed.
        if( !ExecuteScript( _global_state, filename, "OnLoad" ) )
            return false;

        return true;
    }

    /**
     *  Shutdown script system.
     */
    void ncScriptBase::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "Script system shutting down..\n" );
        
        
        for( size_t i = 0; i < m_luaCache.GetSize(); ++i )
        {
            const char * materialName = m_luaCache[i]; // Returns NEKO_NULL if key wasn't found.
            if( materialName ) {
                ncLuaScript * mat = m_luaCache[materialName];
                mat->Kill();
                
                delete mat;
            }
        }
        
        // Delete script cache.
        m_luaCache.Delete();

        lua_close( _global_state );
        _global_state = NEKO_NULL;
    }

    /**
     *  Execute Lua function.
     */
    bool ncScriptBase::ExecuteScript( lua_State *L, const char *filename, const char *function ) {
        ncLuaScript *tmp = m_luaCache[filename];
        ncLuaFunctionPool *tmpFunc = tmp->m_luaFunctions[function];
        return tmpFunc->Call( L, function );
    }

    bool ncScriptBase::ScriptFunction( const char *filename, const char *variable ) {
        return ExecuteScript( _global_state, filename, variable );
    }

    /**
     *  Execute Lua function with parameters.
     */
    template<typename... Args>
    bool ncScriptBase::ExecuteScript( lua_State *L, const char *pScriptName, const char *pFunctionName, const int pArgCount, Args... rest )
    {
        ncLuaScript *tmp = m_luaCache[pScriptName];
        ncLuaFunctionPool *tmpFunc = tmp->m_luaFunctions[pFunctionName];
        return tmpFunc->CallLuaFunction( L, pArgCount, rest... );
    }

    template<typename... Args>
    bool ncScriptBase::ScriptFunction(const char *pScriptName, const char *pFunctionName, const int pArgCount, Args... rest)
    {
        ncLuaScript *tmp = m_luaCache[pScriptName];
        
        ncLuaFunctionPool *tmpFunc = tmp->m_luaFunctions[pFunctionName];
        return tmpFunc->CallLuaFunction( NEKO_NULL, pArgCount, rest... );
    }

    /**
     *  Get constant value.
     */
    template<>
    int ncScriptBase::GetScriptVar<int>( const char *filename, const char *variable )
    {
        ncLuaScript *tmp = m_luaCache[filename];
        return tmp->GetConstant<int>( variable );
    }

    // boolean.
    template<>
    bool ncScriptBase::GetScriptVar<bool>( const char *filename, const char *variable )
    {
        ncLuaScript *tmp = m_luaCache[filename];
        return tmp->GetConstant<bool>( variable );
    }

    //template<>
    //float ncScriptBase::GetScriptVar<float>( const char *filename, const char *variable ) {
    //    return m_luaCache[filename].GetConstant<float>( variable );
    //}

    /**
     *  Register global metatable with class objects.
     */
    void ncScriptBase::RegisterClassObject( lua_State * L, const char *metaname, const luaL_Reg * regs ) {
        lua_pushvalue( L, 1 );
        // Create a luaL metatable.

        const char *_metaname = NekoCString::STR( "luaL_%s", metaname );
        luaL_newmetatable( L, _metaname );
        
        // Register the C functions _into_ the metatable we just created.
        luaL_setfuncs( L, regs, 0 );
        
        // 1| metatable "luaL_Foo"   |-1
        lua_pushvalue( L, -1 );
        
        // Set the "__index" field of the metatable to point to itself
        lua_setfield( L, -1, "__index" );
        
        // This allows Lua scripts to _override_ the metatable of Foo.
        // For high security code this may not be called for but
        // we'll do this to get greater flexibility.
        // metaname is _not_ _metaname
        lua_setglobal( L, metaname );
        
        lua_pop( L, 1 );
        
        g_Core->p_Console->Print( LOG_INFO, "Registered class object \"%s\"\n", metaname );
    }
    
    /**
     *  Initialize Lua state.
     */
    void ncScriptBase::Init( INekoAllocator * allocator )
    {
        g_Core->p_Console->Print( LOG_INFO, "------ Game scripting system initializing ------\n" );
		
        _global_state = luaL_newstate();
        if( _global_state == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_ASSET, "Couldn't initialize Lua VM for script system.\n" );
            return;
        }

        luaL_openlibs( _global_state );

        // Use Lua core libraries? Also we have custom alternatives.
#ifdef _USE_LUAOPEN_
        luaopen_io( _global_state );
        luaopen_math( _global_state );
        luaopen_string( _global_state );
        luaopen_base( _global_state );
        luaopen_debug( _global_state );
#endif
		
        pAllocator = allocator;
        
        // Precache memory for script file data.
        if( !m_luaCache.Create( MAX_LUA_SCRIPTS, pAllocator ) ) {
            g_Core->p_Console->Error( ERR_FATAL, "Could not create Lua cache base with %s size.\n", MAX_LUA_SCRIPTS );
            return;
        }

        // Register custom class objects.
        g_Core->p_Console->Print( LOG_INFO, "Registering class objects..\n" );
        
#ifndef NEKO_SERVER
        RegisterClassObject( _global_state, "fx", ncScriptClassObjects::FXClass );
        RegisterClassObject( _global_state, "sfx", ncScriptClassObjects::SFXClass );
        RegisterClassObject( _global_state, "ent", ncScriptClassObjects::EntityClass );
#endif

        // Load core config now.
        LoadLuaScript( "config", false  );
    }

    /**
     *  Load Lua scripts.
     */
    void ncScriptBase::LoadScripts()
    {
        g_Core->p_Console->Print( LOG_INFO, "--------- Loading game scripts ---------\n" );

        if( Server_Dedicated.Get<bool>() )
        {
//            LoadLuaScript( "serverinit" );
        }
        else
        {
            LoadLuaScript( "vec3" );
            LoadLuaScript( "main" );
            //LoadLuaScript( "player" );
            LoadLuaScript( "env" );
        }

        // Reset Lua state.
        lua_pop( _global_state, 1 );

        g_Core->p_Console->Print( LOG_INFO, "Currently %i scripts were loaded.\n", m_luaCache.GetCount() );
    }

    /**
     *
     *  Lua script.
     *
     */

    /**
     *  Register integer constant.
     */
    template<>
    void ncLuaScript::RegisterConstant<int>( int value, const char *classname, const char *constantname )
    {
        //lua_pushinteger( L, value );
        //lua_setglobal( L, constantname );

        CreateClassTable( classname );

        lua_pushstring( L, constantname );      // Push key onto stack.
        lua_pushinteger( L, value );             // Push value onto stack.

        lua_settable( L, -3 );                  // Add key-value pair to table.

        lua_pop( L, 1 );                     // Pop table from stack.
    }

    /**
     *  Register double constant.
     */
    template<>
    void ncLuaScript::RegisterConstant<double>( const double value, const char *classname, const char *constantname )
    {
        //lua_pushnumber( L, value);
        //lua_setglobal( L, constantname );

        CreateClassTable( classname );

        lua_pushstring( L, constantname );      // Push key onto stack.
        lua_pushnumber( L, (double)value );     // Push value onto stack.

        lua_settable( L, -3 );                  // Add key-value pair to table.

        lua_pop( L, 1 );                     // Pop table from stack.
    }

    /**
     *  Register float constant.
     */
    template<>
    void ncLuaScript::RegisterConstant<float>( const float value, const char *classname, const char *constantname )
    {
        //lua_pushnumber( L, value);
        //lua_setglobal( L, constantname );

        CreateClassTable( classname );

        lua_pushstring( L, constantname );      // Push key onto stack.
        lua_pushnumber( L, (float)value );     // Push value onto stack.

        lua_settable( L, -3 );                  // Add key-value pair to table.

        lua_pop( L, 1 );                     // Pop table from stack.
    }

    /**
     *  Register string constant.
     */
    template<>
    void ncLuaScript::RegisterConstant<const char*>( const char *value, const char *classname, const char *constantname )
    {
        //lua_pushstring( L, value );
        //lua_setglobal( L, constantname );

        CreateClassTable( classname );

        lua_pushstring( L, constantname );      // Push key onto stack.
        lua_pushstring( L, value );             // Push value onto stack.

        lua_settable( L, -3 );                  // Add key-value pair to table.

        lua_pop( L, 1 );                     // Pop table from stack.
    }

    /**
     *  Register boolean constant.
     */
    template<>
    void ncLuaScript::RegisterConstant<bool>( bool value, const char *classname, const char *constantname )
    {
        //lua_pushboolean( L, value );
        //lua_setglobal( L, constantname );

        CreateClassTable( classname );

        lua_pushstring( L, constantname );      // Push key onto stack.
        lua_pushboolean( L, value );             // Push value onto stack.

        lua_settable( L, -3 );                  // Add key-value pair to table.

        lua_pop( L, 1 );                     // Pop table from stack.
    }

    /**
     *  Register function.
     */
    template<>
    void ncLuaScript::RegisterConstant<lua_CFunction>( lua_CFunction value, const char *classname, const char* constantname )
    {
        CreateClassTable( classname );

        lua_pushstring( L, constantname );       // push key onto stack
        lua_pushcfunction( L, value ); // push value onto stack
        lua_settable( L, -3 );               // add key-value pair to table

        lua_pop( L, 1 );                     // Pop table from stack.
    }

    /**
     *  Arguments.
     */
    template<>
    int ncLuaScript::GetArgument<int>( int index ) {
        return static_cast<int>( lua_tointeger( L, index ) );
    }

    template<>
    double ncLuaScript::GetArgument<double>( int index )
    {
        return lua_tonumber( L, index );
    }

    template<>
    char * ncLuaScript::GetArgument<char*>( int index )
    {
        return (char*)lua_tostring( L, index );
    }

    template<>
    bool ncLuaScript::GetArgument<bool>( int index )
    {
        return lua_toboolean( L, index );
    }

    /**
     *  Get argument function count.
     */
    int ncLuaScript::GetArgumentCount() {
        return lua_gettop( L );
    }

    /**
     *  Get constant value.
     */
    template<>
    int ncLuaScript::GetConstant<int>( const char *N ){
        int value = -1;

        if( ( LuaUtilities::lua_intexpr( L, N, &value ) == -1 ) )
            return -1;

        lua_pop( L, 1 );
        return value;
    }

    template<>
    bool ncLuaScript::GetConstant<bool>( const char *N ){
        lua_pushvalue( L, 1 );
        bool value = LuaUtilities::lua_boolexpr( L, N );
        lua_pop( L, 1 );

        return value;
    }

    template<>
    const char * ncLuaScript::GetConstant<const char*>( const char *N ){
        lua_pop( L, 1 );
        return LuaUtilities::lua_stringexpr( L, N, 0 );
    }

    /**
     *  Create table and store variables/functions in it.
     *  Later on in the script we can use namespaces.
     */
    bool ncLuaScript::CreateClassTable( const char *classname ) {
        // Push table onto stack.
        lua_getglobal( L, classname );
        //lua_getfield(lstate, LUA_GLOBALSINDEX, "core");

        // Not a table, create it.
        if( !lua_istable( L, -1 ) )
        {
            lua_createtable( L, 0, 1 );      // Create new table.
            //lua_setfield( L, LUA_GLOBALSINDEX, tableName);  // Add it to global context.
            lua_setglobal( L, classname );
            // Reset table on stack.
            lua_pop(L, 1);                 // Pop table (nil value) from stack.
            //lua_getfield(lstate, LUA_GLOBALSINDEX, tableName);  // Push table onto stack.
            lua_getglobal( L, classname );
            //lua_setglobal( L, "__index" );
            //lua_setfield(L, -2, "__index");
        }
        return true;
    }

    /**
     *  Register class.
     */

    void ncLuaScript::RegisterMeta( const char * metaname, const luaL_Reg * regs )
    {
        lua_pushvalue( L, 1 );
        // Create a luaL metatable.
        const char *_metaname = NekoCString::STR( "luaL_%s", metaname );
        luaL_newmetatable( L, _metaname );

        // Register the C functions _into_ the metatable we just created.
        luaL_setfuncs( L, regs, 0 );

        // 1| metatable "luaL_Foo"   |-1
        lua_pushvalue( L, -1 );

        // Set the "__index" field of the metatable to point to itself
        lua_setfield( L, -1, "__index" );

        // This allows Lua scripts to _override_ the metatable of Foo.
        // For high security code this may not be called for but
        // we'll do this to get greater flexibility.
        // metaname is _not_ _metaname
        lua_setglobal( L, metaname );

        lua_pop( L, 1 );

        g_Core->p_Console->Print( LOG_INFO, "Registered %s class object.\n", metaname );
    }
    
    

    /**
     *  New Lua script instance.
     */
    ncLuaScript::ncLuaScript() {
        L = NEKO_NULL;
    }

    ncLuaScript::ncLuaScript( const int maxLuaFunctions, INekoAllocator * allocator )
    {
        pAllocator = allocator;
        
        if( !m_luaFunctions.Create( maxLuaFunctions, allocator ) ) {
            g_Core->p_Console->Error( ERR_FATAL, "Could not create Lua function cache with %i size.\n", maxLuaFunctions );
        }
    }

    /**
     *  Destructor.
     */
    ncLuaScript::~ncLuaScript()
    {

    }

    /**
     *  Destroy the script and properties.
     */
    void ncLuaScript::Kill()
    {
        // Do not close Lua state! It's just a pointer to main state.
        m_luaFunctions.Delete();
    }
    
    /**
     *  Precache function.
     */

    void ncLuaScript::PrecacheFunction( lua_State *L, const char *N ) {
        ncLuaFunctionPool *temp = (ncLuaFunctionPool*)pAllocator->Alloc( sizeof(ncLuaFunctionPool) );
        temp->AddFunction( L, N );

        m_luaFunctions[N] = temp;
    }

    /**
     *  Set state.
     */
    void ncLuaScript::SetState( lua_State *_L ) {
        L = _L;
    }

    /**
     *  Push Lua function arguments.
     */
    template<typename... Args>
    int ncLuaFunctionPool::PushLuaArgs( lua_State *L, bool next, Args... rest ) {
        lua_pushboolean( L, next );
        return 1 + PushLuaArgs( L, rest...);
    }

    template<typename... Args>
    int ncLuaFunctionPool::PushLuaArgs( lua_State *L, const char * next, Args... rest ) {
        lua_pushstring( L, next );
        return 1 + PushLuaArgs( L, rest...);
    }

    template<typename... Args>
    int ncLuaFunctionPool::PushLuaArgs( lua_State *L, int next, Args... rest ) {
        lua_pushinteger( L, next );
        return 1 + PushLuaArgs( L, rest...);
    }

    template<typename... Args>
    int ncLuaFunctionPool::PushLuaArgs( lua_State *L, const double next, Args... rest ) {
        lua_pushnumber( L, (double)next );
    }

    template<typename... Args>
    int ncLuaFunctionPool::PushLuaArgs( lua_State *L, const float next, Args... rest ) {
        lua_pushnumber( L, (float)next );
    }

    int ncLuaFunctionPool::PushLuaArgs( lua_State *L ) {
        return 0;
    }

    template<>
    void ncLuaFunctionPool::PushTableString<int>( lua_State * L, const char * key, const int value ) {
        lua_pushstring( L, key );
        lua_pushinteger( L, value );
        lua_settable( L, -3 );
    }

    //template<>
    //double ncLua::GetConstant<double>( const char *N ){
        //return lua_( L, N );
    //}
}

#endif
