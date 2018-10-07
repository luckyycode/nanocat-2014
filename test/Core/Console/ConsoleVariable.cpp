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
//  ConsoleVariable.cpp
//  Console variable.. :J
//
//  This code is a part of Neko engine.
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "ConsoleVariable.h"
#include "../Core.h"
#include "../String/StringHelper.h"
#include "../../Platform/Shared/SystemShared.h"

namespace Neko {

    /**
     *  Constructor.
     */
    SConsoleVar::SConsoleVar()
    {
        fval = 0.0f;
        ival = 0;

        bval = true;
    }

    /**
     *  Register integer CVar.
     */
    template<> void SConsoleVar::RegisterCVar<int>(ECvarGroup group, const char *varName, const char *varDescription, int varValue, CVFlag flag, ECvarType type )
    {
        strName = varName;
  
        // Try to adopt another variables.
        ival = varValue;
        fval = float( varValue );
        bval = varValue;
        eGroup = group;
        
        eFlag = (uint8_t)flag;
    }

    /**
     *  Register float CVar.
     */
    template<> void SConsoleVar::RegisterCVar<float>(ECvarGroup group, const char *varName, const char *varDescription, float varValue, CVFlag flag, ECvarType type )
    {
        strName = varName;

        ival = int( varValue );
        fval = varValue;
        bval = varValue;
        eGroup = group;
        eType = type;
        
        eFlag = (uint8_t)flag;
    }

    /**
     *  Register string CVar.
     */
    template<> void SConsoleVar::RegisterCVar<const char*>(ECvarGroup group, const char *varName, const char *varDescription, const char * varValue, CVFlag flag, ECvarType type )
    {
        fval = 0.0f;
        ival = 0;
        
        bval = true;
        strName = varName;
        strStringValue = varValue;
        eGroup = group;
        eType = type;
        
        eFlag = (uint8_t)flag;
    }

    /**
     *  Register boolean CVar.
     */
    template<> void SConsoleVar::RegisterCVar<bool>(ECvarGroup group, const char *varName, const char *varDescription, bool varValue, CVFlag flag, ECvarType type )
    {
        strName = varName;

        ival = bool( varValue );
        fval = bool( varValue );
        bval = varValue;
        eGroup = group;
        eType = type;
        
        eFlag = (uint8_t)flag;
    }

    // "Gets"
    
    /**
     *  Get integer CVar value.
     */
    template<> int SConsoleVar::Get<int>()
    {
        return ival;
    }
    
    /**
     *  Get float CVar.
     */
    template<> float SConsoleVar::Get<float>()
    {
        return fval;
    }

    /**
     *  Get boolean CVar value.
     */
    template<> bool SConsoleVar::Get<bool>()
    {
        return bval;
    }

    /**
     *  Get string CVar value.
     */
    template<> const char * SConsoleVar::Get<const char*>()
    {
        return strStringValue.c_str();
    }

    // "Sets"
    
    /**
     *  Set integer CVar value.
     */
    template<> void SConsoleVar::Set( int value )
    {
        ival = value;
        fval = (float)value;
    }

    /**
     *  Set float CVar value.
     */
    template<> void SConsoleVar::Set( float value )
    {
        fval = value;
        ival = (int)value;
    }

    /**
     *  Set boolean CVar value.
     */
    template<> void SConsoleVar::Set( bool value )
    {
        bval = value;
    }

    /**
     *  Set string CVar value.
     */
    template<> void SConsoleVar::Set( const char *value )
    {
        ival = atoi(value);
        fval = atof(value);
        bval = (bool)ival;
        
        strStringValue = value;
    }

    /**
     *  Get CVar name.
     */
    const char * SConsoleVar::GetName()
    {
        return strName.c_str();
    }

    /**
     *  Lock console variable.
     */
    void SConsoleVar::Lock()
    {
        bLocked = true;
    }
}
