#ifndef MAIN32_H_INCLUDED
#define MAIN32_H_INCLUDED


#ifdef _WIN32

#include "../../Core/Core.h"
#include "../Shared/System.h"
#include "../../Core/Console/Console.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Graphics/OpenGL/OpenGLBase.h"
#include "../../Core/Player/Input/Input.h"
#include "../../Core/Player/Camera/Camera.h"
#include "../../Platform/Shared/SystemShared.h"
namespace Neko {
    //
    //  Console class.
    //
    class ncWindowsConsole {
    public:
        ncWindowsConsole( HINSTANCE hInstance );
        void        SetDone();

        HWND		hWnd;
        HWND		hwndBuffer;

        HFONT		hfBufferFont;
        HWND		hwndInputLine;

        char		consoleText[512];
        int	windowWidth, windowHeight;

        WNDPROC		SysInputLineWndProc;
    };

    extern ncWindowsConsole *wConsole;

    //
    //  Window class.
    //
    class ncMain32 {
    public:
        ncMain32();
        ~ncMain32();

        //
        //   Renderer thread caller method.
        //
        void CreateRendererThread();

        //
        //  Initialize renderer and load its game assets.
        //
        void InitializeRT();

        //
        //  Preload.
        //
        void Load();

        //
        //  Create console.
        //
        void CreateConsole();

        LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

        //
        //  Create window and load game engine.
        //
        void        Initialize();

        //
        //  Register Raw input devices.
        //
        void        RegisterInputDevices();

        //
        //  Show window.
        //
        void        Show();

        //
        //  Remove window.
        //
        void        RemoveWindow();

        //
        //  Get last mouse position.
        //
        inline void GetMousePosition( int &outX, int &outY ) {
            outX = _lastX;
            outY = _lastY;
        }

        //
        //  Set last mouse position.
        //
        inline void SetMousePosition( int inX, int inY ) {
            _lastX = inX;
            _lastY = inY;
        }

        //
        //  Window handle.
        //
        inline HWND GetHWND() {
            return h_Window;
        }

        //
        //  Window graphics handle.
        //
        inline HGLRC GetGLRC() {
            return h_GraphicsContext;
        }

        //
        //  Device context.
        //
        inline HDC GetHDC() {
            return h_DeviceContext;
        }

        //
        //  Needs to be edited by renderer thread, moved here.
        HGLRC       h_GraphicsContext;

    private:
        HWND        h_Window;
        HDC         h_DeviceContext;

        //
        //  Last mouse data.
        //
        int _lastX;
        int _lastY;
    };

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    static ncMain32* ApplicationHandle = 0;

    extern ncMain32 *wWindow;


}

#endif
#endif // MAIN32_H_INCLUDED
