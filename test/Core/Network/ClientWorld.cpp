//
//  ClientWorld.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/30/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "ClientWorld.h"
#include "../../World/BeautifulEnvironment.h"

namespace Neko {
    
    /**
     *  Default constructor.
     */
    ClientWorld::ClientWorld() {
        
    }
    
    /**
     *  Default destructor.
     */
    ClientWorld::~ClientWorld() {
        
    }
    
    /**
     *  Initialize client world properties.
     */
    void ClientWorld::Initialize()
    {
        g_Core->p_Console->Print( LOG_INFO, "Initializing client world..\n" );
    }
    
    /**
     *  Create a world.
     *
     *  @note - This MUST be done on the renderer thread, that's why we set it as the renderer task ( look down ).
     */
    static void CreateRendererWorld( void * arg )
    {
        //  Random seed number.
        const uint64_t randSeed = g_Core->p_ClientWorld->m_worldSeed[0];   //1600106500; // 881282866
        const uint64_t randSeed2 = randSeed;//rand();
        const uint64_t randSeed3 = randSeed;//rand();
        
        g_Core->p_Console->Print( LOG_INFO, "Random world seed: %ld/%ld/%ld\n", randSeed, randSeed2, randSeed3 );
        
        //  Noise parameters.
        NoisePerlinParams largeParams;
        largeParams.octave = 10;
        largeParams.freq = 2;
        largeParams.amp = 2;
        largeParams.seed = randSeed;
        
        NoisePerlinParams mediumParams;
        mediumParams.octave = 3;
        mediumParams.freq = 2;
        mediumParams.amp = 1;
        mediumParams.seed = randSeed2;
        
        NoisePerlinParams typeParams;
        typeParams.octave = 2;
        typeParams.freq = 2;
        typeParams.amp = 1;
        typeParams.seed = randSeed3;
        
        g_pbEnv->Makeup( largeParams, mediumParams, typeParams );
  
    }
    
    /**
     *  Create a client world.
     */
    void ClientWorld::CreateWorld(const uint32_t &seed)
    {
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Creating the client world..\n" );
        
        m_worldSeed[0] = seed;
        
        // Create world on the renderer thread only.
        g_mainRenderer->Perform( CreateRendererWorld, this );
    }
    
}