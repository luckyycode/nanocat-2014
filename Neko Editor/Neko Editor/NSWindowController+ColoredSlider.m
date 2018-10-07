//
//  NSWindowController+ColoredSlider.m
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import "NSWindowController+ColoredSlider.h"

@implementation ColoredSlider

- (void) setFirstColor: (CGFloat)r2 g1:(CGFloat)g2 b1:(CGFloat)b2 a1:(CGFloat)a2 {
    r = r2;
    g = g2;
    b = b2;
    a = a2;
}


- (void) setSecondColor: (CGFloat)r2 g1:(CGFloat)g2 b1:(CGFloat)b2 a1:(CGFloat)a2 {
    sr = r2;
    sg = g2;
    sb = b2;
    sa = a2;
}

- (void)drawBarInside:(NSRect)rect flipped:(BOOL)flipped {
    
    //  [super drawBarInside:rect flipped:flipped];
    
    rect.size.height = 2.0;
    
    // Bar radius
    CGFloat barRadius = 2.5;
    
    // Knob position depending on control min/max value and current control value.
    CGFloat value = ([self doubleValue]  - [self minValue]) / ([self maxValue] - [self minValue]);
    
    // Final Left Part Width
    CGFloat finalWidth = value * ([[self controlView] frame].size.width - 8);
    
    // Left Part Rect
    NSRect leftRect = rect;
    leftRect.size.width = finalWidth;

    // Draw Left Part
    NSBezierPath* bg = [NSBezierPath bezierPathWithRoundedRect: rect xRadius: barRadius yRadius: barRadius];
    [[NSColor colorWithRed:r green:g blue:b alpha:a] setFill];
    
    [bg fill];
    
    // Draw Right Part
    NSBezierPath* active = [NSBezierPath bezierPathWithRoundedRect: leftRect xRadius: barRadius yRadius: barRadius];
    
    [[NSColor colorWithRed:sr green:sg blue:sb alpha:sa] setFill];
    
    [active fill];
    
    
}

@end