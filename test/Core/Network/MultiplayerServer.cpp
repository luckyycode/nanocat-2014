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
//  MultiplayerServer.cpp
//  Multiplayer: Game server..
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "../../Graphics/Renderer/Renderer.h"
#include "../../Platform/Shared/System.h"
#include "../../World/BeautifulEnvironment.h"
#include "../Console/Console.h"
#include "../Console/ConsoleCommand.h"
#include "../Core.h"
#include "../Player/Camera/Camera.h"
#include "../Streams/DeltaBitMessage.h"
#include "../String/StringHelper.h"
#include "Multiplayer.h"
#include "MultiplayerClient.h" // Some console variables.
#include "MultiplayerServer.h"
#include "Network.h"
#include "ServerWorld.h"


namespace Neko {
    
    //!  Maximum clients on server.
    SConsoleVar  * Server_Maxclients = 0;

    //!  Server public name.
    SConsoleVar  * Server_Name = 0;

//    //!  Cheats enabled?
//    SConsoleVar  Server_Fun( "server_fun", "Turn on cheats? u lil sneaky ;)", true, CVFlag::NeedsRefresh );

    //!  Server framerate.
    SConsoleVar  * Server_Maxfps = 0;

    //!  Name which is going to be used in chat.
    SConsoleVar  * Server_Sayname = 0;//( "sayname", "Friskies", "Server name to be used in chat.", CVFlag::NeedsRefresh );

    //!  Default bot name.
    SConsoleVar   * Server_Botname = 0;//( "botname", "Whiskas", "Server bot name.", CVFlag::None );

    //!   Is server dedicated?
    SConsoleVar  * Server_Dedicated = 0;

    //!   Inactive client timeout.
    SConsoleVar  * Server_Clienttimeout = 0;

    //!   Status print per x seconds.
    SConsoleVar  * Server_Statusperiod = 0;
    // SConsoleVar server_cutebots( "server", "cutebots", "Smiling and nice bots.", "1", CVFLAG_KID );


    /**
     *  Server client.
     */
    CServerClient::CServerClient( void )
    {
        lastMessageTime = 0;    // Last packet time.
        lastConnectTime = 0;    // Last connection packet time.

        lastReceivedCommand = 0;
        lastExecutedCommand = 0;
        lastAcknowledgedMessageAt = 0;
        lastRenewStateMessage = 0;
        lastMessageReceivedAt = 0;
        
        cachedReliableCommandAt = 0;
        commandSequence = 0;        // Acknowledge sequence.
        commandsAcknowledged = 0;
        
        ghostifiedAt = 0;
        response = 0;
    }

    /**
     *  Initialize client entity.
     *
     *  This is required only when server creates player type of entity.
     */
    void CServerClient::CreateEntity()
    {
        // Initialize entity states.
        EntityState = new SServerClientEntityState*[SERVERCLIENT_BASES];
        
        int32_t i;
        
        // Reset states.
        for( i = 0; i < SERVERCLIENT_BASES; ++i ) {
            EntityState[i] = NEKO_NULL;
        }
        
        // Create a new server player entity.
        m_pEntity = new PlayerEntity();
        m_pEntity->Initialize(); // should be set by default in constructor
        
        memset( &m_pEntity->m_Link, 0x00, sizeof(SLink) );
        
        // Add the new entity to entity list.
        SList::AddHead( &g_Core->p_Server->m_Entities, &m_pEntity->m_Link, this->m_pEntity );
        
        m_pEntity->SpawnKey = g_Core->p_Server->EntitySpawnId;
        m_pEntity->Spawn();
        
        // Increment the spawn key value.
        ++g_Core->p_Server->EntitySpawnId;
        
        state = EServerClientState::Active;
    }
    
    /**
     *  Remove server client entity.
     */
    void CServerClient::RemoveEntity()
    {
        SList::RemoveAt( &g_Core->p_Server->m_Entities, &this->m_pEntity->m_Link );
        
        delete m_pEntity;
        m_pEntity = NEKO_NULL;
    }
    
    /**
     *  Process user command.
     *
     *  @param command - A just recieved user command.
     */
    void CServerClient::UpdateEntity( UserCommand *command )
    {
        // Set the last command.
        m_pEntity->Command = command;
        
        // Do entity events.
        m_pEntity->Process();
    }
    
    /**
     *  Process user command every frame.
     */
    void CServerClient::FrameUpdate(Neko::SBitMessage *message)
    {
        // FIX ME ( improve me )
        
        // Client entity misc.
        m_pEntity->m_vRotation.x = message->ReadFloat();
        m_pEntity->m_vRotation.y = message->ReadFloat();
        m_pEntity->m_vRotation.z = message->ReadFloat();
        
        m_pEntity->vMoveSensitivty.x = message->ReadFloat();
        m_pEntity->vMoveSensitivty.y = message->ReadFloat();
    }

    const static uint64_t   kServerMemSize = Megabyte( 64 );
    
