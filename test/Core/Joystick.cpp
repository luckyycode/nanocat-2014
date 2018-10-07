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
//  Joystick.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 3/17/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#include "Joystick.h"
#include "Core.h"
#include "Utilities/Utils.h"
#include "Streams/MemoryStream.h"   // some math methods
#include "../AssetCommon/FileSystem.h"
#include "../Math/GameMath.h"   // some math methods

namespace Neko {
    
    // Item names.
    static const char * DefaultControllerItemNames[7] = {
        "leftx",
        "lefty",
        
        "rightx",
        "righty",
        
        "lefttrigger",
        "righttrigger",
        
        NEKO_NULL
    };
    
    // Button names.
    static const char * DefaultControllerButtonNames[16] = {
        "a",
        "b",
        "x",
        "y",
        
        "back",
        "guide",
        "start",
        
        "leftstick",
        "rightstick",
        
        "leftshoulder",
        "rightshoulder",
        
        "dpup",
        "dpdown",
        "dpleft",
        "dpright",
        
        NEKO_NULL
    };
    
    static SControllerMappingItem *     g_pSupportedControllers = NEKO_NULL;
    static SControllerMappingItem *     g_pXInputMapping = NEKO_NULL;
    
    /**
     *  Search for previously initialized joysticks, else create a new one.
     */
    IGamepad * IGamepad::InitSearch( const int32_t deviceIndex )
    {
//        IGamepad * gamecontroller;
//        
//        SLink * head;
//        SLink * cur;
//        
//        head = &m_JoystickList.m_sList;
//        
//        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
//            gamecontroller = (IGamepad *)cur->m_ptrData;
//            
//            // Check if it wasn't initialized yet.
//            if( gamecontroller->pHardwareInfo == NEKO_NULL ) {
//                continue;
//            } else {
//                
//                if( g_Core->pSysJoystick->GetInstanceIdByIndex( deviceIndex ) == gamecontroller->iInstanceId ) {
//                    // Already exists
//                    return gamecontroller;
//                }
//            }
//        }
        
        // If nothing was found initialize gamepad.
        Init( deviceIndex );
        return this;
    }
    
    /**
     *  Initialize a new joystick instance.
     */
    void IGamepad::Init( const int32_t deviceIndex )
    {
        const char * joystickname = NEKO_NULL;

        g_Core->m_pSysJoystick->Open( this, deviceIndex ) ;
        
        name = NEKO_NULL;
        balls = NEKO_NULL;
        
        joystickname = g_Core->m_pSysJoystick->GetNameByIndex( deviceIndex );
        if( joystickname != NEKO_NULL ) {
            name = strdup( joystickname );
        }
        
        if( iAxesNum > 0 ) {
            axes = (int16_t *) malloc( iAxesNum * sizeof(int16_t) );
            memset( axes, 0, iAxesNum * sizeof(int16_t) );
        }
        
        if( iHatsNum > 0 ) {
            hats = (uint8_t *) malloc( iHatsNum * sizeof(uint8_t) );
            memset( hats, 0, iHatsNum * sizeof(uint8_t) );
        }
        
        if( iBallsNum > 0 ) {
            balls = ( Vec2i *) malloc( iBallsNum * sizeof(Vec2i) );
            memset( balls, 0, iBallsNum * sizeof(Vec2i) );
        }
        
        if( iButtonNum > 0 ) {
            buttons = (uint8_t *) malloc( iButtonNum * sizeof(uint8_t) );
            memset( buttons, 0, iButtonNum * sizeof(uint8_t) );
        }
        
        // Check memory.
        if( ((iAxesNum > 0) && !axes) || ((iBallsNum > 0) && !balls) || ((iHatsNum > 0) && !hats) || ((iButtonNum > 0) && !buttons)) {
            g_Core->m_pSysJoystick->Close( this );
            
            return ;
        }

        powerLevel = EJoystickPowerLevel::Unknown;
        
        g_Core->p_Console->Print( LOG_INFO, "Detected joystick: \"%s\"\n", joystickname );
        
        // Add joystick to list.
        ++iRefCount;
        
        // Link the joystick in the list.
        SList::AddTail( &m_JoystickList, &m_Link, this );
    }
    
