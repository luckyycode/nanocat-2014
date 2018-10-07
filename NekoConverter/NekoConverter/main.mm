//
//  main.cpp
//  NekoConverter
//
//  Created by Neko Code on 3/29/15.
//  Copyright (c) 2015 nekocode. All rights reserved.
//


#include <stdio.h>

#define NEKO_NO_ASSIMP
#define NEKO_CONVERTER
#define __CONVERTER__

#	if defined( __APPLE__ )

// Independed systems
#include "../../test/Core/String/StringHelper.h"
#include "../../test/Core/Streams/BitMessage.h"

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>

namespace {
    
    /**
     * Since it won't work properly,
     * we need to write custom path finder.
     */
    const char * GetBundlePath()
    {
        const char  *fs_path;
        
        NSString * bundlePath = [[NSBundle mainBundle] resourcePath];
        fs_path = [bundlePath UTF8String];
        
        return fs_path;
    }
}


#	else

// Independed systems
#include "../src/Core/String/StringHelper.h"
#include "../src/Core/Streams/BitMessage.h"

#define int32_t int
#define uint32_t unsigned int
#define uint64_t unsigned long long
#define Byte unsigned char
#define write _write

#pragma comment (lib, "Ws2_32.lib" )

#include <basetsd.h>
#define ssize_t SSIZE_T

#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstdlib>

#include "scandir.h"

namespace {
    
    /**
     *	Get current directory.
     */
    const char * GetBundlePath()
    {
        char cwd[512];
        _getcwd(cwd, 512);
        
        return cwd;
    }
}

#	endif

int32_t LittleLong( int32_t l )
{
    Byte b1, b2, b3, b4;
    
    b1 = l & 255;
    b2 = (l >> 8) & 255;
    b3 = (l >> 16) & 255;
    b4 = (l >> 24) & 255;
    
    return ((int32_t)b1 << 24) + ((int32_t)b2 << 16) + ((int32_t)b3 << 8) + b4;
}

// These structs must be the same as in FileSystem.h


//!  Package header.
#define PAKHEADER (( 'K' << 24 ) + ( 'C' << 16 ) + ( 'A' << 8 ) + 'P' )

typedef struct
{
    uint32_t magic;
    
    uint32_t dir_start;
    uint32_t entry_num;
    
}  raw_pak_header_t;

typedef struct
{
    char name[56];
    
    uint32_t offset;
    uint32_t length;
    
}  raw_pak_entry_t;


static FILE *w_pak_fp;
static raw_pak_entry_t w_pak_entry;

// OWHMAIGAOSH! Global variables..
raw_pak_entry_t	pfiles[2048];
raw_pak_entry_t	*pf;

int32_t iCount = 0;
size_t iBytesPacked = 0; // and compiled

FILE    * packageHndl = nullptr;

int32_t		dirnamelen;
int32_t		compiled_assets;

inline const char *				 GetArgAtIndex(const int32_t idx)  { return Neko::NekoCString::lastCommandArguments[idx]; }

/**
 *  Get file size.
 */
const size_t GetFileLength( FILE *f )
{
    fseek( f, 0, SEEK_END);
    int32_t size = ftell(f );
    fseek(f , 0, SEEK_SET);
    return size;
}

/**
 *  Write to file.
 */
void Write( FILE *f, const void *buffer, int len )
{
    if( fwrite (buffer, 1, len, f) != (size_t)len ) {
        printf(  "File system failed to write." );
    }
}

/**
 *  Open file for read.
 */
FILE *OpenRead( const char * filename )
{
    FILE    *f;
    f = fopen( filename, "rb" );
    if( !f ) {
        return nullptr;
    }
    
    // Also returns null if fopen failed.
    return f;
}

/**
 *  Read file and get its data.
 */
void Read( FILE *f, void *data, long len )
{
    if ( fread( data, 1, len, f ) != (size_t)len ) {
        printf( "CFileSystem::Read failed to read file." );
    }
}

/**
 *  Load file and get its data.
 */
