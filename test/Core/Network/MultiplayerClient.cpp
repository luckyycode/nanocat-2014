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
//         _j__j__j_)     `-------/ /__j__j__j_
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
//  MultiplayerClient.cpp
//  Multiplayer: Client side..
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "../../Platform/Shared/System.h"
#include "../../Core/Core.h"
#include "../../Core/Console/ConsoleCommand.h"
#include "../../Core/Console/Console.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Core/String/StringHelper.h"
#include "../../Math/GameMath.h"
#include "../../Core/Streams/Streams.h"
#include "../Player/Camera/Camera.h"
#include "../Player/Input/Input.h" // Key states.
#include "Multiplayer.h"
#include "MultiplayerServer.h"
#include "MultiplayerClient.h"
#include "Network.h"
#include "ServerWorld.h"    // Entity movement.
#include "ClientWorld.h"    // World creationism.


namespace Neko {
    
    /*   User name.   */
    SConsoleVar     * NameVar = 0;
    
    /*    Server timeout. ( In seconds )   */
    SConsoleVar      * Client_ServerTimeout = 0;
    
    /*   Maximum packets to be send per frame.   */
    SConsoleVar      * Client_MaximumPackets = 0;
    
    /*  Snapshot delay.   */
    SConsoleVar       * Client_SnapDelay = 0;
    
    /**
     *  Disconnect from current server.
     */
    void ncClient::Disconnect( void )
    {
        // Forced disconnect.
        g_Core->p_Client->DisconnectForced( "User disconnect.", true );
    }
    
    static const uint64_t kClientMemorySize = Megabyte( 16 );
    
    /**
     *  Initialize client stuff.
     */
    void ncClient::Initialize( INekoAllocator * allocator )
    {
        int32_t i;
        
        m_bInitialized = false;
        
        m_curSnapshot = NEKO_NULL;
        m_snapHistory = NEKO_NULL;
        m_snapshotEntityData = NEKO_NULL;
        
        Response = 0;
        ClientNum = 0;
        
        TimeBase = 0;
        Frametime = 0;
        Time = 0;
        
        LastServerMessage = 0;
        LastMessageReceivedAt = 0;
        LastExecutedReliableAt = 0;
        LastReliableMessage = 0;
        LastConnectPacketTime = 0;
        
        ReliableAcknowledged = 0;
        ReliableSequence = 0;
        
        TimeSinceLastPacket = 0;
        m_curSnapshot = NEKO_NULL;
        
        
        pAllocatorHandle = allocator;
        pAllocator = NekoAllocator::newStackAllocator( kClientMemorySize, *pAllocatorHandle );
        
        g_Core->p_Console->Print( LOG_INFO, "Client initializing...\n" );
        
        // Register console variables.
        Client_MaximumPackets = g_Core->p_Console->RegisterCVar( ECvarGroup::Client, "iClientMaxpackets", "Maximum packets to be send per frame.", 35, CVFlag::None, ECvarType::Int );
        Client_ServerTimeout = g_Core->p_Console->RegisterCVar( ECvarGroup::Client, "iClientTimeout", "Client server timeout.", 30, CVFlag::None, ECvarType::Int );
        Client_SnapDelay = g_Core->p_Console->RegisterCVar( ECvarGroup::Client, "iClientSnapDelay", "Snapshot interpolation delay.", 50, CVFlag::None, ECvarType::Int );
        NameVar = g_Core->p_Console->RegisterCVar(  ECvarGroup::Client, "sClientName", "Client name.", "", CVFlag::None, ECvarType::String );
        
        // Set default name.
        NameVar->Set<const char*>( g_Core->p_System->GetCurrentUsername() );
        
        g_Core->p_Console->LoadIni( "neko" );
        
        // Default client state.
        State = CLIENT_IDLE;
        
        NekoCString::Copy( Name, NameVar->Get<const char*>() );
        
        // Create new server property.
        memset( &CurrentServer, 0x00, sizeof(SNetdata) );
        
        // Reset values.
        ReliableAcknowledged = 0;
        ReliableSequence = 0;
        ServerId = 0;
        PreviousFrameTime = 0;
        
        // Is client a host?
        m_isHost = false;
        
        // User commands data.
        userCommandBackup = (UserCommand *)pAllocator->Alloc( sizeof(UserCommand) * USER_CMD_BACKUP );
        userCommandNum = 0;
        
        // User command queue.
        userCommandQueue = (UserCommand *)pAllocator->Alloc( sizeof(UserCommand) * USER_CMD_QUEUE_SIZE );
        userCommandQueueCount = 0;
        
        // Server entity data.
        m_entityStates = (SBitMessage **)pAllocator->Alloc( sizeof(SBitMessage*) * MAX_SERVER_ENTITIES );
        
        for( i = 0; i < MAX_SERVER_ENTITIES; ++i ) {
            m_entityStates[i] = NEKO_NULL;
        }
        
        // Snapshot history.
        m_snapHistorySize = 32;
        m_snapHistory = (Snapshot **)pAllocator->Alloc( sizeof(Snapshot*) * m_snapHistorySize );
        
        for( int i = 0; i < m_snapHistorySize; ++i ) {
            m_snapHistory[i] = NEKO_NULL;
        }
        
        // Snapshot entities.
        m_entityData = (Entity **)pAllocator->Alloc( sizeof(Entity *) * MAX_SERVER_ENTITIES );    // Client snaphot entity data.
        m_snapshotEntityData = (Entity **)pAllocator->Alloc( sizeof(Entity *) * MAX_SERVER_ENTITIES );    // Snapshot interpolation entities.
        m_snapToInterpolate = (Snapshot *)pAllocator->Alloc( sizeof(Snapshot) );
        m_snapToInterpolate->SetData( m_entityData, MAX_SERVER_ENTITIES );
        
        m_bHasWorld = false;    // Sets to 'true' when initialization state got recieved ( and when world got created ).
        m_bInitialized = true;
        
        m_localSnapshot = NEKO_NULL;
        
        g_Core->p_ClientWorld = (ClientWorld *)pAllocator->Alloc( sizeof(ClientWorld) );
        g_Core->p_ClientWorld->Initialize();
    }
    
