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
//  Network.cpp
//  Game network manager..
//
//  Created by Neko Vision on 11/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//


#include "Network.h"
#include "MultiplayerServer.h"
#include "MultiplayerClient.h"

#include "../String/StringHelper.h"

#include "../Console/Console.h"

namespace Neko {
    
    /***
     **      Console Variables.
     **/
    
    /*   Network port.   */
    SConsoleVar    * Network_Port = 0;
    
    /*   Network ip address.     */
    SConsoleVar    * Network_IPAddress = 0;
    
    /*   Networking address type.   */
    SConsoleVar    * Network_AddressType = 0;
    
    /*   Local ip address.  */
    SConsoleVar    * Network_LocalIPAddress = 0;
  
    /*  Set local address?   */
    SConsoleVar    * Network_SetLocalAddress = 0;

    
    /**
     *  Initialize network.
     *
     *  @note - calls only once.
     */
    void CNetwork::Initialize( INekoAllocator * allocator )
    {
        uint32_t port, i;
        
        // Socket flags to prevent blocking.
        int16_t flags;
        
        // Blocking.
        bool g_allow;
        
        struct hostent * hp = NEKO_NULL;
        static char n_hostname[MAX_NET_HOSTNAME_LENGTH]; // If requested.
        
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Initializing networking..\n" );
        
        i = 0;
        
        m_bRunning = false ;
        // Register network console vars.
        Network_AddressType = g_Core->p_Console->RegisterCVar( ECvarGroup::Network, "iNetworkType", "Network address type.", 0, CVFlag::NeedsRefresh, ECvarType::Int );
        Network_IPAddress = g_Core->p_Console->RegisterCVar( ECvarGroup::Network, "sNetworkIP", "Network IP-Address.", "0.0.0.0", CVFlag::NeedsRefresh, ECvarType::String );
        Network_LocalIPAddress = g_Core->p_Console->RegisterCVar( ECvarGroup::Network, "sNetworkLocalIp", "Network local IP-Address.", "0", CVFlag::NeedsRefresh, ECvarType::String );
        Network_Port = g_Core->p_Console->RegisterCVar( ECvarGroup::Network, "iNetworkPort", "Network port to be binded and used.", 4004, CVFlag::NeedsRefresh, ECvarType::Int );
        Network_SetLocalAddress = g_Core->p_Console->RegisterCVar(  ECvarGroup::Network, "bNetworkForceAddress", "Set network local address?", true, CVFlag::NeedsRefresh, ECvarType::Int );
        
        g_Core->p_Console->LoadIni( "neko" );
        
#   if defined( _WIN32 )
        // We have to initialize socket on Windows.
        g_Core->p_Console->Print( LOG_INFO, "Initializing Winsocket as requested..\n" );
        WSADATA  w;
        if( WSAStartup( WINSOCKET_STARTUP_CODE, &w ) != 0 ) {
            g_Core->p_Console->Error( ERR_NETWORK, "Failed to initialize Windows socket.\n" );
            return;
        }
#   endif

        pAllocatorHandle = allocator;
        const size_t sz = Megabyte( 3 );
        pAllocator = NekoAllocator::newStackAllocator( sz, *pAllocatorHandle );
        
        // Create network socket.
        g_Core->p_Console->Print( LOG_INFO, "Creating network socket using udp protocol..\n" );
        
        this->NetworkSocket = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
        if( this->NetworkSocket == - 1 ) {
            g_Core->p_Console->Print( LOG_ERROR, "%s\n", strerror( errno ) );
            g_Core->p_Console->Error( ERR_NETWORK, "Could not initialize network socket. %s", strerror( errno ) );
            
            return;
        }
        
        
        // Prevent the network socket blocking.
#   if defined( _WIN32 )
        
        long /*uint32_t*/unsigned blockmode;
        blockmode = 1;
        
        if( ioctlsocket( this->NetworkSocket, FIONBIO, &blockmode ) == -1 ) {
            g_Core->p_Console->Error( ERR_NETWORK, "ioctl failed to make socket to non-blocking mode.\n");
            return;
        }
        
#   else
        //// Mac, and another *nix systems.
        
        flags = fcntl( this->NetworkSocket, F_GETFL );
        flags |= O_NONBLOCK;
        
        // Update flags.
        fcntl( this->NetworkSocket, F_SETFL, flags );
        
        if( ioctl( NetworkSocket, FIONBIO, &g_allow ) == -1 ) {
            g_Core->p_Console->Error( ERR_NETWORK, "ioctl failed to make socket to non-blocking mode.\n");
            return;
        }
        
        
        // In some cases it fails to make it non-blocking.
        flags = fcntl( NetworkSocket, F_GETFL );
        flags |= O_NONBLOCK;
        
        fcntl( NetworkSocket, F_SETFL, flags );
    
#   endif

        // Get local data.
        if( gethostname( n_hostname, sizeof(n_hostname) ) == -1 ) {
            g_Core->p_Console->Error( ERR_NETWORK, "Network::Init() - Could not get host name.\n" );
            return;
        }

        const char * n_localIP = NEKO_NULL;
        
        // Set local network address?
        if( Network_SetLocalAddress->Get<bool>() )  {
            hp = gethostbyname( n_hostname );
            
            if( !hp ) {
                g_Core->p_Console->Error( ERR_FATAL, "Network::Init() - Couldn't get host by name. \n%s\n", strerror( errno ) );
                return;
            }
            
            // Get address string.
            n_localIP = inet_ntoa( *(struct in_addr *)*hp->h_addr_list );
        } else {
            n_localIP = "null";
        }
        
        // Some information.
        g_Core->p_Console->Print( LOG_INFO, "Our local host name is '%s'\n", n_hostname );
        g_Core->p_Console->Print( LOG_INFO, "Our local address is %s\n", n_localIP );
        
        Network_LocalIPAddress->Set<const char*>( n_localIP );
        
        memset( &DataAddr, 0, sizeof(struct sockaddr_in) );
        
        port = Network_Port->Get<int>();

        // Set socket broadcasting.
        if( setsockopt( NetworkSocket, SOL_SOCKET, SO_BROADCAST, (char * )&i, sizeof(i) ) == -1 ) {
            g_Core->p_Console->Error( ERR_NETWORK, "Failed to setup socket broadcasting.\n" );
            
            return;
        }
        
        // Scan for available ports.
        g_Core->p_Console->Print( LOG_INFO, "Binding network socket with parameters..\n" );
        
        
        // Find the best configuration and break; - out // haha
        while( true )
        {
            //  Reset current configuration.
            memset( &DataAddr, 0, sizeof(struct sockaddr_in) );
            
            DataAddr.sin_family = AF_INET;
            DataAddr.sin_port = htons( port );
            
            switch( Network_AddressType->Get<int>() )
            {
                case 0: // Any.
                    g_Core->p_Console->Print( LOG_INFO, "Our network type is any.\n" );
                    DataAddr.sin_addr.s_addr = INADDR_ANY;
                    break;
                case 1: // Set automatically.
                    g_Core->p_Console->Print( LOG_INFO, "Our network type is automatic.\n" );
                    
                    hp = gethostbyname(n_hostname);
                    if( !hp ) {
                        g_Core->p_Console->Error( ERR_NETWORK, "Couldn't resolve \"%s\"", n_hostname );
                    }
                    
                    // Copy address data.
                    memcpy( (void*)&DataAddr.sin_addr, hp->h_addr_list[0], hp->h_length );
                    
                    break;
                case 2: // Manual
                    g_Core->p_Console->Print( LOG_INFO, "Our network type is manual. Name: \"%s\"\n", Network_IPAddress->Get<const char*>() );
                    
                    break;
            }
            
            // Try to bind the address.
            if( bind( NetworkSocket, (struct sockaddr *)&DataAddr, sizeof(DataAddr) ) != 0 ) {
                g_Core->p_Console->Print( LOG_WARN, "Network::Init(): bind failed, trying to change the port..\n" );
                
                ++port;
                
                Network_Port->Set<int>( port );
                continue;
            }
            
            // Everything is okay, break!
            break;
        }
        
        // Create network queues.
        g_Core->p_Console->Print( LOG_INFO, "Creating network queue\n" );
        
        m_networkMessageQueue = (SQueueType*)pAllocator->Alloc( sizeof(SQueueType) * MAX_NETWORK_QUEUE );
        m_loopbackServerQueue = (SQueueType*)pAllocator->Alloc( sizeof(SQueueType) * MAX_SERVERSIDE_QUEUE );
        m_loopbackClientQueue = (SQueueType*)pAllocator->Alloc( sizeof(SQueueType) * MAX_CLIENTSIDE_QUEUE );
        
        m_nextQueueLocation = 0;
        m_nextMessageForProcessing = 0;
        
        m_loopbackTotalClientQueues = 0;
        m_loopbackTotalServerQueues = 0;

        // Reset queues.
        EmptyQueue( m_networkMessageQueue, MAX_NETWORK_QUEUE );
        EmptyQueue( m_loopbackServerQueue, MAX_SERVERSIDE_QUEUE );
        EmptyQueue( m_loopbackClientQueue, MAX_CLIENTSIDE_QUEUE );
        
        
        // Set network information console variables.
        Network_IPAddress->Set<const char*>( inet_ntoa(DataAddr.sin_addr) );
        Network_Port->Set<int>( port );
        
        if( Network_AddressType->Get<int>() == 1 ) {
            g_Core->p_Console->Print( LOG_INFO, "Network: Listening on '%s:%i'. \n", inet_ntoa(DataAddr.sin_addr), port );
        } else {
            g_Core->p_Console->Print( LOG_INFO, "Network: Listening on '%i' port. \n", port );
        }
            
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        
        // Turn on the networking.
        m_bRunning = true ;
    }
    
