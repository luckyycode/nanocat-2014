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
//  mainm.cpp
//  Neko engine entry point for Apple systems.
//
//  Created by Neko Code on 8/26/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#import "SharedApple.h"

#ifndef iOS_BUILD

#import <Cocoa/Cocoa.h>

#import "MacWindow.h"
#import "AppDelegate.h"

#import "MacUtilities.h"

#import "../../../Core/Core.h"
#import "../../../Graphics/Renderer/Renderer.h"
#import "../../Shared/System.h"
#import "../../../Core/Player/Camera/Camera.h"
#import "../../../Graphics/GraphicsManager.h"
#import "../../../Core/Player/Input/Input.h"

#ifdef USES_GLFW
/**
 *  Mouse click action.
 */
static void mouse_click( GLFWwindow * window, int32_t button, int32_t action, int32_t mods )
{
    double x, y;
    glfwGetCursorPos( window, &x, &y );
    
    // Left button..
    if( button == GLFW_MOUSE_BUTTON_LEFT )
    {
        if( action == GLFW_PRESS )
        {
            Neko::g_Core->p_Input->OnMouseDown( (int32_t)x, (int32_t)y, Neko::MB_LEFTKEY );
        }
        else
        {
            Neko::g_Core->p_Input->OnMouseUp( (int32_t)x, (int32_t)y, Neko::MB_LEFTKEY );
        }
    }
    else if( button == GLFW_MOUSE_BUTTON_RIGHT )
    {
        if( action == GLFW_PRESS )
        {
            Neko::g_Core->p_Input->OnMouseDown( (int32_t)x, (int32_t)y, Neko::MB_RIGHTKEY );
        }
        else
        {
            Neko::g_Core->p_Input->OnMouseUp( (int32_t)x, (int32_t)y, Neko::MB_RIGHTKEY );
        }
    }
}

/**
 *  Mouse move callback.
 */
static void mouse_move( GLFWwindow * window, double x, double y )
{
    Neko::g_Core->p_Camera->_lastX = (int32_t)x;
    Neko::g_Core->p_Camera->_lastY = (int32_t)y;
    
    Neko::g_Core->p_Input->OnMouseMove( Neko::g_Core->p_Camera->_lastX, Neko::g_Core->p_Camera->_lastY );
}

/**
 *  Key callback.
 */
static void key_callback( GLFWwindow * window, int32_t key, int32_t scancode, int32_t action, int32_t mods )
{
    if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
    
    printf( "%c %c\n", (char)key, (char)scancode);
    
    if( action == GLFW_PRESS )
    {
        Neko::g_Core->p_Input->OnKeyPress( (char)key );
    }
    else
    {
        Neko::g_Core->p_Input->OnKeyUp( (char)key );
    }
}

/**
 *  Create GLFW window with OpenGL context.
 */
static void CreateGLBase( Neko::INekoThread * thread, void * arg1, void * args )
{

    GLFWwindow * window = (GLFWwindow*)arg1;
    
    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 ) ;

    /**     Here we initialize Neko renderer.       **/
    Neko::g_pGraphicsManager->Initialize( Neko::GraphicsManager::INTERFACE_OPENGL );      // Initialize OpenGL and Engine Renderer.
    
    
    // Run the renderer loop.
    uint32_t LastTime = 0;
    uint32_t FrameTime = 0;
    /*uint32_t*/ Neko::g_mainRenderer->Time = 0;
    
    static const int32_t RendererFramePerSecond = 60;
    static const float scale = 1.0f;
    
    uint32_t msec, minMsec = 1000 / RendererFramePerSecond;
    
    // Some things.
    Neko::g_pGraphicsManager->GetCurrentInterface()->OnLoad();

    while( !glfwWindowShouldClose( window ) )
    {
        // Your logic/draw code goes here.
        
        // Frame rate count.
        do
        {
            FrameTime = Neko::g_Core->p_System->Milliseconds();
            
            if ( LastTime > FrameTime )
            {
                LastTime = FrameTime;
            }
            
            msec = FrameTime - LastTime;
            
        } while ( msec < minMsec );
        
        LastTime = FrameTime;
        msec = (msec * scale);
        
        if( msec < 1 )
        {
            msec = 1;
        }
        else if( msec > 5000 )
        {
            msec = 5000;
        }
        
        Neko::g_mainRenderer->Time += msec;
        
        // Render now.
        Neko::g_mainRenderer->Render( msec );

        // Update player movement.
        Neko::g_Core->p_Camera->Frame( msec );      // Should you move me to core thread? Not now, you make strange stuff!
 
        glfwSwapBuffers(window);
    }

    // Renderer shutdown.
    Neko::g_mainRenderer->Shutdown();
}
#endif

int main( int argc, const char * argv[] )
{
    
#ifndef USES_GLFW
    printf( "Initializing NekoAppDelegate..\n" );
    
    // Since we don't have Info.plist we need this to make window active.
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    
    // Focus to our window.
    [NSApp activateIgnoringOtherApps:YES];
    
    @autoreleasepool
    {
        [NekoApplication sharedApplication];
        NekoApplication * application = [NekoApplication sharedApplication];
        
        NekoApplicationDelegate * appDelegate = [[NekoApplicationDelegate alloc] init];
        
        [application setDelegate:appDelegate];
        [application run];
    }
#else
    
    Neko::INekoThreadLock * m_threadLock;
    Neko::INekoThread    * m_rendererThread;
    
    // Stuff which must be loaded first.
    const char * path = ncMacUtilities::GetBundlePath();
    
    // Preload Neko engine.
    Neko::g_Core->Preload( path );
    
    // Preload Neko renderer properties.
    Neko::g_mainRenderer->Preload();
    // Itself, Neko renderer initializes in another thread!
    
    // Initialize Neko engine.
    Neko::g_Core->Initialize();
    
    
    GLFWwindow * window;

    if( glfwInit() == false )
    {
        // Error..
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);  // yes, 3 and 2!!!
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow( 640, 480, "Neko Engine", NEKO_NULL, NEKO_NULL );
    
    if( window == NEKO_NULL )
    {
        glfwTerminate();
    }
    
    int left, top, right, bottom;
    glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
    
    NSRect mainDisplayRect = CGRectMake( left, top, 640, 480 );

    Neko::Window_Width.Set<int>( (int32_t)mainDisplayRect.size.width );
    Neko::Window_Height.Set<int>( (int32_t)mainDisplayRect.size.height );
    
    Neko::Window_OriginX.Set<int>( (int32_t)mainDisplayRect.origin.x );
    Neko::Window_OriginY.Set<int>( (int32_t)mainDisplayRect.origin.y );
 
    Neko::g_mainRenderer->displayWidth = mainDisplayRect.size.width;
    Neko::g_mainRenderer->displayHeight = mainDisplayRect.size.height;

    
    glfwSetKeyCallback( window, key_callback );
    glfwSetCursorPosCallback( window, mouse_move );
    glfwSetMouseButtonCallback( window, mouse_click );
    
    Neko::g_Core->p_Console->Print( Neko::LOG_INFO, "Creating renderer thread..\n" );
    
    // Create renderer thread lock.
    m_threadLock = Neko::CCore::CreateLock();
    
    // Renderer thread...
    m_rendererThread = Neko::CCore::CreateThread( Neko::INekoThread::PRIORITY_HIGH, 1, CreateGLBase, window, NEKO_NULL );
    
    while( true )
    {
        @autoreleasepool
        {
            Neko::g_Core->Frame();
        }
        
        
        glfwPollEvents();
    }
    
#endif
    
    return EXIT_SUCCESS;
}

#else

// wut

#endif