    /**
     *  Add ack command.
     *
     *  @param command - A string command to send.
     */
    void ncClient::SendCommand( const char * command )
    {
        int32_t unacknowledged;
        
        unacknowledged = ReliableSequence - ReliableAcknowledged;
        
        // Too many commands aren't acknowledged.
        if( unacknowledged > MAX_RELIABLE_MESSAGES ) {
            g_Core->p_Console->Error( ERR_FATAL, "Client command overflow." );
            return;
        }
        
        ++ReliableSequence;
        ReliableCommands[ReliableSequence & (MAX_RELIABLE_MESSAGES - 1)] = (char*)command;
    }
    
    /**
     *  Check client timeout.
     */
    void ncClient::CheckForTimeout( void )
    {
        if( State == CLIENT_IDLE ) {
            return;
        }
        
        // Check for timeout.
        if( Time - LastMessageReceivedAt > ( Client_ServerTimeout->Get<int>() * 10000 /* ms */ ) ) {
            g_Core->p_Console->Print( LOG_INFO, "Connection timed out.\n" );
            
            // Disconnect from server and clear info.
            Disconnect();
        }
    }
    
    /**
     *  Connect to the server.
     *
     *  @param ipaddr - IP address string.
     *  @param port - Address port.
     *
     *  @note ipaddr can be 'localhost'.
     */
    void ncClient::Connect( const char * ipaddr, const uint32_t port )
    {
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Client::Connect(): connecting to %s:%i\n", ipaddr, port );
        
        
        // Clear current server properties.
        memset( &CurrentServer, 0x00, sizeof( SNetdata ) );
        
        // Check if it's a local client.
        if( !strcmp( "localhost", ipaddr ) && port == Network_Port->Get<int>() ) {
            g_Core->p_Console->Print( LOG_INFO, "Client::Connect(): address type is loopback.\n" );
            g_Core->p_Server->m_bRunning = true ;
            CurrentServer.addressType = ENetAddressType::Loopback;
            
            this->m_isHost = true;
        } else {
            g_Core->p_Server->m_bRunning = false ;
            CurrentServer.addressType = ENetAddressType::IP;
        }
        
        // Resolve the network address.
        // If it changes then re-assign it.
        if( !CNetwork::Resolve( &CurrentServer, ipaddr ) ) {
            return;
        }
        
        // Set the server port.
        CurrentServer.Port = htons( port );
        CurrentServer.Address.sin_port = htons( port );
        CurrentServer.Address.sin_family = AF_INET;
        
        // Set the destination IP address. ( Don't use for the local stuff! )
        if( this->m_isHost == false ) {
            CurrentServer.Address.sin_addr.s_addr = inet_addr( ipaddr );
        }
        
        // Create network channel.
        g_Core->p_Network->CreateChannel( ENetworkChannelType::ClientServer, &Channel, &CurrentServer );
        
        // Set client to be active.
        m_bRunning = true ;
        
        // By this we're sending requests to the given server address.
        State = CLIENT_PRECONNECTING;
        
        // Initial parameters.
        LastMessageReceivedAt = Time;
        LastConnectPacketTime = -999999;
    }
    
