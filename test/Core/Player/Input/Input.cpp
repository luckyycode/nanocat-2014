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
//  Input.cpp
//  Game input.. :}
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../../ScriptSupport/Scripting.h"
#include "../../Core.h"
#include "../../../Platform/Shared/System.h"
#include "../../Console/Console.h"
#include "Input.h"
#include "../Camera/Camera.h"
#include "../../../Platform/Shared/SystemShared.h"
// Temp.
#include "../../../World/BeautifulEnvironment.h"
#include "../../Network/MultiplayerServer.h"
#include "../../Network/MultiplayerClient.h"
#include "../../UserInterface/UserMenu.h"

namespace Neko {

    /**
     *  Input instance.
     */
    CInputInterface::CInputInterface() : x( 0 ), y( 0 )
    {
        
    }

    /**
     *   Initialize user input stuff.
     */
    void CInputInterface::Initialize()
    {
        g_Core->p_Console->Print( LOG_INFO, "Initializing input..\n" );

        SetMouseX( 0 );
        SetMouseY( 0 );

        mouseButtonStates[MB_LEFTKEY] = false;
        mouseButtonStates[MB_RIGHTKEY] = false;
        
        uint32_t    i;
        
        // Reset key states.
        
        for( i = 0; i < MAX_KEYBOARD_KEYS; ++i ) {
            keysBase[i].kHold = false;
        }
    }

    /**
     *  Key press handle.
     */
    void CInputInterface::OnKeyPress( char key )
    {
#ifdef _WIN32 // WTF?
        key += 32;
#endif
        if( (int32_t)key >= MAX_KEYBOARD_KEYS ) {
            return;
        }
        
        i_lastPressedKey = key;
        keysBase[key].kHold = true;

        // System keys.
        switch( key )
        {
            case 'u':
                g_Core->p_Camera->SetUpdateFrustum( true );
                break;

            case 'i':
                g_Core->p_Camera->SetUpdateFrustum( false );
                break;

            case 'c':
                Render_Secret =  true ;
                break;

            case 'b':
                Render_Secret2 = !Render_Secret2 ;
                break;

			case 'l':


				g_Core->p_Server->CreateSession();
				g_Core->p_Client->Connect("localhost", 4004);
				g_mainRenderer->p_UserInterface->CloseMenu("main");
				break;
        }
    }

    /**
     *   Handles key release.
     */
    void CInputInterface::OnKeyUp( char key )
    {
#ifdef _WIN32 // WTF?
        key += 32;
#endif
        i_lastReleasedKey = key;

        keysBase[key].kHold = false;
    }

    /**
     *  Mouse move.
     */
    void CInputInterface::OnMouseMove( int32_t x, int32_t y )
    {
        SetMouseX( x );
        SetMouseY( y );
    }

    /**
     *  Mouse key press.
     */
    void CInputInterface::OnMouseDown( int32_t x, int32_t y, int32_t key )
    {
        mouseButtonStates[key] = true;
    }

    /**
     *  Mouse key release.
     */
    void CInputInterface::OnMouseUp( int32_t x, int32_t y, int32_t key )
    {
        mouseButtonStates[key] = false;
    }

    /**
     *  Get mousey X.
     */
    const int32_t CInputInterface::GetMouseX()
    {
        return this->x;
    }

    /**
     *  Get mousey Y.
     */
    const int32_t CInputInterface::GetMouseY()
    {
        return this->y;
    }

    /**
     *  Set mousey X.
     */
    void CInputInterface::SetMouseX( uint32_t value )
    {
        x = value;
    }

    /**
     *  Set mousey Y.
     */
    void CInputInterface::SetMouseY( uint32_t value )
    {
        y = value;
    }

    /**
     *  Mouse scroll handler.
     */
    void CInputInterface::OnMouseScroll( bool isScrollingDown )
    {
        
    }
    
