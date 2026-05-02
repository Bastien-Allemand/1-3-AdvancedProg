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

void EncryptSingleCharacter(const wchar_t* imagePath, wchar_t character, HWND hWnd)
{
    if (character == L'\0')
    {
        MessageBox(hWnd, L"Please enter a valid character.", L"Error", MB_OK);
        return;
    }

    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(imagePath);
    if (!bitmap) return;

    if (bitmap->GetWidth() * bitmap->GetHeight() < 8)
    {
        MessageBox(hWnd, L"Image is too small to encrypt a character.", L"Error", MB_OK);
        delete bitmap;
        return;
    }

    for (int i = 0; i < 8; ++i)
    {
        Color color;
        bitmap->GetPixel(i, 0, &color);

        BYTE blue = color.GetBlue();
        bool bit = (character & (1 << i)) != 0;

        // Modify the LSB of the blue component
        if (bit)
            blue |= 1; // Set LSB to 1
        else
            blue &= ~1; // Set LSB to 0

        // Create new color with modified blue value
        Color newColor(color.GetRed(), color.GetGreen(), blue, color.GetAlpha());
        bitmap->SetPixel(i, 0, newColor);
    }

    CLSID clsid;
    GetEncoderClsid(L"image/png", &clsid);
    bitmap->Save(L"output.png", &clsid, NULL);
    delete bitmap;

    MessageBox(hWnd, L"Character encrypted into the image.", L"Success", MB_OK);
}

void DecryptSingleCharacter(const wchar_t* imagePath, HWND hWnd)
{// Create a new Bitmap object from the specified image path
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(imagePath);
    // Check if the bitmap was created successfully
    if (!bitmap) return;
    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();
    if (width * height < 8)
    {
        // Display an error message if the image is too small
        MessageBox(hWnd, L"Image is too small to decrypt a character.", L"Error", MB_OK);
        delete bitmap; // Clean up the allocated bitmap
        return;
    }

    wchar_t character = 0; // Variable to hold the decrypted character
    // Loop through the first 8 pixels in the first row of the image
    for (int i = 0; i < 8; ++i)
    {
        Color color; // Variable to hold the color of the pixel
        bitmap->GetPixel(i, 0, &color); // Get the color of the pixel at (i, 0)

        BYTE blue = color.GetBlue();

        if (blue & 1)
        {//update the character by setting the corresponding bit
            character |= (1 << i);
        }
    }// Clean up the allocated bitmap
    delete bitmap;
    std::wstring message = L"Decrypted Character: ";
    message += character;

    MessageBox(hWnd, message.c_str(), L"Decryption", MB_OK);
}

// This function retrieves the CLSID (Class ID) of an image encoder based on the given MIME type format.
// Parameters:
// - format: A pointer to a wide string (WCHAR) that specifies the MIME type of the desired image format.
// - pClsid: A pointer to a CLSID structure where the function will store the found CLSID.
// Returns:
// - The index of the encoder if found, or -1 if the encoder is not found or an error occurs.
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;   // Variable to hold the number of image encoders
    UINT size = 0;  // Variable to hold the size of the image codec info array
    ImageCodecInfo* pImageCodecInfo = NULL; // Pointer to hold the image codec information
    // Get the size of the image encoders array
    GetImageEncodersSize(&num, &size);
    // If size is 0, it means there are no image encoders available
    if (size == 0) return -1;
    // Allocate memory for the image codec info array based on the size retrieved
    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    // Check if memory allocation was successful
    if (pImageCodecInfo == NULL) return -1;
    // Retrieve the image encoders information into the allocated array
    GetImageEncoders(num, size, pImageCodecInfo);
    // Loop through the image codec info array to find a matching MIME type
    for (UINT j = 0; j < num; ++j)
    {// Compare the current codec's MIME type with the provided format
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {// If a match is found, store the CLSID in the provided pointer
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo); // Free the allocated memory
            return j; // Return the index of the found encoder
        }
    }// If no matching encoder is found, free the allocated memory and return -1
    free(pImageCodecInfo);
    return -1;
}