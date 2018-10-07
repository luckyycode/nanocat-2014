//
//  NSOpenGLView+OpenGL4View.h
//  Neko Effects Editor
//
//  Created by Neko Code on 5/25/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>

#import <QuartzCore/CVDisplayLink.h>
#import "MacUtilities.h"

@interface OpenGL4View : NSOpenGLView {
    CVDisplayLinkRef displayLink;
}

@end