    /**
     *  Initialize server.
     *
     *  @note - Load only on application start. Never calls again.
     */
    void CServer::Initialize( INekoAllocator * allocator )
    {
        uint32_t   t1, t2; // Load time.
        
        t1 = g_Core->p_System->Milliseconds();
        
        g_Core->p_Console->Print( LOG_INFO, "Server initializing...\n" );
        
        assert( !m_bInitialized );
        
        pAllocatorHandle = allocator;
        pAllocator = pAllocatorHandle;// NekoAllocator::newStackAllocator( kServerMemSize, *pAllocatorHandle );
        
        g_Core->p_Console->Print( LOG_INFO, "Registering server console variables..\n" );
        // Register server console variables.
        Server_Maxclients = g_Core->p_Console->RegisterCVar( ECvarGroup::Server, "iMaxClients", "Maximum amount of clients on server.", 5, CVFlag::NeedsRefresh, ECvarType::Int );
        Server_Name = g_Core->p_Console->RegisterCVar( ECvarGroup::Server, "sServerName", "Server name to be shown to all players.", DEFAULT_SERVER_NAME, CVFlag::NeedsRefresh, ECvarType::String );
        Server_Maxfps = g_Core->p_Console->RegisterCVar( ECvarGroup::Server, "iMaxFramerate", "Server framerate.", 20, CVFlag::NeedsRefresh, ECvarType::Int );
        Server_Sayname = g_Core->p_Console->RegisterCVar( ECvarGroup::Server, "sSayName", "Server name to be shown in chat.", "Friskies", CVFlag::NeedsRefresh, ECvarType::String );
        Server_Botname = g_Core->p_Console->RegisterCVar(  ECvarGroup::Server, "sBotName", "Server bot client name.", "Whiskas", CVFlag::NeedsRefresh, ECvarType::String );
        Server_Dedicated = g_Core->p_Console->RegisterCVar(  ECvarGroup::Server, "bDedicated", "Is server running in dedicated mode?", false, CVFlag::NeedsRefresh, ECvarType::Int );
        Server_Clienttimeout = g_Core->p_Console->RegisterCVar(  ECvarGroup::Server, "iClientTimeout", "Server client timeout time in seconds.", 30, CVFlag::NeedsRefresh, ECvarType::Int );
        Server_Statusperiod = g_Core->p_Console->RegisterCVar( ECvarGroup::Server, "iStatusPrintPeriod", "Print status per X seconds.", 120, CVFlag::None, ECvarType::Int );
        // g_Core->p_Console->RegisterCVar( Server_Cute );
        
        g_Core->p_Console->LoadIni( "neko" );
        
        // Initial values.
        Time = 0;
        LastTime = 0;
        LastInfoPrintTime = 0;
        EntitySpawnId = 0;
        
        ServerIdentificator = 0;
        LastInfoPrintTime = 0;
        Time = 0;
        LastTime = 0;
        
        memset( &m_Clients, 0x00, sizeof(SList) );
        m_State = EServerState::Idle;
        
        // Clear response data.
        memset( Response, 0x00, sizeof(SServerResponseData) * MAX_RESPONSES );
        
        // Check server console variables.
        CheckParams();
        
        // Initialize the server world.
        p_ServerWorld = (CServerWorld *)pAllocator->Alloc( sizeof(CServerWorld) );
        p_ServerWorld->Initialize();
        
        g_Core->p_Console->Print( LOG_INFO, "Initializing server client data...\n" );
        
        // Initialize client slots.
        SetupClients( Server_Maxclients->Get<int>() );
        
        // Let user know some stuff.
        g_Core->p_Console->Print( LOG_INFO, "Our server name is '%s'\n", Server_Name->Get<const char*>() );
        g_Core->p_Console->Print( LOG_INFO, "Maximum client allowed - '%i'\n", Server_Maxclients->Get<int>() );
        g_Core->p_Console->Print( LOG_INFO, "Our network port is '%i'\n", Network_Port->Get<int>() );
        
        
        m_bInitialized      = true;
        t2 = g_Core->p_System->Milliseconds();
        
        g_Core->p_Console->Print( LOG_INFO, "Server took %i ms to initialize.\n", t2 - t1 );
        g_Core->p_Console->Print( LOG_NONE, "\n" );
    }
    
    /**
     *  Create server.
     */
    void CServer::CreateSession( void )
    {
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        
        // Set server state.
        m_State = EServerState::Loading;

        g_Core->p_Console->Print( LOG_INFO, "Creating server session..\n" );
        g_Core->p_Console->Print( LOG_INFO, "Server name: %s\n", Server_Name->Get<const char*>() );
        
        // Will be reseted to default value if needed.
        SetupClients( Server_Maxclients->Get<int>() );
        
        // Create an entity data.
        g_Core->p_Console->Print( LOG_INFO, "Creating server entities\n" );
        
        //! Create entity linker.
        SList::CreateList( &m_Entities );

        // Create a world properties.
        p_ServerWorld->CreateWorld();
        
        // Some user information.
        g_Core->p_Console->Print( LOG_NONE, "\n");
        g_Core->p_Console->Print( LOG_INFO, "Maximum clients allowed: %i\n", Server_Maxclients->Get<int>() );
        g_Core->p_Console->Print( LOG_INFO, "Our network port is %i\n", Network_Port->Get<int>() );
        g_Core->p_Console->Print( LOG_NONE, "\n");

        m_bRunning = true ;
        g_Core->p_Client->m_bRunning = false ; // Safety reasons.

        g_Core->p_Console->Print( LOG_INFO, "....world has been created!\n" );
        
        
        // Reset server session id, so we get a nice value here.
        ServerIdentificator = 0xFF ^ g_Core->GetTime();

        // Everything went okay, set game to be active.
        g_Core->p_Console->Print( LOG_INFO, "Server is active now.\n" );
        g_Core->p_Console->Print( LOG_INFO, "> -------------------\n" );
        
        // Time warp fix.
        m_State   = EServerState::Game;
        Time    = 0;
    }
    