long Load( const char * filename, void **buffer )
{
    FILE        *f;
    void        *data;
    long        len;
    
    f = OpenRead( filename );
    
    if( !f ) {
        return -1;
    }
    
    len     = GetFileLength( f );
    data    = new Byte[len + 1]; // TODO: Remove new
    
    ((char*)data)[len] = 0;
    
    Read( f, data, len );
    fclose( f );
    
    *buffer = data;
    return len;
}


bool PAK_OpenWrite(const char *filename)
{
    w_pak_fp = fopen(filename, "wb");
    
    if (! w_pak_fp)
    {
        printf("PAK_OpenWrite: cannot create file: %s\n", filename);
        return false;
    }
    
    printf("Created PAK file: %s\n", filename);
    
    // write out a dummy header
    raw_pak_header_t header;
    memset(&header, 0, sizeof(header));
    
    fwrite(&header, sizeof(raw_pak_header_t), 1, w_pak_fp);
    fflush(w_pak_fp);
    
    return true;
}

#define ALIGN_LEN(x)  (((x) + 3) & ~3)

//!  Package header.
#define PAKHEADER (( 'K' << 24 ) + ( 'C' << 16 ) + ( 'A' << 8 ) + 'P' )

void PAK_CloseWrite(void)
{
    fflush(w_pak_fp);
    
    // write the directory
    
    printf("Writing PAK directory\n");
    
    raw_pak_header_t header;
    
    header.magic = PAKHEADER;
    
    header.dir_start = (int)ftell(w_pak_fp);
    header.entry_num = 0;
    
    for (int32_t i = 0; i < iCount; ++i  )
    {
        raw_pak_entry_t *E = & pfiles[i];
        
        fwrite(E, sizeof(raw_pak_entry_t), 1, w_pak_fp);
        
        header.entry_num++;
    }
    
    fflush(w_pak_fp);
    
    // finally write the _real_ PAK header
    header.entry_num *= sizeof(raw_pak_entry_t);
    
    header.dir_start = LittleLong(header.dir_start);
    header.entry_num = LittleLong(header.entry_num);
    
    fseek(w_pak_fp, 0, SEEK_SET);
    
    fwrite(&header, sizeof(header), 1, w_pak_fp);
    
    fflush(w_pak_fp);
    fclose(w_pak_fp);
    
    printf("Closed PAK file\n");
}


void PAK_NewLump(const char *name)
{
//    assert(strlen(name) <= 55);
    
    memset(&w_pak_entry, 0, sizeof(w_pak_entry));
    
    strcpy(w_pak_entry.name, name);
    
    w_pak_entry.offset = (uint32_t)ftell(w_pak_fp);
}

bool PAK_AppendData(const void *data, int length)
{
//    if (length == 0)
//        return true;
//    
//    assert(length > 0);
    
    return (fwrite(data, length, 1, w_pak_fp) == 1);
}


void PAK_FinishLump(void)
{
    int len = (int)ftell(w_pak_fp) - (int)w_pak_entry.offset;
    
    // pad lumps to a multiple of four bytes
    int padding = ALIGN_LEN(len) - len;
    
    if (padding > 0)
    {
        static uint8_t zeros[4] = { 0,0,0,0 };
        
        fwrite(zeros, padding, 1, w_pak_fp);
    }
    
    // fix endianness
    w_pak_entry.offset = LittleLong(w_pak_entry.offset);
    w_pak_entry.length = LittleLong(len);
    
    pfiles[iCount] = w_pak_entry;
    ++iCount;
}

/**
 *  Make directory files into one package.
 */
