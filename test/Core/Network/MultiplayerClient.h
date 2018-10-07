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
//  MultiplayerClient.h
//  Game client..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef client_h
#define client_h

#include "Network.h"
#include "../../Math/GameMath.h"
#include "Multiplayer.h"
#include "PlayerEntity.h"

namespace Neko {
    /**
     *  Response repeat interval.
     */
    static const int32_t RESPONSE_REPEAT = 1000;
    
    /**
     *  User command backup size.
     */
    static const int32_t USER_CMD_BACKUP = 64;
    
    /**
     *  User command queu size.
     */
    static const int32_t USER_CMD_QUEUE_SIZE = 256;
    
    /**
     *  Snapshot base size.
     */
    static const int32_t BASE_PACKET_SIZE = 2048;
    
    /**
     *  Maximum client reliable commands.
     */
    static const uint32_t MAX_RELIABLE_MESSAGES = 64;
    
    /**
     *  Client state.
     */
    enum ClientState_t
    {
        // Not connected to any server.
        CLIENT_IDLE             = 0x1,
        // Trying to connecting to server
        CLIENT_PRECONNECTING    = 0x2,
        // No information from server for a long time.
        CLIENT_NORESPONSE       = 0x4,
        // Client is connecting.
        CLIENT_CONNECTING       = 0x8,
        // Client is connected.
        CLIENT_CONNECTED        = 0x16,
        // Client is active and in game.
        CLIENT_INGAME           = 0x32
    };    // Local client state.
    
    /**
     *  Multiplayer client.
     */
    class ncClient
    {
        NEKO_NONCOPYABLE( ncClient );
        
    public:
        
        ncClient();
        ~ncClient();
        
        /**
         *  Disconnect.
         */
        void Disconnect( void );
        
        /**
         *  Initialize client.
         */
        void Initialize( INekoAllocator * allocator );
        
        /**
         *  Send acknowledge command.
         */
        void SendCommand( const char * command );
        
        /**
         *  Check for timeout.
         */
        void CheckForTimeout( void );
        
        /**
         *  Connect to server.
         */
        void Connect( const char * ipaddr, const uint32_t port );

        /**
         *  Predict player.
         */
        void PredictEntity();
        
        /**
         *  Process acknowledge commands.
         */
        void ReadServerCommands( SBitMessage *msg );
        
        /**
         *  Process server frame.
         */
        void ReadServerFrame( SBitMessage * msg );
        
        /**
         *  Set current snapshot.
         */
        void SetLocalSnapshot();
        
        /**
         *  Process initializing state message.
         */
        void ReadClientInit( SBitMessage * msg );
        
        /**
         *  Send command packet.
         */
        void SendCommands( void );
        
        /**
         *  Create client command.
         */
        void CreateUserCommand();
        
        /**
         *  Check commands to be sent.
         */
        void CheckForCommands( const uint32_t time );
        
        /**
         *  Parse incoming commands.
         */
        void ParseCommands( SNetdata *from, SBitMessage *buffer );
        
        /**
         *  Connectionless packet parser.
         */
        void Connectionless( SNetdata *from, Byte * data );
        
        /**
         *  Client update.
         */
        void Process( SNetdata *from,  Byte *buffer );
        
        /**
         *  Reconnect.
         */
        void Reconnect( void );
        
        /**
         *  Disconnect.
         */
        void DisconnectForced( const char * msg, bool forced );
        
        /**
         *  Change client name.
         */
        void ChangeName( void );
        
        /**
         *  Check connect commands.
         */
        void CheckForConnect( void );

        /**
         *  Client update.
         */
        void Frame( uint32_t msec );
        
        /**
         *  Get user command.
         */
        UserCommand * GetCommand();
        
        /**
         *  Get user command at index.
         */
        UserCommand * GetCommand( int32_t index );
        
        /**
         *  Shutdown the client.
         */
        void Shutdown();
        
        void Say( void );
        
        /**
         *  Client state.
         */
        ClientState_t       State;
        
        /**
         *  Server-client channel.
         */
        ncNetchannel        Channel;
        
        /**
         *  Current server.
         */
        SNetdata           CurrentServer;
        
        /**
         *  Client name.
         */
        char                Name[18];
        
        /**
         *  Are we the current server?
         */
        bool                m_isHost;

        INekoAllocator      * pAllocator = 0;
        INekoAllocator      * pAllocatorHandle = 0;
        
        // .. Uh oh, a lot of integers.
        
        /**
         *  Server identity.
         */
        int32_t                ServerId;    // Server session id.
        uint32_t                ClientNum;  // Client id on the server.
        int32_t                 Response;       // Last server response.
        
        uint32_t                 TimeBase;      // Server delta time.
        uint32_t                 Frametime;     // Client frametime.
        uint32_t                 Time;          // Client time.

        int32_t                 LastServerMessage;  // Last server message.
        int32_t                 LastMessageReceivedAt;  // Last message time.
        int32_t                 LastExecutedReliableAt;    // Last executed reliable message at.
        int32_t                 LastReliableMessage;     // Last reliable message sent at.
        int32_t                 LastConnectPacketTime;      // Last connect request packet send time.
        
        int32_t                 ReliableAcknowledged;        // Reliable commands acknowledged.
        int32_t                 ReliableSequence;            // Reliable sequence id.

        uint32_t                 TimeSinceLastPacket;   // Time since the last packet was recieved.
        uint32_t                PreviousFrameTime;      // Last snapshot time.
        
        int32_t                 userCommandNum;         // Total user commands.
        int32_t                 userCommandQueueCount;
        
        const char          * ReliableCommands[MAX_RELIABLE_MESSAGES];   // Client reliable commands.

        UserCommand         * userCommandBackup;         // User commands history.
        UserCommand         * userCommandQueue;          // User commands queue.

        SBitMessage        **m_entityStates;            // Server entity states.
        
        Snapshot * m_pLastSnapshot;       // Last entity snapshot.
        Snapshot * m_curSnapshot;        // Current snapshot.
        Snapshot * m_localSnapshot;           // Local player entity snapshot.
        
        Snapshot * m_snapToInterpolate;
        
        Snapshot ** m_snapHistory;    // Snapshot history.
        int32_t m_snapHistorySize;    // Snapshot history size.

        Entity  **m_entityData, **m_snapshotEntityData; // Server entities.
        
        SBitMessage    * clientStaticStream;
        
        bool    m_bInitialized;
        bool    m_bHasWorld;
        
        bool    m_bRunning;
    };
    
    
    // CLIENT
    extern SConsoleVar      * NameVar;                               // Client name.
    extern SConsoleVar      * Client_ServerTimeout;                  // In seconds.
    extern SConsoleVar      * Client_MaximumPackets;                     // Max client packets per frame to be sended.
}

#endif
