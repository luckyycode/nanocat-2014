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
//  Network.cpp
//  Network manager..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef network_h
#define network_h

#include "../Core.h"
#include "../../Platform/Shared/SystemShared.h"
#include "../Streams/Streams.h" // Memory streams for networking.

namespace Neko {
#ifdef _WIN32 // Windows requires to start winsocket first.
    #define WINSOCKET_STARTUP_CODE       0x0101
#endif
    
    //!  Network protocol id.
    static const int32_t NETWORK_PROTOCOL_ID = 0x1337c0de;
    
    //!  Maximum packet size.
    static const int32_t MAX_UDP_PACKET_SIZE = 8192;
    
    //!   System network host name length.
    static const int32_t MAX_NET_HOSTNAME_LENGTH = 256;         // Network host name.
    
    //!  Maximum network clients.
    static const int32_t MAX_CLIENTS_NUM = 1024;
    
    //!  Maximum bind change tries.
    static const int32_t MAX_NETWORK_BIND_CHANGE = 16;
    
    //!  Network buffer size.
    const static uint32_t MAX_NETWORK_QUEUE = 200;
    const static uint32_t MAX_MESSAGE_SIZE = 4096;
    // In case if something fails.
//#define NETWORK_IGNORE_GETHOSTBYNAME
    
    const static uint32_t MAX_SERVERSIDE_QUEUE = 256;   //! Maximum loopback queue requests.
    const static uint32_t MAX_CLIENTSIDE_QUEUE = 256;   //! Same here.
    
    // Last raw received message.
    //extern byte network_message_buffer[MAX_UDP_PACKET];
    
    static const int32_t NETWORK_PROTOCOL_MINOR = 37;
    static const int32_t NETWORK_PROTOCOL_MAJOR = 13;
    static const int32_t NETWORK_PROTOCOL	= ( NETWORK_PROTOCOL_MAJOR << 16 ) + NETWORK_PROTOCOL_MINOR;
    
    ///   Network channel types.
    enum class ENetworkChannelType : int32_t  {
        ServerClient, // Server to client.
        ClientServer // Client to server.
    };
    
    /**
     *  Network address type.
     */
    enum class ENetAddressType : int32_t {
        // Bot.
        Bot,
        // Host user.
        Loopback,
        // User from 'outside'.
        IP
    }; // Network channel type.
    
    ///  Network queue.
    struct SQueueType
    {
        /*  Can queue be used? */
        bool active;
        /*  Queue raw address. */
        struct sockaddr_in address;
        /*  Queue size. */
        size_t size;
        /*  Queue message data. */
        char message[MAX_MESSAGE_SIZE];
    };
    
    ///  Network address data.
    struct SNetdata {
    public:
        
        SNetdata();
        SNetdata( struct sockaddr_in *_socketaddr, unsigned int port );
        
        /**
         *  Create a new netdata struct.
         *
         *  @param _socketaddr Raw  socket address.
         *  @param port        Port number.
         */
        void                Create( struct sockaddr_in *_socketaddr, unsigned int port );
        
        /**
         *  IP address.
         */
        uint8_t    ip[4];      // xxx.xxx.xxx.xxx, 255 max
        /**
         *  Port.
         */
        uint32_t Port;
        
        /**
         *  Address type ( i.e. loopback )
         */
        ENetAddressType addressType;
        
        /**
         *  Socket address.
         */
        struct sockaddr_in Address;
    };
    
    ///  Network channel.
    struct ncNetchannel
    {
        ENetworkChannelType type;
        SNetdata address;
        
        int32_t sequenceIn;
        int32_t sequenceOut;
    };
    
    ///  Network manager.
    class CNetwork
    {
        NEKO_NONCOPYABLE( CNetwork );
        
    public:
        CNetwork();
        ~CNetwork();
        
        /**
         *  Initialize networking.
         */
        void                Initialize( INekoAllocator * allocator );
        
        /**
         *  Network update.
         */
        void                Frame( void );
        
        /**
         *  Shutdown teh networking.
         */
        void                Shutdown( void );
        
