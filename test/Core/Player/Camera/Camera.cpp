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
//  Camera.cpp
//  Game camera.. :o
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "../../Joystick.h"
#include "../../Core.h"
#include "Camera.h"
#include "../../Console/Console.h"
#include "../../Utilities/Utils.h"
#include "../Input/Input.h"
#include "../../../World/BeautifulEnvironment.h"
#include "../../../Platform/Shared/System.h"
#include "../../Network/MultiplayerClient.h"

//#include <float.h>

// Get any device input value.
#   if defined( NEKO_APPLE_FAMILY )
    #include "MacUtilities.h"

#   if !defined( NEKO_IOS )// u woot
    #include "iOSUtilities.h"
#   endif

#   else
    #include "../../../Platform/Windows/Main32.h" // Input values, should be moved to its Input class.
#   endif

namespace Neko {
    
    //!  Default mouse yaw sensivity.
    const static float kBaseMouseVerticalSensivity = 0.1f;
    //!  Default mouse pitch sensivity.
    const static float kBaseMouseHorizontalSensivity = 0.1f;
    
    
    //!  Default controller yaw sensivity.
    const static float kBaseGamepadVerticalSensivity = 12.1f;
    //!  Default controller pitch sensivity.
    const static float kBaseGamepadHorizontalSensivity = 12.1f;
    

    //!  Field of view.
    SConsoleVar     * GameView_FieldOfView = 0;
    
    /**
     *  Camera instance.
     */
    CCamera::CCamera()
    {
        
    }
    
    static void TransformPattern( float * seq, const int32_t length, const float theta, const float scale );
    static float HaltonSeq( int32_t prime, int32_t index = 1 );
    
    static void InitializeHalton_2_3( float * seq, const int32_t length );
    
    static int32_t AccessLength2( DistributionPattern pattern );
    static int32_t AccessLength( DistributionPattern pattern );
    static const float * AccessPointData( DistributionPattern pattern );
    
    // Jitter offset.
    float * points_Halton_2_3_x8 = NEKO_NULL;
    float * points_Halton_2_3_x16 = NEKO_NULL;
    float * points_Halton_2_3_x32 = NEKO_NULL;
    float * points_Halton_2_3_x256 = NEKO_NULL;

    /**
     *  Sample camera jitter offset.
     *
     *  @param pattern <#pattern description#>
     *  @param index   <#index description#>
     *
     *  @return <#return value description#>
     */
    Vec2 CCamera::Sample( DistributionPattern pattern, const int32_t index )
    {
        const float * points = AccessPointData( pattern );
        
        if( points == NEKO_NULL ) {   // not initialized yet
            return Vec2(0.0f, 0.0f);
        }
        
        int32_t n = AccessLength(pattern);
        int32_t i = index % n;
        
        float x = m_fPatternScale * points[2 * i + 0];
        float y = m_fPatternScale * points[2 * i + 1];
        
//        if (pattern != Pattern::MotionPerp2)
        return Vec2(x, y);
//        else
//            return Vector2(x, y).Rotate(Vector2.right.SignedAngle(focalMotionDir));
    }
    
    /**
     *  Get camera perspective projection.
     */
    ncMatrix4 CCamera::GetPerspectiveProjection( float left, float right, float bottom, float top, float n, float f )
    {
        float x = (2.0f * n) / (right - left);
        float y = (2.0f * n) / (top - bottom);
        float a = (right + left) / (right - left);
        float b = (top + bottom) / (top - bottom);
        float c = -(f + n) / (f - n);
        float d = -(2.0f * f * n) / (f - n);
        float e = -1.0f;
        
        ncMatrix4 m;
        
        m.m[0] = x; m.m[4] = 0; m.m[8] = a; m.m[12] = 0;
        m.m[1] = 0; m.m[5] = y; m.m[9] = b; m.m[13] = 0;
        m.m[2] = 0; m.m[6] = 0; m.m[10] = c; m.m[14] = d;
        m.m[3] = 0; m.m[7] = 0; m.m[11] = e; m.m[15] = 0;
        
        return m;
    }
    
