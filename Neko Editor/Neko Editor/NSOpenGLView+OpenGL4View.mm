//
//  NSOpenGLView+OpenGL4View.m
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import "NSOpenGLView+OpenGL4View.h"

@interface OpenGL4View (PrivateMethods)
- (void) initGL;

@end

@implementation OpenGL4View


#define NEKO_EDITOR

#include "Core.h"
#include "Renderer.h"
#include "Camera.h"
#include "Input.h"

using namespace Neko;


- (CVReturn) getFrameForTime:(const CVTimeStamp*)outputTime
{
    // There is no autorelease pool when this method is called
    // because it will be called from a background thread.
    // It's important to create one or app can leak objects.
   // NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    [self drawView];
    
    //[pool release];
    return kCVReturnSuccess;
}

// This is the renderer output callback function
static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,
                                      const CVTimeStamp* now,
                                      const CVTimeStamp* outputTime,
                                      CVOptionFlags flagsIn,
                                      CVOptionFlags* flagsOut,
                                      void* displayLinkContext)
{
    CVReturn result = [(__bridge OpenGL4View*)displayLinkContext getFrameForTime:outputTime];
    return result;
}

- (void) awakeFromNib
{

    NSOpenGLPixelFormatAttribute attrs[] =
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 24,
        // Must specify the 3.2 Core Profile to use OpenGL 3.2
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion4_1Core,
        0
    };
    
    NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    
    if (!pf)
    {
        NSLog(@"No OpenGL pixel format");
    }
	   
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:pf shareContext:nil];
    
    // When we're using a CoreProfile context, crash if we call a legacy OpenGL function
    // This will make it much more obvious where and when such a function call is made so
    // that we can remove such calls.
    // Without this we'd simply get GL_INVALID_OPERATION error for calling legacy functions
    // but it would be more difficult to see where that function was called.
    CGLEnable([context CGLContextObj], kCGLCECrashOnRemovedFunctions);
    
    [self setPixelFormat:pf];
    
    [self setOpenGLContext:context];
    
    // Opt-In to Retina resolution
    [self setWantsBestResolutionOpenGLSurface:YES];
}

- (void) prepareOpenGL
{
    [super prepareOpenGL];
    
    // Make all the OpenGL calls to setup rendering
    //  and build the necessary rendering objects
    [self initGL];
    
    // Create a display link capable of being used with all active displays
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    
    // Set the renderer output callback function
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallback, (__bridge void *)(self));
    
    // Set the display link for the current renderer
    CGLContextObj cglContext = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    
    // Activate the display link
    CVDisplayLinkStart(displayLink);
    
    // Register to be notified when the window closes so we can stop the displaylink
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(windowWillClose:)
                                                 name:NSWindowWillCloseNotification
                                               object:[self window]];
}

- (void) windowWillClose:(NSNotification*)notification
{
    // Stop the display link when the window is closing because default
    // OpenGL render buffers will be destroyed.  If display link continues to
    // fire without renderbuffers, OpenGL draw calls will set errors.
    
    CVDisplayLinkStop(displayLink);
}

