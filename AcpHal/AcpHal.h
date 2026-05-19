// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the ACPHAL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// ACPHAL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef ACPHAL_EXPORTS
#define ACPHAL_API __declspec(dllexport)
#else
#define ACPHAL_API __declspec(dllimport)
#endif

// This class is exported from the dll
class ACPHAL_API CAcpHal {
public:
	CAcpHal(void);
	// TODO: add your methods here.
};

extern ACPHAL_API int nAcpHal;

ACPHAL_API int fnAcpHal(void);
