//
//  Multiplayer.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/24/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef Multiplayer_cpp
#define Multiplayer_cpp

#include "../../Platform/Shared/SystemShared.h"
#include "../Streams/BitMessage.h"
#include "../../Math/GameMath.h"
#include "../Streams/DeltaBitMessage.h"
#include "../Physics/PlayerPhysics.h"
#include "../Utilities/List.h"

namespace Neko {
    
    /**
     *  Entity types.
     */
    enum ServerEntityType
    {
        ENTITY_CLIENT   = 0,  // Client entity.
        ENTITY_CLIENTSELF,
        ENTITY_DUMMY        // His side.
    };
    
    /**
     *  Client command buttons.
     */
    enum ClientButtons
    {
        Forward     = 1,
        Backward    = 2,
        Left        = 4,
        Right       = 8
    };
    
    /**
     *  User command.
     */
    struct UserCommand
    {
        /**
         *  Server time when user command was inited.
         */
        uint32_t ServerTime;
        
        /**
         *  Command buttons.
         */
        int32_t Buttons;    // int16??
        
        /**
         *  Write commands.
         */
        void Serialize( SBitMessage * message, UserCommand * old );
        
        /**
         *  Read commands.
         */
        void Deserialize( SBitMessage * message, UserCommand * old );
        
        /**
         *  Check if button is pressed.
         */
        inline bool IsPressed( ClientButtons button ) const
        {
            return (Buttons & button) == button;
        }
    };

    /**
     *  A server entity.
     */
    struct Entity : public CPhysicsObject
    {
        /**
         *  Entity spawn key.
         */
        int32_t SpawnKey;
        
        /**
         *  Entity type.
         */
        int32_t TypeCode;

        /**
         *  Initialize entity.
         */
        virtual void Initialize() = 0;
        
        /**
         *  Spawn entity.
         */
        virtual void Spawn() = 0;
        
        /**
         *  Entity update.
         */
        virtual void Process() = 0;

        /**
         *  Update entity in the server world.
         */
        virtual void Update( float msec )   = 0;
        
        SLink   m_Link;
        
        Entity()
        {
            TypeCode = 0;
            SpawnKey = -1;
        }
        
        virtual ~Entity()
        {
            
        };
        
        /**
         *  Write entity information into the message.
         */
        virtual void Deserialize( DeltaBitMessage * message ) ;
        
        /**
         *  Read entity information.
         */
        virtual void Serialize( DeltaBitMessage * message );
        
        /**
         *  Interpolate entity.
         */
        virtual void InterpolateFrom( Entity * ent, float time );
        
        /**
         *  A method to create entity.
         */
        static Entity * Create( int typeCode );
        
        /**
         *  Clone entity with properties.
         */
        virtual void Clone( Entity * obj ) = 0;
    };

}

#endif /* Multiplayer_cpp */
