//
//  ViewController.h
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import "NSWindowController+ColoredSlider.h"

@interface ViewController : NSViewController <NSTextFieldDelegate>
{
    CAGradientLayer * red_gradient;
    CAGradientLayer * green_gradient;
    CAGradientLayer * blue_gradient;
    
    ColoredSlider * cs1, * cs2, * cs3; // r, g, b

    
}

- (IBAction)sliderChanged:(id)sender;
@property (weak) IBOutlet NSButton *allowModificators;
@property (weak) IBOutlet NSComboBox *originMod;
@property (weak) IBOutlet NSComboBox *scaleMod;
@property (weak) IBOutlet NSComboBox *colorMod;
@property (weak) IBOutlet NSComboBox *gravityMod;

@property (weak) IBOutlet NSSlider *originX;
@property (weak) IBOutlet NSSlider *originU;
@property (weak) IBOutlet NSSlider *originZ;
@property (weak) IBOutlet NSView *viewColorBar;

@property (weak) IBOutlet NSSlider *speedSlider;

@property (weak) IBOutlet NSSlider *maxVelocityX;
@property (weak) IBOutlet NSSlider *maxVelocityY;
@property (weak) IBOutlet NSSlider *maxVelocityZ;

@property (weak) IBOutlet NSSlider *minVelocityX;
@property (weak) IBOutlet NSSlider *minVelocityY;
@property (weak) IBOutlet NSSlider *minVelocityZ;

@property (weak) IBOutlet NSSlider *gravityX;
@property (weak) IBOutlet NSSlider *gravityY;
@property (weak) IBOutlet NSSlider *gravityZ;

@property (weak) IBOutlet NSTextField *amountTextField;
@property (weak) IBOutlet NSTextField *minLifeTimeTextField;
@property (weak) IBOutlet NSTextField *maxLifeTimeTextField;

@property (weak) IBOutlet NSTextField *rValue;
@property (weak) IBOutlet NSTextField *gValue;
@property (weak) IBOutlet NSTextField *bValue;

@property (weak) IBOutlet NSImageView *materialPreview;

@property (weak) IBOutlet NSSlider *sizeSlider;

@property (weak) IBOutlet NSSlider *colorRedSlider;
@property (weak) IBOutlet NSSlider *colorGreenSlider;
@property (weak) IBOutlet NSSlider *colorBlueSlider;
@property (weak) IBOutlet NSTextField *resultingColor;


@end