    /**
     *  Get camera perpsective projection by texel offset values.
     */
    ncMatrix4 CCamera::GetPerspectiveProjection( float texelOffsetX, float texelOffsetY )
    {
        float oneExtentY = tanf(0.5f * nkMath::Deg2Rad * GameView_FieldOfView->Get<float>());
        float oneExtentX = oneExtentY * g_mainRenderer->renderAspectRatio;
        float texelSizeX = oneExtentX / (0.5f * g_mainRenderer->renderWidth);
        float texelSizeY = oneExtentY / (0.5f * g_mainRenderer->renderHeight);
        float oneJitterX = texelSizeX * texelOffsetX;
        float oneJitterY = texelSizeY * texelOffsetY;
        
        float xm = (oneJitterX - oneExtentX) * m_fNearDist;
        float xp = (oneJitterX + oneExtentX) * m_fNearDist;
        float ym = (oneJitterY - oneExtentY) * m_fNearDist;
        float yp = (oneJitterY + oneExtentY) * m_fNearDist;
        
        return GetPerspectiveProjection( xm, xp, ym, yp, m_fNearDist, m_fFarDist );
    }
    
    /**
     *  Get camera perspective projection for given corner coordinates.
     */
    Vec4 CCamera::GetPerspectiveProjectionCornerRay( float texelOffsetX, float texelOffsetY )
    {
        float oneExtentY = tanf(0.5f * nkMath::Deg2Rad * GameView_FieldOfView->Get<float>());
        float oneExtentX = oneExtentY * g_mainRenderer->renderAspectRatio;
        float texelSizeX = oneExtentX / (0.5f * g_mainRenderer->renderWidth);
        float texelSizeY = oneExtentY / (0.5f * g_mainRenderer->renderHeight);
        float oneJitterX = texelSizeX * texelOffsetX;
        float oneJitterY = texelSizeY * texelOffsetY;
        
        return Vec4( oneExtentX, oneExtentY, oneJitterX, oneJitterY );
    }
    
    /**
     *  Called before camera culls the scene.
     */
    void CCamera::OnPreCull()
    {
//            // update motion dir
//            {
//                Vector3 oldWorld = focalMotionPos;
//                Vector3 newWorld = 0.1f * Vector3(0.0f, 0.0f, 1.0f);// = camera.transform.TransformVector(0.1f * Vector3(1.0f, 0.0f, 0.0f));
//
//                float hey2[16];
//
//                newWorld.TransformVector(g_Core->p_Camera->ViewMatrix);
//
////                var m = Matrix4x4(camera.transform.position, camera.transform.rotation, Vector3.one); //no scale...
////                m = Matrix4x4.Inverse(m);
//                ncMatrix4 var;
//                var.Identity();
//                var.Translate( g_Core->p_Camera->vEye );
//                var.RotateX( g_Core->p_Camera->vLook.x );
//                var.RotateY( g_Core->p_Camera->vLook.y );
//                var.RotateZ( g_Core->p_Camera->vLook.z );
//                NekoUtils::gluInvertMatrix(var.m, hey2);
//
//                hey2[8] *= -1;
//                hey2[9] *= -1;
//                hey2[10] *= -1;
//                hey2[11] *= -1;
//
//                ncMatrix4 wtf(hey2);
//
//
//                Vector3 oldPoint = (wtf * Vector4(oldWorld, 1.0)).xyz();
//                Vector3 newPoint = (wtf * Vector4(newWorld, 1.0)).xyz();
//                Vector3 newDeltas = (newPoint - oldPoint);
//                Vector3 newDelta = Vector3(newDeltas.x, newDeltas.y, 0.0f);
//
//                float mag = newDelta.Length();
//                if (mag != 0.0f)
//                {
//                    Vector3 dir = newDelta / mag;// yes, apparently this is necessary instead of newDelta.normalized... because facepalm
//                    if (dir.LengthSq() != 0.0f)
//                    {
//                        focalMotionPos = newWorld;
//                        focalMotionDir = dir;
////                        focalMotionDir.x = nkMath::LerpfAlt(focalMotionDir.x, dir.x, 0.2f);
////                        focalMotionDir.y = nkMath::LerpfAlt(focalMotionDir.y, dir.y, 0.2f);
////                        focalMotionDir.z = nkMath::LerpfAlt(focalMotionDir.z, dir.z, 0.2f);
//                        //Debug.Log("CHANGE focalMotionDir " + focalMotionDir.ToString("G4") + " delta was " + newDelta.ToString("G4") + " delta.mag " + newDelta.magnitude);
//                    }
//                }
//            }

        // Update jitter
        if( m_bJitter ) {
            m_iActiveIndex += 1;
            m_iActiveIndex %= AccessLength( m_ePattern );
            
            Vec2 sample = Sample( m_ePattern, m_iActiveIndex );
            m_vActiveSample.z = m_vActiveSample.x;
            m_vActiveSample.w = m_vActiveSample.y;
            m_vActiveSample.x = sample.x;
            m_vActiveSample.y = sample.y;

            ProjectionMatrix = GetPerspectiveProjection( sample.x, sample.y );
        } else {
            m_vActiveSample = Vec4(0.0f);
            m_iActiveIndex = -1;
        }
    }

