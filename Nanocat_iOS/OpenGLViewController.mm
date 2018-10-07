//
//  OpenGLViewController.m
//  MyCode
//
//  Created by Neko Code on 5/2/14.
//  Copyright (c) 2014 neko. All rights reserved.
//


#include "Core.h"
#include "Renderer.h"
#include "OpenGL.h"
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

@interface OpenGLViewController()

// Some connections.
@property (weak, nonatomic) IBOutlet JSAnalogueStick *AnalogueStick;
@property (weak, nonatomic) IBOutlet UIView *backView;
@property (weak, nonatomic) IBOutlet UIActivityIndicatorView *activityIndicator;
@property (weak, nonatomic) IBOutlet UILabel *loadingLabel;
@property (weak, nonatomic) IBOutlet UIButton *magicButton;
@property (weak, nonatomic) IBOutlet UIToolbar *devToolbar;

// OpenGL context.
@property (strong, nonatomic) EAGLContext *context;
@end

@implementation OpenGLViewController
- (IBAction)MagicButton:(id)sender {
    bEnv->Feedback();
}

static char homePath[256];

#pragma mark - View stuff.
- (id)initWithCoder:(NSCoder*)coder {

    self = [super initWithCoder:coder];
    
    if (!self) {
        return nil;
    }
    
    // Preload app logic.
    g_Core->Preload( iOS_GetDocumentsFolder() );
    g_mainRenderer->Preload();
    
    g_Core->UseGraphics = true; // We don't have any other choice on iOS, right?!?!

    return self;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
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
    view.drawableStencilFormat  = GLKViewDrawableStencilFormat8;
    
    self.view = view;
    
    [EAGLContext setCurrentContext:self.context];

    glClearColor( 0.4, 0.9, 0.9, 0.0 );

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    self.preferredFramesPerSecond = 60;
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    
    
    // Blur Effect
    //UIBlurEffect *blurEffect = [UIBlurEffect effectWithStyle:UIBlurEffectStyleDark];
    //UIVisualEffectView *bluredEffectView = [[UIVisualEffectView alloc] initWithEffect:blurEffect];
    //[bluredEffectView setFrame:self.devToolbar.frame];
    //[self.devToolbar setAlpha:0.0f];
    [self.devToolbar setAlpha:0.5f];
    //[self.devToolbar insertSubview:bluredEffectView belowSubview:self.devToolbar];
    
    
    // Manage some stuff.
    [self.loadingLabel setHidden:YES];
    [self.activityIndicator setHidden:YES];
    [self.AnalogueStick setHidden:NO];
    [self.magicButton setHidden:NO];
    
    [self.loadingLabel setText:@"Please wait.."];
    
    // Load application logic now.
    g_Core->Initialize();
    gl_Core->Initialize();
    
    gl_Core->Initialized = true;
    //_opengl.OnResize( Render_Width.GetInteger(), Render_Height.GetInteger() );
    
    g_Core->Loaded();
}

- (void)dealloc {

}

#pragma mark - Drawing and frame methods.
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    [self Render:g_Core->Time];
}

- (void)update {
    g_Core->Frame();
}

