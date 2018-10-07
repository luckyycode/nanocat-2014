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

#import "MetalView.h"
#import "common.h" // Shader common file.

#import "Camera.h"
#import "BeautifulEnvironment.h"
#import "Mesh.h"

#import "MacWindow.h"

#import "MetalExternal.h"

#import "Hashtable.h"
#import "MaterialLoader.h"
#import "../../../AssetCommon/AssetBase.h"

#import "../../../Graphics/GraphicsManager.h"

#import "MetalReferences.h"

/**
 *  Metal implementation.
 */
@implementation MetalView
{
    id <MTLTexture>  _depthTex;
    id <MTLTexture>  _stencilTex;
    id <MTLTexture>  _colorTextures[3]; // These are for textures 1-3 (as needed), texture 0 is owned by the drawable.
    
    id<MTLLibrary>  library;
    
    id<MTLCommandBuffer> commandBuffer;
    id <MTLRenderCommandEncoder> commandEncoder;
    
    id<MTLDepthStencilState>    _compositionDepthState;
    id<MTLDepthStencilState>    _gbufferDepthState;
    
    id<MTLBuffer> vertexBuffer;
    id<MTLBuffer> indexBuffer;

    Neko::ncMesh * coolBadass ;

    
    id<MTLBuffer>               _quadPositionBuffer;
    id<MTLBuffer>               _quadTexcoordBuffer;
    
    dispatch_semaphore_t        _inflight_semaphore;
}

@synthesize currentDrawable    = _currentDrawable;

MetalData  local_MetalData;
MetalData * MetalDataBase = &local_MetalData;

using namespace Neko; // Umm...?

- (MTLRenderPassColorAttachmentDescriptor *)colorAttachmentDescriptorWithTexture: (id <MTLTexture>) texture
                                                                      clearColor: (MTLClearColor) clearColor
                                                                      loadAction: (MTLLoadAction) loadAction
                                                                     storeAction: (MTLStoreAction) storeAction
{
    MTLRenderPassColorAttachmentDescriptor *attachment = [MTLRenderPassColorAttachmentDescriptor new];
    attachment.texture = texture;
    attachment.loadAction = loadAction;
    attachment.storeAction = storeAction;
    attachment.clearColor = clearColor;
    
    return attachment;
}

