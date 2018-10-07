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
//  Threading.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 10/11/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "Threading.h"

namespace Neko {
    
#   if !defined( _WIN32 )
    
    /**
     *  Thread method callback.
     *
     *  @param arg An argument to pass.
     *
     *  @return NEKO_NULL.
     */
    void * UnixThreadCallback( void * arg )
    {
        ThreadArgs * args = (ThreadArgs*)arg;
        
        // Callback method.
        (*args->m_callback) (args->m_thread, args->m_arg1, args->m_arg2);
        
        // Delete when method ends.
        delete args;
        return NEKO_NULL;
    }
    
    /**
     *  Thread constructor.
     */
    UnixThread::UnixThread( int32_t priority, int32_t threadNum, ThreadCallback callback, void * arg1, void ** args )
    {
        uint32_t i;
        
        // Use pthread property.
        
        pthread_attr_t attr;
        
        pthread_attr_init( &attr );
        pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );
        
        m_threadCount = threadNum;
        m_threads = new pthread_t[threadNum];
        
        for( i = 0; i < m_threadCount; ++i ) {
            ThreadArgs * threadArgs = new ThreadArgs;
            
            threadArgs->m_callback = callback;
            threadArgs->m_thread = this;
            
            // Set the arguments.
            threadArgs->m_arg1 = arg1;
            
            // See if we can use more arguments..
            if( args != NEKO_NULL ) {
                threadArgs->m_arg2 = args[i];
            } else {
                threadArgs->m_arg2 = NEKO_NULL;
            }
            
            // Create a thread.
            pthread_create( &m_threads[i], &attr, UnixThreadCallback, threadArgs );
        }
        
        // Destroy temporary thread attributes.
        pthread_attr_destroy( &attr );
    }
    
    /**
     *  Destructor.
     */
    UnixThread::~UnixThread()
    {
        delete m_threads;
        m_threads = NEKO_NULL;
    }
    
    /**
     *  Wait until thread is over.
     *
     *  @param msec Time in milliseconds.
     */
    void UnixThread::CallbackEnd( const float msec )
    {
        int32_t i;
        
        for( i = 0; i < m_threadCount; ++i ) {
            void * status = NEKO_NULL;
            
            // Join thread.
            pthread_join( m_threads[i], &status );
        }
    }
    
    /**
     *  Set thread priority.
     *
     *  @param priority Priority level.
     */
    void UnixThread::SetPriority( const int32_t priority )
    {
        printf( "UnixThread::SetPriority: Implement me!\n" );
        
//        int32_t i;
//        int32_t defaultPriority = SCHED_RR;
//        
//        switch( priority )
//        {
//            case PRIORITY_LOW:
//                defaultPriority = SCHED_OTHER;
//                break;
//            case PRIORITY_NORMAL:
//                defaultPriority = SCHED_RR;
//                break;
//            case PRIORITY_HIGH:
//                defaultPriority = SCHED_FIFO;  // Real time priority.
//                break;
//        }
//        
//        for( i = 0; i < m_threadCount; ++i )
//        {
//            struct sched_param param;
//            pthread_setschedparam( m_threads[i], defaultPriority, &param );
//        }
    }
    
    
    /*      Thread mutex methods.       */
    
    /**
     *  Create a new thread lock mutex.
     */
    UnixThreadLock::UnixThreadLock()
    {
        pthread_mutexattr_t attr;
        
        pthread_mutexattr_init( &attr );
        pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE );
        
        // Initialize thread mutex.
        pthread_mutex_init( &m_mutex, &attr );
        
        // Destroy attribute list.
        pthread_mutexattr_destroy( &attr );
    }
    
    /**
     *  Destructor.
     */
    UnixThreadLock::~UnixThreadLock()
    {
        pthread_mutex_destroy( &m_mutex );
    }
    
    /**
     *  Lock mutex.
     */
    void UnixThreadLock::Lock()
    {
        pthread_mutex_lock( &m_mutex );
    }
    
    /**
     *  Unlock mutex.
     */
    void UnixThreadLock::Unlock()
    {
        pthread_mutex_unlock( &m_mutex );
    }
    
    
    /*      Thread event methods.       */
    
    /**
     *  Create a new condition variable.
     */
    UnixThreadEvent::UnixThreadEvent()
    {
        pthread_mutex_init( &m_mutex, NEKO_NULL /* No arguments. */ );
        pthread_cond_init( &m_condition, NEKO_NULL );
    }
    
    /**
     *  Destroy a condition variable.
     */
    UnixThreadEvent::~UnixThreadEvent()
    {
        pthread_mutex_destroy( &m_mutex );
        pthread_cond_destroy( &m_condition );
    }
    
    /**
     *  Call the event.
     */
    void UnixThreadEvent::Signal()
    {
        pthread_mutex_lock( &m_mutex );
        pthread_cond_signal( &m_condition );
        pthread_mutex_unlock( &m_mutex );
    }
    
    /**
     *  Call events.
     */
    void UnixThreadEvent::Broadcast()
    {
        pthread_mutex_lock( &m_mutex );
        pthread_cond_broadcast( &m_condition );
        pthread_mutex_unlock( &m_mutex );
    }
    
    /**
     *  Wait when event will be called.
     *
     *  @param time A maximum time value.
     */
    void UnixThreadEvent::Wait( const float time )
    {
        pthread_mutex_lock( &m_mutex );
        pthread_cond_wait( &m_condition, &m_mutex );
        pthread_mutex_unlock( &m_mutex );
    }
    
    /// ============================================================================================
    
    /**
     *  Create a new condition variable.
     */
    UnixThreadEventLockless::UnixThreadEventLockless( UnixThreadLock * lock )
    {
        pthread_cond_init( &m_condition, NEKO_NULL );
    }
    
    /**
     *  Destroy a condition variable.
     */
    UnixThreadEventLockless::~UnixThreadEventLockless()
    {
        pthread_cond_destroy( &m_condition );
    }
    
    /**
     *  Call the event.
     */
    void UnixThreadEventLockless::Signal()
    {
        pthread_cond_signal( &m_condition );
    }
    
    /**
     *  Call the event.
     */
    void UnixThreadEventLockless::Broadcast()
    {
        pthread_cond_broadcast( &m_condition );
    }
    
    /**
     *  Wait when event will be called.
     *
     *  @param time A maximum time value.
     */
    void UnixThreadEventLockless::Wait( const float time )
    {
        pthread_cond_wait( &m_condition, &m_lock->m_mutex );
    }
    
    /// ============================================================================================