    /**
     *  Empty queue.
     *
     *  @param queue - Queue to empty.
     *  @param queueSize - Size of a queue to empty.
     */
    void CNetwork::EmptyQueue( SQueueType *queue, const uint32_t queueSize )
    {
        uint32_t i;
        
        for( i = 0; i < queueSize; ++i ) {
            queue[i].active = false;
            queue[i].size = 0;
        }
    }
    
    /**
     *  Parse all received data.
     */
    void CNetwork::Frame( void )
    {
        if( !m_bRunning ) {
            return;
        }

        // Server address.
        SNetdata remoteEP;
        
        size_t bytes;
        Byte data[MAX_MESSAGE_SIZE + 4 /* Protocol header. */];
        
        // -- Maybe I should use 'socklen_t' ?!?!
#   if defined( _WIN32 )
        int32_t packet_len;
#   else
        uint32_t packet_len;
#   endif

        packet_len = sizeof( RecvAddr );
        
        // Recieve raw data.
        bytes = recvfrom( NetworkSocket, (char*)&data, sizeof(data), 0, (struct sockaddr *)&RecvAddr, &packet_len );
        
        // Something went wrong.
        if( bytes == -1 )
        {
#   if defined( _WIN32 )
            if( errno == WSAEWOULDBLOCK || errno == WSAECONNREFUSED ) {
                return;
            }
#   else
            if( errno == EWOULDBLOCK || errno == ECONNREFUSED ) {
                return;
            }
#   endif
           // g_Core->p_Console->Print( LOG_WARN, "CNetwork::recvfrom() - %s\n", strerror( errno ) );
            
            return;
        }
        
        // Network packet size check.
        if( bytes >= MAX_MESSAGE_SIZE ) {
            g_Core->p_Console->Print( LOG_WARN, "Network packet from \"%s\" exceedes maximum packet size!\n", inet_ntoa( RecvAddr.sin_addr ) );
            
            return;
        }
        
        // Check a protocol.
        if ( data[0] != (Byte)( NETWORK_PROTOCOL_ID >> 24 ) ||
            data[1] != (Byte)( ( NETWORK_PROTOCOL_ID >> 16 ) & 0xFF ) ||
            data[2] != (Byte)( ( NETWORK_PROTOCOL_ID >> 8 ) & 0xFF ) ||
            data[3] != (Byte)( NETWORK_PROTOCOL_ID & 0xFF ) ) {
            
            g_Core->p_Console->Print( LOG_WARN, "User from \"%s\" has wrong network protocolId.\n", inet_ntoa( RecvAddr.sin_addr) );
            return;
        }
        
        // Add a new network queue.
        
        // Set queue properties.
        m_networkMessageQueue[m_nextQueueLocation].address = RecvAddr;
        m_networkMessageQueue[m_nextQueueLocation].size = bytes;
        
        // Copy a message.
        memcpy( m_networkMessageQueue[m_nextQueueLocation].message, &data[4], bytes - 4 );
        
        // Prevent race conditions.
        m_networkMessageQueue[m_nextQueueLocation].active = true;

        // See if the next message is ready.
        if( m_networkMessageQueue[m_nextMessageForProcessing].active ) {
            remoteEP.Address = m_networkMessageQueue[m_nextMessageForProcessing].address;
            
            // If everything is passed through, and it's all is okay.. do it!
            // Assign data packet.
            // Decide where we should give our packet.
            if( g_Core->p_Server->m_bRunning ) {
                g_Core->p_Server->Process( &remoteEP,
                                   (Byte*)m_networkMessageQueue[m_nextMessageForProcessing].message  );
            } else {
                g_Core->p_Client->Process( &remoteEP,
                                   (Byte*)m_networkMessageQueue[m_nextMessageForProcessing].message );
            }
            
            m_networkMessageQueue[m_nextMessageForProcessing].message[bytes] = 0;
            
            
            // - Post processing stage.
            
            // Set the message as processed.
            m_networkMessageQueue[m_nextMessageForProcessing].active = false;
            
            // Increment the queue position.
            ++m_nextMessageForProcessing;
            if( m_nextMessageForProcessing == MAX_NETWORK_QUEUE ) {
                m_nextMessageForProcessing = 0;
            }
        }
        
        // Increment the queue position.
        ++m_nextQueueLocation;
        if( m_nextQueueLocation == MAX_NETWORK_QUEUE ) {
            m_nextQueueLocation = 0;
        }

//        network_message_buffer[bytes] = 0;
    }
    
