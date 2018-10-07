//
//  Nanocat engine.
//
//  Window & context creator.
//
//  Created by Neko Vision on 22/08/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

// Broken since I raged on Windows.
// Fix me.
// Fixed.

#include "../../Core/Core.h"
#include "../../Core/Console/Console.h"
#include "../Shared/System.h"
#include "../Shared/SystemShared.h"
#include "../../Core/Console/Console.h"
#include "../../Graphics/Renderer/Renderer.h"
#include "../../Graphics/OpenGL/OpenGLBase.h"
#include "../../Core/Player/Input/Input.h"
#include "../../Core/Player/Camera/Camera.h"

#ifdef _WIN32

#include "Main32.h"
//#include "D3D11Base.h" // F#cking shame, requires me to use only MSVC compiler.. no thx

void ConsoleLogo();


namespace Neko {

    /**
     *    Print logo.
     *    Art.
     */
    static void ConsoleLogo() {
    #ifdef _WIN32
        HANDLE hOut;

        hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY );
    #endif


        const char * meow[10]={

        "                                                           \n",
        "  .___  ___.  _______   ______   ____    __    ____       \n",
        "  |   \\/   | |   ____| /  __  \\  \\   \\  /  \\  /   /   \n",
        "  |  \\  /  | |  |__   |  |  |  |  \\   \\/    \\/   /   \n",
        "  |  |\\/|  | |   __|  |  |  |  |   \\            /      \n",
        "  |  |  |  | |  |____ |  `--'  |    \\    /\\    /    \n",
        "  |__|  |__| |_______| \\______/      \\__/  \\__/   \n",
        "                         \n",
        "                v0.7 night, by nekocode\n",
        "                   Nanocat engine\n"
    };

        int i;
        for( i = 0; i < 10; i++ ) {
            g_Core->p_Console->Print( LOG_NONE, meow[i] );
        }

        g_Core->p_Console->Print( LOG_NONE, "\n" );

        printf( "--------------------------------------------------------------------------------\n" );


    #ifdef _WIN32
        SetConsoleTextAttribute(hOut, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY );
    #endif

