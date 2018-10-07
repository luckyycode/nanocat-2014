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
//  List.h
//  Array list implementation.
//
//  This file is a part of Neko engine.
//  Created by Neko Code on 2/17/15.
//  Copyright (c) 2015 Neko Vision. All rights reserved.
//

#ifndef __Nanocat_List__
#define __Nanocat_List__

#include "../GameMemory.h"

namespace Neko {

    class SLink;
    
    /// Simple object linker.
    class SimpleLink
    {
    public:
        
        /**
         *  Create object linker.
         */
        inline void Create()
        {
            Reset();
        }
        
        /**
         *  Remove object linker.
         */
        inline void                 Destroy();
        
        /**
         *  Add an object after node.
         */
        inline void                 Pop( SimpleLink * link );
        
        /**
         *  Add an object before node.
         */
        inline void                 Push( SimpleLink * link );
        
        /**
         *  Reset linker.
         */
        inline void Reset()
        {
            m_pPrev = m_pNext = (SLink*) this;
        }
        
        /**
         *  Array accessor.
         */
        inline SLink *              operator [] ( size_t at );
        
        /**
         *  Are our nodes last?
         */
        inline bool                 IsLast();
        
    private:
    public:
        
        /**
         *  Linker nodes.
         */
        SLink * m_pPrev, * m_pNext;
    };
    
    /// Object linker.
    class SLink : public SimpleLink
    {
    public:
        /**
         *  Constructor.
         */
        inline SLink( bool needsClean = false )
        {
            if( needsClean == true ) {
                Reset();
            }
        }
        
        inline void InitData( void * data )
        {
            Reset();
            m_ptrData = data;
        }
        
        void * m_ptrData;
    };
    
    /**
     *  Remove object linker.
     */
    inline void SimpleLink::Destroy()
    {
        m_pPrev->m_pNext = m_pNext;
        m_pNext->m_pPrev = m_pPrev;
        
        Reset();
    }
    
    /**
     *  Add an object after node.
     */
    inline void SimpleLink::Pop( SimpleLink * link )
    {
        link->m_pPrev = (SLink*) this;
        link->m_pNext = m_pNext;
        link->m_pPrev->m_pNext = link->m_pNext->m_pPrev = (SLink *)link;
    }
    
    /**
     *  Add an object before node.
     */
    inline void SimpleLink::Push( SimpleLink * link )
    {
        link->m_pPrev = m_pPrev;
        link->m_pNext = (SLink *) this;
        link->m_pPrev->m_pNext = link->m_pNext->m_pPrev = (SLink *) link;
    }
    
    /**
     *  Array accessor.
     */
    inline SLink * SimpleLink::operator[]( size_t at )
    {
        return ((SLink**) this)[at];
    }
    
    /**
     *  Are our nodes last?
     */
    inline bool SimpleLink::IsLast()
    {
        return (m_pPrev == this && m_pNext == this);
    }

    /// Node based list ( without allocations ).
    class SList
    {
    public:
        
        /**
         *  Initialize a new list.
         */
        inline static void CreateList( SList * list )
        {
            list->m_sList.Reset();
            list->m_iCount = 0;
        }
        
        /**
         *  Insert link.
         */
        inline static void Insert( SimpleLink * after, SimpleLink * link )
        {
            after->Pop( link );
        }
        
        /**
         *  Add object after node.
         */
        inline static void Pop( SList * list, SLink * after, SLink * link, void * data )
        {
            link->m_ptrData = data;
            Insert( after, link );
            ++list->m_iCount;
        }
        
        /**
         *  Add node head.
         */
        inline static void AddHead( SList * list, SLink * link, void * data )
        {
            Pop( list, &list->m_sList, link, data );
        }
        
        /**
         *  Add node tail.
         */
        inline static void AddTail( SList * list, SLink * link, void * data )
        {
            Pop( list, list->m_sList.m_pPrev, link, data );
        }
        
        /**
         *  Remove link at index.
         */
        inline static void RemoveAt( SList * list, SLink * link )
        {
            link->Destroy();
            --list->m_iCount;
        }
        
        /**
         *  Constructor.
         */
        inline SList()
        {
            m_sList.Reset();
            m_iCount = 0;
        }
        
        
    private:
    public:
        
        /**
         *  List linker.
         */
        SLink   m_sList;
        
        /**
         *  Objects in our list.
         */
        size_t  m_iCount;
    };
}

#endif