    /**
     *  Set maximum clients to be allowed.
     *  
     *  @param maxclients Maximum client amount to allocate.
     */
    void CServer::SetupClients( const uint32_t maxclients )
    {
        // Maximum value.
        if( maxclients >= MAX_CLIENTS_NUM ) {
            g_Core->p_Console->Print( LOG_WARN, "Could not set max clients value, maximum %i clients.\n", MAX_CLIENTS_NUM );
            Server_Maxclients->Set<int>( 512 );
            
            g_Core->p_Console->Print( LOG_INFO, "Maximum clients value set to 512\n" );
        } else if( maxclients < 1 ) {
            // Minumum value.
            g_Core->p_Console->Print( LOG_WARN, "Could not set max clients value to 0, minimum one client.\n" );
            Server_Maxclients->Set<int>( 24 );
            
            g_Core->p_Console->Print( LOG_INFO, "Maximum clients value set to 24 clients.\n" );
        }
        
        // We got nice checking here.
        SList::CreateList( &m_Clients );

        g_Core->p_Console->Print( LOG_DEVELOPER, "%i client slots loaded.\n", Server_Maxclients->Get<int>() );
    }
    
    
    /**
     *  Process client messages and commands.
     *
     *  @param client A server client currently processing.
     *  @param msg Bit stream message.
     */
    void CServer::ReadClientMessage( CServerClient * client, SBitMessage * msg )
    {
        int32_t commandSequence;
        const char * command;
        
        commandSequence = msg->ReadInt32();
        command = msg->ReadString();
        
        // Check if we recieve commands too fast.
        if( client->state != EServerClientState::Free && (this->Time < client->cachedReliableCommandAt) && client->address.addressType == ENetAddressType::IP )
        {
            g_Core->p_Console->Print( LOG_DEVELOPER, "ReadClientMessage(): client ( %s ) is command spamming\n", client->name );
        }
        
        client->cachedReliableCommandAt = this->Time + 1000; /* 1 sec. */
    
        if( commandSequence > client->lastExecutedCommand ) {
            
            // String parsing =>.<=

            NekoCString::TokenizeString( command );

            // TODO: MOVE ME
            // Process client commands now.
            if( !strcmp( NekoCString::lastCommandArguments[0], "disconnect" ) ) {
                DisconnectClient( client, "User quit" );
            } else if( !strcmp( NekoCString::lastCommandArguments[0], "say" ) ) {
                g_Core->p_Console->Print( LOG_NONE, "%s said: %s\n", client->name, NekoCString::lastCommandArguments[1] );
            }

        
            // Set last received values.
            client->lastReceivedCommand = commandSequence;
            client->lastExecutedCommand = commandSequence;
        } else {
            // Some probably commands were lost.
        }
    }
    
    const static int32_t MAX_USERCMDS_READ = 64;
    
    /**
     *  Process user(client) commands.
     *
     *  @param client - A client currently processing.
     *  @param message - Bitstream message data.
     */
    void CServer::ReadUserCommands( CServerClient *client, SBitMessage *message )
    {
        Byte numCommands;
        
        UserCommand commands[MAX_USERCMDS_READ]; // static?
        UserCommand * lastCommand = NEKO_NULL;
        
        int32_t i;
        numCommands = message->ReadByte();
        
    
        for( i = 0; i < numCommands; ++i ) {
            commands[i].Deserialize( message, lastCommand );
            
            lastCommand = &commands[i];
        }
        
        // Create a player entity.
        if( client->state != EServerClientState::Ghost && client->state != EServerClientState::Active ) {
            client->CreateEntity();
        }
        
        
        for( i = 0; i < numCommands; ++i ) {
            client->UpdateEntity( &commands[i] );
        }
        
        client->FrameUpdate( message );
    }
    
    const static uint32_t MSG_INITBUFFER_SIZE = 32;
    
    /**
     *  Send server state to a client.
     *
     *  @param client - A client currently processing.
     *  @param message - A bitstream message.
     */
    void CServer::SendNewState( CServerClient *client )
    {
        Byte initBuffer[MSG_INITBUFFER_SIZE];
        SBitMessage message( initBuffer, MSG_INITBUFFER_SIZE );
        
        // Write current command sequence. MOVE ME
        message.WriteInt16( client->channel.sequenceOut );
        
        message.WriteByte( COMMANDHEADER_INITSTATE );
        
        // Write a current state.
        message.WriteInt16( this->ServerIdentificator );
        // .. to do something else in the future..
        
        // Write a world information into the message.
        p_ServerWorld->SerializeWorld( &message );
        
        message.WriteByte( 0xFF );
        
        // Send packet now.
        g_Core->p_Network->SendMessageChannel( &client->channel, &message );
        
        client->lastRenewStateMessage = client->channel.sequenceOut;
    }
    
