//
//  ClientWorld.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/30/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef ClientWorld_hpp
#define ClientWorld_hpp

#include "../../Platform/Shared/SystemShared.h"
#include "../Core.h"    // Non-copyable define.

namespace Neko {
    
    /**
     *  Client side world.
     */
    class ClientWorld
    {
        NEKO_NONCOPYABLE( ClientWorld );
    public:
        
        /**
         *  Default constructor.
         */
        ClientWorld();
        
        /**
         *  Default destructor
         */
        ~ClientWorld();
        
        /**
         *  Initialize client world preferences.
         */
        void                Initialize();
        
        /**
         *  Create a local world.
         */
        void                CreateWorld( const uint32_t & seed );
        
        uint32_t    m_worldSeed[3];
    private:
        
    };
}

#endif /* ClientWorld_hpp */
