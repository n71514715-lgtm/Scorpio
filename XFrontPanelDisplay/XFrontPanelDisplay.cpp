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
    __declspec(dllexport) HRESULT XFrontPanelPresentBuffer(
        void* pBuffer)
    {
        OutputDebugStringA("[Scorpio/XFrontPanel] XFrontPanelPresentBuffer stubbed\n");
        return S_OK;
    }

   
    
    __declspec(dllexport) HRESULT XFrontPanelGetLightStates(
        void* pStates)
    {
        OutputDebugStringA("[Scorpio/XFrontPanel] XFrontPanelGetLightStates stubbed\n");
        if (pStates) memset(pStates, 0, 64);
        return S_OK;
    }

    __declspec(dllexport) HRESULT XFrontPanelSetLightStates(
        void* pStates)
    {
        OutputDebugStringA("[Scorpio/XFrontPanel] XFrontPanelSetLightStates stubbed\n");
        return S_OK;
    }
    __declspec(dllexport) HRESULT XFrontPanelGetButtonStates(
        UINT32* pStates)
    {
        OutputDebugStringA("[Scorpio/XFrontPanel] XFrontPanelGetButtonStates stubbed\n");
        if (pStates) *pStates = 0;
        return S_OK;
    }
} // extern "C"