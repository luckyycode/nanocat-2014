//
//  NSViewController+GraphicsView.m
//  Nanocat
//
//  Created by Kawaii Neko on 11/7/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#import "GraphicsView.h"
#import "../../../Core/Core.h"

@implementation EventDelegatingView

- (void)runNekoGame
{
   
}

- (id)init
{
    self = [super init];
//     [self setAcceptsTouchEvents:YES];
    
    return self;
}

- (void)awakeFromNib
{
//     [self setAcceptsTouchEvents:YES];
}

/**
 *  Always let it be NO, causes event system to freeze.
 */
- (BOOL)acceptsTouchEvents
{
    return NO;
}


- (BOOL)isOpaque
{
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    return YES;
}

// The following two methods allow a view to accept key input events. (literally they say, YES, please send me those events if I'm the center of attention.)
- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (void)cursorUpdate:(NSEvent *)event
{
    
}


- (id)initWithWindow:(NSWindow*)window
{
    if( self = [super init] )
    {
        self->m_Window = window;
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
    printf( "wtf\n" );
}


// Notice these don't do anything but call the eventDelegate. I could do whatever here, but I didn't.
// The NICE thing about delgation is, the originating object stays in control of it sends to its delegate.
// However, true to the meaning of the word 'delegate', once you pass something to the delegate, you have delegated some decision making power to that delegate object and no longer have any control (if you did, you might have a bad code smell in terms of the delegation design pattern.)
- (void)mouseDown:(NSEvent *)theEvent
{
    NSEvent *newEvent = theEvent;
    
    while (YES)
    {
        newEvent = [[self window] nextEventMatchingMask:NSLeftMouseDraggedMask | NSLeftMouseUpMask];
        
        if (NSLeftMouseUp == [newEvent type]) {
            break;
        }
    }
   
    [self.eventDelegate view:self didHandleEvent:theEvent];
    
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    NSEvent *newEvent = theEvent;
    
    while (YES)
    {
        newEvent = [[self window] nextEventMatchingMask:NSRightMouseDraggedMask | NSRightMouseUpMask];
        
        if (NSRightMouseUp == [newEvent type])
        {
            break;
        }
    }
    
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent
{
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent
{
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

- (void)keyDown:(NSEvent *)theEvent
{
//    NSEvent *newEvent = theEvent;
//    
//    while (YES)
//    {
//        newEvent = [[self window] nextEventMatchingMask:NSKeyUpMask];
//        
//        if (NSKeyUp == [newEvent type])
//        {
//            break;
//        }
//    }
//   
//    
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

- (void)scrollWheel:(NSEvent *)theEvent
{
    [self.eventDelegate view:self didHandleEvent:theEvent];
}

@end