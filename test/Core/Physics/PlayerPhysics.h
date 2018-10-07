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
//  PlayerPhysics.hpp
//  Neko engine
//
//  Created by Kawaii Neko on 10/13/15.
//  Copyright © 2015 Neko Vision. All rights reserved.
//

#ifndef PlayerPhysics_hpp
#define PlayerPhysics_hpp

#include "../../Math/BoundingBox.h"
#include "../../Math/BoundingSphere.h"
#include "../../World/Quadtree/Quadtree.h"  // World ground nodes.
#include "../../Core/Core.h"

namespace Neko {
    
    /**
     *  Object owner type.
     */
    enum class EObjectWorldType : int32_t {
        ServerWorld    = 0,
        ClientWorld
    };
    
    class CQuadtreeNode;
    
    ///   Physical object.
    class CPhysicsObject
    {
    public:
        
        // Object flags.
        const static int32_t    PHYS_CHARACTER      = 0x1;
        const static int32_t    PHYS_NONMOVABLE     = 0x2;  // A static world object.
        const static int32_t    PHYS_MOVABLE        = 0x4;  // A movable object but not a character!
        
        // Physics object flags.
        const static int32_t    PHYS_APPLYGRAVITY     = 0x1;  // Do do we need to apply the gravity force on the object?
        
        
        CPhysicsObject();
        virtual ~CPhysicsObject();
        
        /**
         *  Set object position.
         *
         *  @param newPos A new position to set.
         */
        inline void                 SetObjectPos( const Vec3 & newPos )   {    m_vPosition = newPos;  }
        
        /**
         *  Get object position.
         */
        inline Vec3                GetObjectPos()   {   return m_vPosition; }
        
        /**
         *  Set object velocity.
         *
         *  @param newVel A new position to set.
         */
        inline void                 SetVelocity( const Vec3 & newVel )   {    m_vVelocity = newVel;  }
        
        /**
         *  Get object velocity.
         */
        inline Vec3 &              GetVelocity()   {   return m_vVelocity; }
        
        /**
         *  Set object acceleration vector.
         *
         *  @param newAccel A new acceleration to set.
         */
        inline void                 SetAcceleration( const Vec3 & newAccel )   {    m_vAcceleration = newAccel;  }
        
        /**
         *  Get object acceleration.
         */
        inline Vec3 &              GetAcceleration()   {   return m_vAcceleration; }
        
        /**
         *  Set object default acceleration vector.
         *
         *  @param newAccel A new default acceleration to set.
         */
        inline void                 SetDefaultAcceleration( const Vec3 & newAccel )   {    m_vDefaultAcceleration = newAccel;  }
        
        /**
         *  Get object default acceleration.
         */
        inline void                 ResetAcceleration()   {   m_vAcceleration = m_vDefaultAcceleration; }
        
        /**
         *  Set object radius.
         *
         *  @param newRadius A new radius value.
         */
        inline void                 SetRadius( const float & newRadius )    {    m_fRadius = newRadius;  }
        
        /**
         *  Get object radius.
         */
        inline const float &                GetRadius() const  {   return m_fRadius;   }
        
        /**
         *  Set object mass.
         *
         *  @param newMass A new mass value.
         */
        inline void                 SetMass( const float & newMass )    {    m_fMass = newMass;  }
        
        /**
         *  Get object mass.
         */
        inline const float &                GetMass() const  {   return m_fMass;   }
        
        /**
         *  Is object on the ground?
         */
        inline bool                 IsOnGround()    {   return m_bCollided; }
        
        /**
         *  Set a corresponding value.
         */
        inline void             SetOnGround( bool value )   {   m_bCollided = value;    }
        
        /**
         *  Get object flag property.
         */
        inline int32_t              GetObjectFlags() const   {   return objectFlags;   }
        
        /**
         *  Set object flag property.
         *
         *  @param flag An option to set.
         */
        inline void                 SetObjectFlag( int32_t flag )   {   objectFlags |= flag;  }
        
        /**
         *  Get object's physics flag property.
         */
        inline int32_t              GetObjectPhysFlags() const   {   return physicsFlags;   }
        
        /**
         *  Set object's physics flag property.
         *
         *  @param flag An option to set.
         */
        inline void                 SetObjectPhysFlag( int32_t flag )   {   physicsFlags |= flag;  }
        
        /**
         *  Set object state to be standing on the node.
         *
         *  @param node A quadtree world node.
         */
        inline void                 StandingOn( const CQuadtreeNode * node )   {   m_pNode = node; }
        
        /**
         *  Get where object is currently standing.
         */
        inline const                CQuadtreeNode * GetStandingOn() const {   return m_pNode; }
        
        /**
         *  Set bounding box dimension.
         *
         *  @param min Minimum point.
         *  @param max Maximum point.
         */
        inline void     SetDimensions( const Vec3 & min, const Vec3 & max ) {
            m_bBox = SBoundingBox( min, max );
        }
        
        /**
         *  Get object's bounding box.
         */
        inline SBoundingBox *               GetBoundingBox()  {       return &m_bBox; }
        
        /**
         *  Set bounding box position.
         *
         *  @param newPos A corresponding value.
         */
        inline void     SetBBoxOrigin( const Vec3 & newPos ) {
            m_bBox.Translate( newPos );
        }
        
        /**
         *  Set object scale.
         *
         *  @param newScale A new scale value to set.
         */
        inline void                 SetScale( const Vec3 & newScale )  { m_vScale = newScale; }
        
        /**
         *  Get object's scale.
         */
        inline const Vec3 &                GetScale() const   {   return m_vScale;    }
        
        /**
         *  Set object rotation.
         *
         *  @param newRotation A new rotation value to set.
         */
        inline void                 SetRotation( const Vec3 & newRotation )  { m_vRotation = newRotation; }
        
