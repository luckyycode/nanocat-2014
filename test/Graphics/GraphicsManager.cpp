//
//  GraphicsManager.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 10/12/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "GraphicsManager.h"

#ifdef USES_METAL
    #include "../Platform/Apple/OSX/MetalExternal.h"
#endif

namespace Neko {
    
    /**
     *  Constructor.
     */
    GraphicsManager::GraphicsManager()
    {
        m_graphicsInterface = NEKO_NULL;
    }
    
    /**
     *  Destructor.
     */
    GraphicsManager::~GraphicsManager()
    {
        
    }
    
    /**
     *  Initialize graphics manager.
     */
    void GraphicsManager::Initialize( const int32_t interface )
    {
        g_Core->p_Console->Print( LOG_INFO, "Initializing graphics interface..\n" );
        
        switch( interface )
        {
            case INTERFACE_OPENGL:
                
                m_graphicsInterface = new COpenGLAPI();
                
                break;
            
            case INTERFACE_DIRECTX:
                
                // TODO
                
                break;
                
#ifdef USES_METAL
            case INTERFACE_METAL:
                
                // Initialize Metal instance to access it's API from here.
                // Use 'metalInstance' to use Metal API from C++/C code.
                m_graphicsInterface = new Metal();
                
                break;
#endif
                
            default:
                CCore::Assert( "GraphicsManager: Undefined type in Init" );
                break;
        }
        
        m_graphicsInterface->Initialize();
    }
    
    GraphicsManager * g_pGraphicsManager;
}