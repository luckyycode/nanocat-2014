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
//  UserMenu.h
//  A menu manager.
//
//  Created by Kawaii Neko on 8/3/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//


#ifndef UserMenu_cpp
#define UserMenu_cpp

#include "MenuDef.h"
#include "../CoreDef.h" // Non-copyable define.

namespace Neko {

    class MenuDef;
    
    /**
     *  User interface manager.
     */
    class UserInterface
    {
        NEKO_NONCOPYABLE( UserInterface );
        
    public:
        UserInterface();
        ~UserInterface();
        
        /**
         *   Initialize menu manager.
         */
        void					Initialize( INekoAllocator * allocator );
        
        /**
         *  Update method.
         */
        void					Frame( double time );
        
        /**
         *   Render all menus.
         */
        void					Render();
        
        /**
         *   Add new menu.
         */
        void					AddMenu( MenuDef * menu );
        
        /**
         *  Get menu by name.
         */
        MenuDef             * GetMenu( const char * name );
        
        /**
         *   Is menu opened?
         */
        bool					IsMenuOpen( const char * name );
        
        /**
         *   Open menu.
         */
        void					OpenMenu( const char * name );
        
        /**
         *   Close menu.
         */
        void					CloseMenu( const char * name );
        
        /**
         *  Shutdown UI.
         */
        void                Shutdown();
        
        
        /**
         *   Opened menus.
         */
        const uint32_t              GetOpenedMenusCount() const ;
        
        /**
         *   All loaded menus.
         */
        const uint32_t              GetLoadedMenusCount() const;
        
        
    private:
        
        INekoAllocator  * pAllocator;
        
        // Number of currently opened menus
        int32_t					numOpenedMenus;
        
        // An array of MenuDefs (menus)
        CVectorList<MenuDef*>	menuList;
        
        void                    UnloadAll();
        
    };
}

#endif /* UserMenu_cpp */
