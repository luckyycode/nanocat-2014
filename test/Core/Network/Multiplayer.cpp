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
//         _j__j__j_\     `-------/ /__j__j__j_
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
//  Multiplayer.cpp
//  Common multiplayer methods and classes.
//
//  Created by Kawaii Neko on 9/24/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "Multiplayer.h"
#include "PlayerEntity.h"

#include "MultiplayerClient.h"

namespace Neko {
    
    /**
     *  Read user commands.
     */
    void UserCommand::Deserialize( SBitMessage * message, UserCommand * old )
    {
        // Read an existing user commands.
        if( old == NEKO_NULL )
        {
            ServerTime = message->ReadInt32();
            Buttons = (ClientButtons)message->ReadInt32();
        }
        else
        {
            // Read user commands.
            if( message->ReadBool() == true )
            {
                ServerTime = old->ServerTime + message->ReadByte(); // uint??
            }
            else
            {
                ServerTime = message->ReadInt32();
            }
            
            Buttons = (ClientButtons)message->ReadDeltaInt16( (int16_t)old->Buttons );
        }
    }
    
    /**
     *  Command delay.
     */
    const static int32_t NEW_COMMAND_PERIOD = 255;
    
    /**
     *  Write user commands.
     */
    void UserCommand::Serialize( SBitMessage * message, UserCommand * old )
    {
        // Create an existing information.
        if( old == NEKO_NULL )
        {
            message->WriteInt16( ServerTime );
            message->WriteInt16( (int16_t)Buttons );
        }
        else
        {
            // Write the new information if changed.
            if( (ServerTime - old->ServerTime) < NEW_COMMAND_PERIOD )
            {
                message->WriteBool( true );
                message->WriteByte( (Byte)(ServerTime - old->ServerTime ) );
            }
            else
            {
                message->WriteBool( false );
                message->WriteInt16( ServerTime );
            }
            
            message->WriteDeltaInt16( (int16_t)Buttons, (int16_t)old->Buttons );
        }
    }
    
    /**
     *  Update an entity object in the server world. WHOA
     *
     *  @note This method can be called by an abstract classes.
     */
    void Entity::Update( float msec )
    {
        // TODO something
    }
    
    /**
     *  Create an entity.
     *
     *  @param typeCode - An entity type to create.
     */
    Entity * Entity::Create( int32_t typeCode )
    {
        switch( typeCode )
        {
            case 0:
                return new PlayerEntity();
                
            default:
                g_Core->p_Console->Print( LOG_INFO, "Entity::Create() - server warning, undefined <typeCode=0x%d> property\n", typeCode );
                
                return NEKO_NULL;
        }
    }
    
    /**
     *  Entity interpolation.
     *
     *  @param from - Current entity.
     *  @param factor - Time property.
     */
    void Entity::InterpolateFrom( Entity * ent, float time )
    {
        if( ent == NEKO_NULL ) {
            // I don't know why I put it here.
            return;
        }
        
        // Interpolate position and rotation.
        m_vRotation = (m_vRotation * time) + (ent->m_vRotation * (1.0f - time) );
        m_vPosition = (m_vPosition * time) + (ent->m_vPosition * (1.0f - time) );
    }
    
    /**
     *  Write entity information.
     *
     *  @note - abstract method!
     */
    void Entity::Serialize( DeltaBitMessage * message )
    {
        // Write position.
        message->WriteFloat( m_vPosition.x );
        message->WriteFloat( m_vPosition.y );
        message->WriteFloat( m_vPosition.z );
        
        // Write rotation.
        message->WriteFloat( m_vRotation.x );
        message->WriteFloat( m_vRotation.y );
        message->WriteFloat( m_vRotation.z );
    }
    
    /**
     *  Read entity information.
     *
     *  @note - abstract method!
     */
    void Entity::Deserialize( DeltaBitMessage * message )
    {
        // Read position.
        m_vPosition = Vec3( message->ReadFloat(), message->ReadFloat(), message->ReadFloat() );
        
        // Read rotation.
        m_vRotation = Vec3( message->ReadFloat(), message->ReadFloat(), message->ReadFloat() );
    }
    
    /**
     *  Get a local entity for a client.
     */
    PlayerEntity * Snapshot::GetLocalPlayer() const {
        return (PlayerEntity*)Entities[g_Core->p_Client->ClientNum];
    }
    
    /**
     *  Set a local entity.
     */
    void Snapshot::SetLocalPlayer( PlayerEntity *entity ) {
        Entities[g_Core->p_Client->ClientNum] = entity;
    }
}