    /**
     *  Send a loopback packet.
     *
     *  @param type - Network channel type.
     *  @param len - Packet size.
     *  @param data - Packet raw message data.
     */
    void CNetwork::SendLoopbackPacket( ENetworkChannelType type, unsigned long len, const void *data )
    {
        SQueueType * queue = NEKO_NULL;

        if( type == ENetworkChannelType::ClientServer ) {
            // Client to server.
            queue = &m_loopbackServerQueue[m_loopbackTotalServerQueues];

            ++m_loopbackTotalServerQueues ;
            
            if( m_loopbackTotalServerQueues >= MAX_CLIENTSIDE_QUEUE )  {
                m_loopbackTotalServerQueues = 0;
            }
        } else if( type ==   ENetworkChannelType::ServerClient ) {
            // Server to client.
            queue = &m_loopbackClientQueue[m_loopbackTotalClientQueues];

            ++m_loopbackTotalClientQueues ;
            
            if( m_loopbackTotalClientQueues >= MAX_SERVERSIDE_QUEUE ) {
                m_loopbackTotalClientQueues = 0;
            }
        }
        

        // Server or client queue.
        queue->active = true;
        memcpy( queue->message, data, len );
        queue->size = (int)len;
    }
    
    /**
     *  Get a loopback packet.
     *
     *  @param type - Network packet type.
     */
    Byte * CNetwork::GetLoopbackPacket( ENetworkChannelType type )
    {
        SQueueType * queue = NEKO_NULL;
        uint32_t queueCount;
        
        if( type == ENetworkChannelType::ClientServer ) {   // Client to server.

            // Use server queue.
            queue = m_loopbackServerQueue;
            queueCount = m_loopbackTotalServerQueues;
            
            if( m_loopbackTotalServerQueues > 0 ) {
                --m_loopbackTotalServerQueues;
            }
            
        } else if( type ==
            ENetworkChannelType::ServerClient ) {    // Server to client.

            // Use client queue.
            queue = m_loopbackClientQueue;
            queueCount = m_loopbackTotalClientQueues;
            
            if( m_loopbackTotalClientQueues > 0 ) {
                --m_loopbackTotalClientQueues;
            }
        }
        
        if( queue && queueCount > 0 ) {
            --queueCount;
            if( queue[queueCount].active ) {
                queue[queueCount].active = false;
                return (Byte*)queue[queueCount].message;    // Can be NEKO_NULL!
            } else {
                return NEKO_NULL;
            }
        } else {
            return NEKO_NULL;
        }
    }
    
