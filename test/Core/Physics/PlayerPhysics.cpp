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
//  PlayerPhysics.cpp
//  Neko engine
//
//  Created by Kawaii Neko on 10/13/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#include "PlayerPhysics.h"
#include "../../World/Quadtree/Quadtree.h"  // World ground nodes.
#include "../../AssetCommon/Sound/SoundManager.h"   // Footsteps, landing, etc...
#include "../../Core/Player/Camera/Camera.h"    // Camera instance.
#include "../../Core/Network/MultiplayerServer.h"   // Server world.

#include "../../Math/GameMath.h"
#include "../../Math/Plane.h"
#include "../../Math/Vec2.h"
#include "../../Math/Vec3.h"
#include "../../Math/Vec4.h"
#include "../../Math/PolygonMath.h"

namespace Neko {
    
    /**
     *  Constructor.
     */
    CWorldPhysics::CWorldPhysics()
    {
    
    }
    
    /**
     *  Destructor.
     */
    CWorldPhysics::~CWorldPhysics()
    {
        
    }
    
    /**
     *  Get collision offset from the normal.
     */
    Vec3 CWorldPhysics::GetCollisionOffset( Vec3 &vNormal, float radius, float distance )
    {
        Vec3 vOffset;
        float distanceOver;
        
        vOffset = Vec3( 0.0f );
        
        // If distance is greater than zero, it's a polygon.
        if( distance > 0.0f ) {
            // Find the direction vector and distance.
            distanceOver = radius - distance;
            vOffset = vNormal * distanceOver;
        }
        
        // Colliding from behind.
        else {
            // Find the direction vector and distance.
            distanceOver = radius + distance;
            vOffset = vNormal * -distanceOver;
        }
        
        // Return the offset to not be intersecting.
        return vOffset;
    }

    /**
     *  Update physics.
     */
    void CWorldPhysics::Update()
    {

    }
    
    /**
     *  Update object movement.
     *
     *  @param pObject A physics object.
     *  @param time    Delta time.
     */
    void CWorldPhysics::UpdateMovement( CPhysicsObject *pObject, const float time )
    {
        return;
        SMoveInfo       moveInfo;
        SMotionInfo     motionInfo; // Gravity force is also set in the constructor.
        
        Vec3   * movementNormal;
        Vec3   objectNormal;
            
        bool        friction;
        
        // Magnitude.
        float       velocityLength;
        float       accelerationLength;
        
        float       timeDelta;
        
        // Movement info.
        moveInfo.m_fDeltaTime = time;
        moveInfo.m_pObject = pObject;
        
//        motionInfo.m_fSlideRatio = 1.0;

        // Get vectors' magnitudes.
        velocityLength = pObject->GetVelocity().Length();
        accelerationLength = pObject->GetAcceleration().Length();
        
        // Time integral.
        timeDelta = 0.5f * time * time;
        
        friction = false;
        
        // Reset the movement if needed.
        if( accelerationLength < 0.1f ) {
            pObject->m_vAcceleration.MakeNull();
            accelerationLength = 0;
        }
        
        if( velocityLength < 0.1f ) {
            pObject->m_vVelocity.MakeNull();
            velocityLength = 0;
        }
        
        Vec3 accelForce;
        Vec3 forceDir;
        
        // Check if we're standing on the ground.
        if( pObject->IsOnGround() ) {
            // Check again..
            if( velocityLength < 0.1f && accelerationLength < 0.1f ) {
                pObject->m_vVelocity.MakeNull();
                pObject->ResetAcceleration();
                return;
            }
            
            // Normal offset.
            movementNormal = (Vec3*)&pObject->m_pNode->m_planeNormal;
            
            accelForce = pObject->GetAcceleration() + motionInfo.m_vForce;
            forceDir = motionInfo.m_vForce;
            
            forceDir.Normalize();
            
            // Initialy hit the ground surface.
            float fDotN = Vec3::Dot( forceDir, *movementNormal );
            float fDotA;
            
            /**  Check if we're falling from the world node ( mountains, etc ).  **/
            if( fDotN > motionInfo.m_fSlideRatio ) {
                float fAccelMag = accelForce.Length();
                fDotA = Vec3::Dot( *movementNormal, accelForce );
                pObject->m_vAcceleration = accelForce - *movementNormal * -fDotA;
                
                // Don't allow it to accelerate up the slope..
                float fAccelDotForce = Vec3::Dot(pObject->m_vAcceleration, forceDir);
                
                if (fAccelDotForce < 0.0f) {
                    pObject->m_vAcceleration = pObject->m_vAcceleration - forceDir * fAccelDotForce;
                }
                
                pObject->m_vAcceleration.Normalize(fAccelMag);
                pObject->m_vAcceleration = pObject->m_vAcceleration / 15.0f;
                
                printf( "break\n ");
            }
            
            /**     Check for a physical actions.   **/
            else
            {
                // Figure out what our new velocity would be if only the force was used (i.e. are they jumping?)
                Vec3 vNewVel = pObject->m_vVelocity + motionInfo.m_vForce * time;
                // If we're going to be moving away from the plane, use the full force
                if( Vec3::Dot( vNewVel, *movementNormal ) > 0.01f )
                {
                    pObject->m_vAcceleration = accelForce;
                    
                    g_Core->p_Console->Print( LOG_INFO, "Jump" );
                }
                // If the acceleration without the force isn't moving into the surface, project it there
                else if( Vec3::Dot( pObject->m_vAcceleration, *movementNormal ) > 0.01f ) {
//                        float fAccelMag = pObject->m_vAcceleration.Length();
//                        pObject->m_vAcceleration = pObject->m_vAcceleration + *movementNormal / (Vec3::Dot(*movementNormal,pObject->m_vAcceleration) + 1.0f);
//                        pObject->m_vAcceleration.Normalize(fAccelMag);
//                        pObject->m_vAcceleration = pObject->m_vAcceleration / 15.0f;
                    
                    friction = true;
                    
                    g_Core->p_Console->Print( LOG_INFO, "Downhill" );
                } else {
                    friction = true;
                    
                    g_Core->p_Console->Print( LOG_INFO, "Walk" );
                }
            }
        }
        
        /**     Apply the fall gravity.     **/
        else
        {
            pObject->m_vAcceleration = pObject->m_vAcceleration + motionInfo.m_vForce;
        }
    }
    