    /**
     *   Update matrices.
     */
    void CCamera::UpdateMatrices()
    {
        if( !bUpdateCamera ) {
            return;
        }
        
        // @todo - use p_System for system input
        
        // Super hax.
#   if defined( NEKO_APPLE_FAMILY )
        // Mouse.
        ncMacUtilities::OSX_GetDeltaMousePosition( &g_Core->p_Camera->g_curMousePosition.x, &g_Core->p_Camera->g_curMousePosition.y );
        
#   if NEKO_IOS
        // Touches.
        g_curMousePosition.x = nciOSUtilities::GetLastTouchPositionX();
        g_curMousePosition.y = nciOSUtilities::GetLastTouchPositionY();
#   endif
        
#   else
        // RAW DEVICE INPUT
        wWindow->GetMousePosition( g_Core->p_Camera->g_curMousePosition.x, g_Core->p_Camera->g_curMousePosition.y );
#   endif
        
        uint8_t    bMovingForward;
        uint8_t    bMovingBackward;
        uint8_t    bMovingLeft;
        uint8_t    bMovingRight;
        
        // Make a rotation matrix.
        RotationMatrix.Identity();
        
        // sensitivity threshold
        const float threshold = 0.1f;
        
        // Rotation.
        if( m_bIsGamepad && g_Core->m_pGamepad[0] != NEKO_NULL ) {
            float axisHoriz = (float)g_Core->m_pGamepad[0]->GetAxisMapped( EGameControllerAxis::RightX ) / 32767.0f;
            float axisVert = (float)g_Core->m_pGamepad[0]->GetAxisMapped( EGameControllerAxis::RightY ) / 32767.0f;
            
            if( axisHoriz > threshold || axisHoriz < -threshold  ) {
                m_horizontalAngle = kBaseGamepadHorizontalSensivity * axisHoriz;
            } else {
                m_horizontalAngle = 0.0f;
            }
            
            if( axisVert > threshold || axisVert < -threshold ) {
                m_verticalAngle = kBaseGamepadVerticalSensivity * axisVert;
            } else {
                m_verticalAngle = 0.0f;
            }
            
            m_fGamepadMoveAxisSensitivity.x = (float)g_Core->m_pGamepad[0]->GetAxisMapped( EGameControllerAxis::LeftY ) / 32767.0f;
            m_fGamepadMoveAxisSensitivity.y = (float)g_Core->m_pGamepad[0]->GetAxisMapped( EGameControllerAxis::LeftX ) / 32767.0f;
            
            bMovingForward = m_fGamepadMoveAxisSensitivity.x < -threshold;
            bMovingBackward = m_fGamepadMoveAxisSensitivity.x > threshold;
            bMovingLeft = m_fGamepadMoveAxisSensitivity.y < -threshold;
            bMovingRight = m_fGamepadMoveAxisSensitivity.y > threshold;
            
        } else {
            // Mouse input.
            
            m_horizontalAngle = kBaseMouseHorizontalSensivity * g_lastMousePosition.x;
            m_verticalAngle = kBaseMouseVerticalSensivity * g_lastMousePosition.y;
            
            bMovingForward = g_Core->p_Input->keysBase['w'].kHold;
            bMovingBackward = g_Core->p_Input->keysBase['s'].kHold;
            bMovingLeft = g_Core->p_Input->keysBase['a'].kHold;
            bMovingRight = g_Core->p_Input->keysBase['d'].kHold;
        }
        
        // Default (raw) mapping
        //  m_fGamepadMoveAxisSensitivity.y -1 = left 1 = right
        //  m_fGamepadMoveAxisSensitivity.x -1 = forward 1 = backward
        //        printf( "m_fGamepadMoveAxisSensitivity.xy = (%.2f, %.2f)\n", m_fGamepadMoveAxisSensitivity.x, m_fGamepadMoveAxisSensitivity.y );
        
        // this is lame
        
        if( bMovingForward ) {
            g_Core->p_Input->m_bMovingForward = true;
        } else {
            g_Core->p_Input->m_bMovingForward = false;
        }
        
        if( bMovingBackward ) {
            g_Core->p_Input->m_bMovingBackward = true;
        } else {
            g_Core->p_Input->m_bMovingBackward = false;
        }
        
        if( bMovingLeft ) {
            g_Core->p_Input->m_bMovingLeft = true;
        } else {
            g_Core->p_Input->m_bMovingLeft = false;
        }
        
        if( bMovingRight ) {
            g_Core->p_Input->m_bMovingRight = true;
        } else {
            g_Core->p_Input->m_bMovingRight = false;
        }
        
        NormalizeAngles();
        
        // Y axis.
        RotationMatrix.Rotate( -m_verticalAngle, vRight  );
        vLook.TransformVector( RotationMatrix );
        vUp.TransformVector( RotationMatrix );
        
        // X axis.
        RotationMatrix.Rotate( -m_horizontalAngle, VECTOR_UP );
        vLook.TransformVector( RotationMatrix );
        vUp.TransformVector( RotationMatrix );
       
        // Update view matrix.
        UpdateViewMatrix();
        
        // View projection matrix.
        ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
        
        // Last mouse position.
        g_lastMousePosition.x = g_curMousePosition.x;
        g_lastMousePosition.y = g_curMousePosition.y;
        
    }
    
