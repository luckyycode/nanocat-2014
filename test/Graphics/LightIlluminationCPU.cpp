//
//  LightIllumination.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 12/11/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "LightIlluminationCPU.h"
#include "../Core/Core.h"
#include "../Core/Utilities/VectorList.h"
#include "../Core/Utilities/List.h"
#include "../Core/Physics/PlayerPhysics.h"  // Ray tracing.

namespace Neko {
    
    // TODO!
    // TODO VPL.
    
    /// Global illumination indirect light.
    class IndirectLight
    {
    public:
        
        //! Source position.
        Vec3   position;
        
        //! Source color.
        Vec3   radiant;
        
        //! Source light radius.
        float   radius;
        
        //! Update interval.
        int32_t    updateInterval;
        
        //! Last update time.
        int32_t    time;
        
        //! Amount of photon.
        int32_t     photonCount;
        
        //! Amount of reflected photon.
        int32_t     photonReflectionCount;
        
        SLink   m_Link;
        
        /**
         *  Default constructor.
         */
        IndirectLight()
        {
            radiant = Vec3( 1.0f, 1.0f, 1.0f );
            radius = 1.0f;
            
            updateInterval = -1;
            photonCount = 0;
            photonReflectionCount = 0;
        }
        
        /**
         *  Make a new indirect light.
         */
        IndirectLight( const Vec3& radiant, float radius, int32_t photonCount = 100,
                      int32_t photonReflectionCount = 4, int32_t updateInterval = -1 )
        {
            this->radiant = radiant;
            this->radius = radius;
            this->updateInterval = updateInterval;
            this->photonCount = photonCount;
            this->photonReflectionCount = photonReflectionCount;
        }
        
        ~IndirectLight()
        {
            
        }
    };
    
    /// Light illumination emitter.
    class IndirectEmitter
    {
    public:
        
        /**
         *  List of emitters.
         */
        SList   emiterList;
        
        /**
         *  Constructor.
         */
        IndirectEmitter()
        {
            
        }
        
        ~IndirectEmitter()
        {
            
        }
        
        /**
         *  Add a new emitter.
         */
        void addEmiter( IndirectLight * emiter )
        {
            removeEmiter(emiter);
            
            SList::AddTail( &emiterList, &emiter->m_Link, emiter );
        }
        
        /**
         *  Remove emitter.
         */
        void removeEmiter( IndirectLight* emiter )
        {
            SList::RemoveAt( &emiterList, &emiter->m_Link );
        }
        
        /**
         *  Trace a ray and find closest geometry at intersection.
         *
         *  @param orig          Source origin.
         *  @param dir           Light direction.
         *  @param radius        Radius.
         */
        void traceRadiant( const Vec3& orig, const Vec3& dir,
                          float radius, Vec3& radiant, int32_t reflectionIdx )
        {
            SRaycastInfo hit;
            
//            // Если луч не персёкся ни с чем, то выходим из функции
//            if( !world->traceRay(hit,dir,radius) ) return;
//            
//            // Домножаем текущий цвет на цвет материала
//            radiant = hit.material.color * radiant;
//            
//            // Отправляем текущий цвет в нашу текстуру модели
//            hit.model->uploadRadiant(hit.u,hit.v,radiant);
//            
//            
//            if( reflectionIdx > 0 )
//                traceRadiant(dir.reflect(hit.normal), radius, radiant, reflectionIdx - 1 );
        }
        
        /**
         *  Update source.
         */
        void updateEmiter( IndirectLight* emiter )
        {
            for( int32_t i = 0; i < emiter->photonCount; ++i )
            {
                // Генерируем случайный луч.
                Vec3 dir = Vec3(1.0);//generateRandomDirection();
                
                // Трассируем "фотон"
                traceRadiant( emiter->position, dir, emiter->radius, 
                             emiter->radiant, emiter->photonReflectionCount);
            }
        }
        
        /**
         *  Update light sources.
         */
        void processFrame( int32_t time )
        {
            SLink   * head;
            SLink   * cur;
            
            head = &emiterList.m_sList;
            
            for( cur = head->m_pNext; cur != head; cur = cur->m_pNext )
            {
                IndirectLight* emiter = (IndirectLight *) cur->m_ptrData;
                
                // Проверяем, если апдейт интервал равен -1 или 0,
                // то значит обнавлять такой источник придётя каждый кадр
                // что не очень хорошо, но всё же...
                if( emiter->updateInterval == -1 || emiter->updateInterval == 0 )
                {
                    updateEmiter(emiter);
                    continue;
                }
                
                // Если текущее время источника равно -1,
                // то значит источник ещё ни разу не обновлялся
                if( emiter->time == -1 )
                {
                    updateEmiter(emiter);
                    emiter->time = time;
                    continue;
                }
                
                // Прибавляем к прошлому времени источника апдейт интервал
                // и если то, что у нас получилось равно текущему времени:
                if( emiter->time + emiter->updateInterval == time )
                {
                    updateEmiter(emiter);
                    emiter->time = time;
                }
            }
        }
    };
    
//    void main( in Fragment fragment , out Output target , uniform sampler2D photonMap ){
//        vec2 texcoord2 = fragment.photonTexcoord;
//        
//        float3 indirect = float3(0,0,0);
//        const float multipler = 1.0f / 9.0f;
//        for( int x = -1; x < 1; x++ ){
//            indirect += tex2D(photonMap,texcoord2,int2(x,0)).rgb;
//        }
//        for( int y = -1; y < 1; y++ ){
//            indirect += tex2D(photonMap,texcoord2,int2(0,y)).rgb;
//        }
//        indirect = clamp(indirect * multipler,0.0f,1.0f);
//        
//        // Далее идёт направленое освещение и в итоге
//        target.color = direct + indirect;
//    }
}