//
//  NSWindowController+ColoredSlider.h
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ColoredSlider : NSSliderCell {
    CGFloat r, g, b, a;
    CGFloat sr, sg, sb, sa;
}

- (void) setFirstColor: (CGFloat)r g1:(CGFloat)g b1:(CGFloat)b a1:(CGFloat)a;
- (void) setSecondColor: (CGFloat)r g1:(CGFloat)g b1:(CGFloat)b a1:(CGFloat)a;

@end
