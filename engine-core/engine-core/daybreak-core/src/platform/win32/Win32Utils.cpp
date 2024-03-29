#include "daybreak.h"
#include "Win32Utils.h"

bool DAYBREAK_API win32::util::AddBitmap(const wchar_t* szFileName, HDC hWinDC, int x, int y)
{
    BITMAP qBitmap;
    HDC hLocalDC = CreateCompatibleDC(hWinDC);

    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, szFileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == NULL) {
        MessageBox(NULL, L"LoadImage Failed", L"Error", MB_OK);
        return false;
    }
    GetObject(reinterpret_cast<HGDIOBJ>(hBitmap), sizeof(BITMAP), reinterpret_cast<LPVOID>(&qBitmap));

    SelectObject(hLocalDC, hBitmap);


    if (!BitBlt(hWinDC, x, y, qBitmap.bmWidth, qBitmap.bmHeight, hLocalDC, 0, 0, SRCCOPY)) {
        MessageBox(NULL, L"Blit Failed", L"Error", MB_OK);
        return false;
    }

    // Unitialize and deallocate resources
    ::DeleteDC(hLocalDC);
    ::DeleteObject(hBitmap);
    return true;
}
