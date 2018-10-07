//
//  OSXJoystick.hpp
//  Nanocat
//
//  Created by Kawaii Neko on 3/17/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef OSXJoystick_hpp
#define OSXJoystick_hpp

#include <IOKit/hid/IOHIDLib.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDUsageTables.h>

/* For force feedback testing. */
#include <ForceFeedback/ForceFeedback.h>
#include <ForceFeedback/ForceFeedbackConstants.h>

#include "../../Shared/SystemShared.h"
#include "../../../Core/Joystick.h"

namespace Neko {
    
    class CXJoystick : public IGamepadSystemInterface
    {
    public:
        
        CXJoystick()
        {
            
        }
        
        virtual ~CXJoystick()
        {
            
        }
        
        virtual int32_t                Init();
        virtual int32_t                 Open( IGamepad * gamepad, int32_t deviceIndex );
        virtual void                Close( IGamepad * gamepad ) ;
        
        virtual void                Update( IGamepad * gamepad );
        
        virtual void                Detect( IGamepad * gamepad );
        
        virtual void                Remove( IGamepad * gamepad );
        
        virtual int32_t                 GetNumJoysticks();
        virtual const char *            GetNameByIndex( const uint32_t index );
        virtual SIJoystickID                GetInstanceIdByIndex( const uint32_t index );
        virtual JoystickGUID                GetDeviceGuid( const uint32_t index );
        virtual JoystickGUID                GetJoystickGuid( IGamepad * gamepad );
        
        virtual bool                IsJoystickAttached( IGamepad * gamepad );
    };
    
    struct recElement
    {
        IOHIDElementRef elementRef;
        IOHIDElementCookie cookie;
        uint32_t usagePage, usage;      /* HID usage */
        SInt32 min;                   /* reported min value possible */
        SInt32 max;                   /* reported max value possible */
        
        /* runtime variables used for auto-calibration */
        SInt32 minReport;             /* min returned value */
        SInt32 maxReport;             /* max returned value */
        
        struct recElement *pNext;   /* next element in list */
    };
    
    typedef struct recElement recElement;
    struct SJoystickHardwareInfo
    {
        IOHIDDeviceRef deviceRef;   /* HIDManager device handle */
        io_service_t ffservice;     /* Interface for force feedback, 0 = no ff */
        
        char product[256];          /* name of product */
        uint32_t usage;                 /* usage page from IOUSBHID Parser.h which defines general usage */
        uint32_t usagePage;             /* usage within above page from IOUSBHID Parser.h which defines specific usage */
        
        int axes;                  /* number of axis (calculated, not reported by device) */
        int buttons;               /* number of buttons (calculated, not reported by device) */
        int hats;                  /* number of hat switches (calculated, not reported by device) */
        int elements;              /* number of total elements (should be total of above) (calculated, not reported by device) */
        
        recElement *firstAxis;
        recElement *firstButton;
        recElement *firstHat;
        
        bool removed;
        
        int iInstanceId;
        JoystickGUID guid;
        
        struct SJoystickHardwareInfo *pNext;      /* next device */
    };
    
    typedef struct SJoystickHardwareInfo recDevice;
}

#endif /* OSXJoystick_hpp */