void PackDirectory(const char * dir)
{
    void	 * buffer;
    
    struct dirent ** namelist, *ent;
    
    int	i;
    unsigned int count, len;
    
    struct stat st;
    
    char		newdir[256];
    char		fullname[2048];
    char		*name;
    char    fullFileName[2048];
    
    count = scandir(dir,&namelist, nullptr, nullptr);
    
    int index = 0;
    
    for (i = 0; i < count; ++i)
    {
        ent = namelist[i];
        name = ent->d_name;
        
        if (name[0] == '.') {
            continue;
        }
        
        strcpy(fullname, dir);
        strcat(fullname, "/");
        strcat(fullname, name);
        
        strcpy(fullFileName, fullname + dirnamelen);
        
        if (stat(fullname, &st) == -1) {
            printf("fstating %s", fullFileName);
        }
        
        if (st.st_mode & S_IFDIR) {	 // directory
            strcpy(newdir, fullname);
            PackDirectory(newdir);
            continue;
        }
        
        len = Load(fullname, &buffer);
        
        // Compile required assets.
        if (!strcmp("materials", (dir + dirnamelen))) {
            printf("Compiling \"%s\" material asset..\n", fullname + dirnamelen);
            
            // Tokenize raw material file.
            const int32_t tokenCount = Neko::NekoCString::TokenizeString((const char*)buffer);
            
            // Look for the latest syntax brace.
            while (true) {
                
                if (index >= tokenCount) {
                    break;
                }
                
                const char * material = GetArgAtIndex(index++);
                if (strcmp(GetArgAtIndex(index), "{")) {
                    printf("Material \"%s\" - expected '{{', found '%s'\n", material, GetArgAtIndex(index));
                    ++index;
                    break;
                }
                
                Byte bitData[16384];
                Neko::SBitMessage  *  bitPacker = new Neko::SBitMessage( bitData, 16384 );
                
                char moron[256];
                sprintf( moron, "materials/%s", material );
                
                // Header.
                bitPacker->WriteByte(0x07);
        
                while (true) {
                    const char * token = GetArgAtIndex(index++);
                    if (token == nullptr) {
                        break;
                    }
                    
                    // Sanity checks.
                    if (!strcmp(token, "}")) {
                        bitPacker->WriteByte(0xff);
                        
                        PAK_NewLump(moron);
                        PAK_AppendData(bitPacker->Data, bitPacker->Size);
                        PAK_FinishLump();
                    
                        printf("Compiling..%53s : %7lu b\n", moron, bitPacker->Size);
                        
                        delete bitPacker;
                        
                        pf++;
                        break;
                    }
                    
                    // Material images.
                    if (!strncmp(token, "dif", 3)) {
                        const char * map = GetArgAtIndex(index);
                        
                        bitPacker->WriteByte(0x02);
                        bitPacker->WriteString(map);
                        
                        ++index;
                    }
                    
                    if (!strncmp(token, "norm", 4)) {
                        const char * map = GetArgAtIndex(index);
                        
                        bitPacker->WriteByte(0x04);
                        bitPacker->WriteString(map);
                        
                        ++index;
                    }
                    
                    if (!strncmp(token, "spec", 4)) {
                        const char * map = GetArgAtIndex(index);
                        
                        bitPacker->WriteByte(0x08);
                        bitPacker->WriteString(map);
                        
                        ++index;
                    }
                    
                    // Material properties.
                    if (!strncmp(token, "tile", 4)) {
                        const int32_t tileMode = atoi(GetArgAtIndex(index));
                        
                        bitPacker->WriteByte(0x16);
                        bitPacker->WriteByte((Byte)tileMode);
                        
                        ++index;
                    }
                    
                    if (!strncmp(token, "mipmap", 6)) {
                        const int32_t mipmapMode = atoi(GetArgAtIndex(index));
                        
                        bitPacker->WriteByte(0x32);
                        bitPacker->WriteByte((Byte)mipmapMode);
                        
                        ++index;
                    }
                }
            }
            
            ++compiled_assets;
        }
        // Else, pack another assets.
        else {
        
            PAK_NewLump(fullFileName);
            PAK_AppendData(buffer, len);
            PAK_FinishLump();
        
            printf("%64s : %6llu kb\n", fullFileName, len / 1024LL);
        }
    }
}


/*
 * From Quake2:
 */

// Not enforced by this extractor. Was enforced by the game, we just warn.
#define MAX_FILES_IN_PAK 4096

// 4CC 'PACK'
#define PAK_HEADER_IDENT (('K' << 24) + ('C' << 16) + ('A' << 8) + 'P')

static FILE *r_pak_fp;

