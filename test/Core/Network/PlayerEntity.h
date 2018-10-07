//
//  PlayerEntity.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/26/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef PlayerEntity_hpp
#define PlayerEntity_hpp

#include "Multiplayer.h"
#include "../Streams/DeltaBitMessage.h"

namespace Neko {
    
    /**
     *  Player entity.
     */
    struct PlayerEntity : public Entity
    {
        /**
         *  Current player entity command.
         */
        UserCommand * Command;
        uint32_t LastCommandTime;   // Last command recieved at.
        
        /**
         *  Entity direction.
         */
        Vec3 vUp;
        Vec3 vRight;
        
        Vec2   vMoveSensitivty;    // mostly for controllers
        
        Vec3   vCurDirection;
        
        /**
         *  Create a new player entity.
         */
        PlayerEntity()
        {
            this->Initialize();
        }
        
        ~PlayerEntity()
        {
            
        }
        
        /**
         *  Initialize player entity.
         */
        void Initialize()
        {
            TypeCode = 0;
            SpawnKey = 0;
            LastCommandTime = 0;
            
            SetObjectPos( Vec3( 0.0f ) );
        }
        
        /**
         *  Read entity data.
         */
        void Deserialize( DeltaBitMessage * message ) ;
        
        /**
         *  Write entity data.
         */
        void Serialize( DeltaBitMessage * message );
        
        /**
         *  Spawn an entity.
         */
        void Spawn()
        {
            // Set object properties.
            SetObjectPos( Vec3( 0.0f, 70.0f, 0.0f ) );
            SetRotation( Vec3( -10.5f, -0.5f, -0.5f ) );
            SetVelocity( Vec3( 1.0f ) );
            
            SetRadius( 7.0f );
            SetMass( 1.0f );
            
            // Object flags.
            SetObjectFlag( CPhysicsObject::PHYS_CHARACTER );
            SetObjectPhysFlag( CPhysicsObject::PHYS_APPLYGRAVITY );
            
            const float speed = 1201.0f;
            SetAcceleration( Vec3( speed ) );
            SetDefaultAcceleration( Vec3( speed ) );
            
            m_vConstVelocity = Vec3( 1.0f );
            
            vUp = Vec3( 1.0f, 1.0f, 0.0f );
            vRight = Vec3( 1.0f, 0.0f, 0.0f );
        }
        
        /**
         *  Update an entity in the server world.
         */
        void Update( float msec );
        
        /**
         *  Update player entity.
         */
        void Process();

        void Clone( Entity * obj  )
        {
            static PlayerEntity playerEntity;
            playerEntity.SetObjectPos( GetObjectPos() );
            playerEntity.SetRotation( GetRotation() );
            playerEntity.vRight = vRight;
            playerEntity.vUp = vUp;
            playerEntity.SpawnKey = SpawnKey;

            *obj = playerEntity;
        }
        
//        /**
//         *  Clone player entity.
//         *
//         *  @note   Don't forget to delete.
//         */
//        inline Entity * Clone()
//        {
//            PlayerEntity *playerEntity = new Entity;
//            playerEntity->SetObjectPos( GetObjectPos() );
//            playerEntity->SetRotation( GetRotation() );
//            playerEntity->vRight = vRight;
//            playerEntity->vUp = vUp;
//            playerEntity->SpawnKey = SpawnKey;
//
//            return playerEntity;
//        }
    };
    
    
    /**
     *  Snapshot.
     */
    struct Snapshot
    {
        /**
         *  Snapshot sequence.
         */
        int32_t Sequence;
        
        /**
         *  Server time.
         */
        int32_t ServerTime;
        
        /**
         *  Client time.
         */
        int32_t ClientTime;
        
        /**
         *  Amount of entities.
         */
        uint32_t EntityNum;
        
        /**
         *  Entities in snapshot.
         */
        Entity ** Entities;
        
        Snapshot()
        {
            printf( "snapshot ctor\n" );
            assert( !"Snapshot(): default constructor shouldn't be called ( yet ) !" );
        }
        
        /**
         *  Create a new snapshot.
         *
         *  @param entities     Entity state bases.
         *  @param entityNum    Entity amount.
         */
        Snapshot( Entity ** entities, const uint32_t entityNum ) //: Entities(entities), EntityNum(entityNum)
        {
            SetData(entities, entityNum);
        }
        
        /**
         *  Default destructor.
         */
        ~Snapshot()
        {
            
        }
        
        /**
         *  Get a local player entity.
         */
        PlayerEntity * GetLocalPlayer() const;
        
        /**
         *  Set snapshot data.
         */
        inline void SetData( Entity ** entities, const uint32_t entityNum )
        {
            int32_t i ;
            
            Entities = entities;
            EntityNum = entityNum;
            
            for( i = 0; i < entityNum; ++i ) {
                Entities[i] = NEKO_NULL;
            }
        }
        
        /**
         *  Set a local player entity.
         */
        void SetLocalPlayer( PlayerEntity * entity );
        
        /**
         *  Snapshot entity interpolation.
         */
        static Snapshot * Interpolate( Snapshot * curSnap, Entity** entityData, uint32_t entityNum, Snapshot* left, Snapshot* right, uint32_t time );
    };
}

#endif /* PlayerEntity_hpp */