- (void) initGL
{
    // The reshape function may have changed the thread to which our OpenGL
    // context is attached before prepareOpenGL and initGL are called.  So call
    // makeCurrentContext to ensure that our OpenGL context current to this
    // thread (i.e. makeCurrentContext directs all OpenGL calls on this thread
    // to [self openGLContext])
    [[self openGLContext] makeCurrentContext];
    
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
    
    // Init our renderer.
    srand( (unsigned)time( NULL ) );
    
    [NSApp activateIgnoringOtherApps:YES];
    
    // O Dear Lord, thank you for this function.
    //CGAssociateMouseAndMouseCursorPosition( false );
    
    // Create Mac console window.
    //CreateConsole();
    
    // Stuff which must be loaded first.
    g_Core->Preload( ncMacUtilities::GetBundlePath() );
    g_mainRenderer->Preload();
    
    g_Core->Initialize();

    //
    //  Get context.
    //
    CGLContextObj ctx;
    memset( &ctx, NULL, sizeof(CGLContextObj) );
    ctx = CGLGetCurrentContext();
    if( !ctx ) {
        g_Core->p_Console->Error( ERR_OPENGL, "Couldn't get CoreGL context.\n" );
        return;
    }
    
    // Enable Apple's multi-threaded GL engine -- it's generally useful for
    // high vertex throughput. Not high fragment situations
    if( Render_UseAppleMTE.Get<bool>() ) {
        g_Core->p_Console->Print( LOG_INFO, "Enabling Apple Multithreaded engine..\n" );
        
        if( CGLEnable( ctx, kCGLCEMPEngine ) ) {
            g_Core->p_Console->Error( ERR_FATAL, "Couldn't enable Apple Multithreaded engine." );
            return;
        }
    }
    
    // Change backbuffer resolution -- good for performance.
    const GLint backingSize[2] = { Render_Width.Get<int>(), Render_Height.Get<int>() };
    
    CGLSetParameter((CGLContextObj)ctx, kCGLCPSurfaceBackingSize, backingSize);
    CGLEnable((CGLContextObj)ctx, kCGLCESurfaceBackingSize);
    GLint opacity=0;
    CGLSetParameter(ctx,kCGLCPSurfaceOpacity,&opacity);
    
    GLint vblSynch = 1;
    [[self openGLContext] setValues:&vblSynch forParameter:NSOpenGLCPSwapInterval];
    
    
    gl_Core->Initialize();      // Initialize OpenGL and Engine Renderer.
    

    
    // Some things.
    gl_Core->Initialized = true;
    gl_Core->OnResize( Render_Width.Get<int>(), Render_Height.Get<int>() );
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)canBecomeKeyView {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    g_Core->p_Input->OnKeyPress( [[theEvent characters] UTF8String][0] );
}

- (void)keyUp:(NSEvent *)theEvent {
    g_Core->p_Input->OnKeyUp( [[theEvent characters] UTF8String][0] );
}

- (void) reshape
{
    [super reshape];
    
    // We draw on a secondary thread through the display link. However, when
    // resizing the view, -drawRect is called on the main thread.
    // Add a mutex around to avoid the threads accessing the context
    // simultaneously when resizing.
    CGLLockContext([[self openGLContext] CGLContextObj]);
    
    // Get the view size in Points
    NSRect viewRectPoints = [self bounds];
    
    
    // Rendering at retina resolutions will reduce aliasing, but at the potential
    // cost of framerate and battery life due to the GPU needing to render more
    // pixels.
    
    // Any calculations the renderer does which use pixel dimentions, must be
    // in "retina" space.  [NSView convertRectToBacking] converts point sizes
    // to pixel sizes.  Thus the renderer gets the size in pixels, not points,
    // so that it can set it's viewport and perform and other pixel based
    // calculations appropriately.
    // viewRectPixels will be larger (2x) than viewRectPoints for retina displays.
    // viewRectPixels will be the same as viewRectPoints for non-retina displays
    NSRect viewRectPixels = [self convertRectToBacking:viewRectPoints];

    // Set the new dimensions in our renderer

    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}


- (void)renewGState
{
    // Called whenever graphics state updated (such as window resize)
    
    // OpenGL rendering is not synchronous with other rendering on the OSX.
    // Therefore, call disableScreenUpdatesUntilFlush so the window server
    // doesn't render non-OpenGL content in the window asynchronously from
    // OpenGL content, which could cause flickering.  (non-OpenGL content
    // includes the title bar and drawing done by the app with other APIs)
    [[self window] disableScreenUpdatesUntilFlush];
    
    [super renewGState];
}

- (void) drawRect: (NSRect) theRect
{
    // Called during resize operations
    
    // Avoid flickering during resize by drawiing
    [self drawView];
}

- (void) drawView
{
    [[self openGLContext] makeCurrentContext];
    
    // We draw on a secondary thread through the display link
    // When resizing the view, -reshape is called automatically on the main
    // thread. Add a mutex around to avoid the threads accessing the context
    // simultaneously when resizing
    CGLLockContext([[self openGLContext] CGLContextObj]);
    
    g_Core->Frame();


    CGLFlushDrawable([[self openGLContext] CGLContextObj]);
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}

- (void) dealloc
{
    // Stop the display link BEFORE releasing anything in the view
    // otherwise the display link thread may call into the view and crash
    // when it encounters something that has been release
    CVDisplayLinkStop(displayLink);
    
    CVDisplayLinkRelease(displayLink);
    
}
@end