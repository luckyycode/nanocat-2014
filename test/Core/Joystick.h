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
//  Joystick.h
//  Neko engine
//
//  Created by Kawaii Neko on 3/17/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Joystick_h
#define Joystick_h

#include "../Platform/Shared/SystemShared.h"
#include "../Math/Vec2.h"
#include "Console/ConsoleVariable.h"
#include "Utilities/List.h"
#include "CoreDef.h"

namespace Neko {

    /// Joystick hardware guid.
    struct JoystickGUID {
        uint8_t     data[16];
    };
    
    /// Controller axes.
    enum class EGameControllerAxis : int8_t
    {
        Invalid = -1,
        
        LeftX,
        LeftY,
        RightX,
        RightY,
        TriggerLeft,
        TriggerRight,
        
        Max
    };
    
    /// Controller buttons.
    enum class EGameControllerButton : int8_t
    {
        Invalid = -1,
        
        ButtonA,
        ButtonB,
        ButtonX,
        ButtonY,
        ButtonBack,
        ButtonGuide,
        ButtonStart,
        
        LeftStick,
        RightStick,
        
        LeftShoulder,
        RightShoulder,
        
        DpadUp,
        DpadDown,
        DpadLeft,
        DpadRight,
        
        Max
    };
    
    /// Bind type.
    enum class EGameControllerBindType : int8_t
    {
        None = 0,
        
        Button,
        Axis,
        Hat,
        
        Max
    };
    
    /// Joystick battery level.
    enum class EJoystickPowerLevel : int8_t
    {
        Unknown = -1,
        Empty,
        Low,
        Medium,
        Full,
        
        Wired,
        
        MaxValue
    } ;
    
    /// Joystick ID (not hardware)
    typedef short int  SIJoystickID;;
    
#define kMaxEntries             20
#define kMaxHatEntries          0x3f + 1
    
    /// Button bind pair.
    struct EGameControllerButtonBind
    {
        union {
            int8_t button;
            int8_t axis;
            
            struct {
                int16_t hat;
                int8_t hat_mask;
            } hat;
        } value;
        
        EGameControllerBindType bindType;
        
    };

    struct SJoystickHardwareInfo; // platform depend
    class IGamepadController;
    
    /// ============================================================================================
    /// Gamepad ( joystick ) class.
    class IGamepad
    {
        NEKO_NONCOPYABLE( IGamepad );
        
    public:
        
        IGamepad() : iRefCount(0), bForceReset(false), hats(NEKO_NULL), balls(NEKO_NULL), axes(NEKO_NULL), buttons(NEKO_NULL),
                        iAxesNum(0), iBallsNum(0), iButtonNum(0), iHatsNum(0), pHardwareInfo(NEKO_NULL)
        {

        }
        
        virtual ~IGamepad() {
            
        }
        
        /**
         *  Initialize the gamepad.
         */
        void                Init( const int32_t deviceIndex );
        IGamepad *          InitSearch( const int32_t deviceIndex );
        
        /**
         *  Close the gamepad.
         */
        void                Close();
        
        
        /**
         *  Get hat value.
         */
        uint8_t               GetHat( int32_t hat );
        
        /**
         *  Get axis value.
         */
        int16_t              GetAxis( int32_t axis );
        
        /**
         *  Get button value.
         */
        uint8_t               GetButton( int32_t button );
        
        /**
         *  Get ball value.
         */
        int32_t             GetBall( int32_t ball, int32_t * x, int32_t * y );

        
        
        /**
         *  Attached game controller.
         */
        bool                GetAttached();
        
        void                OnJoystickAxisAction( uint8_t axis, int16_t value );
		void                OnJoystickButtonAction(uint8_t button, uint8_t state);
		void                OnJoystickHatAction(uint8_t hat, uint8_t value);
        
        
        void                Remove();

        /**
         * Get the friendly name of this joystick.
         */
        const char *                GetJoystickName() {       return name;    }
        
