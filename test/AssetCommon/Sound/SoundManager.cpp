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
//  SoundManager.cpp
//  Sound manager and loader.. ;D
//
//  Created by Neko Vision on 22/12/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//


// @todo - needs to be improved and updated

#include "SoundManager.h"
#include "../../Core/Core.h"
#include "../FileSystem.h"
#include "../AssetBase.h"
#include "../../Core/Player/Camera/Camera.h"


#include "../../Core/Streams/Streams.h" // MEM_fread
#include "../../Core/Streams/MemoryStream.h"

#   if !defined( NEKO_SERVER )

namespace Neko {

    /**
     *  New sound manager instance.
     */
    CSoundManager::CSoundManager()
    {
        bInitialized = false;
        bUpdateListener = false;

        //mSoundBuffers = NEKO_NULL;

        pDevice = NEKO_NULL;         // OpenAL device.
        pContext = NEKO_NULL;        // OpenAL context.
    }
    
    /**
     *  Get OpenAL error string for a code.
     */
    static const char * GetErrorString( int32_t err )
    {
        switch ( err ) {
            case AL_NO_ERROR:
                return "AL_NO_ERROR";
            
            case AL_INVALID_NAME:
                return "AL_INVALID_NAME";
                
            case AL_INVALID_ENUM:
                return "AL_INVALID_ENUM";
                
            case AL_INVALID_VALUE:
                return "AL_INVALID_VALUE";
                
            case AL_INVALID_OPERATION:
                return "AL_INVALID_OPERATION";
                
            case AL_OUT_OF_MEMORY:
                return "AL_OUT_OF_MEMORY";
                
            default:
                return "No such error code";
        }
    }

    /**
     *  Initialize sound system.
     */
    void CSoundManager::Initialize( INekoAllocator * allocator )
    {
        if( !g_Core->UsesGraphics() ) {
            return;
        }
     
        if( Skip_AudioEngine->Get<bool>() ) {
            g_Core->p_Console->Print( LOG_INFO, "Skipping sound system engine load..\n" );
            return;
        }
        
        g_Core->p_Console->Print( LOG_INFO, "Initializing sound system...\n" );
        
        if( pDevice != NEKO_NULL ) {
            return;
        }
        
        pAllocatorHandle = allocator;
        
        
        // TODO: sometimes engine stucks at alcOpenDevice
        
        // Create OpenAL device.
        pDevice = alcOpenDevice( NEKO_NULL );
        if( pDevice == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "Could not initialize OpenAL: \"%s\".", GetErrorString( alGetError() ) );
            return;
        }

        // Create OpenAL Context.
        pContext = alcCreateContext( pDevice, NEKO_NULL );

        if( pContext == NEKO_NULL || alcMakeContextCurrent(pContext) == ALC_FALSE ) {
            if( pContext != NEKO_NULL ) {
                alcDestroyContext( pContext );
            }

            alcCloseDevice( pDevice );
            g_Core->p_Console->Error( ERR_FATAL, "Could not create OpenAL context.");

            return;
        }
        
        // Check for some errors.
        if( alGetError () != AL_NO_ERROR ) {
            g_Core->p_Console->Error( ERR_FATAL, "OpenAL error: %i\n", alGetError() );
        }

        // Create pools.
        pAllocator = NekoAllocator::newPoolAllocator( sizeof(CSound), __alignof(CSound), kSoundCachePoolSize, *pAllocatorHandle );
        pEntAllocator = NekoAllocator::newPoolAllocator( sizeof(CSoundEnt), __alignof(CSoundEnt), kSoundCachePoolSize / 2, *pAllocatorHandle );
        
