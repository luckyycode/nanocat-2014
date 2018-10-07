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
//  ItemDef.cpp
//  An item used in MenuDef property.
//
//  Created by Kawaii Neko on 8/3/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//


#include "../../Graphics/Renderer/Renderer.h" // Drawing & stuff.
#include "../Player/Input/Input.h" // Input.
#include "../../Graphics/Renderer/FontRenderer.h" // Font renderer.
//#include "../../World/Mesh.h" // Quad data.
#include "../../AssetCommon/Sound/SoundManager.h"
#include "../Player/Camera/Camera.h"
#include "../../AssetCommon/Material/MaterialLoader.h" // Materials.
#include "ItemDef.h"

namespace Neko {
    /**
     *  Constructor.
     */
    ItemDef::ItemDef()
    {
        Reset();
    }
    
    /**
     *  Create a new item.
     */
    ItemDef::ItemDef( UserInterfaceItemType type, int32_t flags, const char * name )
    {
        Create( type, flags, name );
    }
    
    /**
     *  Create a new ItemDef.
     */
    void ItemDef::Create( UserInterfaceItemType type, int32_t flags, const char * name )
    {
        Reset();
        
        this->name = name;
        this->itemType = type;
        this->flags = flags;
    }
    
    /**
     *  Destructor.
     */
    ItemDef::~ItemDef()
    {
        Clear();
        
        this->name = "";
        this->itemType = itemType;
        this->flags = 0;
    }
    
    /**
     *  Clear this item.
     */
    void ItemDef::Clear()
    {
        
    }
    
    /**
     *  Reset this item.
     */
    void ItemDef::Reset()
    {
        Clear();
        
        this->name = "";
        this->visible = true;
        
        this->borderSize = 1.0f;
        this->borderColor = Vec4( 1.0f );
        
        this->string = "";
        this->textColor = Vec4( 1.0f );
        this->fontSize = 1;
        
        this->color = Vec4( 1.0f );
        
        this->OnFocus = NEKO_NULL;
        this->OnAction = NEKO_NULL;
        this->OnSecondaryAction = NEKO_NULL;
    }
    
    /**
     *  Update item.
     */
    void ItemDef::Frame()
    {
        if( !visible )
        {
            return;
        }
        
        // Sizes.
        if( frame.z > 0.0f && frame.w > 0.0f )
        {
            positiveOffset = true;
        }
        else
        {
            positiveOffset = false;
        }
        
        collFrame = frame;
        g_mainRenderer->AdjustElemSize( &collFrame.x, &collFrame.y, &collFrame.z, &collFrame.w );
    
        // Check if mouse touches our item.
        if( positiveOffset && IsMouseOver( collFrame ) )
        {
            focused = true;
        }
        else focused = false;
        
        // Action on mouse cursor focus.
        if( OnFocus != NEKO_NULL )
        {
            OnFocus( this, focused );
        }
        
        switch( itemType )
        {
            // -- Button item.
            case UI_TYPE_BUTTON:
                if( focused )
                {
                    if( g_Core->p_Input->IsMouseButtonPressed( MB_LEFTKEY ) && OnAction != NEKO_NULL )
                    {
                        g_Core->p_SoundSystem->PlaySoundAt( "scissors", g_Core->p_Camera->vEye, 1.0f, false, false );
                        
                        OnAction( this );
                        
                        g_Core->p_Input->mouseButtonStates[MB_LEFTKEY] = false;
                        focused = false;
                    }
                    else if( g_Core->p_Input->IsMouseButtonPressed( MB_RIGHTKEY ) && OnSecondaryAction != NEKO_NULL )
                    {
                        // Second button action.
                        OnSecondaryAction( this );
                        
                        g_Core->p_Input->mouseButtonStates[MB_RIGHTKEY] = false;
                        focused = false;
                    }
                }
                break;
                
            case UI_TYPE_DEFAULT:
                // Nothing to do here (yet).
                break;
                
            default:
                break;
        }
    }
    
    /**
     *  Render item.
     */
    void ItemDef::Render()
    {
        if( !visible )
            return;
       
        // If we can see it..
        if( positiveOffset )
        {
            // Material..
            if( ( flags & UI_ITEM_IMAGE ) )
            {
                if( material != NEKO_NULL )
                {
                    //mat->Bind();
                    g_mainRenderer->BindTexture( 0, material->Image.GetId() );
                    g_mainRenderer->RenderQuadAt( frame + Vec4( material->Image.Width, material->Image.Height, 0.0f, 0.0f ), color );
                }
                else {
                    g_mainRenderer->UnbindTexture( 0 );
                    g_mainRenderer->RenderQuadAt( frame, color );
                }
            }
            
            // Border.
            if( ( flags & UI_ITEM_BORDER ) )
            {
                // Left line
                Vec4 leftLine = Vec4( frame.x - borderSize, frame.y - borderSize, borderSize, frame.w + (borderSize * 2) );
                // Right line
                Vec4 rightLine = Vec4( frame.x + frame.z, frame.y - borderSize, borderSize, frame.w + (borderSize * 2) );
                // Top line
                Vec4 topLine = Vec4( frame.x, frame.y - borderSize, frame.z, borderSize );
                // Bottom line
                Vec4 bottomLine = Vec4( frame.x, frame.y + frame.w, frame.z, borderSize );
                
                // TODO: render line
                g_mainRenderer->RenderQuadAt( leftLine, borderColor );
                g_mainRenderer->RenderQuadAt( rightLine, borderColor );
                g_mainRenderer->RenderQuadAt( topLine, borderColor );
                g_mainRenderer->RenderQuadAt( bottomLine, borderColor );
            }
        }
        
        
        // String.
        if( (flags & UI_ITEM_TEXT) )
        {
            g_mainRenderer->p_CoreFont->DrawString( textColor, frame.x + textOffset.x, frame.y + textOffset.y, fontSize, textAlign, string );
        }
    }
    
    /**
     *  Does mouse touch item frame?
     */
    bool ItemDef::IsMouseOver( int32_t x, int32_t y, int32_t w, int32_t h )
    {
        static Vec2i point[2];
        
        point[0] = Vec2i( x, y );
        point[1] = Vec2i( x + w, y + h );
        
        int32_t mousex = g_Core->p_Input->GetMouseX();
        int32_t mousey = g_Core->p_Input->GetMouseY();
        
        if( mousex >= point[0].x && mousey >= point[0].y &&
           /* Rect */
           mousex <= point[1].x && mousey <= point[1].y )
        {
            // In rectangle.
            return true;
        }
        
        return false;
    }
    
    bool ItemDef::IsMouseOver( Vec4 & rect )
    {
        return IsMouseOver( (int)rect.x, (int)rect.y, (int)rect.z, (int)rect.w );
    }
}