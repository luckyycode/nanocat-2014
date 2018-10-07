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
//  Main64.cpp
//  Neko engine
//
//  Created by Neko Vision on 31/12/2013.
//  Copyright (c) 2013 Neko Vision. All rights


#ifdef _WIN32

#include <direct.h>
#include "Main32.h"
//#include "D3D11Base.h"

//
// Use this for raw mouse input.
#   if !defined( HID_USAGE_PAGE_GENERIC )
        #define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#   endif
#   if !defined( HID_USAGE_GENERIC_MOUSE )
        #define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#   endif


namespace Neko {

    // @TODO
#if defined( _WIN64 )// MS!!!!! Update this dayum library!
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
#endif

    #define MAIN_WINDOW_NAME     "Neko engine"
    #define MAIN_WINDOW_CLASS    "Win64SkyCat"

    ncMain32    * wWindow = 0;

    /**
     *  Constructor.
     */
    ncMain32::ncMain32()
    {

    }

    /**
     *  Destructor.
     */
    ncMain32::~ncMain32()
    {
        RemoveWindow();
    }

    /**
     *  Create game console.
     */
    void ncMain32::CreateConsole()
    {
        // Create new console.
        HINSTANCE hInstanceForConsole = (HINSTANCE)GetModuleHandle(NULL);

        wConsole = new ncWindowsConsole( hInstanceForConsole );
        // Show the window.
        wConsole->SetDone();
    }

    /**
     *  Load game systems.
     */
    void ncMain32::Load()
    {
        char    _cwd[MAX_PATH];

        // Reset timing.
        srand( time((unsigned)NULL) );
		g_Core = new CCore();

		// init devices

        _getcwd( _cwd, MAX_PATH ); // getcwd is deprecated, use _getcwd

        // We need to initialize some core stuff.
        g_Core->Preload( _cwd );
    }


    /**
     *  Initialize input.
     */
    void ncMain32::RegisterInputDevices()
    {
        g_Core->p_Console->Print( LOG_INFO, "Creating input devices..\n" );

        // Create basic mouse input.
        RAWINPUTDEVICE Rid[1];

        Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
        Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
        Rid[0].dwFlags = RIDEV_INPUTSINK;
        Rid[0].hwndTarget = h_Window;

        RegisterRawInputDevices( Rid, 1, sizeof(Rid[0]) );
    }

    /**
     *  Message handler.
     */
    LRESULT CALLBACK ncMain32::MessageHandler( HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam ) {
        static int32_t _mx, _my;

        switch( umsg )
        {
            case WM_INPUT: {
				// 32 bits
//                UINT dwSize = 40;
//                static BYTE lpb[40];

				// 64 bits
				uint8_t lpb[sizeof(RAWINPUT)] = {};
				uint32_t dwSize = sizeof(RAWINPUT);
                GetRawInputData( (HRAWINPUT)lparam, RID_INPUT, lpb, &dwSize, sizeof( RAWINPUTHEADER ) );

                RAWINPUT* raw = (RAWINPUT*)lpb;

                if( raw->header.dwType == RIM_TYPEMOUSE ) {
                    int32_t xPosRelative = raw->data.mouse.lLastX * 1.7;
                    int32_t yPosRelative = raw->data.mouse.lLastY * 1.7;

                    wWindow->SetMousePosition( xPosRelative, yPosRelative );
                }

                break;
            }

            // mouse down
            case WM_LBUTTONDOWN:
                g_Core->p_Input->OnMouseDown( GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), MB_LEFTKEY );
            break;

            // mouse up
            case WM_LBUTTONUP:
                g_Core->p_Input->OnMouseUp( GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), MB_LEFTKEY );
            break;

            case WM_KEYUP:
                g_Core->p_Input->OnKeyUp( (char)wparam );
                break;

            case WM_KEYDOWN: {
                g_Core->p_Input->OnKeyPress( (char)wparam );

                switch (wparam) {
                    case VK_ESCAPE:
                        PostQuitMessage(0);
                    break;
                }
            }
            break;


