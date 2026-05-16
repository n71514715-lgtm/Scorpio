// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the XFRONTPANELDISPLAY_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// XFRONTPANELDISPLAY_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef XFRONTPANELDISPLAY_EXPORTS
#define XFRONTPANELDISPLAY_API __declspec(dllexport)
#else
#define XFRONTPANELDISPLAY_API __declspec(dllimport)
#endif

// This class is exported from the dll
class XFRONTPANELDISPLAY_API CXFrontPanelDisplay {
public:
	CXFrontPanelDisplay(void);
	// TODO: add your methods here.
};

extern XFRONTPANELDISPLAY_API int nXFrontPanelDisplay;

XFRONTPANELDISPLAY_API int fnXFrontPanelDisplay(void);