static raw_pak_header_t r_header;

static raw_pak_entry_t * r_directory;

#define LogPrintf printf
#define SYS_ASSERT assert

bool PAK_OpenRead(const char *filename)
{
    r_pak_fp = fopen(filename, "rb");
    
    if (! r_pak_fp)
    {
        LogPrintf("PAK_OpenRead: no such file: %s\n", filename);
        return false;
    }
    
    LogPrintf("Opened PAK file: %s\n", filename);
    
    if (fread(&r_header, sizeof(r_header), 1, r_pak_fp) != 1)
    {
        LogPrintf("PAK_OpenRead: failed reading header\n");
        fclose(r_pak_fp);
        return false;
    }
    
    if (r_header.magic != PAKHEADER)
    {
        LogPrintf("PAK_OpenRead: not a PAK file!\n");
        fclose(r_pak_fp);
        return false;
    }
    
    r_header.dir_start = LittleLong(r_header.dir_start);
    r_header.entry_num = LittleLong(r_header.entry_num);
    
    // convert directory length to entry count
    r_header.entry_num /= sizeof(raw_pak_entry_t);
    
    /* read directory */
    
    if (r_header.entry_num >= 5000)  // sanity check
    {
        LogPrintf("PAK_OpenRead: bad header (%d entries?)\n", r_header.entry_num);
        fclose(r_pak_fp);
        return false;
    }
    
    if (fseek(r_pak_fp, r_header.dir_start, SEEK_SET) != 0)
    {
        LogPrintf("PAK_OpenRead: cannot seek to directory (at 0x%08x)\n", r_header.dir_start);
        fclose(r_pak_fp);
        return false;
    }
    
    r_directory = new raw_pak_entry_t[r_header.entry_num + 1];
    
    for (int i = 0; i < (int)r_header.entry_num; i++)
    {
        raw_pak_entry_t *E = &r_directory[i];
        
        int res = fread(E, sizeof(raw_pak_entry_t), 1, r_pak_fp);
        
        if (res == EOF || res != 1 || ferror(r_pak_fp))
        {
            if (i == 0)
            {
                LogPrintf("PAK_OpenRead: could not read any dir-entries!\n");
                
                delete[] r_directory;
                r_directory = NULL;
                
                fclose(r_pak_fp);
                return false;
            }
            
            LogPrintf("PAK_OpenRead: hit EOF reading dir-entry %d\n", i);
            
            // truncate directory
            r_header.entry_num = i;
            break;
        }
        
        // make sure name is NUL terminated.
        E->name[55] = 0;
        
        E->offset = LittleLong(E->offset);
        E->length = LittleLong(E->length);
        
        //  DebugPrintf(" %4d: %08x %08x : %s\n", i, E->offset, E->length, E->name);
    }
    
    return true; // OK
}


void PAK_CloseRead(void)
{
    fclose(r_pak_fp);
    
    LogPrintf("Closed PAK file\n");
    
    delete[] r_directory;
    r_directory = NULL;
}


int PAK_NumEntries(void)
{
    return (int)r_header.entry_num;
}


int PAK_FindEntry(const char *name)
{
    for (unsigned int i = 0; i < r_header.entry_num; i++)
    {
        if (strcmp(name, r_directory[i].name) == 0)
            return i;
    }
    
    return -1; // not found
}


int PAK_EntryLen(int entry)
{
    SYS_ASSERT(entry >= 0 && entry < (int)r_header.entry_num);
    
    return r_directory[entry].length;
}


const char * PAK_EntryName(int entry)
{
    SYS_ASSERT(entry >= 0 && entry < (int)r_header.entry_num);
    
    return r_directory[entry].name;
}

bool PAK_ReadData(int entry, int offset, int length, void *buffer)
{
    raw_pak_entry_t *E = &r_directory[entry];
    
    if ((uint32_t)offset + (uint32_t)length > E->length)  // EOF
        return false;
    
    if (fseek(r_pak_fp, E->offset + offset, SEEK_SET) != 0)
        return false;
    
    int res = fread(buffer, length, 1, r_pak_fp);
    
    return (res == 1);
}


