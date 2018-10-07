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
//  Threading.h
//  Neko engine
//
//  Created by Kawaii Neko on 10/11/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef Threading_hpp
#define Threading_hpp

#include "../Platform/Shared/SystemShared.h"

#   if !defined( _WIN64 )
        #include <pthread/pthread.h>
#	else
		#include <process.h>	// _beginthreadex
#   endif

namespace Neko {
    
    /// ============================================================================================
    /// System thread handler.
    class INekoThread
    {
    public:
        
        // Thread priorities.
        static const int32_t PRIORITY_LOW       = 0x1;
        static const int32_t PRIORITY_NORMAL    = 0x2;
        static const int32_t PRIORITY_HIGH      = 0x4;
        
        /**
         *  Wait until thread will be over.
         *
         *  @param msec Time in milliseconds.
         */
        virtual void                CallbackEnd( const float msec ) = 0;
        
        /**
         *  Set thread priority.
         *
         *  @param priority Priority level.
         */
        virtual void                SetPriority( const int32_t priority ) = 0;
    };
    
    // Thread callback method.
    typedef void (*ThreadCallback) ( INekoThread * thread, void * arg1, void * arg2 /* void arg3 */ );
    
    /// ============================================================================================
    /// Thread lock interface.
    class INekoThreadLock
    {
    public:
        
        /**
         *  Default lock destructor.
         */
        virtual ~INekoThreadLock() {
            
        }
        
        /**
         *  Lock.
         */
        virtual void                Lock() = 0;
        
        /**
         *  Unlock.
         */
        virtual void                Unlock() = 0;
    };
    
    /// ============================================================================================
    /// Thread events.
    class INekoThreadEvent
    {
    public:
        
        /**
         *  Destructor.
         */
        virtual ~INekoThreadEvent() {
            
        }
        
        /**
         *  Call the event.
         */
        virtual void                Signal() = 0;
        
        /**
         *  Call all events.
         */
        virtual void                Broadcast() = 0;
        
        /**
         *  Wait for the event call.
         *
         *  @param max A value when time exceedes by.
         */
        virtual void                Wait( const float max ) = 0;
        
    };
    
#   if !defined( _WIN32 ) // _WIN64 is same // Should get rid of ifdefs
    
    /// ============================================================================================
    /// A thread which uses pthread property.
    class UnixThread : public INekoThread
    {
    public:
        
        /**
         *  Thread constructor.
         */
        UnixThread( int32_t priority, int32_t threadNum, ThreadCallback callback, void * arg1, void ** args );
        
        /**
         *  Destructor.
         */
        virtual                 ~UnixThread();
        
        /**
         *  Wait until thread will be over.
         */
        void                CallbackEnd( const float msec );
        
        /**
         *  Set thread priority.
         *
         *  @param priority Priority level.
         */
        virtual void                SetPriority( const int32_t priority );
        
    private:
    protected:
        
        //! Thread amount.
        int32_t     m_threadCount;
        
        //! Threads.
        pthread_t   *m_threads;
    };
    
    /// ============================================================================================
    /// POSIX-style lock.
    class UnixThreadLock : public INekoThreadLock
    {
    public:
        
        /**
         *  Constructor.
         */
        UnixThreadLock();
        
        /**
         *  Destructor.
         */
        virtual                 ~UnixThreadLock();
        
        /**
         *  Lock.
         */
        virtual void                Lock();
        
        /**
         *  Unlock.
         */
        virtual void                Unlock();
        
//    private:
//    protected:
        
        //!  Thread mutex.
        pthread_mutex_t m_mutex;
    };
    
    /// ============================================================================================
    /// POSIX-style thread event.
    class UnixThreadEvent : public INekoThreadEvent
    {
    public:
        
        /**
         *  Constructor.
         */
        UnixThreadEvent();
        
        /**
         *  Destructor.
         */
        virtual                 ~UnixThreadEvent();
        
        /**
         *  Call the event.
         */
        virtual void                Signal();
        
        /**
         *  Call events.
         */
        virtual void                Broadcast();
        
        /**
         *  Wait when event will be called.
         *
         *  @param time Time.
         */
        virtual void                Wait( const float time );
    private:
    protected:
        
        //!  Condition variable.
        pthread_cond_t  m_condition;
        
        //!  Mutex lock.
        pthread_mutex_t m_mutex;
    };
    
    /// POSIX-style thread event lockless.
    class UnixThreadEventLockless : public INekoThreadEvent
    {
    public:
        
        /**
         *  Constructor.
         */
        UnixThreadEventLockless( UnixThreadLock * lock );
        
        /**
         *  Destructor.
         */
        virtual                 ~UnixThreadEventLockless();
        
        /**
         *  Call the event.
         */
        virtual void                Signal();
        
        /**
         *  Call events.
         */
        virtual void                Broadcast();
        
        /**
         *  Wait when event will be called.
         *
         *  @param time Time.
         */
        virtual void                Wait( const float time );
    private:
    protected:
        
        //!  Condition variable.
        pthread_cond_t  m_condition;
        
        //!  Mutex lock.
        UnixThreadLock * m_lock;
    };
    

#   else    // _WIN64
    
    /// Windows thread.
    class Win64Thread : public INekoThread
    {
    public:
        /**
         *  Constructor.
         */
		Win64Thread( const int32_t threadCount, const int priority, ThreadCallback callback,
                    void * arg, void ** args );
        
        /**
         *  Destructor.
         */
        virtual ~Win64Thread();
        
        
        /**
         *  Set thread priority.
         */
        virtual void                SetPriority( const int priority );
        
        void                CallbackEnd( const float msec );
        
    protected:
        
        // system stuff
        
        HANDLE  * m_Handles;
        int32_t m_iThreadCount;
        
        friend unsigned __stdcall WinThreadProc( void * threadObj );
    };
    
    /// Thread lock.
    class Win64Lock : public INekoThreadLock
    {
    public:
        
        /**
         *  Constructor.
         */
        Win64Lock();
        
        /**
         *  Destructor.
         */
        virtual ~Win64Lock();
        
        
        /**
         *  Lock it.
         */
        virtual void                Lock();
        
        /**
         *  Unlock it.
         */
        virtual void                Unlock();
    protected:
        
        CRITICAL_SECTION m_section;
    };
    
    /// Thread event.
    class Win64Event : public INekoThreadEvent
    {
    public:
        
        /**
         *  Constructor.
         */
        Win64Event();
        
        /**
         *  Constructor
         */
        virtual ~Win64Event();
        
        
        /**
         *  Signal the event
         */
        virtual void                Signal();
        
        /**
         *  Signal all events.
         */
        virtual void                Broadcast();
        
        /**
         *  Wait for the event to be called
         */
        virtual void                Wait( float limit );
        
    protected:
        HANDLE m_Event;
    };
    
#   endif
    
    /// ============================================================================================
    ///  Thread arguments.
    struct ThreadArgs
    {
    public:
        INekoThread      * m_thread;
        ThreadCallback  m_callback;
        
        void    * m_arg1;
        void    * m_arg2;
    };

}

#endif /* Threading_hpp */
