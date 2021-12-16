// SprinkledLauncher.cpp : Defines the entry point for the application.
//

/*
    Sprinkled Launcher - a minimalist WIN32 API launcher for Doom Source Ports
    Copyright (C) 2021  Adam Bilbrough

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "framework.h"
#include "LarnLauncher.h"

// Max strings
#define MAX_LOADSTRING 1000

// A pragma for our shell library (ShellExecuteInfo)
#pragma comment(lib, "shell32.lib")

WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class nam

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SPRINKLEDLAUNCHER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SPRINKLEDLAUNCHER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPRINKLEDLAUNCHER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = CreateSolidBrush(RGB(240, 240, 240)); //RGB colours for the typical 'Classic Windows' look.
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SPRINKLEDLAUNCHER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 280, 280, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
// This is where we declare and define all our Windows for the control items we want to use
// These are linked to our hWnd window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateWindowW(TEXT("button"), TEXT("Launch Larn"),
            WS_VISIBLE | WS_CHILD,
            12, 180, 115, 30,
            hWnd, (HMENU)ID_LARN_BUTTON, NULL, NULL);
        CreateWindow(L"STATIC", help,
            WS_CHILD | WS_VISIBLE | SS_LEFT,
            0, 0, 700, 180,
            hWnd, 0, NULL, NULL);
        CommandLineArgs = CreateWindow(L"EDIT", 0,
            WS_CHILD | WS_VISIBLE | SS_LEFT | WS_BORDER,
            12, 120, 115, 30,
            hWnd, (HMENU)6, NULL, NULL);
        break;

    // This is where we set the actions for each control we have placed in a switch/case system
    case WM_COMMAND:
        {
            // We get our arguments from the Edit Control like this
            // A lot of people will try to use GetWindowText/GetWindowLength
            // but this is a more efficient way to do it.
            argTextLength = SendMessage(CommandLineArgs, WM_GETTEXTLENGTH, 0, 0);                   // Getting the length of the string we have typed in the edit control
            argText = new wchar_t[argTextLength];                                                   // The text we have typed plus the length into a wchar_t
            SendMessage(CommandLineArgs, WM_GETTEXT, (WPARAM)argTextLength + 1, (LPARAM)argText);   // The sending of the combined Edit control, the text and the expected length + the extra byte for the NULL termination
            int wmId = LOWORD(wParam);                                                              // This gets the 'clickling' so we don't have to declare it for each case

            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_LARN_BUTTON:
            {
                // This is where we execute the binary we want + all the arguments from our edit control via lpParameters which takes an LPCWSTR argument which is compatible with our LPTSTR
                SHELLEXECUTEINFO ExecuteLarn;

                memset(&ExecuteLarn, 0, sizeof(ExecuteLarn));

                ExecuteLarn.cbSize = sizeof(ExecuteLarn);
                ExecuteLarn.fMask = 0;
                ExecuteLarn.hwnd = 0;
                ExecuteLarn.lpVerb = _T("open");                    // Operation to perform
                ExecuteLarn.lpFile = _T("Larn.exe");                // Application name
                ExecuteLarn.lpParameters = argText;                 // Additional parameters
                ExecuteLarn.lpDirectory = 0;                        // Default directory
                ExecuteLarn.nShow = SW_SHOW;
                ExecuteLarn.hInstApp = 0;

                // If it fails, bail out!  We should probably put some dialog box here in the future...
                if (ShellExecuteEx(&ExecuteLarn) == FALSE)
                {
                    ExitProcess(1);
                }
                // Otherwise be happy
                ExitProcess(0);
            }
            break;
        }
    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