    /**
     *  Create a new client.
     *
     *  @param from A network packet structure with client address data.
     *  @param reponse A response identificator.
     *  @param name A string client name.
     *  @param version Client version string.
     */
    void CServer::CreateClient( SNetdata * from, int32_t response, const char * name, const char * version )
    {
        int32_t     x, i;
        int32_t		ping;
        
        // Create temporary client ( to be deleted ).
        CServerClient * p_temp = NEKO_NULL;
        
        i = 0;
        x = 0;
        ping = 0;
        
        // Local players don't need response verification.
        if( !g_Core->p_Network->IsLanAddress( &from->Address ) ) {
            for( i = 0; i < MAX_RESPONSES; ++i ) {
                // Compare addresses.
                if ( g_Core->p_Network->CompareAddress( &from->Address, &Response[i].Address.Address ) ) {
                    // We found what we need.
                    if ( response == Response[i].Response ) {
                        g_Core->p_Console->Print( LOG_DEVELOPER, "CreateClient(): - found connection data for %s\n", inet_ntoa(from->Address.sin_addr) );
                        break;
                    }
                }
            }
            
            // This may happen.
            if( i == MAX_RESPONSES ) {
                g_Core->p_Network->PrintOutOfBand( 
            ENetworkChannelType::ServerClient, from, "print \"No response data.\"" );
                g_Core->p_Console->Print( LOG_INFO, "No reponse data found for %s ( vers. %s )\n", name, version );
                return;
            }
            
            ping = (Time - Response[i].PingAt);
            
            g_Core->p_Console->Print( LOG_INFO, "Client is connecting...\n" );
            
            // If we have passed our tests before...
            Response[i].Connected = true;

        } else {
            /* A local client. */
            ping = 0;
            
            g_Core->p_Console->Print( LOG_INFO, "Local client is connecting..\n" );
        }
        

        // Finally create a client entry.
        
        
        // Create temporary client.
        p_temp = (CServerClient*)pAllocator->Alloc(  sizeof(CServerClient) );
        
        if( p_temp == NEKO_NULL )  { // Arghhh
            g_Core->p_Console->Error( ERR_SERVER, "Could not create a new client. Out of memory.\n" );
            return;
        }
        
        
        // Keep this first.
        p_temp->address.Address = from->Address;
        p_temp->address.Port = htons( from->Address.sin_port );  // Set port.

        // Copy data strings.
        NekoCString::Copy( p_temp->version, version ); // Copy version ( why do we need this??! )
        NekoCString::Copy( p_temp->name, name ); // Copy name.
        
        // Copy address data.
        g_Core->p_Network->CopyAddress( &p_temp->address, &from->Address.sin_addr );

        // Misc. information.
        p_temp->response = response;                        // Response index.
        p_temp->clientnum = (uint32_t)m_Clients.m_iCount;                      // Set client number.

        p_temp->state = EServerClientState::Connected;                     // Set client state.
        
        g_Core->p_Console->Print( LOG_INFO, "Client's address is \"%i.%i.%i.%i:%i\"\n",
                         p_temp->address.ip[0], p_temp->address.ip[1], p_temp->address.ip[2], p_temp->address.ip[3], p_temp->address.Port );
        
        
        // Check for local address.
        if( g_Core->p_Network->IsLanAddress( &p_temp->address ) /*  Check address. */ && (Network_Port->Get<int>() == p_temp->address.Port /* wooot :c */ )  ) {
            // Client is a host.
            p_temp->type = ENetAddressType::Loopback;
            p_temp->address.addressType = ENetAddressType::Loopback;
            g_Core->p_Console->Print( LOG_DEVELOPER, "CreateClient() - client has a local address.\n" );
        } else {
            p_temp->type = ENetAddressType::IP; // Else set client type to "outside" one.
            p_temp->address.addressType = ENetAddressType::IP;
        }
        
        // Keep these values.
        p_temp->lastMessageTime = Time;
        p_temp->lastConnectTime = Time;
        p_temp->lastMessageReceivedAt = Time;

        // Create network channel.
        g_Core->p_Network->CreateChannel( 
            ENetworkChannelType::ServerClient, &p_temp->channel, &p_temp->address );
        
        // Look up for empty client slots.
        // ..... and give our newcomer client a server slot!
        SList::AddHead( &m_Clients, &p_temp->m_Link, p_temp );
        
        // Soo if it's all is okay.. send connect response to our client.
        g_Core->p_Network->PrintOutOfBand( 
            ENetworkChannelType::ServerClient, from, "connectResponse %i %i", m_Clients.m_iCount - 1, 1337 );
        g_Core->p_Console->Print( LOG_INFO, "%s has joined the adventure. ( %i ) \n", name, m_Clients.m_iCount - 1 );
        
        // Clear response for this index.
        memset( &Response[i], 0, sizeof( SServerResponseData ) );
        
        // Finally!
        g_Core->p_Console->Print( LOG_INFO, "We got %i client(s) now.\n", m_Clients.m_iCount );
    }
    
    /**
     *  Server connectionless packets.
     *
     *  @param from A network data containing address and ip.
     *  @param data Network data recieved.
     */
    void CServer::Connectionless( SNetdata * from,  Byte * data )
    {
        // Skip first four bytes.
        // Use memcpy next time.
        data += 4;
//        memmove( &queue->message[0], &queue->message[4], queue->size - 4 );

        NekoCString::TokenizeString( (const char*)data );

        // Client requests a response.
        if( !strcmp(NekoCString::lastCommandArguments[0], "getresponse" ) ) {
            GetResponse(from);
        }
        
        // connect "response id" "Username" "Version"
        else if( !strcmp(NekoCString::lastCommandArguments[0], "connect" ) ) {
            CreateClient(from, atoi(NekoCString::lastCommandArguments[1]), NekoCString::lastCommandArguments[2], NekoCString::lastCommandArguments[3]);
        }
    }
    
    /**
     *  Parse client commands.
     *
     *  @param queue A network packet data containing address, etc..
     *  @param packet Bit message packet to read.
     */
    void CServer::ParseClients( SNetdata * queue, SBitMessage * packet )
    {
        if( m_Clients.m_iCount < 1 ) {
            return;
        }
        
        CServerClient * serverClientPtr;
        Byte command;
        uint32_t stateId;

        command = 0;
        serverClientPtr = GetClientByAddress( &queue->Address );    // Get a client by the address.
        
        // No client was found.
        if( serverClientPtr == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_INFO, "Server::ParseClients(): couldn't find client by ip \"%s\".\n", inet_ntoa( queue->Address.sin_addr) );
            return;
        }

        // Begin reading if everything is okay.
        packet->BeginReading();
        
        serverClientPtr->lastMessageReceivedAt = Time;
        serverClientPtr->lastAcknowledgedMessageAt = packet->ReadInt32();
        serverClientPtr->commandsAcknowledged = packet->ReadInt32();
        
        // Client server state id.
        stateId = packet->ReadInt32();
        
        // State id differs from the server, probably old client data,
        // so try to renew it.
        if( stateId != this->ServerIdentificator ) {
            if( serverClientPtr->lastAcknowledgedMessageAt >= serverClientPtr->lastRenewStateMessage ) {
                SendNewState( serverClientPtr );
            }
            
            return; // Don't go further, client has a wrong session id.
        }
        