    /**
     *  Normalize view angles.
     */
    void CCamera::NormalizeAngles()
    {
        m_horizontalAngle = fmodf( m_horizontalAngle, 360.0f );
        
        // fmodf can return negative values, but this will make them all positive.
        if( m_horizontalAngle < 0.0f ) {
            m_horizontalAngle += 360.0f;
        }
        
        // Bounds.
        if( m_verticalAngle > MaxVerticalAngle ) {
            m_verticalAngle = MaxVerticalAngle;
        } else if( m_verticalAngle < -MaxVerticalAngle ) {
            m_verticalAngle = -MaxVerticalAngle;
        }
    }
    
    /**
     *  Calculate view matrix.
     */
    void CCamera::UpdateViewMatrix()
    {
        ViewMatrix.Identity();
        
        // View.
        vLook.Normalize();
        
        g_Core->p_Camera->vRight.Cross( vLook, g_Core->p_Camera->vUp );
        g_Core->p_Camera->vRight.Normalize();
        
        vUp.Cross( g_Core->p_Camera->vRight, vLook );
        vUp.Normalize();
        
        // Setup view matrix.
        ViewMatrix.m[0] =  vRight.x;
        ViewMatrix.m[1] =  vUp.x;
        ViewMatrix.m[2] = -vLook.x;
        ViewMatrix.m[3] =  0.0f;
        
        ViewMatrix.m[4] =  vRight.y;
        ViewMatrix.m[5] =  vUp.y;
        ViewMatrix.m[6] = -vLook.y;
        ViewMatrix.m[7] =  0.0f;
        
        ViewMatrix.m[8]  =  vRight.z;
        ViewMatrix.m[9]  =  vUp.z;
        ViewMatrix.m[10] = -vLook.z;
        ViewMatrix.m[11] =  0.0f;
        
        ViewMatrix.m[12] = -Vec3::Dot( vRight, vEye );
        ViewMatrix.m[13] = -Vec3::Dot( vUp, vEye );
        ViewMatrix.m[14] =  Vec3::Dot( vLook, vEye );
        ViewMatrix.m[15] =  1.0f;
    }
    
    /**
     *  Stuff to do after rendering frame.
     */
    void CCamera::OnAfterFrame()
    {
        prevViewProj = g_Core->p_Camera->ProjectionMatrix * g_Core->p_Camera->ViewMatrix;
    }
    
    /**
     *  Destroy the camera entity.
     */
    void CCamera::Destroy()
    {
        delete m_hObject;
    }
    