/**
 *  Get min number.
 */
template<class T>
inline T Min( T min, T max )
{
    return max < min ? max : min;
}

void PAK_ListEntries(void)
{
    if( r_header.entry_num == 0 ) {
        printf("PAK file is empty\n");
    } else {
        FILE * file = nullptr;
        
        for( int i = 0; i < (int)r_header.entry_num; ++i )
        {
            raw_pak_entry_t *E = &r_directory[i];
            char    filename[1024];
            // Find full path name.
            sprintf(filename, "%s/%s", GetBundlePath(), E->name );//argv[1]);
            
            
            int entry_len = PAK_EntryLen(i);
            
            bool failed = false;
            
            FILE *fp = fopen(filename, "wb+");
            if (fp) {
                static Byte buffer[2048];
                
                // transfer data from PAK into new file
                int position = 0;
                
                while (position < entry_len) {
                    int count = Min((int)sizeof(buffer), entry_len - position);
                    
                    if (! PAK_ReadData(i, position, count, buffer)) {
                        printf("FAILURE: read error on %d bytes\n\n", count);
                        failed = true;
                        break;
                    }
                    
                    if (1 != fwrite(buffer, count, 1, fp)) {
                        printf("FAILURE: write error on %d bytes\n\n", count);
                        failed = true;
                        break;
                    }
                    
                    position += count;
                }
                
                fclose(fp);
            } else {
                printf("FAILURE: cannot create output file: %s\n\n", filename);
                failed = true;
            }

        }
    }
}


int main(int argc, const char * argv[]) {
    // insert code here...
    //std::cout << "Hello, World!\n";
    
    compiled_assets = 0;
    
    char c[128];
    printf("\n- -----------------------------------------------------------------------------\n ");
    printf("Neko engine converter tool\n");
    printf("Build: %s, %s\n\n", __DATE__, __TIME__);
    //    if( argc != 3 ) {
    //        printf( "No arguments found! Please make sure that there are arguments.\n" );
    //        return -1;
    //    }
    //printf( "argc = %i\n", argc );
    
    printf("\nUsage:\n");
    printf("\t-b\t\tBuild packed archive from a chosen directory.\n");
    printf("\t-d\t\tUnpack all files from archive.\n");
    
    printf("\nWhat you want to do with \n");
    printf("\t%s ?\n", argv[1]);
    printf("- -----------------------------------------------------------------------------\n ");
    
    fscanf(stdin, "%s", c);
    argv[1] = "shared_data";
    argv[2] = "shared_data";
    const char * dir = GetBundlePath();
    const char * fullName = Neko::NekoCString::STR("%s/data/%s.pak", dir, argv[1]);
    
    /* Compressing */
    if (!strcmp(c, "-b")) {
        char    outname[1024];
        
        // Find full path name.
        printf("Processing %s\n\n", fullName);
        sprintf(outname, "%s", fullName);//argv[1]);
        
        
        // Open ( create ) new package file.
        PAK_OpenWrite(outname);

        // New path for package.
        dirnamelen = strlen(argv[2]) + 1;
        
        // Pack directory now.
        PackDirectory(argv[2]);
        // Close package.
        PAK_CloseWrite();

        // Print information.
        printf("Successfully compiled %i assets.\n", compiled_assets);
//        printf("%i files packed in %zu bytes ( %lu kb )\n", iCount, iBytesPacked, iBytesPacked / 1024);
        
    } else if( !strcmp(c,"-u")) {
        
        char    outname[1024];
        
        // Find full path name.
        printf("Processing %s\n\n", fullName);
        sprintf(outname, "%s", fullName);//argv[1]);
        
        
        // Open ( create ) new package file.
        PAK_OpenRead(outname);
        
        // New path for package.
        dirnamelen = strlen(argv[2]) + 1;
        PAK_ListEntries();
        
        // Close package.
        PAK_CloseRead();
        

        // Success.
    }
    
    printf("Done!\n");
    
#	if defined( _WIN32 ) 
    system( "pause" );
#	endif
    
    return 0;
}