#   else
    /// ============================================================================================
    
    class WinThreadArgs
    {
    public:
        
        INekoThread     * m_pThread;
        ThreadCallback  m_Callback;
        
        void        * m_pArg1;
        void        * m_pArg2;
    };
    
    
    unsigned __stdcall WinThreadProc( void * argPtr )
    {
        WinThreadArgs * args = (WinThreadArgs *) argPtr;
        
        (*args->m_Callback) (args->m_pThread, args->m_pArg1, args->m_pArg2);
        delete args;
        
        return 0;
    }
    
    /**
     *  Create a new thread
     */
	Win64Thread::Win64Thread(const int32_t threadCount, const int priority, ThreadCallback callback, void * arg, void ** threadArgs)
    {
        m_iThreadCount = threadCount;
        m_Handles = new HANDLE[threadCount];
        
        for( int32_t i(0); i < m_iThreadCount; ++i ) {
            WinThreadArgs * args = new WinThreadArgs;
            
            args->m_pThread = this;
            args->m_Callback = callback;
            args->m_pArg1 = arg;
            
			if (threadArgs != NEKO_NULL){
				args->m_pArg2 = threadArgs[i];
            } else {
                args->m_pArg2 = NEKO_NULL;
            }
            
            m_Handles[i] = (HANDLE) _beginthreadex( NEKO_NULL, 0, WinThreadProc, args, CREATE_SUSPENDED, NEKO_NULL );
            SetPriority( priority );
            
            ResumeThread( m_Handles[i] );
        }
    }
    
    /**
     *  Destructor
     */
    Win64Thread::~Win64Thread()
    {
        for( int32_t i(0); i < m_iThreadCount; ++i ) {
            CloseHandle( m_Handles[i] );
        }
        
        delete m_Handles;
        m_Handles = NEKO_NULL;
    }
    
    /**
     *  Set thread priority.
     */
    void Win64Thread::SetPriority( const int priority )
    {
        int winPriority = THREAD_PRIORITY_NORMAL;
        switch( priority ) {
            case PRIORITY_LOW:
                winPriority = THREAD_PRIORITY_LOWEST;
                break;
                
            case PRIORITY_NORMAL:
                winPriority = THREAD_PRIORITY_NORMAL;
                break;
                
            case PRIORITY_HIGH:
                winPriority = THREAD_PRIORITY_HIGHEST;
                break;
        }
        
        for( int32_t i(0); i < m_iThreadCount; ++i ) {
            SetThreadPriority( m_Handles[i], winPriority );
        }
    }
    
    void Win64Thread::CallbackEnd( const float msec )
    {
        DWORD result = WaitForMultipleObjects( m_iThreadCount, m_Handles, true, (DWORD) msec );
        if( result != WAIT_OBJECT_0 ) {
            printf("thread shutdown wait failed.");
        }
    }
    
    /// ============================================================================================
    Win64Lock g_GlobalLock;
    
    /**
     *  Create a new lock
     */
    Win64Lock::Win64Lock()
    {
        InitializeCriticalSection( &m_section );
    }
    
    /**
     *  Destructor
     */
    Win64Lock::~Win64Lock()
    {
        DeleteCriticalSection( &m_section );
    }
    
    /**
     *  Lock it.
     */
    void Win64Lock::Lock()
    {
        EnterCriticalSection(  &m_section );
    }
    
    /**
     *  Unlock it.
     */
    void Win64Lock::Unlock()
    {
        LeaveCriticalSection( &m_section );
    }
    
    void GlobalLock()
    {
        g_GlobalLock.Lock();
    }
    
    void GlobalUnlock()
    {
        g_GlobalLock.Unlock();
    }
    
    /// ============================================================================================
    
    Win64Event::Win64Event()
    {
        m_Event = CreateEvent( NEKO_NULL, false, false, NEKO_NULL );
    }
    
    Win64Event::~Win64Event()
    {
        CloseHandle( m_Event );
    }
    
    void Win64Event::Signal()
    {
        SetEvent( m_Event );
    }
    
    void Win64Event::Broadcast()
    {
        PulseEvent( m_Event );
    }
    
    void Win64Event::Wait( float limit )
    {
        WaitForSingleObject( m_Event, (DWORD) limit );
    }
    
    /// ============================================================================================
    
#   endif
    
}