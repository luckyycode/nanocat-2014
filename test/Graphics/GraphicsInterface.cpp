//
//  GraphicsInterface.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 10/12/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "GraphicsInterface.h"
#include "Renderer/Renderer.h"

namespace Neko {
    
    /**
     *  Initialize graphical interface.
     */
    void GraphicsInterface::Initialize()
    {
        // Renderer initialization.
        g_mainRenderer->Initialize( this );
    }
    
    /**
     *  Context resize action.
     */
    void GraphicsInterface::OnResize( const int32_t w, const int32_t h )
    {
        
    }
    
    /**
     *  When graphics interface got created..
     */
    void GraphicsInterface::OnLoad()
    {
        OnResize( Render_Width->Get<int>(), Render_Height->Get<int>() );
    }
    
    /**
     *  Shutdown the graphical interface.
     */
    void GraphicsInterface::Shutdown()
    {
        
    }
}