    /**
     *  Process server frame.
     *
     *  @param message - A bitstream message to read.
     */
    void ncClient::ReadServerFrame( SBitMessage *message )
    {
        int32_t entityNumber;   // Entity indexing.
        int32_t spawnKey;       // Entity spawn key.
        int32_t typeCode;       // Entity type.
        int32_t i;
        int32_t totalSnaps;     // Snapshot count.
        int32_t snapDelay;      // Snapshot delay.
        
        static Snapshot snapshot( m_entityData, MAX_SERVER_ENTITIES );
        SBitMessage    * entityBase = NEKO_NULL;
        
        
        snapshot.Sequence = Channel.sequenceIn;
        snapshot.ServerTime = message->ReadInt32();   // Server time base.
        
        // Total commands acknowledged.
        this->ReliableAcknowledged = message->ReadInt32();
        
        // Check for an overflow.
        //      if ( ReliableAcknowledged < ReliableSequence - 128 ) {
        //           ReliableAcknowledged = ReliableSequence;
        //      }
        
        // Update time.
        this->TimeBase = (snapshot.ServerTime - Time);
        
        entityNumber = message->ReadInt32();
        
        while( entityNumber != MAX_SERVER_ENTITIES )    // Don't use 0xff as a mark here.
        {
            entityBase = m_entityStates[entityNumber];    // Can be NEKO_NULL value.
            DeltaBitMessage deltaMessage( entityBase, message );
            
            spawnKey = deltaMessage.ReadInt16();    // Read the spawn key.
            typeCode = deltaMessage.ReadInt16();    // Read the entity type.
            
            // Set server entities.
            if( snapshot.Entities[entityNumber] == NEKO_NULL ) {
                printf( "Entity: null\n" );
                
                g_Core->p_Console->Print( LOG_INFO, "ReadFrame(): A new snapshot entity to create with properties:\n" );
                g_Core->p_Console->Print( LOG_INFO, "|\tentityId: %i\n", entityNumber );
                g_Core->p_Console->Print( LOG_INFO, "|\tspawnKey: %i\n", spawnKey );
                g_Core->p_Console->Print( LOG_INFO, "|\ttypeCode: %i\n", typeCode );
                
                memset( &snapshot.Entities[entityNumber], 0x00, sizeof(Entity*) );
                
                snapshot.Entities[entityNumber] = Entity::Create( typeCode );
                
                // Never ever use loop breaks here.
            }
            
            // Read the entity information.
            snapshot.Entities[entityNumber]->Deserialize( &deltaMessage );
            // Set the current state.
            m_entityStates[entityNumber] = deltaMessage._newBaseStream;
            // Current entity number.
            entityNumber = message->ReadInt32();
        }
        
        // Get a free space for a new snapshot.
        for( i = (m_snapHistorySize - 2); i >= 0; --i ) {
            m_snapHistory[i + 1] = m_snapHistory[i];
        }
        
        // Set the current snapshot.
        m_snapHistory[0] = &snapshot;
        
        const int32_t delayMsec = Client_SnapDelay->Get<int>();
        totalSnaps = 0;
        
        // Compare a new snapshot with a previous to find the last one.
        for( i = 0; i < m_snapHistorySize; ++i ) {
            if( m_snapHistory[i] != NEKO_NULL ) {
                if( m_snapHistory[i]->ServerTime < (m_snapHistory[0]->ServerTime - delayMsec) ) {
                    totalSnaps = i;
                    break;
                }
            }
        }
        
        // Snapshot delay based on snapshot history count.
        snapDelay = nkMath::Max( 0, nkMath::Min( (m_snapHistorySize - 2), totalSnaps ));
        
        m_pLastSnapshot = m_snapHistory[snapDelay + 1];
        m_curSnapshot = m_snapHistory[snapDelay];
        
        // Set the new snapshot as a current one.
        if( m_curSnapshot == NEKO_NULL ) {
            m_curSnapshot = m_snapHistory[0];
        }
        
        m_curSnapshot->ClientTime = Time;
    }
    