        g_Core->p_Console->Print( LOG_NONE, "Loading... Welcome, %s!\n", g_Core->p_System->GetCurrentUsername() );
        g_Core->p_Console->Print( LOG_NONE, "\n" );
    }

    //
    // Console.
    #define CONSOLE_CLASSNAME "SkyCat32Console"
    #define CONSOLE_NAME "Nanocat Console"

    ncWindowsConsole *wConsole;

    /**
     *  Console events.
     */
    static LONG WINAPI ConsoleProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
        switch( uMsg ) {
            case WM_CLOSE:
                // On case if close button clicked, quit.
                g_Core->Quit( "User quit." );
            return 0;

            case WM_CTLCOLORSTATIC:
                if( (HWND)lParam == wConsole->hwndBuffer ) {
                    // Kawaii colors.
                    SetBkColor( (HDC) wParam, RGB( 0x00, 0x00, 0x00 ) );
                    SetTextColor( (HDC) wParam, RGB( 0x75, 0xff, 0xd8 ) );
                    return (long)0;
                }
            break;

            case WM_ACTIVATE:
                if( LOWORD( wParam ) != WA_INACTIVE ) {
                    SetFocus( wConsole->hwndInputLine );
                }
            break;
        }

        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    /**
     *  Input line events.
     */
    static const int MAX_CONSOLE_INPUTLINE = 1024;
    static LONG WINAPI InputLine( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) {
        char cInputBuffer[MAX_CONSOLE_INPUTLINE];

        switch ( uMsg ) {
            case WM_KILLFOCUS:
                if ( (HWND) wParam == wConsole->hWnd ) {
                    SetFocus( hWnd );
                    return 0;
                }
                break;

            case WM_CHAR:
                // Enter.
                if ( wParam == 13 ) {
                    GetWindowText( wConsole->hwndInputLine, cInputBuffer, sizeof( cInputBuffer ) );

                    strncat( wConsole->consoleText, cInputBuffer, sizeof( wConsole->consoleText ) - strlen( wConsole->consoleText ) - 5 );
                    strcat( wConsole->consoleText, "\n" );

                    SetWindowText( wConsole->hwndInputLine, "" );

                    g_Core->p_Console->Execute( cInputBuffer );

                    return 0;
                }
        }

        return CallWindowProc( wConsole->SysInputLineWndProc, hWnd, uMsg, wParam, lParam );
    }

    /**
     *  Create console.
     */
    ncWindowsConsole::ncWindowsConsole( HINSTANCE hInstance ) {

        HDC hDC;
        WNDCLASS wc;
        RECT rect;

        int swidth, sheight;
        const int style = WS_POPUPWINDOW | WS_CAPTION | WS_MINIMIZEBOX;

        memset( &wc, 0, sizeof( wc ) );

        wc.style         = 0;
        wc.lpfnWndProc   = (WNDPROC)ConsoleProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = 0;
        wc.hInstance     = hInstance;
        wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
        wc.lpszMenuName  = 0;
        wc.lpszClassName = CONSOLE_CLASSNAME;

        if ( !RegisterClass( &wc ) )
            return;

        rect.left = 0;
        rect.right = 540;
        rect.top = 0;
        rect.bottom = 370;
        AdjustWindowRect( &rect, style, FALSE );

        hDC = GetDC( GetDesktopWindow() );
        swidth = GetDeviceCaps( hDC, HORZRES );
        sheight = GetDeviceCaps( hDC, VERTRES );
        ReleaseDC( GetDesktopWindow(), hDC );

        windowWidth = rect.right - rect.left + 1;
        windowHeight = rect.bottom - rect.top + 1;

        hWnd = CreateWindowEx( 0,
                                   CONSOLE_CLASSNAME,
                                   CONSOLE_NAME,
                                   style,
                                   ( swidth - 600 ) / 2, ( sheight - 410 ) / 2 , rect.right - rect.left + 1, rect.bottom - rect.top + 1,
                                   NULL,
                                   NULL,
                                   hInstance,
                                   NULL );

        if( hWnd == NULL ) {
            return;
        }

        hDC = GetDC( hWnd );

        hfBufferFont = CreateFont( 12,
                                          8,
                                          0,
                                          0,
                                          FW_LIGHT,
                                          0,
                                          0,
                                          0,
                                          DEFAULT_CHARSET,
                                          OUT_DEFAULT_PRECIS,
                                          CLIP_DEFAULT_PRECIS,
                                          DEFAULT_QUALITY,
                                          FF_MODERN | FIXED_PITCH,
                                          "Lucida Console" );

        ReleaseDC( hWnd, hDC );

        hwndInputLine = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER |
                                                    ES_LEFT | ES_AUTOHSCROLL,
                                                    6, 340, 528, 20,
                                                    hWnd,
                                                    ( HMENU ) CON_INPUT_ID,
                                                    hInstance, NULL );

        hwndBuffer = CreateWindow( "edit", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                                    ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
                                                    6, 10, 526, 324,
                                                    hWnd,
                                                    ( HMENU ) CON_EDIT_ID,
                                                    hInstance, NULL );

        SendMessage( hwndBuffer, WM_SETFONT, ( WPARAM ) hfBufferFont, 0 );
    }

    /**
     *  Show console.
     */
    void ncWindowsConsole::SetDone() {
		
        SysInputLineWndProc = (WNDPROC)SetWindowLong( hwndInputLine, GWLP_WNDPROC, (long)InputLine );
        SendMessage( hwndInputLine, WM_SETFONT, (WPARAM)hfBufferFont, 0 );

        ShowWindow( hWnd, SW_SHOWDEFAULT);
        UpdateWindow( hWnd );

        SetForegroundWindow( hWnd );
        SetFocus( hwndInputLine );
    }
}

using namespace Neko;

static void RendererThread( void * args ) {
    ApplicationHandle->CreateRendererThread();
}

/**
 *
 *  Main entry to point to our application.
 *  Uses main instead of WinMain.
 *
 */
int main( int argc, char ** argv ) {


    MSG     msg;

    // Remove Windows console.
   // FreeConsole();

    // Create new instance.
    wWindow = new ncMain32;

    // Create console.
    wWindow->CreateConsole();

    // Make console look nicer.
    system( "COLOR 0F" );
    SetConsoleTitle( "Neko engine" );
   // Neko::ConsoleLogo(); // Meow!

    // Create window and load game engine!
    wWindow->Initialize();

   // ncD3D11Base *s = new ncD3D11Base;
    //s->InitD3D( 640, 480, true, wWindow->GetHWND(), false, 0.1f, 100000.0f );

    // - -------------------------------------------------------------------------------

    ZeroMemory( &msg, sizeof(msg) );

    // Do not use 'GetMessage' in this loop. Makes large event delay.
    while( msg.message != WM_QUIT )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            // Networking, input, etc.
            // Rendering is made in another thread.
            g_Core->Frame();
        }

       // Sleep(1);
    }

    // Remove window and context after all.
    delete wWindow;

	return msg.wParam; // Exit The Program.
}

#endif




