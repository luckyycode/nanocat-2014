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
//  MenuDef.cpp
//  An organized property to render the whole menu with its items.
//
//  Created by Kawaii Neko on 8/3/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "MenuDef.h"
#include "../Console/Console.h"

namespace Neko {

    /**
     *  Constructor.
     */
    MenuDef::MenuDef( const char * nname )
    {
//        Create( nname );
    }
    
    /**
     *  Create MenuDef.
     */
    void MenuDef::Create( const char *nname, INekoAllocator * allocator )
    {
        Reset();
        
        pAllocator = allocator;
        this->name = nname;
        
        itemList.Create(32, pAllocator);
        
        g_Core->p_Console->Print( LOG_INFO, "AllocMenuDef(): A new MenuDef property with name \"%s\" was created.\n", nname );
    }
    
    /**
     *  Destructor.
     */
    MenuDef::~MenuDef()
    {
        uint32_t i;
        
        for( i = 0; i < itemList.GetCount(); ++i )
        {
            
            ItemDef *item = itemList[i];
            
            if( item != NEKO_NULL ) {
                pAllocator->Dealloc( item ) ;
            }
        }
        
        itemList.Clear();
        
        memset( (void*)this, 0, sizeof( MenuDef ) );
        Reset();
    }
    
    /**
     *  Reset current MenuDef.
     */
    void MenuDef::Reset()
    {
        this->drawCursor = this->active = this->opened = false;
        
        this->Init = NEKO_NULL;
        this->Update = NEKO_NULL;
        this->OnOpen = NEKO_NULL;
        this->OnClose = NEKO_NULL;
    }
    
    /**
     *  Menu name.
     */
    const char * MenuDef::GetName()
    {
        return name;
    }
    
    /**
     *  Is Menu open?
     */
    bool MenuDef::IsOpen()
    {
        return this->opened;
    }
    
    /**
     *  Is Menu active?
     */
    bool MenuDef::IsActive()
    {
        return this->active;
    }
    
    
    
    /**
     *  Find item in menu.
     */
    ItemDef	* MenuDef::FindItem( const char * name )
    {
        if( !name )
        {
            return NEKO_NULL;
        }
        
        uint32_t 	i;
        
        for( i = 0; i < itemList.GetCount(); ++i )
        {
            // Find an item by the name.
            if( !strcmp( name, itemList[i]->name ) )
            {
                return itemList[i];
            }
        }
        
        return NEKO_NULL;
    }

    /**
     *  Add new item to menu.
     */
    void MenuDef::AddItem( ItemDef * newItem )
    {
        if( newItem == NEKO_NULL )
        {
            return;
        }
        
        // Check if already exists.
        if( newItem->name != NEKO_NULL && FindItem( newItem->name ) != NEKO_NULL )
        {
            return;
        }
        
        itemList.PushBack( newItem );
    }
}