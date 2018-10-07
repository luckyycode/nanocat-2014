//
//  ServerWorld.cpp
//  Nanocat
//
//  Created by Kawaii Neko on 9/27/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "ServerWorld.h"
#include "../../World/BeautifulEnvironment.h"   // World generation.

namespace Neko {
    
    /**
     *  Initialize server world.
     *
     *  @note - This initializes all things we need to generate client world.
     */
    void CServerWorld::Initialize()
    {
        g_Core->p_Console->Print( LOG_INFO, "Initializing server world..\n" );
        
        PrepareWorld();
    }
    
    /**
     *  Prepare server world.
     *
     *  @note - Generates random seed if required.
     */
    void CServerWorld::PrepareWorld()
    {
        m_worldSeed[0] = (int32_t)rand();
        m_worldSeed[1] = m_worldSeed[0];
        m_worldSeed[2] = m_worldSeed[0];
    }
    
    /**
     *  Create the server world.
     */
    void CServerWorld::CreateWorld()
    {
        g_Core->p_Console->Print( LOG_INFO, "Creating a server world\n " );

        // Server MUST be able to use methods of Beautiful Environment.
        // TODO: local worlds
        
    }
    
    /**
     *  Set random entity origin.
     *
     *  @param pEntity An entity to process.
     */
    void CServerWorld::SetRandomEntityPosition( CPhysicsObject *pEntity )
    {
        if( pEntity == NEKO_NULL ) {
            return;
        }
        
        
    }
    
    /**
     *  Decrease object velocity after it finished moving.
     *
     *  @param pEntity   A server object.
     *  @param msInSec   Millisecond frametime in seconds.
     *  @param direction Object direction.
     */
    void CServerWorld::EndObjectVelocity( CPhysicsObject * pEntity, float msInSec, const Vec3 & direction )
    {
        // Camera doesn't move along the X, Y or Z axis anymore..
        
        if( direction.LengthSq() == 0.0f )
        {
            if( pEntity->m_vVelocity.x > 0.0f ) {
                if( (pEntity->m_vVelocity.x -= pEntity->GetAcceleration().x * msInSec) < 0.0f ) {
                    pEntity->m_vVelocity.x = 0.0f;
                }
            } else {
                if( (pEntity->m_vVelocity.x += pEntity->GetAcceleration().x * msInSec) > 0.0f ) {
                    pEntity->m_vVelocity.x = 0.0f;
                }
            }
     
            if( pEntity->m_vVelocity.y > 0.0f ) {
                if( (pEntity->m_vVelocity.y -= pEntity->GetAcceleration().y * msInSec) < 0.0f ) {
                    pEntity->m_vVelocity.y = 0.0f;
                }
            } else {
                if( (pEntity->m_vVelocity.y += pEntity->GetAcceleration().y * msInSec) > 0.0f ) {
                    pEntity->m_vVelocity.y = 0.0f;
                }
            }
        
            if( pEntity->m_vVelocity.z > 0.0f ) {
                if( (pEntity->m_vVelocity.z -= pEntity->GetAcceleration().z * msInSec) < 0.0f ) {
                    pEntity->m_vVelocity.z = 0.0f;
                }
            } else {
                if( (pEntity->m_vVelocity.z += pEntity->GetAcceleration().z * msInSec) > 0.0f ) {
                    pEntity->m_vVelocity.z = 0.0f;
                }
            }
        }
    }
    
