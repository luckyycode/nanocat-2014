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
//  ItemDef.h
//  An item used in MenuDef property.
//
//  Created by Kawaii Neko on 8/3/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//


#ifndef _ITEMDEF_
#define _ITEMDEF_

#include "../../AssetCommon/Material/MaterialLoader.h" // Materials.
#include "../../Math/GameMath.h" // Accessing some 2d stuff.
#include "../../Math/Vec4.h"

namespace Neko {
    
    // UI items.
#define UI_ITEM_IMAGE		2
#define UI_ITEM_BORDER		4
#define UI_ITEM_TEXT		8
    
#define UI_TEXTALIGN_DEFAULT        1
#define UI_TEXTALIGN_LEFT           2
#define UI_TEXTALIGN_RIGHT          4
#define UI_TEXTALIGN_BOTTOM         8
#define UI_TEXTALIGN_TOP            16
#define UI_TEXTALIGN_LEFT_BOTTOM    32
#define UI_TEXTALIGN_LEFT_TOP       64
#define UI_TEXTALIGN_RIGHT_BOTTOM   128
#define UI_TEXTALIGN_RIGHT_TOP      256
    
    // 0, 0 - Default
    // 1, 0 - Left
    // 2, 0 - Right
    
    // 0, 0 - Default
    // 0, 1 - Bottom
    // 0, 2 - Top
    
    // 1, 1 - Left bottom
    // 1, 2 - Left top
    // 2, 1 - Right bottom
    // 2, 2 - Right top
    
    static const Vec2i TEXTALIGN_DEFAULT( 0, 0 );
    
    static const Vec2i TEXTALIGN_LEFT( 1, 0 );
    static const Vec2i TEXTALIGN_RIGHT( 2, 0 );
    
    static const Vec2i TEXTALIGN_BOTTOM( 0, 1 );
    static const Vec2i TEXTALIGN_TOP( 0, 2 );
    
    static const Vec2i TEXTALIGN_LEFT_BOTTOM( 1, 1 );
    static const Vec2i TEXTALIGN_LEFT_TOP( 1, 2 );
    static const Vec2i TEXTALIGN_RIGHT_BOTTOM( 2, 1 );
    static const Vec2i TEXTALIGN_RIGHT_TOP( 2, 2 );
    
    
    struct SMaterial;
    
    /**
     *  UI item types.
     */
    enum UserInterfaceItemType
    {
        UI_TYPE_DEFAULT = 0,
        UI_TYPE_BUTTON,
        UI_TYPE_DUMMY
    };
    
    
    /**
     *  Items used in menus.
     */
    class ItemDef
    {
    public:
        ItemDef();
        ItemDef( UserInterfaceItemType type, int flags = 0, const char * name = NEKO_NULL );
        
        ~ItemDef();
        
        /**
         *  Create a new ItemDef.
         */
        void Create( UserInterfaceItemType type, int flags = 0, const char * name = NEKO_NULL );
        
        /**
         *  Update item.
         */
        void			Frame();
        
        /**
         *  Draw item.
         */
        void			Render();
        
        /**
         *  Reset item ( initial state ).
         */
        void			Reset();
        
        /**
         *  Clear item.
         */
        void			Clear();
        
        /**
         *  Is mouse over this item?
         */
        bool				IsMouseOver( int x, int y, int w, int h );
        bool				IsMouseOver( Vec4 & rect );
        
    public:
        
        /**
         *  Item type.
         */
        UserInterfaceItemType		itemType;
        
        /**
         *  Item flags.
         */
        int             flags;
        
        /**
         *  Is item visible?
         */
        bool				visible;
        
        /**
         *  Is item active? ( focused )
         */
        bool				focused;
        
        /**
         *  Is positive frame?
         */
        bool				positiveOffset;
        
        /**
         *  Item frame rectangle.
         */
        Vec4				frame;
        Vec4				collFrame;
        
        /**
         *  Border size ( if have any ).
         */
        float                       borderSize;
        
        /**
         *  Border color in RGBA ( if have any ).
         */
        Vec4				borderColor;
        
        /**
         *  Text alignment.
         */
        Vec2i                 textAlign;
        
        /**
         *  Text color in RGBA.
         */
        Vec4				textColor;
        
        /**
         *  Text offset.
         */
        Vec2i                 textOffset;
        
        /**
         *  Font size.
         */
        float				fontSize;
        
        /**
         *  Font type.
         */
        int					font;			// enum UI_FONT_TYPE
        
        /**
         *  Material image.
         */
        SMaterial          * material;
        
        /**
         *  Material color in RGBA.
         */
        Vec4		color;
        
        const char *			name;
        const char *			string;
        
        /**
         *  Callback for mouse touch.
         */
        void				(*OnFocus)( ItemDef * self, bool state );
        
        /**
         *  Callback for mouse interaction ( left click ).
         */
        void				(*OnAction)( ItemDef * self );
        
        /**
         *  Callback for mouse interaction ( right click ).
         */
        void				(*OnSecondaryAction)( ItemDef *self );
        
    };

}

#endif /*_ITEMDEF_*/
