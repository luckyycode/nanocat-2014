//          **
//         *               *   *    _      *
//               *                 / )
//             *    /\__/\ *       ( (  *
//           ,-.,-.,)    (.,-.,-.,-.) ).,-.,-.
//          | @|  ={      }= | @|  / / | @|o |
//         _j__j__j_\     `-------/ /__j__j__j_
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
//  Camera.h
//  Game camera.. :v)
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include "../../Core.h" // non-copyable define
#include "../../../Math/GameMath.h"
#include "../../../Platform/Shared/SystemShared.h"
#include "../../Console/ConsoleVariable.h"
#include "../../../Math/Frustum.h"
#include "../../Physics/PlayerPhysics.h"

namespace Neko {

    //!  Max angle to turn camera.
    static const float MaxVerticalAngle = 95.0f;

    //! Reference to an object class.
    struct CPhysicsObject;
    
    // Distributions
    static float points_Still[2] = {
        0.5f, 0.5f,
    };
    
    static float points_Uniform2[4] =  {
        -0.25f, -0.25f, //ll
        0.25f,  0.25f,  //ur
    };
    
    static float points_Uniform4[8] = {
        -0.25f, -0.25f, //ll
        0.25f, -0.25f,  //lr
        0.25f,  0.25f,  //ur
        -0.25f,  0.25f, //ul
    };
    
    static float points_Uniform4_Helix[8] = {
        -0.25f, -0.25f, //ll  3  1
        0.25f,  0.25f,  //ur   \/|
        0.25f, -0.25f,  //lr   /\|
        -0.25f,  0.25f, //ul  0  2
    };
    
    static float points_Uniform4_DoubleHelix[16] = {
        -0.25f, -0.25f, //ll  3  1
        0.25f,  0.25f,  //ur   \/|
        0.25f, -0.25f,  //lr   /\|
        -0.25f,  0.25f, //ul  0  2
        -0.25f, -0.25f, //ll  6--7
        0.25f, -0.25f,  //lr   \'
        -0.25f,  0.25f, //ul    \'
        0.25f,  0.25f,  //ur  4--5
    };
    
    static float points_SkewButterfly[8] =  {
        -0.250f, -0.250f,
        0.250f,  0.250f,
        0.125f, -0.125f,
        -0.125f,  0.125f,
    };
    
    static float points_Rotated4[8] = {
        -0.125f, -0.375f,   //ll
        0.375f, -0.125f,    //lr
        0.125f,  0.375f,    //ur
        -0.375f,  0.125f,   //ul
    };
    
    static float points_Rotated4_Helix[8] = {
        -0.125f, -0.375f,   //ll  3  1
        0.125f,  0.375f,    //ur   \/|
        0.375f, -0.125f,    //lr   /\|
        -0.375f,  0.125f,   //ul  0  2
    };
    
    static float points_Rotated4_Helix2[8] = {
        -0.125f, -0.375f,   //ll  2--1
        0.125f,  0.375f,    //ur   \/
        -0.375f,  0.125f,   //ul   /\'
        0.375f, -0.125f,    //lr  0  3
    };
    
    static float points_Poisson10[20] = {
        -0.16795960f    * 0.25f,  0.65544910f * 0.25f,
        -0.69096030f    * 0.25f,  0.59015970f * 0.25f,
        0.49843820f     * 0.25f,  0.83099720f * 0.25f,
        0.17230150f     * 0.25f, -0.03882703f * 0.25f,
        -0.60772670f    * 0.25f, -0.06013587f * 0.25f,
        0.65606390f     * 0.25f,  0.24007600f * 0.25f,
        0.80348370f     * 0.25f, -0.48096900f * 0.25f,
        0.33436540f     * 0.25f, -0.73007030f * 0.25f,
        -0.47839520f    * 0.25f, -0.56005300f * 0.25f,
        -0.12388120f    * 0.25f, -0.96633990f * 0.25f,
    };
    
    static float points_Pentagram[10] = {
        0.000000f   * 0.5f,  0.525731f * 0.5f,  // head
        -0.309017f  * 0.5f, -0.425325f * 0.5f,  // lleg
        0.500000f   * 0.5f,  0.162460f * 0.5f,  // rarm
        -0.500000f  * 0.5f,  0.162460f * 0.5f,  // larm
        0.309017f   * 0.5f, -0.425325f * 0.5f,  // rleg
    };
    
    static float points_MotionPerp2[4] = {
        0.00f, -0.25f,
        0.00f,  0.25f,
    };
    
    enum DistributionPattern
    {
        Still,
        Uniform2,
        Uniform4,
        Uniform4_Helix,
        Uniform4_DoubleHelix,
        SkewButterfly,
        Rotated4,
        Rotated4_Helix,
        Rotated4_Helix2,
        Poisson10,
        Pentagram,
        Halton_2_3_X8,
        Halton_2_3_X16,
        Halton_2_3_X32,
        Halton_2_3_X256,
        MotionPerp2,
    };
    
    ///   Camera.
    class CCamera
    {
        NEKO_NONCOPYABLE( CCamera );
        
    public:
        
        //!  Camera frustum.
        CFrustum    m_CommonFrustum;
        
        //! Camera position. Only 'lastPosition' property will affect this.
        Vec3   vEye;
        
        //!  Camera rotation.
        Vec3   vLook; // Direction.
        
        //!  Camera up vector.
        Vec3   vUp;
        //!  Camera right vector.
        Vec3   vRight;

        /**
         *  Constructor.
         */
        CCamera();
        
#   if defined( __APPLE__ )
        // Relative to view mouse coordinates.
        float _lastX, _lastY, _lastXt, _lastYt;
#   endif
        
