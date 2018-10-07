//
//  iOSUtilities.m
//  Neko
//
//  Created by Neko Code on 1/29/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#import "SharedApple.h"

#ifdef iOS_BUILD

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#import "iOSUtilities.h"
ipoint2d LastPosition;

namespace nciOSUtilities {
    
    /**
     *  iOS Alert.
     */
    void MessageBox( const char *_title, const char *_message ) {
        UIAlertView *_view = [[UIAlertView alloc] initWithTitle:[NSString stringWithFormat:@"%s", _title] message:[NSString stringWithFormat:@"%s", _message] delegate:nil cancelButtonTitle:@"Okay" otherButtonTitles:nil];
        [_view show];
    }
    
    /**
     *  Last touch position X.
     */
    int32_t GetLastTouchPositionX() {
        return LastPosition._x;
    }
    
    /**
     *  Last touch position Y.
     */
    int32_t GetLastTouchPositionY() {
        return LastPosition._y;
    }
}

#endif