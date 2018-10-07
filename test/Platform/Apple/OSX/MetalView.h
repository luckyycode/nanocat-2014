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
//  MetalView.h
//  Metal OSX context view. ;D
//
//  Created by Kawaii Neko on 6/13/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import "../../../AssetCommon/Material/MaterialLoader.h"


class MetalData {
public:
    __strong id<MTLDevice> device;
    __strong MTLRenderPassDescriptor *passDescriptor;
    
    __strong id<MTLCommandQueue> commandQueue;
    
    __strong id<MTLRenderPipelineState> pipeline;
    __strong id<MTLRenderPipelineState> compositionPipeline;
    
    __strong  id<MTLBuffer> uniformBuffer;
    
    __strong id<MTLLibrary>  library;
    
    __strong id<MTLCommandBuffer> commandBuffer;
    __strong id <MTLRenderCommandEncoder> commandEncoder;
};

extern MetalData * MetalDataBase;

@interface MetalView : NSView

// Current Metal Layer.
@property (nonatomic, weak) CAMetalLayer *metalLayer;

// the current drawable created within the view's CAMetalLayer
@property (nonatomic, readonly) id <CAMetalDrawable> currentDrawable;


- (void)draw;
- (void)make;



@end