    /**
     *  Initialize new camera.
     */
    void CCamera::Initialize()
    {
        int32_t     i;
        
        g_Core->p_Console->Print( LOG_INFO, "Camera initializing...\n" );
        
        // Initial stuff.
        vEye = Vec3( 0.0f, 6.0f, 0.0f );
        vLook = lastRotation = Vec3( -10.5f, -0.5f, -0.5f );
        vUp = Vec3( 1.0f, 1.0f, 0.0f );
        vRight = Vec3( 1.0f, 0.0f, 0.0f );
        
        // Setup player object.
        m_hObject = new CPhysicsObject();
        
        m_hObject->SetRadius( 7.0f );
        m_hObject->SetObjectFlag( CPhysicsObject::PHYS_CHARACTER );
        m_hObject->SetObjectPos( vEye );
        m_hObject->SetMass( 1.0f );
        m_hObject->SetDimensions( Vec3( -4.0f, -1.0f, -4.0f ), Vec3( 4.0f, 2.0f, 4.0f ) );
        
        bUpdateCamera = false;
        bUpdateFrustum = true;
        
        bSprinting = false;
        m_bIsGamepad = false;
        
        m_fCameraSpeed = 0.02f;
        
        m_bJitter = false;   // SSTAA
        
        m_vFocalMotionPos = Vec3(0.0f);
        m_vFocalMotionDir = Vec3(0.0f, 0.0f, 1.0f);
        
        m_ePattern = DistributionPattern::Halton_2_3_X16;
        m_fPatternScale = 1.0f;
        
        m_vActiveSample = Vec4(0.0f);// xy = current sample, zw = previous sample
        m_iActiveIndex = -1;
        
        // Register camera console variables.
        GameView_FieldOfView = g_Core->p_Console->RegisterCVar( ECvarGroup::Player, "iCameraFov", "Camera Field of View.", 45.0f, CVFlag::NeedsRefresh, ECvarType::Float );
        
        m_fLastMoveAt = 0.0f;
        m_fLastLookAt = 0.0f;
        lastPosition = Vec3( 0.0f, 6.0f, 0.0f );
        
        // Reset matrices.
        ViewMatrix.Identity();
        RotationMatrix.Identity();
        prevViewProj.Identity();
        ViewProjectionMatrix.Identity();
        
        // Setup projection matrix.
        // 2.0f for OVR.
        const float aspectRatio = Render_Width->Get<float>() / ( Render_OVR->Get<float>() ? 2.0f : 1.0f ) / Render_Height->Get<float>();
        const float fieldOfView = GameView_FieldOfView->Get<float>();
        
        // Default z things!
        SetNearDistance( 0.1f );
        SetFarDistance( 3000.0 );
        
        // Setup projection matrix.
        ProjectionMatrix.CreatePerspective( fieldOfView, aspectRatio, m_fNearDist, m_fFarDist );
        
        // Setup projection invert matrix.
        SMemoryTempFrame * invMatrixData = _PushMemoryFrame( pClassLinearAllocator );
        float *tempMatrix = (float*)PushMemory( invMatrixData, sizeof(float) * 16 );
        
        // Use standart GLU library function.
        NekoUtils::gluInvertMatrix( g_Core->p_Camera->ProjectionMatrix.m, tempMatrix );
        
        for( i = 0; i < 16 ; ++i ) {
            ProjectionInvertMatrix.m[i] = (tempMatrix[i]);
        }
        
        _PopMemoryFrame( invMatrixData );
        
        // Initialize frustum jitter ( Temporal Anti-aliasing )
        
        // points_Pentagram
        Vec2 vh = Vec2(points_Pentagram[0] - points_Pentagram[2], points_Pentagram[1] - points_Pentagram[3]);
        Vec2 vu = Vec2(0.0f, 1.0f);
        TransformPattern( (float *)points_Pentagram, 10, nkMath::Deg2Rad * (0.5f * Vec2::Angle(vu, vh)), 1.0f );
        
        points_Halton_2_3_x8 = new float[8 * 2];
        points_Halton_2_3_x16 = new float[16 * 2];
        points_Halton_2_3_x32 = new float[32 * 2];
        points_Halton_2_3_x256 = new float[256 * 2];
        
        // points_Halton_2_3_xN
        InitializeHalton_2_3(points_Halton_2_3_x8, 8 * 2);
        InitializeHalton_2_3(points_Halton_2_3_x16, 16 * 2);
        InitializeHalton_2_3(points_Halton_2_3_x32, 32 * 2 );
        InitializeHalton_2_3(points_Halton_2_3_x256, 256 * 2);
        
        printf("local points_Halton_2_3_x256 = { ");
        for (int i = 0; i < 256 * 2; ++i)
        {
            printf( "%f,", points_Halton_2_3_x256[i]);
        }
        printf("}\n");
        
        bInitialized = true;
        g_Core->p_Console->Print( LOG_INFO, "Camera( perspective;%7.1f;%7.1f ) - created.\n", m_fNearDist, m_fFarDist );
    }
    