            // Any other messages send to the default message handler as our application won't make use of them.
            default: {
                return DefWindowProc(hwnd, umsg, wparam, lparam);
            }
        }
    }

    /**
     *  Show main window.
     */
    void ncMain32::Show()
    {
        ShowWindow( h_Window, 1 );
        UpdateWindow( h_Window );

        SetFocus( h_Window );
    }

    /**
     *  Remove window with its graphical content.
     */
    void ncMain32::RemoveWindow()
    {
        wglMakeCurrent( NULL, NULL );
        wglDeleteContext( h_GraphicsContext );
        ReleaseDC( h_Window, h_DeviceContext );

        DestroyWindow( h_Window );
    }


    /**
     *  Create new main window.
     */
    LRESULT CALLBACK WindowProc( HWND, UINT, WPARAM, LPARAM );
    void ncMain32::Initialize()
    {
        Load();

        DWORD _exstyle, _style;
        HINSTANCE hInstance;
        WNDCLASSEX wcex;

        // Make a bit spacing between lines.
        g_Core->p_Console->Print( LOG_NONE, "\n" );

        ApplicationHandle = this;

        if( g_Core->UsesGraphics() ) {
            // Register the window class.
            g_Core->p_Console->Print( LOG_INFO, "Creating window...\n" );

            hInstance               = GetModuleHandle( NULL );
            wcex.cbSize             = sizeof(WNDCLASSEX);
            wcex.style              = CS_OWNDC; // CS_HREDRAW | CS_VREDRAW |
            wcex.lpfnWndProc        = WindowProc;
            wcex.cbClsExtra         = 0;
            wcex.cbWndExtra         = 0;
            wcex.hInstance          = hInstance;
            wcex.hIcon              = LoadIcon(NULL, IDI_APPLICATION);
            wcex.hCursor            = LoadCursor(NULL, IDC_ARROW);
            wcex.hbrBackground      = (HBRUSH)GetStockObject( BLACK_BRUSH );
            wcex.lpszMenuName       = NULL;
            wcex.lpszClassName      = MAIN_WINDOW_CLASS;
            wcex.hIconSm            = (HICON)LoadImage( NULL, "icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED ); //LoadIcon( NULL, IDI_APPLICATION );

            if( !RegisterClassEx( &wcex ) ) {
                g_Core->p_Console->Error( ERR_FATAL, "Could not register window class." );
                return;
            }

            RECT wrect;

            wrect.left      = 0;
            wrect.right     = Render_Width.Get<int>();
            wrect.top       = 0;
            wrect.bottom    = Render_Height.Get<int>();

            // Select window mode.
            if( Render_Fullscreen.Get<bool>() ) {
                DEVMODE dmScreenSettings;

                dmScreenSettings.dmSize			= sizeof(DEVMODE);
                dmScreenSettings.dmPelsWidth	= wrect.right;
                dmScreenSettings.dmPelsHeight	= wrect.bottom;
                dmScreenSettings.dmBitsPerPel	= 32;
                dmScreenSettings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

                if( ChangeDisplaySettings( &dmScreenSettings, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL ) {
                    g_Core->p_Console->Print( LOG_ERROR, "Could not set full screen mode. Probably wrong resolution set.\n" );
                    g_Core->p_Console->Print( LOG_INFO, "Setting windowed mode..\n" );

                    Render_Fullscreen.Set( false );        // Lock further changes.
                    Render_Fullscreen.Lock();
                }

            } else // Windowed.
                g_Core->p_Console->Print( LOG_INFO, "Setting windowed mode..\n" );


            // Full-screen mode is still on is still enabled, so we are going to create the full screen window.
            if( Render_Fullscreen.Get<bool>() ) {
                g_Core->p_Console->Print( LOG_INFO, "Setting fullscreen mode..\n" );

                _exstyle = WS_EX_APPWINDOW;
                _style = WS_POPUP;
            } else {    // Extended style.

                _exstyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
                _style = WS_OVERLAPPEDWINDOW;
            }

            AdjustWindowRectEx( &wrect, _style, FALSE, _exstyle );

            // Create window now.
            if ( !( h_Window = CreateWindowEx( _exstyle,
                                        MAIN_WINDOW_CLASS,
                                        MAIN_WINDOW_NAME,
                                        WS_CLIPSIBLINGS |
                                        WS_CLIPCHILDREN |
                                        _style,
                                        0, 0,
                                        wrect.right,
                                        wrect.bottom,
                                        NULL,
                                        NULL,
                                        hInstance,
                                        NULL )) ) {

                g_Core->p_Console->Error( ERR_FATAL, "Could not create main window." );
                return;
            }

            // Get device context.
            h_DeviceContext = GetDC( h_Window );
            g_Core->p_Console->Print( LOG_INFO, "Created window. Resolution: %ix%i\n", Render_Width.Get<int>(), Render_Height.Get<int>() );

            RegisterInputDevices();
		}


		InitializeRT();

       // gl_Core->Initialized = true;

        // Resize window manually.
       // gl_Core->OnResize( Render_Width.Get<int>(), Render_Height.Get<int>() );

        //volatile bool _quit = false;


    }

    /**
     *  Windows events.
     */
    LRESULT CALLBACK WindowProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
        static int _width;
        static int _height;

        switch (uMsg)
        {
            case WM_SIZE:
                _width = LOWORD(lParam);
                _height = HIWORD(lParam);

                //gl_Core->OnResize( _width, _height );
            break;

            case WM_CLOSE:
                g_Core->Quit( "User quit." );
                PostQuitMessage(0);
            break;

            case WM_DESTROY:
                return 0;

            default: {
                return ApplicationHandle->MessageHandler( hwnd, uMsg, wParam, lParam );
            }
        }

        return 0;
    }


    /**
     *  Rendering stuff.
     */


//  User defined window messages the render thread uses to communicate.
#define UWM_PAUSE   (WM_APP + 1)
#define UWM_RESIZE  (WM_APP + 2)
#define UWM_SHOW    (WM_APP + 3)
#define UWM_STOP    (WM_APP + 4)

    static void __threadProc( UINT uMsg, WPARAM wParam, LPARAM lParam )
    {
        switch(uMsg)
        {
            case UWM_PAUSE:

                // notification that we should either pause or resume rendering

                break;

            case UWM_RESIZE:

                // notification that viewport(s) need(s) to be resized

                break;

            case UWM_STOP:

                // notification that we should stop the render thread from executing

                break;
        }
    }

	static bool wtf = true;

    /**
     *  Create OpenGL context.
     */
    static void CreateGLContext( INekoThread * thread, void * arg1, void * arg2 ) {

		ncMain32 * _base = (ncMain32*)arg1;

        //  Query for a pixel format that fits the attributes we want.
        PIXELFORMATDESCRIPTOR pfd;
        int _format = 0, result = 0;

        //
        //  Create default OpenGL context with its reported system version.
        //

        // Pixel format setup.
        ZeroMemory( &pfd, sizeof(pfd) );

        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 8;
        pfd.cDepthBits = 24;
        pfd.iLayerType = PFD_MAIN_PLANE;

        _format = ChoosePixelFormat( _base->GetHDC(), &pfd );

        if ( !SetPixelFormat( _base->GetHDC(), _format, &pfd ) ) {
            g_Core->p_Console->Error( ERR_FATAL, "Failed to setup window pixel format.\n" );
            return;
        }

        _base->h_GraphicsContext = wglCreateContext( _base->GetHDC() );
        if( _base->h_GraphicsContext == NULL ) {
            g_Core->p_Console->Error( ERR_OPENGL, "Could not create OpenGL context.\n" );
            return;
        }

        // Set the rendering context to active.
        result = wglMakeCurrent( _base->GetHDC(), _base->h_GraphicsContext );
        if( result != 1 ) {
            g_Core->p_Console->Print( LOG_INFO, "Couldn't set OpenGL context.\n" );
            return;
        }

        g_Core->p_Console->Print( LOG_INFO, "Reported OpenGL version: %s\n", glGetString( GL_VERSION ) );

        //
        //  Important, let extensions load now, so we can use OpenGL 3 and higher things!
        //
//        m_OpenGL->LoadExtensionList();
		// Load the OpenGL extensions that this application will be using.
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if( wglChoosePixelFormatARB == NULL ) {
            g_Core->p_Console->Error( ERR_FATAL, "WTF");
        }

        // After OpenGL context created, use GLEW library.
        // It's a really good library! =)
        glewInit();

        int pixfmt[8];
        unsigned int numpf;
        static PIXELFORMATDESCRIPTOR pfds =
        {
              sizeof(PIXELFORMATDESCRIPTOR),
              1,
              PFD_DRAW_TO_WINDOW |
              PFD_SUPPORT_OPENGL |
              PFD_DOUBLEBUFFER,
              PFD_TYPE_RGBA,
              32,
              0, 0, 0, 0, 0, 0,
              0,
              0,
              0,
              0, 0, 0, 0,
              24,
              0,
              0,
              PFD_MAIN_PLANE,
              0,
              0, 0, 0
        };


        // WGL_CONTEXT_MAJOR_VERSION_ARB
        // WGL_CONTEXT_MINOR_VERSION_ARB

        // WGL_CONTEXT_FLAGS_ARB

        // WGL_CONTEXT_PROFILE_MASK_ARB

        const static int attribs[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 1,
            WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0, 0
        };

        const static int piAttribIList[] = {
            WGL_DRAW_TO_WINDOW_ARB, TRUE,
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
            WGL_SUPPORT_OPENGL_ARB, TRUE,
            WGL_DOUBLE_BUFFER_ARB, TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 24,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0, 0
        };

        wglChoosePixelFormatARB( _base->GetHDC(), piAttribIList, NULL, 8, pixfmt, &numpf );

        SetPixelFormat( _base->GetHDC(), pixfmt[0], &pfds );
        HGLRC hRC2 =  wglCreateContextAttribsARB( _base->GetHDC(), 0, attribs );

        // If we couldn't create context...
        if( !hRC2 || !wglMakeCurrent( _base->GetHDC(), hRC2 ) ) {
            g_Core->p_Console->Error( ERR_FATAL, "Could not to create context or make it active. ( GetLastError = %d )\n", GetLastError( ) );
            return;
        }

        g_Core->p_Console->Print( LOG_INFO, "Created OpenGL context..\n" );


		// OpenGL preferences ( calls renderer load ).
		g_pGraphicsManager = new GraphicsManager();
		// Initialize OpenGL and Neko Renderer.
		g_pGraphicsManager->Initialize(GraphicsManager::INTERFACE_OPENGL);

		g_pGraphicsManager->GetCurrentInterface()->OnLoad();

		// Show window.
		wWindow->Show();
		g_mainRenderer->Time = 0;
		volatile bool bRendererQuit = false;

		unsigned int LastTime = 0;
		unsigned int FrameTime = 0;
		unsigned int RendererTime = 0;

		MSG msg;

		INekoThreadLock * lock = CCore::CreateLock();

		while (!bRendererQuit) {

			lock->Lock();


			// Windows messages.
			if (PeekMessage(&msg, NULL, UWM_PAUSE, UWM_STOP, PM_REMOVE))
				__threadProc(msg.message, msg.wParam, msg.lParam);

			int msec, minMsec;
			float scale;

			// Main game time count.
			if (60 > 1) {
				minMsec = 1000 / 60;
			}
			else {
				minMsec = 1;
			}

			do {
				FrameTime = g_Core->p_System->Milliseconds();
				if (LastTime > FrameTime) {
					LastTime = FrameTime;
				}
				msec = FrameTime - LastTime;
			} while (msec < minMsec);

			LastTime = FrameTime;

			scale = 1.0f;

			msec = (unsigned int)(msec * scale);

			if (msec < 1)
				msec = 1;
			else if (msec > 5000)
				msec = 5000;

			if (msec > 500)
				g_Core->p_Console->Print(LOG_INFO, "Nanocat renderer overloaded.. or just stuck for %i ms\n", msec);

			g_mainRenderer->Time += msec;

			g_mainRenderer->Render(msec);

			SwapBuffers(wWindow->GetHDC());

			Sleep(1);

			lock->Unlock();
		}

		// Renderer shutdown.
		g_mainRenderer->Shutdown();
    }

    static void RendererThread( INekoThread * thread, void * arg1, void * arg2 ) {
        ApplicationHandle->CreateRendererThread();
    }

    /**
     *  Renderer thread initializer.
     */
    void ncMain32::InitializeRT() {
        if( g_Core->UsesGraphics() ) {
            g_Core->p_Console->Print( LOG_INFO, "Creating renderer thread..\n" );

			INekoThread * rendererThread = CCore::CreateThread( INekoThread::PRIORITY_NORMAL, 1, CreateGLContext, this, nullptr );

           // g_Core->SetRendererThread( rendererThread );
        }
    }

}

#   if defined(_MSC_VER) && _MSC_VER < 1900


int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#   endif
#endif