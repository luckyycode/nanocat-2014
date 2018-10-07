//
//  ViewController.m
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import "ViewController.h"

#import "MacUtilities.h"
#include "MaterialLoader.h" // Material API

#include "BeautifulEnvironment.h"

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    red_gradient = [CAGradientLayer layer];
    green_gradient = [CAGradientLayer layer];
    blue_gradient = [CAGradientLayer layer];
    
    cs1 = [[ColoredSlider alloc] init];
    cs2 = [[ColoredSlider alloc] init];
    cs3 = [[ColoredSlider alloc] init];

    // Do any additional setup after loading the view.
}

- (IBAction)onColorSwitch:(id)sender {
    if( [sender state] == NSOnState ) {
        [self.rValue setEnabled:NO];
        [self.gValue setEnabled:NO];
        [self.bValue setEnabled:NO];
        
        [self.colorRedSlider setEnabled:NO];
        [self.colorBlueSlider setEnabled:NO];
        [self.colorGreenSlider setEnabled:NO];
        [self.resultingColor setEnabled:NO];
        NSLog( @"Random color - yes" );
    } else {
        [self.rValue setEnabled:YES];
        [self.gValue setEnabled:YES];
        [self.bValue setEnabled:YES];
        
        [self.colorRedSlider setEnabled:YES];
        [self.colorBlueSlider setEnabled:YES];
        [self.colorGreenSlider setEnabled:YES];
        [self.resultingColor setEnabled:YES];
        NSLog( @"Random color - no" );
    }
}

- (IBAction)modificatorSwitch:(id)sender {
    if( [sender state] != NSOnState ) {
        [self.originMod setEnabled:NO];
        [self.scaleMod setEnabled:NO];
        [self.colorMod setEnabled:NO];
        [self.gravityMod setEnabled:NO];
        
        NSLog( @"Allow modificators - yes" );
    } else {
        [self.originMod setEnabled:YES];
        [self.scaleMod setEnabled:YES];
        [self.colorMod setEnabled:YES];
        [self.gravityMod setEnabled:YES];
        
        NSLog( @"Allow modificators - no" );
    }
    
}

- (IBAction)openImage:(id)sender {

    // Create the File Open Dialog class.
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    
    // Enable the selection of files in the dialog.
    [openDlg setCanChooseFiles:YES];
    
    // Disable the selection of directories in the dialog.
    [openDlg setCanChooseDirectories:NO];
    
    // Set title.
    [openDlg setTitle:@"Pick an image"];
    
    // Only one file allowed.
    [openDlg setAllowsMultipleSelection:NO];
    
    // Set image file type names.
    NSArray * fileTypeNames = [[NSArray alloc] initWithObjects:@"jpg", @"tga", @"png", @"bmp", nil];
    [openDlg setAllowedFileTypes:fileTypeNames];
    
    // Display the dialog.  If the OK button was pressed,
    // process the files.
    if ( [openDlg runModal] == NSModalResponseOK )
    {
        // Get an array containing the full filenames of all
        // files and directories selected.
        //NSArray* files = [openDlg filenames];
        
        // Get directory 'URL'.
        //NSURL * fullDirectoryPathURL = [openDlg directoryURL];
        
        // Make NSURL to string.
        //NSString * fullDirectoryPath = [fullDirectoryPathURL absoluteString];
        
        // Remove URL prefix 'file://'.
        //NSString * directoryPath = [fullDirectoryPath substringFromIndex:7];
        
        // Loop through all the files and process them.
        for( NSURL* URL in [openDlg URLs] )
        {
            NSString * fileName = [URL path];
            //NSString * filePathWithName = [NSString stringWithFormat:@"%@/%@", directoryPath, fileName];
            
            NSImage * imageChoosen = [[NSImage alloc] initWithContentsOfFile:fileName];
            if( !imageChoosen ) {
                NSLog( @"Dafuq? Failed to load image file from %@\n", fileName );
            } else {
                // Create new material entry.
                NSLog( @"Creating new material for Neko.." );
                
                // Set preview image.
                [self.materialPreview setImage:imageChoosen];
                
                const unsigned int iWidth = imageChoosen.size.width;
                const unsigned int iHeight = imageChoosen.size.height;
                
                NSString * fileNameNoExtension = [fileName stringByDeletingPathExtension];
            
                NSData * imageData = [imageChoosen TIFFRepresentation];

                NSUInteger len = [imageData length];
                Byte * byteData = new Byte[len];
                memcpy(byteData, [imageData bytes], len);
                
                NSBitmapImageRep *rep =
                [NSBitmapImageRep imageRepWithData: imageData];

                Neko::f_AssetBase->p_MaterialBase->CreateMaterial( [fileNameNoExtension UTF8String], iWidth, iHeight, byteData, [rep bitsPerPixel]);
                
                delete [] byteData;
                
                Neko::ncGFXParticle * sceneParticle = Neko::g_pbEnv->particleEngine->GetParticleAtIndex( 0 );
                sceneParticle->SetMaterial( [fileNameNoExtension UTF8String] );
            }
        }
    }
}

- (IBAction)sliderValueChanged:(NSSlider *)sender {
    //NSLog(@"slider value = %f", [self.colorRedSlider doubleValue]);
    [self UpdateSliderValues:NO];
}

- (IBAction)generateRandom:(id)sender {
    
    NSLog( @"Generating random particle.." );
    
    
    
}