    /**
     *  Network channel.
     */
    void CNetwork::CreateChannel( ENetworkChannelType type, ncNetchannel * chan, SNetdata * adr )
    {
        chan->address.Port = adr->Port;
//        chan->address.Socket = NetworkSocket;
        chan->type = type;
        chan->address.addressType = adr->addressType; // ! Very imporant
        
        memcpy( &chan->address.Address, &adr->Address, sizeof(adr->Address) );
    }
    
    /**
     *  Send message to chosen channel.
     */
    void CNetwork::SendMessageChannel( ncNetchannel * chan, SBitMessage *msg )
    {
        
        ++chan->sequenceOut;// += 1;
        
        // Local address.
        if( chan->address.addressType == ENetAddressType::IP ) {
            SendPacket( msg->Size, msg->Data, &chan->address );
        } else if( chan->address.addressType == ENetAddressType::Loopback ) {
            SendLoopbackPacket( chan->type, msg->Size, msg->Data );
        }
    }
    
    bool CNetwork::ProcessChannel( ncNetchannel *chan, byte *packet )
    {
        return true;
    }
    
    /**
     *  Shutdown networking.
     */
    void CNetwork::Shutdown( void )
    {
        if( !m_bRunning ) {
            return;
        }
        
        // Remove queue base.
        pAllocator->Dealloc( m_loopbackClientQueue );
        pAllocator->Dealloc( m_loopbackServerQueue );
        pAllocator->Dealloc( m_networkMessageQueue );
        
        NekoAllocator::deleteStackAllocator( (CStackAllocator *)pAllocator, pAllocatorHandle );
        
        // 2 - Disable receiving/sending
        shutdown( NetworkSocket, 2 );

		// So much differences.. wow... just wow..
#   if !defined( _WIN32 )
        close( NetworkSocket );
#   else
        closesocket(NetworkSocket);
        // Windows needs sockets to be closed.
        WSACleanup();
#   endif
  
        g_Core->p_Console->Print( LOG_INFO, "Successfully closed the network socket.\n" );
    }
    

