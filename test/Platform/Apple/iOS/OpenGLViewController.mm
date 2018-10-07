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
//  OpenGLViewController.m
//  MyCode
//
//  Created by Neko Code on 5/2/14.
//  Copyright (c) 2014 neko. All rights reserved.
//


#include "Core.h"
#include "Renderer.h"
#include "OpenGLBase.h"
#include "Camera.h"
#include "Input.h"
#include "Console.h"
#include "ConsoleCommand.h"
#include "System.h"

#include "Utils.h"

#include "BeautifulEnvironment.h"


#import "OpenGLViewController.h"

#import <OpenGLES/ES3/gl.h>
#import	<OpenGLES/EAGL.h>
#import	<OpenGLES/EAGLDrawable.h>

#import "iOSUtilities.h"


@interface OpenGLViewController()

// Some connections.
@property (weak, nonatomic) IBOutlet JSAnalogueStick *AnalogueStick;

@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityIndicator;
@property (weak, nonatomic) IBOutlet UILabel *loadingLabel;
@property (weak, nonatomic) IBOutlet UIButton *magicButton;
@property (weak, nonatomic) IBOutlet UIToolbar *devToolbar;

// OpenGL context.
@property (strong, nonatomic) EAGLContext *context;
@end

@implementation OpenGLViewController

static char homePath[1024];

#pragma mark - View stuff.
- (id)initWithCoder:(NSCoder*)coder {

    self = [super initWithCoder:coder];
    
    if (!self) {
        return nil;
    }


    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    const char * dir = iOS_GetDocumentsFolder();
    
    g_Core->Preload( dir );
    g_mainRenderer->Preload();
    g_Core->SetUsesGraphics( true );// g_Core->UseGraphics = true; // We don't have any other choice on iOS, right?!?!
    
    [self.AnalogueStick setDelegate:self];
    
    UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    [self.view addGestureRecognizer:pinchGesture];
    
    CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.view.layer;
    
    self.view.opaque = YES;
    
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                    nil];
    
    // Preload app logic.

    // Initialize iOS Open GL ES 3.0 context.
    if( !self.context )
        self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    
    if( !self.context ) {
        UIAlertView *e = [[UIAlertView alloc] initWithTitle:@"OpenGL error" message:@"Could not initialize Open GLES context. Application will exit." delegate:nil cancelButtonTitle:@"Okay" otherButtonTitles:nil];
        [e show];
        
        g_Core->Quit( "OpenGL error" );
    }
    
    // Hide status bar.
    [[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone];
    
    // Setup some properties.
    GLKView *view = (GLKView *)self.view;
    
    view.context = self.context;
    
    view.drawableDepthFormat    = GLKViewDrawableDepthFormat24;
    view.drawableColorFormat    = GLKViewDrawableColorFormatRGBA8888;
    view.drawableMultisample    = GLKViewDrawableMultisampleNone;
    view.drawableStencilFormat  = GLKViewDrawableStencilFormatNone;
    
    self.view = view;
    
    [EAGLContext setCurrentContext:self.context];

    glClearColor( 0.4, 0.9, 0.9, 0.0 );

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    self.preferredFramesPerSecond = 60; // Sure... it should be very smooth, yea!?
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];

    // Load application logic now.
    g_Core->Initialize();
    gl_Core->OnResize( Render_Width.Get<int>(), Render_Height.Get<int>() );
    gl_Core->Initialize();
    
    gl_Core->Initialized = true;

    g_Core->Loaded();
}

- (void)dealloc {

}

#pragma mark - Drawing and frame methods.
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    [self Render:g_Core->GetTime()];
}

- (void)update {
    g_Core->Frame();
    
}
//  [(GLKView*)self.view bindDrawable];
- (void)RenderToShader:(CRenderer::ESceneEye)eye  {
    
    //g_pbEnvShadows->RenderShadow();
    
//    [(GLKView*)self.view bindDrawable];
    
}

/*
    Lets see the world!
*/
- (void)Render:(int)msec {
    
}

#pragma mark - Touch methods.
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    // Update last touch position.
    UITouch * touch = [touches anyObject];
    CGPoint location = [touch locationInView:self.view];
    CGPoint lastLoc = [touch previousLocationInView:self.view];
    CGPoint diff = CGPointMake(lastLoc.x - location.x, lastLoc.y - location.y);

    LastPosition._x = (int)diff.x;
    LastPosition._y = (int)diff.y;
    
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {

}

float fCurrentScale = 1.0;
float fLastScale = 1.0;

- (void)handlePinch:(UIPinchGestureRecognizer*)sender {
    
    fCurrentScale += [sender scale] - mLastScale;
    fLastScale = [sender scale];
    
    if( sender.state == UIGestureRecognizerStateEnded ) {
        mLastScale = 1.0;
    }
}

