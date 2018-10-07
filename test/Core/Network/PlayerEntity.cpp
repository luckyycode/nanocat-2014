//
//  PlayerEntity.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/26/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "PlayerEntity.h"
#include "MultiplayerServer.h"  // Time property.

namespace Neko {
    
    
    /**
     *  Update an entity in the server world.
     */
    void PlayerEntity::Update( float msec )
    {
        // Calculate entity velocity.
        g_Core->p_Server->p_ServerWorld->CalculateVelocity( this, msec, vCurDirection );
        
        // Update the entity base.
        Entity::Update( msec );
        
        // Check if object isn't moving anymore and decrease the velocity.
        g_Core->p_Server->p_ServerWorld->EndObjectVelocity( this, msec, vCurDirection );
    }
    
    const static int32_t MOVE_MSEC = 200;
    
    /**
     *  Player entity movement.
     */
    void PlayerEntity::Process()
    {
        UserCommand * command = Command;
        int32_t     msec;
        
        if( command->ServerTime < (g_Core->p_Server->Time - 1000) ) {
            command->ServerTime = g_Core->p_Server->Time - 1000;
        } else if( command->ServerTime > (g_Core->p_Server->Time + MOVE_MSEC) ) {
            command->ServerTime = g_Core->p_Server->Time + MOVE_MSEC;
        }
        
        msec = command->ServerTime - LastCommandTime;
        
        // Command wasn't renewed.
        if( msec < 1 ) {
            return;
        } else if( msec > MOVE_MSEC ) {
            msec = MOVE_MSEC;
        }
        
        // Perform entity move.
        g_Core->p_Server->p_ServerWorld->PerformEntityMove( this, command );
    }
    
    /**
     *  Read player entity information.
     */
    void PlayerEntity::Deserialize( DeltaBitMessage * message )
    {
//        printf(" the hell, man!\n" );
        LastCommandTime = message->ReadInt16();
        
        // Calls an entity base.
        Entity::Deserialize( message );
    }
    
    /**
     *  Write player entity information.
     */
    void PlayerEntity::Serialize( DeltaBitMessage * message )
    {
        message->WriteInt16( LastCommandTime );
        
        // Calls an entity base too.
        Entity::Serialize( message );
    }
    
    /**
     *  Interpolate entities.
     *
     *  @param entityData Entity state bases.
     *  @param entityNum  Total entities.
     *  @param left       Previous entities.
     *  @param right      New entities.
     *  @param time       Time property.
     *
     *  @return Snapshot with the new interpolated entity data.
     */
    Snapshot * Snapshot::Interpolate( Snapshot * curSnap, Entity** entityData, uint32_t entityNum, Snapshot* left, Snapshot* right, uint32_t time )
    {
        if( left == NEKO_NULL ) {
            // Probably wasn't inited yet.
            return right;
        }
        
        float       lerp;
        uint32_t    i;
        
        Snapshot * snap = curSnap;//new Snapshot(entityData, entityNum);
        
        snap->Sequence = right->Sequence;
        snap->ServerTime = time;
        
        snap->EntityNum = entityNum;
        snap->Entities = right->Entities ;    // Copy entities.
        
        lerp = nkMath::Clamp( (float)(time - left->ClientTime) / (float)(right->ClientTime - left->ClientTime), 0.0f, 1.0f);
        
        for( i = 0; i < entityNum; ++i )    // TODO: probably could split some
        {
            // Check if original and interpolating entity exists.
            if( left->Entities[i] == NEKO_NULL || right->Entities[i] == NEKO_NULL ) {
                continue;
            }
            
            // Copy entity.
            snap->Entities[i]->Clone( snap->Entities[i] );
            
            // Interpolate entities.
            snap->Entities[i]->InterpolateFrom( left->Entities[i], lerp );
        }
        
        return snap;
    }
}