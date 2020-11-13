#pragma comment(linker, "/merge:.rdata=.text")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "UxTheme.lib")

#define WINVER          0x0501
#define _WIN32_WINNT    0x0501
#define _WIN32_IE       0x0501

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <gdiplus.h>
#include <comip.h>
#include <xmllite.h>
#include <UxTheme.h>
#include <tlhelp32.h>
#include <psapi.h>

#include <vector>
#include <algorithm>

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                  0x020E
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED        0x031E
#endif