    /**
     *  Process server ack commands.
     */
    void ncClient::ReadServerCommands( SBitMessage * msg )
    {
        int32_t commandSequence;
        const char * command;
        
        commandSequence = msg->ReadInt32();
        command = msg->ReadString();
        
        //  Check if we have right order.
        if( commandSequence > LastExecutedReliableAt ) { // Check packet order.
            
            /***
             * Gooosh, a string tokenizer!.
             **/
            NekoCString::TokenizeString( command );
            
            
            // Parse commands now.
            if( !strcmp( NekoCString::lastCommandArguments[0], "disconnect" ) ) {
                DisconnectForced(NekoCString::lastCommandArguments[1], false);
            } else if( !strcmp( NekoCString::lastCommandArguments[0], "print") ) {
                g_Core->p_Console->Print( LOG_INFO, "Server: %s\n", NekoCString::lastCommandArguments[1] );
            }
            
            LastExecutedReliableAt = commandSequence;
            LastReliableMessage = commandSequence;
        }
    }
    
    /**
     *  Send command packet to server.
     */
    void ncClient::SendCommands( void )
    {
        
//        if( !Client_Running.Get<bool>() )
//            return;
        
        int32_t i;
        UserCommand * command = NEKO_NULL, * lastCommand = NEKO_NULL;
        
        // Create a message.
        Byte packetBuf[BASE_PACKET_SIZE];
        SBitMessage msg( packetBuf, BASE_PACKET_SIZE );
        
        // Write header.
        msg.WriteInt16( this->LastServerMessage );
        msg.WriteInt16( this->LastReliableMessage );
        msg.WriteInt16( this->ServerId );
        
        // Write commands with ids.
        for( i = ReliableAcknowledged + 1; i <= ReliableSequence; ++i ) {
            msg.WriteByte( COMMANDHEADER_ACK );
            msg.WriteInt16( i );
            msg.WriteString( ReliableCommands[i & MAX_RELIABLE_MESSAGES - 1] );
        }
        
        
        // Write command byte.
        msg.WriteByte( COMMANDHEADER_MOVE );
        
        // Command count.
        msg.WriteByte( (Byte)userCommandQueueCount );
        
        // Send all queued user commands.
        while( (command = GetCommand()) != NEKO_NULL ) {
            command->Serialize( &msg, lastCommand );
            
            lastCommand = command;
        }
        
        // TODO: fix me
        msg.WriteFloat( g_Core->p_Camera->vLook.x );
        msg.WriteFloat( g_Core->p_Camera->vLook.y );
        msg.WriteFloat( g_Core->p_Camera->vLook.z );
        
        
        msg.WriteFloat( g_Core->p_Camera->m_fGamepadMoveAxisSensitivity.x );
        msg.WriteFloat( g_Core->p_Camera->m_fGamepadMoveAxisSensitivity.y );
        
        // Write final command byte.
        msg.WriteByte( 0xFF );
        
        
        // Send now.
        g_Core->p_Network->SendMessageChannel( &Channel, &msg );
        
    }
    
    /**
     *  Send queue commands.
     */
    void ncClient::CheckForCommands( const uint32_t time )
    {
        TimeSinceLastPacket += time;
        
        // Packet sending rate.
        if( TimeSinceLastPacket > ( 1000 / Client_MaximumPackets->Get<int>() ) )
        {
            SendCommands();
            TimeSinceLastPacket = 0;
        }
    }
    
    /**
     *  Set current snapshot.
     */
    void ncClient::SetLocalSnapshot()
    {
        if( m_snapHistory[0] == NEKO_NULL ) {
            return;
        }
        
        if( m_curSnapshot != NEKO_NULL ) {
            m_localSnapshot = Snapshot::Interpolate( m_snapToInterpolate, m_snapshotEntityData, MAX_SERVER_ENTITIES,
                                                    m_pLastSnapshot, m_curSnapshot,
                                                    (uint32_t)(Time - Client_SnapDelay->Get<int>()) );
        }
    }
    
    //#define DEBUG_PREDICTION    // Player entity prediction.
    
