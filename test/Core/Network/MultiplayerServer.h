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
//  MultiplayerServer.h
//  Game server..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef server_h
#define server_h

#include "Network.h"
#include "../../Math/GameMath.h"
#include "PlayerEntity.h"
#include "Multiplayer.h"
#include "../Utilities/List.h"
#include "ServerWorld.h"

namespace Neko {
    
    //!  Total reliable messages.
    static const int32_t MAX_RELIABLESERVERMESSAGE = 1024;
    
    //!  Snapshot size.
    static const int32_t MAX_SNAPSHOT_SIZE = 8192;
    
    //!  Client authorize time limit.
    static const int32_t SERVER_CLIENTAUTHORIZE_TIME = 1000;
    
    //!  Default server-client amount.
    static const int32_t SERVER_DEFAULT_CLIENTNUM = 24;
    
    //!  Server client bases.
    const static uint32_t SERVERCLIENT_BASES = 32;
    
    //!  Entity states.
    const static uint32_t ENTITYSTATE_SIZE = 32; // Should be able to customize.
    
    //!  Server packet delay.
    const static uint32_t MAX_SERVERPACKET_DELAY = 28;
    
#define DEFAULT_SERVER_NAME "Meow server"

    ///  Serverclient state.
    enum class EServerClientState
    {
        Free    = 0,
        Connecting,
        Connected ,
        Authorizing ,
        Ghost ,
        Active
    };
    
    ///   Server state.
    enum class EServerState
    {
        Idle     = 0,
        Loading,
        EAuthorize ,
        Error ,
        Game ,
        Locked // Locked, clients are not able to connect.
    };
    
    /**
     *  Max server responses.
     */
    static const int32_t MAX_RESPONSES = 1024;
    
    struct SServerClientEntityState;
    
    ///  Server response data for each client.
    struct SServerResponseData
    {
        int32_t			Response;
        
        int32_t			PingAt;
        int32_t			FirstAt;
        
        int32_t			Time;
        
        bool	    Connected;
        SNetdata   Address;
    };
    
    /// ============================================================================================
    
    ///   Server client!
    class CServerClient
    {
    public:
        
        /**
         *  Create new server client.
         */
        CServerClient();
        
        /**
         *  Initialize client entity.
         */
        void                    CreateEntity();
        
        /**
         *  Remove entity.
         */
        void                    RemoveEntity();
        
        /**
         *  Process command.
         */
        void                    UpdateEntity( UserCommand * command );
        
        /**
         *  Called every frame to update the client.
         */
        void                    FrameUpdate(    SBitMessage * message );
        
        //!  Server client name string.
        char                    name[18];
        char                    version[2]; //!\ version
        
        
        //!  Server client address.
        SNetdata               address;
        
        //!  Client state.
        EServerClientState             state;
        
        //!  Network address type ( bot, local, internet ).
        ENetAddressType            type;
        
        //!  Server-client channel.
        ncNetchannel                channel;

        //!  Client num.
        int32_t                     clientnum;
        
        //!  Response id.
        int32_t                     response;
        
        int32_t                     lastRenewStateMessage;
        int32_t                     lastMessageTime;    // Last message time.
        int32_t                     lastConnectTime;    // A time when client got connected.
        int32_t                     lastReceivedCommand;
        int32_t                     lastExecutedCommand;
        int32_t                     lastMessageReceivedAt;
        int32_t                     lastAcknowledgedMessageAt;
        int32_t                     cachedReliableCommandAt;
        
        char                    * clientCommands[MAX_RELIABLESERVERMESSAGE];
        
        uint32_t                    commandSequence;
        uint32_t                    commandsAcknowledged;
        
        int32_t                     ghostifiedAt;
        
        int32_t                     currentPing;
        
        PlayerEntity                * m_pEntity;
        SServerClientEntityState      ** EntityState;
        
        SLink    m_Link;
    };
    
    ///  Server client entity base.
    struct SServerClientEntityState
    {
        /**
         *  Constructor.
         */
        SServerClientEntityState();
        
        /**
         *  Destructor.
         */
        ~SServerClientEntityState();
        
        /**
         *  Constructor with a client handle.
         */
        SServerClientEntityState( CServerClient * client );
        
        
        /**
         *  Get server client's entity base.
         */
        inline SBitMessage * Get() {
            return &Bases[Client->lastAcknowledgedMessageAt % ENTITYSTATE_SIZE];
        }
        
        /**
         *  Set server cl\ient entity base.
         */
        inline void Set( SBitMessage *newBase ) {
            Bases[Client->channel.sequenceOut % ENTITYSTATE_SIZE] = *newBase;
        }
        
        //!  Client handle.
        CServerClient  * Client;
        
        //!  Bit message packet handle.
        SBitMessage    * Bases;
        
        //!  Was it used?
        bool    Used;
        
    };
    
    /// ============================================================================================
    
    ///  Multiplayer server!
    class CServer
    {
        NEKO_NONCOPYABLE( CServer );
        