#pragma mark - Buttons
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if( alertView.tag == 2 ) {
        UITextField *data = [alertView textFieldAtIndex:0];
        
        g_Core->p_Console->Execute( [data.text UTF8String] );
    }
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    if( actionSheet.tag == 1 ) {
       
        if( buttonIndex == 0 ) { // Create local game.
            gl_Core->OnResize( 640, 480 );
            g_Core->p_Console->Execute( "local" );
        }
        else if( buttonIndex == 1 ) { // Execute a command.
            UIAlertView *e = [[UIAlertView alloc] initWithTitle:@"Console" message:nil delegate:self cancelButtonTitle:@"Execute" otherButtonTitles:nil];
            e.alertViewStyle = UIAlertViewStylePlainTextInput;
            e.tag = 2;
            [e show];
        }
        else if( buttonIndex == 2 ) { // Capture a screenshot.
            //_system.Quit( "User quit." );
            //[self captureToPhotoAlbum];
             g_Core->p_Console->Execute("sm cat.sm1");
        }
    }
}

// temp
- (IBAction)woot {
    UIActionSheet *pl = [[UIActionSheet alloc] initWithTitle:@"Please choose" delegate:self cancelButtonTitle:@"Snapshot" destructiveButtonTitle:@"Create local game" otherButtonTitles:@"Execute a command", nil];
    
    [pl setTag:1];
    [pl showInView:self.view];
}

#pragma mark - Analogue stick

- (void)analogueStickDidChangeValue:(JSAnalogueStick *)analogueStick ended:(BOOL)end {
    
    float LnormalizedX = self.AnalogueStick.xValue;
    float LnormalizedY = self.AnalogueStick.yValue;
    
//    NSLog( @"yep" );
    
    int32_t rX = end == YES ? 128 : 255 * ( 1 + LnormalizedX ) / 2;        // x axis
    int32_t rY = end == YES ? 128 : 255 * ( 1 + LnormalizedY ) / 2;        // y axis

    // FB
    if( rY >= 127 ) {
        g_Core->p_Input->MakeKeyEvent( NCKEY_FORWARD );
    } else if( rY <= 127 ) {
        g_Core->p_Input->MakeKeyEvent( NCKEY_BACKWARD );
    }
    
    // LR
    if( rY >= 127 ) {
        g_Core->p_Input->MakeKeyEvent( NCKEY_FORWARD );
    } else if( rY <= 127 ) {
        g_Core->p_Input->MakeKeyEvent( NCKEY_BACKWARD );
    }
    
}

#pragma mark - Get Documents folder.

const char *iOS_GetDocumentsFolder( void ) {
    
    char *p;
    
    if( *homePath )
        return homePath;
    
    if( (p = getenv("HOME")) != NEKO_NULL ) {
        strncpy( homePath, p, sizeof(homePath) );
        strcat( homePath, "/Documents" );
        
        if( mkdir( homePath, 0777 ) ) {
            if( errno != EEXIST ) {
                g_Core->p_Console->Error( ERR_FILESYSTEM, "No 'Documents' folder found." );
            }
        }
        return homePath;
    }
    
    return ""; // Is it good?
}

#pragma mark - Capture a screenshot.
-(UIImage *) glToUIImage {
    
    // Fix me: real screen resolution for all devices.
    const int32_t width = 1136;//_renderer.renderWidth;
    const int32_t height = 640;//_renderer.renderHeight;
    
    NSInteger myDataLength = width * height * 4;
    
    // Allocate memory for screen data.
    GLubyte *buffer = new GLubyte[myDataLength];
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    
    // OpenGL renders "upside down".
    GLubyte *buffer2 = new GLubyte[myDataLength];
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width * 4; x++) {
            buffer2[((height - 1) - y) * width * 4 + x] = buffer[y * 4 * width + x];
        }
    }
    
    // make data provider with data.
    CGDataProviderRef provider = CGDataProviderCreateWithData( NEKO_NULL, buffer2, myDataLength, NEKO_NULL );
    
    // Prepare some data.
    int32_t bitsPerComponent = 8;
    int32_t bitsPerPixel = 32;
    int32_t bytesPerRow = 4 * width;
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    // Make CGImage.
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NEKO_NULL, NO, renderingIntent);
    
    // Make UIImage.
    UIImage *myImage = [UIImage imageWithCGImage:imageRef];
    return myImage;
}

-(void)captureToPhotoAlbum {
    UIImage *image = [self glToUIImage];
    UIImageWriteToSavedPhotosAlbum(image, self, nil, nil);
}

// heart
//fs = "precision mediump float; uniform float time; void main(void) { highp vec2 p = (2.0*gl_FragCoord.xy-vec2(960., 640.0))/640.; p.y -= 0.25; mediump float tt = mod(time,1.0); mediump float ss = (sin(tt)*0.5+1.0)*0.3 + 0.5; ss -= ss*0.2*sin(tt*30.0)*exp(-tt*4.0); p *= vec2(0.5,1.5) + ss*vec2(0.5,-0.5); mediump float a = atan(p.x,p.y)/3.141593; mediump float r = length(p); mediump float h = abs(a); mediump float d = (13.0*h - 22.0*h*h + 10.0*h*h*h)/(6.0-5.0*h); mediump float s = 1.0-0.5*clamp(r/d,0.0,1.0); s = 0.75 + 0.75*p.x; s *= 1.0-0.25*r; s = 0.5 + 0.7*s*ss; s *= 0.5+0.5*pow( 1.0-clamp(r/d, 0.0, 1.0 ), 0.1 ); highp vec3 hcol = vec3(1.0,0.5*r,0.3)*s; gl_FragColor = vec4(hcol,1.0); }";

@end