        /**
         *  Get object's rotation.
         */
        inline const Vec3 &                GetRotation() const   {   return m_vRotation;    }
        
    public:
//    private:
        
        //!  Bounding box.
        SBoundingBox   m_bBox;
        
        BoundingSphere  m_pbSphere;
        
        //!  Object position.
        Vec3   m_vPosition;
        
        //!  Velocity.
        Vec3   m_vVelocity;
        Vec3   m_vConstVelocity;
        
        //!  Acceleration vector.
        Vec3   m_vAcceleration;
        Vec3   m_vDefaultAcceleration;
        
        //!  Object scale.
        Vec3   m_vScale;
        
        //!  Object rotation.
        Vec3   m_vRotation;
        
        //!  Object friction.
        float   m_fFriction;
        
        //!  Object radius.
        float   m_fRadius;
        
        //!  Object mass.
        float   m_fMass;
        
        //!  Has collided?
        bool    m_bCollided;
        
        //!  Object flag options.
        int32_t     objectFlags;
        
        //!  Object's physics flags.
        int32_t     physicsFlags;
        
        //!  Object world type.
        EObjectWorldType     m_worldType;
        
        //!  World node.
        const CQuadtreeNode      *m_pNode;
        
        //!  Object standing on.
        CPhysicsObject      *m_pStandingOn;
    };

    ///  Motion.
    struct SMotionInfo
    {
        Vec3   m_vForce;
        Vec3   m_vUnitForce;
        
        float   m_fSlideRatio;
        float   m_fForceMag;
        
        /**
         *  Constructor.
         */
        SMotionInfo() {
            Vec3 force( 0.0f, -2.0f, 0.0f );
            
            // Set the gravity value.
            SetForce( &force );
            
            m_fSlideRatio = -0.7071f;
        }
        
        /**
         *  Set motion force.
         *
         *  @param pForce Force value to set.
         */
        inline void SetForce( const Vec3 * pForce )
        {
            m_vForce = *pForce;
            m_fForceMag = m_vForce.Length();
            
            // Check if we can set the force.
            if( m_fForceMag > 0.00001f ) {
                m_vUnitForce = m_vForce;
                m_vUnitForce = m_vUnitForce / m_fForceMag;
            } else {
                m_vUnitForce.MakeNull();
            }
        }
    };
    
    /**
     *  Player movement information.
     */
    struct SMoveInfo
    {
        CPhysicsObject  * m_pObject;
        float       m_fDeltaTime;
        Vec3   m_vDisplacement;
    };
    
    /**
     *  A small struct to store object physics values.
     */
    struct SContainerPhysics
    {
        Vec3   m_vVelocity;
        Vec3   m_vAcceleration;
        
        CPhysicsObject  * m_pHandle;
    };
    
    /**
     *  Ray cast hit test information.
     */
    struct SRaycastInfo
    {
        Vec3   m_vDirection;
        Vec3   m_vOrigin;
        Vec3   m_vHitOrigin;
        
        int32_t m_bCollided;
        
        float   m_fDistance;
        float   m_fMaxDistance;
        
        CPhysicsObject * m_pObject; //! Can be NEKO_NULL.
        
        CQuadtreeNode   * m_pGroundNode;    //! World ground node. Can be NEKO_NULL.
        
        SRaycastInfo() {
            m_pGroundNode = NEKO_NULL;
            m_pObject = NEKO_NULL;
        }
    };
    
    /// World physics.
    class CWorldPhysics
    {
        NEKO_NONCOPYABLE( CWorldPhysics );
        
    public:
        
        /**
         *  Constructor.
         */
        CWorldPhysics();
        
        /**
         *  Destructor.
         */
        ~CWorldPhysics();
        
        /**
         *  Update physics.
         */
        void                Update();
        
        /**
         *  Update ground nodes with its objects.
         */
        void                UpdateWorldNode( CQuadtreeNode * node, CPhysicsObject * hObject );
        
        /**
         *  Update movement.
         */
        void                UpdateMovement( CPhysicsObject * pObject, const float time );
        
        /**
         *  Check world object collision.
         *
         *  @param node    Quadtree node.
         *  @param hObject An object.
         */
        int32_t                 CheckWorldCollision( CQuadtreeNode * node, CPhysicsObject * hObject );
        
        /**
         *  Check ray hit intersection.
         */
        int32_t                 RayHitIntersection( SRaycastInfo * raycastInfo, bool touchObjects = false );
        
        /**
         *  Check world ground ray hit intersection.
         */
        int32_t                 RayHitGroundIntersection( SRaycastInfo * raycastInfo );
        
        /**
         *  Check object collision.
         *
         *  @param node    A quadtree node.
         *  @param hObject A quadtree chunk objects.
         */
        int32_t                 CheckObjectCollision( CQuadtreeNode * node, CPhysicsObject * hObject );
    
        /**
         *  Calculate collision information.
         */
        bool                CalculateCollisionInfo( CPhysicsObject * pObject, Vec3 * pObjectVel, Vec3 * pStopPlane, Vec3 * pVelocityAdd, Vec3 * pForce );
    
        /**
         *  Retrieve collision offset.
         *
         *  @param vNormal  Polygon normal direction.
         */
        static Vec3                GetCollisionOffset( Vec3 &vNormal, float radius, float distance );
    };
    
    extern CWorldPhysics * g_pWorldPhysics;
    
    /// Player ( character ) physics.
    class CPlayerPhysics
    {
        NEKO_NONCOPYABLE( CPlayerPhysics );
        
    public:
        
        /**
         *  Constructor.
         */
        CPlayerPhysics();
        
        /**
         *  Destructor.
         */
        ~CPlayerPhysics();
        
    };
}

#endif /* PlayerPhysics_hpp */
