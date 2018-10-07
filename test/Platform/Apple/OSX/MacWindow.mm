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
//  MacWindow.m
//  OSX Window manager. :O
//
//  Created by Neko Code on 1/2/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#import "../../../Core/Core.h"        // Includes "SharedApple.h"
#import "../../../Graphics/OpenGL/OpenGLBase.h"
#import "../../../Graphics/Renderer/Renderer.h"
#import "../../../Core/Console/Console.h"
#import "../../../Core/Network/MultiplayerServer.h"
//#import "MacOpenGLView.h"
#import "../../../Core/Player/Input/Input.h"
#import "../../../Core/Player/Camera/Camera.h"
#import "../../../Graphics/GraphicsManager.h"
#import "../../Shared/System.h"

#   if !defined( iOS_BUILD  ) // U woot mate

#import "OSXJoystick.hpp"
#import <Foundation/Foundation.h>

// Note from Apple developer: it's better to use NSTimer
//#import <QuartzCore/CVDisplayLink.h>

// Creates Mac OSX native window ( finally ) and attaches OpenGL or Metal context.
// TODO: better look
// TODO: Use AllocMemory instead of 'new'
// TODO: Metal API workaround

#   if defined( USES_METAL )
    #import "MetalView.h"           // Metal NSView
    #import "MetalExternal.h"       // Neko API
    #import "MetalReferences.h"     // Structs for Neko API
#   endif

#import "MacUtilities.h"    // Apple utilities.
#import "MacWindow.h"
#import "GraphicsView.h"
#import "OSX_External.h"    // Neko API

namespace Neko {
    
    //!  Metal handler. MOVE ME
    MetalView   * g_pMetalBase = 0;
    
    bool rendererDowntime = false;
    
    const static uint32_t Render_PixelColorSize = 32;
    const static uint32_t Render_DepthColorSize = 24;
    const static uint32_t Render_AlphaColorSize = 8;
    
    /**
     *  Constructor.
     */
    OSXWindow::OSXWindow()
    {
     
    }
    
    /**
     *  Create console window to display our logs.
     */
    void OSXWindow::CreateConsole()
    {
        g_Core->p_Console->Print( LOG_INFO, "Implement CreateConsole!!\n" );
    }
    
    /**
     *  Pick a display to create window in.
     *
     *  @TODO   Time out.
     */
    static NSScreen * PickDisplay()
    {
        NSScreen * chosenScreen;
        CGPoint mousePoint;
        
        // Pick display.
        g_Core->p_Console->Print( LOG_INFO, "Picking a display to create window in..\n" );
        
        mousePoint = [NSEvent mouseLocation];
        chosenScreen = nil;
        
        // -- Lookup for available displays.
        for( NSScreen * screen in [NSScreen screens] ) {
            // Found a display.
            if( NSPointInRect( mousePoint, [screen frame] ) == YES ) {
                chosenScreen = screen;
                break;
            }
        }
        
        return chosenScreen;
    }
    
    /**
     *  Video memory size.
     */
    static const uint64_t GetVideoMemorySize()
    {
#if defined( USES_OPENGL )
        // Don't forget to set CGLContext!!
        uint64_t VideoMemory = 0;
        
        NSOpenGLContext * NSContext = [NSOpenGLContext currentContext];
        CGLContextObj Context = NSContext ? (CGLContextObj)[NSContext CGLContextObj] : NEKO_NULL;
        
        if( Context ) {
            // Get screen.
            GLint VirtualScreen = [NSContext currentVirtualScreen];
            GLint RendererID = 0;
            GLint DisplayMask = 0;
            // Get current pixel format.
            CGLPixelFormatObj PixelFormat = CGLGetPixelFormat(Context);
            
            if( PixelFormat && CGLDescribePixelFormat(PixelFormat, VirtualScreen, kCGLPFADisplayMask, &DisplayMask ) == kCGLNoError && CGLGetParameter(Context, kCGLCPCurrentRendererID, &RendererID) == kCGLNoError) {
                GLint Num = 0;
                CGLRendererInfoObj Renderer;
                
                CGLQueryRendererInfo((GLuint)DisplayMask, &Renderer, &Num);
                
                // Loop thro renderers and get info.
                for( GLint i = 0; i < Num; ++i ) {
                    GLint ThisRendererID = 0;
                    CGLDescribeRenderer(Renderer, i, kCGLRPRendererID, &ThisRendererID);
                    
                    // Check renderers.
                    if( ThisRendererID == RendererID ) {
                        GLint MemoryMB = 0;
                        CGLDescribeRenderer( Renderer, i, kCGLRPVideoMemoryMegabytes, (GLint*)&MemoryMB );
                        VideoMemory = (uint64)MemoryMB * 1024llu * 1024llu;
                        break;
                    }
                }
                
                CGLDestroyRendererInfo( Renderer );
            }
        }
        return VideoMemory;
#else
        // Metal implementation ( TODO )!
        return 0;
#endif
    }
    
