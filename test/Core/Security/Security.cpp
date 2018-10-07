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
//  Security.h
//
//  Security functions and stuff. D:
//
//  Created by Neko Code on 11/17/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Security.h"

#include "../Core.h"
#include "../../Platform/Shared/SystemShared.h"

namespace Neko {
    ncSecurity local_securitySector;
    ncSecurity *g_Security = &local_securitySector;
    
    /**
     *  Make XOR encryption/decrypyion by multichar key.
     */
    const char * MakeXORMultichar( char * value, char * key )
    {
        short unsigned int klen = strlen( key );
        short unsigned int vlen = strlen( value );
        short unsigned int k = 0;
        
        char * retval = value;
        
        for( int v = 0; v < vlen; v++ ) {
            retval[v] = value[v] ^ key[k];
            k = ( ++k < klen ? k : 0 );
        }
        
        return retval;
    }
    
    /**
     *  Make XOR encryption/decryption.
     */
    char * MakeXOR( char * value, char key  ) {
        
        short unsigned int vlen = strlen( value );
        char * output = value;
        
        for (int i = 0; i < vlen; i++)
            output[i] = value[i] ^ key;
        
        return output;
    }
}
