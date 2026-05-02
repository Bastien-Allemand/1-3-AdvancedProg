#include "WindowsProject2.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <iostream>

#include "Cryptage.h"
using namespace std;
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
ULONG_PTR gdiplusToken;
wchar_t selectedImagePath[MAX_PATH] = L"";
#define ID_LOAD_BUTTON 101
#define IDC_MESSAGE_EDIT 105
#define IDC_EDIT_BORDER 106
#define ID_ENCRYPT_BUTTON 110
#define ID_DECRYPT_BUTTON 111
#define ID_INITIALIZE_BUTTON 112
int currentWidth = 800;
int currentHeight = 600;

bool OpenImageFile(HWND hWnd, wchar_t* filePath, size_t filePathSize)
{
    OPENFILENAME ofn = { 0 };
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = L"Image Files\0*.BMP;*.JPG;*.JPEG;*.PNG;*.GIF\0All Files\0*.*\0";//different files alowed to be loaded
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = static_cast<DWORD>(filePathSize);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return GetOpenFileName(&ofn) == TRUE;
}

void SetBlueLSBToZero(const wchar_t* imagePath)
{
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(imagePath);
    if (!bitmap) return;

    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();

    for (int y = 0; y < height; ++y) 
    {
        for (int x = 0; x < width; ++x) 
        {
            Color color;
            bitmap->GetPixel(x, y, &color);
            BYTE blue = color.GetBlue();

            // Set the LSB of the blue channel to 0
            blue &= ~1; // Clear LSB of blue

            Color newColor(color.GetRed(), color.GetGreen(), blue, color.GetAlpha());
            bitmap->SetPixel(x, y, newColor);
        }
    }

    // Save the modified image
    CLSID clsid;
    GetEncoderClsid(L"image/png", &clsid);
    bitmap->Save(L"output.png", &clsid, NULL);
    delete bitmap;
}

void CopyImageToBmp(const wchar_t* sourcePath, const wchar_t* destPath) 
{
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(sourcePath);
    if (!bitmap) return;

    // Save the bitmap as BMP
    CLSID clsid;
    GetEncoderClsid(L"image/bmp", &clsid);
    bitmap->Save(destPath, &clsid, NULL);

    delete bitmap;
}

VOID OnPaint(HDC hdc)
{
    Graphics graphics(hdc);

    if (wcslen(selectedImagePath) > 0)
    {
        Image image(selectedImagePath);

        if (image.GetLastStatus() == Ok && image.GetWidth() > 0 && image.GetHeight() > 0)
        {
            int originalWidth = image.GetWidth();
            int originalHeight = image.GetHeight();

            double aspectRatio = static_cast<double>(originalWidth) / originalHeight;

            int newWidth, newHeight;

            if (currentWidth / static_cast<double>(currentHeight) > aspectRatio)
            {
                newHeight = currentHeight;
                newWidth = static_cast<int>(currentHeight * aspectRatio);
            }
            else
            {
                newWidth = currentWidth;
                newHeight = static_cast<int>(currentWidth / aspectRatio);
            }

            int x = (currentWidth - newWidth) / 2 + 110;
            int y = (currentHeight - newHeight) / 2;

            graphics.DrawImage(&image, x, y, newWidth - 110, newHeight - 110);
        }
        else
        {
            MessageBox(NULL, L"Failed to load image. Please check the file path.", L"Error", MB_OK);
        }
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Register window class
    WNDCLASS wndClass = { 0 };
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndClass.lpszClassName = L"ImageLoaderApp";
    RegisterClass(&wndClass);

    // Create window
    HWND hWnd = CreateWindow(
        L"ImageLoaderApp", L"Image Loader with Button",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Shutdown GDI+
    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC          hdc;
    PAINTSTRUCT  ps;
    static RECT rcBmp;

    switch (message)
    {
    case WM_CREATE:
        // Create buttons
        CreateWindow(L"BUTTON", L"Load Image", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,10, 10, 100, 30, hWnd, (HMENU)ID_LOAD_BUTTON, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

        CreateWindow(L"BUTTON", L"Encrypt MSG", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,10, 90, 100, 30, hWnd, (HMENU)ID_ENCRYPT_BUTTON, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

        CreateWindow(L"BUTTON", L"Decrypt MSG", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,10, 130, 100, 30, hWnd, (HMENU)ID_DECRYPT_BUTTON, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

        CreateWindow(L"BUTTON", L"Initialize", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,10, 50, 100, 30, hWnd, (HMENU)ID_INITIALIZE_BUTTON, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

        CreateWindow(L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_GRAYFRAME, 5, 165, 100, 30, hWnd, (HMENU)IDC_EDIT_BORDER, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

        CreateWindow(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 170, 90, 20, hWnd, (HMENU)IDC_MESSAGE_EDIT, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
        break;
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 500;
        mmi->ptMinTrackSize.y = 400;
    }
    break;
    case WM_SIZE:
    {
        currentWidth = LOWORD(lParam);
        currentHeight = HIWORD(lParam);
        InvalidateRect(hWnd, NULL, TRUE); // Request window redraw
    }
    break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_LOAD_BUTTON) {
            if (OpenImageFile(hWnd, selectedImagePath, MAX_PATH)) {
                InvalidateRect(hWnd, NULL, TRUE); // Request window redraw
            }
        }
        else if (LOWORD(wParam) == ID_ENCRYPT_BUTTON)
        {
            if (wcslen(selectedImagePath) == 0)
            {
                MessageBox(hWnd, L"Please load an image first.", L"Error", MB_OK);
                break;
            }
            wchar_t messageBuffer[256];
            GetWindowTextW(GetDlgItem(hWnd, IDC_MESSAGE_EDIT), messageBuffer, 256);
            EncryptSingleCharacter(selectedImagePath, *messageBuffer, hWnd);
        }
        else if (LOWORD(wParam) == ID_DECRYPT_BUTTON) 
        {
            if (wcslen(selectedImagePath) == 0)
            {
                MessageBox(hWnd, L"Please load an image first.", L"Error", MB_OK);
                break;
            }

            DecryptSingleCharacter(selectedImagePath, hWnd);
        }
        else if (LOWORD(wParam) == ID_INITIALIZE_BUTTON)
        {
            SetBlueLSBToZero(selectedImagePath); // Set LSB of blue channel to 0 for all pixels

            MessageBox(hWnd, L"All blue LSBs set to 0.", L"Initialization", MB_OK);
        }
        break;

    case WM_PAINT:

        hdc = BeginPaint(hWnd, &ps);

        OnPaint(hdc);

        EndPaint(hWnd, &ps);

        break;

    case WM_DESTROY:

        PostQuitMessage(0);

        break;

    default:

        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}