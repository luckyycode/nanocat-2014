//
//  OSXJoystick.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 3/17/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#include "OSXJoystick.hpp"
#include "../../../Core/Core.h"

namespace Neko {
    
#define NEKO_GAMEPAD_RUNLOOP_MODE CFSTR("NekoJoystick")
    
    /* The base object of the HID Manager API */
    static IOHIDManagerRef hidman = NEKO_NULL;
    
    /* Linked list of all available devices */
    static recDevice *gpDeviceList = NEKO_NULL;
    
    /* static incrementing counter for new joystick devices seen on the system. Devices should start with index 0 */
    static int s_joystick_iInstanceId = -1;
    
    static recDevice *GetDeviceForIndex( int32_t deviceIndex )
    {
        recDevice *device = gpDeviceList;
        while( device ) {
            if( !device->removed ) {
                if( deviceIndex == 0 ) {
                    break;
                }
                
                --deviceIndex;
            }
            
            device = device->pNext;
        }
        return device;
    }
    
    static void FreeElementList(recElement *pElement)
    {
        while (pElement) {
            recElement *pElementNext = pElement->pNext;
            free(pElement);
            pElement = pElementNext;
        }
    }
    
    static recDevice * FreeDevice(recDevice *removeDevice)
    {
        recDevice *  pDeviceNext = NEKO_NULL;
        
        if( removeDevice ) {
            if( removeDevice->deviceRef ) {
                IOHIDDeviceUnscheduleFromRunLoop( removeDevice->deviceRef, CFRunLoopGetCurrent(), NEKO_GAMEPAD_RUNLOOP_MODE );
                removeDevice->deviceRef = NEKO_NULL;
            }
            
            /* save next device prior to disposing of this device */
            pDeviceNext = removeDevice->pNext;
            
            if( gpDeviceList == removeDevice ) {
                gpDeviceList = pDeviceNext;
            } else {
                recDevice *device = gpDeviceList;
                
                while( device->pNext != removeDevice ) {
                    device = device->pNext;
                }
                
                device->pNext = pDeviceNext;
            }
            
            removeDevice->pNext = NEKO_NULL;
            
            /* free element lists */
            FreeElementList( removeDevice->firstAxis );
            FreeElementList( removeDevice->firstButton );
            FreeElementList( removeDevice->firstHat );
            
            free( removeDevice );
        }
        
        return pDeviceNext;
    }
    
    static SInt32 GetHIDElementState(recDevice *pDevice, recElement *pElement)
    {
        SInt32 value = 0;
        
        if (pDevice && pElement) {
            IOHIDValueRef valueRef;
            if (IOHIDDeviceGetValue(pDevice->deviceRef, pElement->elementRef, &valueRef) == kIOReturnSuccess) {
                value = (SInt32) IOHIDValueGetIntegerValue(valueRef);
                
                /* record min and max for auto calibration */
                if (value < pElement->minReport) {
                    pElement->minReport = value;
                }
                if (value > pElement->maxReport) {
                    pElement->maxReport = value;
                }
            }
        }
        
        return value;
    }
    
    static SInt32 GetHIDScaledCalibratedState(recDevice * pDevice, recElement * pElement, SInt32 min, SInt32 max)
    {
        const float deviceScale = max - min;
        const float readScale = pElement->maxReport - pElement->minReport;
        const SInt32 value = GetHIDElementState(pDevice, pElement);
        if (readScale == 0) {
            return value;           /* no scaling at all */
        }
        return ((value - pElement->minReport) * deviceScale / readScale) + min;
    }
    
    static void JoystickDeviceWasRemovedCallback( void *ctx, IOReturn result, void *sender)
    {
        printf("JoystickDeviceWasRemovedCallback\n" );
        
        recDevice *device = (recDevice *) ctx;
        
        device->removed = true;
        device->deviceRef = NEKO_NULL; // deviceRef was invalidated due to the remove
        
        
        // Remove joystick.
        g_Core->RemoveGameController( device->iInstanceId );
    }
    
    
    static void AddHIDElement(const void *value, void *parameter);
    
