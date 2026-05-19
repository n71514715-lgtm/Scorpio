// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the SCORPIOHOOK_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// SCORPIOHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SCORPIOHOOK_EXPORTS
#define SCORPIOHOOK_API __declspec(dllexport)
#else
#define SCORPIOHOOK_API __declspec(dllimport)
#endif

// This class is exported from the dll
class SCORPIOHOOK_API CScorpioHook {
public:
	CScorpioHook(void);
	// TODO: add your methods here.
};

extern SCORPIOHOOK_API int nScorpioHook;

SCORPIOHOOK_API int fnScorpioHook(void);
