// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the XMEM_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// XMEM_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef XMEM_EXPORTS
#define XMEM_API __declspec(dllexport)
#else
#define XMEM_API __declspec(dllimport)
#endif

// This class is exported from the dll
class XMEM_API Cxmem {
public:
	Cxmem(void);
	// TODO: add your methods here.
};

extern XMEM_API int nxmem;

XMEM_API int fnxmem(void);
