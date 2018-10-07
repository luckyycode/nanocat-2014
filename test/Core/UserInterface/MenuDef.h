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
//  MenuDef.h
//  An organized property to render the whole menu with its items.
//
//  Created by Kawaii Neko on 8/3/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//


#ifndef MenuDef_cpp
#define MenuDef_cpp

#include "ItemDef.h"
//#include <vector>
#include "../Utilities/VectorList.h"

namespace Neko {

    /**
     *  Menu with items.
     */
    class MenuDef
    {
    public:
  
        /**
         *  Create new MenuDef with a name.
         */
        MenuDef( const char * name );
        ~MenuDef();
        
        /**
         *  Create a new MenuDef.
         */
        void Create( const char * name, INekoAllocator * allocator );
        
        /**
         *  Menu name.
         */
        const char *		GetName();
        
        /**
         *  Is menu focused?
         */
        bool					IsActive();
        
        /**
         *  Is menu opened?
         */
        bool					IsOpen();
        
        /**
         *  Clear menu items.
         */
        void					Reset();
        
        
        CVectorList<ItemDef*>	GetGroup( const char * name );
        
        /**
         *  Search ItemDefs by a name.
         */
        ItemDef					* FindItem( const char * name );
        
        /**
         *   Add new ItemDef to menu.
         */
        void			AddItem( ItemDef * newItem );
        
        
    public:
        
        INekoAllocator  * pAllocator;
        
        /**
         *  Draw cursor in this menu?
         */
        bool					drawCursor;
        
        /**
         *  Is menu active?
         */
        bool                        active;
        
        /**
         *  Is menu opened?
         **/
        bool                        opened;
        
        /**
         *  ItemDef list.
         */
        CVectorList<ItemDef*>      itemList;
        
        /**
         *  Menu name.
         */
        const char *				name;
        
        /**
         *  Precache items here.
         */
        void					(*Init)( MenuDef * self );
        
        /**
         *  Update items.
         */
        void					(*Update)( MenuDef * self );
        
        /**
         *  On menu open.
         */
        void					(*OnOpen)( MenuDef * self );
        
        /**
         *  On menu close.
         */
        void					(*OnClose)( MenuDef * self );
        
    private:
        
    };
}

#endif /* MenuDef_cpp */
