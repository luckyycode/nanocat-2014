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
//  SoundManager.h
//  Sound loader and manager. ;D
//
//  Created by Neko Vision on 22/12/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//


#ifndef __sound_h__
#define __sound_h__

#include "../../Core/CoreDef.h" // non-copyable define.
#include "../../Core/String/String.h"
#include "../../Core/Utilities/Hashtable.h"
#include "../../Core/Utilities/VectorList.h"
#include "../../Math/Vec3.h"
#include "../../Platform/Shared/SystemShared.h"


//
// Sound manager.
//

#   if !defined( NEKO_SERVER )
namespace Neko {
    
    //!  Max sounds allowed to load.
    static const int32_t MAX_SOUNDS = 64;
    
    //!  Max sounds allowed to load.
    static const int32_t MAX_SOUNDENTS = 64;
    
    //!  Max channel sources.
    static const int32_t MAX_CHANNEL_SOURCES = 6;

    //!  Max sound name length.
    static const int32_t MAX_SOUNDNAME_LENGTH = 18;
    
    class   CSoundManager;
    
    ///  WAVE sound.
    struct WaveSound
    {
        char    type[4];
        
        int32_t     size, chunkSize;
        int32_t     sampleRate, avgBps;
        int32_t     dataSize;
        
        int16_t   bytesPerSample, bitsPerSample;
        int16_t   formatType, channels;
        
        Byte    * soundBuffer;
    };
    
    ///  Simple sound struct.
    struct CSound
    {
        CStr       m_Name;
        uint32_t    m_Buffer;
        double      m_Duration;
    };
    
    ///  Sound entity struct.
    struct CSoundEnt
    {
        /**
         *  Default constructor.
         */
        CSoundEnt() : m_SoundPtr( NEKO_NULL ), m_vOrigin( 0.0f ), dStartTime( 0.0 ), fVolume( 1.0f ), bLoops( false )
        {
            
        }
        
        // Pointer to the original sound.
        CSound          * m_SoundPtr;
        
        // Sound entity origin.
        Vec3   m_vOrigin;
        
        // Sound source.
        uint32_t        m_iSource;
        
        // Volume.
        float           fVolume;
        
        // Looping sound?
        bool            bLoops;
        
        // Play times.
        double          dStartTime;
        double          dEndTime;
    };

    ///   Sound manager.
    class CSoundManager
    {
        NEKO_NONCOPYABLE( CSoundManager );
        
    public:
        
        /**
         *  Constructor.
         */
        CSoundManager();
        
        /**
         *  Destructor.
         */
        ~CSoundManager();
        
        /**
         *  Initialize sound system.
         */
        void                    Initialize( INekoAllocator * allocator );
        
        /**
         *   Shutdown sound system.
         */
        void                    Shutdown();
        
        /**
         *   Unload all sounds.
         */
        void                    UnloadAllSounds();
        
        /**
         *   Find sound by name.
         */
        CSound  *               FindSound( const char * name );
        
        // sound,wav,ocean,1.0,1.0,false,false,oceanwave
        /**
         *  Load a sound with properties.
         */
        void                LoadSound( const char * name );

        /**
         *  Load WAVE sound.
         */
        void                LoadSoundWav( const char * soundname, WaveSound *wav );
        
        /**
         *  Spawn sound at desired position. 
         */
        CSoundEnt *                 PlaySoundAt( const char * name, const Vec3& origin, const float volume, bool looping, bool distanceCare );
        
        /**
         *   Update listener position and rotation.
         */
        void                UpdateListener( uint32_t time );

    private:
        
        INekoAllocator  * pAllocator = 0;
        INekoAllocator  * pEntAllocator = 0;
        
        INekoAllocator  * pAllocatorHandle = 0;
        
        //!  OpenAL current sound device.
        ALCdevice    * pDevice;
        
        //!  OpenAL context.
        ALCcontext   * pContext;
        
        //!  Sound cache.
        CHashMap<const char*, CSound*>  m_soundCache;
        CVectorList<CSoundEnt*>    m_soundEnts;
        
        //!  Is sound system initialized?
        bool    bInitialized;
        
        //!  Do we need to update the listener?
        bool    bUpdateListener;
        
        //!  Listener entity volume.
        float   fListenerVolume;
        
        //!  Sound system time.
        double  dTime;
    };
}
#   endif

#endif