- (void)setupRenderPassDescriptorForTexture:(id <MTLTexture>) texture
{
    MTLPixelFormat colorAttachmentFormat[4];
    MTLClearColor colorAttachmentClearValue[4];
    MTLPixelFormat depthPixelFormat;
    double depthAttachmentClearValue;
    MTLPixelFormat stencilPixelFormat;
    uint32_t stencilAttachmentClearValue;
    


    colorAttachmentFormat[0] = MTLPixelFormatBGRA8Unorm;
    colorAttachmentClearValue[0] = MTLClearColorMake( 0.0, 0.0, 1.0, 0.0 );
    
    colorAttachmentFormat[1] = MTLPixelFormatBGRA8Unorm;
    colorAttachmentClearValue[1] = MTLClearColorMake( 1.0, 0.0, 0.0, 0.0 );
    
    colorAttachmentFormat[2] = MTLPixelFormatR32Float;
    colorAttachmentClearValue[2] = MTLClearColorMake( 0.0, 1.0, 1.0, 0.0 );
    
    colorAttachmentFormat[3] = MTLPixelFormatBGRA8Unorm;
    colorAttachmentClearValue[3] = MTLClearColorMake(1.0, 0.0, 1.0, 0.0);
    
    depthPixelFormat = MTLPixelFormatDepth32Float;
    depthAttachmentClearValue = 1.0;
    
    stencilPixelFormat = MTLPixelFormatStencil8;
    stencilAttachmentClearValue = 0;
    
    // create renderpass descriptor lazily when we first need it
    if (MetalDataBase->passDescriptor == nil)
        MetalDataBase->passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
    
    // set up the first color attachment at index 0.
    // This will be the attachment we present to the screen so set its store action to store.
    MTLRenderPassColorAttachmentDescriptor *colorAttachment0 = [self colorAttachmentDescriptorWithTexture:texture
                                                                                               clearColor:colorAttachmentClearValue[0]
                                                                                               loadAction:MTLLoadActionClear
                                                                                              storeAction:MTLStoreActionStore];
    MetalDataBase->passDescriptor.colorAttachments[0] = colorAttachment0;
    ;
    // we only need to update the other attachments if something has changed (ie, rotation or layer changed size)
    BOOL doUpdate = (!_colorTextures[0]) || ( _colorTextures[0].width != texture.width  ) ||  (  _colorTextures[0].height != texture.height );
    if (doUpdate)
    {
        // color attachments 1..3 will not be presented and should be discarded so set their store action to dont care
        for (int i = 1; i <= 3; i++)
            if (colorAttachmentFormat[i] != MTLPixelFormatInvalid)
            {
                NSLog( @"Creating color attachment #%i\n", i );
                
                // color format 0 is only used by the drawable so we skip it here
                MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: colorAttachmentFormat[i]
                                                                                                width: texture.width
                                                                                               height: texture.height
                                                                                            mipmapped: NO];
                desc.usage = MTLTextureUsageRenderTarget;
                _colorTextures[i-1] = [MetalDataBase->device newTextureWithDescriptor: desc];
                
                MTLRenderPassColorAttachmentDescriptor *colorAttachment = [self colorAttachmentDescriptorWithTexture:_colorTextures[i-1]
                                                                                                          clearColor:colorAttachmentClearValue[i]
                                                                                                          loadAction:MTLLoadActionClear
                                                                                                         storeAction:MTLStoreActionDontCare];
                MetalDataBase->passDescriptor.colorAttachments[i] = colorAttachment;
            }
    }
    
    if( depthPixelFormat != MTLPixelFormatInvalid )
    {
        BOOL doUpdate = ( _depthTex.width != texture.width  ) ||  ( _depthTex.height != texture.height );
        if(!_depthTex || doUpdate)
        {
            //  If we need a depth texture and don't have one, or if the depth texture we have is the wrong size
            //  Then allocate one of the proper size
            MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: depthPixelFormat
                                                                                            width: texture.width
                                                                                           height: texture.height
                                                                                        mipmapped: NO];
            
            desc.textureType = MTLTextureType2D;
            desc.resourceOptions = MTLResourceStorageModePrivate;
            desc.usage = MTLTextureUsageRenderTarget;
            
            _depthTex = [MetalDataBase->device newTextureWithDescriptor: desc];
            
            MTLRenderPassDepthAttachmentDescriptor *depthAttachment = MetalDataBase->passDescriptor.depthAttachment;
            
            depthAttachment.texture = _depthTex;
            depthAttachment.loadAction = MTLLoadActionClear;
            depthAttachment.storeAction = MTLStoreActionDontCare;
            depthAttachment.clearDepth = 1.0;
        }
    }    
}