- (void)RenderToShader:(ncRenderer::SceneEye)eye  {
    
    g_mainRenderer->g_sceneBuffer[eye]->Bind();

    glViewport( self.view.frame.origin.x, self.view.frame.origin.y, 640.0, 480.0 );
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    g_mainRenderer->RenderWorld( g_Core->Time, eye );
    
    g_Console->Render();
    
    g_mainRenderer->g_sceneBuffer[eye]->Unbind();
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Here comes shader.
    g_mainRenderer->sceneShader->Use();
    g_mainRenderer->sceneShader->SetUniform( "perspectiveInvMatrix", 1, GL_FALSE, g_playerCamera->ProjectionInvertMatrix.m );
    
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[eye]->GetDiffuseTexture() );
    
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[eye]->GetDepthTexture() );
    
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->lens_texture );
    
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[eye]->GetNoColorMaskTexture() );
    
    glActiveTexture( GL_TEXTURE4 );
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[eye]->GetNormalTexture() );
    
    glActiveTexture( GL_TEXTURE5 );
    glBindTexture( GL_TEXTURE_2D, bEnv->dPointLightRenderer._pointLightTexture );
    
    glActiveTexture( GL_TEXTURE6 );
    glBindTexture( GL_TEXTURE_2D, g_materialBase->Find( "lambertMap" )->Image.TextureID );
    
    //glActiveTexture( GL_TEXTURE7 );
    //glBindTexture( GL_TEXTURE_2D, g_materialBase->Find( "sunglare" )->Image.TextureID );
    
    glViewport( 0.0f, 0.0f, g_mainRenderer->renderWidth, g_mainRenderer->renderHeight );
    
    g_mainRenderer->sceneShader->SetUniform( "time", g_Core->Time / 10000.0f );
    g_mainRenderer->sceneShader->SetUniform( "lightDirection", bEnv->GetSunLightPosition() );
    g_mainRenderer->sceneShader->SetUniform( "worldMatrix", 1, GL_FALSE, g_playerCamera->ViewMatrix.m );
    g_mainRenderer->sceneShader->SetUniform( "mFogColor", bEnv->GetSkyLightColor() );
    if( g_mainRenderer->m_updatePostProcess ) {
        static GLint viewport[4];
        
        glGetIntegerv( GL_VIEWPORT, viewport );
        
        static GLfloat winX = 0;
        static GLfloat winY = 0;
        static GLfloat winZ = 0;
        
        ncUtils::gluProject(	 bEnv->GetSunLightPosition().x,
                            bEnv->GetSunLightPosition().y,
                            bEnv->GetSunLightPosition().z,
                            g_playerCamera->ViewMatrix.m,
                            g_playerCamera->ProjectionMatrix.m,
                            viewport,
                            &winX,
                            &winY,
                            &winZ);
        
        
        float uniformLightX = winX / ( (float)g_mainRenderer->renderWidth / g_mainRenderer->renderAspectRatio );
        float uniformLightY = winY / ( (float)g_mainRenderer->renderHeight / g_mainRenderer->renderAspectRatio ) ;
        g_mainRenderer->sceneShader->SetUniform( "lightPositionOnScreen", uniformLightX, uniformLightY );
        
        g_mainRenderer->m_updatePostProcess = false;
    }
    
    glBindVertexArray( g_mainRenderer->eye_vao[eye] );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( 0 );
    
    //glActiveTexture( GL_TEXTURE7 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE6 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE5 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE4 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    g_mainRenderer->sceneShader->Next();
}

/*
    Lets see the world!
*/
- (void)Render:(int)msec {
    
    if( !gl_Core->Initialized && !g_mainRenderer->Initialized )
        return;

    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // Here we render the world.
    
    /*
     Render some stuff first.
    */
    g_mainRenderer->PreRender();
    
    // Here you can tell what you want to render.

    [self RenderToShader:ncRenderer::EYE_FULL];
    
    // Console is moved out from *render to texture* since we don't care about its rendering,
    // it must be always shown correctly.
    //_gconsole.Render();
}