    /**
     *  Get key character.
     */
    char CInputInterface::GetKeyFromNum( uint32_t key )
    {
        // mac keycodes
        //12 |13 |14 |15 |17 |16 |32 |34 |31 |35 |33 |30 |0 |1 |2 |3 |5 |4 |38 |40 |37 |41 |39 |42 |6 |7 |8 |9 |11 |45 |46 |43 |47 |44 |53 |
        //18 |19 |20 |21 |23 |22 |26 |28 |25 |29 |53 |
        
#ifdef __APPLE__
        switch( key )
        {
                
            case 18: return '1';
            case 19: return '2';
            case 20: return '3';
            case 21: return '4';
            case 23: return '5';
            case 22: return '6';
            case 26: return '7';
            case 28: return '8';
            case 25: return '9';
            case 29: return '0';
                
            case 12: return 'q';
            case 13: return 'w';
            case 14: return 'e';
            case 15: return 'r';
            case 17: return 't';
            case 16: return 'y';
            case 32: return 'u';
            case 34: return 'i';
            case 31: return 'o';
            case 35: return 'p';
            case 33: return '[';
            case 30: return ']';
            case 0: return 'a';
            case 1: return 's';
            case 2: return 'd';
            case 3: return 'f';
            case 5: return 'g';
            case 4: return 'h';
            case 38: return 'j';
            case 40: return 'k';
            case 37: return 'l';
            case 41: return ':';
            case 39: return 'g';
            case 42: return '\\';
            case 6: return 'z';
            case 7: return 'x';
            case 8: return 'c';
            case 9: return 'v';
            case 11: return 'b';
            case 45: return 'n';
            case 46: return 'm';
            case 43: return ',';
            case 47: return '.';
            case 44: return '/';
            case 49: return ' ';
            case 51: return ' ';
            case 27: return '_';
                
            default:
                return ' ';
        }
#elif __linux__
        switch( key )
        {
            case 94: return '<';
            case 10: return '1';
            case 11: return '2';
            case 12: return '3';
            case 13: return '4';
            case 14: return '5';
            case 15: return '6';
            case 16: return '7';
            case 17: return '8';
            case 18: return '9';
            case 19: return '0';
            case 20: return '_'; // Fix me
            case 21: return '=';
            case 24: return 'q';
            case 25: return 'w';
            case 26: return 'e';
            case 27: return 'r';
            case 28: return 't';
            case 29: return 'y';
            case 30: return 'u';
            case 31: return 'i';
            case 32: return 'o';
            case 33: return 'p';
            case 34: return '[';
            case 35: return ']';
            case 38: return 'a';
            case 39: return 's';
            case 40: return 'd';
            case 41: return 'f';
            case 42: return 'g';
            case 43: return 'h';
            case 44: return 'j';
            case 45: return 'k';
            case 46: return 'l';
            case 47: return ';';
            case 48: return '"';
            case 51: return '\\';
            case 49: return '`';
            case 52: return 'z';
            case 53: return 'x';
            case 54: return 'c';
            case 55: return 'v';
            case 56: return 'b';
            case 57: return 'n';
            case 58: return 'm';
            case 59: return ',';
            case 60: return '.';
            case 61: return '/';
                
            default:
                return ' ';
        }
        
#elif _WIN32
        switch( key )
        {
            case 129: return '~';
            case 49: return '1';
            case 50: return '2';
            case 51: return '3';
            case 52: return '4';
            case 53: return '5';
            case 54: return '6';
            case 55: return '7';
            case 56: return '8';
            case 57: return '9';
            case 48: return '0';
            case 189: return '_';
                // 8 - backspace
            case 81: return 'q';
            case 87: return 'w';
            case 69: return 'e';
            case 82: return 'r';
            case 84: return 't';
            case 89: return 'y';
            case 85: return 'u';
            case 73: return 'i';
            case 79: return 'o';
            case 80: return 'p';
            case 219: return '[';
            case 221: return ']';
            case 65: return 'a';
            case 83: return 's';
            case 68: return 'd';
            case 70: return 'f';
            case 71: return 'g';
            case 72: return 'h';
            case 74: return 'j';
            case 75: return 'k';
            case 76: return 'l';
            case 186: return ';';
            case 222: return '\'';
            case 220: return '\\';
            case 226: return '|';
            case 90: return 'z';
            case 88: return 'x';
            case 67: return 'c';
            case 86: return 'v';
            case 66: return 'b';
            case 78: return 'n';
            case 77: return 'm';
            case 188: return ',';
            case 190: return '.';
            case 191: return '/';
                // 32 - space
                
            default: return ' ';
        }
#endif // _WIN32
        
        return ' ';
    }
}