    /**
     *  Calculate object velocity based on the direction.
     */
    void CServerWorld::CalculateVelocity( CPhysicsObject * pEntity, const float msInSec, const Vec3 & direction )
    {
        Vec3   absDir;

        // Check for a very small changes.
        if( pEntity->m_vVelocity.LengthSq() <= 0.01f ) {
            pEntity->m_vVelocity.MakeNull();
        }
        
        
        absDir = direction;
        
        // If we're looking down the Y value is probably negative in most cases,
        // what will result a wrong multiplication with the acceleration value and
        // return a positive value, what makes the entity fly up.
//        if( absDir.y <= 0.0f )
//        {
//            // Pump it a bit. Doesn't change actual direction value.
////            absDir.y = 0.001f;
//            absDir.y = fabs(absDir.y);
//        }
        
        // Check if we're moving..
        if( pEntity->m_vVelocity.LengthSq() != 0.0f )
        {
            // Calculate object displacement.
            Vec3 displacement;
            // Use Newton's Second Law.
            displacement = (pEntity->m_vVelocity * msInSec) + (absDir * pEntity->m_vAcceleration * 0.5f * msInSec * msInSec );
            // Set object origin.
            pEntity->m_vPosition = pEntity->m_vPosition + displacement;
        }
        
        /**     Calculate current velocity from  (direction * acceleration).    **/
        
        // Update velocity when we have a direction set.
        if( direction.LengthSq() != 0.0f ) {
            // Object is moving..
            
            pEntity->m_vVelocity = pEntity->m_vVelocity + absDir * pEntity->m_vAcceleration * msInSec;
        } else {  // Else reset it to zero.
            // Object is in the still state..
            // NOTE: for some reason even if object is not moving, velocity pulls it up, so this line fixes it.
            pEntity->m_vVelocity.MakeNull();
        }
        
        // X axis
        if( pEntity->m_vVelocity.x > pEntity->m_vConstVelocity.x ) {
            pEntity->m_vVelocity.x = pEntity->m_vConstVelocity.x;
        } else if( pEntity->m_vVelocity.x < -pEntity->m_vConstVelocity.x ) {
            pEntity->m_vVelocity.x = -pEntity->m_vConstVelocity.x;
        }
        
        // Y axis
        if( pEntity->m_vVelocity.y > pEntity->m_vConstVelocity.y ) {
            pEntity->m_vVelocity.y = pEntity->m_vConstVelocity.y;
        } else if( pEntity->m_vVelocity.y < -pEntity->m_vConstVelocity.y ) {
            pEntity->m_vVelocity.y = -pEntity->m_vConstVelocity.y;
        }
        
        // Z axis
        if( pEntity->m_vVelocity.z > pEntity->m_vConstVelocity.z ) {
            pEntity->m_vVelocity.z = pEntity->m_vConstVelocity.z;
        } else if( pEntity->m_vVelocity.z < -pEntity->m_vConstVelocity.z ) {
            pEntity->m_vVelocity.z = -pEntity->m_vConstVelocity.z;
        }
    }
    
    /**
     *  Perform entity movement.
     *
     *  @param entity - An entity we use to apply user commands.
     *  @param command - A command base.
     *
     *  @note - Can be called as from the server side and client side.
     */
    void CServerWorld::PerformEntityMove( PlayerEntity *entity,  UserCommand *command )
    {
        // A time property when command should be finished.
        uint32_t        finalTime;
        PlayerEntity*   playerEntity;
        
        uint32_t      msec;
        
        finalTime = command->ServerTime;
        playerEntity = entity;

        // Previous command??
        if( finalTime < playerEntity->LastCommandTime ) {
            return;
        }
        
        if( finalTime > (playerEntity->LastCommandTime + 1000) )    /** One second. **/ {
            playerEntity->LastCommandTime = finalTime - 1000;
        }
        
        // Calculate entity direction/position.
        playerEntity->vRight.Cross( playerEntity->GetRotation(), playerEntity->vUp );
        playerEntity->vRight.Normalize();
        
        playerEntity->vUp.Cross( playerEntity->vRight, playerEntity->GetRotation() );
        playerEntity->vUp.Normalize();
        
        const Vec3 & tmpLook = playerEntity->GetRotation();
        const Vec3 & tmpRight = playerEntity->vRight;
        
        // Perform entity movement until the final time.
        while( playerEntity->LastCommandTime != finalTime )
        {
            msec = finalTime - playerEntity->LastCommandTime;
            
            // Prevent client move command time overflow.
            if( msec > ENT_MOVE_MSEC ) {
                msec = ENT_MOVE_MSEC;
            }
            
            // Command server time.
            command->ServerTime = playerEntity->LastCommandTime + msec;
            
            // Entity last command time.
            playerEntity->LastCommandTime = command->ServerTime;
            
            const Vec3 velocity = playerEntity->GetVelocity();

            // Reset the direction.
            playerEntity->vCurDirection.MakeNull();
            
            
            /**     Player movement command parsing.    **/
            
            
            /**     Forward & backward.    **/
            if( command->IsPressed( ClientButtons::Forward ) ) {
                playerEntity->vCurDirection = tmpLook;
                
                playerEntity->m_vVelocity = Vec3( velocity.x, velocity.y, 0.0f );
            } else if( command->IsPressed( ClientButtons::Backward ) ) {
                playerEntity->vCurDirection = -tmpLook;
                
                playerEntity->m_vVelocity = Vec3( velocity.x, velocity.y, 0.0f );
            }
            
            /**     Left & right.   **/
            else if( command->IsPressed( ClientButtons::Left ) ) {
                playerEntity->vCurDirection = -tmpRight;
                
                playerEntity->m_vVelocity = Vec3( 0.0f, velocity.y, velocity.z );
            } else if( command->IsPressed( ClientButtons::Right ) ) {
                playerEntity->vCurDirection = tmpRight;
                
                playerEntity->m_vVelocity = Vec3( 0.0f, velocity.y, velocity.z );
            }
            
        }
    }
    
    /**
     *  Write a world information.
     */
    void CServerWorld::SerializeWorld( SBitMessage *message )
    {
        // Write a world seed.
        message->WriteLong( m_worldSeed[0] );
    }
}