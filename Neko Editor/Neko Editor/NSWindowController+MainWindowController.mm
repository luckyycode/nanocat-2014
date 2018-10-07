//
//  NSWindowController+MainWindowController.m
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//



#import "NSWindowController+MainWindowController.h"


@interface MainWindowController (PrivateMethods)

@end

@implementation MainWindowController



- (void)windowDidLoad {
    [super windowDidLoad];
    
    self.window.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];
   // self.window.styleMask = self.window.styleMask | NSFullSizeContentViewWindowMask;
   // self.window.titleVisibility = NSWindowTitleHidden;
    self.window.titlebarAppearsTransparent = YES;
    self.window.movableByWindowBackground = YES;
}

@end
