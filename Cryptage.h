#pragma once
#include "WindowsProject2.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <iostream>
#include "Cryptage.h"
void EncryptSingleCharacter(const wchar_t* imagePath, wchar_t character, HWND hWnd);

void DecryptSingleCharacter(const wchar_t* imagePath, HWND hWnd);

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);