    /**
     *  Create graphical base.
     */
    static void CreateGLBase( INekoThread * thread, void * arg1, void * arg2 )
    {
        //  Get context.
        CGLContextObj   ctx;
        
        int32_t     opacity;
        int32_t     vblSynch;
        
        MacRendererBase * base = (MacRendererBase*)arg1;
        
        // Now create graphics context on the renderer thread OpenGL, Metal.
        
#   if defined( USES_METAL ) // Metal implementation.
        
        // Create Metal view and context.
        // temp temp temp temp workaround
        NSRect rect = CGRectMake( 0.0, 0.0, 1366.0, 768.0 );
        _metalView = [[MetalView alloc] initWithFrame:rect];
        
        [base.hWindow setContentView:_metalView];
        
#   else // OpenGL
        
        //  Pixel format attributes.
        const NSOpenGLPixelFormatAttribute attributes[] =
        {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAColorSize, static_cast<NSOpenGLPixelFormatAttribute>(Render_PixelColorSize),
            NSOpenGLPFADepthSize, static_cast<NSOpenGLPixelFormatAttribute>(Render_DepthColorSize),
            NSOpenGLPFAAlphaSize, static_cast<NSOpenGLPixelFormatAttribute>(Render_AlphaColorSize),
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFAAccelerated,
            NSOpenGLPFANoRecovery,
            0,
        };
        
        // Yup, create OpenGL pixel format.
        g_Core->p_Console->Print( LOG_INFO, "Creating OpenGL pixel format..\n" );
        
        NSOpenGLPixelFormat * pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
        
        if( pixelFormat == nil ) {
            g_Core->p_Console->Error( ERR_OPENGL, "No valid matching OpenGL Pixel Format found!" );
            
            return;
        }
        
        g_Core->p_Console->Print( LOG_INFO, "Creating OpenGL context..\n" );
        
        // Create context.
        base->mainContext = [[NSOpenGLView alloc] initWithFrame:base->mainDisplayRect pixelFormat:pixelFormat];
//        [mainContext setWantsBestResolutionOpenGLSurface:YES];
        
        // Make OpenGL context to be current.
        [[base->mainContext openGLContext] makeCurrentContext];
        
        ctx = [[base->mainContext openGLContext] CGLContextObj];// CGLGetCurrentContext();
        
        // Enable Apple's multi-threaded GL engine -- it's generally useful for
        // high vertex throughput. Not high fragment situations
        if( Render_UseAppleMTE->Get<bool>() == true ) {
            g_Core->p_Console->Print( LOG_INFO, "Enabling Apple Multithreaded engine..\n" );
            
            if( CGLEnable( ctx, kCGLCEMPEngine ) ) {
                g_Core->p_Console->Error( ERR_FATAL, "Couldn't enable Apple Multithreaded engine." );
                return;
            }
        }
        
        // Change backbuffer resolution -- good for performance.
        const int32_t backingSize[2] = { Render_Width->Get<int>(), Render_Height->Get<int>() };
        
        CGLSetParameter( (CGLContextObj)ctx, kCGLCPSurfaceBackingSize, backingSize );
        CGLEnable( (CGLContextObj)ctx, kCGLCESurfaceBackingSize );
        
        // Set parameters.
        
        opacity = 0;
        CGLSetParameter( ctx,kCGLCPSurfaceOpacity, &opacity );
        
        vblSynch = 1;
        [[base->mainContext openGLContext] setValues:&vblSynch forParameter:NSOpenGLCPSwapInterval];
        
        const uint64_t video_mem = GetVideoMemorySize() / (1024 * 1024);
        g_Core->p_Console->Print( LOG_INFO, "OpenGL reported video memory: %d MB\n", (int)video_mem );
        
        // - -----
        //     Here we initialize Neko renderer.
        g_pGraphicsManager = new GraphicsManager();
        // Initialize OpenGL and Neko Renderer.
        g_pGraphicsManager->Initialize( GraphicsManager::INTERFACE_OPENGL );
        
        // Update after initialization.
        [[base->mainContext openGLContext] update];
        [[base->mainContext openGLContext] flushBuffer];
        
#   endif
        
        // Attach graphics context to the main window.
        dispatch_sync(dispatch_get_main_queue(), ^() {
            [NSApp finishLaunching];
            [NSApp setWindowsNeedUpdate:YES];
        }); // Main thread queue.
        
        // Some things.
        g_pGraphicsManager->GetCurrentInterface()->OnLoad();
        
        //  Renderer loop.
        
        uint32_t LastTime = 0;
        uint32_t FrameTime = 0;
        /*uint32_t*/ g_mainRenderer->Time = 0;
        
        // Renderer must run smoothly at 30 or 60 frames per second constantly.
        static const int32_t RendererFramePerSecond = 60;
        static const float scale = 1.0f;
        
        uint32_t msec, minMsec = 1000 / RendererFramePerSecond;
        
        NSRunLoop * currentRunLoop = [NSRunLoop currentRunLoop]; // Using NSTimer.
        NSDate  * destDate = [[NSDate alloc] init];
        
        // 10.11 FIX!
        // Added this so window isn't bugged now ( appears instantly ).
        [currentRunLoop addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
        // - ---
        base->m_bReady = YES;
        
        // Launch renderer loop.
        do {
            @autoreleasepool {
                base->m_threadLock->Lock();
            
                // Run the runloop to process OS events
                [currentRunLoop runUntilDate:destDate];
                // Your logic/draw code goes here.
                
                // Frame rate count.
                do {
                    // Renderer queue is already in Neko renderer loop.
                    FrameTime = g_Core->p_System->Milliseconds();
                    
                    if ( LastTime > FrameTime ) {
                        LastTime = FrameTime;
                    }
                    
                    msec = FrameTime - LastTime;
                    
                } while ( msec < minMsec );
                
                LastTime = FrameTime;
                msec = (msec * scale);
                
                if( msec < 1 ) {
                    msec = 1;
                } else if( msec > 5000 ) {
                    msec = 5000;
                }
                
                g_mainRenderer->Time += msec;
                
                // Render now.
                
#   if defined( USES_METAL )
                // TODO: use GraphicsInstance.
                [_metalView draw];
#   else
                g_mainRenderer->Render( msec );
#   endif
          
                if( base->mainContext != nil ) {
#   if defined( USES_OPENGL )
                    // OpenGL context is a double buffered.
                    [[base->mainContext openGLContext] flushBuffer];
#   endif
                }
                
                
                // Calculate the new date
//                NSDate *newDate = [[NSDate alloc] initWithTimeInterval:1.0f/60 sinceDate:destDate];
//                destDate = newDate;
                
                base->m_threadLock->Unlock();
            }
            
        } while( !rendererDowntime );
        
        // Kill thread lock.
        CCore::KillLock( base->m_threadLock );
        
        // Renderer shutdown.
        g_mainRenderer->Shutdown();
    }
 
    /**
     *  Get system events.
     */
    void GetEvents()
    {
        NSEvent * event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                             untilDate:[NSDate distantPast]
                                                inMode:NSDefaultRunLoopMode
                                               dequeue:YES];
        if( event != nil ) {
            [NSApp sendEvent:event];
        }
        
        g_Core->AfterPostEvent();
    }
    
