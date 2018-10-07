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
//  FileSystem.h
//  File system.. :vD
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef files_h
#define files_h

#include "../Platform/Shared/SystemShared.h"
#include "../Core/Console/ConsoleVariable.h"
#include "../Core/Core.h" // non-copyable define.

#   if defined( __CONVERTER__ )
        #include "huffman.h"
#   endif

namespace Neko {
    
    static const uint32_t MAX_PACKAGE_FILES = 2048; // per package
    static const uint32_t MAX_PACKAGES      = 8;
    
    //!  Package header.
    #define PAKHEADER (( 'K' << 24 ) + ( 'C' << 16 ) + ( 'A' << 8 ) + 'P' )
    
    /**
     *  Files in package struct.
     */
    struct pakfile_t
    {
        char    _name[56];
        int32_t _pos, _len;
    };
    
    ///  Package file loader.
    struct SPackFile
    {
    public://private:
        
        /**
         *  Load new package file.
         */
        SPackFile( const char * filename );
        
        /**
         *  Destructor.
         */
        ~SPackFile() {

        }
        
        /**
         *  Load.
         */
        void                Load( const char * filename, INekoAllocator * allocator );
        
        /**
         *  Remove the package data.
         */
        void                Kill();
        
        //!  File handler.
        FILE    * fOwner;
        //Byte * uncompressed;
        //Byte * uncompressed_src;
        
        //!  Package files inside.
        uint32_t     iFilesNum;
    
        //!  File offset and length data.
        pakfile_t   * pFiles;
        
        //!  File name.
        CStr       sFilename;
 
        //!  Get data for specified file.
        AssetDataPool GetData( const char * filename, size_t & length, bool needsStringTerminate = false );
        AssetDataPool GetData( const char * filename, SMemoryTempFrame * memoryPool = NEKO_NULL, bool needsStringTerminate = false );
        
        /**
         *  Get package file handler.
         */
        inline FILE *               GetHandle() const {     return fOwner;    }
        
        /**
         *  Get package name.
         */
        inline const char *                 GetPackageName() const {     return sFilename;  }
        
        /**
         *  Get package files.
         */
        inline pakfile_t *              GetFiles() const {     return pFiles; }
        
        /**
         *  Get package file count.
         */
        inline const uint32_t               GetFileCount() const {     return iFilesNum; }
        
        //void Close();
        
        INekoAllocator  *   pAllocator; // used for file storage
    };
    
    ///  Package header struct.
    struct pakheader_t
    {
//        char     _id[4];
        
        int32_t _id;
        int32_t     _offset;
        int32_t     _length;
    };
    
    ///  Package file.
    struct realpakfile_t
    {
        char    name[56];
        int32_t filepos, filelen;
    };

    ///  File system.
    class CFileSystem
    {
        NEKO_NONCOPYABLE( CFileSystem );
        
    public:
        
        /**
         *  Constructor.
         */
        CFileSystem();
        
        /**
         *  Destructor.
         */
        ~CFileSystem()
        {
            
        }
        
        /**
         *  Initialize file system with current directory.
         */
        void Initialize( const char * defaultpath, INekoAllocator * allocator );
        void GenerateConfigurationFile( void );
        
        /**
         *  Shutdown the filesystem.
         */
        void Shutdown( void );

        /**
         *  Get file name from given path.
         */
        static const char * GetFileName( const char * path );
        
        /**
         *  Get file extension.
         */
        static const char * GetFileExtension( const char * filename );
        
        /**
         *  Get file name without extension.
         */
        static const char * GetFileNameWithoutExtension( const char * src );
        
        /**
         *  Read configuration from script.
         */
        void ReadConfiguration( void );
        
        /**
         *  Close package file.
         */
        void ClosePak( SPackFile * pack );

        /**
         *  Get file length for given file.
         */
        static const size_t GetFileLength( FILE *f );
        
        /**
         *  Write to file.
         */
        static void Write( FILE *f, const void *buffer, int len );
        
        /**
         *  Read from file.
         */
        static void Read( FILE *f, void *data, long len );
        
        /**
         *  Safe open write to file.
         */
        static FILE *OpenWrite( const char * filename );
        
        /**
         *  Safe open read from file.
         */
        static FILE *OpenRead( const char * filename );
        
        /**
         *  Load data from the file.
         */
        static long Load( const char * filename, void **buffer, SMemoryTempFrame * tempMemory );
        
        /**
         *  Load new package file.
         */
        static SPackFile * LoadPak( const char * filename );
        
        /**
         *  Get package by name.
         */
        SPackFile * GetPak( const char * file );
        
        /**
         *  Add package and set its identificator.
         */
        void AddPackage( const char * pakName, const char * Id );
        
        //static
    private:
        
        INekoAllocator      * pAllocatorHandle = 0;
        INekoAllocator      * pResourceAllocator = 0;
        INekoAllocator      * pPackageFilePool = 0;
        
        /**
         *  Package cache.
         */
        CHashMap<const char*, SPackFile*> m_fPackCache;
    };
    
    // FILE SYSTEM
    extern SConsoleVar        * Filesystem_Path;                           // Current game path.
    extern SConsoleVar        * Filesystem_Log;                            // Log filename.
    extern SConsoleVar        * Filesystem_Logging;                        // Is file logging enabled?

}

#endif