        /**
         *  Initialize our new camera instance.
         */
        void                Initialize( void );
        
        /**
         *  Called before camera culling.
         */
        void                OnPreCull();
        
        /**
         *  Destroy the camera entity.
         */
        void                Destroy();
        
        /**
         *  Reset camera.
         */
        void                Reset( void );
        
        /**
         *  Check for movement.
         */
        void                Movement( char key );
        
        /**
         *  Camera update.
         */
        void                    Frame( float msec );
        
//        /**
//         *  Update movement.
//         */
//        void                UpdateMovement( unsigned int msec );
        
        /**
         *  Update rotation matrices.
         */
        void                UpdateMatrices();
        
        /**
         *  Normalize rotation angles.
         */
        void                    NormalizeAngles();
        
        /**
         *  Update view matrix.
         */
        void                UpdateViewMatrix();
        
        /**
         *  Get perspective corner ray for UV.
         */
        Vec4               GetPerspectiveProjectionCornerRay( float texelOffsetX, float texelOffsetY );
        
        /**
         *  Get perspective projection.
         */
        ncMatrix4               GetPerspectiveProjection( float texelOffsetX, float texelOffsetY );
        ncMatrix4               GetPerspectiveProjection( float left, float right, float bottom, float top, float near, float far );
        
        /**
         *  Last mouse position.
         */
        Vec2i  g_lastMousePosition;
        
        /**
         *  New mouse position.
         */
        Vec2i  g_curMousePosition;
        
        /**
         *  Last time since camera moved.
         */
        const float &                   GetLastMoveTime() const;
        
        /**
         *  Get last move position.
         */
        const Vec3&                GetLastMovePosition() const;
        
        /**
         *  Shake camera.
         */
        void                Shake( float deltaTime );
        
        // Matrices.
        // Note, OpenGLs MVP is PVM.
        ncMatrix4 RotationMatrix;
        ncMatrix4 ProjectionMatrix;
        ncMatrix4 ProjectionMatrix2;
        ncMatrix4 ViewMatrix;
        //        ncMatrix4 NormalMatrix;
        ncMatrix4 ProjectionInvertMatrix;
        ncMatrix4 ViewProjectionMatrix;
        // Don't touch our new buddy.
        ncMatrix4 prevViewProj;
        
//        ncMatrix4 vOrthoMatrix;
        
        /**
         *  Update previous view projection, etc..
         */
        void                OnAfterFrame();
        
        /**
         *  Is camera initialized?
         */
        inline bool                 IsInitialized() const {     return bInitialized; }

        // Another cool stuff.

        /**
         *  Set camera position.
         */
        void                SetPosition( const Vec3 pos, const float time );
        
        /**
         *  Last camera position.
         */
        inline Vec3 &              GetLastPosition( void ) {     return lastPosition; }
        
        /**
         *  Set projection far property.
         */
        void                SetFarDistance( const float _farValue );
        
        /**
         *  Set projection near property.
         */
        void                SetNearDistance( const float _nearValue );
        
        const float &               GetNearPlaneDistance() const;
        const float &               GetFarPlaneDistance() const ;
        
        /**
         *  Update frustum?
         */
        inline const bool UpdateFrustum() const {
            return bUpdateFrustum;
        }
        
        /**
         *  Update camera?
         */
        inline const bool UpdateCamera() const {
            return bUpdateCamera;
        }
        
        /**
         *  Set camera update activeness.
         */
        inline void SetUpdateCamera( bool _update ) {
            bUpdateCamera = _update;
        }
        
        /**
         *  Set camera frustum activeness.
         */
        inline void SetUpdateFrustum( bool _update ) {
            bUpdateFrustum = _update;
        }
        
        /**
         *  Get camera entity object.
         */
        inline CPhysicsObject *                 GetCameraEnt() const    {       return m_hObject;   }
        
        //! Last camera height.
        float   m_fLastPositionY;
        
        //! Last camera origin. Affects current position.
        Vec3    lastPosition;
        
        //! Camera near distance.
        float   m_fNearDist;
        //! Camera far distance.
        float   m_fFarDist;
        
        //! Not a constant! Recieves realtime values.
        Vec2   m_fGamepadMoveAxisSensitivity;
        
        //! Is current camera controlled by a gamepad?
        uint8_t    m_bIsGamepad:1;
        
        //! Jitter camera? Used for SSTAA.
        bool  m_bJitter;
        
        //! Sample offset.
        Vec4    m_vActiveSample; // xy = current sample, zw = previous sample
        //! Active offset.
        int32_t m_iActiveIndex;
        
        
    private:
        
        /**
         *  Sample jitter offset.
         */
        Vec2                Sample( DistributionPattern pattern, const int32_t index );
        
        //! Motion origin.
        Vec3 m_vFocalMotionPos;
        //! Motion direction.
        Vec3 m_vFocalMotionDir;
        
        //! Camera jitter pattern.
        DistributionPattern m_ePattern;
        //! Jitter pattern scale.
        float m_fPatternScale;
        
        CPhysicsObject  * m_hObject;
        
        bool bInitialized;
        
        float m_fLastMoveAt;
        float m_fLastLookAt;
        
        float m_horizontalAngle;
        float m_verticalAngle;

        float m_fCameraSpeed;
        
        bool bUpdateCamera;
        bool bUpdateFrustum;
        
        bool bSprinting;

        Vec3 lastRotation;
    };
    
    extern SConsoleVar    * GameView_FieldOfView;
}
#endif
