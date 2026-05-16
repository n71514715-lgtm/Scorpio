#include "pch.h"
#include <Windows.h>

// Scorpio - Xbox One Translation Layer
// XFrontPanelDisplay.dll - Xbox Front Panel Stub
// The Xbox One has a small LED display on the front
// On PC this is completely irrelevant - pure stub

extern "C" {

    __declspec(dllexport) void XFrontPanelGetScreenDimensions(
        unsigned int* pWidth,
        unsigned int* pHeight)
    {
        // No front panel on PC - return zero dimensions
        if (pWidth) *pWidth = 0;
        if (pHeight) *pHeight = 0;
    }

    __declspec(dllexport) int XFrontPanelIsSupported()
    {
        // Tell the game there's no front panel
        return 0;
    }

    __declspec(dllexport) void XFrontPanelSetScreenPixels(
        void* pBuffer)
    {
        // Nothing to do on PC
    }

} // extern "C"