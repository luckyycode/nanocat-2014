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
//
//  System.h
//
//  OS-depend system things.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef system_h
#define system_h

#include "../../Core/Core.h"
#include "../../Core/CoreDef.h"

// System manager.

namespace Neko {

    ///  Syxstem message box types.
    enum ncSysMessageBoxType
    {
        MB_INFO = 0,
        MB_ERROR,
        MB_WARN,
        MB_NONE,
        MB_FUN
    } ;

    static const int32_t MAX_USERNAME_LENGTH = 512;
    
    ///  System manager.
    class CSystem
    {
    public:
        
        /**
         *  Constructor.
         */
        CSystem();
        //~CSystem();

        
        /**
         *  Initialize system properties.
         */
        void                Initialize();
        void                Frame();
        
        /**
         *  Totally quit the application.
         */
        void                Quit();
        
        /**
         *  Update system usage stats.
         */
        void                UpdateStats( GameMemory * game_memory );

        /**
         *  Get system current user name.
         */
        const char *                GetCurrentUsername();
        
#   if defined( _WIN32 ) // gettimeofday implementation
	#   if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
            #define DELTA_EPOCH_IN_MICROSECS  116444736000000000Ui64 // CORRECT
	#   else
            #define DELTA_EPOCH_IN_MICROSECS  116444736000000000ULL // CORRECT
	#   endif


		/* FILETIME of Jan 1 1970 00:00:00. */
		static const unsigned __int64 epoch = ((unsigned __int64)116444736000000000ULL);

		/*
		 * timezone information is stored outside the kernel so tzp isn't used anymore.
		 *
		 * Note: this function is not for Win32 high precision timing purpose. See
		 * elapsed_time().
		*/
		inline int gettimeofday(struct timeval * tp, struct timezone * tzp)
		{
				FILETIME    file_time;
				SYSTEMTIME  system_time;
				ULARGE_INTEGER ularge;

				GetSystemTime(&system_time);
				SystemTimeToFileTime(&system_time, &file_time);
				ularge.LowPart = file_time.dwLowDateTime;
				ularge.HighPart = file_time.dwHighDateTime;

				tp->tv_sec = (long)((ularge.QuadPart - epoch) / 10000000L);
				tp->tv_usec = (long)(system_time.wMilliseconds * 1000);

				return 0;
			}
#   endif

        /**
         *  Get current time in milliseconds.
         */
        inline uint32_t Milliseconds( void )
        {
            static unsigned long _systemTime = false;
            struct timeval Current;
            
            gettimeofday( &Current, NEKO_NULL );
            
            if ( !_systemTime ) {
                _systemTime = Current.tv_sec;
                return Current.tv_usec / 1000;
            }
            
            _curtime = (uint32_t)(Current.tv_sec - _systemTime) * 1000 + Current.tv_usec / 1000;
            
            return _curtime;
        }
        
        /**
         *  Show system information.
         */
        void                ShowInfo();

        void                PrintSystemTime();

        static void                 MessageBox( ncSysMessageBoxType mType, const char *mTitle, const char *mMessage );
        static void                 HideWindow();

        void                ToggleMouse( bool mEnable ) ;
        
    private:

        int32_t     m_iCPUCores;
        int32_t     m_iCPUSpeed;
        
        int32_t     m_iPhysMemory, m_iVirtMemory;
        
        const int               GetSysCTLValue( const char key[], void *dest );
        const unsigned long                 GetSysFreeMemory();

        const char *                GetProcessorName();


//        ncDeviceType    m_CurrentDevice;

    protected:
    private:
        
        unsigned long _systemTime;
        int32_t     _curtime;
    };
}

#endif
