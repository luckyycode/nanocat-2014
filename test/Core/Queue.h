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
//  Queue.hpp
//  Neko engine
//
//  Created by Kawaii Neko on 4/16/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#ifndef Queues_hpp
#define Queues_hpp

#include "Core.h"
#include "Threading.h"

namespace Neko {
    
    /// Queue status codes.
    enum /*class*/ EQueueStatus
    {
        Ok = 0,
        Invalid = -1,
        LockError = -2,
        NoData = -3,
        InvalidElem = -4,
        InvalidCb = -5
    } ;
    
    struct queue_element_t
    {
        void *data;
        struct queue_element_t *next;
    };
    
    /// Queue system
    class CQueue
    {
    public:
        
        /**
         *  Create a queue.
         */
        void                Create( uint16_t max_elements = 0 );
        void                CreateSorted( int8_t asc, int (*cmp)(void *, void *) ); //!<
        void                CreateSortedLimited(uint16_t max_elements, int8_t asc, int (*cmp)(void *, void *)); //!<
        
        
        /**
         *  Destroy queue.
         */
        int8_t              Destroy( uint8_t fd, void (*ff)(void *));
        
        
        /**
         *  Flushes queue and removes all elements.
         */
        int8_t              FlushInternal(uint8_t fd, void (*ff)(void *));
        int8_t               Flush();
        
        /**
         *  Lock queue.
         */
        int8_t              LockInternal();
        
        /**
         *  Unlock it.
         */
        int8_t              UnlockInternal();
        
        
        /**
         *  Element count in queue.
         */
        uint16_t            GetCount();
        
        /**
         *  Is queue empty?
         */
        int8_t              IsEmpty();
        
        /**
         *  Put a new element to queue.
         */
        int8_t             Put( void * e );
        
        /**
         *  Wait till new data set or queue hit its limit or queue will be removed.
         */
        uint8_t         PutWait( void * e );
        
        /**
         *  Get first element in queue.
         */
        int8_t              Get( void ** e );
        int8_t              GetWait( void ** e );
        int8_t              GetInternal( void **e, int8_t hasAction, int (*cmp)(void *, void *), void *cmpel );
        
        
        /**
         *  Destroy queue and free memory.
         */
        int8_t              DestroyComplete( void (*ff)(void *));
        
        /**
         *  Flush queue and free memory.
         */
        int8_t              FlushComplete( void (*ff)(void *));

        /**
         * adds an element to the queue.
         * when action is NEKO_NULL the function returns with an error code.
         * queue _has_ to be locked.
         *
         * q - the queue
         * el - the element
         * action - specifies what should be executed if max_elements is reached.
         *
         * returns < 0 => error, 0 okay
         */
        int8_t              PutInternal( void *el, int8_t hasAction );
        
        
        /**
         *  Sets if the queue will accept new data.
         */
        int8_t              SetNewData( uint8_t v );
        
        /**
         *  Check if queue can accept new data.
         */
        uint8_t              GetNewData();
        
        
        queue_element_t     * first_el, * last_el;
        
        //! Max number of elements.
        uint16_t num_els;
        uint16_t max_els;
        
        //! Data flag.
        uint8_t new_data;
        
        //! Sorted queue.
        int8_t sort;
        int8_t asc_order;
        
        int     (*cmp_el)(void *, void *);
        
        // multithreaded
        INekoThreadLock     * m_pConditionLock;
        INekoThreadEvent    * m_pConditionPut, * m_pConditionGet;
        
    };

}

#endif /* Queue_hpp */