    public:
        
        /**
         *  Create new server.
         */
        CServer();
        
        /**
         *  Initialize server!
         */
        void                Initialize( INekoAllocator * allocator );
        
        /**
         *  Create server session.
         */
        void                CreateSession( void );
        
        /**
         *  Prepare client slots.
         */
        void                SetupClients( const uint32_t maxclients );
        
//        void Maprestart( void );
        
        /**
         *  Process client messages.
         */
        void                ReadClientMessage( CServerClient * client, SBitMessage *msg );
        
        /**
         *  Process user(client) commands.
         */
        void                ReadUserCommands( CServerClient * client, SBitMessage * msg );
        
        /**
         *  Create client.
         */
        void                CreateClient( SNetdata * from, int response, const char * name, const char * version );
        
        /**
         *  Parse connectionless packets.
         */
        void                Connectionless( SNetdata * from,  Byte * data );
        
        /**
         *  Parse clients.
         */
        void                ParseClients( SNetdata * from, SBitMessage * packet );
        
        /**
         *  Send frame to clients.
         */
        void                UpdateClient( CServerClient * cl );
        void                UpdateClients( void );
        
        /**
         *  Server incoming packet parser.
         */
        void                Process( SNetdata * from,  Byte * buffer );
        
        /**
         *  Check server parameters.
         */
        void                CheckParams( void );
        
        /**
         *  Clear world.
         */
        void                ClearWorld( const char * msg );
        
        /**
         *  Print server info.
         */
        void                PrintInfo( void );
        
        /**
         *  Check for client timeouts.
         */
        void                CheckTimeouts( void );
        
        /**
         *  Server update.
         */
        void                Frame( int msec );
        
        /**
         *  Update server world.
         */
        void                UpdateServerWorld( int32_t msec );
        
        /**
         *  Disconnect client.
         *  @note - Leave message empty if there's no reason.
         */
        void                DisconnectClient( CServerClient * client, const char * message );
        
        /**
         *  Get response from client address ( connectionless ).
         */
        void                GetResponse( SNetdata * from );
        
        /**
         *  Check for bad clients.
         */
        void                CheckGhosts( void );
        
        /**
         *  Print status.
         */
        void                PrintStatus( void );
        
        /**
         *  Server end message ( last one ).
         */
        void                SendByeMessage( const char * msg );
        
        /**
         *  Send initialization state.
         */
        void                SendNewState( CServerClient * client );
        
        /**
         *  Kick client.
         */
        void                KickClient( CServerClient * client, const char * message );
        
        /**
         *  Add acknowledge command.
         */
        void                AddReliableCommand( CServerClient * cl, bool isDisconnect, char * message );
        
        /**
         *  Send acknowledge command.
         */
        void                SendCommand( CServerClient * cl, bool isDisconnect, const char * cmd, ... );
        
        /**
         *  Remove available bots.
         */
        void                RemoveBots( void );
        
        /**
         *  Disconnect server ( mostly shutdownin' ).
         */
        void                Disconnect( void );
         
        /**
         *  Shutdown server.
         */
        void                Shutdown( const char * finalmsg );
        
        /**
         *  Add server bot.
         */
        void                AddBot( void );
        
        
        /**
         *  Get server-client by number.
         */
        CServerClient *                 GetClientByNum( int num );
        
        /**
         *  Get server-client by address.
         */
        CServerClient *                 GetClientByAddress( const sockaddr_in * data );
        
        
        INekoAllocator  *   pAllocatorHandle = 0;
        INekoAllocator  *   pAllocator = 0;
        
        
        //!  Is server initialized?
        bool            m_bInitialized = false;

        //!  Server Id.
        int32_t             ServerIdentificator;
        
        //!  Last frame update time ( ms ).
        int32_t             LastTime;
        
        //!  Server frametime.
        int32_t             frameMsec;
        
        //!  Server time.
        int32_t             Time;
        
        //!  Last information print time.
        int32_t             LastInfoPrintTime;
        
        //!  Entity spawn key state.
        int32_t             EntitySpawnId;
        
        
        //!  Server state.
        EServerState                 m_State;
        
        //!  Server clients.
        SList              m_Clients;
        
        //!  Server response data.
        SServerResponseData	    Response[MAX_RESPONSES];
        
        //!  Server world entity world.
        SList           m_Entities;
        
        
        //! Server world!
        CServerWorld        * p_ServerWorld;
        
        bool        m_bRunning;
    };
    
    
    // SERVER
    extern SConsoleVar       * Server_Name;                       // Server name.
    extern SConsoleVar       * Server_Maxclients;                     // Maximum server clients.
    extern SConsoleVar       * Server_Fun;                         // Can server clients use cheat commands?
    extern SConsoleVar       * Server_Maxfps;                            // Server fps.
    extern SConsoleVar       * Server_Sayname;                        // What name server should have in the chat?
    extern SConsoleVar       * Server_Dedicated;                           // Is server dedicated?
    

}

#endif