    /**
     *  Reset camera position, rotation, etc.
     */
    void CCamera::Reset()
    {
        vEye = Vec3( 0.0f, 6.0f, 0.0f );
        vLook = lastRotation = Vec3( -10.5f, -0.5f, -0.5f );
        vUp = Vec3( 1.0f, 1.0f, 0.0f );
        vRight = Vec3( 1.0f, 0.0f, 0.0f );
        
        g_Core->p_Console->Print( LOG_INFO, "Camera(): reset\n" );
    }
    
    /**
     *  Camera update function.
     *
     *  @param msec Time in seconds.
     */
    void CCamera::Frame( float msec )
    {
        if( !bUpdateCamera ) {
            return;
        }
        
        float t;
        
        // Time movement delay since last move.
        t = nkMath::Clamp( ( msec / 1000.0f ), 0.0f, 1.0f );
        
        // Update camera physics object.
        m_hObject->SetObjectPos( vEye );
        
        // Smooth camera.
        vEye = (vEye * t) + (lastPosition * (1.0f - t) );
        //        vEye.x = nkMath::LerpfAlt( vEye.x, lastPosition.x/* + displacement.z*/, t );
        //        vEye.y = nkMath::LerpfAlt( vEye.y, lastPosition.y/* + displacement.y*/, t );
        //        vEye.z = nkMath::LerpfAlt( vEye.z, lastPosition.z/* + displacement.x*/, t );
        
        // Update rotation.
        //vLook.x = lastRotation.x;//nkMath::LerpfAlt( vLook.x, lastRotation.x, (t / 30.0f) / 15.0f );
        //vLook.y = lastRotation.y;//nkMath::LerpfAlt( vLook.y, lastRotation.y, (t / 30.0f) / 15.0f );
        //vLook.z = lastRotation.z;//nkMath::LerpfAlt( vLook.z, lastRotation.z, (t / 30.0f) / 15.0f );
    }
    
    
    /**
     *  Get last move time.
     */
    const float & CCamera::GetLastMoveTime( void ) const    {       return m_fLastMoveAt;   }
    
    /**
     *   Get last move position.
     */
    const Vec3& CCamera::GetLastMovePosition( void ) const {       return lastPosition;    }
    
    /**
     *  Set camera position.
     *
     *  @param pos      A new camera origin.
     *  @param time     Time in seconds when moved.
     */
    void CCamera::SetPosition( const Vec3 pos, const float time )
    {
        if( pos.x == 0.0f || pos.y == 0.0f || pos.z == 0.0f ) {
            return;
        }
        
        // TODO: FIXME: lastPosition throws NaN values somehow.
        if( std::isnan( pos.x ) || std::isnan( pos.y ) || std::isnan( pos.z ) ) {
            printf( "packet loss ( nan value )\n" );
            return;
        }
        
        if( std::isinf( pos.x ) || std::isinf( pos.y ) || std::isinf( pos.z ) ) {
            printf( "packet loss ( inf value )\n" );
            return;
        }
        
        // Check if we have moved.
        if( pos.LengthSq() != 0.0f ) {
            lastPosition.x = pos.x;
            lastPosition.y = pos.y;
            lastPosition.z = pos.z;
            
            m_fLastMoveAt = time; // g_Core->GetTime();
        }
    }
    
    /**
     *  Transform point pattern.
     */
    static void TransformPattern( float * seq, const int32_t length, const float theta, const float scale )
    {
        float cs = cosf( theta );
        float sn = sinf( theta );
        
        for( int32_t i = 0, j = 1, n = length; i != n; i += 2, j += 2 ) {
            float x = scale * seq[i];
            float y = scale * seq[j];
            seq[i] = x * cs - y * sn;
            seq[j] = x * sn + y * cs;
        }
    }
    