        do {    //  Read command packet until '0xFF' byte.
            command = packet->ReadByte(); // Command header byte.

            switch(command) {
                /*  Entity move. */
                case COMMANDHEADER_MOVE:
                    ReadUserCommands( serverClientPtr, packet );
                    break;
                    
                /*  Reliable commands. */
                case COMMANDHEADER_ACK:
                    ReadClientMessage( serverClientPtr, packet );
                    break;
            }
        } while( command != 0xFF );

    }
    

    /**
     *  Send current game frame to client.
     *
     *  @param cl - A client currently processing.
     */
    void CServer::UpdateClient( CServerClient * cl )
    {
        if( cl == NEKO_NULL ) {
            return;
        }
        
        // Since bots are useless now.
        if( cl->type == ENetAddressType::Bot ) {
            return;
        }
        
        int32_t     i, j;
        bool    usesDelta;
        
        Entity * entity;
        SBitMessage * entityBase;
        
        // Create frame packet with data.
        Byte packetBuf[MAX_SNAPSHOT_SIZE];
        SBitMessage msg( packetBuf, MAX_SNAPSHOT_SIZE );
        
        msg.WriteInt16( cl->channel.sequenceOut );
  
        /**     Send reliable commands.     */
        for ( i = cl->commandsAcknowledged + 1; i <= cl->commandSequence; ++i ) {
            msg.WriteByte( COMMANDHEADER_ACK );
            msg.WriteInt16( i );
            msg.WriteString( cl->clientCommands[i & (MAX_RELIABLESERVERMESSAGE - 1)] );
        }
        
        usesDelta = true;
        
        /* Server entities. */
        msg.WriteByte( COMMANDHEADER_SERVERENTITY );
        msg.WriteInt16( Time );
        msg.WriteInt16( cl->lastReceivedCommand );
        
        // Calculate a client ping ( in ms ).
        cl->currentPing = this->Time - cl->lastMessageReceivedAt;
        
        if( (cl->channel.sequenceOut - cl->lastAcknowledgedMessageAt) >= MAX_SERVERPACKET_DELAY ) {
            g_Core->p_Console->Print( LOG_WARN, "SendFrame(): client %s got a large packet delay\n", cl->name );
            usesDelta = false;  // Renew.
        }
        
        // Server world entities.
        // TODO: Make clients to entities. - DONE
        
        // Using client data, okay for now, BUT
        // make me into server entity. - DONE
        
        SLink * head = &m_Entities.m_sList;
        
        j = 0;
//        for( j = 0; j < MAX_SERVER_ENTITIES; ++j )
        for( SLink * cur = head->m_pNext; cur != head; )
    	    {
            entity = (Entity *)cur->m_ptrData;// Entities[j];
            cur = cur->m_pNext;
            
            if( entity == NEKO_NULL ) {
                continue;
            }
            
            if( cl->EntityState == NEKO_NULL ) {
                continue;
            }
            
            // Entity number.
            msg.WriteInt16( j );
            
            if( usesDelta ) {
                entityBase = (cl->EntityState[j] ? cl->EntityState[j]->Get() : NEKO_NULL);
            } else {
                entityBase = NEKO_NULL;
            }
            
            /* static? */ DeltaBitMessage deltaMessage( entityBase, &msg );
            
            // write the spawn key
            deltaMessage.WriteInt16( entity->SpawnKey );
            
            // write the type code
            deltaMessage.WriteInt16( entity->TypeCode );
            
            // write the actual data
            entity->Serialize( &deltaMessage );
            
            // Create a new entity state for this client if not inited.
            if( cl->EntityState[j] == NEKO_NULL )
            {
                printf( "EntityState: null\n" );
                cl->EntityState[j] = new SServerClientEntityState( cl );
            }
            
            // Set the entity state.
            cl->EntityState[j]->Set( deltaMessage._newBaseStream );
            
            ++j;    // I feel bad about this, replace me by Entity Index.
        }
        
        /* End byte for entity data. */
        msg.WriteInt16( MAX_SERVER_ENTITIES );
        msg.WriteByte( 0xFF );
        
        g_Core->p_Network->SendMessageChannel( &cl->channel, &msg );
    }
    
    /**
     *  Send current game server world to all clients.
     */
    void CServer::UpdateClients( void )
    {
        SLink * head = &m_Clients.m_sList;
        SLink * cur;
        
        CServerClient * client;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            client = (CServerClient *)cur->m_ptrData;
            
            if( client->state == EServerClientState::Active ) {
                UpdateClient( client );
            }

        }
    }
    
    /**
     *  Listen for incoming data.
     *
     *  @param from - Currently recieved network information.
     *  @param data - Raw data recieved.
     */
    void CServer::Process( SNetdata * from,  Byte * data )
    {
//        if( !Server_Active.Get<bool>() )
//            return;

        // Client out of band packets.
        if( strlen((const char*)data) >= 4 && *(int*)data == -1 ) {
            Connectionless( from, data );
            return;
        }
        
        // Parse existing clients.
        SBitMessage message( (Byte*)data, MAX_MESSAGE_SIZE, true /* copy ( pointer to new buffer ) */ );
        ParseClients( from, &message );
    }
    
    
    /**
     *  Check server console variables.
     *
     *  @note: we check client maximum number in SetupClients.
     *  @return
     */
    void CServer::CheckParams( void )
    {
        g_Core->p_Console->Print( LOG_INFO, "Checking server configuration..\n" );
        
        if( strlen( Server_Name->Get<const char*>() ) > 64 ) {
            g_Core->p_Console->Print( LOG_WARN, "CheckParams(): Current server name is too long. Resetting to the default.\n" );
            Server_Name->Set<const char*>( DEFAULT_SERVER_NAME );
        }
        
        // Already checked in SetupClients, but nevermind...
        if( Server_Maxclients->Get<int>() > MAX_CLIENTS_NUM ) {
            g_Core->p_Console->Print( LOG_INFO, "CheckParams(): Server maximum clients value exceedes the limit, resetting..\n" );
            Server_Maxclients->Set<int>( SERVER_DEFAULT_CLIENTNUM );
            return;
        }
    }
    
    /**
     *  Clear the world.
     *
     *  @param msg Message.
     */
    void CServer::ClearWorld( const char * msg )
    {
        if( !m_bRunning ) {
            g_Core->p_Console->Print( LOG_WARN, "ClearWorld - I've tried to clear the world, but there's no server running!\n" );
            return;
        }
        
//        g_mainRenderer->RemoveWorld( "Server" );
    }
    
    /**
     *  Print server information.
     */
    void CServer::PrintInfo( void )
    {
        if( !m_bRunning ) {
            return;
        }
        
        if( Time - LastInfoPrintTime > (Server_Statusperiod->Get<int>() * 1000) )  {
            LastInfoPrintTime = Time;
            
            g_Core->p_Console->Print( LOG_INFO, "\n" );
            g_Core->p_Console->Print( LOG_INFO, "------------------ Server status ---------------------\n" );
            g_Core->p_Console->Print( LOG_INFO, "Server: \"%s\". We got %i users online.\n", Server_Name->Get<const char*>(), m_Clients.m_iCount );
            g_Core->p_Console->Print( LOG_INFO, "We are alive for %i minute(s).\n", (Time / 60000) );
        }
    }
    
    /**
     *  Check client timeouts.
     */
    void CServer::CheckTimeouts( void )
    {
        if( !m_bRunning ) {
            return;
        }
        
        SLink * head;
        SLink * cur;
        
        CServerClient * client;
        
        head = &m_Clients.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            client = (CServerClient *)cur->m_ptrData;
            
            if( (client->state == EServerClientState::Ghost || client->state == EServerClientState::Free) ) {
                continue;
            }
            
            if( client->type == ENetAddressType::Bot ) {
                continue;
            }
    
            if( client->lastMessageTime > Time ) {
                client->lastMessageTime = Time;
            }
            
            if( Time - client->lastMessageReceivedAt > Server_Clienttimeout->Get<int>() * 10000 ) {
                g_Core->p_Console->Print( LOG_INFO, "%s timed out.\n", client->name );
                KickClient( client, "Connection timed out." );
            }
        }
    }
    
    /**
     *  Server frame function.
     *
     *  @param msec Current game time.
     */
    void CServer::Frame( int msec )
    {
        if( !m_bRunning )  {
            return;
        }
        
        if( Server_Maxfps->Get<int>() < 1 ) {
            Server_Maxfps->Set<int>( 10 );
        }

        frameMsec = 1000 / Server_Maxfps->Get<int>();
        LastTime += msec;
        
        while( LastTime >= frameMsec )
        {
            LastTime -= frameMsec;
            Time += frameMsec;
            
            // Send the simulated world to all clients.
            UpdateClients();
            
            // Check ghost clients.
            CheckGhosts();
            
            // Check client timeouts.
            CheckTimeouts();
            
            // Update server world.
            UpdateServerWorld( frameMsec );
            
            // Print server information per X seconds.
//            PrintInfo();
        }
    }
    
    /**
     *  Update server world.
     *
     *  @param msec Server frametime.
     */
    void CServer::UpdateServerWorld( int32_t msec )
    {
        float   msecInSec;

		if (m_Entities.m_iCount < 1) {
			return;
		}
        
        Entity *    pEntity = NEKO_NULL;
        
        msecInSec = (float)msec / 1000.0f;
        
        // TODO: replace MAX_SERVER_ENTITIES by realtime value.
        SLink * head = &m_Entities.m_sList;
//        for( i = 0; i < MAX_SERVER_ENTITIES; ++i )
        for( SLink * cur = head->m_pNext; cur != head; )
        {
            // Get an entity at index.
            pEntity = (Entity*)cur->m_ptrData;// Entities[i];
            cur = cur->m_pNext;
            
            // If entity doesn't exist then don't go further.
            if( pEntity == NEKO_NULL ) {
                continue;
            }
            
            // Update an entity.
            pEntity->Update( msecInSec );
        }
    }

    /**
     *  Disconnect client with a reason.
     *
     *  @param client - Client currently processing.
     *  @param message - Reason message.
     */
    void CServer::DisconnectClient( CServerClient * client, const char * message )
    {
        if( client->state == EServerClientState::Ghost ) {
            return;
        }
        
        if( !message ) {
            message = "No reason."; // meh
        }
        
        SendCommand( client, true, "disconnect \"%s\"", message );
        UpdateClient( client );
        
        g_Core->p_Console->Print( LOG_INFO, "%s disconnected from server. %s\n", client->name, message );
        
        SendCommand( NEKO_NULL, false, "print \"%s disconnected. %s\"", client->name, message );
        UpdateClient( client );
        
        client->ghostifiedAt = Time;
        client->state = EServerClientState::Ghost;
    }
    
    /**
     *  Get auth response from client.
     *
     *  @param from - A network information.
     */
    void CServer::GetResponse( SNetdata * from )
    {
        int32_t i;
        int32_t previous;
        int32_t previousTime;

        SServerResponseData *response = NEKO_NULL;
        
        // Get a nice value here.
        previousTime = 0x7FFFFFFF;
        
        // Check if response already exists for this client.
        response = &Response[0];
        
        // Don't overflow.
        for ( i = 0 ; i < MAX_RESPONSES; ++i, ++response ) {
            if ( !response->Connected && g_Core->p_Network->CompareAddress( &from->Address, &response->Address.Address ) ) {
                break;  // We already have got client's information.
            }
            
            if ( response->Time < previousTime ) {
                previousTime = response->Time;
                previous = i;
            }
        }
        
        // Generate a new response data.
        if ( i == MAX_RESPONSES )  {
            // Just connected.
            response = &Response[previous];
            
            /* Generate nice random number for this response. */
            response->Response = ( (rand() << 16) ^ rand() ) ^ Time;
            response->Address = *from; // TODO: use memcpy here?
            
            if( g_Core->p_Network->IsLanAddress( &response->Address.Address ) ) {
                response->Address.addressType = ENetAddressType::Loopback;
            } else {
                response->Address.addressType = ENetAddressType::IP;
            }
            
            response->FirstAt = Time;
            response->Time = Time;
            response->Connected = false;
            
            i = previous;
        }
        
        if ( Time - response->FirstAt > SERVER_CLIENTAUTHORIZE_TIME )  {
            g_Core->p_Console->Print( LOG_INFO, "Authorizing client... ( %s:%i )\n", inet_ntoa(response->Address.Address.sin_addr), ntohs(response->Address.Address.sin_port) );
            
            // TODO: authorizing on server database!
            
            
            response->PingAt = Time;
            g_Core->p_Network->PrintOutOfBand( 
            ENetworkChannelType::ServerClient, &response->Address, "requestResponse %i", response->Response );
            return;
        }
        
        // Kick client if get here.
        // Edit: Don't!
        g_Core->p_Console->Print( LOG_WARN, "Trying to authorize %s ( %i authtime )\n", inet_ntoa(response->Address.Address.sin_addr), Time - response->FirstAt );
        // zeromem( response, sizeof(ncResponse) );
    }
    
    /**
     *  Kick stuck players.
     */
    void CServer::CheckGhosts( void )
    {
        SLink * head;
        SLink * cur;
        
        CServerClient * client;
        
        head = &m_Clients.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            client = (CServerClient *)cur->m_ptrData;
            
            // Check for a client state.
            if( client->state == EServerClientState::Ghost ) {
                // Check a client timeout.
                if( Time - client->ghostifiedAt > 1000 )  {
                    SList::RemoveAt( &m_Clients, &client->m_Link );
                    
                    memset( client, 0x00, sizeof(CServerClient) );
                    
                    client->state = EServerClientState::Free;
                    delete client;
                }
            }
        }
    }
    
    /**
     *  Get clients status and server information.
     */
    void CServer::PrintStatus( void )
    {
        if( !m_bRunning )  {
            g_Core->p_Console->Print( LOG_INFO, "Server is not active!\n" );
            return;
        }
        
        SLink * head;
        SLink * cur;
        
        CServerClient * client;
        
        const char * state;
        
        // Print server information first.
        g_Core->p_Console->Print( LOG_INFO, "Our server name is '%s'\n", Server_Name->Get<const char*>() );
        g_Core->p_Console->Print( LOG_INFO, "Our server address is '%s:%i'\n", Network_IPAddress->Get<const char*>(), Network_Port->Get<int>() );
        g_Core->p_Console->Print( LOG_INFO, "We got %i clients online\n", m_Clients.m_iCount );
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        
        // Print player information now.
        
        head = &m_Clients.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            client = (CServerClient *) cur->m_ptrData;
            
            switch( client->state ) {
                case EServerClientState::Ghost:
                    state = "No response";
                    break;
                case EServerClientState::Connected:
                    state = "Okay";
                    break;
                case EServerClientState::Connecting:
                    state = "Connecting";
                    break;
                default:
                    state = "Unknown";
                    break;
            }
            
            g_Core->p_Console->Print( LOG_NONE, "%i. %s - %s (%i)\n", client->clientnum, client->name, state, client->currentPing );
        }
    }
    
    /**
     *  Get client by number.
     *
     *  @param num - Client number ( identificator ).
     */
    CServerClient * CServer::GetClientByNum( int32_t num )
    {
        SLink * cur;
        CServerClient * client;
        
        for( cur = m_Clients.m_sList.m_pNext; cur != &m_Clients.m_sList; cur = cur->m_pNext ) {
            client = (CServerClient *)cur->m_ptrData;
            
            if(client->clientnum == num) {
                return client;
            }
        }
        
        return NEKO_NULL; // meh
    }
    