        /**
         * Get the instance id for this opened joystick
         */
        SIJoystickID                GetInstanceID()   const {       return iInstanceId;   }
        
//    private:
        
        //! Device instance.
        SIJoystickID    iInstanceId;
        
        //! Gamepad name.
        char *      name;
        
        //! Number of axes in device.
        int32_t         iAxesNum;
        //! Device axes.
        int16_t *       axes;
        
        //! Number of hats in device.
        int32_t         iHatsNum;
        //! Device hats.
        uint8_t *       hats;
        
        //! Number of balls in device.
        int32_t         iBallsNum;
        //! Device balls.
        Vec2i *        balls;
        
        //! Number of buttons in device.
        int32_t         iButtonNum;
        //! Device buttons.
        uint8_t *       buttons;
        
        int32_t         iRefCount;
        
        //! Platform depend joystick information.
        SJoystickHardwareInfo     *    pHardwareInfo;
        
        bool        bForceReset;
        
        //! Next entry.
        SLink   m_Link;
        
        //! Joystick power level.
        EJoystickPowerLevel     powerLevel;
    };
    
    enum StaticInfo
    {
        MaxAxes = static_cast<int32_t>(EGameControllerAxis::Max),
        MaxButtons = static_cast<int32_t>(EGameControllerButton::Max),
    };
    
    /// ============================================================================================
    /// Controller map binding.
    struct SControllerMapping
    {
        JoystickGUID    guid;
        const char * name;
        
        //! Raw buttons
        int32_t     iButtons[StaticInfo::MaxButtons];
        //! Raw axes as buttons.
        int32_t     iAxesButton[StaticInfo::MaxButtons];
        
        //! Raw axes.
        int32_t     iAxes[StaticInfo::MaxAxes];
        //! Raw buttons as axis.
        int32_t     iButtonAxix[StaticInfo::MaxAxes];
        
        struct SHatMapping {
            int32_t hat;
            uint8_t mask;   // state
        };
        
        SHatMapping buttonHat[MaxButtons];
        
        EGameControllerAxis controllerAxes[kMaxEntries];
        EGameControllerAxis controllerButtonAxis[kMaxEntries];
        
        EGameControllerButton controllerButton[kMaxEntries];
        EGameControllerButton controllerAxisButton[kMaxEntries];
        EGameControllerButton controllerHatButton[kMaxHatEntries];
        
    };
    
    /// ============================================================================================
    /// Game controller interface for multiple platforms.
    class IGamepadSystemInterface
    {
    public:
        
        IGamepadSystemInterface() { }

        virtual ~IGamepadSystemInterface() { }
        
        /**
         *  Initialize interface.
         */
        virtual int32_t             Init() = 0;
        
        /**
         *  Open joystick.
         */
        virtual int32_t             Open( IGamepad * gamepad, int32_t deviceIndex ) = 0;
        
        /**
         *  Close joystick.
         */
        virtual void                Close( IGamepad * gamepad ) = 0;
        
        /**
         *  Update joystick.
         */
        virtual void                Update( IGamepad * gamepad ) = 0;
        
        /**
         *  Detect any connected joystick.
         */
        virtual void                Detect( IGamepad * gamepad ) = 0;
        
        /**
         *  Remove joystick.
         */
        virtual void                Remove( IGamepad * gamepad ) = 0;
        
        
        /**
         *  Number of joysticks in the system.
         */
        virtual int32_t             GetNumJoysticks() = 0;
        
        
        /**
         *  Get joystick name by device index.
         */
        virtual const char *                GetNameByIndex( const uint32_t index ) = 0;
        virtual SIJoystickID            GetInstanceIdByIndex( const uint32_t index ) = 0;
        virtual JoystickGUID            GetDeviceGuid( const uint32_t index ) = 0;
        virtual JoystickGUID            GetJoystickGuid( IGamepad * gamepad ) = 0;
        
