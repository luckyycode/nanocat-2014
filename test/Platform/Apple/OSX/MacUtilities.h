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
//  MacUtils.h
//  OSX utilities. :P
//
//  Created by Neko Code on 9/2/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

namespace ncMacUtilities {
    
    /**
     *  Toggle mouse visibility.
     */
    void ToggleMouseVisibility( bool mVisible );
    
    /**
     *  Get endless mouse position.
     */
    void OSX_GetDeltaMousePosition( int32_t *x, int32_t *y );
    
    /**
     *  Messagebox.
     */
    void MassageBox( const char *_title, const char *_msg );

    /**
     *  Get bundle path.
     */
    const char * GetBundlePath( bool returnOnlyBundlePath = false );
    
    /**
     *  Get system version of iOS/OSX.
     */
    const char * GetSystemVersion();
};