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
//  FileSystem.cpp
//  File system.. :vD
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../Platform/Shared/SystemShared.h"
#include "../Core/Core.h"
#include "../Core/Console/ConsoleCommand.h"
#include "../Core/Console/Console.h"
#include "FileSystem.h"
#include "../Core/String/StringHelper.h"
//#include "LocalGame.h"
#include "../Core/Utilities/Utils.h"

#include "../Libraries/Huffman/huffman_lib.h"


#include "../Core/Streams/Streams.h"

#   if defined( _WIN64 )
	#include <io.h> // read/write file handle
#   endif

namespace Neko {
    
    static const uint64_t kPackageFileMaxCache = Megabyte( 16 ); // header info
    
    //!   Log file name.
    SConsoleVar  * Filesystem_Log = 0;
    
    //!   Log file path.
    SConsoleVar  * Filesystem_Path = 0;
    
    //!   Is file logging enabled?
    SConsoleVar  * Filesystem_Logging = 0;

    /**
     *  Constructor.
     */
    CFileSystem::CFileSystem()
    {

    }

    /**
     *  Load simple PAK file.
     *
     *  @param filename     Package filename.
     */
    SPackFile::SPackFile( const char * filename )
    {
     
    }
    
    /**
     *  Load.
     */
    void SPackFile::Load( const char *filename, INekoAllocator * allocator )
    {
        uint32_t    i;
        
        pakheader_t     header;
        pakfile_t       * filesToAdd;
        
        size_t      checksum;
        
        pAllocator = allocator;
        fOwner = CFileSystem::OpenRead( NC_TEXT( "%s/data/%s", Filesystem_Path->Get<const char*>(), filename) );
        
        if( fOwner == NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_ERROR, "Couldn't not load %s package file.\n", filename );
            return;
        }
        
        // Read a header.
        fread( &header, sizeof(header), 1, fOwner );

        // Check if we have a right header.
        if(  header._id != PAKHEADER ) {// memcmp((const void*)header._id, (const void*)PAKHEADER2, 4) != 0  ) {
            g_Core->p_Console->Error( ERR_FILESYSTEM, "PackageLoader(): Couldn't load %s, it isn't pak file! ( Got %i, %x )", filename, header._id, header._id );
            return; // eh.
        }
        
        // If it's okay...
        // Continue!
        header._offset = NekoUtils::LittleLong( header._offset );
        header._length = NekoUtils::LittleLong( header._length );
        
        fseek( fOwner, header._offset, SEEK_SET );
        
        iFilesNum = header._length / sizeof(realpakfile_t);
        
        // Check file amount.
        if( iFilesNum > MAX_PACKAGE_FILES ) {
            g_Core->p_Console->Error( ERR_FATAL, "PackageLoader(): %s has %i files, while maximum is %i!", filename, iFilesNum, MAX_PACKAGE_FILES );
            return; // eeeeeh..
        }
        
