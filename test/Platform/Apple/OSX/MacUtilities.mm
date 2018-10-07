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
//  MacUtils.m
//  OSX Utilities. :P
//
//  Created by Neko Code on 9/2/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "SystemShared.h"

#ifndef iOS_BUILD

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#import "MacUtilities.h"

namespace ncMacUtilities {
    /**
     *   Get mouse position.
     */
    void OSX_GetDeltaMousePosition( int32_t *x, int32_t *y )
    {
        CGGetLastMouseDelta( x, y );
    }

    // TODO: Message box class for different kinds of platforms.
    
    /**
     *   Popup window.
     */
    void MassageBox( const char *_title, const char *_msg )
    {
        NSString * message = [NSString stringWithFormat:@"%s", _msg];
        NSString * title = [NSString stringWithFormat:@"%s", _title];

        // All Cocoa related thingies must be launched on the main thread!
        dispatch_sync(dispatch_get_main_queue(), ^()
        {
            // Ignore this damned warning.
            NSAlert * alert = [[NSAlert alloc] init];
            
            [alert setMessageText:title];
//            [alert setShowsHelp:YES];
            
            [alert setInformativeText:message];
            [alert setAlertStyle:NSWarningAlertStyle];
            
            [alert addButtonWithTitle:@"Okay"];
            
            id appDelegate = [NSApp delegate];
            
            [NSApp setDelegate: nil];
            
            // Show a mouse cursor.
            CGDisplayShowCursor( kCGDirectMainDisplay );
            CGAssociateMouseAndMouseCursorPosition( true );
            
            [alert runModal];
            [NSApp setDelegate: appDelegate];
        });

    }
    
    /**
     *   Since that thing ( getcwd ) won't work properly,
     *   we need to write custom path finder.
     */
    const char * GetBundlePath( bool returnOnlyBundlePath )
    {
        const char  *fs_path;
        
        NSString *bundlePath = [[NSBundle mainBundle] resourcePath];
    
        if( !returnOnlyBundlePath ) {
            NSString *secondParentPath = [[bundlePath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
            NSString *thirdParentPath = [[secondParentPath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
        
            fs_path = [thirdParentPath UTF8String];
        } else {
            fs_path = [bundlePath UTF8String];
        }
    
    
        return fs_path;
    }
    
    /**
     *   Current Mac/iOS version running.
     */
    const char * GetSystemVersion()
    {
        NSString *operatingSystemVersionString  = [[NSProcessInfo processInfo] operatingSystemVersionString ];
        return [operatingSystemVersionString  UTF8String];
    }

    /**
     *  Toggle mouse visibility.
     */
    void ToggleMouseVisiblity( bool mVisible )
    {
        if( mVisible ) {
            // Show mouse cursor.
            CGDisplayShowCursor( kCGDirectMainDisplay );
            CGAssociateMouseAndMouseCursorPosition( true );
        } else {
            CGDisplayHideCursor( kCGDirectMainDisplay );
            CGAssociateMouseAndMouseCursorPosition( false );
        }
    }
}


#endif