    /* Call AddHIDElement() on all elements in an array of IOHIDElementRefs */
    static void AddHIDElements(CFArrayRef array, recDevice *pDevice)
    {
        const CFRange range = { 0, CFArrayGetCount(array) };
        CFArrayApplyFunction(array, range, AddHIDElement, pDevice);
    }
    
    static bool ElementAlreadyAdded(const IOHIDElementCookie cookie, const recElement *listitem) {
        while (listitem) {
            if (listitem->cookie == cookie) {
                return true;
            }
            listitem = listitem->pNext;
        }
        return false;
    }
    
    /* See if we care about this HID element, and if so, note it in our recDevice. */
    static void AddHIDElement(const void *value, void *parameter)
    {
        recDevice *pDevice = (recDevice *) parameter;
        IOHIDElementRef refElement = (IOHIDElementRef) value;
        const CFTypeID elementTypeID = refElement ? CFGetTypeID(refElement) : 0;
        
        if (refElement && (elementTypeID == IOHIDElementGetTypeID())) {
            const IOHIDElementCookie cookie = IOHIDElementGetCookie(refElement);
            const uint32_t usagePage = IOHIDElementGetUsagePage(refElement);
            const uint32_t usage = IOHIDElementGetUsage(refElement);
            recElement *element = NEKO_NULL;
            recElement **headElement = NEKO_NULL;
            
            /* look at types of interest */
            switch (IOHIDElementGetType(refElement)) {
                case kIOHIDElementTypeInput_Misc:
                case kIOHIDElementTypeInput_Button:
                case kIOHIDElementTypeInput_Axis: {
                    switch (usagePage) {    /* only interested in kHIDPage_GenericDesktop and kHIDPage_Button */
                        case kHIDPage_GenericDesktop:
                            switch (usage) {
                                case kHIDUsage_GD_X:
                                case kHIDUsage_GD_Y:
                                case kHIDUsage_GD_Z:
                                case kHIDUsage_GD_Rx:
                                case kHIDUsage_GD_Ry:
                                case kHIDUsage_GD_Rz:
                                case kHIDUsage_GD_Slider:
                                case kHIDUsage_GD_Dial:
                                case kHIDUsage_GD_Wheel:
                                    if (!ElementAlreadyAdded(cookie, pDevice->firstAxis)) {
                                        element = (recElement *) calloc(1, sizeof (recElement));
                                        if (element) {
                                            pDevice->axes++;
                                            headElement = &(pDevice->firstAxis);
                                        }
                                    }
                                    break;
                                    
                                case kHIDUsage_GD_Hatswitch:
                                    if (!ElementAlreadyAdded(cookie, pDevice->firstHat)) {
                                        element = (recElement *) calloc(1, sizeof (recElement));
                                        if (element) {
                                            pDevice->hats++;
                                            headElement = &(pDevice->firstHat);
                                        }
                                    }
                                    break;
                                case kHIDUsage_GD_DPadUp:
                                case kHIDUsage_GD_DPadDown:
                                case kHIDUsage_GD_DPadRight:
                                case kHIDUsage_GD_DPadLeft:
                                case kHIDUsage_GD_Start:
                                case kHIDUsage_GD_Select:
                                    if (!ElementAlreadyAdded(cookie, pDevice->firstButton)) {
                                        element = (recElement *) calloc(1, sizeof (recElement));
                                        if (element) {
                                            pDevice->buttons++;
                                            headElement = &(pDevice->firstButton);
                                        }
                                    }
                                    break;
                            }
                            break;
                            
                        case kHIDPage_Simulation:
                            switch (usage) {
                                case kHIDUsage_Sim_Rudder:
                                case kHIDUsage_Sim_Throttle:
                                    if (!ElementAlreadyAdded(cookie, pDevice->firstAxis)) {
                                        element = (recElement *) calloc(1, sizeof (recElement));
                                        if (element) {
                                            pDevice->axes++;
                                            headElement = &(pDevice->firstAxis);
                                        }
                                    }
                                    break;
                                    
                                default:
                                    break;
                            }
                            break;
                            
                        case kHIDPage_Button:
                        case kHIDPage_Consumer: /* e.g. 'pause' button on Steelseries MFi gamepads. */
                            if (!ElementAlreadyAdded(cookie, pDevice->firstButton)) {
                                element = (recElement *) calloc(1, sizeof (recElement));
                                if (element) {
                                    pDevice->buttons++;
                                    headElement = &(pDevice->firstButton);
                                }
                            }
                            break;
                            
                        default:
                            break;
                    }
                }
                    break;
                    
                case kIOHIDElementTypeCollection: {
                    CFArrayRef array = IOHIDElementGetChildren(refElement);
                    if (array) {
                        AddHIDElements(array, pDevice);
                    }
                }
                    break;
                    
                default:
                    break;
            }
            
            if (element && headElement) {       /* add to list */
                recElement *elementPrevious = NEKO_NULL;
                recElement *elementCurrent = *headElement;
                while (elementCurrent && usage >= elementCurrent->usage) {
                    elementPrevious = elementCurrent;
                    elementCurrent = elementCurrent->pNext;
                }
                if (elementPrevious) {
                    elementPrevious->pNext = element;
                } else {
                    *headElement = element;
                }
                
                element->elementRef = refElement;
                element->usagePage = usagePage;
                element->usage = usage;
                element->pNext = elementCurrent;
                
                element->minReport = element->min = (SInt32) IOHIDElementGetLogicalMin(refElement);
                element->maxReport = element->max = (SInt32) IOHIDElementGetLogicalMax(refElement);
                element->cookie = IOHIDElementGetCookie(refElement);
                
                pDevice->elements++;
            }
        }
    }
    