        /**
         *  Assign packet to server or client.
         */
        void                Assign( /*SNetdata * from, const Byte * buffer, size_t bufferSize*/SQueueType * queue );
        
        /**
         *  Send raw packet.
         */
        void                SendPacket( unsigned long len, const void *data, SNetdata * from );
        
        /**
         *  Send raw packet.
         */
        void                SendPacket( unsigned long len, const void *data, SQueueType * from );
        
        /**
         *  Send a loopback packet.
         */
        void                SendLoopbackPacket( ENetworkChannelType type, unsigned long len, const void * data );
        
        /**
         *  Get a loopback packet.
         */
        Byte *              GetLoopbackPacket( ENetworkChannelType type );
        
        /**
         *  Print out of band.
         */
        void                PrintOutOfBand( ENetworkChannelType type, SQueueType * queue, const char * format, ... ) ;
        
        /**
         *  Print out of band.
         */
        void                PrintOutOfBand( ENetworkChannelType type, SNetdata * queue, const char * format, ... ) ;
        
        /**
         *  Empty queue.
         */
        void                EmptyQueue( SQueueType * queue, const uint32_t queueSize );
        
        /**
         *  Print out of band.
         */
//        void PrintOutOfBandData( struct sockaddr_in * socket, Byte * format, int len );
        
        /**
         *  Compare network addresses.
         */
        bool                CompareAddress( const SNetdata * t1, const SNetdata * t2 );
        
        /**
         *  Compare network addresses.
         */
        bool                CompareAddress( const sockaddr_in * t1, const sockaddr_in * t2 );
        
        /**
         *  Is it a local address?
         */
        bool                IsLanAddress( SNetdata *adr );
        
        /**
         *  Is it ( a string type ) local address?
         */
        bool                    IsLanAddress( struct sockaddr_in *adr );
        
        /**
         *  Is system address?
         */
        bool                IsReservedAddress( SNetdata * adr );
        
        /**
         *  Network address to a string type.
         */
        const char *                AddressToString( SNetdata * adr );
        
        /**
         *  Resolve address.
         */
        static bool                 Resolve( SNetdata *address, const char * host );
        
        /**
         *  Create network channel.
         */
        void                CreateChannel( ENetworkChannelType type, ncNetchannel *chan, SNetdata *adr );
        
        /**
         *  Copy address.
         */
        static void                 CopyAddress( SNetdata * addr, struct in_addr * socketAddr );
        
        /**
         *  Compare addresses.
         */
        bool                    CompareNetAddress( SNetdata * adr1, SNetdata * adr2 );
        
        /**
         *  Send message to channel.
         */
        void                SendMessageChannel( ncNetchannel *chan, SBitMessage *msg );
        
        /**
         *  Process channel data.
         */
        bool                ProcessChannel( ncNetchannel *chan, Byte *packet );

    private:
      
        bool        m_bRunning;
        
        INekoAllocator  * pAllocator = 0;
        INekoAllocator  * pAllocatorHandle = 0;
        
        //!  Current network socket.
        uint32_t NetworkSocket;
        
        //!  Network addresses.
        struct sockaddr_in DataAddr, RecvAddr;

        
        //!  Network packet queue.
        SQueueType* m_networkMessageQueue;
        
        //!  Loopback server packet queue.
        SQueueType* m_loopbackServerQueue;
        
        //!  Loopback client packet queue.
        SQueueType* m_loopbackClientQueue;
        
        //!  Total queues for loopback client queue.
        int32_t m_loopbackTotalClientQueues;
        
        //!  Total queues for loopback server queue.
        int32_t m_loopbackTotalServerQueues;
        
        //!  Network queue locations.
        int32_t m_nextQueueLocation, m_nextMessageForProcessing;
     
    };
    
    // NETWORK
    extern SConsoleVar       * Network_Port;                         // Network port.
    extern SConsoleVar       * Network_IPAddress;                    // Network ip address.
    extern SConsoleVar       * Network_LocalIPAddress;               // Local Device IP Address.
}

#endif
