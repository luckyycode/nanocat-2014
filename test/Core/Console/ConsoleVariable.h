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
//  ConsoleVariable.h
//  Console variable manager.. :J
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef consolevar_h
#define consolevar_h

#include "../String/StringHelper.h"
#include "../String/String.h"

namespace Neko {

    //!   Maximum console variable space to be allocated.
    static const int32_t MAX_CONSOLEVARIABLES = 96;

    ///  Console variable flag.
    enum class CVFlag : uint8_t
    {
        None            = 0x1,      // No protection.
        Readonly        = 0x2,      // Read only.
        NeedsRefresh    = 0x4,      // Needs restart to apply.
        Locked          = 0x8,      // Locked. No changes allowed.
        System          = 0x16,     // System one.
        Kid             = 0x32,      // Cheats.
        Hidden          = 0x64      // Not shown
    };
    
    /// Cvar groups.
    enum class ECvarGroup
    {
        Display = 0,
        Input,
        Server,
        Client,
        Network,
        Engine,
        Player,
        World,
        
        Max
    };
    
    enum class ECvarType : uint8_t
    {
        Int = 0x0,  // same as bool
        Float = 0x1,
        String = 0x2
    };
    
    ///  Console variable.
    struct SConsoleVar
    {
    public:
        SConsoleVar();
        virtual ~SConsoleVar() { }

        /**
         *  Get CVar name.
         */
        const char *                GetName();

        /**
         *  Register a CVar.
         */
        template<class t> void              RegisterCVar( ECvarGroup group, const char *varName, const char *varDescription, t varValue, CVFlag flag, ECvarType type );

        /**
         *  Getter.
         */
        template<class t> t                 Get();

        /**
         *  Setter.
         */
        template<class t> void              Set( t value );

        /**
         *  Locker.
         */
        void                Lock();

        //!  Float value.
        float fval;

        //! Integer value.
        int ival;

        //! Boolean value.
        uint8_t bval:1;
        
        //! Cvar flag.
        uint8_t eFlag:1;

        //!  String value.
        CStr strStringValue;

        //!  Console variable name.
        CStr    strName;
        
        //! Cvar group.
        ECvarGroup    eGroup;
        
        //! Cvar type.
        ECvarType   eType;

        //!  Is cvar locked?
        uint8_t     bLocked:1;
        
    protected:
    };

    /**
     *
     *  External definitions.
     *
     */
    template<> void SConsoleVar::RegisterCVar( ECvarGroup group, const char *varName, const char *varDescription, float varValue, Neko::CVFlag flag, ECvarType type );
    template<> void SConsoleVar::RegisterCVar( ECvarGroup group, const char *varName, const char *varDescription, int varValue, Neko::CVFlag flag, ECvarType type );
    template<> void SConsoleVar::RegisterCVar( ECvarGroup group, const char *varName, const char *varDescription, bool varValue, Neko::CVFlag flag, ECvarType type );
    template<> void SConsoleVar::RegisterCVar( ECvarGroup group, const char *varName, const char *varDescription, const char *varValue, Neko::CVFlag flag, ECvarType type );

   // template <> void SConsoleVar<const char*>::Set( const char* value ).
}

#endif
