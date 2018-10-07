//
//  NSViewController+GraphicsView.h
//  Nanocat
//
//  Created by Kawaii Neko on 11/7/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol EventDelegatingViewDelegate <NSObject>

- (void)view:(NSView *)aView didHandleEvent:(NSEvent *)anEvent;

@end

@interface EventDelegatingView : NSView
{
    NSWindow * m_Window;
}

@property id<EventDelegatingViewDelegate> eventDelegate;

@end