    /**
     *  Joystick axis event.
     */
    void IGamepad::OnJoystickAxisAction( uint8_t axis, int16_t value )
    {
        if( axis >= iAxesNum ) {
            return;
        }
        
        if( value == axes[axis] ) {
            return;
        }
        
        axes[axis] = value;
    
        IGamepadController * gamecontroller;
        
        SLink * head;
        SLink * cur;
        
        head = &m_ControllerList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ){
            gamecontroller = (IGamepadController *)cur->m_ptrData;
            
//            printf( "my instance id: %i looking for: %i\n", iInstanceId, gamecontroller->joystick->iInstanceId );
            
            if( gamecontroller->joystick->iInstanceId == iInstanceId ) {
                
                if( gamecontroller->mapping.controllerAxes[axis] >= (EGameControllerAxis)0 ) {
                    
                    EGameControllerAxis axis2 = gamecontroller->mapping.controllerAxes[axis];
                    int16_t value2 = value;
                    
                    // Axis buttons.
                    switch( (EGameControllerAxis)axis ) {
                        case EGameControllerAxis::TriggerLeft:
                        case EGameControllerAxis::TriggerRight:
                            value2 = value2 / 2 + 16384;
                        default:
                            break;
                    }
                    
                    
                    axis = (int8_t)axis2;
                    value = value2;
                    
                } else if( gamecontroller->mapping.controllerAxisButton[axis] >= (EGameControllerButton)0 ) {
                    OnJoystickButtonAction((int8_t)gamecontroller->mapping.controllerAxisButton[axis], nkMath::Abs(value) > 32768/2 ? 1 : 0);
                }
                
                break;
            }
        }
    }

    /**
     *  Joystick touch event.
     */
    void IGamepad::OnJoystickButtonAction( uint8_t button, uint8_t state )
    {
        switch( state ) {
            case 1:
                // down
                break;
            case 0:
                // up
                break;
            default:
                return;
        }
        
        if( button >= iButtonNum ) {
            return;
        }
        
        if( state == buttons[button] ) {
            return;
        }

        buttons[button] = state;

        IGamepadController * gamecontroller = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_ControllerList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ){
            gamecontroller = (IGamepadController *)cur->m_ptrData;
            
            if( gamecontroller->joystick->iInstanceId == iInstanceId ) {
                if( gamecontroller->mapping.controllerButton[button] >= (EGameControllerButton)0 ) {
                    
                    button = (uint8_t)gamecontroller->mapping.controllerButton[button];
                    
                } else if( gamecontroller->mapping.controllerButtonAxis[button] >= (EGameControllerAxis)0 ) {
                    
                    button = (uint8_t)gamecontroller->mapping.controllerButtonAxis[button];
                    state = state > 0 ? 255 : 0;
                }
                break;
            }
        }
    }
    
    /**
     *  Joystick hat action.
     */
    void IGamepad::OnJoystickHatAction( uint8_t hat, uint8_t value )
    {
        if( hat >= iHatsNum ) {
            return;
        }
        
        if( value == hats[hat] ) {
            return;
        }

        hats[hat] = value;

        IGamepadController * gamecontroller = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_ControllerList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            gamecontroller = (IGamepadController *)cur->m_ptrData;
            
            if( gamecontroller->joystick->iInstanceId == iInstanceId ) {
                
                // Check for the hat states.
                uint8_t bSame = gamecontroller->hatState[hat] & value;
                uint8_t bChanged = gamecontroller->hatState[hat] ^ bSame;
                
                int32_t bHighHat = hat << 4;
                
                if( bChanged & GAMEPAD_HAT_DOWN ) {
                    OnJoystickButtonAction( (int32_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_DOWN], 0);
                }
                
                if( bChanged & GAMEPAD_HAT_UP ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_UP], 0);
                }
                
                if( bChanged & GAMEPAD_HAT_LEFT ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_LEFT], 0);
                }
                
                if( bChanged & GAMEPAD_HAT_RIGHT ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_RIGHT], 0);
                }
                
                // Button press.
                
                bChanged = value ^ bSame;
                
                if( bChanged & GAMEPAD_HAT_DOWN ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_DOWN], 1);
                }
                
                if( bChanged & GAMEPAD_HAT_UP ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_UP], 1);
                }
                
                if( bChanged & GAMEPAD_HAT_LEFT ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_LEFT], 1);
                }
                
                if( bChanged & GAMEPAD_HAT_RIGHT ) {
                    OnJoystickButtonAction( (uint8_t)gamecontroller->mapping.controllerHatButton[bHighHat | GAMEPAD_HAT_RIGHT], 1);
                }
                
                gamecontroller->hatState[hat] = value;
                break;
            }
        }
    }
    
    /**
     *  Update the gamepads.
     */
    void CGameControllerBase::UpdateGamepads()
    {
        IGamepad * joystick = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_JoystickList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            joystick = (IGamepad *)cur->m_ptrData;
            
            // Update joystick's hardware side.
            g_Core->m_pSysJoystick->Update(joystick);
            
            if( joystick->bForceReset ) {
                int32_t i;
                
                // Reset states.
                for( i = 0; i < joystick->iAxesNum; ++i ) {
                    joystick->OnJoystickAxisAction( i, 0);
                }
                
                for( i = 0; i < joystick->iButtonNum; ++i ) {
                    joystick->OnJoystickButtonAction( i, 0);
                }
                
                for( i = 0; i < joystick->iHatsNum; ++i ) {
                    joystick->OnJoystickHatAction( i, GAMEPAD_HAT_CENTERED);
                }
                
                joystick->bForceReset = false;
            }
            
            // Check if joystick needs to be closed.
            if( joystick->iRefCount <= 0 ) {
                joystick->Close();
            }
        }
        
        // Detect hardware connected.
        g_Core->m_pSysJoystick->Detect( joystick );
    }

    /**
     *  Joystick axis current state.
     */
    int16_t IGamepad::GetAxis( int32_t axis )
    {
        int16_t state;
        
        if( axis < iAxesNum ) {
            state = axes[axis];
        } else {
            printf( "Joystick only has %d axes", iAxesNum );
            state = 0;
        }
        
        return state;
    }
    
    /**
     *  Joystick hat state.
     */
    uint8_t IGamepad::GetHat( int32_t hat )
    {
        uint8_t state;
        
        if( hat < iHatsNum ) {
            state = hats[hat];
        } else {
            printf("Joystick only has %d hats", iHatsNum);
            state = 0;
        }
        
        return state;
    }
    
    /**
     *  Joystick ball state.
     */
    int32_t IGamepad::GetBall( int32_t ball, int32_t *dx, int32_t *dy )
    {
        int32_t retval;
        retval = 0;
        
        if( ball < iBallsNum ) {
            if( dx != NEKO_NULL ) {
                *dx = balls[ball].x;
            }
            
            if( dy != NEKO_NULL ) {
                *dy = balls[ball].y;
            }
            
            balls[ball].x = 0;
            balls[ball].y = 0;
        } else {
            printf("Joystick only has %d balls", iBallsNum);
            return 0;
        }
        
        return retval;
    }
    
    /**
     *  Joystick button state.
     */
    uint8_t IGamepad::GetButton( int32_t button )
    {
        uint8_t state;
        
        if( button < iButtonNum ) {
            state = buttons[button];
        } else {
            printf( "Joystick only has %d buttons", iButtonNum );
            state = 0;
        }
        return state;
    }

    /**
     *  Close a joystick.
     */
    void IGamepad::Close()
    {
        if( --iRefCount > 0 ) {
            return;
        }

        g_Core->p_Console->Print( LOG_INFO, "Closing joystick \"%s\"\n", name );
        
        g_Core->m_pSysJoystick->Close( this );
        pHardwareInfo = NEKO_NULL;
        
        SList::CreateList( &m_JoystickList );
        
        free( name );
        
        // -- Check what needs to be freed.
        
        if( axes != NEKO_NULL ) {
            free( axes );
        }
        
        if( hats != NEKO_NULL ) {
            free( hats );
        }
        
        if( balls != NEKO_NULL ) {
            free( balls );
        }
        
        if( buttons != NEKO_NULL ) {
            free( buttons );
        }
    }
    
    /**
     *  Remove a joystick.
     */
    void IGamepad::Remove()
    {
        IGamepad * joystick;
        
        SLink * head;
        SLink * cur;
        
        head = &m_JoystickList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            joystick = (IGamepad *)cur->m_ptrData;
            
            joystick->iRefCount = 1;
            joystick->Close();
        }
        
        SList::CreateList( &m_JoystickList );
        
        g_Core->m_pSysJoystick->Remove( this);
    }

    /**
     *  String GUID to its struct.
     */
    JoystickGUID JoystickGetGUIDFromString( const char * guidStr )
    {
        JoystickGUID guid;
        int32_t maxoutputbytes = sizeof( guid );
        
        size_t len = strlen( guidStr );
        uint8_t * p;
        size_t i;
        
        len = (len) & ~0x1;
        
        memset( &guid, 0x00, sizeof(guid) );
        
        p = (uint8_t *)&guid;
        for( i = 0; (i < len) && ((p - (uint8_t *)&guid) < maxoutputbytes); i += 2, p++ ) {
            *p = (NekoUtils::Nibble(guidStr[i]) << 4) | NekoUtils::Nibble(guidStr[i + 1]);
        }
        
        return guid;
    }

    /**
     *  Axis string to enum item.
     */
    EGameControllerAxis GetAxisFromString( const char *dataStr )
    {
        int32_t entry;
        
        if( !dataStr || !dataStr[0] ) {
            return EGameControllerAxis::Invalid;
        }
        
        for( entry = 0; DefaultControllerItemNames[entry]; ++entry ) {
            // compare strings.. ( case dependent! )
            if( !strcasecmp( dataStr, DefaultControllerItemNames[entry] ) ) {
                return (EGameControllerAxis)entry;
            }
        }
        return EGameControllerAxis::Invalid;
    }

    /**
     *  Button string to enum item.
     */
    EGameControllerButton GetButtonFromString( const char *dataStr )
    {
        int32_t entry;
        if( !dataStr || !dataStr[0] ) {
            return EGameControllerButton::Invalid;
        }
        
        for( entry = 0; DefaultControllerButtonNames[entry]; ++entry ) {
            if( strcasecmp(dataStr, DefaultControllerButtonNames[entry]) == 0 ) {
                return (EGameControllerButton)entry;
            }
        }
        
        return EGameControllerButton::Invalid;
    }

    /**
     *  Find mappings for specified GUID.
     */
    SControllerMappingItem * GetControllerMappingForGUID( JoystickGUID * guid )
    {
        SControllerMappingItem *pSupportedController = g_pSupportedControllers;
        
        while( pSupportedController != NEKO_NULL ) {
            // see if we found what we need
            if( memcmp( guid, &pSupportedController->guid, sizeof( *guid ) ) == 0 ) {
                return pSupportedController;
            }
            
            pSupportedController = pSupportedController->next;
        }
        
        return NEKO_NULL;
    }
    
    //@todo: limits
    
    /**
     *  Parse game controller button.
     */
    void GameControllerParseButton( const char * gameButtonStr, const char * joystickStr, struct SControllerMapping * mapping )
    {
        int32_t iButton;
        
        EGameControllerButton button;
        EGameControllerAxis axis;
        
        // Info from string.
        button = GetButtonFromString( gameButtonStr );
        axis = GetAxisFromString( gameButtonStr );
        iButton = atoi( &joystickStr[1] );
        
        if( joystickStr[0] == 'a' ) {
            
            if( axis != EGameControllerAxis::Invalid ) {
                mapping->iAxes[(uint32_t)axis] = iButton;
                mapping->controllerAxes[iButton] = axis;
            } else if( button != EGameControllerButton::Invalid ) {
                mapping->iAxesButton[(uint32_t)button] = iButton;
                mapping->controllerAxisButton[iButton] = button;
            } 
            
        } else if( joystickStr[0] == 'b' ) {
            
            if( button != EGameControllerButton::Invalid ) {
                mapping->iButtons[(uint32_t)button] = iButton;
                mapping->controllerButton[iButton] = button;
            } else if( axis != EGameControllerAxis::Invalid ) {
                mapping->iButtonAxix[(uint32_t)axis] = iButton;
                mapping->controllerButtonAxis[iButton] = axis;
            }
            
        } else if( joystickStr[0] == 'h' ) {
            
            // hat and its mask
            int32_t hat = atoi( &joystickStr[1] );
            int32_t mask = atoi( &joystickStr[3] );
            
            if( button != EGameControllerButton::Invalid ) {
                int32_t ridx;
                
                mapping->buttonHat[(uint32_t)button].hat = hat;
                mapping->buttonHat[(uint32_t)button].mask = mask;
                
                ridx = (hat << 4) | mask;
                mapping->controllerHatButton[ridx] = button;
                
            } else if( axis != EGameControllerAxis::Invalid ) {
                //@nothing to do
            }
        }
    }
    
    /**
     *  Parse controller mapping.
     */
    static void GameControllerParseControllerConfigString( SControllerMapping *mapping, const char *dataStr )
    {
        char gameButtonStr[20];
        char joystickStr[20];
        bool bGameButton = true;
        
        int32_t i = 0;
        const char *pchPos = dataStr;
        
        memset( gameButtonStr, 0x0, sizeof(gameButtonStr) );
        memset( joystickStr, 0x0, sizeof(joystickStr) );
        
        while( pchPos && *pchPos ) {
            if( *pchPos == ':' ) {
                i = 0;
                bGameButton = false;
            } else if( *pchPos == ' ' ) {
                
            } else if( *pchPos == ',' ) {
                i = 0;
                bGameButton = true;
                GameControllerParseButton( gameButtonStr, joystickStr, mapping );
                
                memset( gameButtonStr, 0x0, sizeof(gameButtonStr) );
                memset( joystickStr, 0x0, sizeof(joystickStr) );
                
            } else if( bGameButton ) {
                gameButtonStr[i] = *pchPos;
                ++i;
            } else {
                joystickStr[i] = *pchPos;
                ++i;
            }
            
            ++pchPos;
        }
        
        GameControllerParseButton(gameButtonStr, joystickStr, mapping);
    }

    
    /**
     *  Load a button mapping.
     */
    void LoadButtonMapping( SControllerMapping * mapping, JoystickGUID guid, const char * mappingName, const char * pchMapping )
    {
        int32_t j;
        
        mapping->guid = guid;
        mapping->name = mappingName;
        
        for( j = 0; j < (uint32_t)EGameControllerAxis::Max; ++j ) {
            mapping->iAxes[j] = -1;
            mapping->iButtonAxix[j] = -1;
        }
        for( j = 0; j < (uint32_t)EGameControllerButton::Max; ++j ) {
            mapping->iButtons[j] = -1;
            mapping->iAxesButton[j] = -1;
            mapping->buttonHat[j].hat = -1;
        }
        
        for( j = 0; j < kMaxEntries; ++j ) {
            mapping->controllerAxes[j] = EGameControllerAxis::Invalid;
            mapping->controllerButtonAxis[j] = EGameControllerAxis::Invalid;
            mapping->controllerButton[j] = EGameControllerButton::Invalid;
            mapping->controllerAxisButton[j] = EGameControllerButton::Invalid;
        }
        
        for( j = 0; j < kMaxHatEntries; ++j ) {
            mapping->controllerHatButton[j] = EGameControllerButton::Invalid;
        }
        
        GameControllerParseControllerConfigString(mapping, pchMapping);
    }
    
    /**
     *  Look for GUID string in mapping data.
     */
    char * GetControllerGUIDFromMappingString(const char *mapping)
    {
        const char *pFirstComma = strchr( mapping, ',' );
        if( pFirstComma ) {
            char *guidStr = (char *)malloc( pFirstComma - mapping + 1 );
            if( !guidStr ) {
                printf("OutOfMemory();");
                
                return NEKO_NULL;
            }
            
            memcpy(guidStr, mapping, pFirstComma - mapping);
            guidStr[pFirstComma - mapping] = 0;
            
            return guidStr;
        }
        
        return NEKO_NULL;
    }
    
    /**
     *  Name info from mapping data.
     */
    char * GetControllerNameFromMappingString( const char * mapping )
    {
        const char * pFirstComma, * pSecondComma;
        char * mappingName;
        
        pFirstComma = strchr( mapping, ',' );
        if( pFirstComma == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        pSecondComma = strchr( pFirstComma + 1, ',' );
        if( pSecondComma == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        mappingName = (char*)malloc( pSecondComma - pFirstComma );
        if( mappingName == NEKO_NULL  ) {
            printf("GetControllerNameFromMappingString");
            return NEKO_NULL;
        }
        
        memcpy( mappingName, pFirstComma + 1, pSecondComma - pFirstComma );
        mappingName[pSecondComma - pFirstComma - 1] = 0;
        
        return mappingName;
    }
    
    /**
     *  Button info from mapping data.
     */
    char * GetControllerMappingFromMappingString( const char * mapping )
    {
        const char * pFirstComma, * pSecondComma;
        
        pFirstComma = strchr( mapping, ',' );
        if( pFirstComma == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        pSecondComma = strchr( pFirstComma + 1, ',' );
        if( pSecondComma == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        return strdup( pSecondComma + 1 );
    }
    
    /**
     *  Refresh game controller mapping.
     */
    void GameControllerRefreshMapping( SControllerMappingItem * pControllerMapping )
    {
        IGamepadController * gamecontroller = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_ControllerList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            gamecontroller = (IGamepadController *)cur->m_ptrData;
            
            if( !memcmp(&gamecontroller->mapping.guid, &pControllerMapping->guid, sizeof(pControllerMapping->guid)) ) {
                LoadButtonMapping( &gamecontroller->mapping, pControllerMapping->guid, pControllerMapping->name, pControllerMapping->mapping );
            }
        }
    }

    /**
     *  Add mapping from GUID.
     */
    static SControllerMappingItem * AddMappingForGUID( JoystickGUID jGUID, const char *mappingString, bool *existing )
    {
        char * mappingName;
        char * pchMapping;
        SControllerMappingItem *pControllerMapping;
        
        mappingName = GetControllerNameFromMappingString( mappingString );
        if( mappingName == NEKO_NULL ) {
            printf("Couldn't parse name from %s", mappingString);
            
            return NEKO_NULL;
        }
        
        pchMapping = GetControllerMappingFromMappingString( mappingString );
        if( pchMapping == NEKO_NULL ) {
            free(mappingName);
            
            printf("Couldn't parse %s", mappingString);
            return NEKO_NULL;
        }
        
        pControllerMapping = GetControllerMappingForGUID( &jGUID );
        
        if( pControllerMapping != NEKO_NULL ) {
            free( pControllerMapping->name );
            pControllerMapping->name = mappingName;
            
            free( pControllerMapping->mapping );
            pControllerMapping->mapping = pchMapping;
            
            GameControllerRefreshMapping(pControllerMapping);
            *existing = true;
        } else {
            pControllerMapping = (SControllerMappingItem *) malloc(sizeof(*pControllerMapping));
            
            if( !pControllerMapping ) {
                free( mappingName );
                free( pchMapping );
                
                return NEKO_NULL;
            }
            
            pControllerMapping->guid = jGUID;
            pControllerMapping->name = mappingName;
            pControllerMapping->mapping = pchMapping;
            pControllerMapping->next = g_pSupportedControllers;
            
            g_pSupportedControllers = pControllerMapping;
            *existing = false;
        }
        
        return pControllerMapping;
    }

    /**
     *  Add mapping entry.
     */
    int32_t GameControllerAddMapping( const char * mappingString )
    {
        char * guidStr;
        JoystickGUID jGUID;
        
        bool is_xinput_mapping = false;
        bool existing = false;
        
        SControllerMappingItem * pControllerMapping;
        
        if( mappingString == NEKO_NULL ) {
            printf( "empty mappingString" );
            return 0;
        }
        
        guidStr = GetControllerGUIDFromMappingString( mappingString );
        if( guidStr == NEKO_NULL ) {
            printf( "Couldn't parse GUID from %s", mappingString );
            return 0;
        }
        
        // Check if its Xinput
        if( !strcasecmp(guidStr, "xinput") ) {
            is_xinput_mapping = true;
        }

        jGUID = JoystickGetGUIDFromString(guidStr);
        free( guidStr );
        
        pControllerMapping = AddMappingForGUID(jGUID, mappingString, &existing);

        if( existing ) {
            return 0;
        } else {
            if( is_xinput_mapping ) {
                g_pXInputMapping = pControllerMapping;
            }

            return 1;
        }
    }
    
    const static char * kLinuxXboxStr = "none,X360 Wireless Controller,a:b0,b:b1,back:b6,dpdown:b14,dpleft:b11,dpright:b12,dpup:b13,guide:b8,leftshoulder:b4,leftstick:b9,lefttrigger:a2,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:a5,rightx:a3,righty:a4,start:b7,x:b2,y:b3,";

    /**
     *  Helper function to determine pre-calculated offset to certain joystick mappings
     */
    SControllerMappingItem * GetControllerMapping( const int32_t deviceIndex )
    {
        JoystickGUID jGUID = g_Core->m_pSysJoystick->GetDeviceGuid( deviceIndex );
        SControllerMappingItem * mapping;
        
        mapping = GetControllerMappingForGUID(&jGUID);
        
#   if defined( JOYSTICK_XINPUT )
        if( mapping == NEKO_NULL && IsXInputGamepad_DeviceIndex(deviceIndex) ) {
            mapping = g_pXInputMapping;
        }
#   endif

#   if defined( __LINUX__ )
        if( mapping != NEKO_NULL ) {
            const char *name = JoystickNameForIndex(deviceIndex);
            if( name ) {
                if( strstr(name, "Xbox 360 Wireless Receiver") ) {
                    bool existing;
                    mapping = AddMappingForGUID( jGUID,
                                                           kLinuxXboxStr,
                                                           &existing );
                }
            }
        }
#   endif
        
        if( mapping == NEKO_NULL ) {
            const char *name = g_Core->m_pSysJoystick->GetNameByIndex(deviceIndex);
            if( name != NEKO_NULL ) {
                if( strstr(name, "Xbox") || strstr(name, "X-Box") ) {
                    mapping = g_pXInputMapping;
                }
            }
        }
        
        return mapping;
    }

    /**
     *  Check if device at index is a real game controller.
     */
    bool CGameControllerBase::IsGameController( int32_t deviceIndex )
    {
        SControllerMappingItem *pSupportedController =  GetControllerMapping( deviceIndex );
        if( pSupportedController != NEKO_NULL ) {
            return true;
        }
        
        return false;
    }
    
    
    /**
     *  Get the mapped button.
     */
    uint8_t IGamepadController::GetButtonMapped( EGameControllerButton button )
    {
        const int32_t iButton = (int32_t)button;
        
        if( mapping.iButtons[iButton] >= 0 ) {
            return joystick->GetButton(mapping.iButtons[iButton] );
        } else if( mapping.iAxesButton[iButton] >= 0 ) {
            int16_t value;
            
            value = joystick->GetAxis( mapping.iAxesButton[iButton] );
            
            if( nkMath::Abs(value) > 32768 / 2 ) {
                return 1;
            }
            
            return 0;
            
        } else if( mapping.buttonHat[iButton].hat >= 0 ) {
            uint8_t value;
            value = joystick->GetHat( mapping.buttonHat[iButton].hat);
            
            if( value & mapping.buttonHat[iButton].mask ) {
                return 1;
            }
            
            return 0;
        }
        
        return 0;
    }

    /**
     *  Get the mapped axis.
     */
    int16_t IGamepadController::GetAxisMapped( Neko::EGameControllerAxis axis )
    {
        const int32_t iAxis = (const int32_t)axis;
        
        if( mapping.iAxes[iAxis] >= 0 ) {
            int16_t value = joystick->GetAxis( mapping.iAxes[iAxis] );
            
            switch( axis ) {
                // Process them almost as buttons.
                case EGameControllerAxis::TriggerLeft:
                case EGameControllerAxis::TriggerRight:
                    value = value / 2 + 16384;
                default:
                    break;
            }
            
            return value;
            
        } else if( mapping.iButtonAxix[iAxis] >= 0 ) {
            uint8_t value;
            
            value = joystick->GetButton( mapping.iButtonAxix[iAxis] );
            if( value > 0 ) {
                return 32767;
            }
            
            return 0;
            
        }
        return 0;
    }

    /**
     *  Check controller existance.
     */
    IGamepadController * CheckControllerExistance( const int32_t deviceIndex )
    {
        if( m_ControllerList.m_iCount == 0 ) {
            return NEKO_NULL;
        }
        
        IGamepadController * gamecontroller;
        
        SLink * head;
        SLink * cur;
        
        head = &m_ControllerList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            gamecontroller = (IGamepadController *)cur->m_ptrData;
            
            if( gamecontroller == NEKO_NULL ) {
                continue;
            }
            
            if( gamecontroller->joystick == NEKO_NULL ) {
                continue;
            }
            
            if( gamecontroller->pSysJoystick->GetInstanceIdByIndex(deviceIndex) == gamecontroller->joystick->iInstanceId ) {
                ++gamecontroller->iRefCount;
                
                return gamecontroller;
            }
        }
        
        return NEKO_NULL;
    }

    /**
     *  Open a new controller.
     */
    void IGamepadController::OpenController( const int32_t deviceIndex )
    {
        SControllerMappingItem * pSupportedController = NEKO_NULL;

        // Find a mapping.
        pSupportedController =  GetControllerMapping(deviceIndex);
        if( pSupportedController == NEKO_NULL ) {
            printf("Couldn't find mapping for device (%d)", deviceIndex);
            
            return;
        }
        
        // Create joystick.
        joystick = new IGamepad();
        joystick = joystick->InitSearch(deviceIndex);
  
        // Load mappings.
        LoadButtonMapping( &mapping, pSupportedController->guid, pSupportedController->name, pSupportedController->mapping );
        
        ++iRefCount;
        SList::AddHead( &m_ControllerList, &m_Link, this );

        // Initial update.
        g_Core->m_pSysJoystick->Update( joystick );
    }

    /**
     *  Get the joystick binding.
     */
    EGameControllerButtonBind IGamepadController::GetBindForAxis( EGameControllerAxis axis)
    {
        EGameControllerButtonBind bind;
        memset( &bind, 0x0, sizeof(bind) );
        
        if(  axis == EGameControllerAxis::Invalid ) {
            return bind;
        }
     
        const uint32_t iAxis = (const uint32_t)axis;
        
        if( mapping.iAxes[iAxis] >= 0 ) {
            bind.bindType = EGameControllerBindType::Axis;
            bind.value.button = mapping.iAxes[iAxis];
        } else if( mapping.iButtonAxix[iAxis] >= 0 ) {
            bind.bindType = EGameControllerBindType::Button;
            bind.value.button = mapping.iButtonAxix[iAxis];
        }
        
        return bind;
    }
    
    
    /**
     *  Get the joystick binding.
     */
    EGameControllerButtonBind IGamepadController::GetBindForButton( EGameControllerButton button)
    {
        EGameControllerButtonBind bind;
        memset( &bind, 0x0, sizeof(bind) );
        
        if(  button == EGameControllerButton::Invalid ) {
            return bind;
        }
        
        if( mapping.iButtons[(uint32_t)button] >= 0 ) {
            bind.bindType = EGameControllerBindType::Button;
            bind.value.button = mapping.iButtons[(uint32_t)button];
        } else if( mapping.iAxesButton[(uint32_t)button] >= 0 ) {
            bind.bindType = EGameControllerBindType::Axis;
            bind.value.axis = mapping.iAxesButton[(uint32_t)button];
        } else if( mapping.buttonHat[(uint32_t)button].hat >= 0 ) {
            bind.bindType = EGameControllerBindType::Hat;
            bind.value.hat.hat = mapping.buttonHat[(uint32_t)button].hat;
            bind.value.hat.hat_mask = mapping.buttonHat[(uint32_t)button].mask;
        }
        
        return bind;
    }
    
    /**
     *  Close game controller.
     */
    void IGamepadController::CloseController()
    {
        if( joystick == NEKO_NULL ) {
            return;
        }
        
        joystick->Close();
        delete joystick;
        
        SList::RemoveAt( &m_ControllerList, &m_Link );
    }
    
    /**
     *  Quit the game controller system.
     */
    void CGameControllerBase::RemoveControllers()
    {
        SControllerMappingItem *pControllerMap;
        IGamepadController *gamecontroller;
        
        SLink * head;
        SLink * cur;
        
        head = &m_ControllerList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            gamecontroller = (IGamepadController *)cur->m_ptrData;
            
            gamecontroller->iRefCount = 1;
            gamecontroller->CloseController();
        }

        // that should be removed on the application quit.
        while( g_pSupportedControllers ) {
            pControllerMap = g_pSupportedControllers;
            g_pSupportedControllers = g_pSupportedControllers->next;
            
            free( pControllerMap->name );
            free( pControllerMap->mapping );
            free( pControllerMap );
        }
    }
    
    /**
     *  Initialise game controller system.
     */
    void CGameControllerBase::Init()
    {
        if( g_Core->m_pSysJoystick == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "Couldn't initialize game controller interface\n" );
            return;
        }
        
        char    * line;
        int32_t     charsRead;
        
        SPackFile   * package = NEKO_NULL;
        AssetDataPool asset;
        
        const char * presetFilename = NekoCString::STR("presets/input/%s.pres", Gamepad_Preset->Get<const char*>() );
        
        package = g_Core->p_FileSystem->GetPak( "shared_data" );
        asset = package->GetData( presetFilename );
        
#define MAX_PRESET_FILE_LENGTH  16384
        
        // Parse asset list now.
        line = (char *)PushMemory( asset.tempPool, sizeof(char) * MAX_PRESET_FILE_LENGTH ); // Temporary line for per line parsing.
        
        charsRead = 0;
        
        // Read line by line.
        while( str_readline( line, MAX_PRESET_FILE_LENGTH, (const char *)asset.tempData, charsRead ) && line != NEKO_NULL ) {
            GameControllerAddMapping( line );
            line[0] = '\0'; // clear previous data
        }
        
        // Don't forget to..
        _PopMemoryFrame( asset.tempPool );

        // Controller list.
        SList::CreateList( &m_ControllerList );
        
        pSysJoystick = g_Core->m_pSysJoystick; // system interface
        // Initialize joystick..
        pSysJoystick->Init();
    }
    
    /*
     * Find the joystick that owns this instance id
     */
    IGamepad * JoystickFromInstanceID( SIJoystickID joyid )
    {
        IGamepad * joystick;
        
        SLink * head;
        SLink * cur;
        
        head = &m_JoystickList.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            joystick = (IGamepad *)cur->m_ptrData;
            
            if( joystick->iInstanceId == joyid ) {
                return joystick;
            }
        }
        
        return NEKO_NULL;
    }

}