    /**
     *  Triangle intersection.
     *
     *  @param orig Ray origin.
     *  @param dir  Ray direction.
     *  @param v0   Point 1.
     *  @param v1   Point 2.
     *  @param v2   Point 3.
     *
     *  @return Had any intersection?
     */
    static bool IntersectTriangle( const Vec3& orig, const Vec3& dir, Vec3& v0, Vec3& v1, Vec3& v2, float* t, float* u, float* v )
    {
        // Find vectors for two edges sharing vert0
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        
        // Begin calculating determinant - also used to calculate U parameter
        Vec3 pvec;
        pvec = Vec3::CrossTwoVectors( dir, edge2 );
        
        // If determinant is near zero, ray lies in plane of triangle
        float det = Vec3::Dot( edge1, pvec );
        
        Vec3 tvec;
        if( det > 0 ) {
            tvec = orig - v0;
        } else {
            tvec = v0 - orig;
            det = -det;
        }
        
        if( det < 0.0001f ) {
            return false;
        }
        
        // Calculate U parameter and test bounds
        *u = Vec3::Dot( tvec, pvec );
        if( *u < 0.0f || *u > det ) {
            return false;
        }
        
        // Prepare to test V parameter
        Vec3 qvec;
        qvec = Vec3::CrossTwoVectors( tvec, edge1 );
        
        // Calculate V parameter and test bounds
        *v = Vec3::Dot( dir, qvec );
        
        if( *v < 0.0f || *u + *v > det ) {
            return false;
        }
        
        // Calculate t, scale parameters, ray intersects triangle
        *t = Vec3::Dot( edge2, qvec );
        
        float fInvDet = 1.0f / det;
        *t *= fInvDet;
        *u *= fInvDet;
        *v *= fInvDet;
        
        // Had intersection.
        return true;
    }
    
    /**
     *  Ray hit intersection test on the world ground.
     */
    int32_t CWorldPhysics::RayHitGroundIntersection( SRaycastInfo *raycastInfo )
    {
        uint32_t    i;
        Vec3   * collisionData;
        
        collisionData = raycastInfo->m_pGroundNode->m_pCollisionChunks;
        
        // Point and texture coordinates.
        float t, u, v;
        
        // Go through all the triangles on the current world node.
        for( i = 0; i < raycastInfo->m_pGroundNode->m_iColNum; i += 3 /* Triangle sides */ ) {
            // NOTE: 'STriangle' property use here is soo slow.
            
            // Invert direction if we're below the triangle.
            if( collisionData[i].y > (raycastInfo->m_vOrigin.y - 4.0f) ) {
                raycastInfo->m_vDirection.y *= -1.0f;
            }
            
            // Triangle intersection.
            if( IntersectTriangle( raycastInfo->m_vOrigin, raycastInfo->m_vDirection,
                                  collisionData[i],
                                  collisionData[i + 1],
                                  collisionData[i + 2],
                                  &t, &u, &v ))
            {
                // Intersection hit.
                raycastInfo->m_vHitOrigin = raycastInfo->m_vOrigin + t * raycastInfo->m_vDirection;
                
                return 1;
            }
        }
        
        return 0;
    }
    
