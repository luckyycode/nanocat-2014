//
//  GraphicsManager.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 10/12/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef GraphicsManager_hpp
#define GraphicsManager_hpp

#include "../Core/Core.h"
#include "OpenGL/OpenGLBase.h"  // OpenGL support.
#include "GraphicsInterface.h"

namespace Neko {
    
    /// Graphics manager.
    class GraphicsManager
    {
        NEKO_NONCOPYABLE( GraphicsManager );
        
    public:
        
        // Widely used graphic APIs.
        const static int32_t INTERFACE_OPENGL   = 0x1;
        const static int32_t INTERFACE_DIRECTX  = 0x2;
        const static int32_t INTERFACE_METAL    = 0x4;
        
        /**
         *  Constructor.
         */
        GraphicsManager();
        
        /**
         *  Destructor.
         */
        ~GraphicsManager();
        
        
        /**
         *  Initialize graphics manager.
         */
        void                Initialize( const int32_t interface );
        
        /**
         *  Get current graphics interface.
         */
        inline GraphicsInterface *              GetCurrentInterface() const  {       return m_graphicsInterface; }
        
    protected:
    private:
        
        /**
         *  Current graphics API.
         */
        GraphicsInterface   * m_graphicsInterface;
    };
    
    extern GraphicsManager * g_pGraphicsManager;
}

#endif /* GraphicsManager_hpp */
