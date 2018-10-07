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
//  UserMenu.cpp
//  A menu manager.
//
//  Created by Kawaii Neko on 8/3/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//


#include "UserMenu.h"

#include "../Console/Console.h"
#include "../Core.h"
#include "../Utilities/List.h"
#include "../ScriptSupport/Scripting.h"
#include "../Network/MultiplayerClient.h"
#include "../Network/MultiplayerServer.h"
#include "../../AssetCommon/AssetBase.h"
#include "../../Graphics/Renderer/Renderer.h"

//#include "BeautifulEnvironment.h" // temp
//#include "MenuList.h"

namespace Neko {
 
    /*
     *  Constructor.
     */
    UserInterface::UserInterface() : numOpenedMenus(0) {

    }
    
    /**
     *  Destructor.
     */
    UserInterface::~UserInterface() {
        
    }
    
    /**
     *  Shut down user interface system.
     */
    void UserInterface::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "UserInterface: shutting down..\n" );
        
        UnloadAll();
    }
    
    void Init_Menu_Main();
    /**
     *  Initialize user interface manager and create menus.
     */
    void UserInterface::Initialize( INekoAllocator * allocator )
    {
        pAllocator = allocator;
        
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "UserInterface(): Init\n" );
        menuList.Create( 128, pAllocator );
        
        Init_Menu_Main();
        
        
