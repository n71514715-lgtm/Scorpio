// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the GAMEINPUT_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// GAMEINPUT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef GAMEINPUT_EXPORTS
#define GAMEINPUT_API __declspec(dllexport)
#else
#define GAMEINPUT_API __declspec(dllimport)
#endif

// This class is exported from the dll
class GAMEINPUT_API CGameInput {
public:
	CGameInput(void);
	// TODO: add your methods here.
};

extern GAMEINPUT_API int nGameInput;

GAMEINPUT_API int fnGameInput(void);
