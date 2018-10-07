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
//  Queue.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 4/16/16.
//  Copyright © 2016 Neko Vision. All rights reserved.
//

#include "Queue.h"
#include "Core.h"

namespace Neko {
    
    /**
     *  Lock queue.
     */
    int8_t CQueue::LockInternal()
    {
        // all errors are unrecoverable for us
        m_pConditionLock->Lock();
        
        return EQueueStatus::Ok;
    }

    /**
     *  Unlock it.
     */
    int8_t CQueue::UnlockInternal()
    {
        // all errors are unrecoverable for us
        m_pConditionLock->Unlock();
        
        return EQueueStatus::Ok;
    }
    
    /**
     *  Destroy queue.
     */
    int8_t CQueue::Destroy( uint8_t fd, void (*ff)(void *) )
    {
        // firstly deallocate all data
        int error = EQueueStatus::Ok;
        // make sure no new data comes and wake all waiting threads
        error = SetNewData( 0 );
        error = LockInternal();
        
        // release internal element memory
        error = FlushInternal( fd, ff );
        
        // destroy lock and queue etc
        g_Core->KillEvent( m_pConditionGet );
        g_Core->KillEvent( m_pConditionPut );
        
        error = UnlockInternal();
        g_Core->KillLock( m_pConditionLock );
        
        return error;
    }
    
    /**
     *  Flush queue.
     *
     *  @param fd <#fd description#>
     *  @param ff <#ff description#>
     *
     *  @return <#return value description#>
     */
    int8_t CQueue::FlushInternal(uint8_t fd, void (*ff)(void *))
    {
        queue_element_t *qe = first_el;
        queue_element_t *nqe = NEKO_NULL;
        
        while( qe != NEKO_NULL ) {
            nqe = qe->next;
            if( fd != 0 && ff == NEKO_NULL ) {
                free(qe->data);
            } else if( fd != 0 && ff != NEKO_NULL ) {
                ff(qe->data);
            }
            
            free(qe);
            qe = nqe;
        }
        
        first_el = NEKO_NULL;
        last_el = NEKO_NULL;
        num_els = 0;
        
        return EQueueStatus::Ok;
    }
    
    /**
     *  Put element into queue.
     */
    int8_t CQueue::PutInternal( void *el, int8_t hasAction )
    {
        if( new_data == 0 ) { // no new data allowed
            return EQueueStatus::NoData;
        }
        
        // max_elements already reached?
        // if condition _needs_ to be in sync with while loop below!
        if( num_els == (UINT16_MAX - 1) || (max_els != 0 && num_els == max_els) ) {
            if( hasAction == 0 ) {
                return EQueueStatus::NoData;
            } else {
                while( (num_els == (UINT16_MAX - 1) || (max_els != 0 && num_els == max_els)) && new_data != 0 ) {
                    m_pConditionPut->Signal();
                }
                if( new_data == 0 ) {
                    return EQueueStatus::NoData;
                }
            }
        }
        
        queue_element_t * new_el = (queue_element_t *)malloc( sizeof(queue_element_t) );
        if( new_el == NEKO_NULL ) {
            return -1;
        }
        
        new_el->data = el;
        new_el->next = NEKO_NULL;
        
        if( sort == 0 || first_el == NEKO_NULL ) {
            // insert at the end when we don't want to sort or the queue is empty
            if( last_el == NEKO_NULL ) {
                first_el = new_el;
            } else {
                last_el->next = new_el;
            }
            
            last_el = new_el;
        } else {
            // search appropriate place to sort element in
            queue_element_t *s = first_el; // s != NEKO_NULL, because of if condition above
            queue_element_t *t = NEKO_NULL;
            
            int32_t asc_first_el = asc_order != 0 && cmp_el( s->data, el ) >= 0;
            int32_t desc_first_el = asc_order == 0 && cmp_el( s->data, el ) <= 0;
            
            if( asc_first_el == 0 && desc_first_el == 0 ) {
                // element will be inserted between s and t
                for( s = first_el, t = s->next; s != NEKO_NULL && t != NEKO_NULL; s = t, t = t->next ) {
                    if( asc_order != 0 && cmp_el(s->data, el) <= 0 && cmp_el(el, t->data) <= 0 ) {
                        // asc: s <= e <= t
                        break;
                    } else if( asc_order == 0 && cmp_el(s->data, el) >= 0 && cmp_el(el, t->data) >= 0 ) {
                        // desc: s >= e >= t
                        break;
                    }
                }
                
                s->next = new_el;
                new_el->next = t;
                
                if( t == NEKO_NULL ) {
                    last_el = new_el;
                }
            } else if( asc_first_el != 0 || desc_first_el != 0 ) {
                // add at front
                new_el->next = first_el;
                first_el = new_el;
            }
        }
        
        ++num_els;
        // notify only one waiting thread, so that we don't have to check and fall to sleep because we were to slow
        m_pConditionGet->Signal();
        
        return EQueueStatus::Ok;
    }
    