+ (id)layerClass {
    return [CAMetalLayer class];
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    if ((self = [super initWithCoder:aDecoder])) {
        [self make];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frameRect {
    if ((self = [super initWithFrame:frameRect]))
    {
        NSLog( @"initWithFrame - MetalView" );
        [self make];
    }

    return self;
}

- (void)make {
    [self setWantsLayer:YES];
    [self setNeedsDisplay:YES];

    
    // Create layer.
    _metalLayer = [CAMetalLayer layer];
    
    // Set layer.
    [self setLayer:_metalLayer];
    
    // Create Metal device.
    MetalDataBase->device = MTLCreateSystemDefaultDevice();
    
    _metalLayer.device = MetalDataBase->device;
    _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    CGSize rectSize = CGSizeMake( 1366.0, 768.0 );
    _metalLayer.drawableSize = rectSize;
    
//    [_metalLayer setFramebufferOnly:YES];
//    [_metalLayer setPresentsWithTransaction:NO];
    
    _inflight_semaphore = dispatch_semaphore_create(3);
    
    // - ---

    library = [MetalDataBase->device newDefaultLibrary];

    NSError *error = nil;
    
    float texcoords[] =
    {
        0.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };
    
    //All the combinations of quads needed.
    float quadVerts[] =
    {
        -1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, -1.0f,
        
        -1.0f, 1.0f,
        0.0f, 0.0f,
        -1.0f, 0.0f,
        -1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        
        0.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        
        -1.0f, 0.0f,
        0.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, -1.0f,
        
        0.0f, 0.0f,
        1.0f, -1.0f,
        0.0f, -1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, -1.0f,
        
    };

    
    _quadPositionBuffer = [MetalDataBase->device newBufferWithBytes:quadVerts length:sizeof(quadVerts) options:0];
    _quadTexcoordBuffer = [MetalDataBase->device newBufferWithBytes:texcoords length:sizeof(texcoords) options:0];
    
    
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    vertexDescriptor.attributes[0].offset = 0;

    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[1].bufferIndex = 1;
    vertexDescriptor.attributes[1].offset = 0;
    
    vertexDescriptor.attributes[2].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[2].bufferIndex = 2;
    vertexDescriptor.attributes[2].offset = 0;

    vertexDescriptor.layouts[0].stride = sizeof(Vec3);
    vertexDescriptor.layouts[1].stride = sizeof(Vec3);
    vertexDescriptor.layouts[2].stride = sizeof(Vec2);

    
    // Initialize new pipeline descriptor.
    MTLRenderPipelineDescriptor *pipelineDescriptor = [MTLRenderPipelineDescriptor new];
    
    pipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipelineDescriptor.colorAttachments[1].pixelFormat = MTLPixelFormatBGRA8Unorm;
    pipelineDescriptor.colorAttachments[2].pixelFormat = MTLPixelFormatR32Float;
    pipelineDescriptor.colorAttachments[3].pixelFormat = MTLPixelFormatBGRA8Unorm;
//    pipelineDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatStencil8;
    pipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;
    

    pipelineDescriptor.label = @"Composite";


    id<MTLFunction> vertComposite = [library newFunctionWithName:@"composition_vert"];
    id<MTLFunction> fragComposite = [library newFunctionWithName:@"composition_frag"];
    
    pipelineDescriptor.vertexFunction = vertComposite;
    pipelineDescriptor.fragmentFunction = fragComposite;
    
    MetalDataBase->compositionPipeline = [MetalDataBase->device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
    
    
    
    pipelineDescriptor.label = @"Mesh";
    
    
    id<MTLFunction> vertexFunc = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentFunc = [library newFunctionWithName:@"fragment_main"];
    
    // Mesh buffer.
    pipelineDescriptor.vertexFunction = vertexFunc;
    pipelineDescriptor.fragmentFunction = fragmentFunc;
    pipelineDescriptor.vertexDescriptor = vertexDescriptor;
    MetalDataBase->pipeline = [MetalDataBase->device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];

    
    

    MTLDepthStencilDescriptor *depthStencilDescriptor = [MTLDepthStencilDescriptor new];
    MTLStencilDescriptor *stencilState = [[MTLStencilDescriptor alloc] init];
    
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLess;
    depthStencilDescriptor.depthWriteEnabled = YES;
    
    _compositionDepthState = [MetalDataBase->device newDepthStencilStateWithDescriptor:depthStencilDescriptor];

    stencilState.stencilCompareFunction = MTLCompareFunctionAlways;
    stencilState.stencilFailureOperation = MTLStencilOperationKeep;
    stencilState.depthFailureOperation = MTLStencilOperationKeep;
    stencilState.depthStencilPassOperation = MTLStencilOperationReplace;
    stencilState.readMask = 0xFF;
    stencilState.writeMask = 0xFF;
    depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLessEqual;
    depthStencilDescriptor.frontFaceStencil = stencilState;
    depthStencilDescriptor.backFaceStencil = stencilState;
    
    _gbufferDepthState = [MetalDataBase->device newDepthStencilStateWithDescriptor: depthStencilDescriptor];
    
    
    
    
    
    
    g_pGraphicsManager->Initialize( GraphicsManager::INTERFACE_METAL );
//    g_mainRenderer->Initialize();
    
//    coolBadass = new Neko::ncMesh();
//    coolBadass->Owner = f_AssetBase->p_MeshBase->Find( "Gaige.obj" );
//    coolBadass->m_vPosition = Vec3(0.0f);
//    coolBadass->m_fScale = 1.0f;
    
    
    
    
    
    MetalDataBase->commandQueue = [MetalDataBase->device newCommandQueue];
}

simd::float4x4 ncMatrixToSIMDMatrix( const Neko::ncMatrix4 & matrix ) {
    simd::float4 a = { matrix.m[0], matrix.m[1], matrix.m[2], matrix.m[3] };
    simd::float4 b = { matrix.m[4], matrix.m[5], matrix.m[6], matrix.m[7] };
    simd::float4 c = { matrix.m[8], matrix.m[9], matrix.m[10], matrix.m[11] };
    simd::float4 d = { matrix.m[12], matrix.m[13], matrix.m[14], matrix.m[15] };
    
    simd::float4x4 mat = { a, b, c, d };
    return mat;
}

- (void)drawQuadWithRect:(id<MTLRenderCommandEncoder>) encoder offset:(int)quadOffset useTexture:(bool)textured
{
    
    int offset = quadOffset * sizeof(float) * 12;
    
    [encoder setVertexBuffer: _quadPositionBuffer offset: offset atIndex: 0];
    
    if( textured ) {
        [encoder setVertexBuffer: _quadTexcoordBuffer offset: 0 atIndex: 1];
    }
    
    [encoder drawPrimitives: MTLPrimitiveTypeTriangle vertexStart: 0 vertexCount: 6];
}


- (void)draw {
    // Update view matrix.
    g_Core->p_Camera->UpdateMatrices();

    ncMatrix4 wtf;
    wtf.Identity();
    wtf.Scale(Vec3(20.0));

    simd::float4x4 modelMatrix = ncMatrixToSIMDMatrix(wtf);

    
    simd::float4x4 viewMatrix = ncMatrixToSIMDMatrix(g_Core->p_Camera->ViewMatrix);
    simd::float4x4 projectionMatrix = ncMatrixToSIMDMatrix(Neko::g_Core->p_Camera->ProjectionMatrix);
    
    
    Uniforms uniforms;
    
    simd::float4x4 modelView = viewMatrix * modelMatrix;
    uniforms.modelViewMatrix = modelView;
    
    simd::float4x4 modelViewProj = projectionMatrix * modelView;
    uniforms.modelViewProjectionMatrix = modelViewProj;
    
    simd::float3x3 normalMatrix = { modelView.columns[0].xyz, modelView.columns[1].xyz, modelView.columns[2].xyz };
    uniforms.normalMatrix = simd::transpose(simd::inverse(normalMatrix));
    
    MetalDataBase->uniformBuffer = [MetalDataBase->device newBufferWithBytes:(void *)&uniforms
                                             length:sizeof(Uniforms)
                                            options:MTLResourceOptionCPUCacheModeDefault];
    

    
    [self redraw];
}

- (void)didMoveToWindow
{
    [self redraw];
    
    
}

- (void)redraw
{
    _currentDrawable = [self.metalLayer nextDrawable];
    
    if( !_currentDrawable ) {
        NSLog(@"Failed to get a drawable!");
        MetalDataBase->passDescriptor = nil;
        return;
    }
    else {
        [self setupRenderPassDescriptorForTexture: _currentDrawable.texture];
    }


    // Command buffer.
    MetalDataBase->commandBuffer = [MetalDataBase->commandQueue commandBuffer];
    // Command encoder.
    MetalDataBase->commandEncoder = [MetalDataBase->commandBuffer renderCommandEncoderWithDescriptor:MetalDataBase->passDescriptor];
    
    
    
    [MetalDataBase->commandEncoder setRenderPipelineState:MetalDataBase->pipeline];
    [MetalDataBase->commandEncoder setCullMode: MTLCullModeNone];
    [MetalDataBase->commandEncoder setDepthStencilState: _compositionDepthState];
    [MetalDataBase->commandEncoder setStencilReferenceValue: 128];
    
    coolBadass->Render();
//
//    
    [MetalDataBase->commandEncoder setRenderPipelineState: MetalDataBase->compositionPipeline];
    [MetalDataBase->commandEncoder setCullMode: MTLCullModeNone];
    [MetalDataBase->commandEncoder setStencilReferenceValue: 128];
    [MetalDataBase->commandEncoder setDepthStencilState: _compositionDepthState];

    [MetalDataBase->commandEncoder setFragmentTexture:_currentDrawable.texture atIndex:0];
    [self drawQuadWithRect:MetalDataBase->commandEncoder offset:0 useTexture:false];
    

    

    // End frame - --------------------------
    [MetalDataBase->commandEncoder endEncoding];
    
    [MetalDataBase->commandBuffer presentDrawable:_currentDrawable];
    [MetalDataBase->commandBuffer commit];
    
    
    
    MetalDataBase->commandBuffer = nil;
}





@end



