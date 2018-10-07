
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
//  AppDelegate.m
//  Application delegate for OSX. *.*
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#import "AppDelegate.h"

#ifndef iOS_BUILD

#import "MacUtilities.h"
#import "../../Core/Core.h"
#import "../../Core/Player/Input/Input.h"
#import "../../Core/Player/Camera/Camera.h"
#import "../../Platform/Shared/System.h"

@implementation NekoApplication

// From http://cocoadev.com/index.pl?GameKeyboardHandlingAlmost
// This works around an AppKit bug, where key up events while holding
// down the command key don't get sent to the key window.
- (void)sendEvent:(NSEvent *)event
{
    if( event == nil || event == (void*)0xbebebebe ) {
        return;
    }
    
    if ([event type] == NSKeyUp && ([event modifierFlags] & NSCommandKeyMask))
        [[self keyWindow] sendEvent:event];
    else
        [super sendEvent:event];
}

@end

@implementation NekoApplicationDelegate : NSObject

void PostEmptyEvent(void)
{
    @autoreleasepool
    {
        NSEvent * event = [NSEvent otherEventWithType:NSApplicationDefined
                                             location:NSMakePoint(0, 0)
                                        modifierFlags:0
                                            timestamp:0
                                         windowNumber:0
                                              context:nil
                                              subtype:0
                                                data1:0
                                                data2:0];
        [NSApp postEvent:event atStart:YES];
        
    }
}


/**
 *  Here we initialize engine after main and NSApplication pool have been invoked.
 */
- (id)init
{
    if( self = [super init] )
    {
        // Allocate and initialize window and stuff here ..
        NSLog( @"init() is about to initialize OSX Base.....\n\n" );

        // Create Mac console window.
        //CreateConsole();

        // Create OSX window.
        osx_mwindow = new Neko::OSXWindow();
        
        _eventDelegatingView = [[EventDelegatingView alloc] init];
        _eventDelegatingView.eventDelegate = self;
        
        osx_mwindow->Initialize();
        osx_mwindow->base->eventDelegatingView = _eventDelegatingView;
        
        window = osx_mwindow->base->hWindow;
        
        PostEmptyEvent();
    }
    return self;
}

/************************************************
 *
 *             EVENT PARSING   :OO
 *
 ************************************************/


- (void)view:(NSView *)aView didHandleEvent:(NSEvent *)anEvent
{
//    osx_mwindow->base->m_threadLock->Lock();
    
    NSPoint location;
    
    NSEvent * event = anEvent;
    
    
    NSPoint lastPoint;      // Last mouse X,Y point.
    NSPoint lastRelativePoint;  // Last mouse X,Y point corrected to view size ( for correct UI usage ).
    
    
    // Update relative point.
    lastPoint.x = Neko::g_Core->p_Camera->_lastXt;
    lastPoint.y = Neko::g_Core->p_Camera->_lastYt;
   
    // Events.
    switch ([event type])
    {
        case NSKeyDown:
            
            if( (int32_t)[[event charactersIgnoringModifiers] UTF8String][0] == 27 ) {
                // ESC Exits
                //                        bQuit = true;
            }
            
            Neko::g_Core->p_Input->OnKeyPress( [[event charactersIgnoringModifiers] UTF8String][0] );
            
            break;
            
        case NSKeyUp:
            Neko::g_Core->p_Input->OnKeyUp( [[event charactersIgnoringModifiers] UTF8String][0] );
            
            break;
            
        case NSMouseMoved:
            location = [event locationInWindow];
            Neko::g_Core->p_Camera->_lastXt = location.x;
            Neko::g_Core->p_Camera->_lastYt = location.y;
            
            Neko::g_Core->p_Input->OnMouseMove( Neko::g_Core->p_Camera->_lastX, Neko::g_Core->p_Camera->_lastY );
            
            break;
            
            //
            // - Left mouse key events.
            
        case NSLeftMouseDown:
            location = [event locationInWindow];
            Neko::g_Core->p_Input->OnMouseDown( location.x, location.y, Neko::MB_LEFTKEY );
            
            break;
            
        case NSLeftMouseUp:
            location = [event locationInWindow];
            Neko::g_Core->p_Input->OnMouseUp( location.x, location.y, Neko::MB_LEFTKEY );
            
            break;
            
            //
            // - Right mouse key events.
            
        case NSRightMouseDown:
            location = [event locationInWindow];
            Neko::g_Core->p_Input->OnMouseDown( location.x, location.y, Neko::MB_RIGHTKEY );
            
            break;
            
        case NSRightMouseUp:
            location = [event locationInWindow];
            Neko::g_Core->p_Input->OnMouseUp( location.x, location.y, Neko::MB_RIGHTKEY );
            
            break;
            
            //        case NSScrollWheel:
            //        {
            //            if( [event scrollingDeltaY] < 0.0f )
            //            {
            //                g_Core->p_Input->OnMouseScroll( YES );
            //            }
            //            else
            //            {
            //                g_Core->p_Input->OnMouseScroll( NO );
            //            }
            //        }
            //            break;
        default:
            break;
    }
    
    lastRelativePoint = [osx_mwindow->base->mainContext convertPoint:lastPoint fromView:nil];
    Neko::g_Core->p_Camera->_lastX = lastRelativePoint.x;
    Neko::g_Core->p_Camera->_lastY =  [osx_mwindow->base->mainContext frame].size.height - lastRelativePoint.y;
    
    
//    osx_mwindow->base->m_threadLock->Unlock();
}





- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSLog( @"applicationDidFinishLaunching() called" );
    
#ifdef USES_OPENGL
    // Add OpenGL context view to window.
    [osx_mwindow->base->hWindow makeFirstResponder:osx_mwindow->base->mainContext];
    [osx_mwindow->base->hWindow setContentView:osx_mwindow->base->mainContext];
#endif
    
    // Add event handler.
    [osx_mwindow->base->hWindow setNextResponder:osx_mwindow->base->eventDelegatingView];
    [[osx_mwindow->base->hWindow contentView] addSubview:osx_mwindow->base->eventDelegatingView];
    
    
    // Show window.
    [osx_mwindow->base->hWindow makeKeyAndOrderFront:NSApp];
    
    [NSApp stop:nil];
    PostEmptyEvent();
    
    // Run the game loop.
    osx_mwindow->NekoEventLoop();
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    PostEmptyEvent();
    
    delete osx_mwindow;
    osx_mwindow = NEKO_NULL;
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    Neko::g_Core->Frame();
    NSLog( @"applicationWillFinishLaunching" );
    [window makeKeyAndOrderFront:self];
}

- (void)dealloc
{
    
}

@end


#else // iOS.


@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    // Override point for customization after application launch.
    
    
    UIImage *navBackgroundImage = [UIImage imageNamed:@"nav_bg"];
    [[UINavigationBar appearance] setBackgroundImage:navBackgroundImage forBarMetrics:UIBarMetricsDefault];
    
    // color of button
    [[UINavigationBar appearance] setTintColor:[UIColor whiteColor]];
    
    [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
    [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent];
    
    
    // "back" navigation icon
    //[[UINavigationBar appearance] setBackIndicatorImage:[UIImage imageNamed:@"back_btn.png"]];
    //[[UINavigationBar appearance] setBackIndicatorTransitionMaskImage:[UIImage imageNamed:@"back_btn.png"]];
    
    // font style
    
    int red_c = 127;
    int green_c = 127;
    int blue_c = 127;
    
    float alpha_c = 1.0;
    
    [[UIBarItem appearance] setTitleTextAttributes:[NSDictionary dictionaryWithObjectsAndKeys:
                                                    [UIColor colorWithRed:red_c/255.0 green:green_c/255.0 blue:blue_c/255.0 alpha:alpha_c],
                                                    [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0],
                                                    [NSValue valueWithUIOffset:UIOffsetMake(0, 1)],
                                                    [UIFont fontWithName:@"AmericanTypewriter" size:0.0],
                                                    nil]
                                          forState:UIControlStateNormal];
    
    return YES;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application {
    [[UIApplication sharedApplication] setStatusBarStyle:UIStatusBarStyleLightContent animated:NO];
    [UIApplication sharedApplication].statusBarStyle = UIStatusBarStyleLightContent;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end

#endif
