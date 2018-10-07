//
//  iOSUtilities.h
//  Neko
//
//  Created by Neko Code on 1/29/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

// Last touch point.
typedef struct _ipoint {
    int _x;
    int _y;
} ipoint2d;

namespace nciOSUtilities {
    void MessageBox( const char *_title, const char *_message );
    
    int GetLastTouchPositionX();
    int GetLastTouchPositionY();
    
};

extern ipoint2d LastPosition;