//#define ADDRESS_STRING_COMPARE // Doesn't work yet
    
    /**
     *  Get client by an ip address.
     *
     *  @param data - Raw network information.
     */
    CServerClient * CServer::GetClientByAddress( const sockaddr_in * data )
    {
        SLink * head;
        SLink * cur;
        
        CServerClient * client;
        
        head = &m_Clients.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            client = (CServerClient *) cur->m_ptrData;
            
            if( g_Core->p_Network->CompareAddress( &client->address.Address, data ) ) {
                if( client->address.Port == htons(data->sin_port) ) {
                    return client; // F-f-f-fffound one!
                }
            }
        }
        
        return NEKO_NULL;
    }
    
    /**
     *  Send Bye-bye message to all clients.
     *
     *  @param msg - A reason message.
     */
    void CServer::SendByeMessage( const char * msg )
    {
        g_Core->p_Console->Print( LOG_INFO, "SendByeMessage: Implement me!\n" );
    }
    
    /**
     *  Kick client(s) with a reason.
     */
    void CServer::KickClient( CServerClient * client, const char * message )
    {
        if( !m_bRunning ) {
            g_Core->p_Console->Print( LOG_INFO, "Server is not active.\n" );
            return;
        }
        
        int32_t i;
        SServerResponseData * response;
        
        // Safety.
        if( client->state == EServerClientState::Free || client->state == EServerClientState::Ghost ) {
            return;
        }
        
        response = &Response[0];
        
        for( i = 0; i < MAX_RESPONSES; ++i, ++response ) {
            if( g_Core->p_Network->CompareAddress( &client->address.Address, &response->Address.Address ) ) {
                memset( response, 0x00, sizeof(SServerResponseData) );
                break; // return;
            }
        }
 
        AddReliableCommand( client, true, (char*)NC_TEXT( "disconnect \"%s\"", message ) );
        UpdateClient( client );
        
        g_Core->p_Console->Print( LOG_INFO, "%s disconnected: \"%s\"\n", client->name, message );
        SendCommand( NEKO_NULL, true, "print \"%s disconnected: %s\"", client->name, message );
        
        // Remove server client entity.
        client->RemoveEntity();
        
        // Set the client state to zombie so it will be removed after some milliseconds.
        client->state = EServerClientState::Ghost;
        client->ghostifiedAt = Time;
    }
    
    /**
     *  Add ack command.
     *
     *  @param cl - A client currently processing.
     *  @param isDisconnect - Is it forced disconnect command?
     *  @param message - A reason message.
     */
    void CServer::AddReliableCommand( CServerClient *cl, bool isDisconnect, char * message )
    {
        uint32_t unAcked;
        
        unAcked = cl->commandSequence - cl->commandsAcknowledged;
        
        /* We can't clear ack buffer for now, so throw error. */
        if( (unAcked > MAX_RELIABLESERVERMESSAGE) && !isDisconnect )  {
            DisconnectClient( cl, "Server command overflow." );
        }
        
        ++cl->commandSequence;// += 1;
        cl->clientCommands[cl->commandSequence & (MAX_RELIABLESERVERMESSAGE-1)] = message;
    }
    
    /**
     *  Send ack command to client(s).
     *
     *  @note - Use NEKO_NULL as first parameter to send a reliable command to
     *  all clients.
     *  @param cl - A client currently processing.
     *  @param isDisconnect - Is it forced disconnect command?
     *  @param cmd - A command string.
     *
     *  @return
     */
    
    void CServer::SendCommand( CServerClient * cl, bool isDisconnect, const char * cmd, ... )
    {
        va_list argptr;
        static char message[1024];
        
        va_start( argptr, cmd );
        vsnprintf( (char * )message, sizeof(message), cmd, argptr );
        va_end( argptr );
        
        if( cl == NEKO_NULL ) {
            SLink * head;
            SLink * cur;
            
            CServerClient * client;
            
            head = &m_Clients.m_sList;
            
            for( cur = head->m_pNext; cur != head; cur = cur->m_pNext )  {
                client = (CServerClient *) cur->m_ptrData;
                AddReliableCommand( client, isDisconnect, message );
            }
        }  else {
            AddReliableCommand( cl, isDisconnect, message );
        }
    }
    
    /**
     *  Remove the bots.
     */
    void CServer::RemoveBots( void )
    {
        if( !m_bRunning )  {
            return;
        }
        
        g_Core->p_Console->Print( LOG_INFO, "Removing all bots from the game.\n" );
        
        SLink * head;
        SLink * cur;
        
        CServerClient * client;
        
        head = &m_Clients.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            client = (CServerClient *) cur->m_ptrData;
            
            if(client->type == ENetAddressType::Bot) {
                g_Core->p_Console->Print( LOG_INFO, "%s kicked..\n", client->name );
                DisconnectClient(client, "svr");
            }
        }
    }
    
    /**
     *  Close the server.
     */
    void CServer::Disconnect( void )
    {
        if( !m_bRunning ) {
            return;
        }
        
        // Clear the world and close all connections.
        RemoveBots();                                    // Remove all bots.
        SendByeMessage( "Server quit.\n" );              // Kick all clients.
        ClearWorld( "Server disconnect" );               // Clear the world.
        
        m_bRunning = false ;
        
        // Disconnect local client. ( If exists ).
        g_Core->Disconnect();
    }
    
    /**
     *  Server shutdown.
     *  Called on application quit.
     *
     *  @param finalmsg - A final message string to be sent to all clients.
     */
    void CServer::Shutdown( const char * finalmsg )
    {
        if( !m_bRunning ) {
            g_Core->p_Console->Print( LOG_INFO, "No server running!\n" );
            return;
        }
        
        g_Core->p_Console->Print( LOG_INFO, "Server shutting down...\n" );
        
        // Send last message to all clients and disconnect them.
        Disconnect();
        
        // We must forget all information about clients.
        m_Entities.m_sList.Destroy();
        memset( &m_Entities, 0x00, sizeof(SList) );
        
        g_Core->p_Console->Print( LOG_INFO, "Server quit after %i minute(s) since last session launch.\n", ( Time / 60000 ) );
        
        Time    =   0;
        m_State   =   EServerState::Idle;
        
        pAllocator->Dealloc( p_ServerWorld );
//        NekoAllocator::deleteStackAllocator( (CStackAllocator *)pAllocator, pAllocatorHandle );
    }
    
    /**
     *  Create new server instance.
     */
    CServer::CServer() : m_bInitialized( false )
    {
        
    }
    
    /**
     *  Add bot to server.
     *  Takes one client slot.
     */
    void CServer::AddBot( void )
    {
        g_Core->p_Console->Print( LOG_INFO, "Implement me!\n" );
    }
    
    // Server client entities.
    
    /**
     *  Create a new server client entity base.
     */
    SServerClientEntityState::SServerClientEntityState( CServerClient * client )
    {
        // Set the server client.
        this->Client = client;
        
        // Set entity states.
        this->Bases = new SBitMessage[ENTITYSTATE_SIZE];
        
        // Used?
        this->Used = true;
    }
    
    /**
     *  Default constructor for ServerClientEntityBase.
     */
    SServerClientEntityState::SServerClientEntityState()
    {
        Bases = NEKO_NULL;
        Client = NEKO_NULL;
        Used = false;
    }
    
    /**
     *  Default destructor!!
     */
    SServerClientEntityState::~SServerClientEntityState()
    {
        
    }

}