        // Some infos.
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "OpenAL initialized. Sound device: \"%s\"\n", alcGetString( pDevice, ALC_DEVICE_SPECIFIER ) );
        g_Core->p_Console->Print( LOG_INFO, "Used memory to create cache pools: %llu mb\n", ByteInMegabyte(kSoundCachePoolSize) );
        g_Core->p_Console->Print( LOG_NONE, "\n" );

        // Create sound cache.
        if( !m_soundCache.Create( MAX_SOUNDS, pAllocator, pClassLinearAllocator ) ) {
            g_Core->p_Console->Print( LOG_INFO, ";;;\n" );
        }

        // Create sound entity cache.
        m_soundEnts.Create( MAX_SOUNDENTS, pLinearAllocator2 );
        
        UpdateListener( 0 );
        fListenerVolume = 0.1f;
        
        bUpdateListener = true;
        bInitialized = true;
    }

    /**
     *  Update listener preferences.
     */
    void CSoundManager::UpdateListener( uint32_t time )
    {
        if( bUpdateListener == false ) {
            return;
        }
        
        this->dTime = (double)time;
        
        uint32_t    i;
        CSoundEnt   * ent;

        // Listener origin.
        ALfloat ListenerOri[] =
        {
            g_Core->p_Camera->vLook.x, g_Core->p_Camera->vLook.y, g_Core->p_Camera->vLook.z,
            g_Core->p_Camera->vUp.x, g_Core->p_Camera->vUp.y, g_Core->p_Camera->vUp.z
        };
        
        // Preferences.
        alListenerf( AL_GAIN, fListenerVolume );

        // Position.
        alListener3f( AL_POSITION, g_Core->p_Camera->vEye.x, g_Core->p_Camera->vEye.y, g_Core->p_Camera->vEye.z );
        
        // Rotation.
        alListenerfv( AL_ORIENTATION, ListenerOri );
        alListener3f( AL_VELOCITY, 0.0, 0.0, 0.0 );
        
        
    updateSnd:
        
        // Sound entities.
        for( i = 0; i < m_soundEnts.GetCount(); ++i )
        {
            ent = m_soundEnts[i];
            
            if( ent == NEKO_NULL ) {
                continue;
            }
            
            // Check for the end time.
            if( (ent->bLoops == false && (ent->dEndTime + 1000.0 /* give it a bit of time */ ) < this->dTime) || ent->dEndTime == 0.0 ) {
                alSourceStop( ent->m_iSource );
                alDeleteSources( 1, &ent->m_iSource );
                
                pEntAllocator->Dealloc( ent );
                
                m_soundEnts.Delete( i );
                
                goto updateSnd;
            }
            
            alSourcef( ent->m_iSource, AL_GAIN, ent->fVolume );
            alSourcei( ent->m_iSource, AL_LOOPING, ent->bLoops ? AL_TRUE : AL_FALSE );
            
            // Sound entity origin.
            alSource3f( ent->m_iSource, AL_POSITION, ent->m_vOrigin.x, ent->m_vOrigin.y, ent->m_vOrigin.z );
        }
    }

    static int32_t  last_soundSize;
    static int32_t  last_soundChannels;
    static int32_t  last_soundBits;
    static int32_t  last_soundFrequency;
    
    /**
     *  Load the sound.
     */
    void CSoundManager::LoadSound( const char * name )
    {
        CSound * sound = NEKO_NULL;
        
        // Check if we already have got sound loaded.
        sound = FindSound( name );
        if( sound != NEKO_NULL ) {
            return;
        }
        
        static WaveSound snd;
        
        double duration;
        int32_t lengthInSamples;
        
        memset( &snd, 0x00, sizeof(WaveSound) );
        
        // Load wave sound.
        LoadSoundWav( name, &snd );
        
        ALuint format = 0;
        
        // Check for a sound format.
        if( snd.bitsPerSample == 8 ) {
            if( snd.channels == 1 ) {
                format = AL_FORMAT_MONO8;
            } else if( snd.channels == 2 ) {
                format = AL_FORMAT_STEREO8;
            }
        } else if( snd.bitsPerSample == 16 ) {
            if( snd.channels == 1 ) {
                format = AL_FORMAT_MONO16;
            } else if( snd.channels == 2 ) {
                format = AL_FORMAT_STEREO16;
            }
        }
        
        // Create a new sound struct.
        sound = NekoAllocator::AllocateNew<CSound>(pAllocator);// (CSound *)pAllocator->Alloc( sizeof(CSound) ) ;
        
        // Create buffer now.
        alGenBuffers( 1, &sound->m_Buffer );
        alBufferData( sound->m_Buffer, format, snd.soundBuffer, snd.dataSize, snd.sampleRate );
        
        // Sound properties.
        alGetBufferi( sound->m_Buffer, AL_SIZE, &last_soundSize );
        alGetBufferi( sound->m_Buffer, AL_CHANNELS, &last_soundChannels );
        alGetBufferi( sound->m_Buffer, AL_BITS, &last_soundBits );
        alGetBufferi( sound->m_Buffer, AL_FREQUENCY, &last_soundFrequency );
        
        lengthInSamples = last_soundSize * 8 / (last_soundChannels * last_soundBits);
        duration = (lengthInSamples / (float)last_soundFrequency);
        
//        duration = (uint32)((1000.0f * (float)m_dwDataSize / (float)nAvgBytesPerSec) + 0.5f);
        
        sound->m_Name = name;
        sound->m_Duration = duration;
        
        m_soundCache[name] = sound;
    }

    /**
     *  Spawn sound at desired position with parameters.
     */
    CSoundEnt *CSoundManager::PlaySoundAt( const char * name, const Vec3& origin, const float volume, bool looping, bool distanceCare )
    {
        CSound      * snd;
        CSoundEnt   * ent = NEKO_NULL;
        
        g_Core->p_Console->Print( LOG_INFO, "Spawning sound \"%s\" at ( %4.2f, %4.2f, %4.2f )\n", name, origin.x, origin.y, origin.z );

        // Check if we have this sound loaded.
        snd = FindSound( name );
        if( snd == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_WARN, "SpawnSoundAt() - Unable to spawn \"%s\" sound.\n", name );
            return NEKO_NULL;
        }
        
        // Create a new sound entity.
        ent = (CSoundEnt*)pEntAllocator->Alloc( sizeof(CSoundEnt) );
        
        if( ent == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        // Add a new entity to the list.
        m_soundEnts.PushBack( ent );
        
        ent->fVolume = volume;
        ent->bLoops = looping;
        ent->m_vOrigin = origin;
        
        ent->m_SoundPtr = snd;
        
        ent->dStartTime = this->dTime;
        ent->dEndTime = ent->dStartTime + ent->m_SoundPtr->m_Duration;
        
        // Check if we have vaild AL source.
        if( alIsSource( ent->m_iSource ) == AL_FALSE ) {
            // Create OpenAL source.
            alGenSources( 1, &ent->m_iSource );
        }
        
        // Set sound origin.
        alSource3f( ent->m_iSource, AL_POSITION, ent->m_vOrigin.x, ent->m_vOrigin.y, ent->m_vOrigin.z );
        // Buffer handle.
        alSourcei( ent->m_iSource, AL_BUFFER, ent->m_SoundPtr->m_Buffer );
        // Is looping?
        alSourcei( ent->m_iSource, AL_LOOPING, ent->bLoops ? AL_TRUE : AL_FALSE );
        // Sound volume.
        alSourcef( ent->m_iSource, AL_GAIN, ent->fVolume );
        
        // Play the sound.
        alSourcePlay( ent->m_iSource );

        return ent;
    }

    /**
     *  Get sound by name.
     */
    CSound *CSoundManager::FindSound( const char * sSound )
    {
        if( sSound == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
        CSound * cachedSound = m_soundCache[sSound];
        
        if( cachedSound != NEKO_NULL && cachedSound->m_Buffer ) {
            return cachedSound;
        }

        return NEKO_NULL;
    }

    /**
     *  Shutdown sound system.
     */
    CSoundManager::~CSoundManager()
    {
        Shutdown();
    }
    
    
    /**
     *  Unload all sounds.
     */
    void CSoundManager::UnloadAllSounds()
    {
        g_Core->p_Console->Print( LOG_INFO, "Unloading all sounds...\n" );
        
        CSound * snd = NEKO_NULL;
        
        SLink * head;
        SLink * cur;
        
        head = &m_soundCache.m_List.m_sList;
        
        for ( cur = head->m_pNext; cur != head; cur = cur->m_pNext ) {
            snd = (CSound *)cur->m_ptrData;
            
            alDeleteBuffers( 1, &snd->m_Buffer );
            
            pAllocator->Dealloc(snd) ;
            snd = NEKO_NULL;
        }
        
        // Delete sound cache.
        m_soundCache.Delete();
        m_soundEnts.ClearAll();
    }
    
    
    /**
     *  Shutdown sound system.
     */
    void CSoundManager::Shutdown()
    {
        if( !bInitialized && pDevice != NEKO_NULL ) {
            return;
        }

        //int i;

        g_Core->p_Console->Print( LOG_INFO, "Sound base unitializing..\n"/*"Unloading sounds..\n"*/ );

        UnloadAllSounds();

        // Turn off current context.
        alcMakeContextCurrent( NEKO_NULL );
        // Destroy context.
        alcDestroyContext( pContext );
        // Close device.
        alcCloseDevice( pDevice );

        bInitialized = false;

        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pEntAllocator, pAllocatorHandle );
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pAllocator, pAllocatorHandle );
        
        g_Core->p_Console->Print( LOG_INFO, "OpenAL uninitialized.\n" );
    }

    /**
     *  Load WAVE sound.
     */
    void CSoundManager::LoadSoundWav( const char * soundname, WaveSound *wav )
    {
        // Find a package with all our sounds and get a sound we need.
        SPackFile * soundInfoPak = g_Core->p_FileSystem->GetPak( "sounds" );
        AssetDataPool soundData = soundInfoPak->GetData( NC_TEXT( "%s.wav", soundname )  );
        
        // Check if we got anything.
        if( soundData.tempData == NEKO_NULL ) {
            return;
        }
        
        ncMemoryStream * stream = (ncMemoryStream*)PushMemory( soundData.tempPool, sizeof(ncMemoryStream ) );// new ncMemoryStream( &soundData.tempData );
        stream->SetSourceBuffer( &soundData.tempData );
        
        stream->Read( wav->type, sizeof(char), 4 );
        if( wav->type[0] != 'R' || wav->type[1] != 'I' || wav->type[2] != 'F' || wav->type[3] != 'F' ) {
            // RIFF
            g_Core->p_Console->Error( ERR_ASSET, "Couldn't load \"%s\" sound file, no RIFF.", soundname );
        }

        stream->Read( &wav->size, sizeof(int), 1 );
        stream->Read( wav->type, sizeof(char), 4 );

        if( wav->type[0] != 'W' || wav->type[1] != 'A' || wav->type[2] != 'V' || wav->type[3] != 'E' ) {
            // WAVE
            g_Core->p_Console->Error( ERR_ASSET, "Couldn't load \"%s\" sound file, no WAVE.", soundname );
        }

        stream->Read( wav->type, sizeof(char), 4 );
        if( wav->type[0] != 'f' || wav->type[1] != 'm' || wav->type[2] != 't' || wav->type[3] != ' ') {
            // fmt
            g_Core->p_Console->Error( ERR_ASSET, "Couldn't load \"%s\" sound file, no fmt.", soundname );
        }

        // Read data.
        stream->Read( &wav->chunkSize, sizeof(int), 1 );
        stream->Read( &wav->formatType, sizeof(short), 1 );
        stream->Read( &wav->channels, sizeof(short), 1 );
        stream->Read( &wav->sampleRate, sizeof(int), 1 );
        stream->Read( &wav->avgBps, sizeof(int), 1 );
        stream->Read( &wav->bytesPerSample, sizeof(short), 1 );
        stream->Read( &wav->bitsPerSample, sizeof(short), 1 );

        stream->Read( wav->type, sizeof(char), 4 );
        if( wav->type[0] != 'd' || wav->type[1] != 'a' || wav->type[2] != 't' || wav->type[3] != 'a' ) {
            // data
            g_Core->p_Console->Error( ERR_ASSET, "Couldn't load \"%s\" sound file, no data.", soundname );
        }

        stream->Read( &wav->dataSize, sizeof(int), 1 );

        SMemoryTempFrame * tempSoundMem = _PushMemoryFrame( pLinearAllocator2 );
        
        wav->soundBuffer = (Byte*)PushMemory( tempSoundMem, sizeof(Byte) * wav->dataSize );
        stream->Read( wav->soundBuffer, sizeof(Byte), wav->dataSize );
        
        _PopMemoryFrame( tempSoundMem );
        _PopMemoryFrame( soundData.tempPool );
    }
}

#endif