    /**
     *  Raycast hit world collision test.
     *
     *  @param node         A world node.
     *  @param raycastInfo  Ray cast information.
     *  @param touchObjects Should ray check any object standing before the ground?
     *
     *  @return Intersection value.
     */
    int32_t CWorldPhysics::RayHitIntersection( SRaycastInfo * raycastInfo, bool touchObjects )
    {
        // Check if node exists.
        if( raycastInfo->m_pGroundNode != NEKO_NULL ) {
            RayHitGroundIntersection( raycastInfo );
        }

        return 0;
    }
    
    /**
     *  Update ground nodes with its objects.
     *
     *  @param node     A ground node.
     *  @param hObject  A physics object.
     *
     *  @note           Node's type can be 'water'!
     */
    void CWorldPhysics::UpdateWorldNode( CQuadtreeNode * node, CPhysicsObject * hObject )
    {
        return;
        
        if( hObject == NEKO_NULL ) {
            return;
        }
        
        // Terrain ground node.
        if( node->m_qType == TERRAIN ) {
            CheckWorldCollision( node, hObject );
        }
        
        // "Liquid" node.
        else if( node->m_qType == OCEAN ) {
            // TODO
        } else {
            assert( !"UpdateWorldNode(): Unknown node type." );
        }
    }

    /**
     *  Check for objects collision.
     *
     *  @param node    A world node.
     *  @param hObject An object.
     */
    int32_t CWorldPhysics::CheckObjectCollision( CQuadtreeNode *node,  CPhysicsObject *hObject )
    {
        uint32_t    objectCount;
        
        SBoundingBox    * meshBoundingBox;
        CPhysicsObject  * meshHandle;
        
        objectCount = node->m_terrainChunk->m_TerrainObjectCount;
        
        // Don't check object collision unless there are none.
        if( objectCount < 1 ) {
            return 0;
        }
        
        // Loop through node's chunk objects.
        SLink   * head;
        SLink   * cur;
        
        head = &node->m_terrainChunk->m_terrainObjects.m_sList;
        
        for( cur = head->m_pNext; cur != head; cur = cur->m_pNext )
        {
            // Get a mesh.
            meshHandle = (CPhysicsObject *) cur->m_ptrData;
            
            // Get mesh bounding box.
            meshBoundingBox = meshHandle->GetBoundingBox();
            
            // Check if player's object bounding box is touching another object's bounding box.
            if( SBoundingBox::ContainsBoxes( meshBoundingBox->min, meshBoundingBox->max, hObject->GetBoundingBox()->min + hObject->GetObjectPos(), hObject->GetBoundingBox()->max + hObject->GetObjectPos() ) ) {
                // Close to the object.
//                printf( "close to a bbox\n");
            }
        }
        
        return 0;
    }
    
    const static float NORMAL_SLOPE = 0.707f;
    