    static bool GetDeviceInfo(IOHIDDeviceRef hidDevice, recDevice *pDevice)
    {
        UInt32 *guid32 = NEKO_NULL;
        CFTypeRef refCF = NEKO_NULL;
        CFArrayRef array = NEKO_NULL;
        
        /* get usage page and usage */
        refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDPrimaryUsagePageKey));
        if (refCF) {
            CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &pDevice->usagePage);
        }
        if (pDevice->usagePage != kHIDPage_GenericDesktop) {
            return false; /* Filter device list to non-keyboard/mouse stuff */
        }
        
        refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDPrimaryUsageKey));
        if (refCF) {
            CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &pDevice->usage);
        }
        
        if ((pDevice->usage != kHIDUsage_GD_Joystick &&
             pDevice->usage != kHIDUsage_GD_GamePad &&
             pDevice->usage != kHIDUsage_GD_MultiAxisController)) {
            return false; /* Filter device list to non-keyboard/mouse stuff */
        }
        
        pDevice->deviceRef = hidDevice;
        
        /* get device name */
        refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDProductKey));
        if (!refCF) {
            /* Maybe we can't get "AwesomeJoystick2000", but we can get "Logitech"? */
            refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDManufacturerKey));
        }
        if ((!refCF) || (!CFStringGetCString((CFStringRef)refCF, pDevice->product, sizeof (pDevice->product), kCFStringEncodingUTF8))) {
            strlcpy(pDevice->product, "Unidentified joystick", sizeof (pDevice->product));
        }
        
        refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDVendorIDKey));
        if (refCF) {
            CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &pDevice->guid.data[0]);
        }
        
        refCF = IOHIDDeviceGetProperty(hidDevice, CFSTR(kIOHIDProductIDKey));
        if (refCF) {
            CFNumberGetValue((CFNumberRef)refCF, kCFNumberSInt32Type, &pDevice->guid.data[8]);
        }
        
        /* Check to make sure we have a vendor and product ID
         If we don't, use the same algorithm as the Linux code for Bluetooth devices */
        guid32 = (UInt32*)pDevice->guid.data;
        if (!guid32[0] && !guid32[1]) {
            /* If we don't have a vendor and product ID this is probably a Bluetooth device */
            const UInt16 BUS_BLUETOOTH = 0x05;
            UInt16 *guid16 = (UInt16 *)guid32;
            *guid16++ = BUS_BLUETOOTH;
            *guid16++ = 0;
            strlcpy((char*)guid16, pDevice->product, sizeof(pDevice->guid.data) - 4);
        }
        
        array = IOHIDDeviceCopyMatchingElements(hidDevice, NEKO_NULL, kIOHIDOptionsTypeNone);
        if (array) {
            AddHIDElements(array, pDevice);
            CFRelease(array);
        }
        
        return true;
    }
    
    static bool JoystickAlreadyKnown(IOHIDDeviceRef ioHIDDeviceObject)
    {
        recDevice * i;
        
        for( i = gpDeviceList; i != NEKO_NULL; i = i->pNext ) {
            if( i->deviceRef == ioHIDDeviceObject ) {
                return true;
            }
        }
        
        return false;
    }
    
    static void JoystickDeviceWasAddedCallback(void *ctx, IOReturn res, void *sender, IOHIDDeviceRef ioHIDDeviceObject)
    {
        printf( "JoystickDeviceWasAddedCallback\n" );
        
        recDevice *device;
        int deviceIndex = 0;
        
        if( res != kIOReturnSuccess ) {
            return;
        }
        
        if( JoystickAlreadyKnown(ioHIDDeviceObject) ) {
            printf( "JoystickAlreadyKnown\n") ;
            
            return;
        }
        
        device = (recDevice *) calloc(1, sizeof(recDevice));
        
        if( device == NEKO_NULL ) {
            printf("OutOfMemory();\n");
            return;
        }
        
        if( !GetDeviceInfo(ioHIDDeviceObject, device) ) {
            printf("GetDeviceInfo\n");
            free(device);
            return;   /* not a device we care about, probably. */
        }
        
        /* Get notified when this device is disconnected. */
        IOHIDDeviceRegisterRemovalCallback(ioHIDDeviceObject, JoystickDeviceWasRemovedCallback, device);
        IOHIDDeviceScheduleWithRunLoop(ioHIDDeviceObject, CFRunLoopGetCurrent(), NEKO_GAMEPAD_RUNLOOP_MODE);
        
        /* Allocate an instance ID for this device */
        device->iInstanceId = ++s_joystick_iInstanceId;
        
        /* We have to do some storage of the io_service_t for HapticOpenFromJoystick */
        const io_service_t ioservice = IOHIDDeviceGetService(ioHIDDeviceObject);
#if HAPTIC_IOKIT
        if ((ioservice) && (FFIsForceFeedback(ioservice) == FF_OK)) {
            device->ffservice = ioservice;
            MacHaptic_MaybeAddDevice(ioservice);
        }
#endif
        
        
        /* Add device to the end of the list */
        if ( !gpDeviceList ) {
            gpDeviceList = device;
        } else {
            recDevice *curdevice;
            
            curdevice = gpDeviceList;
            while ( curdevice->pNext ) {
                ++deviceIndex;
                curdevice = curdevice->pNext;
            }
            curdevice->pNext = device;
            ++deviceIndex;  /* bump by one since we counted by pNext. */
        }
        
        // Add a controller.
        g_Core->AddGameController( deviceIndex );
    }
    
    static bool ConfigHIDManager(CFArrayRef matchingArray)
    {
        CFRunLoopRef runloop = CFRunLoopGetCurrent();
        
        if (IOHIDManagerOpen(hidman, kIOHIDOptionsTypeNone) != kIOReturnSuccess) {
            printf( "ConfigHIDManager() failed\n" );
            return false;
        }
        
        IOHIDManagerSetDeviceMatchingMultiple(hidman, matchingArray);
        IOHIDManagerRegisterDeviceMatchingCallback(hidman, JoystickDeviceWasAddedCallback, NEKO_NULL);
        IOHIDManagerScheduleWithRunLoop(hidman, runloop, NEKO_GAMEPAD_RUNLOOP_MODE);
        
        printf( "registering hotplug mode\n" );
        
        while( CFRunLoopRunInMode( NEKO_GAMEPAD_RUNLOOP_MODE, 0, TRUE ) == kCFRunLoopRunHandledSource ) {
            /* no-op. Callback fires once per existing device. */
        }
        
        /* future hotplug events will come through NEKO_GAMEPAD_RUNLOOP_MODE now. */
        
        return true;  /* good to go. */
    }
    
    
    static CFDictionaryRef CreateHIDDeviceMatchDictionary(const UInt32 page, const UInt32 usage, int *okay)
    {
        CFDictionaryRef retval = NEKO_NULL;
        CFNumberRef pageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
        CFNumberRef usageNumRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
        const void *keys[2] = { (void *) CFSTR(kIOHIDDeviceUsagePageKey), (void *) CFSTR(kIOHIDDeviceUsageKey) };
        const void *vals[2] = { (void *) pageNumRef, (void *) usageNumRef };
        
        if (pageNumRef && usageNumRef) {
            retval = CFDictionaryCreate(kCFAllocatorDefault, keys, vals, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        }
        
        if (pageNumRef) {
            CFRelease(pageNumRef);
        }
        if (usageNumRef) {
            CFRelease(usageNumRef);
        }
        
        if (!retval) {
            *okay = 0;
        }
        
        return retval;
    }
    
    bool CreateHIDManager(void)
    {
        bool retval = false;
        int okay = 1;
        const void *vals[] = {
            (void *) CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick, &okay),
            (void *) CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad, &okay),
            (void *) CreateHIDDeviceMatchDictionary(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController, &okay),
        };
        const size_t numElements = (sizeof(vals)/sizeof(vals[0]));
        CFArrayRef array = okay ? CFArrayCreate(kCFAllocatorDefault, vals, numElements, &kCFTypeArrayCallBacks) : NEKO_NULL;
        size_t i;
        
        printf( "interfaces: %zu array = %s\n", numElements, array == NEKO_NULL ? "NEKO_NULL" : "not empty");
        
        for (i = 0; i < numElements; i++) {
            if (vals[i]) {
                CFRelease((CFTypeRef) vals[i]);
            }
        }
        
        if (array) {
            hidman = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
            if (hidman != NEKO_NULL) {
                retval = ConfigHIDManager(array);
            }
            CFRelease(array);
        }
        
        return retval;
    }

    int32_t CXJoystick::Open( IGamepad *gamepad, int32_t deviceIndex )
    {
        recDevice *device = GetDeviceForIndex(deviceIndex);
        
        gamepad->iInstanceId = device->iInstanceId;
        gamepad->pHardwareInfo = device;
        gamepad->name = device->product;
        
        gamepad->iAxesNum = device->axes;
        gamepad->iHatsNum = device->hats;
        gamepad->iBallsNum = 0;
        gamepad->iButtonNum = device->buttons;
        
        return 0;
    }
    
    void CXJoystick::Close( IGamepad * gamepad )
    {
        
    }
    
    int32_t CXJoystick::GetNumJoysticks()
    {
        recDevice *device = gpDeviceList;
        int nJoySticks = 0;
        
        while( device ) {
            if( !device->removed ) {
                nJoySticks++;
            }
            device = device->pNext;
        }
        
        return nJoySticks;
    }
    
    int32_t CXJoystick::Init()
    {
        if( gpDeviceList ) {
            printf("Joystick: Device list already inited.");
            return 0;
        }
        
        if( !CreateHIDManager() ) {
            printf("Joystick: Couldn't initialize HID Manager");
            return 0;
        }
        
        return GetNumJoysticks();
    }
    
    void CXJoystick::Update( IGamepad *gamepad )
    {
        recDevice *device = gamepad->pHardwareInfo;
        recElement *element;
        SInt32 value, range;
        int i;
        
        if( !device ) {
            return;
        }
        
        if( device->removed ) {      /* device was unplugged; ignore it. */
            if( gamepad->pHardwareInfo ) {
                gamepad->bForceReset = true;
                gamepad->pHardwareInfo = NEKO_NULL;
            }
            return;
        }
        
        element = device->firstAxis;
        i = 0;
        while( element ) {
            value = GetHIDScaledCalibratedState( device, element, -32768, 32767 );
            
            if( value != gamepad->axes[i] ) {
                gamepad->OnJoystickAxisAction( i, value);
            }
            
            element = element->pNext;
            ++i;
        }
        
        element = device->firstButton;
        i = 0;
        while( element ) {
            value = GetHIDElementState(device, element);
            
            if( value > 1 ) {          /* handle pressure-sensitive buttons */
                value = 1;
            }
            
            if( value != gamepad->buttons[i] ) {
                gamepad->OnJoystickButtonAction( i, value);
            }
            
            element = element->pNext;
            ++i;
        }
        
        element = device->firstHat;
        i = 0;
        while( element ) {
            UInt8 pos = 0;
            
            range = (element->max - element->min + 1);
            value = GetHIDElementState(device, element) - element->min;
            
            if( range == 4 ) {         /* 4 position hatswitch - scale up value */
                value *= 2;
            } else if( range != 8 ) {    /* Neither a 4 nor 8 positions - fall back to default position (centered) */
                value = -1;
            }
            
            switch( value ) {
                case 0:
                    pos = GAMEPAD_HAT_UP;
                    break;
                case 1:
                    pos = GAMEPAD_HAT_RIGHTUP;
                    break;
                case 2:
                    pos = GAMEPAD_HAT_RIGHT;
                    break;
                case 3:
                    pos = GAMEPAD_HAT_RIGHTDOWN;
                    break;
                case 4:
                    pos = GAMEPAD_HAT_DOWN;
                    break;
                case 5:
                    pos = GAMEPAD_HAT_LEFTDOWN;
                    break;
                case 6:
                    pos = GAMEPAD_HAT_LEFT;
                    break;
                case 7:
                    pos = GAMEPAD_HAT_LEFTUP;
                    break;
                default:
                    pos = GAMEPAD_HAT_CENTERED;
                    break;
            }
            
            if( pos != gamepad->hats[i] ) {
                gamepad->OnJoystickHatAction( i, pos);
            }
            
            element = element->pNext;
            ++i;
        }
    }
    
    void CXJoystick::Detect( IGamepad *gamepad )
    {
        recDevice *device = gpDeviceList;
        while( device ) {
            if( device->removed ) {
                device = FreeDevice( device );
            } else {
                device = device->pNext;
            }
        }
        
        while( CFRunLoopRunInMode( NEKO_GAMEPAD_RUNLOOP_MODE, 0, TRUE ) == kCFRunLoopRunHandledSource ) {
            /* no-op. Pending callbacks will fire in CFRunLoopRunInMode(). */
        }
    }
    
    void CXJoystick::Remove( IGamepad * gamepad )
    {
        while( FreeDevice(gpDeviceList) ) {
            /* spin */
        }
        
        if (hidman) {
            IOHIDManagerUnscheduleFromRunLoop(hidman, CFRunLoopGetCurrent(), NEKO_GAMEPAD_RUNLOOP_MODE);
            IOHIDManagerClose(hidman, kIOHIDOptionsTypeNone);
            CFRelease(hidman);
            hidman = NEKO_NULL;
        }
    }
    
    const char * CXJoystick::GetNameByIndex( const uint32_t index )
    {
        recDevice * device = GetDeviceForIndex(index);
        return device ? device->product : "UNKNOWN";
    }
    
    SIJoystickID CXJoystick::GetInstanceIdByIndex( const uint32_t index )
    {
        recDevice *device = GetDeviceForIndex(index);
        return device ? device->iInstanceId : 0;
    }
    
    JoystickGUID CXJoystick::GetDeviceGuid( const uint32_t index )
    {
        recDevice * device = GetDeviceForIndex( index );
        JoystickGUID guid;
        
        if( device ) {
            guid = device->guid;
        } else {
            memset( &guid, 0x00, sizeof(JoystickGUID) );
        }
        
        return guid;
    }
    
    JoystickGUID CXJoystick::GetJoystickGuid( Neko::IGamepad * gamepad )
    {
        return gamepad->pHardwareInfo->guid;
    }
    
    bool CXJoystick::IsJoystickAttached( Neko::IGamepad * gamepad )
    {
        
        return gamepad->pHardwareInfo != NEKO_NULL;
    }
}