        // Allocate space for package files information.
        filesToAdd = (pakfile_t*) pAllocator->Alloc( sizeof(pakfile_t) * header._length );
        if( filesToAdd == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "Couldn't not load \"%s\", out of memory.\n", filename );
            return;
        }
        
        fread( filesToAdd, header._length, 1, fOwner );
        checksum =	-1; // Just leave it for now.
        
        // Read all files in the package.
        for( i = 0; i < iFilesNum; ++i ) {
            filesToAdd[i]._len = NekoUtils::LittleLong( filesToAdd[i]._len );
            filesToAdd[i]._pos = NekoUtils::LittleLong( filesToAdd[i]._pos );
        }
        
        sFilename = filename;
        pFiles = filesToAdd;
        
        g_Core->p_Console->Print( LOG_INFO, "PackageLoader(): %s loaded.. ( %i files )\n", filename, iFilesNum );
    }

    /**
     *  Remove the package file.
     */
    void SPackFile::Kill()
    {
        // Check if we still have file handle.
        if( fOwner != NEKO_NULL ) {
            fclose( fOwner );
        }
        
        // ... and its package files.
        if( pFiles != NEKO_NULL ) {
            pAllocator->Dealloc( pFiles );
        }
        
        iFilesNum = 0;
    }
    
    /**
     *  Read Pak file.
     */
    AssetDataPool SPackFile::GetData( const char *filename, SMemoryTempFrame * memoryPool, bool needsStringTerminate )
    {
        uint32_t    i;
        size_t size;

        AssetDataPool pool;
 
        pool.tempData = NEKO_NULL;
        for( i = 0; i < iFilesNum; ++i ) {
            // Find the file by the name.
            if( !strcmp( filename, pFiles[i]._name ) ) {
                if( memoryPool == NEKO_NULL ) {
                    pool.tempPool = _PushMemoryFrame( pLinearAllocator2 );     // Memory frame will be closed outside.
                } else {
                    pool.tempPool = memoryPool;
                }
                
                pool.tempData = (uint8_t *)PushMemory( pool.tempPool, (pFiles[i]._len + 1) );

                fseek( fOwner, pFiles[i]._pos, SEEK_SET );
                
                size = fread( pool.tempData, 1, pFiles[i]._len, fOwner );

                // Usually used by the textfiles ( scripts, shaders, etc ).
                if( needsStringTerminate ) {
                    pool.tempData[pFiles[i]._len] = '\0';
                }
                
                // Check the file size.
                if( size == 0 ) {
                    g_Core->p_Console->Error( ERR_FATAL, "\"%s\": - requested file \"%s\" has no data!", sFilename.c_str(), filename );
                    return pool;
                }
                
                return pool;
            }
        }
        
        return pool;
    }
 
    /**
     *  Close Pak file.
     */
    void CFileSystem::ClosePak( SPackFile * pack )
    {
        if( pack != NEKO_NULL ) {
            g_Core->p_Console->Print( LOG_INFO, "PackageLoader(): Closing %s...\n", pack->GetPackageName() );
            
            pakfile_t * files = pack->GetFiles();
            
            if( files != NEKO_NULL ) {
                pack->pAllocator->Dealloc( files );
                files = NEKO_NULL;
            }
            
            pResourceAllocator->Dealloc( pack );
            pack = NEKO_NULL;
        }
    }
    
    /**
     *  Get pak-file.
     *
     *  @return null if file not found.
     */
    SPackFile * CFileSystem::GetPak( const char *file )
    {
        SPackFile * pak = m_fPackCache[file];
      
        return pak;
    }

    /**
     *  Add new game package.
     */
    void CFileSystem::AddPackage( const char *pakName, const char * Id )
    {
        SPackFile * newPak = (SPackFile *)pResourceAllocator->Alloc( sizeof(SPackFile) );
        if( newPak == NEKO_NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "FileSystem::AddPackage(): failed to add \"%s\" game package.\n", pakName );
            
            return;
        }
        
        newPak->Load( pakName, pAllocatorHandle );
     
        m_fPackCache[Id] = newPak;
    }

    /**
     *  Initialize file system.
     */
    void CFileSystem::Initialize( const char * defaultpath, INekoAllocator * allocator )
    {
        g_Core->p_Console->Print( LOG_INFO, "Loading file system..\n" );
        
        pAllocatorHandle = allocator;
        pResourceAllocator = NekoAllocator::newPoolAllocator( sizeof(SPackFile), __alignof(SPackFile), sizeof(SPackFile) * MAX_PACKAGES, *pAllocatorHandle );
        pPackageFilePool = NekoAllocator::newPoolAllocator( sizeof(pakfile_t), __alignof(pakfile_t), kPackageFileMaxCache, *pAllocatorHandle );
        
        // Register filesystem console variables.
        Filesystem_Log = g_Core->p_Console->RegisterCVar( ECvarGroup::Engine, "sLogName", "Log file.", "logfile.txt", CVFlag::NeedsRefresh, ECvarType::String );
        Filesystem_Logging= g_Core->p_Console->RegisterCVar( ECvarGroup::Engine, "bLogging", "Turn on logging?", false, CVFlag::NeedsRefresh, ECvarType::Int );
        Filesystem_Path = g_Core->p_Console->RegisterCVar(  ECvarGroup::Engine, "sNekoDirectory", "Current directory.", "", CVFlag::Hidden, ECvarType::String );
        
        // Path length.
        if( strlen( defaultpath ) > (MAX_PATH + 1) ) {
            g_Core->p_Console->Error( ERR_FILESYSTEM, "I am placed far far away. Please make a path shorter.\n", MAX_PATH );
            return;
        }

#   if !defined( NEKO_SERVER )
        Filesystem_Path->Set<const char*>( NC_TEXT( "%s/%s", defaultpath, DEFAULT_EXEC_PATH ) );
#   else
        Filesystem_Path->Set<const char*>( defaultpath );
#   endif
        
        g_Core->p_Console->LoadIni( "neko" );
        
        /* Notify */
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Current directory: \"%s\"\n", defaultpath );
        g_Core->p_Console->CheckForLogging();

        // Add commands.