    /**
     *  Check object collision.
     *  Checks for a ground nodes.
     */
    int32_t CWorldPhysics::CheckWorldCollision( CQuadtreeNode * node, CPhysicsObject * hObject )
    {
        SMotionInfo     motionInfo;
        
        Vec3   vNormal;
        Vec3   vOffset;
        Vec3   vIntersection;
        
        Vec3 * collisionData;
        
        Vec3   vEnd;
        
        float   fTimeCollided;
        float   fDistance;
        
        // Camera sphere body radius.
        float   fRadius;
        
        uint32_t    i;
        
        // Camera bounds ( sphere ) classification.
        uint32_t classification;
        
        // Collision data.
        collisionData = node->m_pCollisionChunks;
        
        // Object radius.
        fRadius = hObject->GetRadius();
        fTimeCollided = 0.0f;
        
        vEnd = hObject->m_vPosition;
        
        hObject->SetOnGround( false );
     
        /**     Check the ground collision with the player.     **/
        
        // Go through all the triangles
        for( i = 0; i < node->m_iColNum; i += 3 /* Triangle sides */ ) {
            // Make a triangle.
            Vec3 vTriangle[3] =
            {
                collisionData[i],
                collisionData[i + 1],
                collisionData[i + 2]
            };
            
            // Make the normal.
            vNormal = Vec3::MakeNormal( vTriangle );
            
            // Distance from the plane
            fDistance = 0.0f;
            
            // FRONT, BEHIND, or INTERSECTS the plane.
            classification = ncPlane::ClassifySphere( hObject->m_vPosition, vNormal, vTriangle[0], fRadius, fDistance );
            
            // If we intersect the plane, check further.
            if( classification == nkMath::POINT_ON_PLANE /* Intersects */ ) {
                // Pseudo intersection point on the plane.
                
                // Now we want to project onto the triangle's plane.
                vOffset = vNormal * fDistance;
                
                // Subtract it from the center.
                vIntersection = hObject->m_vPosition - vOffset;

                // We first check if our intersection point is inside the triangle.
                if( ncPolygonMath::InsidePolygon( vIntersection, vTriangle, 3 ) || ncPolygonMath::EdgeSphereCollision( hObject->m_vPosition, vTriangle, 3, fRadius / 2.0f ) )
                {
                    // If we get here, we have collided!
                    vOffset = GetCollisionOffset( vNormal, fRadius, fDistance );
                
                    // Object is on the ground.
                    hObject->SetOnGround( true );
                    
                    // Time when the object got collided.
                    fTimeCollided = g_Core->GetTime();
                    
                    // Current node normal.
                    node->m_planeNormal = vOffset;
                }
                
//                vEnd = objectPos + vOffset;
            }
        }

        // Object collision.
//        CheckObjectCollision( node, hObject );
 
        // Check if we are falling down..
        float fDistFell;
        
        // Object is on the ground.
        if( hObject->IsOnGround() ) {
            // Currently standing on quadtree world node..
            hObject->StandingOn( node );

            // Calculate the fall distance.
            fDistFell = g_Core->p_Camera->m_fLastPositionY - hObject->GetObjectPos().y;
            g_Core->p_Camera->m_fLastPositionY = hObject->GetObjectPos().y;
      
            // Check if we have fell down.. ( using last known ground position ).
            if( fDistFell > 1.0f ) {
                // Fell down.. ( landed )
                
                // TODO: Check if we can play a sound on the current surface.
//                g_Core->p_SoundSystem->PlaySoundAt( "scissors", hObject->m_vPosition, 1.0f, false, false );
                
                // TODO: much more
                g_Core->p_Console->Print( LOG_INFO, "Fall distance: %4.1f m\n", fDistFell / 100.0f );
            }
            
            // Reset the acceleration back to the still state ( we won't fall through ).
            hObject->ResetAcceleration();
            
            // Fall through.
            if ( node->m_planeNormal.y < NORMAL_SLOPE ) {
                hObject->SetOnGround( false );
            }
        } else {
            // Not on the ground..
            hObject->StandingOn( NEKO_NULL );    // TODO: Replace a null by somewhat else.
        }
        
        // Set the last ground position to be the current object y position.
        if( hObject->IsOnGround() == false ) {
            // Set the last object height value on the ground.
            if( hObject->GetObjectPos().y > g_Core->p_Camera->m_fLastPositionY ) {
                g_Core->p_Camera->m_fLastPositionY = hObject->GetObjectPos().y;
            }
        }
        
//        UpdateMovement( hObject, ((float)g_Core->p_Server->frameMsec / 1000.0f) );
        
//        printf( "Collision points: %i\n", i );
//        printf( "Collision check: %f\n", t2 - t1 );
        
        return -1;
    }
    
    /**
     *  Calculate collision information.
     *
     *  @param pObject      Current object.
     *  @param pObjectVel   Object velocity.
     *
     *  @return Could calculate info?
     */
    bool CWorldPhysics::CalculateCollisionInfo( CPhysicsObject *pObject, Vec3 *pObjectVel, Vec3 *pStopPlane,
                                               Vec3 *pVelocityAdd, Vec3 *pForce )
    {
        
        return true;
    }
    
    CWorldPhysics   * g_pWorldPhysics;

    /**
     *  Constructor.
     */
    CPlayerPhysics::CPlayerPhysics()
    {
        
    }
    
    /**
     *  Destructor.
     */
    CPlayerPhysics::~CPlayerPhysics()
    {
        
    }

    /**     Physics object      **/
    CPhysicsObject::CPhysicsObject() : m_pStandingOn(NEKO_NULL), m_pNode(NEKO_NULL), m_fFriction(0.0f), m_fRadius(0.0f), m_fMass(0.0f)
    {
        m_worldType = EObjectWorldType::ServerWorld;
        
        m_vScale = Vec3( 1.0f );
        m_vPosition = Vec3( 0.0f );
        
        objectFlags = 0;
        physicsFlags = 0;
        
        m_vAcceleration.MakeNull();
        m_vVelocity.MakeNull();
        m_vConstVelocity.MakeNull();
    }
    
    CPhysicsObject::~CPhysicsObject()
    {
        
    }
    
}