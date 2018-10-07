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
//  MacWindow.h
//  OSX Window manager. :O
//
//  Created by Neko Code on 1/2/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import "MetalView.h"
#import "GraphicsView.h"

#import "../../Shared/SystemShared.h"

namespace Neko {
    
    extern MetalView * g_pMetalBase;

    /**
     *  Window handler.
     */
    struct MacRendererBase
    {
        NSScreen * chosenScreen;
        
        /**
         *  Display frame.
         */
        NSRect mainDisplayRect;
        
        NSOpenGLView * mainContext;
        
        
        /**
         *  Window handler.
         */
        __strong NSWindow *hWindow;
        
        // Renderer lock.
        INekoThreadLock  * m_threadLock;
        
        __strong EventDelegatingView *eventDelegatingView;
        
        BOOL        m_bReady;
        BOOL        contextCanBeAdded;
    };

    /**
     *  OSX window.
     */
    class OSXWindow
    {
    public:
        OSXWindow();

        /**
         *  Initialize and create window.
         */
        void Initialize();
        
        /**
         *  Run the loop!!1
         */
        void NekoEventLoop();
        
        INekoThreadLock  * m_threadLock;
        INekoThread      * m_rendererThread;
        
        MacRendererBase * base;
        
    private:

        void CreateConsole();

#ifdef USES_METAL
        MetalView * _metalView;
#endif
    };
}