    // http://en.wikipedia.org/wiki/Halton_sequence
    static float HaltonSeq( int32_t prime, int32_t index  )
    {
        float r = 0.0f;
        float f = 1.0f;
        int32_t i = index;
        
        while( i > 0 ) {
            f /= prime;
            r += f * (i % prime);
            i = static_cast<int32_t>(floorf((float)i / (float)prime));
        }
        return r;
    }
    
    /**
     *  Initialize Halton sequence.
     */
    static void InitializeHalton_2_3( float * seq, const int32_t length )
    {
        for( int32_t i = 0, n = (length / 2); i != n; ++i ) {
            float u = HaltonSeq(2, i + 1) - 0.5f;
            float v = HaltonSeq(3, i + 1) - 0.5f;
            seq[2 * i + 0] = u;
            seq[2 * i + 1] = v;
        }
    }
    
    /**
     *  Point distrubution data.
     */
    static const float * AccessPointData( DistributionPattern pattern )
    {
        switch( pattern )
        {
            case DistributionPattern::Still:
                return points_Still;
            case DistributionPattern::Uniform2:
                return points_Uniform2;
            case DistributionPattern::Uniform4:
                return points_Uniform4;
            case DistributionPattern::Uniform4_Helix:
                return points_Uniform4_Helix;
            case DistributionPattern::Uniform4_DoubleHelix:
                return points_Uniform4_DoubleHelix;
            case DistributionPattern::SkewButterfly:
                return points_SkewButterfly;
            case DistributionPattern::Rotated4:
                return points_Rotated4;
            case DistributionPattern::Rotated4_Helix:
                return points_Rotated4_Helix;
            case DistributionPattern::Rotated4_Helix2:
                return points_Rotated4_Helix2;
            case DistributionPattern::Poisson10:
                return points_Poisson10;
            case DistributionPattern::Pentagram:
                return points_Pentagram;
            case DistributionPattern::Halton_2_3_X8:
                return points_Halton_2_3_x8;
            case DistributionPattern::Halton_2_3_X16:
                return points_Halton_2_3_x16;
            case DistributionPattern::Halton_2_3_X32:
                return points_Halton_2_3_x32;
            case DistributionPattern::Halton_2_3_X256:
                return points_Halton_2_3_x256;
            case DistributionPattern::MotionPerp2:
                return points_MotionPerp2;
            default:
                printf("missing point distribution");
                return points_Halton_2_3_x16;
        }
    }
    
    static int32_t AccessLength2( DistributionPattern pattern )
    {
        switch (pattern)
        {
            case DistributionPattern::Still: return 2;
            case DistributionPattern::Uniform2: return 4;
            case DistributionPattern::Uniform4: return 8;
            case DistributionPattern::Uniform4_Helix:  return 8;
            case DistributionPattern::Uniform4_DoubleHelix: return 12;
            case DistributionPattern::SkewButterfly:  return 8;
            case DistributionPattern::Rotated4: return 8;
            case DistributionPattern::Rotated4_Helix:  return 8;
            case DistributionPattern::Rotated4_Helix2: return 6;
            case DistributionPattern::Poisson10: return 20;
            case DistributionPattern::Pentagram:  return 10;
            case DistributionPattern::Halton_2_3_X8: return 16;
            case DistributionPattern::Halton_2_3_X16: return 32;
            case DistributionPattern::Halton_2_3_X32:  return 64;
            case DistributionPattern::Halton_2_3_X256: return 512;
            case DistributionPattern::MotionPerp2: return 4;
            default:
                printf("missing point distribution");
                return 32;
        }
    }
    
    /**
     *  Pattern length.
     */
    static int32_t AccessLength(DistributionPattern pattern)
    {
        return AccessLength2( pattern ) / 2;
    }
    
    /**
     *  Set far plane distance.
     */
    void CCamera::SetFarDistance( const float _farValue )   {       m_fFarDist = _farValue;  }
    
    /**
     *   Set near plane distance.
     */
    void CCamera::SetNearDistance( const float _nearValue ) {       m_fNearDist = _nearValue;    }
    
    /**
     *  Get near plane distance.
     */
    const float &               CCamera::GetNearPlaneDistance() const {       return m_fNearDist;  }
    
    /**
     *  Get far plane position.
     */
    const float &               CCamera::GetFarPlaneDistance() const  {       return m_fFarDist;   }
}