//        g_Core->p_ScriptBase->LoadLuaScript( "ui/menus/mainmenu" );
    }
    
    /**
     *  Is menu open?
     */
    bool UserInterface::IsMenuOpen( const char * name )
    {
        uint32_t i;
        
        for( i = 0; i < GetLoadedMenusCount(); ++i ) {
            if( !strcmp( name, menuList[i]->GetName() ) && !menuList[i]->IsOpen() ) {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     *  Open a menu.
     */
    void UserInterface::OpenMenu( const char * name )
    {
        MenuDef	* menu = NEKO_NULL;
        
        uint32_t i;
        uint32_t j;
        
        // Check if menus are loaded.
        if( !GetLoadedMenusCount() /* !bMenusLoaded */ ) {
            g_Core->p_Console->Print( LOG_WARN, "UserInterface::OpenMenu(): No MenuDefs\n" );
            return;
        }
        
        for( i = 0; i < GetLoadedMenusCount(); ++i ) {
            menu = menuList[i];
            
            if( !strcmp( name, menuList[i]->GetName() ) /*&& !menuList[i]->IsOpen()  do not reopen this! */ ) {
                ++numOpenedMenus;
                menu->opened = true;
                menu->active = true;
                
                g_Core->p_Console->Print( LOG_INFO, "UserInterface: Opened menu \"%s\"\n", menu->name );
   
                // If we got callback..
                if( menuList[i]->OnOpen != NEKO_NULL ) {
                    menuList[i]->OnOpen( menu );
                }
                
                // Hide all another menus.
                for( j = 0; j < GetLoadedMenusCount(); ++j ) {
                    if( menuList[j]->IsActive() && j != i ) {
                        menuList[j]->active = false;
                    }
                }
                
                return;
            }
        }
        
        g_Core->p_Console->Print( LOG_WARN, "UserInterface::OpenMenu(): MenuDef \"%s\" does not exist\n", name );
    }
    
    
    /**
     *  Close specified menu.
     */
    void UserInterface::CloseMenu( const char * name )
    {
        MenuDef	 * menu = NEKO_NULL;
        
        uint32_t i;
        uint32_t j;
        
        for( i = 0; i < GetLoadedMenusCount(); ++i ) {
            menu = menuList[i];
            
            if( !strcmp( name, menu->GetName() ) && menu->IsOpen() ) {
                --numOpenedMenus;
                
                menu->active = false;
                menu->opened = false;
                
                if( menu->OnClose != NEKO_NULL ) {
                    menu->OnClose( menu );
                }
                
                // activate previous menu
                for( j = GetLoadedMenusCount() - 1; j != -1; --j ) {
                    
                    if( menuList[j]->IsOpen() && !menuList[j]->IsActive() ) {
                        menuList[j]->active = true;
                    }
                }
                
                return;
            }
        }
    }
    
    /**
     *  Update menus.
     */
    void UserInterface::Frame( double time )
    {
        static uint32_t	i, j;
        
        MenuDef * menu = NEKO_NULL;
        ItemDef	* item = NEKO_NULL;
        
        // Update menus.
        for( i = 0; i < GetLoadedMenusCount(); ++i ) {
            menu = menuList[i];
            
            // Check if we can be used or updated.
            if( menu == NEKO_NULL || !menu->IsOpen() || !menu->IsActive() ) {
                continue;
            }
            
            // Update menu.
            if( menu->Update != NEKO_NULL ) {
                menu->Update( menu );
            }

            // Process items.
            for( j = 0; j < menu->itemList.GetCount(); ++j ) {
                item = menu->itemList[j];
                
                if( item != NEKO_NULL ) {
                    item->Frame();
                }
            }
        }
    }
    
    /**
     *  Render all menus.
     */
    void UserInterface::Render()
    {
        // Blending.
        g_mainRenderer->SetBlending( true, EBlendMode::AlphaPremultiplied, EBlendMode::AlphaPremultiplied );
        
        static uint32_t	i, j;
        MenuDef * menu = NEKO_NULL;
        
        for( i = 0; i < GetLoadedMenusCount(); ++i ) {
            menu = menuList[i];
            
            if( menu == NEKO_NULL || !menu->IsOpen() /*|| !menu->IsActive()*/ ) {
                continue;
            }
            
            for( j = 0; j < menu->itemList.GetCount(); ++j ) {
                
                if( menu->itemList[j] != NEKO_NULL ) {
                    menu->itemList[j]->Render();
                }
            }
        }
        
        g_mainRenderer->SetBlending( false, EBlendMode::Disabled, EBlendMode::Disabled );
    }
    
    /**
     *  Unload all menus.
     */
    void UserInterface::UnloadAll()
    {
        static uint32_t i;
        
        MenuDef	* menu = NEKO_NULL;
        
        for( i = 0; i < GetLoadedMenusCount(); ++i ) {
            menu = menuList[i];
            
            if( menu == NEKO_NULL ) {
                continue;
            }
            
            // Remove items.
            pAllocator->Dealloc( menu ) ;
            menu = NEKO_NULL;
        }
        
        // Clear menu cache.
        menuList.Clear();
        
        numOpenedMenus = 0;
    }

    /**
     *  Add menu.
     */
    void UserInterface::AddMenu( MenuDef * menu )
    {
        if( menu == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "UserInterface: Attempt to add unitialized MenuDef.\n" );
            return;
        }
        
        g_Core->p_Console->Print( LOG_INFO, "UserInterface: Added menu - \"%s\"\n", menu->name );
        return menuList.PushBack( menu );
    }
    
    /**
     *  Amount of opened menus.
     */
    const uint32_t UserInterface::GetOpenedMenusCount() const {
        return numOpenedMenus;
    }
    
    /**
     *  Amount of loaded menus.
     */
    const uint32_t UserInterface::GetLoadedMenusCount() const {
        return (uint32_t)menuList.GetCount();
    }
    
    /**
     *  Find menu by name.
     */
    MenuDef * UserInterface::GetMenu( const char * name )
    {
        uint32_t i;
        
        for( i = 0; i < menuList.GetCount(); ++i ) {
            if( !strcmp( name, menuList[i]->GetName() ) ) {
                return menuList[i];
            }
        }
        
        return NEKO_NULL;
    }

    
    MenuDef *Menu_Main	= NEKO_NULL;

    static void ButtonFocus( ItemDef * self, bool state ) {
        if( state == false ) {
            self->color = Vec4( 0.92,0.92,0.92,1.0 );
            self->textColor = Vec4( 0.92,0.92,0.92,1.0 );
        }
        else {
            self->color = Vec4( 0.9,0.1,0.1,1.0 );
            self->textColor = Vec4( 0.9,0.1,0.1,1.0 );
        }
    }
    

    static void Update( MenuDef * self ) {

    }
    
    
    static void OnOpen( MenuDef * self ) {

    }
    
    static void OnClose( MenuDef *self ) {

    }

    static void CreateWorld( ItemDef *self ) {
        g_Core->p_Client->Connect( "localhost", 4004 );
        g_mainRenderer->p_UserInterface->CloseMenu( "main" );
    }
    static void CreateServerWorld( ItemDef *self ) {
        g_Core->p_Server->CreateSession();
//        p_UserInterface->CloseMenu( "main" );
    }
    

    void Init_Menu_Main()
    {
        Menu_Main = (MenuDef*)malloc(sizeof(MenuDef));//(MenuDef *)pMainAllocProxy->Alloc(sizeof(MenuDef) ) ;
        Menu_Main->Create( "main", pMainAllocProxy );
        g_mainRenderer->p_UserInterface->AddMenu( Menu_Main );
        
        Menu_Main->Update = Update;
        Menu_Main->drawCursor = true;
        
        Menu_Main->OnOpen = OnOpen;
        Menu_Main->OnClose = OnClose;

        
        ItemDef    * item = NEKO_NULL;

        item = new ItemDef();//(ItemDef *)pMainAllocProxy->Alloc( sizeof(ItemDef) ) ;
        item->Create( UI_TYPE_BUTTON, UI_ITEM_TEXT, "title" );
        Menu_Main->AddItem( item );
        
        item->frame = Vec4( 100, 240, 190, 10 );
        item->visible = true;
        item->fontSize = 10.0;
        item->textOffset = Vec2i( 0, 0 );
        item->textColor = Vec4( /*0.1,0.6,*/1.0,1.0,1.0,0.5 );
        item->textAlign = TEXTALIGN_BOTTOM;
        item->font = 0;
        item->string = "Create world ( Click me! )";
        item->OnFocus = ButtonFocus;
        item->OnAction = CreateWorld;
        
        item = new ItemDef();//(ItemDef *)pMainAllocProxy->Alloc( sizeof(ItemDef) ) ;
        item->Create( UI_TYPE_BUTTON, UI_ITEM_TEXT, "title2" );
        Menu_Main->AddItem( item );
        
        item->frame = Vec4( 100, 260, 190, 10 );
        item->visible = true;
        item->fontSize = 10.0;
        item->textOffset = Vec2i( 0, 0 );
        item->textColor = Vec4( /*0.1,0.6,*/1.0,1.0,1.0,0.5 );
        item->textAlign = TEXTALIGN_BOTTOM;
        item->font = 0;
        item->string = "Create server world ( Click me! )";
        item->OnFocus = ButtonFocus;
        item->OnAction = CreateServerWorld;
 
    }


}