#pragma mark - Touch methods.
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    
    UITouch * touch = [touches anyObject];
    CGPoint location = [touch locationInView:self.view];
    CGPoint lastLoc = [touch previousLocationInView:self.view];
    CGPoint diff = CGPointMake(lastLoc.x - location.x, lastLoc.y - location.y);
    
    float rotX = -1 * GLKMathDegreesToRadians(diff.y / 2.0);
    float rotY = -1 * GLKMathDegreesToRadians(diff.x / 2.0);
    
    g_playerCamera->RotationMatrix.Identity();
    
    g_Input->SetMouseHold(false);// = false;
    

    if( diff.y != 0 ) {
        g_playerCamera->RotationMatrix.Rotate( rotX * 17.0, g_playerCamera->g_vRight );
        
        g_playerCamera->lastRotation.TransformVector( g_playerCamera->RotationMatrix );
        g_playerCamera->g_vUp.TransformVector( g_playerCamera->RotationMatrix );
    }
    
    if( diff.x != 0 ){
        g_playerCamera->RotationMatrix.Rotate( rotY * 17.0, VECTOR_UP );
        
        g_playerCamera->lastRotation.TransformVector( g_playerCamera->RotationMatrix );
        g_playerCamera->g_vUp.TransformVector( g_playerCamera->RotationMatrix );
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {

}

float mCurrentScale = 1.0;
float mLastScale = 1.0;

- (void)handlePinch:(UIPinchGestureRecognizer*)sender {
    
    mCurrentScale += [sender scale] - mLastScale;
    mLastScale = [sender scale];
    
    if( sender.state == UIGestureRecognizerStateEnded ) {
        mLastScale = 1.0;
    }
}

#pragma mark - Buttons
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex {
    if( alertView.tag == 2 ) {
        UITextField *data = [alertView textFieldAtIndex:0];
        
        g_Console->Execute( [data.text UTF8String] );
    }
}

- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
    if( actionSheet.tag == 1 ) {
       
        if( buttonIndex == 0 ) { // Create local game.
            gl_Core->OnResize( 640, 480 );
            g_Console->Execute( "local" );
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
             g_Console->Execute("sm cat.sm1");
        }
    }
}

- (IBAction)woot {
    UIActionSheet *pl = [[UIActionSheet alloc] initWithTitle:@"Please choose" delegate:self cancelButtonTitle:@"Snapshot" destructiveButtonTitle:@"Create local game" otherButtonTitles:@"Execute a command", nil];
    
    [pl setTag:1];
    [pl showInView:self.view];
}

#pragma mark - Analogue stick

- (void)analogueStickDidChangeValue:(JSAnalogueStick *)analogueStick ended:(BOOL)end {
    
    float LnormalizedX = self.AnalogueStick.xValue;
    float LnormalizedY = self.AnalogueStick.yValue;
    
    int rX = end == YES ? 128 : 255 * ( 1 + LnormalizedX ) / 2;        // x axis
    int rY = end == YES ? 128 : 255 * ( 1 + LnormalizedY ) / 2;        // y axis

    // FB
    if( rY >= 127 ) {
        g_Input->MakeKeyEvent( NCKEY_FORWARD );
    } else if( rY <= 127 ) {
        g_Input->MakeKeyEvent( NCKEY_BACKWARD );
    }
    
    // LR
    if( rX >= 127 ) {
        
    } else if( rX <= 127 ) {
        
    }
}

#pragma mark - Get Documents folder.

const char *iOS_GetDocumentsFolder( void ) {
    
    char *p;
    
    if( *homePath )
        return homePath;
    
    if( (p = getenv("HOME")) != NULL ) {
        strncpy( homePath, p, sizeof(homePath) );
        strcat( homePath, "/Documents" );
        
        if( mkdir( homePath, 0777 ) ) {
            if( errno != EEXIST ) {
                g_Core->Error( ERR_FILESYSTEM, "No 'Documents' folder found." );
            }
        }
        return homePath;
    }
    
    return ""; // Is it good?
}

#pragma mark - Capture a screenshot.
-(UIImage *) glToUIImage {
    
    // Fix me: real screen resolution for all devices.
    const int width = 1136;//_renderer.renderWidth;
    const int height = 640;//_renderer.renderHeight;
    
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
    CGDataProviderRef provider = CGDataProviderCreateWithData( NULL, buffer2, myDataLength, NULL );
    
    // Prepare some data.
    int bitsPerComponent = 8;
    int bitsPerPixel = 32;
    int bytesPerRow = 4 * width;
    
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;
    
    // Make CGImage.
    CGImageRef imageRef = CGImageCreate(width, height, bitsPerComponent, bitsPerPixel, bytesPerRow, colorSpaceRef, bitmapInfo, provider, NULL, NO, renderingIntent);
    
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