//        c_CommandManager->Add( "writeconfig", lazyFileGenerateConfig );
        
        m_fPackCache.Create( MAX_PACKAGES, pResourceAllocator, allocator );
        
        g_Core->p_Console->Print( LOG_NONE, "\n" );
        g_Core->p_Console->Print( LOG_INFO, "Loading main packages...\n" );
        
        // Add system packages.
        AddPackage( "shared_data.pak", "shared_data" );
        AddPackage( "textures.pak", "texturedata" );
        AddPackage( "meshes.pak", "meshes" );
        AddPackage( "sounds.pak", "sounds" );
    }

    /**
     *  Generate configuration file.
     *  If 'raw' is true - generate a new file with default values.
     */
    void CFileSystem::GenerateConfigurationFile( void )
    {
        // TODO something
    }

    /**
     *  Shutdown the file system.
     */
    void CFileSystem::Shutdown( void )
    {
        g_Core->p_Console->Print( LOG_INFO, "File system shutting down...\n" );

        // Close the log file.
        if( g_Core->p_Console->GetLogFile() != NEKO_NULL ) {
            fclose( g_Core->p_Console->GetLogFile() );
        }
        
        for( size_t i = 0; i < m_fPackCache.GetSize(); ++i ) {
            const char * materialName = m_fPackCache[i]; // Returns NEKO_NULL if key wasn't found.
            if( materialName != NEKO_NULL ) {
                SPackFile * mat = m_fPackCache[materialName];
                mat->Kill(); // Delete file cache.
                
                pResourceAllocator->Dealloc( mat );
                mat = NEKO_NULL;
            }
        }
        
        // Remove the cache.
        m_fPackCache.Delete();
        
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pPackageFilePool, pAllocatorHandle );
        NekoAllocator::deletePoolAllocator( (CPoolAllocator *)pResourceAllocator, pAllocatorHandle );
    }

    /**
     *  Get file name from a path.
     */
    const char *CFileSystem::GetFileName( const char * path )
    {
        const char *filename;

        // Loop thru every slash.
        filename = strrchr( path, '\\' );

        if( filename == NEKO_NULL ) {
            filename = path;
        } else {
            filename++;
        }

        return filename;
    }

    /**
     *  Get file extension.
     */
    const char * CFileSystem::GetFileExtension( const char * filename )
    {
        const char * dot;

        dot = strrchr( filename, '.' );

        if( !dot || dot == filename ) {
            return "null";
        }

        // ok
        return dot + 1;
    }

    /**
     *  Get filename without extension.
     */
    const char * CFileSystem::GetFileNameWithoutExtension( const char * src )
    {
        static char result[128];
        
//        char * retstr;
        
        if( src == NEKO_NULL ) {
            return NEKO_NULL;
        }
        
//        if( ( retstr = (char*)malloc( strlen( src ) + 1 ) ) == NEKO_NULL )
//            return NEKO_NULL;
        
        strcpy( result, src );
        char * lastdot = strrchr( result, '.' );
        
        if( lastdot != NEKO_NULL ) {
            *lastdot = '\0';
        }
        
        return (const char *)result;
    }

    /**
     *  Get file size.
     */
    const size_t CFileSystem::GetFileLength( FILE *f )
    {
        fseek( f, 0, SEEK_END);
        int32_t size = ftell(f );
        fseek(f , 0, SEEK_SET);
        return size;
    }

    /**
     *  Write to file.
     */
    void CFileSystem::Write( FILE *f, const void *buffer, int len )
    {
        if( fwrite (buffer, 1, len, f) != (size_t)len ) {
            g_Core->p_Console->Error( ERR_FILESYSTEM, "File system failed to write." );
        }
    }

    /**
     *  Open file for writing. ( Nor create it )
     */
    FILE *CFileSystem::OpenWrite( const char * filename )
    {
        FILE   *f;

        f = fopen( filename, "wb" );

        if (!f) {
            g_Core->p_Console->Print( LOG_ERROR, "File system failed to open \"%s\".\n", filename );
        }

        return f;
    }

    /**
     *  Open file for read.
     */
    FILE *CFileSystem::OpenRead( const char * filename )
    {
        FILE    *f;
        f = fopen( filename, "rb" );
        if( !f ) {
            return NEKO_NULL;
        }

        // Also returns null if fopen failed.
        return f;
    }

    /**
     *  Read file and get its data.
     */
    void CFileSystem::Read( FILE *f, void *data, long len )
    {
        if ( fread( data, 1, len, f ) != (size_t)len ) {
            g_Core->p_Console->Error( ERR_FILESYSTEM, "CFileSystem::Read failed to read file." );
        }
    }

    /**
     *  Load file and get its data.
     */
    long CFileSystem::Load( const char * filename, void **buffer, SMemoryTempFrame * tempMemory )
    {
        FILE        *f;
        Byte        *data;
        long        len;

        f = OpenRead( filename );

        if( !f ) {
            return -1;
        }

        len     = GetFileLength( f );
        data    = (Byte *)PushMemory( tempMemory, sizeof(Byte) * (len + 1) ) ; // TODO: Remove new

        ((char*)data)[len] = 0;

        Read( f, data, len );
        fclose( f );

        *buffer = data;
        return len;
    }
}
