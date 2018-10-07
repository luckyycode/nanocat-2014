//
//  ServerWorld.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/27/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef ServerWorld_hpp
#define ServerWorld_hpp

#include "PlayerEntity.h"
#include "Multiplayer.h"
#include "../Streams/BitMessage.h" // World serialization.

namespace Neko {
    
    const static int32_t ENT_MOVE_MSEC = 66;
    
    struct PlayerEntity;
    
    /**
     *  Server world base.
     */
    class CServerWorld
    {
        NEKO_NONCOPYABLE( CServerWorld );
    public:
        
        CServerWorld() { }
        ~CServerWorld() { }
        
        /**
         *  Initialize server world.
         */
        void Initialize();
        
        /**
         *  Create the server world.
         */
        void CreateWorld();
        
        /**
         *  Generate world properties.
         */
        void PrepareWorld();
        
        /**
         *  Write world information into the message.
         */
        void SerializeWorld( SBitMessage * message );
        
        /**
         *  Perform entity movement by given commands.
         */
        void PerformEntityMove( PlayerEntity * entity, UserCommand * command );
        
        /**
         *  Calculate a velocity vector for the object.
         */
        void CalculateVelocity( CPhysicsObject * pEntity, const float msInSec, const Vec3 & direction );
        
        void EndObjectVelocity( CPhysicsObject * pEntity, const float msInSec, const Vec3 & direction );
        
        /**
         *  Set random entity position in the world.
         */
        void SetRandomEntityPosition( CPhysicsObject * pEntity );
        
        /**
         *  World seed.
         */
        uint32_t        m_worldSeed[3];
        
        CQuadtreeNode  * m_tmpNode;
    };
}

#endif /* ServerWorld_hpp */