- (void)UpdateSliderValues:(BOOL)colorUpdate {
    
    float sliderRedValue = ([self.colorRedSlider floatValue]) / 255.0;
    float sliderGreenSlider = ([self.colorGreenSlider floatValue]) / 255.0;
    float sliderBlueValue = ([self.colorBlueSlider floatValue]) / 255.0;
    
    if( colorUpdate ) { // Used textfields to update colors.
        sliderRedValue = [self.rValue doubleValue] / 255.0;
        sliderGreenSlider = [self.gValue doubleValue] / 255.0;
        sliderBlueValue = [self.bValue doubleValue] / 255.0;

        [self.colorRedSlider setDoubleValue:sliderRedValue * 255.0];
        [self.colorGreenSlider setDoubleValue:sliderGreenSlider * 255.0];
        [self.colorBlueSlider setDoubleValue:sliderBlueValue * 255.0];
    }

    [cs1 setFirstColor:sliderRedValue g1:0.0 b1:0.0 a1:1.0];
    [cs1 setSecondColor:sliderRedValue g1:0.0 b1:0.0 a1:1.0];
    
    [cs2 setFirstColor:0.0 g1:sliderGreenSlider b1:0.0 a1:1.0];
    [cs2 setSecondColor:0.0 g1:sliderGreenSlider b1:0.0 a1:1.0];
    
    [cs3 setFirstColor:0.0 g1:0.0 b1:sliderBlueValue a1:1.0];
    [cs3 setSecondColor:0.0 g1:0.0 b1:sliderBlueValue a1:1.0];
    
    
    [self.colorBlueSlider setCell:cs3];
    [self.colorGreenSlider setCell:cs2];
    [self.colorRedSlider setCell:cs1];
    
    if( !colorUpdate ) {
        [self.rValue setStringValue:[NSString stringWithFormat:@"%i", (int)[self.colorRedSlider floatValue]]];
        [self.gValue setStringValue:[NSString stringWithFormat:@"%i", (int)[self.colorGreenSlider floatValue]]];
        [self.bValue setStringValue:[NSString stringWithFormat:@"%i", (int)[self.colorBlueSlider floatValue]]];
    }
    [self.viewColorBar setWantsLayer:YES];
    [self.viewColorBar.layer setBackgroundColor:[[NSColor colorWithRed:sliderRedValue green:sliderGreenSlider blue:sliderBlueValue alpha:1.0] CGColor]];
    //[self.viewColorBar setTextColor:[NSColor colorWithRed:sliderRedValue green:sliderGreenSlider blue:sliderBlueValue alpha:1.0]];
    
    if( Neko::g_pbEnv->particleEngine ) {
        Neko::ncGFXParticle * sceneParticle = Neko::g_pbEnv->particleEngine->GetParticleAtIndex( 0 );
        
        Neko::Vec3 color = Neko::Vec3( sliderRedValue, sliderGreenSlider, sliderBlueValue );
        Neko::Vec3 gravity = Neko::Vec3( [self.gravityX floatValue], [self.gravityY floatValue], [self.gravityZ floatValue] );
        Neko::Vec3 minVelocity = Neko::Vec3( [self.minVelocityX floatValue], [self.minVelocityY floatValue], [self.minVelocityZ floatValue] );
        Neko::Vec3 maxVelocity = Neko::Vec3( [self.maxVelocityX floatValue], [self.maxVelocityY floatValue], [self.maxVelocityZ floatValue] );
        
        sceneParticle->SetColor( color );
        sceneParticle->SetSize( [self.sizeSlider floatValue] );
        sceneParticle->SetAmount( [self.amountTextField intValue] );
        sceneParticle->SetLifeTime( [self.minLifeTimeTextField floatValue], [self.maxLifeTimeTextField floatValue] );
        sceneParticle->SetGravity( gravity );
        sceneParticle->SetVelocity( minVelocity, maxVelocity );
        sceneParticle->SetSpeed( [self.speedSlider floatValue] );
    }
    //[[self.colorBlueSlider addTarget:self action:@selector(sliderValueChanged:) forControlEvents:NSControlEventValueChanged];
}

- (void)viewDidAppear {
    [super viewDidAppear];
 
    
    [self UpdateSliderValues:NO];
    
    [self.colorRedSlider setAction:@selector(sliderValueChanged:)];
    [self.colorGreenSlider setAction:@selector(sliderValueChanged:)];
    [self.colorBlueSlider setAction:@selector(sliderValueChanged:)];
    
    [self.sizeSlider setAction:@selector(sliderValueChanged:)];
    
    [self.maxVelocityX setAction:@selector(sliderValueChanged:)];
    [self.maxVelocityY setAction:@selector(sliderValueChanged:)];
    [self.maxVelocityZ setAction:@selector(sliderValueChanged:)];
    
    [self.minVelocityX setAction:@selector(sliderValueChanged:)];
    [self.minVelocityY setAction:@selector(sliderValueChanged:)];
    [self.minVelocityZ setAction:@selector(sliderValueChanged:)];
    
    [self.gravityX setAction:@selector(sliderValueChanged:)];
    [self.gravityY setAction:@selector(sliderValueChanged:)];
    [self.gravityZ setAction:@selector(sliderValueChanged:)];
    
    [self.speedSlider setAction:@selector(sliderValueChanged:)];
    
    // Maximum color for pixel.
    [self.colorRedSlider setMaxValue:255.0];
    [self.colorGreenSlider setMaxValue:255.0];
    [self.colorBlueSlider setMaxValue:255.0];
    
    self.rValue.delegate = self; self.rValue.tag = 1;
    self.gValue.delegate = self; self.gValue.tag = 2;
    self.bValue.delegate = self; self.bValue.tag = 3;
}

- (void)controlTextDidChange:(NSNotification *)notification {
     //NSTextField *textField = [notification object];
 
     [self UpdateSliderValues:YES];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}


@end