    /**
     *  Predict player.
     */
    void ncClient::PredictEntity()
    {
        if( m_localSnapshot == NEKO_NULL || m_localSnapshot->GetLocalPlayer() == NEKO_NULL ) {
            printf( "PredictEntity(): null snapshot or no local entity\n" );
            return;
        }
        
        int32_t i;
        int32_t currentCommand;
        
        UserCommand * command;  // Command at index.
        Snapshot * predictSnap;
        PlayerEntity * localPlayer;
        
#   if defined( DEBUG_PREDICTION )
        PlayerEntity * originalPlayer;
        originalPlayer  = (PlayerEntity*)m_localSnapshot->GetLocalPlayer()->Clone();
#   endif
        
        predictSnap = m_curSnapshot;
        
        if( predictSnap == NEKO_NULL || predictSnap->GetLocalPlayer() == NEKO_NULL ) {
            predictSnap = m_pLastSnapshot;
            
            if( predictSnap == NEKO_NULL || predictSnap->GetLocalPlayer() == NEKO_NULL ) {
                return;
            }
        }
        
        localPlayer = (PlayerEntity*)predictSnap->GetLocalPlayer();//->Clone();
        m_localSnapshot->SetLocalPlayer( (PlayerEntity*)localPlayer );
        
        currentCommand = userCommandNum;
        
        for( i = (currentCommand - 63); i < currentCommand; ++i ) {
            command = GetCommand( i );
            
            if( command == NEKO_NULL ) {
                continue;
            }
            
            if( command->ServerTime <= predictSnap->GetLocalPlayer()->LastCommandTime ) {
                continue;
            }
            
#   if defined( DEBUG_PREDICTION )
            if( originalPlayer->LastCommandTime == m_localSnapshot->GetLocalPlayer()->LastCommandTime )
            {
                if( originalPlayer->Position.x != m_localSnapshot->GetLocalPlayer()->Position.x &&
                   originalPlayer->Position.y != m_localSnapshot->GetLocalPlayer()->Position.y &&
                   originalPlayer->Position.z != m_localSnapshot->GetLocalPlayer()->Position.z)
                {
                    printf( "Client prediction error (position) \n" );
                }
                
            }
#   endif
            
            // Player movement.
            g_Core->p_Server->p_ServerWorld->PerformEntityMove( (PlayerEntity*)m_localSnapshot->GetLocalPlayer(), command );
            
        }
        
        PreviousFrameTime = Time;
    }
    
    /**
     *  Parse server commands.
     */
    void ncClient::ParseCommands( SNetdata *from, SBitMessage * buffer )
    {
//        if( !Client_Running.Get<bool>() )
//            return;
        
        buffer->BeginReading(); // Begin parsing.
        
        LastMessageReceivedAt = Time;
        LastServerMessage = buffer->ReadInt32();
        
        int32_t cmd;    // Last command recieved.
        do {    // Read until 0xFF byte ( command packet end ).
            
            cmd = buffer->ReadByte();
            
            switch(cmd) {
                    /*  Server entity update. */
                case COMMANDHEADER_SERVERENTITY:
                    ReadServerFrame( buffer );
                    break;
                    
                    /*  Process serverclient commands. */
                case COMMANDHEADER_SERVERACK:
                    ReadServerCommands( buffer );
                    break;
                    
                    /*  Serialize server information to a client. */
                case COMMANDHEADER_INITSTATE:
                    ReadClientInit( buffer );
                    break;
            }
            
        }
        while( cmd != 0xFF /* means the packet end or corrupted */ );
    }
    
    /**
     *  Client connectionless packets.
     */
    void ncClient::Connectionless( SNetdata *from,  Byte * data )
    {
        // It's guaranteed that data is not a NEKO_NULL ( unless this method is not called from somewhere else ).
        
        data += 4;  // Skip first four bytes ( 0xFFFFFFFF ).
        
        
        // Tokenize a message.
        NekoCString::TokenizeString( (char*)data );
        
        
        
        // Parse commands.
        if( !strcmp( NekoCString::lastCommandArguments[0], "requestResponse" ) ) {
            // Request accepted, try to connect.
            Response = atoi(NekoCString::lastCommandArguments[1]);
            State = CLIENT_CONNECTING;
            LastConnectPacketTime = -99999;
            //CurrentServer = from;
        } else if( !strcmp( NekoCString::lastCommandArguments[0], "print" ) ) {
            // 'print' command.
            // Server sends messages like SRV_HELLO so make a parser for it.
            g_Core->p_Console->Print( LOG_INFO, "%s\n", NekoCString::lastCommandArguments[1] );
        } else if( !strcmp( NekoCString::lastCommandArguments[0], "connectResponse" ) ) {
            // Connect request, successfuly connected.
            g_Core->p_Console->Print( LOG_INFO, "Connected to the server.\n" );
            
            ClientNum = atoi( NekoCString::lastCommandArguments[1] );
            g_Core->p_Console->Print( LOG_INFO, "Server client number: %i\n", ClientNum );
            State = CLIENT_CONNECTED;
            
            SendCommand( "say \"wtf\"" );
        }
    }
    