        virtual bool                IsJoystickAttached( IGamepad * gamepad ) = 0;
        
    };
    
    /// Mapping item.
    struct SControllerMappingItem
    {
        char *  name;
        char *  mapping;
        
        JoystickGUID    guid;
        SControllerMappingItem * next;
    } ;
    
    
    /// ============================================================================================
    /// Controller system.
    class CGameControllerBase
    {
        NEKO_NONCOPYABLE( CGameControllerBase );
        
    public:
        
        /**
         *  Initialize system.
         */
        void                Init();
        
        /**
         *  Update gamepads.
         */
        void                UpdateGamepads();
        
        /**
         *  Check if the requested device is controller.
         */
        bool                IsGameController( int32_t deviceIndex );
        
        /**
         *  Get number of joysticks in the system.
         */
        const uint32_t              GetNumJoysticks() const { return pSysJoystick->GetNumJoysticks();   }
        
        /**
         *  Remove controller.
         */
        void                RemoveControllers();
        
        
        //! System game controller interface.
        IGamepadSystemInterface     * pSysJoystick;
    };
    
    /// ============================================================================================
    /// Gamepad controller interface.
    class IGamepadController
    {
    public:
        
        /**
         *  Initialize game controller.
         */
        void                Initialize();
        
        /**
         *  Open controller at index.
         */
        void                OpenController( const int32_t deviceIndex );

        /**
         *  Get bind for button.
         */
        EGameControllerButtonBind                   GetBindForButton( EGameControllerButton button);
        
        /**
         *  Get bind for axis.
         */
        EGameControllerButtonBind                   GetBindForAxis( EGameControllerAxis axis);
        
        
        /**
         *  Get button id.
         */
        uint8_t             GetButton( EGameControllerButton button );
        
        /**
         *  Get axis id.
         */
        int16_t             GetAxis( EGameControllerAxis axis );
        
        
        
        /**
         *  Get the mapped axis.
         */
        int16_t                 GetAxisMapped( EGameControllerAxis axis );
        
        /**
         *  Get the mapped button.
         */
        uint8_t                 GetButtonMapped( EGameControllerButton button );
        
        /**
         *  Close controller.
         */
        void                CloseController();
        
        /**
         *  Is current game controller attached to joystick?
         */
        const bool                IsGetAttached() const {      return joystick->GetAttached(); }
        
        /**
         *  Get controller name.
         */
        const char *                GetName() {     return mapping.name;    }
        
        IGamepadSystemInterface     * pSysJoystick;
        
        //! Joystick attached.
        IGamepad *  joystick;
        int32_t     iRefCount;
        
        //! Hat states.
        uint8_t       hatState[4];
        
        //! Current controller mapping.
        SControllerMapping      mapping;
        
        SLink    m_Link;
    };
    
    //! Controllers list.
    static SList    m_ControllerList;
    //! Joystick list.
    static SList    m_JoystickList;

#define GAMEPAD_HAT_CENTERED    0x00
#define GAMEPAD_HAT_UP          0x01
#define GAMEPAD_HAT_RIGHT       0x02
#define GAMEPAD_HAT_DOWN        0x04
#define GAMEPAD_HAT_LEFT        0x08
#define GAMEPAD_HAT_RIGHTUP     (GAMEPAD_HAT_RIGHT | GAMEPAD_HAT_UP)
#define GAMEPAD_HAT_RIGHTDOWN   (GAMEPAD_HAT_RIGHT | GAMEPAD_HAT_DOWN)
#define GAMEPAD_HAT_LEFTUP      (GAMEPAD_HAT_LEFT | GAMEPAD_HAT_UP)
#define GAMEPAD_HAT_LEFTDOWN    (GAMEPAD_HAT_LEFT | GAMEPAD_HAT_DOWN)
    
    extern IGamepadController * CheckControllerExistance( const int32_t deviceIndex );
    
    extern SConsoleVar     * Gamepad_Preset;
}


#endif /* Joystick_h */
