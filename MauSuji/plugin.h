
#ifndef PLUGIN_H
#define PLUGIN_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif	//__cplusplus

#define PLUGIN_VERSION 4

typedef struct tagPREFS {
	HINSTANCE hInstance;
	int id;
	DLGPROC DlgProc;
	LPARAM lParam;
}PREFS;

typedef struct tagCOMMANDARGV {
	int nValue[4];
	WCHAR szText[4][MAX_PATH];
}COMMANDARGV;

typedef struct tagCONFIGPARAM {
	BOOL bInitialized;	//TRUE:èâä˙âªçœÇ›Å@FALSE:èâä˙âªÇ≥ÇÍÇƒÇ¢Ç»Ç¢
	COMMANDARGV argv;
}CONFIGPARAM;

typedef struct tagCOMMANDPARAM {
	HWND hwnd;
	WCHAR szPath[MAX_PATH];
	POINT ptStart;
	POINT ptEnd;
	COMMANDARGV argv;
}COMMANDPARAM;

typedef struct tagPLUGIN {
	int version;
	DWORD flags;
	WCHAR *name;
	HWND hwndParent;
	HINSTANCE hDllInstance;
	BOOL (*Init)(void);
	void (*Quit)(void);
	void (*Config)(PREFS* prefs, CONFIGPARAM* configparam);
	void (*Command)(COMMANDPARAM* param);
	void (*Description)(COMMANDARGV* argv, LPWSTR lpszText, int nLength);
}PLUGIN;

#ifdef __cplusplus
}
#endif	//__cplusplus

#endif	//PLUGIN_H