    /**
     *      Send packet.
     */
    void CNetwork::SendPacket( unsigned long len, const void * data, SNetdata * from )
    {
        size_t ret;
        Byte packet[MAX_MESSAGE_SIZE + 4]; // FIX MEEEEEE
        
        // First four bytes - protocol.
        packet[0] = (Byte)( NETWORK_PROTOCOL_ID >> 24 );
        packet[1] = (Byte)( ( NETWORK_PROTOCOL_ID >> 16 ) & 0xFF );
        packet[2] = (Byte)( ( NETWORK_PROTOCOL_ID >> 8 ) & 0xFF );
        packet[3] = (Byte)( ( NETWORK_PROTOCOL_ID ) & 0xFF );
        
        memcpy( &packet[4], data, len );
        
        ret = sendto( NetworkSocket, (char *)packet, len + 4 /* header */, 0, (struct sockaddr *)&from->Address, sizeof(from->Address) );
        
        if( ret == -1 ) {
            g_Core->p_Console->Print( LOG_ERROR, "SendPacket(): could not send packet to %s with size %d bytes\n", inet_ntoa(from->Address.sin_addr), len + 4 /* header too */ );
        }
    }
    
    /**
     *      Send packet.
     */
    void CNetwork::SendPacket( unsigned long len, const void * data, SQueueType * from )
    {
        size_t ret;
        Byte packet[MAX_MESSAGE_SIZE + 4]; // FIX MEEEEEE
        
        // First four bytes - protocol.
        packet[0] = (Byte)( NETWORK_PROTOCOL_ID >> 24 );
        packet[1] = (Byte)( ( NETWORK_PROTOCOL_ID >> 16 ) & 0xFF );
        packet[2] = (Byte)( ( NETWORK_PROTOCOL_ID >> 8 ) & 0xFF );
        packet[3] = (Byte)( ( NETWORK_PROTOCOL_ID ) & 0xFF );
        
        memcpy( &packet[4], data, len );
        
        ret = sendto( NetworkSocket, (char *)packet, len + 4 /* header */, 0, (struct sockaddr *)&from->address, sizeof(from->address) );
        
        if( ret == -1 ) {
            g_Core->p_Console->Print( LOG_ERROR, "SendPacket(): could not send packet to %s with size %d bytes\n", inet_ntoa(from->address.sin_addr), len + 4 /* header too */ );
        }
    }