    /**
     *  Get element from queue.
     */
    int8_t CQueue::GetInternal(void **e, int8_t hasAction, int (*cmp)(void *, void *), void *cmpel )
    {
        // are elements in the queue?
        if( num_els == 0 ) {
            if( hasAction == 0 ) {
                *e = NEKO_NULL;
                return EQueueStatus::NoData;
            } else {
                while( num_els == 0 && new_data != 0 ) {
                    m_pConditionGet->Wait( 1000.0f );
                }
                
                if( num_els == 0 && new_data == 0 ) {
                    return EQueueStatus::NoData;
                }
            }
        }
        
        // get first element (which fulfills the requirements)
        queue_element_t * el_prev = NEKO_NULL, * el = first_el;
        while( cmp != NEKO_NULL && el != NEKO_NULL && 0 != cmp(el, cmpel) ) {
            el_prev = el;
            el = el->next;
        }
        
        if( el != NEKO_NULL && el_prev == NEKO_NULL ) {
            // first element is removed
            first_el = el->next;
            
            if( first_el == NEKO_NULL ) {
                last_el = NEKO_NULL;
            }
            
            --num_els;
            *e = el->data;
            
            free(el);
        } else if( el != NEKO_NULL && el_prev != NEKO_NULL ) {
            // element in the middle is removed
            el_prev->next = el->next;
            
            --num_els;
            *e = el->data;
            
            free(el);
        } else {
            // element is invalid
            *e = NEKO_NULL;
            return EQueueStatus::InvalidElem;
        }
        
        // notify only one waiting thread
        m_pConditionPut->Signal();
        
        return EQueueStatus::Ok;
    }
    
    /**
     *  Create a new queue.
     */
    void CQueue::Create( uint16_t max_elements )
    {
        m_pConditionLock = g_Core->CreateLock();
        
        m_pConditionGet = g_Core->CreateEvent( m_pConditionLock );
        m_pConditionPut = g_Core->CreateEvent( m_pConditionLock );
        
        first_el = NEKO_NULL;
        last_el = NEKO_NULL;
        num_els = 0;
        max_els = 0;
        new_data = 1;
        sort = 0;
        asc_order = 1;
        cmp_el = NEKO_NULL;
    }
    
    /**
     *  Create sorted queue.
     */
    void  CQueue::CreateSorted( int8_t asc, int (*cmp)(void *, void *) )
    {
        if( cmp == NEKO_NULL ) {
            return;
        }
        
        Create();
        
        sort = 1;
        asc_order = asc;
        cmp_el = cmp;
    }
    
    /**
     *  Create sorted queue with limit.
     */
    void  CQueue::CreateSortedLimited( uint16_t max_elements, int8_t asc, int (*cmp)(void *, void *) )
    {
        if( cmp == NEKO_NULL ) {
            return;
        }
        
        Create();
        
        max_els = max_elements;
        sort = 1;
        asc_order = asc;
        cmp_el = cmp;
    }
    
    /**
     *  Destroy queue.
     */
    int8_t CQueue::DestroyComplete( void (*ff)(void *) )
    {
        return Destroy( 1, ff );
    }
    
    /**
     *  Flush queue.
     */
    int8_t CQueue::Flush()
    {
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        int8_t r = FlushInternal( 0, NEKO_NULL );
        
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        return r;
    }
    
    /**
     *  Flush queue.
     */
    int8_t CQueue::FlushComplete( void (*ff)(void *) )
    {
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        int8_t r = FlushInternal( 1, ff );
        
        if( UnlockInternal() != 0 )  {
            return EQueueStatus::LockError;
        }
        
        return r;
    }
    
    /**
     *  Get element count in queue.
     */
    uint16_t CQueue::GetCount()
    {
        uint16_t ret = 0;//UINT16_MAX;
        
//        if( LockInternal() != 0 ) {
//            return ret;
//        }
        
        ret = num_els;
        
//        if( UnlockInternal() != 0 ) {
//            return EQueueStatus::LockError;
//        }
        
        return ret;
    }
    
    /**
     *  Is queue empty?
     */
    int8_t CQueue::IsEmpty()
    {
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        uint8_t ret;
        if( first_el == NEKO_NULL || last_el == NEKO_NULL ) {
            ret = 1;
        } else {
            ret = 0;
        }
        
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        return ret;
    }
    
    /**
     *  Allow queue to get new data.
     */
    int8_t CQueue::SetNewData( uint8_t v )
    {
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        new_data = v;
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        if( new_data == 0 ) {
            // notify waiting threads, when new data isn't accepted
            m_pConditionPut->Signal();
            m_pConditionGet->Signal();
        }
        
        return EQueueStatus::Ok;
    }
    
    /**
     *  Is it allowed to get a new data?
     */
    uint8_t CQueue::GetNewData()
    {
        if( LockInternal() != 0 ) {
            return 0;
        }
        
        uint8_t r = new_data;
        
        if( UnlockInternal() != 0 ) {
            return 0;
        }
        
        return r;
    }
    
    /**
     *  Put element into queue.
     */
    int8_t CQueue::Put( void * el )
    {
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        int8_t r = PutInternal( el, 0 );
        
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        return r;
    }
    
    /**
     *  Put element into queue and wait.
     */
    uint8_t CQueue::PutWait( void * el )
    {
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        int8_t r = PutInternal( el, 1 );
        
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        return r;
    }
    
    /**
     *  Get last element from queue.
     */
    int8_t CQueue::Get( void ** e )
    {
        *e = NEKO_NULL;
        if( LockInternal() !=  0 ) {
            return EQueueStatus::LockError;
        }
        
        int8_t r = GetInternal( e, 0, NEKO_NULL, NEKO_NULL );
        
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        return r;
    }
    
    /**
     *  Get last element from queue and wait.
     */
    int8_t CQueue::GetWait( void ** e )
    {
        *e = NEKO_NULL;
        
        if( LockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        int8_t r = GetInternal( e, 1, NEKO_NULL, NEKO_NULL );
        
        if( UnlockInternal() != 0 ) {
            return EQueueStatus::LockError;
        }
        
        return r;
    }
}