    /**
     *  Parse all incoming information from server.
     */
    void ncClient::Process( SNetdata *from, Byte *buffer )
    {
        //        if( !Client_Running.Get<bool>() )
        //            return;
        // Parse connectionless messages.
        if( strlen((char*)buffer) >= 4 && *(int*)buffer == 0xFFFFFFFF /* -1 */ ) {
            Connectionless( from, buffer );
            return;
        }
        
        //        if( from->SockAddress != CurrentServer->SockAddress )
        //            return;
        
        SBitMessage s( (Byte*)buffer, MAX_MESSAGE_SIZE, true );
        ParseCommands( from, &s );
    }
    
    /**
     *  Process the initializing state.
     */
    void ncClient::ReadClientInit( SBitMessage *msg )
    {
        this->ServerId = msg->ReadInt32();
        
        g_Core->p_Console->Print( LOG_INFO, "ReadClientInit(): Recieved server stateId: %i\n", this->ServerId );
        g_Core->p_ClientWorld->CreateWorld( (uint32_t)msg->ReadLong() );
        
        m_bHasWorld = true;
    }
    
    /**
     *  Works like disconnect.
     */
    void ncClient::Reconnect( void )
    {
        // TODO
    }
    
    /**
     *  Disconnect the client from current server ( also clear server info ).
     */
    void ncClient::DisconnectForced( const char * msg, bool forced )
    {
        //        if( !Client_Running.Get<bool>() )
        //            return;
        
        if( State == CLIENT_IDLE ) {
            // Nothing to do.
            return;
        }
        
        SendCommand( "disconnect ");
        SendCommands();
        SendCommand( "disconnect" );
        SendCommands();
        
        //        g_mainRenderer->RemoveWorld( "Client disconnect." ); // Remove current world.
        
        State = CLIENT_IDLE;  // Set our state to IDLE.
        
        // Disable client side.
        m_bRunning = false ;
        
        memset( &CurrentServer, 0x00, sizeof(SNetdata) );
        
        //        ReliableCommands = new char*[16894];
        memset( ReliableCommands, 0, sizeof( ReliableCommands ) );
        
        g_Core->p_Console->Print( LOG_INFO, "Disconnected from server.\n" );
        g_Core->p_Console->Print( LOG_INFO, "\"%s\"\n", msg );
    }
    
    /**
     *  Check connect command.
     */
    void ncClient::CheckForConnect( void )
    {
        
        /* Do not check connect packet while in game or idle. */
        if ( State != CLIENT_CONNECTING && State != CLIENT_PRECONNECTING ) {
            return;
        }
        
        switch( State )
        {
                /*  Request response.  */
            case CLIENT_PRECONNECTING:
                
                if ( Time - LastConnectPacketTime > RESPONSE_REPEAT ) {
                    g_Core->p_Network->PrintOutOfBand( ENetworkChannelType::ClientServer, &CurrentServer, "getresponse" );
                    LastConnectPacketTime = Time;
                    
                    g_Core->p_Console->Print( LOG_INFO, "Awaiting server response..\n" );
                }
                
                break;
                
                /*  Request connect response.  */
            case CLIENT_CONNECTING:
                
                if ( Time - LastConnectPacketTime > RESPONSE_REPEAT ) {
                    g_Core->p_Network->PrintOutOfBand( ENetworkChannelType::ClientServer, &CurrentServer, "connect %i \"%s\" %s", Response, NameVar->Get<const char*>(), NEKO_BUILD_VERSION );
                    LastConnectPacketTime = Time;
                    
                    g_Core->p_Console->Print( LOG_INFO, "Awaiting connect response..\n" );
                }
                
                break;
                
            default:
                g_Core->p_Console->Error( ERR_FATAL, "Unexpected client state in CheckCommand\n" );
                // TODO: drop connnection
                State = ClientState_t::CLIENT_IDLE;
                break;
        }
    }
    