    /**
     *      Print Out Of band message.
     */
    void CNetwork::PrintOutOfBand( ENetworkChannelType type, SNetdata * queue, const char * format, ... )
    {
        va_list		argptr;
        char		string[1024];
        
        // Set the header. ( so the client/server sees that ).
        string[0] = -1;
        string[1] = -1;
        string[2] = -1;
        string[3] = -1;
        
        va_start( argptr, format );
        vsprintf( string+4, format, argptr );
        va_end( argptr );
        
        // Send the data.
        if( queue->addressType == ENetAddressType::IP ) {
            SendPacket( strlen( string ), string, queue );
        } else if( queue->addressType == ENetAddressType::Loopback ) {
            SendLoopbackPacket( type, strlen(string), string );
        }
    }
//    
//    /**
//     *  Send out of band.
//     */
//    void CNetwork::PrintOutOfBandData( struct sockaddr_in * socket, Byte * format, int len ) {
//        Byte		string[MAX_SERVER_COMMAND * 2];
//        int			i;
//        SBitMessage   msg;
//        
//        // Set the header.
//        string[0] = 0xff;
//        string[1] = 0xff;
//        string[2] = 0xff;
//        string[3] = 0xff;
//        
//        for( i = 0; i < len; i++ ) {
//            string[i + 4] = format[i];
//        }
//        
//        msg.Data = string;
//        msg.Size = len + 4;
//        
//        // Send the data.
//        SendPacket( msg.Size, msg.Data, socket );
//    }
//    
    /**
     *  Compare socket addresses.
     */
    bool CNetwork::CompareAddress( const SNetdata *t1, const SNetdata *t2 )
    {
#   if defined( _WIN32 )
        return t1->Address.sin_addr.S_un.S_addr == t2->Address.sin_addr.S_un.S_addr;
#   else
        return t1->Address.sin_addr.s_addr == t2->Address.sin_addr.s_addr;
#   endif
    }
    
    /**
     *  Compare socket addresses.
     */
    bool CNetwork::CompareAddress( const sockaddr_in *t1, const sockaddr_in *t2 )
    {
#   if defined( _WIN32 )
        return t1->sin_addr.S_un.S_addr == t2->sin_addr.S_un.S_addr;
#   else
        return t1->sin_addr.s_addr == t2->sin_addr.s_addr;
#   endif
    }
    
    /**
     *  Check if address is on local machine.
     */
    bool CNetwork::IsLanAddress( SNetdata *adr )
    {
        //  127.0.0.1
        if( (adr->ip[0] == 127) &&
           (adr->ip[1] == 0) &&
           (adr->ip[2] == 0) &&
           (adr->ip[3] == 1) ) {
            return true;
        }
        
        return false;
    }
    
    /**
     *  Check if string address is a local type.
     */
    bool CNetwork::IsLanAddress( struct sockaddr_in *adr )
    {
        static char ipadr[INET_ADDRSTRLEN];
        inet_ntop( AF_INET, &adr->sin_addr, ipadr, INET_ADDRSTRLEN );
        
        if( ipadr[0] == '1' && ipadr[1] == '2' && ipadr[2] == '7' &&
           ipadr[4] /* skiping dots */ == '0' && ipadr[6] /* same */ == '0' && ipadr[8] /* uh */ == '1' ) {
            return true;
        }
        
        return false;
    }
    
    /**
     *  Is system address?
     */
    bool CNetwork::IsReservedAddress( SNetdata *adr ) {
        
        if( IsLanAddress( adr ) )  { // Loopback.
            return true;
        }
        
        if( (adr->ip[0] == 10) ||
           (adr->ip[0] == 127) ||
           (adr->ip[0] == 172 && adr->ip[1] >= 16 && adr->ip[1] <= 31) ||
           (adr->ip[0] == 192 && adr->ip[1] >= 168 ) ) {
               return true; // Reserved address.
        }
        
        return true;
    }
    
