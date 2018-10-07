//
//  CoreDef.h
//  Nanocat
//
//  Created by Kawaii Neko on 11/9/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef CoreDef_h
#define CoreDef_h


namespace Neko {
    
#define NEKO_DEBUG
    
    // >= C++14
#define NEKO_NONCOPYABLE( Class )        \
        Class( const Class& ) = delete;       \
        Class & operator = ( const Class& ) = delete;
    
    // Some defines.
#define NC_ASSERTWARN( what ) if( !what ) g_Core->p_Console->Print( LOG_ASSERT, "Assert warning at line %i, %s function", __LINE__, __FUNCTION__ )
#define NC_TEXT NekoCString::STR

}

#endif /* CoreDef_h */