    /**
     *  Get user command.
     */
    UserCommand* ncClient::GetCommand()
    {
        // No available commands.
        if( userCommandQueueCount < 1 ) {
            return NEKO_NULL;
        }
        
        --userCommandQueueCount;
        return &userCommandQueue[userCommandQueueCount];
    }
    
    /**
     *  Get user command at index.
     */
    UserCommand * ncClient::GetCommand( int32_t index )
    {
        if( index > userCommandNum ) {
            g_Core->p_Console->Print( LOG_WARN, "Client::GetCommand() - exceeded maximum command index\n" );
            return NEKO_NULL;
        }
        
        if( index <= (userCommandNum - USER_CMD_BACKUP) ) {
            return NEKO_NULL;
        }
        
        return &userCommandBackup[index & (USER_CMD_BACKUP - 1)];
    }
    
    /**
     *  Create client command.
     */
    void ncClient::CreateUserCommand()
    {
        UserCommand cmd;
        memset( &cmd, 0x00, sizeof( UserCommand ) );    // Clear previous data if exists.
        
        cmd.ServerTime = (uint32_t)(g_Core->GetTime() + TimeBase);
        
        // TODO: organized look
        if( g_Core->p_Input->m_bMovingForward ) {
            cmd.Buttons |= ClientButtons::Forward;
        }
        
        if( g_Core->p_Input->m_bMovingBackward ) {
            cmd.Buttons |= ClientButtons::Backward;
        }
        
        if( g_Core->p_Input->m_bMovingLeft ) {
            cmd.Buttons |= ClientButtons::Left;
        }
        
        if( g_Core->p_Input->m_bMovingRight ) {
            cmd.Buttons |= ClientButtons::Right;
        }
        
        userCommandQueue[userCommandQueueCount] = cmd;
        ++userCommandQueueCount;
        
        if( userCommandQueueCount > 512 ) {
            userCommandQueueCount = 0;
        }
        
        ++userCommandNum;
        userCommandBackup[userCommandNum & (USER_CMD_BACKUP - 1)] = cmd;
    }
    
    /**
     *  Client update function.
     */
    void ncClient::Frame( uint32_t msec )
    {
        if( !m_bRunning ) {
            return;
        }
        
        if( !m_bInitialized ) {
            return;
        }
        
        // Process client..
        if( State != CLIENT_IDLE )
        {
            
            // Client time.
            Frametime = msec;
            Time += Frametime;
            
            if( State == CLIENT_CONNECTED )
            {
                // Create client command.
                CreateUserCommand();
                
                // Check incoming commands.
                CheckForCommands( Time );
                
                // Set current snapshot.
                SetLocalSnapshot();
                
                // Client movement.
//                PredictEntity();
                // TODO: local player origin is nan sometimes.
                if( m_localSnapshot != NEKO_NULL && m_localSnapshot->Entities != NEKO_NULL & m_localSnapshot->GetLocalPlayer() != NEKO_NULL ) {
                    g_Core->p_Camera->SetPosition( m_localSnapshot->GetLocalPlayer()->GetObjectPos(), Time );
                }
            }
            
            // Check server timeout.
            CheckForTimeout();
            
            // Check connection command.
            CheckForConnect();
        }
    }
    
    /**
     *  Client instance.
     */
    ncClient::ncClient()
    {

    }
    
    /**
     *  Default destructor.
     */
    ncClient::~ncClient()
    {
        
    }
    
    /**
     *  Shutdown the client.
     */
    void ncClient::Shutdown()
    {
        g_Core->p_Console->Print( LOG_INFO, "Client is shutting down..\n" );
        
        DisconnectForced( "User quit", true );
        
        pAllocator->Dealloc( g_Core->p_ClientWorld );
        
        pAllocator->Dealloc( m_snapToInterpolate );
        
        pAllocator->Dealloc( m_snapshotEntityData );
        pAllocator->Dealloc( m_entityData );
        pAllocator->Dealloc( m_snapHistory );
        pAllocator->Dealloc( m_entityStates );
        
        pAllocator->Dealloc( userCommandQueue ) ;
        pAllocator->Dealloc( userCommandBackup ) ;
        
        NekoAllocator::deleteStackAllocator( (CStackAllocator *)pAllocator, pAllocatorHandle );
    }
    
    /**
     *  "Say" command.
     */
    void ncClient::Say( void )
    {
        
    }
}