    /**
     *  Copy address.
     */
    void CNetwork::CopyAddress( SNetdata *addr, struct in_addr * socketAddr)
    {
        addr->ip[0] = (uint8_t)socketAddr->s_addr & 0xFF;
        addr->ip[1] = (uint8_t)( socketAddr->s_addr >> 8) & 0xFF ;
        addr->ip[2] = (uint8_t)( socketAddr->s_addr >> 16 ) & 0xFF;
        addr->ip[3] = (uint8_t)( socketAddr->s_addr >> 24 ) & 0xFF ;
    }
    
    /**
     *  Network address struct to a string type.
     */
    const char * CNetwork::AddressToString( SNetdata *adr )
    {
        static char address[64];
        
        NekoCString::SPrintf( address, sizeof(address), "%i.%i.%i.%i", adr->ip[0], adr->ip[1], adr->ip[2], adr->ip[3] );
        return address;
    }
    
    /**
     *  Compare netdata addresses.
     */
    bool CNetwork::CompareNetAddress( SNetdata *adr1, SNetdata *adr2 )
    {
        if( adr1->ip[0] == adr2->ip[0] &&
           adr1->ip[1] == adr2->ip[1] &&
           adr1->ip[2] == adr2->ip[2] &&
           adr1->ip[3] == adr2->ip[3] ) {
            return true;
        }
        
        return false;
    }
    
    /**
     *  Resolve address.
     */
    bool CNetwork::Resolve( SNetdata * address, const char * host )
    {
        g_Core->p_Console->Print( LOG_INFO, "Network::Resolve() - %s\n", host );
     
        //! Empty address string??
        if( host == NEKO_NULL ) {
            return false;
        }
        
        //! Too short.
        if( strlen(host) < 2 ) {
            return false;
        }
        
        struct hostent *hn;
        hn = gethostbyname( host );
        
        struct in_addr resolvedAddr;
        
        if( !hn ) {
            switch( h_errno ) {
                    // These are default.
                case HOST_NOT_FOUND:
                    g_Core->p_Console->Print( LOG_INFO, "Network::Resolve() - couldn't find a host.\n" );
                    return false;
                case NO_ADDRESS:
                    g_Core->p_Console->Print( LOG_INFO, "Network::Resolve() - host '%s' has no addresses.\n", host );
                    return false;
                case NO_RECOVERY:
                    g_Core->p_Console->Print( LOG_INFO, "Network::Resolve() - non-recoverable name server error.\n" );
                    return false;
                case TRY_AGAIN:
                    g_Core->p_Console->Print( LOG_INFO, "Network::Resolve() - host '%s' is temporarily unavailable.\n", host );
                    return false;
            }
        } else {
            // Get address from hostname data.
            resolvedAddr = *((struct in_addr *) hn->h_addr_list[0]);
            
            // Get address string.
            char adr[INET_ADDRSTRLEN];
            inet_ntop (AF_INET, ((struct in_addr *) &resolvedAddr), adr, INET_ADDRSTRLEN );
            
            g_Core->p_Console->Print( LOG_INFO, "Network::Resolve() - resolved as '%s'\n", adr );
            
            // Copy ip address.
            memcpy( (void *)&address->Address.sin_addr, hn->h_addr_list[0], hn->h_length );
            
            CopyAddress( address, &resolvedAddr );
            //address->port = -1; // Not assigned yet.
   
            return true;
        }
        
        // We'll never get here, so
        return true;
    }

    CNetwork::CNetwork() {
        NetworkSocket = -1;
    }
    
    /**
     *  Destructor.
     */
    CNetwork::~CNetwork() {
        
    }
    
    SNetdata::SNetdata( void )
    {
        this->Port = 0;
//        this->Socket = -1;
    }
    
    SNetdata::SNetdata( struct sockaddr_in *_sockaddress, unsigned int port )
    {
        Create( _sockaddress, port );
    }
    
    void SNetdata::Create( struct sockaddr_in * _sockaddress, unsigned int port )
    {
        this->Address = *_sockaddress;
        this->Port = port;
//        this->Socket = socket;
    }
}