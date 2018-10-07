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
//  Input.h
//  Game input.. :}
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef input_h
#define input_h

namespace Neko {

    /**
     *  Keys base.
     */
    struct ncKeyboardCharBase
    {
        bool kHold;
    } ;
    
    
    /**
     *  Mouse keys.
     */
    static const int32_t MOUSE_KEYS_COUNT = 2;
    
    static const uint32_t  MAX_KEYBOARD_KEYS = 512;
    
    /**
     *  Mouse button states.
     */
    enum MouseButtons
    {
        MB_LEFTKEY = 0,
        MB_RIGHTKEY = 1 // Don't mess up with Windows 'MB_RIGHT' property.
    };
    
    /**
     *  Input manager.
     */
    class CInputInterface
    {
        NEKO_NONCOPYABLE( CInputInterface );
        
    public:
        CInputInterface();
        
        /**
         *  Initialize input.
         */
        void Initialize( void );
        
        /**
         *  Key event handlers ( on press ).
         */
        void OnKeyPress( char key );
        
        /**
         *  Key event handlers ( on release ).
         */
        void OnKeyUp( char key );
        
        /**
         *  On mouse move handler.
         */
        void OnMouseMove( int x, int y );
        
        /**
         *  Mouse key press event.
         */
        void OnMouseDown( int x, int y, int32_t key );
        
        /**
         *  Mouse key release event.
         */
        void OnMouseUp( int x, int y, int32_t key );
        
        /**
         *  Mouse wheel scroll.
         */
        void OnMouseScroll( bool isScrollingDown );
        
        /**
         *  Get key from keycode.
         */
        char GetKeyFromNum( const uint32_t key );
        
        
        /**
         *  Get mouse X value.
         */
        const int32_t GetMouseX( void );
        
        /**
         *  Get mouse Y value.
         */
        const int32_t GetMouseY( void );
        
//        bool IsMouseHold( void );
//        void SetMouseHold( bool value );
        void SetMouseY( const uint value );
        void SetMouseX( const uint value );
        
        /**
         *  Keys base.
         */
        ncKeyboardCharBase keysBase[MAX_KEYBOARD_KEYS];
        
        /**
         *  Get last pressed key.
         */
        inline char GetLastPressedKey()
        {
            return i_lastPressedKey;
        }
        
        /**
         *  Get last released key.
         */
        inline char GetLastReleasedKey()
        {
            return i_lastReleasedKey;
        }
        
        /**
         *  Is mouse key pressed?
         */
        inline bool IsMouseButtonPressed( int button )
        {
            return mouseButtonStates[button];
        }
        
        bool mouseButtonStates[MOUSE_KEYS_COUNT];
        
        // Default key states.
        bool    m_bMovingForward;
        bool    m_bMovingBackward;
        bool    m_bMovingLeft;
        bool    m_bMovingRight;
    
        
    private:
        // Mouse stuff.
//        bool Holding;

        
        uint32_t x;
        uint32_t y;
        
        char i_lastPressedKey, i_lastReleasedKey;
    };
}

#endif