    /**
     *  Run the loop!!1
     */
    void OSXWindow::NekoEventLoop()
    {
        INekoThreadLock * eventLock;
        
        CGEventSourceRef ref = CGEventSourceCreate( kCGEventSourceStateHIDSystemState );
        CGEventSourceSetLocalEventsSuppressionInterval( ref, 0.0 );
        
        g_Core->p_Console->Print( LOG_INFO, "Starting Neko event loop..\n" );
        
        eventLock = CCore::CreateLock();
        
        // Main event loop.
        while( true ) {
            @autoreleasepool {
                eventLock->Lock();
            
                // Let the engine process system events at start.
                if( base->m_bReady == YES ) {
                    g_Core->Frame();
                }
                
                // System events.
                GetEvents();
                
                eventLock->Unlock();
            }
        }
        
        // Kill the renderer.
        rendererDowntime = true;
        
        // Show mouse cursor.
//        CGDisplayShowCursor( kCGDirectMainDisplay );
        
        m_rendererThread->CallbackEnd( 200.0f );
        
        CCore::KillThread( m_rendererThread );
    
        @autoreleasepool {
            NSEvent* event = [NSEvent otherEventWithType:NSApplicationDefined
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
        
        // Quit now.
        g_Core->Quit( "Game quit.", true );
        delete g_Core;  // TODO: FreeMemory
        
    }
    
    static void InitDevices()
    {
        // Create system joystick.
        CXJoystick * joystick = new CXJoystick();
        g_Core->m_pSysJoystick = joystick;
    }
    
    /**
     *  Initialize game engine.
     */
    void OSXWindow::Initialize()
    {
        // Stuff which must be loaded first.
        const char * path = ncMacUtilities::GetBundlePath();
        
        // Initialise the game core.
        g_Core = new CCore(); // TODO: AllocMemory
 
        InitDevices();  // initialize engine devices
        
        // Preload Neko engine.
        g_Core->Preload( path );
      
        // Create a renderer base.
        base = new MacRendererBase();
        base->m_threadLock = CCore::CreateLock();
        base->m_bReady = NO;
        
        // Pick display.
        base->chosenScreen = PickDisplay();
        if( base->chosenScreen == nil ) {
            g_Core->p_Console->Error( ERR_FATAL, "Couldn't pick display to create window." );
            return;
        }
        
        // -- Create window with it's context and initialize Neko renderer.
        g_Core->p_Console->Print( "Creating OS X window..\n" );
        
        NSString * windowTitle;
        NSUInteger windowStyle;
        
        // -- Window title & properties.
        windowTitle = [NSString stringWithCString:"Neko Engine" encoding:NSUTF8StringEncoding];
        windowStyle =  NSTitledWindowMask  | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
        
        // -- Choose main display and its size.
        NSRect rect;
        rect.size.width = 640;
        rect.size.height = 480;
        base->mainDisplayRect = rect;// [base->chosenScreen frame];
        
        // -- Create new window.
        base->hWindow = [[NSWindow alloc] initWithContentRect:base->mainDisplayRect
                                                    styleMask: Render_Fullscreen->Get<bool>() ? NSBorderlessWindowMask : windowStyle
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
        
        // -- Check if window was created.
        if( base->hWindow == NEKO_NULL ) {
            // Oops.
            g_Core->p_Console->Error( ERR_FATAL, "Could not create window." );
            return;
        }
        
        // -- Get real window origin.
        if( Render_Fullscreen->Get<bool>() == false ) {
            NSRect frameRect = base->mainDisplayRect;//[base.chosenScreen visibleFrame];
            //            unsigned int style = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
            NSRect contentRect = [NSWindow contentRectForFrameRect:frameRect styleMask:windowStyle];
            
            Window_Width->Set<int>( (int32_t)contentRect.size.width );
            Window_Height->Set<int>( (int32_t)contentRect.size.height );
            
            g_mainRenderer->m_iWindowX = (int32_t)contentRect.origin.x;
            g_mainRenderer->m_iWindowY = (int32_t)contentRect.origin.y;
        } else {
            Window_Width->Set<int>( (int32_t)base->mainDisplayRect.size.width );
            Window_Height->Set<int>( (int32_t)base->mainDisplayRect.size.height );
            
            g_mainRenderer->m_iWindowX = (int32_t)base->mainDisplayRect.origin.x;
            g_mainRenderer->m_iWindowY = (int32_t)base->mainDisplayRect.origin.y ;
        }
        
        g_mainRenderer->displayWidth = base->mainDisplayRect.size.width;
        g_mainRenderer->displayHeight = base->mainDisplayRect.size.height;
        
        g_Core->p_Console->Print( LOG_INFO, "Renderer viewport resolution: %ix%i\n", Render_Width->Get<int>(), Render_Height->Get<int>() );
        g_Core->p_Console->Print( LOG_INFO, "Window resolution: %ix%i at %i, %i\n", Window_Width->Get<int>(), Window_Height->Get<int>(),
                          g_mainRenderer->m_iWindowX, g_mainRenderer->m_iWindowY );
        
        //  Window settings.
        [base->hWindow setTitle:/*@"Neko engine"*/windowTitle];
        base->hWindow.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
    
        if( Render_Fullscreen->Get<bool>() == true ) {
            [base->hWindow setLevel:NSMainMenuWindowLevel + 1];
            [NSMenu setMenuBarVisible:NO];
        }
        
        [base->hWindow setAcceptsMouseMovedEvents:YES];
        [base->hWindow invalidateShadow];
        [base->hWindow setOpaque:YES];
        [base->hWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
        [base->hWindow setRestorable:NO];
        [base->hWindow canBecomeKeyWindow];
        [base->hWindow acceptsMouseMovedEvents];
        [base->hWindow acceptsFirstResponder];
        
        // Initialize the window controller.
        NSWindowController * controller = [[NSWindowController alloc] initWithWindow:base->hWindow];
        [controller showWindow:nil];
   
        //    Create renderer thread.
        
        g_Core->p_Console->Print( LOG_INFO, "Creating renderer thread..\n" );
        
        // Renderer thread...
        m_rendererThread = CCore::CreateThread( INekoThread::PRIORITY_HIGH, 1, CreateGLBase, base, NEKO_NULL );
        
        // Hide the cursor.
//        CGDisplayHideCursor( kCGDirectMainDisplay );
//        CGDisplayMoveCursorToPoint( kCGDirectMainDisplay, CGPointZero );
 
    }
}
#endif
