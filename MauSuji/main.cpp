
#include "stdafx.h"

#include "../MauHook/MauHook.h"
#include "main.h"
#include "plugin.h"
#include "resource.h"

typedef std::basic_string<TCHAR> tstring;

typedef struct tagTARGETWINDOW {
	int Number;
	int DefaultThru;
	BOOL HookType;
	BOOL WheelRedirect;
	BOOL FreeScroll;
	TCHAR FileName[_MAX_PATH + 1];
	TCHAR ClassName[MAX_CLASSNAME + 1];
	TCHAR ControlID[20];
	TCHAR Comment[MAX_COMMENT + 1];
	TCHAR WindowTitle[256];
}TARGETWINDOW;

typedef struct tagCOMMANDDATA {
	int Case;
	int Wait;
	LONG Key;
	int CommandTarget;
}COMMANDDATA;

typedef struct tagEXECUTECOMMAND {
	int TargetNumber;
	int Button;
	BYTE Modifier;
	int Move[MAX_GESTURE_LEVEL];
	int Repeat;
	COMMANDDATA command[MAX_ACTION_REPEAT];
	TCHAR Comment[MAX_COMMENT + 1];
	int BreakPoint;
}EXECUTECOMMAND;

typedef struct tagMOUSECAPTURE {
	int Level;
	int Button;
	BYTE Modifier;
	int Move[MAX_GESTURE_LEVEL];
	BOOL Start;
}MOUSECAPTURE;

typedef struct tagCOMMAND {
	int Locate;
	int Repeat;
	COMMANDDATA command[MAX_ACTION_REPEAT];
}COMMAND;

typedef struct tagCOMMANDEXTINFO {
	TCHAR path[MAX_PATH];	//プラグインのパス
	int Key[4];
	TCHAR Text[4][_MAX_PATH + 1];
}COMMANDEXTINFO;

typedef struct tagCASE {
	int Number;
	TCHAR Name[30];
}CASE;

typedef struct tagSEARCHCHILDINFO {
	int ClassFlag;
	HWND hwTarget;
	TCHAR ClassName[MAX_CLASSNAME + 1];
	int ControlID;
	TCHAR WindowTitle[256];
}SEARCHCHILDINFO;

typedef struct tagZORDER {
	int Number;
	TCHAR Name[30];
}ZORDER;

typedef struct tagSHADEWINDOW {
	HWND hwnd;
	BOOL bMaximize;
	RECT rect;
}SHADEWINDOW;

typedef struct tagCOMMANDITEM{
	int Case;
	int Wait;
	LONG Key;
	int CommandTarget;
	COMMANDEXTINFO CommandExt;
}COMMANDITEM;
typedef struct tagACTIONITEM{
	int Button;
	BYTE Modifier;
	int Move[MAX_GESTURE_LEVEL];
	TCHAR Comment[MAX_COMMENT + 1];
	std::vector<COMMANDITEM> command;
}ACTIONITEM;
typedef struct tagTARGETITEM{
	int Number;
	int DefaultThru;
	BOOL HookType;
	BOOL WheelRedirect;
	BOOL FreeScroll;
	TCHAR FileName[_MAX_PATH + 1];
	TCHAR ClassName[MAX_CLASSNAME + 1];
	TCHAR ControlID[20];
	TCHAR Comment[MAX_COMMENT + 1];
	TCHAR WindowTitle[256];
	std::vector<ACTIONITEM> action;
}TARGETITEM;
typedef struct tagCONFIG{
	//Main
	BOOL ShowTaskTray;
	int GestureDirection;
	BOOL TimeOutType;
	BOOL DefaultHookType;
	int Priority;
	int ClickWait;
	BOOL WheelRedirect;
	BOOL WheelActive;
	BOOL KuruKuruFlag;
	BOOL DefaultFreeScroll;
	int ScrollSensitivity;
	int KuruKuruTimeOut;
	int CircleRate;
	BOOL DefaultWheelRedirect;
	int CheckInterval;
	int MoveQuantity;
	int GestureTimeOut;
	int GestureStartQuantity;
	int CornerTime;
	int CornerPosX;
	int CornerPosY;
	BOOL CursorChange;
	TCHAR GestureCursor[_MAX_PATH + 1];
	TCHAR WaitCursor[_MAX_PATH + 1];
	//Navi
	int NaviType;
	POINT MauSujiNavi;
	int NaviDeleteTime;
	int NaviWidth;
	POINT TipLocate;
	TCHAR NaviTitle[_MAX_PATH + 1];
	LOGFONT NaviFont;
	COLORREF MainColor;
	COLORREF SubColor;
	COLORREF BackColor;
	COLORREF FrameColor;
	//Character
	TCHAR Character[12][5];
	//MouseGestureTrail
	BOOL MouseGestureTrail;
	DWORD MouseGestureTrailWidth;
	COLORREF MouseGestureTrailColor;
	DWORD MouseGestureTrailDrawMode;
	//Target
	std::vector<TARGETITEM> target;
}CONFIG;

typedef struct tagPLUGINDATA{
	HMODULE hModule;
	TCHAR szPath[MAX_PATH];
	PLUGIN *plugin;
}PLUGINDATA;

enum NaviType { NaviTypeNone = 0, NaviTypeFixed, NaviTypeFloat };

HINSTANCE hInstance;
ULONG_PTR gdiplusToken;
HWND hwndNavi;
HFONT hFont = NULL;
HWND hDlgOption;	//設定ダイアログ

TCHAR szConfigFilePath[MAX_PATH] = TEXT("");	//設定ファイルの絶対パス
BOOL LeftyMode = FALSE;

int NaviType;
BOOL ShowTaskTray;
int GestureDirection;
BOOL TimeOutType;
BOOL WheelRedirect;
BOOL WheelActive;
BOOL KuruKuruFlag;
int Priority;
int ClickWait;
int CheckInterval;
int MoveQuantity;
int GestureTimeOut;
int GestureStartQuantity;
int CornerTime;
int CornerPosX;
int CornerPosY;
BOOL CursorChange;
int NaviWidth;
int NaviDeleteTime;
int ScrollSensitivity;
int KuruKuruTimeOut;
int CircleRate;
LOGFONT lf;
COLORREF MainColor, SubColor, BackColor, FrameColor;
TCHAR NaviTitle[_MAX_PATH + 1];

TCHAR MoveUp[3];
TCHAR MoveDown[3];
TCHAR MoveLeft[3];
TCHAR MoveRight[3];
TCHAR MoveUpLeft[3];
TCHAR MoveUpRight[3];
TCHAR MoveDownLeft[3];
TCHAR MoveDownRight[3];
TCHAR CornerTopLeft[5];
TCHAR CornerTopRight[5];
TCHAR CornerBottomLeft[5];
TCHAR CornerBottomRight[5];

BOOL MouseHookFlag = TRUE;
BOOL IniChange = FALSE;
BOOL ImplementFlag = FALSE;
BOOL HookCheckFlag = TRUE;
BOOL KuruKuruTimeoutFlag = TRUE;

BOOL PopupFlag = FALSE, TimeOutFlag = FALSE, GestureTimeOutTimer = FALSE, NaviDrawFlag = FALSE, ClickOnlyFlag = FALSE;
int DoubleClickFlag = 0;
POINT StartPos, FinishPos, RealPos;
POINT DefPos, LastPos;
int FinishActionNumber = -1;

TCHAR GestureCursor[_MAX_PATH + 1];
TCHAR WaitCursor[_MAX_PATH + 1];

TCHAR GestureString[50];
TCHAR CommentString[MAX_COMMENT + 1];

POINT MauSujiNavi, TipLocate;

BOOL MouseGestureTrail;
DWORD MouseGestureTrailWidth;
COLORREF MouseGestureTrailColor;
DWORD MouseGestureTrailDrawMode;
HWND hwndLayeredWindow = NULL;
HDC hdcMouseGestureTrail = NULL;
HBITMAP hBitmapOldGestureTrail = NULL;

MOUSECAPTURE Gesture;

CASE CommandList[MAX_ACTION_CASE] = {
	{10,TEXT("キーを送る")}, 
	{11,TEXT("キーを押し続ける")}, 
	{12,TEXT("キーを離す")}, 
	{20,TEXT("ボタンを押す")},
	{25,TEXT("クリップボードにコピー")},
	{30,TEXT("プログラム実行")},
	{35,TEXT("waveファイル再生")},
	{40,TEXT("ウィンドウ表示方法変更")},
	{41,TEXT("ウィンドウサイズ変更")},
	{42,TEXT("ウィンドウ位置変更")},
	{55,TEXT("マウスカーソル位置変更")},
	{58,TEXT("開始位置からカーソル移動")},
	{59,TEXT("終了位置からカーソル移動")},
	{50,TEXT("ウィンドウ半透明化")},
	{60,TEXT("スクロール")},
	{65,TEXT("その場でスクロール")},
	{70,TEXT("SendMessage")},
	{71,TEXT("PostMessage")},
	{99,TEXT("設定")},
	{95,TEXT("ブレークポイント")},
	{96,TEXT("アクション終了後に実行")},
//	{100,TEXT("")},	//Reserved
//	{101,TEXT("")},	//Reserved
//	{102,TEXT("")},	//Reserved
//	{103,TEXT("")},	//Reserved
};

ZORDER ZorderList[MAX_ZORDER] = {
	{0,TEXT("手前に表示")},
	{1,TEXT("奥に表示")},
	{2,TEXT("常に手前に表示")},
	{3,TEXT("常に手前に表示を解除")},
	{4,TEXT("常に手前に表示（トグル）")},
	{5,TEXT("元のサイズに戻す")},
	{6,TEXT("最小化")},
	{7,TEXT("最大化")},
	{8,TEXT("閉じる")},
	{9,TEXT("最小化⇔元のサイズ")},
	{10,TEXT("最大化⇔元のサイズ")},
	{11,TEXT("最小化⇔最大化")},
	{12,TEXT("ウィンドウシェード")},
//	{13,TEXT("")},	//Reserved
//	{14,TEXT("")},	//Reserved
//	{15,TEXT("")},	//Reserved
//	{16,TEXT("")},	//Reserved
//	{17,TEXT("")},	//Reserved
//	{18,TEXT("")},	//Reserved
//	{19,TEXT("")},	//Reserved
//	{20,TEXT("")},	//Reserved
//	{21,TEXT("")},	//Reserved
//	{22,TEXT("")},	//Reserved
//	{23,TEXT("")},	//Reserved
//	{24,TEXT("")},	//Reserved
//	{25,TEXT("")},	//Reserved
//	{26,TEXT("")},	//Reserved
//	{27,TEXT("")},	//Reserved
//	{28,TEXT("")},	//Reserved
//	{29,TEXT("")},	//Reserved
};

TARGETWINDOW Default;
std::vector<TARGETWINDOW> Target;
std::vector<EXECUTECOMMAND> Action;
std::vector<COMMANDEXTINFO> CommandExt;
std::vector<POINT> GestureTrail;
std::vector<SHADEWINDOW> shadewindowlist;
std::vector<PLUGINDATA> plugins;

WNDPROC WndProcFinderOld;
HHOOK hHookKeyboard = NULL;

HHOOK hHookMouse = NULL;
TCHAR TargetPath[_MAX_PATH + 1] = TEXT("");
TCHAR TargetClass[MAX_CLASSNAME + 1] = TEXT("");
TCHAR TargetID[MAX_CLASSNAME + 1] = TEXT("");
TCHAR TargetTitle[256] = TEXT("");
int WindowHook = 0;
HWND CallDlg1 = NULL;
HWND CallDlg2 = NULL;

//
int _ListView_HitTest(HWND hwnd, LPLVHITTESTINFO pinfo)
{
	int ret = -1;
	DWORD dwProcessId;
	HANDLE hProcess;
	LPLVHITTESTINFO _pinfo;
	SIZE_T write;

	GetWindowThreadProcessId(hwnd, &dwProcessId);
	hProcess = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, dwProcessId);
	if(hProcess)
	{
		_pinfo = (LPLVHITTESTINFO)VirtualAllocEx(hProcess, NULL, sizeof(LVHITTESTINFO), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if(_pinfo)
		{
			WriteProcessMemory(hProcess, _pinfo, pinfo, sizeof(LVHITTESTINFO), &write);
			ret = ListView_HitTest(hwnd, _pinfo);
			ReadProcessMemory(hProcess, _pinfo, pinfo, sizeof(LVHITTESTINFO), &write);
			VirtualFreeEx(hProcess, _pinfo, 0, MEM_RELEASE);
		}
		CloseHandle(hProcess);
	}
	return ret;
}
//
HTREEITEM _TreeView_HitTest(HWND hwndTV, LPTVHITTESTINFO lpht)
{
	HTREEITEM ret = NULL;
	DWORD dwProcessId;
	HANDLE hProcess;
	LPTVHITTESTINFO _lpht;
	SIZE_T write;

	GetWindowThreadProcessId(hwndTV, &dwProcessId);
	hProcess = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE, FALSE, dwProcessId);
	if(hProcess)
	{
		_lpht = (LPTVHITTESTINFO)VirtualAllocEx(hProcess, NULL, sizeof(TVHITTESTINFO), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
		if(_lpht)
		{
			WriteProcessMemory(hProcess, _lpht, lpht, sizeof(TVHITTESTINFO), &write);
			ret = TreeView_HitTest(hwndTV, _lpht);
			ReadProcessMemory(hProcess, _lpht, lpht, sizeof(TVHITTESTINFO), &write);
			VirtualFreeEx(hProcess, _lpht, 0, MEM_RELEASE);
		}
		CloseHandle(hProcess);
	}
	return ret;
}

int GetTargetHitTestCode()
{
	POINT pt;
	HWND hwnd;

	GetCursorPos(&pt);
	hwnd = WindowFromPoint(pt);

	return (int)SendMessage(hwnd, WM_NCHITTEST, 0, MAKELPARAM(pt.x, pt.y));
}

BOOL GetFileNameFromHwnd(HWND hWnd, LPTSTR lpszFileName, DWORD nSize)
{
	BOOL bResult = FALSE;
	DWORD dwProcessId;
	HANDLE hProcess;

	GetWindowThreadProcessId(hWnd, &dwProcessId);

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);
	if(hProcess)
	{
		HMODULE hModule;
		DWORD dwNeed;

		if(EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwNeed))
		{
			if(GetModuleFileNameEx(hProcess, hModule, lpszFileName, nSize))
				bResult = TRUE;
		}

		if(bResult==FALSE)
		{
			//Vista or later
			typedef BOOL (WINAPI *LPQUERYFULLPROCESSIMAGENAMEA)(HANDLE hProcess, DWORD dwFlags, LPSTR lpExeName, PDWORD lpdwSize);
			typedef BOOL (WINAPI *LPQUERYFULLPROCESSIMAGENAMEW)(HANDLE hProcess, DWORD dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);
			LPQUERYFULLPROCESSIMAGENAMEA QueryFullProcessImageNameA = (LPQUERYFULLPROCESSIMAGENAMEA)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "QueryFullProcessImageNameA");
			LPQUERYFULLPROCESSIMAGENAMEW QueryFullProcessImageNameW = (LPQUERYFULLPROCESSIMAGENAMEW)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "QueryFullProcessImageNameW");
#ifdef UNICODE
#define QueryFullProcessImageName QueryFullProcessImageNameW
#else
#define QueryFullProcessImageName QueryFullProcessImageNameA
#endif
			DWORD dwSize = nSize;
			if(QueryFullProcessImageName)
				bResult = QueryFullProcessImageName(hProcess, 0, lpszFileName, &dwSize);
		}

		CloseHandle(hProcess);
	}

	if(bResult==FALSE)
	{
		HANDLE hProcessSnap;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if(hProcessSnap!=INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 pe32;

			pe32.dwSize = sizeof(PROCESSENTRY32);
			if(Process32First(hProcessSnap, &pe32))
			{
				do{
					if(dwProcessId==pe32.th32ProcessID)
					{
						lstrcpyn(lpszFileName, pe32.szExeFile, nSize);
						bResult = TRUE;
						break;
					}
				}while(Process32Next(hProcessSnap, &pe32));
			}
			CloseHandle(hProcessSnap);
		}
	}
	return bResult;
}

BOOL FindPlugin(LPTSTR lpszPath)
{
	int i;

	for(i=0;i<(int)plugins.size();i++)
	{
		if(!lstrcmpi(plugins[i].szPath, lpszPath))
			return i;
	}
	return -1;
}

BOOL ExecutePlugin(HWND hwnd, POINT ptStart, POINT ptEnd, COMMANDEXTINFO* CommandExt)
{
	int i;
	COMMANDPARAM param;

	i = FindPlugin(CommandExt->path);
	if(i==-1)
		return FALSE;

	param.hwnd = hwnd;
	GetFileNameFromHwnd(hwnd, param.szPath, MAX_PATH);
	param.ptStart = ptStart;
	param.ptEnd = ptEnd;
	param.argv.nValue[0] = CommandExt->Key[0];
	param.argv.nValue[1] = CommandExt->Key[1];
	param.argv.nValue[2] = CommandExt->Key[2];
	param.argv.nValue[3] = CommandExt->Key[3];
	lstrcpy(param.argv.szText[0], CommandExt->Text[0]);
	lstrcpy(param.argv.szText[1], CommandExt->Text[1]);
	lstrcpy(param.argv.szText[2], CommandExt->Text[2]);
	lstrcpy(param.argv.szText[3], CommandExt->Text[3]);
	plugins[i].plugin->Command(&param);	//command

	return TRUE;
}

void LoadPlugin(HWND hwnd, LPTSTR lpszPath)
{
	typedef PLUGIN* (WINAPI *GETPLUGIN)();
	HMODULE hModule;

	hModule = LoadLibrary(lpszPath);
	if(hModule)
	{
		BOOL bLoaded = FALSE;
		GETPLUGIN GetPlugin;

		GetPlugin = (GETPLUGIN)GetProcAddress(hModule, "GetPlugin");
		if(GetPlugin)
		{
			PLUGIN* plugin;

			plugin = GetPlugin();
			if(plugin)
			{
				if(plugin->version>=4)
				{
					plugin->hwndParent = hwnd;
					plugin->hDllInstance = hModule;
					if(plugin->Init())	//init
					{
						TCHAR szExePath[MAX_PATH];
						PLUGINDATA plugindata;

						GetModuleFileName(NULL, szExePath, MAX_PATH);
						plugindata.hModule = hModule;
						PathRelativePathTo(plugindata.szPath, szExePath, FILE_ATTRIBUTE_ARCHIVE, lpszPath, FILE_ATTRIBUTE_ARCHIVE);
						plugindata.plugin = plugin;
						plugins.push_back(plugindata);
						bLoaded = TRUE;
					}
				}
			}
		}
		if(!bLoaded)
			FreeLibrary(hModule);
	}
}

void LoadPlugins(HWND hwnd)
{
	TCHAR szExeDir[MAX_PATH];
	TCHAR szPluginDir[MAX_PATH];
	TCHAR szFileName[MAX_PATH];
	HANDLE hFindFile;
	WIN32_FIND_DATA FindFileData;

	GetModuleFileName(NULL, szExeDir, MAX_PATH);
	PathRemoveFileSpec(szExeDir);
	wsprintf(szPluginDir, TEXT("%s\\plugins"), szExeDir);
	CreateDirectory(szPluginDir, NULL);	//プラグインディレクトリがなければ作成する
	wsprintf(szFileName, TEXT("%s\\*.dll"), szPluginDir);

	hFindFile = FindFirstFile(szFileName, &FindFileData);
	if(hFindFile==INVALID_HANDLE_VALUE)
		return;
	do{
		if(!(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN))
		{
			TCHAR szPath[MAX_PATH];

			wsprintf(szPath, TEXT("%s\\%s"), szPluginDir, FindFileData.cFileName);
			LoadPlugin(hwnd, szPath);
		}
	}while(FindNextFile(hFindFile, &FindFileData));
	FindClose(hFindFile);
}

void UnloadPlugins()
{
	int i;

	for(i=0;i<(int)plugins.size();i++)
	{
		plugins[i].plugin->Quit();	//quit
		FreeLibrary(plugins[i].hModule);
	}
}
//文字列の置換
//srcのstr1をstr2に置換する
int StrReplace(LPCTSTR src, LPCTSTR str1, LPCTSTR str2, LPTSTR dest, int size)
{
	int length = 0;
	LPCTSTR psrc = src;
	LPTSTR pdest = dest;

	while(*psrc!=0)
	{
		if(!memcmp(psrc, str1, lstrlen(str1)*sizeof(TCHAR)))
		{
			if(dest)
			{
				if(pdest+lstrlen(str2)<dest+size)
					memcpy(pdest, str2, lstrlen(str2)*sizeof(TCHAR));
				else
					return 0;
			}
			psrc += lstrlen(str1);
			pdest += lstrlen(str2);
			length += lstrlen(str2);
		}
		else
		{
			if(dest)
			{
				if(pdest+1<dest+size)
					*pdest = *psrc;
				else
					return 0;
			}
			psrc++;
			pdest++;
			length++;
		}
	}
	return length;
}
//文字列を置換（戻り値はHeapFreeする必要がある）
LPTSTR StrReplace(HWND hwnd, LPCTSTR src)
{
	LPTSTR dest = NULL;
	TCHAR szPath[MAX_PATH];
	TCHAR szDir[MAX_PATH];
	int length;
	LPTSTR dest1, dest2, dest3;

	GetFileNameFromHwnd(hwnd, szPath, MAX_PATH);
	lstrcpy(szDir, szPath);
	PathRemoveFileSpec(szDir);

	//%1をフォルダのパスに置換
	length = StrReplace(src, TEXT("%1"), szDir, NULL, 0);
	dest1 = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (length+1)*sizeof(TCHAR));
	StrReplace(src, TEXT("%1"), szDir, dest1, length+1);

	//$pathをフルパスに置換
	length = StrReplace(dest1, TEXT("$path"), szPath, NULL, 0);
	dest2 = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (length+1)*sizeof(TCHAR));
	StrReplace(dest1, TEXT("$path"), szPath, dest2, length+1);

	//$dirをフォルダのパスに置換
	length = StrReplace(dest2, TEXT("$dir"), szDir, NULL, 0);
	dest3 = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (length+1)*sizeof(TCHAR));
	StrReplace(dest2, TEXT("$dir"), szDir, dest3, length+1);

	HeapFree(GetProcessHeap(), 0, dest1);
	HeapFree(GetProcessHeap(), 0, dest2);
	dest = dest3;

	return dest;
}

BOOL bIsWindowsVersionOK(DWORD dwMajor, DWORD dwMinor, WORD dwSPMajor)
{
	DWORDLONG dwlConditionMask = 0;
	OSVERSIONINFOEX osvi;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = dwMajor;
	osvi.dwMinorVersion = dwMinor;
	osvi.wServicePackMajor = dwSPMajor;// Set up the condition mask.

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	// Perform the test.
	return VerifyVersionInfo(&osvi,
		VER_MAJORVERSION| VER_MINORVERSION | VER_SERVICEPACKMAJOR,
		dwlConditionMask);
}

BOOL IsWin7OrLater()
{
	return bIsWindowsVersionOK(6, 1, 0);
}

void _SetForegroundWindow(HWND hwnd)
{
	int nTargetID, nForegroundID;
	DWORD sp_time;
	nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	nTargetID = GetWindowThreadProcessId(hwnd, NULL);
	AttachThreadInput(nTargetID, nForegroundID, TRUE);
	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &sp_time, 0);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0);
	SetForegroundWindow(hwnd);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (PVOID)sp_time, 0);
	AttachThreadInput(nTargetID, nForegroundID, FALSE);
}

BOOL GetWindowPosition(HWND hwnd, LPPOINT lpPoint)
{
	RECT rc;

	if(GetWindowRect(hwnd, &rc))
	{
		lpPoint->x = rc.left;
		lpPoint->y = rc.top;
		return TRUE;
	}
	return FALSE;
}

BOOL WritePrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, const int value, LPCTSTR lpFileName)
{
	TCHAR String[256];
	wsprintf(String, TEXT("%d"), value);
	return WritePrivateProfileString(lpAppName, lpKeyName, String, lpFileName);
}

HRESULT _DwmIsCompositionEnabled( BOOL *pfEnabled )
{
	typedef HRESULT (WINAPI *PFUNC)(BOOL*);
	HRESULT hr = MAKE_HRESULT(1,0,0);	//エラーはこれであっているのか？
	HMODULE hModule;
	hModule = LoadLibrary(TEXT("dwmapi.dll"));
	if(hModule)
	{
		PFUNC pDwmIsCompositionEnabled;
		pDwmIsCompositionEnabled = (PFUNC)GetProcAddress(hModule, "DwmIsCompositionEnabled");
		if(pDwmIsCompositionEnabled)
			hr =  pDwmIsCompositionEnabled(pfEnabled);
		FreeLibrary(hModule);
	}
	return hr;
}

BOOL IsAero()
{
	BOOL fEnable = FALSE;
	_DwmIsCompositionEnabled(&fEnable);
	return fEnable;
}
//軌跡
void DrawTrail()
{
	if(hwndLayeredWindow)
	{
		int i;

		Gdiplus::Graphics graphics(hdcMouseGestureTrail);
		graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		graphics.Clear(Gdiplus::Color(0, 0, 0, 0));

		Gdiplus::Color color;
		color.SetFromCOLORREF(MouseGestureTrailColor);
		Gdiplus::Pen pen(color, (Gdiplus::REAL)MouseGestureTrailWidth);
		pen.SetStartCap(Gdiplus::LineCapRound);
		pen.SetEndCap(Gdiplus::LineCapRound);

		RECT rc;
		GetWindowRect(hwndLayeredWindow, &rc);

		for(i=1;i<(int)GestureTrail.size();i++)
		{
			graphics.DrawLine(&pen,
				(int)(GestureTrail[i-1].x - rc.left),
				(int)(GestureTrail[i-1].y - rc.top),
				(int)(GestureTrail[i].x - rc.left),
				(int)(GestureTrail[i].y - rc.top)
				);
		}

		HDC hdcDst = GetDC(NULL);
		BITMAP bm;
		GetObject(GetCurrentObject(hdcMouseGestureTrail, OBJ_BITMAP), sizeof(BITMAP), &bm);
		SIZE size = {bm.bmWidth, bm.bmHeight};
		POINT ptSrc = {0, 0};
		BLENDFUNCTION blend = {AC_SRC_OVER, 0, 0xff, AC_SRC_ALPHA};
		UpdateLayeredWindow(
			hwndLayeredWindow, hdcDst, NULL, &size, hdcMouseGestureTrail, &ptSrc, 0, &blend, ULW_ALPHA);
		ReleaseDC(NULL, hdcDst);
	}
	else
	{
		HDC hdc;
		HPEN hPen, hPenOld;
		int i;

		hdc = GetDC(NULL);
		hPen = CreatePen(PS_SOLID, MouseGestureTrailWidth, MouseGestureTrailColor);
		hPenOld = (HPEN)SelectObject(hdc, hPen);

		MoveToEx(hdc, GestureTrail[0].x, GestureTrail[0].y, NULL);
		for(i=1;i<(int)GestureTrail.size();i++)
		{
			LineTo(hdc, GestureTrail[i].x, GestureTrail[i].y);
			MoveToEx(hdc, GestureTrail[i].x, GestureTrail[i].y, NULL);
		}

		SelectObject(hdc, hPenOld);
		DeleteObject(hPen);
		ReleaseDC(NULL, hdc);
	}
}

void SetMouseGestureTrailMode(DWORD DrawMode)
{
	if(DrawMode==1)
	{
		//レイヤードウィンドウに描画
		if(hwndLayeredWindow==NULL)
		{
			HDC hdc;
			//dc作成
			hdc = GetDC(NULL);
			hdcMouseGestureTrail = CreateCompatibleDC(hdc);
			ReleaseDC(NULL, hdc);
			//window作成
			hwndLayeredWindow = CreateWindowEx(
				WS_EX_TOOLWINDOW|WS_EX_LAYERED|WS_EX_NOACTIVATE|WS_EX_TRANSPARENT,
				TEXT("MauSujiLayeredWindow"),
				TEXT(""),
				WS_POPUP,
				0, 0, 0, 0,
				NULL, NULL, NULL, NULL);
		}
	}
	else
	{
		//画面に描画
		if(hwndLayeredWindow)
		{
			//window破棄
			DestroyWindow(hwndLayeredWindow);
			hwndLayeredWindow = NULL;
			//dc破棄
			DeleteDC(hdcMouseGestureTrail);
			hdcMouseGestureTrail = NULL;
		}
	}
}

void SetMouseGestureTrailModeAuto()
{
	if(IsAero())
		SetMouseGestureTrailMode(1);
	else
		SetMouseGestureTrailMode(0);
}

void MouseGestureTrailAddPos(POINT pt)
{
	GestureTrail.push_back(pt);
}

void MouseGestureTrailStart()
{
	GestureTrail.clear();

	if(hwndLayeredWindow)
	{
		//ビットマップ作成
		HMONITOR hMonitor;
		RECT rect;
		BITMAPINFO bmi;
		HBITMAP hBitmap;

		hMonitor = MonitorFromPoint(StartPos, MONITOR_DEFAULTTONULL);
		if(hMonitor)
		{
			MONITORINFO mi;
			mi.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(hMonitor, &mi);
			rect = mi.rcMonitor;
		}
		else
		{
			SetRect(&rect, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		}

		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = rect.right-rect.left;
		bmi.bmiHeader.biHeight = rect.bottom-rect.top;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		if(hBitmapOldGestureTrail==NULL)
		{
			hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, NULL, NULL, NULL);
			hBitmapOldGestureTrail = (HBITMAP)SelectObject(hdcMouseGestureTrail, hBitmap);
		}

		SetWindowPos(hwndLayeredWindow, HWND_TOPMOST, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOACTIVATE|SWP_SHOWWINDOW);
	}
}

void MouseGestureTrailEnd()
{
	if(hwndLayeredWindow)
	{
		HBITMAP hBitmap;

		ShowWindow(hwndLayeredWindow, SW_HIDE);

		Gdiplus::Graphics graphics(hdcMouseGestureTrail);
		graphics.Clear(Gdiplus::Color(0, 0, 0, 0));

		HDC hdcDst = GetDC(NULL);
		BITMAP bm;
		GetObject(GetCurrentObject(hdcMouseGestureTrail, OBJ_BITMAP), sizeof(BITMAP), &bm);
		SIZE size = {bm.bmWidth, bm.bmHeight};
		POINT ptSrc = {0, 0};
		BLENDFUNCTION blend = {AC_SRC_OVER, 0, 0xff, AC_SRC_ALPHA};
		UpdateLayeredWindow(
			hwndLayeredWindow, hdcDst, NULL, &size, hdcMouseGestureTrail, &ptSrc, 0, &blend, ULW_ALPHA);
		ReleaseDC(NULL, hdcDst);

		//ビットマップ破棄
		hBitmap = (HBITMAP)SelectObject(hdcMouseGestureTrail, hBitmapOldGestureTrail);
		DeleteObject(hBitmap);
		hBitmapOldGestureTrail = NULL;
	}
	else
	{
		if(GestureTrail.size()>1)
			InvalidateRect(NULL, NULL, 0);
	}

	GestureTrail.clear();
}

void InitMouseGestureTrail(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style         = 0;
	wc.lpfnWndProc   = DefWindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = TEXT("MauSujiLayeredWindow");
	RegisterClass(&wc);
}

void TermMouseGestureTrail(HINSTANCE hInstance)
{
	UnregisterClass(TEXT("MauSujiLayeredWindow"), hInstance);
}

void SortAction()
{
	int i, j, k;
	DWORD valueL, valueR;

	for(i=0;i<(int)Action.size()-1;i++)
	{
		for(j=i+1;j<(int)Action.size();j++)
		{
			valueL = (Action[i].TargetNumber * 0x100 * (MAX_TARGET_DATA + 1)) + (Action[i].Button * 0x100) + Action[i].Modifier;
			valueR = (Action[j].TargetNumber * 0x100 * (MAX_TARGET_DATA + 1)) + (Action[j].Button * 0x100) + Action[j].Modifier;

			if(valueL > valueR)
			{
				std::swap(Action[i], Action[j]);
			}
			else if(valueL == valueR)
			{
				k = 0;
				while(k < (int)Action.size() && Action[i].Move[k] == Action[j].Move[k])
					k ++;
				if(k < (int)Action.size())
				{
					if(Action[i].Move[k] > Action[j].Move[k])
					{
						std::swap(Action[i], Action[j]);
					}
				}
			}
		}
	}
}

int MinTargetNumber()
{
	int i;
	BOOL Number[MAX_TARGET_DATA] = {FALSE};
	int FoundNumber = MAX_TARGET_DATA;

	for(i=0; i<(int)Target.size(); i++)
		Number[Target[i].Number] = TRUE;

	for(i=MAX_TARGET_DATA-1; i>=1; i--)
		if(Number[i] == FALSE)
			FoundNumber = i;

	return FoundNumber;
}

void get_keyname(const LONG SendKey, LPTSTR lpszKeyName, int nSize)
{
	TCHAR szKeyName[1024] = TEXT("");
	UINT modifiers = ((HIBYTE(SendKey) & HOTKEYF_SHIFT) ? MOD_SHIFT : 0) |
		((HIBYTE(SendKey) & HOTKEYF_CONTROL) ? MOD_CONTROL : 0) |
		((HIBYTE(SendKey) & HOTKEYF_ALT) ? MOD_ALT : 0) |
		((HIBYTE(SendKey) & HOTKEYF_WIN) ? MOD_WIN : 0);
	UINT virtkey = LOBYTE(SendKey);
	UINT scan_code;
	int ext_flag = 0;

	if(modifiers & MOD_CONTROL)
		lstrcat(szKeyName, TEXT("Ctrl + "));
	if(modifiers & MOD_SHIFT)
		lstrcat(szKeyName, TEXT("Shift + "));
	if(modifiers & MOD_ALT)
		lstrcat(szKeyName, TEXT("Alt + "));
	if(modifiers & MOD_WIN)
		lstrcat(szKeyName, TEXT("Win + "));

	switch(virtkey)
	{
	case VK_LBUTTON:
		lstrcat(szKeyName, TEXT("Left mouse button"));
		break;
	case VK_RBUTTON:
		lstrcat(szKeyName, TEXT("Right mouse button"));
		break;
	case VK_MBUTTON:
		lstrcat(szKeyName, TEXT("Middle mouse button"));
		break;
	case VK_XBUTTON1:
		lstrcat(szKeyName, TEXT("X1 mouse button"));
		break;
	case VK_XBUTTON2:
		lstrcat(szKeyName, TEXT("X2 mouse button"));
		break;
	case VK_PAUSE:
		lstrcat(szKeyName, TEXT("Pause"));
		break;
	case VK_CAPITAL:
		lstrcat(szKeyName, TEXT("Caps Lock"));
		break;
	case VK_KANJI:
		lstrcat(szKeyName, TEXT("IME On/Off"));
		break;
	case VK_SNAPSHOT:
		lstrcat(szKeyName, TEXT("Print Screen"));
		break;
	case VK_LWIN:
		lstrcat(szKeyName, TEXT("Win"));
		break;
	case VK_SEPARATOR:
		lstrcat(szKeyName, TEXT("Num Enter"));
		break;
	case 0x5F:
		lstrcat(szKeyName, TEXT("Computer Sleep"));
		break;
	case VK_DECIMAL:
		lstrcat(szKeyName, TEXT("Num ."));
		break;
	case 0xA6:
		lstrcat(szKeyName, TEXT("Browser Back"));
		break;
	case 0xA7:
		lstrcat(szKeyName, TEXT("Browser Forward"));
		break;
	case 0xA8:
		lstrcat(szKeyName, TEXT("Browser Refresh"));
		break;
	case 0xA9:
		lstrcat(szKeyName, TEXT("Browser Stop"));
		break;
	case 0xAA:
		lstrcat(szKeyName, TEXT("Browser Search"));
		break;
	case 0xAB:
		lstrcat(szKeyName, TEXT("Browser Favorites"));
		break;
	case 0xAC:
		lstrcat(szKeyName, TEXT("Browser Start and Home"));
		break;
	case 0xAD:
		lstrcat(szKeyName, TEXT("Volume Mute"));
		break;
	case 0xAE:
		lstrcat(szKeyName, TEXT("Volume Down"));
		break;
	case 0xAF:
		lstrcat(szKeyName, TEXT("Volume Up"));
		break;
	case 0xB0:
		lstrcat(szKeyName, TEXT("Next Track"));
		break;
	case 0xB1:
		lstrcat(szKeyName, TEXT("Previous Track"));
		break;
	case 0xB2:
		lstrcat(szKeyName, TEXT("Stop Media"));
		break;
	case 0xB3:
		lstrcat(szKeyName, TEXT("Play/Pause Media"));
		break;
	case 0xB4:
		lstrcat(szKeyName, TEXT("Start Mail"));
		break;
	case 0xB5:
		lstrcat(szKeyName, TEXT("Select Media"));
		break;
	case 0xB6:
		lstrcat(szKeyName, TEXT("Start Application 1"));
		break;
	case 0xB7:
		lstrcat(szKeyName, TEXT("Start Application 2"));
		break;
	default:
		scan_code = MapVirtualKey(virtkey, 0);
		if(virtkey == VK_CANCEL ||
			virtkey == VK_DIVIDE ||
			virtkey == VK_APPS ||
			virtkey == VK_PRIOR ||
			virtkey == VK_NEXT ||
			virtkey == VK_END ||
			virtkey == VK_HOME ||
			virtkey == VK_LEFT ||
			virtkey == VK_UP ||
			virtkey == VK_RIGHT ||
			virtkey == VK_DOWN ||
			virtkey == VK_INSERT ||
			virtkey == VK_DELETE ||
			virtkey == VK_NUMLOCK) ext_flag = 1 << 24;
		GetKeyNameText((scan_code << 16) | ext_flag, szKeyName + lstrlen(szKeyName), 1024-lstrlen(szKeyName));
		break;
	}
	lstrcpyn(lpszKeyName, szKeyName, nSize);
}

void get_modifiername(const BYTE Modifier, LPTSTR lpszModifierName, int nSize)
{
	TCHAR szModifierName[1024] = TEXT("");
	if(Modifier & MODIFIER_CONTROL)
		lstrcat(szModifierName, TEXT("Ctrl + "));
	if(Modifier & MODIFIER_SHIFT)
		lstrcat(szModifierName, TEXT("Shift + "));
	lstrcpyn(lpszModifierName, szModifierName, nSize);
}

void OpenHelp()
{
	TCHAR HelpName[MAX_PATH];
	TCHAR ExePath[MAX_PATH];

	GetModuleFileName(NULL, ExePath, MAX_PATH);
	PathRemoveFileSpec(ExePath);
	wsprintf(HelpName, TEXT("%s\\MauSuji.chm"), ExePath);
	if(PathFileExists(HelpName))
		ShellExecute(NULL, NULL, TEXT("hh.exe"), HelpName, NULL, SW_SHOWDEFAULT);
}

INT_PTR CALLBACK DialogProcVersion(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT_OPENMAUSUJI_VERSION, OPENMAUSUJI_VERSION);
		SetDlgItemText(hDlg, IDC_EDIT_MAUSUJI_VERSION, MAUSUJI_VERSION);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, TRUE);
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	}
	return FALSE;
}

void ShowVersionInfo(HWND hwnd)
{
	static BOOL bOpenDialog = FALSE;

	if(!bOpenDialog)
	{
		bOpenDialog = TRUE;
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_VERSION), hwnd, DialogProcVersion);
		bOpenDialog = FALSE;
	}
}

LPTSTR IntToStr(int value, LPTSTR buffer, int size)
{
	TCHAR szBuffer[1024];
	wsprintf(szBuffer, TEXT("%d"), value);
	lstrcpyn(buffer, szBuffer, size);
	return buffer;
}

void WriteXmlTarget(IXmlWriter* pWriter, TARGETITEM* target)
{
	WCHAR szText[1024];
	int i, j;

	pWriter->WriteStartElement(NULL, L"Target", NULL);
	if(!lstrcmpi(target->ClassName, TEXT("Default")))
	{
		pWriter->WriteAttributeString(NULL, L"ClassName", NULL, L"Default");
	}
	else
	{
		pWriter->WriteAttributeString(NULL, L"Number", NULL, IntToStr(target->Number, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"Succession", NULL, IntToStr(target->DefaultThru, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"HookType", NULL, IntToStr(target->HookType, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"WheelRedirect", NULL, IntToStr(target->WheelRedirect, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FreeScroll", NULL, IntToStr(target->FreeScroll, szText, 1024));
		if(target->FileName[0])
			pWriter->WriteAttributeString(NULL, L"FileName", NULL, target->FileName);
		if(target->WindowTitle[0])
			pWriter->WriteAttributeString(NULL, L"WindowTitle", NULL, target->WindowTitle);
		if(target->ClassName[0])
			pWriter->WriteAttributeString(NULL, L"ClassName", NULL, target->ClassName);
		if(target->ControlID[0])
			pWriter->WriteAttributeString(NULL, L"ControlID", NULL, target->ControlID);
		if(target->Comment[0])
			pWriter->WriteAttributeString(NULL, L"Comment", NULL, target->Comment);
	}
	for(i=0;i<(int)target->action.size();i++)
	{
		pWriter->WriteStartElement(NULL, L"Action", NULL);
		if(LeftyMode && (target->action[i].Button == BUTTON_L || target->action[i].Button == BUTTON_R))
			pWriter->WriteAttributeString(NULL, L"Gesture", NULL, IntToStr(BUTTON_L + BUTTON_R - target->action[i].Button, szText, 1024));
		else
			pWriter->WriteAttributeString(NULL, L"Gesture", NULL, IntToStr(target->action[i].Button, szText, 1024));
		if(!(target->action[i].Modifier & MODIFIER_DISABLE))
			pWriter->WriteAttributeString(NULL, L"Modifier", NULL, IntToStr(target->action[i].Modifier, szText, 1024));
		if(LeftyMode && target->action[i].Move[0] > 0 && (target->action[i].Move[0] == BUTTON_L || target->action[i].Move[0] == BUTTON_R))
			pWriter->WriteAttributeString(NULL, L"Gesture_0", NULL, IntToStr(BUTTON_L + BUTTON_R - target->action[i].Move[0], szText, 1024));
		else
			pWriter->WriteAttributeString(NULL, L"Gesture_0", NULL, IntToStr(target->action[i].Move[0], szText, 1024));
		j = 1;
		while(j < MAX_GESTURE_LEVEL && target->action[i].Move[j] > 0)
		{
			WCHAR szKey[1024];
			wsprintfW(szKey, L"Gesture_%d", j);
			pWriter->WriteAttributeString(NULL, szKey, NULL, IntToStr(target->action[i].Move[j], szText, 1024));
			j++;
		}
		if(target->action[i].Comment[0])
			pWriter->WriteAttributeString(NULL, L"Comment", NULL, target->action[i].Comment);
		for(j=0;j<(int)target->action[i].command.size();j++)
		{
			pWriter->WriteStartElement(NULL, L"Command", NULL);
			pWriter->WriteAttributeString(NULL, L"Case", NULL, IntToStr(target->action[i].command[j].Case, szText, 1024));
			pWriter->WriteAttributeString(NULL, L"CommandTarget", NULL, IntToStr(target->action[i].command[j].CommandTarget, szText, 1024));
			switch(target->action[i].command[j].Case)
			{ 
			case 1:
				pWriter->WriteAttributeString(NULL, L"Wait", NULL, IntToStr(target->action[i].command[j].Wait, szText, 1024));
				pWriter->WriteAttributeString(NULL, L"path", NULL, target->action[i].command[j].CommandExt.path);
				pWriter->WriteAttributeString(NULL, L"value1", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[0], szText, 1024));
				pWriter->WriteAttributeString(NULL, L"value2", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[1], szText, 1024));
				pWriter->WriteAttributeString(NULL, L"value3", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[2], szText, 1024));
				pWriter->WriteAttributeString(NULL, L"value4", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[3], szText, 1024));
				pWriter->WriteAttributeString(NULL, L"text1", NULL, target->action[i].command[j].CommandExt.Text[0]);
				pWriter->WriteAttributeString(NULL, L"text2", NULL, target->action[i].command[j].CommandExt.Text[1]);
				pWriter->WriteAttributeString(NULL, L"text3", NULL, target->action[i].command[j].CommandExt.Text[2]);
				pWriter->WriteAttributeString(NULL, L"text4", NULL, target->action[i].command[j].CommandExt.Text[3]);
				break;
			case 10:case 11:case 12:case 20:case 40:case 41:
			case 42:case 55:case 58:case 59:case 50:case 60:case 65:
				pWriter->WriteAttributeString(NULL, L"Wait", NULL, IntToStr(target->action[i].command[j].Wait, szText, 1024));
				pWriter->WriteAttributeString(NULL, L"Key", NULL, IntToStr(target->action[i].command[j].Key, szText, 1024));
				break;
			case 25:case 30:case 35:
				pWriter->WriteAttributeString(NULL, L"Wait", NULL, IntToStr(target->action[i].command[j].Wait, szText, 1024));
				if(target->action[i].command[j].CommandExt.Text[0][0])
					pWriter->WriteAttributeString(NULL, L"MainText", NULL, target->action[i].command[j].CommandExt.Text[0]);
				if(target->action[i].command[j].CommandExt.Text[1][0])
					pWriter->WriteAttributeString(NULL, L"SubText", NULL, target->action[i].command[j].CommandExt.Text[1]);
				break;
			case 70:case 71:
				pWriter->WriteAttributeString(NULL, L"Wait", NULL, IntToStr(target->action[i].command[j].Wait, szText, 1024));
				pWriter->WriteAttributeString(NULL, L"Message", NULL, target->action[i].command[j].CommandExt.Text[0]);
				pWriter->WriteAttributeString(NULL, L"MessageType", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[0], szText, 1024));
				pWriter->WriteAttributeString(NULL, L"wParam", NULL, target->action[i].command[j].CommandExt.Text[1]);
				pWriter->WriteAttributeString(NULL, L"wParamType", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[1], szText, 1024));
				pWriter->WriteAttributeString(NULL, L"lParam", NULL, target->action[i].command[j].CommandExt.Text[2]);
				pWriter->WriteAttributeString(NULL, L"lParamType", NULL, IntToStr(target->action[i].command[j].CommandExt.Key[2], szText, 1024));
				break;
			case 99:case 95:case 96:
				break;
			default:
				break;
			}
			pWriter->WriteFullEndElement();	//Commmand
		}
		pWriter->WriteFullEndElement();	//Action
	}
	pWriter->WriteFullEndElement();	//Target
}
//CONFIGからXMLを作成
// flags      :0   :すべての設定を出力
//            :1   :すべてのターゲットを出力
//            :2   :指定したひとつのターゲットを出力
// Locate     :    :flagsが2のとき出力するターゲットインデックス
void ConfigWriteXml(CONFIG* config, IXmlWriter* pWriter, DWORD flags, int Locate)
{
	if(flags==1)
	{
		int i;
		pWriter->WriteStartElement(NULL, L"Config", NULL);
		pWriter->WriteStartElement(NULL, L"Targets", NULL);
		for(i=0;i<(int)config->target.size();i++)
			WriteXmlTarget(pWriter, &config->target[i]);
		pWriter->WriteFullEndElement();	//Targets
		pWriter->WriteFullEndElement();	//Config
	}
	else if(flags==2)
	{
		pWriter->WriteStartElement(NULL, L"Config", NULL);
		pWriter->WriteStartElement(NULL, L"Targets", NULL);
		if(Locate<(int)config->target.size())
			WriteXmlTarget(pWriter, &config->target[Locate]);
		pWriter->WriteFullEndElement();	//Targets
		pWriter->WriteFullEndElement();	//Config
	}
	else
	{
		TCHAR szText[1024];
		int i;
		pWriter->WriteStartElement(NULL, L"Config", NULL);
		pWriter->WriteStartElement(NULL, L"Main", NULL);
		pWriter->WriteAttributeString(NULL, L"IniVersion", NULL, IntToStr(INI_VERSION, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"TaskTrayIcon", NULL, IntToStr(config->ShowTaskTray, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"GestureDirection", NULL, IntToStr(config->GestureDirection, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"TimeOutType", NULL, IntToStr(config->TimeOutType, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"DefaultHookType", NULL, IntToStr(config->DefaultHookType, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"Priority", NULL, IntToStr(config->Priority, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"ClickWait", NULL, IntToStr(config->ClickWait, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"WheelRedirect", NULL, IntToStr(config->WheelRedirect, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"WheelActive", NULL, IntToStr(config->WheelActive, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"KuruKuru", NULL, IntToStr(config->KuruKuruFlag, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FreeScroll", NULL, IntToStr(config->DefaultFreeScroll, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"ScrollSensitivity", NULL, IntToStr(config->ScrollSensitivity, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"KuruKuruTimeOut", NULL, IntToStr(config->KuruKuruTimeOut, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"CircleRate", NULL, IntToStr(config->CircleRate, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"DefaultWheelRedirect", NULL, IntToStr(config->DefaultWheelRedirect, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"CheckInterval", NULL, IntToStr(config->CheckInterval, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"GestureSensitivity", NULL, IntToStr(config->MoveQuantity, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"GestureTimeOut", NULL, IntToStr(config->GestureTimeOut, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"GestureStartSensitivity", NULL, IntToStr(config->GestureStartQuantity, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"CornerSensitivity", NULL, IntToStr(config->CornerTime, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"CornerAreaX", NULL, IntToStr(config->CornerPosX, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"CornerAreaY", NULL, IntToStr(config->CornerPosY, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"CursorChange", NULL, IntToStr(config->CursorChange, szText, 1024));
		if(config->GestureCursor[0])
			pWriter->WriteAttributeString(NULL, L"GestureCursor", NULL, config->GestureCursor);
		if(config->WaitCursor[0])
			pWriter->WriteAttributeString(NULL, L"WaitCursor", NULL, config->WaitCursor);
		pWriter->WriteFullEndElement();	//Main
		pWriter->WriteStartElement(NULL, L"Navi", NULL);
		pWriter->WriteAttributeString(NULL, L"NaviType", NULL, IntToStr(config->NaviType, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"NaviPositionX", NULL, IntToStr(config->MauSujiNavi.x, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"NaviPositionY", NULL, IntToStr(config->MauSujiNavi.y, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"NaviDeleteTime", NULL, IntToStr(config->NaviDeleteTime, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"NaviWidth", NULL, IntToStr(config->NaviWidth, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"TipLocateX", NULL, IntToStr(config->TipLocate.x, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"TipLocateY", NULL, IntToStr(config->TipLocate.y, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"NaviTitle", NULL, config->NaviTitle);
		pWriter->WriteAttributeString(NULL, L"FontHeight", NULL, IntToStr(config->NaviFont.lfHeight, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FontWeight", NULL, IntToStr(config->NaviFont.lfWeight, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FontItalic", NULL, IntToStr(config->NaviFont.lfItalic, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FontCharSet", NULL, IntToStr(config->NaviFont.lfCharSet, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"MainTextColor", NULL, IntToStr(config->MainColor, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"SubTextColor", NULL, IntToStr(config->SubColor, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"BackgroundColor", NULL, IntToStr(config->BackColor, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FrameColor", NULL, IntToStr(config->FrameColor, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"FontName", NULL, config->NaviFont.lfFaceName);
		pWriter->WriteFullEndElement();	//Navi
		pWriter->WriteStartElement(NULL, L"Character", NULL);
		pWriter->WriteAttributeString(NULL, L"Up", NULL, config->Character[0]);
		pWriter->WriteAttributeString(NULL, L"Down", NULL, config->Character[1]);
		pWriter->WriteAttributeString(NULL, L"Left", NULL, config->Character[2]);
		pWriter->WriteAttributeString(NULL, L"Right", NULL, config->Character[3]);
		pWriter->WriteAttributeString(NULL, L"UpLeft", NULL, config->Character[4]);
		pWriter->WriteAttributeString(NULL, L"UpRight", NULL, config->Character[5]);
		pWriter->WriteAttributeString(NULL, L"DownLeft", NULL, config->Character[6]);
		pWriter->WriteAttributeString(NULL, L"DownRight", NULL, config->Character[7]);
		pWriter->WriteAttributeString(NULL, L"CornerTopLeft", NULL, config->Character[8]);
		pWriter->WriteAttributeString(NULL, L"CornerTopRight", NULL, config->Character[9]);
		pWriter->WriteAttributeString(NULL, L"CornerBottomLeft", NULL, config->Character[10]);
		pWriter->WriteAttributeString(NULL, L"CornerBottomRight", NULL, config->Character[11]);
		pWriter->WriteFullEndElement();	//Character
		pWriter->WriteStartElement(NULL, L"MouseGestureTrail", NULL);
		pWriter->WriteAttributeString(NULL, L"Enable", NULL, IntToStr(config->MouseGestureTrail, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"Width", NULL, IntToStr(config->MouseGestureTrailWidth, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"Color", NULL, IntToStr(config->MouseGestureTrailColor, szText, 1024));
		pWriter->WriteAttributeString(NULL, L"DrawMode", NULL, IntToStr(config->MouseGestureTrailDrawMode, szText, 1024));
		pWriter->WriteFullEndElement();	//MouseGestureTrail
		pWriter->WriteStartElement(NULL, L"Targets", NULL);
		for(i=0;i<(int)config->target.size();i++)
			WriteXmlTarget(pWriter, &config->target[i]);
		pWriter->WriteFullEndElement();	//Targets
		pWriter->WriteFullEndElement();	//Config
	}

	pWriter->Flush();
}
//CONFIGからXML文字列を作成、戻り値をHeapFreeする
// outputflags:0x01:BOMあり
//            :0x02:XML宣言あり
// flags      :0   :すべての設定を出力
//            :1   :すべてのターゲットを出力
//            :2   :指定したひとつのターゲットを出力
// Locate     :    :flagsが2のとき出力するターゲットインデックス
void ConfigWriteXmlString(CONFIG* config, LPWSTR *xml, DWORD outputflags, DWORD flags, int Locate)
{
	HGLOBAL hXml;
	_com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)>> pStream;
	_com_ptr_t<_com_IIID<IXmlWriter, &__uuidof(IXmlWriter)>> pWriter;
	_com_ptr_t<_com_IIID<IXmlWriterOutput, &__uuidof(IXmlWriterOutput)>> pWriterOutput;
	LPWSTR p;

	hXml = GlobalAlloc(GMEM_MOVEABLE, 0);
	CreateStreamOnHGlobal(hXml, TRUE, &pStream);

	CreateXmlWriter(__uuidof(IXmlWriter), (void**)&pWriter, NULL);
	CreateXmlWriterOutputWithEncodingName(pStream, NULL, L"utf-16", &pWriterOutput);	// エンコード指定
	pWriter->SetOutput(pWriterOutput);
	pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);	// インデント有効化
	pWriter->SetProperty(XmlWriterProperty_ByteOrderMark, (outputflags&0x01)?TRUE:FALSE);	// BOM
	if(outputflags&0x02)
		pWriter->WriteStartDocument(XmlStandalone_Omit);	// <?xml version="1.0" encoding="UTF-16"?>

	ConfigWriteXml(config, pWriter, flags, Locate);

	p = (LPWSTR)GlobalLock(hXml);
	*xml = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (lstrlenW(p)+1)*sizeof(WCHAR));
	lstrcpyW(*xml, p);
	GlobalUnlock(hXml);

	GlobalFree(hXml);

	p = *xml+lstrlenW(*xml);while(p>*xml){if(*p=='>'){*(p+1)='\0';break;}p--;}	//終端にごみがあることがあるので消す
}
//CONFIGからXMLファイルを作成
void ConfigWriteXmlFile(CONFIG* config, LPTSTR path)
{
	_com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)>> pStream;
	_com_ptr_t<_com_IIID<IXmlWriter, &__uuidof(IXmlWriter)>> pWriter;

	SHCreateStreamOnFile(path, STGM_CREATE|STGM_WRITE, &pStream);

	CreateXmlWriter(__uuidof(IXmlWriter), (void**)&pWriter, NULL);
	pWriter->SetOutput(pStream);
	pWriter->SetProperty(XmlWriterProperty_Indent, TRUE);	// インデント有効化
	pWriter->SetProperty(XmlWriterProperty_ByteOrderMark, TRUE);	// BOM
	pWriter->WriteStartDocument(XmlStandalone_Omit);	// <?xml version="1.0" encoding="UTF-8"?>

	ConfigWriteXml(config, pWriter, 0, 0);
}
//XMLからCONFIGを作成
void ConfigReadXml(IXmlReader* pReader, CONFIG* config, UINT* pIniVersion)
{
	XmlNodeType nodeType;
	LPCWSTR pwszLocalName;
	UINT nDepth;
	std::vector<std::wstring> LocalName;
	int i;
	std::wstring path;
	LPCWSTR pwszAttributeName;
	LPCWSTR pwszAttributeValue;

	config->ShowTaskTray = TRUE;
	config->GestureDirection = 4;
	config->TimeOutType = FALSE;
	config->DefaultHookType = 1;
	config->Priority = 0x0020;
	config->ClickWait = 50;
	config->WheelRedirect = FALSE;
	config->WheelActive = FALSE;
	config->KuruKuruFlag = FALSE;
	config->DefaultFreeScroll = FALSE;
	config->ScrollSensitivity = 30;
	config->KuruKuruTimeOut = 200;
	config->CircleRate = 60;
	config->DefaultWheelRedirect = FALSE;
	config->CheckInterval = 50;
	config->MoveQuantity = 50;
	config->GestureTimeOut = 500;
	config->GestureStartQuantity = 30;
	config->CornerTime = 500;
	config->CornerPosX = 70;
	config->CornerPosY = 70;
	config->CursorChange = TRUE;
	lstrcpy(config->GestureCursor, TEXT(""));
	lstrcpy(config->WaitCursor, TEXT(""));

	config->NaviType = NaviTypeFloat;
	config->MauSujiNavi.x = 0;
	config->MauSujiNavi.y = 0;
	config->NaviDeleteTime = 1000;
	config->NaviWidth = 0;
	config->TipLocate.x = 0;
	config->TipLocate.y = 20;
	lstrcpy(config->NaviTitle, TEXT("マウ筋"));
	ZeroMemory(&config->NaviFont, sizeof(LOGFONT));
	config->NaviFont.lfHeight = 12;
	config->NaviFont.lfWeight = 0;
	config->NaviFont.lfItalic = 0;
	config->NaviFont.lfCharSet = 128;
	config->MainColor = 0;
	config->SubColor = 8421504;
	config->BackColor = 12582911;
	config->FrameColor = 0;
	lstrcpy(config->NaviFont.lfFaceName, TEXT("MS UI Gothic"));

	lstrcpy(config->Character[0], TEXT("↑"));
	lstrcpy(config->Character[1], TEXT("↓"));
	lstrcpy(config->Character[2], TEXT("←"));
	lstrcpy(config->Character[3], TEXT("→"));
	lstrcpy(config->Character[4], TEXT("\x2196"));
	lstrcpy(config->Character[5], TEXT("\x2197"));
	lstrcpy(config->Character[6], TEXT("\x2199"));
	lstrcpy(config->Character[7], TEXT("\x2198"));
	lstrcpy(config->Character[8], TEXT("↑←"));
	lstrcpy(config->Character[9], TEXT("↑→"));
	lstrcpy(config->Character[10], TEXT("↓←"));
	lstrcpy(config->Character[11], TEXT("↓→"));

	config->MouseGestureTrail = TRUE;
	config->MouseGestureTrailWidth = 3;
	config->MouseGestureTrailColor = RGB(255, 0, 0);
	config->MouseGestureTrailDrawMode = 0;

	if(pReader==NULL)
		return;

	while(S_OK == pReader->Read(&nodeType))
	{
		switch(nodeType)
		{
		case XmlNodeType_Element:
			pReader->GetLocalName(&pwszLocalName, NULL);
			pReader->GetDepth(&nDepth);

			//XPath風のpath
			while(nDepth!=LocalName.size())
				LocalName.pop_back();
			LocalName.push_back(pwszLocalName);
			path = L"";
			for(i=0;i<(int)LocalName.size();i++)
			{
				path += L"/";
				path += LocalName[i].c_str();
			}

			//attribute
			if(!lstrcmpiW(path.c_str(), L"/Config"))
			{
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/Main") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				do{
					if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
						S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
					{
						if(!lstrcmpiW(pwszAttributeName, L"IniVersion") && pIniVersion)
							*pIniVersion = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"TaskTrayIcon"))
							config->ShowTaskTray = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"GestureDirection"))
							config->GestureDirection = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"TimeOutType"))
							config->TimeOutType = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"DefaultHookType"))
							config->DefaultHookType = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"Priority"))
							config->Priority = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"ClickWait"))
							config->ClickWait = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"WheelRedirect"))
							config->WheelRedirect = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"WheelActive"))
							config->WheelActive = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"KuruKuru"))
							config->KuruKuruFlag = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FreeScroll"))
							config->DefaultFreeScroll = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"ScrollSensitivity"))
							config->ScrollSensitivity = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"KuruKuruTimeOut"))
							config->KuruKuruTimeOut = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"CircleRate"))
							config->CircleRate = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"DefaultWheelRedirect"))
							config->DefaultWheelRedirect = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"CheckInterval"))
							config->CheckInterval = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"GestureSensitivity"))
							config->MoveQuantity = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"GestureTimeOut"))
							config->GestureTimeOut = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"GestureStartSensitivity"))
							config->GestureStartQuantity = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"CornerSensitivity"))
							config->CornerTime = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"CornerAreaX"))
							config->CornerPosX = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"CornerAreaY"))
							config->CornerPosY = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"CursorChange"))
							config->CursorChange = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"GestureCursor"))
							lstrcpynW(config->GestureCursor, pwszAttributeValue, _MAX_PATH);
						if(!lstrcmpiW(pwszAttributeName, L"WaitCursor"))
							lstrcpynW(config->WaitCursor, pwszAttributeValue, _MAX_PATH);
					}
				}while(S_OK == pReader->MoveToNextAttribute());
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/Navi") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				do{
					if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
						S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
					{
						if(!lstrcmpiW(pwszAttributeName, L"NaviType"))
							config->NaviType = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"NaviPositionX"))
							config->MauSujiNavi.x = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"NaviPositionY"))
							config->MauSujiNavi.y = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"NaviDeleteTime"))
							config->NaviDeleteTime = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"NaviWidth"))
							config->NaviWidth = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"TipLocateX"))
							config->TipLocate.x = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"TipLocateY"))
							config->TipLocate.y = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"NaviTitle"))
							lstrcpynW(config->NaviTitle, pwszAttributeValue, _MAX_PATH);
						if(!lstrcmpiW(pwszAttributeName, L"FontHeight"))
							config->NaviFont.lfHeight = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FontWeight"))
							config->NaviFont.lfWeight = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FontItalic"))
							config->NaviFont.lfItalic = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FontCharSet"))
							config->NaviFont.lfCharSet = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"MainTextColor"))
							config->MainColor = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"SubTextColor"))
							config->SubColor = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"BackgroundColor"))
							config->BackColor = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FrameColor"))
							config->FrameColor = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FontName"))
							lstrcpynW(config->NaviFont.lfFaceName, pwszAttributeValue, LF_FACESIZE);
					}
				}while(S_OK == pReader->MoveToNextAttribute());
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/Character") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				do{
					if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
						S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
					{
						if(!lstrcmpiW(pwszAttributeName, L"Up"))
							lstrcpynW(config->Character[0], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"Down"))
							lstrcpynW(config->Character[1], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"Left"))
							lstrcpynW(config->Character[2], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"Right"))
							lstrcpynW(config->Character[3], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"UpLeft"))
							lstrcpynW(config->Character[4], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"UpRight"))
							lstrcpynW(config->Character[5], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"DownLeft"))
							lstrcpynW(config->Character[6], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"DownRight"))
							lstrcpynW(config->Character[7], pwszAttributeValue, 3);
						if(!lstrcmpiW(pwszAttributeName, L"CornerTopLeft"))
							lstrcpynW(config->Character[8], pwszAttributeValue, 5);
						if(!lstrcmpiW(pwszAttributeName, L"CornerTopRight"))
							lstrcpynW(config->Character[9], pwszAttributeValue, 5);
						if(!lstrcmpiW(pwszAttributeName, L"CornerBottomLeft"))
							lstrcpynW(config->Character[10], pwszAttributeValue, 5);
						if(!lstrcmpiW(pwszAttributeName, L"CornerBottomRight"))
							lstrcpynW(config->Character[11], pwszAttributeValue, 5);
					}
				}while(S_OK == pReader->MoveToNextAttribute());
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/MouseGestureTrail") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				do{
					if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
						S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
					{
						if(!lstrcmpiW(pwszAttributeName, L"Enable"))
							config->MouseGestureTrail = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"Width"))
							config->MouseGestureTrailWidth = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"Color"))
							config->MouseGestureTrailColor = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"DrawMode"))
							config->MouseGestureTrailDrawMode = StrToIntW(pwszAttributeValue);
					}
				}while(S_OK == pReader->MoveToNextAttribute());
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/Targets/Target") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				TARGETITEM t = {0};

				do{
					if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
						S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
					{
						if(!lstrcmpiW(pwszAttributeName, L"Number"))
							t.Number = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"Succession"))
							t.DefaultThru = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"HookType"))
							t.HookType = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"WheelRedirect"))
							t.WheelRedirect = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FreeScroll"))
							t.FreeScroll = StrToIntW(pwszAttributeValue);
						if(!lstrcmpiW(pwszAttributeName, L"FileName"))
							lstrcpynW(t.FileName, pwszAttributeValue, _MAX_PATH);
						if(!lstrcmpiW(pwszAttributeName, L"WindowTitle"))
							lstrcpynW(t.WindowTitle, pwszAttributeValue, MAX_CLASSNAME);
						if(!lstrcmpiW(pwszAttributeName, L"ClassName"))
							lstrcpynW(t.ClassName, pwszAttributeValue, MAX_CLASSNAME);
						if(!lstrcmpiW(pwszAttributeName, L"ControlID"))
							lstrcpynW(t.ControlID, pwszAttributeValue, 20);
						if(!lstrcmpiW(pwszAttributeName, L"Comment"))
							lstrcpynW(t.Comment, pwszAttributeValue, MAX_COMMENT);
					}
				}while(S_OK == pReader->MoveToNextAttribute());

				config->target.push_back(t);
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/Targets/Target/Action") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				ACTIONITEM a = {0};
				int i, j;

				i = (int)config->target.size();
				if(i>0)
				{
					a.Modifier = MODIFIER_DISABLE;
					do{
						if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
							S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
						{
							if(!lstrcmpiW(pwszAttributeName, L"Gesture"))
								a.Button = StrToIntW(pwszAttributeValue);
							if(!lstrcmpiW(pwszAttributeName, L"Modifier"))
								a.Modifier = StrToIntW(pwszAttributeValue);
							for(j=0;j<MAX_GESTURE_LEVEL;j++)
							{
								std::wstring AttributeName = L"Gesture_";
								AttributeName += '0'+j;
								if(!lstrcmpiW(pwszAttributeName, AttributeName.c_str()))
									a.Move[j] = StrToIntW(pwszAttributeValue);
							}
							if(!lstrcmpiW(pwszAttributeName, L"Comment"))
								lstrcpynW(a.Comment, pwszAttributeValue, MAX_COMMENT);
						}
					}while(S_OK == pReader->MoveToNextAttribute());

					if(LeftyMode)
					{
						if(a.Button == BUTTON_L || a.Button == BUTTON_R)
							a.Button = BUTTON_L + BUTTON_R - a.Button;
						if(a.Move[0] > 0 && (a.Move[0] == BUTTON_L || a.Move[0] == BUTTON_R))
							a.Move[0] = BUTTON_L + BUTTON_R - a.Move[0];
					}
					config->target[i-1].action.push_back(a);
				}
			}
			if(!lstrcmpiW(path.c_str(), L"/Config/Targets/Target/Action/Command") &&
				S_OK == pReader->MoveToFirstAttribute())
			{
				COMMANDITEM c = {0};
				int i, ii;

				i = (int)config->target.size();
				if(i>0)
				{
					ii = (int)config->target[i-1].action.size();
					if(ii>0)
					{
						//Caseを探す
						do{
							if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
								S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
							{
								if(!lstrcmpiW(pwszAttributeName, L"Case"))
									c.Case = StrToIntW(pwszAttributeValue);
							}
						}while(S_OK == pReader->MoveToNextAttribute());

						if(c.Case==1||
							c.Case==10||c.Case==11||c.Case==12||
							c.Case==20||c.Case==25||
							c.Case==30||c.Case==35||
							c.Case==40||c.Case==41||c.Case==42||
							c.Case==55||c.Case==58||c.Case==59||c.Case==50||
							c.Case==60||c.Case==65||
							c.Case==70||c.Case==71||
							c.Case==99||c.Case==95||c.Case==96)//||c.Case==100||c.Case==101||c.Case==102)
						{
							//CaseがあったらCase以外を探す

							pReader->MoveToFirstAttribute();	//firstに戻す

							do{
								if(S_OK == pReader->GetLocalName(&pwszAttributeName, NULL) &&
									S_OK == pReader->GetValue(&pwszAttributeValue, NULL))
								{
									if(!lstrcmpiW(pwszAttributeName, L"CommandTarget"))
										c.CommandTarget = StrToIntW(pwszAttributeValue);
									switch(c.Case)
									{
									case 1:
										if(!lstrcmpiW(pwszAttributeName, L"Wait"))
											c.Wait = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"path"))
											lstrcpynW(c.CommandExt.path, pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"value1"))
											c.CommandExt.Key[0] = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"value2"))
											c.CommandExt.Key[1] = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"value3"))
											c.CommandExt.Key[2] = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"value4"))
											c.CommandExt.Key[3] = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"text1"))
											lstrcpynW(c.CommandExt.Text[0], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"text2"))
											lstrcpynW(c.CommandExt.Text[1], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"text3"))
											lstrcpynW(c.CommandExt.Text[2], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"text4"))
											lstrcpynW(c.CommandExt.Text[3], pwszAttributeValue, MAX_PATH);
										break;
									case 10:case 11:case 12:case 20:case 40:case 41:
									case 42:case 55:case 58:case 59:case 50:case 60:case 65:
										if(!lstrcmpiW(pwszAttributeName, L"Wait"))
											c.Wait = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"Key"))
											c.Key = StrToIntW(pwszAttributeValue);
										break;
									case 25:case 30:case 35:
										if(!lstrcmpiW(pwszAttributeName, L"Wait"))
											c.Wait = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"MainText"))
											lstrcpynW(c.CommandExt.Text[0], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"SubText"))
											lstrcpynW(c.CommandExt.Text[1], pwszAttributeValue, MAX_PATH);
										break;
									case 70:case 71:
										if(!lstrcmpiW(pwszAttributeName, L"Wait"))
											c.Wait = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"Message"))
											lstrcpynW(c.CommandExt.Text[0], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"MessageType"))
											c.CommandExt.Key[0] = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"wParam"))
											lstrcpynW(c.CommandExt.Text[1], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"wParamType"))
											c.CommandExt.Key[1] = StrToIntW(pwszAttributeValue);
										if(!lstrcmpiW(pwszAttributeName, L"lParam"))
											lstrcpynW(c.CommandExt.Text[2], pwszAttributeValue, MAX_PATH);
										if(!lstrcmpiW(pwszAttributeName, L"lParamType"))
											c.CommandExt.Key[2] = StrToIntW(pwszAttributeValue);
										break;
									case 99:case 95:case 96:
										break;
									default:
										break;
									}
								}
							}while(S_OK == pReader->MoveToNextAttribute());

							config->target[i-1].action[ii-1].command.push_back(c);
						}
					}
				}
			}
			break;
		}
	}
}
//XML文字列からCONFIGを作成
BOOL ConfigReadXmlString(LPCWSTR xml, CONFIG* config, UINT* pIniVersion)
{
	HGLOBAL hXml;
	LPWSTR p;
	_com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)>> pStream;
	_com_ptr_t<_com_IIID<IXmlReader, &__uuidof(IXmlReader)>> pReader;

	hXml = GlobalAlloc(GMEM_MOVEABLE, (lstrlenW(xml)+1)*sizeof(WCHAR));
	p = (LPWSTR)GlobalLock(hXml);
	lstrcpy(p, xml);
	GlobalUnlock(hXml);
	if(FAILED(CreateStreamOnHGlobal(hXml, TRUE, &pStream)))
	{
		pStream = NULL;
		pReader = NULL;
	}
	else
	{
		if(FAILED(CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
		{
			pReader = NULL;
		}
		else
		{
			pReader->SetInput(pStream);
		}
	}

	ConfigReadXml(pReader, config, pIniVersion);

	GlobalFree(hXml);

	if(pReader==NULL)
		return FALSE;

	return TRUE;
}
//XMLファイルからCONFIGを作成
BOOL ConfigReadXmlFile(LPCTSTR path, CONFIG* config, UINT* pIniVersion)
{
	_com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)>> pStream;
	_com_ptr_t<_com_IIID<IXmlReader, &__uuidof(IXmlReader)>> pReader;

	if(FAILED(SHCreateStreamOnFile(path, STGM_READ, &pStream)))
	{
		pStream = NULL;
		pReader = NULL;
	}
	else
	{
		if(FAILED(CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL)))
		{
			pReader = NULL;
		}
		else
		{
			pReader->SetInput(pStream);
		}
	}

	ConfigReadXml(pReader, config, pIniVersion);

	if(pReader==NULL)
		return FALSE;

	return TRUE;
}
//グローバル変数からCONFIGを作成する
void _GetConfig(CONFIG* config)
{
	int i;
	//Main
	config->ShowTaskTray = ShowTaskTray;
	config->GestureDirection = GestureDirection;
	config->TimeOutType = TimeOutType;
	config->DefaultHookType = Default.HookType;
	config->Priority = Priority;
	config->ClickWait = ClickWait;
	config->WheelRedirect = WheelRedirect;
	config->WheelActive = WheelActive;
	config->KuruKuruFlag = KuruKuruFlag;
	config->DefaultFreeScroll = Default.FreeScroll;
	config->ScrollSensitivity = ScrollSensitivity;
	config->KuruKuruTimeOut = KuruKuruTimeOut;
	config->CircleRate = CircleRate;
	config->DefaultWheelRedirect = Default.WheelRedirect;
	config->CheckInterval = CheckInterval;
	config->MoveQuantity = MoveQuantity;
	config->GestureTimeOut = GestureTimeOut;
	config->GestureStartQuantity = GestureStartQuantity;
	config->CornerTime = CornerTime;
	config->CornerPosX = CornerPosX;
	config->CornerPosY = CornerPosY;
	config->CursorChange = CursorChange;
	lstrcpy(config->GestureCursor, GestureCursor);
	lstrcpy(config->WaitCursor, WaitCursor);
	//Navi
	config->NaviType = NaviType;
	config->MauSujiNavi = MauSujiNavi;
	config->NaviDeleteTime = NaviDeleteTime;
	config->NaviWidth = NaviWidth;
	config->TipLocate = TipLocate;
	lstrcpy(config->NaviTitle, NaviTitle);
	config->NaviFont = lf;
	config->MainColor = MainColor;
	config->SubColor = SubColor;
	config->BackColor = BackColor;
	config->FrameColor = FrameColor;
	//Character
	lstrcpy(config->Character[0], MoveUp);
	lstrcpy(config->Character[1], MoveDown);
	lstrcpy(config->Character[2], MoveLeft);
	lstrcpy(config->Character[3], MoveRight);
	lstrcpy(config->Character[4], MoveUpLeft);
	lstrcpy(config->Character[5], MoveUpRight);
	lstrcpy(config->Character[6], MoveDownLeft);
	lstrcpy(config->Character[7], MoveDownRight);
	lstrcpy(config->Character[8], CornerTopLeft);
	lstrcpy(config->Character[9], CornerTopRight);
	lstrcpy(config->Character[10], CornerBottomLeft);
	lstrcpy(config->Character[11], CornerBottomRight);
	//MouseGestureTrail
	config->MouseGestureTrail = MouseGestureTrail;
	config->MouseGestureTrailWidth = MouseGestureTrailWidth;
	config->MouseGestureTrailColor = MouseGestureTrailColor;
	config->MouseGestureTrailDrawMode = MouseGestureTrailDrawMode;
	//Target
	for(i=0;i<=(int)Target.size();i++)
	{
		TARGETITEM t;
		int ii;
		if(i==Target.size())
		{
			lstrcpy(t.ClassName, TEXT("Default"));
		}
		else
		{
			t.Number = Target[i].Number;
			t.DefaultThru = Target[i].DefaultThru;
			t.HookType = Target[i].HookType;
			t.WheelRedirect = Target[i].WheelRedirect;
			t.FreeScroll = Target[i].FreeScroll;
			lstrcpy(t.FileName, Target[i].FileName);
			lstrcpy(t.ClassName, Target[i].ClassName);
			lstrcpy(t.ControlID, Target[i].ControlID);
			lstrcpy(t.Comment, Target[i].Comment);
			lstrcpy(t.WindowTitle, Target[i].WindowTitle);
		}
		for(ii=0;ii<(int)Action.size();ii++)
		{
			if((i==Target.size() && Action[ii].TargetNumber==0) ||
				(i!=Target.size() && Action[ii].TargetNumber==Target[i].Number))
			{
				ACTIONITEM a;
				int iii;
				a.Button = Action[ii].Button;
				a.Modifier = Action[ii].Modifier;
				for(iii=0;iii<MAX_GESTURE_LEVEL;iii++)
					a.Move[iii] = Action[ii].Move[iii];
				lstrcpy(a.Comment, Action[ii].Comment);
				for(iii=0;iii<Action[ii].Repeat;iii++)
				{
					COMMANDITEM c;
					c.Case = Action[ii].command[iii].Case;
					c.CommandTarget = Action[ii].command[iii].CommandTarget;
					switch(c.Case)
					{
					case 1:
						c.Wait = Action[ii].command[iii].Wait;
						lstrcpy(c.CommandExt.path, CommandExt[Action[ii].command[iii].Key].path);
						c.CommandExt.Key[0] = CommandExt[Action[ii].command[iii].Key].Key[0];
						c.CommandExt.Key[1] = CommandExt[Action[ii].command[iii].Key].Key[1];
						c.CommandExt.Key[2] = CommandExt[Action[ii].command[iii].Key].Key[2];
						c.CommandExt.Key[3] = CommandExt[Action[ii].command[iii].Key].Key[3];
						lstrcpy(c.CommandExt.Text[0], CommandExt[Action[ii].command[iii].Key].Text[0]);
						lstrcpy(c.CommandExt.Text[1], CommandExt[Action[ii].command[iii].Key].Text[1]);
						lstrcpy(c.CommandExt.Text[2], CommandExt[Action[ii].command[iii].Key].Text[2]);
						lstrcpy(c.CommandExt.Text[3], CommandExt[Action[ii].command[iii].Key].Text[3]);
						break;
					case 10:case 11:case 12:case 20:case 40:case 41:
					case 42:case 55:case 58:case 59:case 50:case 60:case 65:
						c.Wait = Action[ii].command[iii].Wait;
						c.Key = Action[ii].command[iii].Key;
						break;
					case 25:case 30:case 35:
						c.Wait = Action[ii].command[iii].Wait;
						lstrcpy(c.CommandExt.Text[0], CommandExt[Action[ii].command[iii].Key].Text[0]);
						lstrcpy(c.CommandExt.Text[1], CommandExt[Action[ii].command[iii].Key].Text[1]);
						break;
					case 70:case 71:
						c.Wait = Action[ii].command[iii].Wait;
						lstrcpy(c.CommandExt.Text[0], CommandExt[Action[ii].command[iii].Key].Text[0]);
						c.CommandExt.Key[0] = CommandExt[Action[ii].command[iii].Key].Key[0];
						lstrcpy(c.CommandExt.Text[1], CommandExt[Action[ii].command[iii].Key].Text[1]);
						c.CommandExt.Key[1] = CommandExt[Action[ii].command[iii].Key].Key[1];
						lstrcpy(c.CommandExt.Text[2], CommandExt[Action[ii].command[iii].Key].Text[2]);
						c.CommandExt.Key[2] = CommandExt[Action[ii].command[iii].Key].Key[2];
						break;
					case 99:case 95:case 96:
						break;
					default:
						break;
					}
					a.command.push_back(c);
				}
				t.action.push_back(a);
			}
		}
		config->target.push_back(t);
	}
}
//ターゲット設定、アクション設定、コマンド設定を追加
//ターゲット設定が"default"のときは、アクション設定、コマンド設定を追加
void _AddTarget(TARGETITEM* target)
{
	TARGETWINDOW t;
	int i;
	if(!lstrcmpi(target->ClassName, TEXT("Default")))
	{
		t.Number = 0;
	}
	else
	{
		t.Number = target->Number;
		t.DefaultThru = target->DefaultThru;
		t.HookType = target->HookType;
		t.WheelRedirect = target->WheelRedirect;
		t.FreeScroll = target->FreeScroll;
		lstrcpy(t.FileName, target->FileName);
		lstrcpy(t.ClassName, target->ClassName);
		lstrcpy(t.ControlID, target->ControlID);
		lstrcpy(t.Comment, target->Comment);
		lstrcpy(t.WindowTitle, target->WindowTitle);
		Target.push_back(t);
	}
	for(i=0;i<(int)target->action.size();i++)
	{
		EXECUTECOMMAND a;
		int ii;
		a.TargetNumber = t.Number;
		a.Button = target->action[i].Button;
		a.Modifier = target->action[i].Modifier;
		for(ii=0;ii<MAX_GESTURE_LEVEL;ii++)
			a.Move[ii] = target->action[i].Move[ii];
		a.Repeat = (int)target->action[i].command.size();
		lstrcpy(a.Comment, target->action[i].Comment);
		a.BreakPoint = 0;
		for(ii=0;ii<(int)target->action[i].command.size();ii++)
		{
			COMMANDEXTINFO c;
			a.command[ii].Case = target->action[i].command[ii].Case;
			a.command[ii].CommandTarget = target->action[i].command[ii].CommandTarget;
			switch(a.command[ii].Case)
			{
			case 1:
				a.command[ii].Wait = target->action[i].command[ii].Wait;
				a.command[ii].Key = (int)CommandExt.size();
				lstrcpy(c.path, target->action[i].command[ii].CommandExt.path);
				c.Key[0] = target->action[i].command[ii].CommandExt.Key[0];
				c.Key[1] = target->action[i].command[ii].CommandExt.Key[1];
				c.Key[2] = target->action[i].command[ii].CommandExt.Key[2];
				c.Key[3] = target->action[i].command[ii].CommandExt.Key[3];
				lstrcpy(c.Text[0], target->action[i].command[ii].CommandExt.Text[0]);
				lstrcpy(c.Text[1], target->action[i].command[ii].CommandExt.Text[1]);
				lstrcpy(c.Text[2], target->action[i].command[ii].CommandExt.Text[2]);
				lstrcpy(c.Text[3], target->action[i].command[ii].CommandExt.Text[3]);
				CommandExt.push_back(c);
				break;
			case 10:case 11:case 12:case 20:case 40:case 41:
			case 42:case 55:case 58:case 59:case 50:case 60:case 65:
				a.command[ii].Wait = target->action[i].command[ii].Wait;
				a.command[ii].Key = target->action[i].command[ii].Key;
				break;
			case 25:case 30:case 35:
				a.command[ii].Wait = target->action[i].command[ii].Wait;
				a.command[ii].Key = (int)CommandExt.size();
				lstrcpy(c.Text[0], target->action[i].command[ii].CommandExt.Text[0]);
				lstrcpy(c.Text[1], target->action[i].command[ii].CommandExt.Text[1]);
				CommandExt.push_back(c);
				break;
			case 70:case 71:
				a.command[ii].Wait = target->action[i].command[ii].Wait;
				a.command[ii].Key = (int)CommandExt.size();
				lstrcpy(c.Text[0], target->action[i].command[ii].CommandExt.Text[0]);
				c.Key[0] = target->action[i].command[ii].CommandExt.Key[0];
				lstrcpy(c.Text[1], target->action[i].command[ii].CommandExt.Text[1]);
				c.Key[1] = target->action[i].command[ii].CommandExt.Key[1];
				lstrcpy(c.Text[2], target->action[i].command[ii].CommandExt.Text[2]);
				c.Key[2] = target->action[i].command[ii].CommandExt.Key[2];
				CommandExt.push_back(c);
				break;
			case 99:case 95:case 96:
				break;
			default:
				break;
			}
		}
		Action.push_back(a);
	}
}
//CONFIGからグローバル変数を作成する
void _SetConfig(CONFIG* config)
{
	int i;

	CommandExt.clear();
	Target.clear();
	Action.clear();

	//Main
	ShowTaskTray = config->ShowTaskTray;
	GestureDirection = config->GestureDirection;
	TimeOutType = config->TimeOutType;
	Default.HookType = config->DefaultHookType;
	Priority = config->Priority;
	ClickWait = config->ClickWait;
	WheelRedirect = config->WheelRedirect;
	WheelActive = config->WheelActive;
	KuruKuruFlag = config->KuruKuruFlag;
	Default.FreeScroll = config->DefaultFreeScroll;
	ScrollSensitivity = config->ScrollSensitivity;
	KuruKuruTimeOut = config->KuruKuruTimeOut;
	CircleRate = config->CircleRate;
	Default.WheelRedirect = config->DefaultWheelRedirect;
	CheckInterval = config->CheckInterval;
	MoveQuantity = config->MoveQuantity;
	GestureTimeOut = config->GestureTimeOut;
	GestureStartQuantity = config->GestureStartQuantity;
	CornerTime = config->CornerTime;
	CornerPosX = config->CornerPosX;
	CornerPosY = config->CornerPosY;
	CursorChange = config->CursorChange;
	lstrcpy(GestureCursor, config->GestureCursor);
	lstrcpy(WaitCursor, config->WaitCursor);
	//Navi
	NaviType = config->NaviType;
	MauSujiNavi = config->MauSujiNavi;
	NaviDeleteTime = config->NaviDeleteTime;
	NaviWidth = config->NaviWidth;
	TipLocate = config->TipLocate;
	lstrcpy(NaviTitle, config->NaviTitle);
	lf = config->NaviFont;
	MainColor = config->MainColor;
	SubColor = config->SubColor;
	BackColor = config->BackColor;
	FrameColor = config->FrameColor;
	//Character
	lstrcpy(MoveUp, config->Character[0]);
	lstrcpy(MoveDown, config->Character[1]);
	lstrcpy(MoveLeft, config->Character[2]);
	lstrcpy(MoveRight, config->Character[3]);
	lstrcpy(MoveUpLeft, config->Character[4]);
	lstrcpy(MoveUpRight, config->Character[5]);
	lstrcpy(MoveDownLeft, config->Character[6]);
	lstrcpy(MoveDownRight, config->Character[7]);
	lstrcpy(CornerTopLeft, config->Character[8]);
	lstrcpy(CornerTopRight, config->Character[9]);
	lstrcpy(CornerBottomLeft, config->Character[10]);
	lstrcpy(CornerBottomRight, config->Character[11]);
	//MouseGestureTrail
	MouseGestureTrail = config->MouseGestureTrail;
	MouseGestureTrailWidth = config->MouseGestureTrailWidth;
	MouseGestureTrailColor = config->MouseGestureTrailColor;
	MouseGestureTrailDrawMode = config->MouseGestureTrailDrawMode;
	//Target
	for(i=0;i<(int)config->target.size();i++)
		_AddTarget(&config->target[i]);
}
//iniファイル読み込み
void IniRead(LPTSTR IniName)
{
	TCHAR ProfileSection[100], ProfileKey[100];
	int i, j;

	CommandExt.clear();
	Target.clear();
	Action.clear();

	lstrcpy(ProfileSection, TEXT("Main"));
	ShowTaskTray = GetPrivateProfileInt(ProfileSection, TEXT("TaskTrayIcon"), TRUE, IniName);
	GestureDirection = GetPrivateProfileInt(ProfileSection, TEXT("GestureDirection"), 4, IniName);
	TimeOutType = GetPrivateProfileInt(ProfileSection, TEXT("TimeOutType"), FALSE, IniName);
	Default.HookType = GetPrivateProfileInt(ProfileSection, TEXT("DefaultHookType"), 1, IniName);
	Priority = GetPrivateProfileInt(ProfileSection, TEXT("Priority"), 0x0020, IniName);
	ClickWait = GetPrivateProfileInt(ProfileSection, TEXT("ClickWait"), 50, IniName);
	WheelRedirect = GetPrivateProfileInt(ProfileSection, TEXT("WheelRedirect"), FALSE, IniName);
	WheelActive = GetPrivateProfileInt(ProfileSection, TEXT("WheelActive"), FALSE, IniName);
	KuruKuruFlag = GetPrivateProfileInt(ProfileSection, TEXT("KuruKuru"), FALSE, IniName);
	Default.FreeScroll = GetPrivateProfileInt(ProfileSection, TEXT("FreeScroll"), 0, IniName);
	ScrollSensitivity = GetPrivateProfileInt(ProfileSection, TEXT("ScrollSensitivity"), 30, IniName);
	KuruKuruTimeOut = GetPrivateProfileInt(ProfileSection, TEXT("KuruKuruTimeOut"), 200, IniName);
	CircleRate = GetPrivateProfileInt(ProfileSection, TEXT("CircleRate"), 60, IniName);
	Default.WheelRedirect = GetPrivateProfileInt(ProfileSection, TEXT("DefaultWheelRedirect"), WheelRedirect, IniName);
	CheckInterval = GetPrivateProfileInt(ProfileSection, TEXT("CheckInterval"), 50, IniName);
	MoveQuantity = GetPrivateProfileInt(ProfileSection, TEXT("GestureSensitivity"), 50, IniName);
	GestureTimeOut = GetPrivateProfileInt(ProfileSection, TEXT("GestureTimeOut"), 500, IniName);
	GestureStartQuantity = GetPrivateProfileInt(ProfileSection, TEXT("GestureStartSensitivity"), 30, IniName);
	CornerTime = GetPrivateProfileInt(ProfileSection, TEXT("CornerSensitivity"), 500, IniName);
	CornerPosX = GetPrivateProfileInt(ProfileSection, TEXT("CornerAreaX"), 70, IniName);
	CornerPosY = GetPrivateProfileInt(ProfileSection, TEXT("CornerAreaY"), 70, IniName);
	CursorChange = GetPrivateProfileInt(ProfileSection, TEXT("CursorChange"), TRUE, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("GestureCursor"), TEXT(""), GestureCursor, _MAX_PATH, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("WaitCursor"), TEXT(""), WaitCursor, _MAX_PATH, IniName);

	lstrcpy(ProfileSection, TEXT("Navi"));
	NaviType = GetPrivateProfileInt(ProfileSection, TEXT("NaviType"), NaviTypeFloat, IniName);
	MauSujiNavi.x = GetPrivateProfileInt(ProfileSection, TEXT("NaviPositionX"), 0, IniName);
	MauSujiNavi.y = GetPrivateProfileInt(ProfileSection, TEXT("NaviPositionY"), 0, IniName);
	NaviDeleteTime = GetPrivateProfileInt(ProfileSection, TEXT("NaviDeleteTime"), 1000, IniName);
	NaviWidth = GetPrivateProfileInt(ProfileSection, TEXT("NaviWidth"), 0, IniName);
	TipLocate.x = GetPrivateProfileInt(ProfileSection, TEXT("TipLocateX"), 0, IniName);
	TipLocate.y = GetPrivateProfileInt(ProfileSection, TEXT("TipLocateY"), 20, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("NaviTitle"), TEXT("マウ筋"), NaviTitle, _MAX_PATH, IniName);
	lf.lfHeight = GetPrivateProfileInt(ProfileSection, TEXT("FontHeight"), 12, IniName);
	lf.lfWeight = GetPrivateProfileInt(ProfileSection, TEXT("FontWeight"), 0, IniName);
	lf.lfItalic = GetPrivateProfileInt(ProfileSection, TEXT("FontItalic"), 0, IniName);
	lf.lfCharSet = GetPrivateProfileInt(ProfileSection, TEXT("FontCharSet"), 128, IniName);
	MainColor = GetPrivateProfileInt(ProfileSection, TEXT("MainTextColor"), 0, IniName);
	SubColor = GetPrivateProfileInt(ProfileSection, TEXT("SubTextColor"), 8421504, IniName);
	BackColor = GetPrivateProfileInt(ProfileSection, TEXT("BackgroundColor"), 12582911, IniName);
	FrameColor = GetPrivateProfileInt(ProfileSection, TEXT("FrameColor"), 0, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("FontName"), TEXT("MS UI Gothic"), lf.lfFaceName, 69, IniName);

	lstrcpy(ProfileSection, TEXT("Character"));
	GetPrivateProfileString(ProfileSection, TEXT("Up"), TEXT("↑"), MoveUp, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("Down"), TEXT("↓"), MoveDown, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("Left"), TEXT("←"), MoveLeft, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("Right"), TEXT("→"), MoveRight, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("UpLeft"), TEXT("\x2196"), MoveUpLeft, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("UpRight"), TEXT("\x2197"), MoveUpRight, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("DownLeft"), TEXT("\x2199"), MoveDownLeft, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("DownRight"), TEXT("\x2198"), MoveDownRight, 3, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("CornerTopLeft"), TEXT("↑←"), CornerTopLeft, 5, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("CornerTopRight"), TEXT("↑→"), CornerTopRight, 5, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("CornerBottomLeft"), TEXT("↓←"), CornerBottomLeft, 5, IniName);
	GetPrivateProfileString(ProfileSection, TEXT("CornerBottomRight"), TEXT("↓→"), CornerBottomRight, 5, IniName);

	lstrcpy(ProfileSection, TEXT("MouseGestureTrail"));
	MouseGestureTrail = GetPrivateProfileInt(ProfileSection, TEXT("Enable"), TRUE, IniName);
	MouseGestureTrailWidth = GetPrivateProfileInt(ProfileSection, TEXT("Width"), 3, IniName);
	MouseGestureTrailColor = GetPrivateProfileInt(ProfileSection, TEXT("Color"), RGB(255, 0, 0), IniName);
	MouseGestureTrailDrawMode = GetPrivateProfileInt(ProfileSection, TEXT("DrawMode"), 0, IniName);

	lstrcpy(ProfileSection, TEXT("Target"));
	i = 0;
	while(1)
	{
		TARGETWINDOW t = {0};

		wsprintf(ProfileKey, TEXT("Number_%d"), i);
		t.Number = GetPrivateProfileInt(ProfileSection, ProfileKey, -1, IniName);
		wsprintf(ProfileKey, TEXT("Succession_%d"), i);
		t.DefaultThru = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
		wsprintf(ProfileKey, TEXT("HookType_%d"), i);
		t.HookType = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
		wsprintf(ProfileKey, TEXT("WheelRedirect_%d"), i);
		t.WheelRedirect = GetPrivateProfileInt(ProfileSection, ProfileKey, Default.WheelRedirect, IniName);
		wsprintf(ProfileKey, TEXT("FreeScroll_%d"), i);
		t.FreeScroll = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
		wsprintf(ProfileKey, TEXT("FileName%d"), i);
		GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), t.FileName, _MAX_PATH, IniName);
		wsprintf(ProfileKey, TEXT("WindowTitle%d"), i);
		GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), t.WindowTitle, 256, IniName);
		wsprintf(ProfileKey, TEXT("ClassName%d"), i);
		GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), t.ClassName, MAX_CLASSNAME, IniName);
		wsprintf(ProfileKey, TEXT("ControlID%d"), i);
		GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), t.ControlID, 19, IniName);
		wsprintf(ProfileKey, TEXT("Comment%d"), i);
		GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), t.Comment, MAX_COMMENT, IniName);
		if(t.Number >= 0)
		{
			Target.push_back(t);
			i++;
			if(i >= MAX_TARGET_DATA)
				break;
		}
		else
		{
			break;
		}
	}

	lstrcpy(ProfileSection, TEXT("Action"));
	i = 0;
	while(1)
	{
		EXECUTECOMMAND a = {0};

		wsprintf(ProfileKey, TEXT("TargetNumber%d"), i);
		a.TargetNumber = GetPrivateProfileInt(ProfileSection, ProfileKey, -1, IniName);
		wsprintf(ProfileKey, TEXT("Gesture%d"), i);
		a.Button = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
		wsprintf(ProfileKey, TEXT("Modifier%d"), i);
		a.Modifier = GetPrivateProfileInt(ProfileSection, ProfileKey, MODIFIER_DISABLE, IniName);
		for(j = 0; j < MAX_GESTURE_LEVEL; j++)
		{
			wsprintf(ProfileKey, TEXT("Gesture%d_%d"), i, j);
			a.Move[j] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
		}
		if(LeftyMode)
		{
			if(a.Button == BUTTON_L || a.Button == BUTTON_R)
				a.Button = BUTTON_L + BUTTON_R - a.Button;
			if(a.Move[0] > 0 && (a.Move[0] == BUTTON_L || a.Move[0] == BUTTON_R))
				a.Move[0] = BUTTON_L + BUTTON_R - a.Move[0];
		}
		wsprintf(ProfileKey, TEXT("Comment%d"), i, j);
		GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), a.Comment, MAX_COMMENT, IniName);

		j = -1;
		do{
			COMMANDEXTINFO c = {0};

			j++;
			wsprintf(ProfileKey, TEXT("Case%d_%d"), i, j);
			a.command[j].Case = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
			wsprintf(ProfileKey, TEXT("CommandTarget%d_%d"), i, j);
			a.command[j].CommandTarget = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
			if(a.command[j].Case < 0)
			{
				a.command[j].CommandTarget = -1;
				a.command[j].Case = abs(a.command[j].Case);
			}
			switch( a.command[j].Case )
			{
			case 1:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				a.command[j].Wait = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				a.command[j].Key = (LONG)CommandExt.size();
				wsprintf(ProfileKey, TEXT("path%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.path, _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("value1%d_%d"), i, j);
				c.Key[0] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("value2%d_%d"), i, j);
				c.Key[1] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("value3%d_%d"), i, j);
				c.Key[2] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("value4%d_%d"), i, j);
				c.Key[3] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("text1%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[0], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("text2%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[1], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("text3%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[2], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("text4%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[3], _MAX_PATH, IniName);
				CommandExt.push_back(c);
				break;
			case 25:
			case 30:
			case 35:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				a.command[j].Wait = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				a.command[j].Key = (LONG)CommandExt.size();
				wsprintf(ProfileKey, TEXT("MainText%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[0], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("SubText%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[1], _MAX_PATH, IniName);
				CommandExt.push_back(c);
				break;
			case 65:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				a.command[j].Wait = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("Key%d_%d"), i, j);
				a.command[j].Key = GetPrivateProfileInt(ProfileSection, ProfileKey, 65546, IniName);
				break;
			case 70:
			case 71:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				a.command[j].Wait = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				a.command[j].Key = (LONG)CommandExt.size();
				wsprintf(ProfileKey, TEXT("Message%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[0], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("MessageType%d_%d"), i, j);
				c.Key[0] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("wParam%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[1], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("wParamType%d_%d"), i, j);
				c.Key[1] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("lParam%d_%d"), i, j);
				GetPrivateProfileString(ProfileSection, ProfileKey, TEXT(""), c.Text[2], _MAX_PATH, IniName);
				wsprintf(ProfileKey, TEXT("lParamType%d_%d"), i, j);
				c.Key[2] = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				CommandExt.push_back(c);
				break;
			case 99:
			case 95:
			case 96:
				break;
			default:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				a.command[j].Wait = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				wsprintf(ProfileKey, TEXT("Key%d_%d"), i, j);
				a.command[j].Key = GetPrivateProfileInt(ProfileSection, ProfileKey, 0, IniName);
				break;
			}
		}while(j < MAX_ACTION_REPEAT && a.command[j].Case != 0);

		a.Repeat = j;
		a.BreakPoint = 0;
		if(a.TargetNumber >= 0)
		{
			Action.push_back(a);
			i++;
			if(i >= MAX_ACTION_DATA)
				break;
		}
		else
		{
			break;
		}
	}

	SortAction();
}
//iniファイル書き込み
void IniWrite(LPTSTR IniName)
{
	TCHAR ProfileSection[100], ProfileKey[100];
	int i, j;

	lstrcpy(ProfileSection, TEXT("Main"));
	WritePrivateProfileInt(ProfileSection, TEXT("IniVersion"), INI_VERSION, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("TaskTrayIcon"), ShowTaskTray, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("GestureDirection"), GestureDirection, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("TimeOutType"), TimeOutType, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("DefaultHookType"), Default.HookType, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("Priority"), Priority, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("ClickWait"), ClickWait, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("WheelRedirect"), WheelRedirect, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("WheelActive"), WheelActive, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("KuruKuru"), KuruKuruFlag, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("FreeScroll"), Default.FreeScroll, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("ScrollSensitivity"), ScrollSensitivity, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("KuruKuruTimeOut"), KuruKuruTimeOut, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("CircleRate"), CircleRate, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("DefaultWheelRedirect"), Default.WheelRedirect, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("CheckInterval"), CheckInterval, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("GestureSensitivity"), MoveQuantity, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("GestureTimeOut"), GestureTimeOut, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("GestureStartSensitivity"), GestureStartQuantity, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("CornerSensitivity"), CornerTime, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("CornerAreaX"), CornerPosX, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("CornerAreaY"), CornerPosY, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("CursorChange"), CursorChange, IniName);
	if(GestureCursor[0])
		WritePrivateProfileString(ProfileSection, TEXT("GestureCursor"), GestureCursor, IniName);
	if(WaitCursor[0])
		WritePrivateProfileString(ProfileSection, TEXT("WaitCursor"), WaitCursor, IniName);

	lstrcpy(ProfileSection, TEXT("Navi"));
	WritePrivateProfileInt(ProfileSection, TEXT("NaviType"), NaviType, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("NaviPositionX"), MauSujiNavi.x, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("NaviPositionY"), MauSujiNavi.y, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("NaviDeleteTime"), NaviDeleteTime, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("NaviWidth"), NaviWidth, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("TipLocateX"), TipLocate.x, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("TipLocateY"), TipLocate.y, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("NaviTitle"), NaviTitle, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("FontHeight"), lf.lfHeight, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("FontWeight"), lf.lfWeight, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("FontItalic"), lf.lfItalic, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("FontCharSet"), lf.lfCharSet, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("MainTextColor"), MainColor, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("SubTextColor"), SubColor, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("BackgroundColor"), BackColor, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("FrameColor"), FrameColor, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("FontName"), lf.lfFaceName, IniName);

	lstrcpy(ProfileSection, TEXT("Character"));
	WritePrivateProfileString(ProfileSection, TEXT("Up"), MoveUp, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("Down"), MoveDown, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("Left"), MoveLeft, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("Right"), MoveRight, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("UpLeft"), MoveUpLeft, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("UpRight"), MoveUpRight, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("DownLeft"), MoveDownLeft, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("DownRight"), MoveDownRight, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("CornerTopLeft"), CornerTopLeft, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("CornerTopRight"), CornerTopRight, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("CornerBottomLeft"), CornerBottomLeft, IniName);
	WritePrivateProfileString(ProfileSection, TEXT("CornerBottomRight"), CornerBottomRight, IniName);

	lstrcpy(ProfileSection, TEXT("MouseGestureTrail"));
	WritePrivateProfileInt(ProfileSection, TEXT("Enable"), MouseGestureTrail, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("Width"), MouseGestureTrailWidth, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("Color"), MouseGestureTrailColor, IniName);
	WritePrivateProfileInt(ProfileSection, TEXT("DrawMode"), MouseGestureTrailDrawMode, IniName);

	lstrcpy(ProfileSection, TEXT("Target"));
	WritePrivateProfileString(ProfileSection, NULL, NULL, IniName);
	for(i = 0; i < (int)Target.size(); i++)
	{
		wsprintf(ProfileKey, TEXT("Number_%d"), i);
		WritePrivateProfileInt(ProfileSection, ProfileKey, Target[i].Number, IniName);
		wsprintf(ProfileKey, TEXT("Succession_%d"), i);
		WritePrivateProfileInt(ProfileSection, ProfileKey, Target[i].DefaultThru, IniName);
		wsprintf(ProfileKey, TEXT("HookType_%d"), i);
		WritePrivateProfileInt(ProfileSection, ProfileKey, Target[i].HookType, IniName);
		wsprintf(ProfileKey, TEXT("WheelRedirect_%d"), i);
		WritePrivateProfileInt(ProfileSection, ProfileKey, Target[i].WheelRedirect, IniName);
		wsprintf(ProfileKey, TEXT("FreeScroll_%d"), i);
		WritePrivateProfileInt(ProfileSection, ProfileKey, Target[i].FreeScroll, IniName);
		if(Target[i].FileName[0])
		{
			wsprintf(ProfileKey, TEXT("FileName%d"), i);
			WritePrivateProfileString(ProfileSection, ProfileKey, Target[i].FileName, IniName);
		}
		if(Target[i].WindowTitle[0])
		{
			wsprintf(ProfileKey, TEXT("WindowTitle%d"), i);
			WritePrivateProfileString(ProfileSection, ProfileKey, Target[i].WindowTitle, IniName);
		}
		if(Target[i].ClassName[0])
		{
			wsprintf(ProfileKey, TEXT("ClassName%d"), i);
			WritePrivateProfileString(ProfileSection, ProfileKey, Target[i].ClassName, IniName);
		}
		if(Target[i].ControlID[0])
		{
			wsprintf(ProfileKey, TEXT("ControlID%d"), i);
			WritePrivateProfileString(ProfileSection, ProfileKey, Target[i].ControlID, IniName);
		}
		if(Target[i].Comment[0])
		{
			wsprintf(ProfileKey, TEXT("Comment%d"), i);
			WritePrivateProfileString(ProfileSection, ProfileKey, Target[i].Comment, IniName);
		}
	}

	lstrcpy(ProfileSection, TEXT("Action"));
	WritePrivateProfileString(ProfileSection, NULL, NULL, IniName);
	for(i = 0; i < (int)Action.size(); i++)
	{
		wsprintf(ProfileKey, TEXT("TargetNumber%d"), i);
		WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].TargetNumber, IniName);
		wsprintf(ProfileKey, TEXT("Gesture%d"), i);
		if(LeftyMode && (Action[i].Button == BUTTON_L || Action[i].Button == BUTTON_R))
			WritePrivateProfileInt(ProfileSection, ProfileKey, BUTTON_L + BUTTON_R - Action[i].Button, IniName);
		else
			WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].Button, IniName);
		if(!(Action[i].Modifier & MODIFIER_DISABLE))
		{
			wsprintf(ProfileKey, TEXT("Modifier%d"), i);
			WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].Modifier, IniName);
		}
		wsprintf(ProfileKey, TEXT("Gesture%d_%d"), i, 0);
		if(LeftyMode && Action[i].Move[0] > 0 && (Action[i].Move[0] == BUTTON_L || Action[i].Move[0] == BUTTON_R))
			WritePrivateProfileInt(ProfileSection, ProfileKey, BUTTON_L + BUTTON_R - Action[i].Move[0], IniName);
		else
			WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].Move[0], IniName);
		j = 1;
		while(j < MAX_GESTURE_LEVEL && Action[i].Move[j] > 0)
		{
			wsprintf(ProfileKey, TEXT("Gesture%d_%d"), i, j);
			WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].Move[j], IniName);
			j++;
		}
		if(Action[i].Comment[0])
		{
			wsprintf(ProfileKey, TEXT("Comment%d"), i, j);
			WritePrivateProfileString(ProfileSection, ProfileKey, Action[i].Comment, IniName);
		}
		for(j = 0; j < Action[i].Repeat; j++)
		{
			wsprintf(ProfileKey, TEXT("Case%d_%d"), i, j);
			WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].Case, IniName);
			wsprintf(ProfileKey, TEXT("CommandTarget%d_%d"), i, j);
			WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].CommandTarget, IniName);
			switch( Action[i].command[j].Case )
			{
			case 1:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].Wait, IniName);
				wsprintf(ProfileKey, TEXT("path%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].path, IniName);
				wsprintf(ProfileKey, TEXT("value1%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[0], IniName);
				wsprintf(ProfileKey, TEXT("value2%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[1], IniName);
				wsprintf(ProfileKey, TEXT("value3%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[2], IniName);
				wsprintf(ProfileKey, TEXT("value4%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[3], IniName);
				wsprintf(ProfileKey, TEXT("text1%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[0], IniName);
				wsprintf(ProfileKey, TEXT("text2%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[1], IniName);
				wsprintf(ProfileKey, TEXT("text3%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[2], IniName);
				wsprintf(ProfileKey, TEXT("text4%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[3], IniName);
				break;
			case 25:
			case 30:
			case 35:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].Wait, IniName);
				if(CommandExt[Action[i].command[j].Key].Text[0][0])
				{
					wsprintf(ProfileKey, TEXT("MainText%d_%d"), i, j);
					WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[0], IniName);
				}
				if(CommandExt[Action[i].command[j].Key].Text[1][0])
				{
					wsprintf(ProfileKey, TEXT("SubText%d_%d"), i, j);
					WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[1], IniName);
				}
				break;
			case 70:
			case 71:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].Wait, IniName);
				wsprintf(ProfileKey, TEXT("Message%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[0], IniName);
				wsprintf(ProfileKey, TEXT("MessageType%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[0], IniName);
				wsprintf(ProfileKey, TEXT("wParam%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[1], IniName);
				wsprintf(ProfileKey, TEXT("wParamType%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[1], IniName);
				wsprintf(ProfileKey, TEXT("lParam%d_%d"), i, j);
				WritePrivateProfileString(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Text[2], IniName);
				wsprintf(ProfileKey, TEXT("lParamType%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, CommandExt[Action[i].command[j].Key].Key[2], IniName);
				break;
			case 99:
			case 95:
			case 96:
				break;
			default:
				wsprintf(ProfileKey, TEXT("Wait%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].Wait, IniName);
				wsprintf(ProfileKey, TEXT("Key%d_%d"), i, j);
				WritePrivateProfileInt(ProfileSection, ProfileKey, Action[i].command[j].Key, IniName);
				break;
			}
		}
	}
}

void LoadConfig(CONFIG* config)
{
	TCHAR IniName[MAX_PATH];
	TCHAR ExePath[MAX_PATH];

	GetModuleFileName(NULL, ExePath, MAX_PATH);
	PathRemoveFileSpec(ExePath);

	if(szConfigFilePath[0])
		lstrcpy(IniName, szConfigFilePath);
	else
		wsprintf(IniName, TEXT("%s\\config.xml"), ExePath);
	if(PathFileExists(IniName))
	{
		ConfigReadXmlFile(IniName, config, NULL);
	}
	else
	{
		HGLOBAL hResData;
		LPWSTR xml;

		hResData = LoadResource(NULL, FindResource(NULL, MAKEINTRESOURCE(IDR_CONFIGXML), TEXT("XMLFILE")));
		xml = (LPTSTR)LockResource(hResData);
		ConfigReadXmlString(xml, config, NULL);
	}
}

void SaveConfig(CONFIG* config)
{
	TCHAR IniName[MAX_PATH];
	TCHAR ExePath[MAX_PATH];

	GetModuleFileName(NULL, ExePath, MAX_PATH);
	PathRemoveFileSpec(ExePath);

	if(szConfigFilePath[0])
		lstrcpy(IniName, szConfigFilePath);
	else
		wsprintf(IniName, TEXT("%s\\config.xml"), ExePath);
	ConfigWriteXmlFile(config, IniName);
}

void GestureToString(int GestureCode, LPTSTR lpszText, int nSize)
{
	TCHAR szText[1024];

	switch(GestureCode)
	{
	case BUTTON_L:
		lstrcpyn(lpszText, TEXT("L "), nSize);
		break;
	case BUTTON_M:
		lstrcpyn(lpszText, TEXT("M "), nSize);
		break;
	case BUTTON_R:
		lstrcpyn(lpszText, TEXT("R "), nSize);
		break;
	case BUTTON_X1:
		lstrcpyn(lpszText, TEXT("X1 "), nSize);
		break;
	case BUTTON_X2:
		lstrcpyn(lpszText, TEXT("X2 "), nSize);
		break;
	case MOVE_UP:
		lstrcpyn(lpszText, MoveUp, nSize);
		break;
	case MOVE_DOWN:
		lstrcpyn(lpszText, MoveDown, nSize);
		break;
	case MOVE_LEFT:
		lstrcpyn(lpszText, MoveLeft, nSize);
		break;
	case MOVE_RIGHT:
		lstrcpyn(lpszText, MoveRight, nSize);
		break;
	case MOVE_UPLEFT:
		lstrcpyn(lpszText, MoveUpLeft, nSize);
		break;
	case MOVE_UPRIGHT:
		lstrcpyn(lpszText, MoveUpRight, nSize);
		break;
	case MOVE_DOWNLEFT:
		lstrcpyn(lpszText, MoveDownLeft, nSize);
		break;
	case MOVE_DOWNRIGHT:
		lstrcpyn(lpszText, MoveDownRight, nSize);
		break;
	case WHEEL_UP:
		wsprintf(szText, TEXT("Wheel %s"), MoveUp);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case WHEEL_DOWN:
		wsprintf(szText, TEXT("Wheel %s"), MoveDown);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case WHEEL_LEFT:
		wsprintf(szText, TEXT("Wheel %s"), MoveLeft);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case WHEEL_RIGHT:
		wsprintf(szText, TEXT("Wheel %s"), MoveRight);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_TOP_A:
		wsprintf(szText, TEXT("Corner %sA"), MoveUp);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_BOTTOM_A:
		wsprintf(szText, TEXT("Corner %sA"), MoveDown);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_LEFT_A:
		wsprintf(szText, TEXT("Corner %sA"), MoveLeft);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_RIGHT_A:
		wsprintf(szText, TEXT("Corner %sA"), MoveRight);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_TOP_B:
		wsprintf(szText, TEXT("Corner %sB"), MoveUp);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_BOTTOM_B:
		wsprintf(szText, TEXT("Corner %sB"), MoveDown);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_LEFT_B:
		wsprintf(szText, TEXT("Corner %sB"), MoveLeft);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_RIGHT_B:
		wsprintf(szText, TEXT("Corner %sB"), MoveRight);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_TOP_C:
		wsprintf(szText, TEXT("Corner %sC"), MoveUp);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_BOTTOM_C:
		wsprintf(szText, TEXT("Corner %sC"), MoveDown);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_LEFT_C:
		wsprintf(szText, TEXT("Corner %sC"), MoveLeft);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_RIGHT_C:
		wsprintf(szText, TEXT("Corner %sC"), MoveRight);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_TOPLEFT:
		wsprintf(szText, TEXT("Corner %s"), CornerTopLeft);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_TOPRIGHT:
		wsprintf(szText, TEXT("Corner %s"), CornerTopRight);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_BOTTOMLEFT:
		wsprintf(szText, TEXT("Corner %s"), CornerBottomLeft);
		lstrcpyn(lpszText, szText, nSize);
		break;
	case CORNER_BOTTOMRIGHT:
		wsprintf(szText, TEXT("Corner %s"), CornerBottomRight);
		lstrcpyn(lpszText, szText, nSize);
		break;
	default:
		lstrcpyn(lpszText, TEXT(" "), nSize);
		break;
	}
}

LPTSTR GetGestureString(BYTE* Modifier, int* Button, int* Move, LPTSTR lpszText, int nSize)
{
	TCHAR szText[50] = TEXT("");
	TCHAR szMove[12];
	int i;

	if(Modifier)
		get_modifiername(*Modifier, szText, 50);
	if(Button)
	{
		GestureToString(*Button, szMove, 12);
		lstrcat( szText, szMove );
	}
	if(Move)
	{
		for(i=0;i<MAX_GESTURE_LEVEL;i++)
		{
			if(Move[i])
			{
				GestureToString(Move[i], szMove, 12);
				lstrcat(szText, szMove);
			}
		}
	}
	lstrcpyn(lpszText, szText, nSize);
	return lpszText;
}

void ExecuteSendKey(int Case, LONG SendKey, int ClickWait)
{
	UINT modifiers = ((HIBYTE(SendKey) & HOTKEYF_SHIFT) ? MOD_SHIFT : 0) |
		((HIBYTE(SendKey) & HOTKEYF_CONTROL) ? MOD_CONTROL : 0) |
		((HIBYTE(SendKey) & HOTKEYF_ALT) ? MOD_ALT : 0) |
		((HIBYTE(SendKey) & HOTKEYF_WIN) ? MOD_WIN : 0);
	UINT virtkey = LOBYTE(SendKey);
	UINT Extend = 0;

	switch(virtkey)
	{
	case VK_INSERT:
	case VK_DELETE:
	case VK_HOME:
	case VK_END:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_LEFT:
	case VK_UP:
	case VK_RIGHT:
	case VK_DOWN:
	case VK_NUMLOCK:
	case VK_CANCEL:
	case VK_SNAPSHOT:
	case VK_SEPARATOR:
	case VK_DIVIDE:
		Extend = KEYEVENTF_EXTENDEDKEY;
		break;
	}

	if(Case == 10 || Case == 11)
	{
		//組み合わせキー押下
		if(modifiers & MOD_SHIFT)
			keybd_event(VK_SHIFT, (BYTE)MapVirtualKey(VK_SHIFT, 0), 0, 0);
		if(modifiers & MOD_CONTROL)
			keybd_event(VK_CONTROL, (BYTE)MapVirtualKey(VK_CONTROL, 0), 0, 0);
		if(modifiers & MOD_ALT)
			keybd_event(VK_MENU, (BYTE)MapVirtualKey(VK_MENU, 0), 0, 0);
		if(modifiers & MOD_WIN)
			keybd_event(VK_LWIN, (BYTE)MapVirtualKey(VK_LWIN, 0), 0, 0);

		switch(virtkey)
		{
		case VK_LBUTTON:
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			break;
		case VK_RBUTTON:
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
			break;
		case VK_MBUTTON:
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, 0);
			break;
		case VK_XBUTTON1:
			mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON1, 0);
			break;
		case VK_XBUTTON2:
			mouse_event(MOUSEEVENTF_XDOWN, 0, 0, XBUTTON2, 0);
			break;
		default:
			keybd_event((BYTE)virtkey, (BYTE)MapVirtualKey(virtkey, 0), Extend, 0);
			break;
		}
		Sleep(ClickWait);
	}

	if(Case == 10 || Case == 12)
	{
		switch(virtkey)
		{
		case VK_LBUTTON:
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
			break;
		case VK_RBUTTON:
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
			break;
		case VK_MBUTTON:
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, 0);
			break;
		case VK_XBUTTON1:
			mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON1, 0);
			break;
		case VK_XBUTTON2:
			mouse_event(MOUSEEVENTF_XUP, 0, 0, XBUTTON2, 0);
			break;
		default:
			keybd_event((BYTE)virtkey, (BYTE)MapVirtualKey(virtkey, 0), Extend | KEYEVENTF_KEYUP, 0);
			break;
		}

		//組み合わせキー解除
		if(modifiers & MOD_WIN)
			keybd_event(VK_LWIN, (BYTE)MapVirtualKey(VK_LWIN, 0), KEYEVENTF_KEYUP, 0);
		if(modifiers & MOD_ALT)
			keybd_event(VK_MENU, (BYTE)MapVirtualKey(VK_MENU, 0), KEYEVENTF_KEYUP, 0);
		if(modifiers & MOD_CONTROL)
			keybd_event(VK_CONTROL, (BYTE)MapVirtualKey(VK_CONTROL, 0), KEYEVENTF_KEYUP, 0);
		if(modifiers & MOD_SHIFT)
			keybd_event(VK_SHIFT, (BYTE)MapVirtualKey(VK_SHIFT, 0), KEYEVENTF_KEYUP, 0);
		Sleep(ClickWait);
	}
}

HWND GetDefViewWindowHandle()
{
	HWND hwnd = NULL;
	HWND hwndProgman;

	hwndProgman = FindWindowEx(NULL, NULL, TEXT("Progman"), TEXT("Program Manager"));
	if(hwndProgman)
	{
		HWND hwndDefView;

		hwndDefView = FindWindowEx(hwndProgman, NULL, TEXT("SHELLDLL_DefView"), NULL);
		if(hwndDefView)
		{
			hwnd = hwndDefView;
		}
		else
		{
			HWND hWorkerW = NULL;

			hWorkerW = FindWindowEx(NULL, NULL, TEXT("WorkerW"), NULL);
			while(hWorkerW!=NULL)
			{
				TCHAR szFileName[MAX_PATH];

				GetFileNameFromHwnd(hWorkerW, szFileName, MAX_PATH);
				if( !lstrcmpi(PathFindFileName(szFileName), TEXT("explorer.exe")) )
				{
					hwndDefView = FindWindowEx(hWorkerW, NULL, TEXT("SHELLDLL_DefView"), NULL);
					if(hwndDefView)
					{
						hwnd = hwndDefView;
						break;
					}
				}
				hWorkerW = FindWindowEx(NULL, hWorkerW, TEXT("WorkerW"), NULL);
			}
		}
	}
	return hwnd;
}

HWND GetDesktopWindowHandle()
{
	HWND hwnd = NULL;
	HWND hwndDefView;

	hwndDefView = GetDefViewWindowHandle();
	if(hwndDefView)
	{
		hwnd = FindWindowEx(hwndDefView, NULL, TEXT("SysListView32"), NULL);
		if(hwnd)
		{
			if( !(GetWindowLongPtr(hwnd, GWL_STYLE) & WS_VISIBLE) )
			{
				hwnd = FindWindowEx(hwndDefView, NULL, TEXT("Internet Explorer_Server"), NULL);
				if(hwnd==NULL)
				{
					hwnd = hwndDefView;
				}
			}
		}
	}
	return hwnd;
}

int SearchTarget(HWND hWnd, BOOL CornerMouse)
{
	int FoundTargetNumber = -1;
	TCHAR ExePath[_MAX_PATH + 1];
	TCHAR FilePath[_MAX_PATH + 1];
	TCHAR drive[_MAX_DRIVE + 1];
	TCHAR dir[_MAX_DIR + 1];
	TCHAR fname[_MAX_FNAME + 1];
	TCHAR fext[_MAX_EXT + 1];
	TCHAR FileName[_MAX_PATH + 1];
	int i = 0;
	BOOL FileNameFlag, ClassNameFlag, TargetIDFlag, WindowTitleFlag;

	GetModuleFileName(NULL, ExePath, _MAX_PATH);

	if(!GetFileNameFromHwnd(hWnd, FilePath, _MAX_PATH))
		return -1;

	if(lstrcmp(FilePath, ExePath) == 0)
		return -1;

	_tsplitpath_s( FilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT );
	lstrcpy(FileName, PathFindFileName(FilePath));

	while(i < (int)Target.size() && FoundTargetNumber == -1)
	{
		FileNameFlag = TRUE;
		ClassNameFlag = TRUE;
		TargetIDFlag = TRUE;
		WindowTitleFlag = TRUE;

		if(Target[i].FileName[0])
		{
			if(lstrcmpi(Target[i].FileName, TEXT("Desktop")) == 0)
			{
				if( hWnd == NULL || hWnd != GetDesktopWindowHandle() )
				{
					FileNameFlag = FALSE;
				}
			}
			else
			{
				TCHAR Tdrive[_MAX_DRIVE + 1];
				TCHAR Tdir[_MAX_DIR + 1];
				TCHAR Tfname[_MAX_FNAME + 1];
				TCHAR Tfext[_MAX_EXT + 1];

				_tsplitpath_s( Target[i].FileName, Tdrive, _MAX_DRIVE, Tdir, _MAX_DIR, Tfname, _MAX_FNAME, Tfext, _MAX_EXT );
				if(!((!Tdrive[0] || !lstrcmpi(Tdrive, drive)) &&
					(!Tdir[0] || !lstrcmpi(Tdir, dir)) &&
					(!lstrcmpi(Tfname, fname) && !lstrcmpi(Tfext, fext))))
				{
					FileNameFlag = FALSE;
				}
			}
		}

		if(Target[i].ClassName[0])
		{
			if(CornerMouse == FALSE)
			{
				if(lstrcmpi(Target[i].ClassName, TEXT("HitTestCaptionBar")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTCAPTION:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestScrollBar")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTHSCROLL:
					case HTVSCROLL:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestBorder")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTBORDER:
					case HTBOTTOM:
					case HTBOTTOMLEFT:
					case HTBOTTOMRIGHT:
					case HTLEFT:
					case HTRIGHT:
					case HTTOP:
					case HTTOPLEFT:
					case HTTOPRIGHT:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestMenu")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTMENU:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestListViewIcon")) == 0)
				{
					POINT pt;
					HWND hwnd;
					LVHITTESTINFO info;

					GetCursorPos(&pt);
					hwnd = WindowFromPoint(pt);
					ScreenToClient(hwnd, &pt);
					info.pt = pt;
					_ListView_HitTest(hwnd, &info);

					switch(info.flags)
					{
					case LVHT_ONITEMICON:
					case LVHT_ONITEMLABEL:
					case LVHT_ONITEMSTATEICON:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestTreeViewIcon")) == 0)
				{
					POINT pt;
					HWND hwnd;
					TVHITTESTINFO ht;

					GetCursorPos(&pt);
					hwnd = WindowFromPoint(pt);
					ScreenToClient(hwnd, &pt);
					ht.pt = pt;
					_TreeView_HitTest(hwnd, &ht);

					switch(ht.flags)
					{
					case TVHT_ONITEMICON:
					case TVHT_ONITEMLABEL:
					case TVHT_ONITEMSTATEICON:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestSystemMenuButton")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTSYSMENU:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestMinButton")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTMINBUTTON:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestMaxButton")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTMAXBUTTON:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else if(lstrcmpi(Target[i].ClassName, TEXT("HitTestCloseButton")) == 0)
				{
					switch(GetTargetHitTestCode())
					{
					case HTCLOSE:
						break;
					default:
						ClassNameFlag = FALSE;
						break;
					}
				}
				else
				{
					TCHAR TClassName[_MAX_PATH + 1];
					GetClassName(hWnd, TClassName, MAX_CLASSNAME);
					if(lstrcmpi(TClassName, Target[i].ClassName) != 0)
						ClassNameFlag = FALSE;
				}
			}
			else
			{
				ClassNameFlag = FALSE;
			}
		}

		if(Target[i].ControlID[0])
		{
			if(CornerMouse == FALSE)
			{
				LONG_PTR TargetID = 0;
				SetLastError(0);
				TargetID = GetWindowLongPtr(hWnd, GWLP_ID);
				if(TargetID != StrToInt(Target[i].ControlID) || GetLastError())
					ClassNameFlag = FALSE;
			}
			else
			{
				TargetIDFlag = FALSE;
			}
		}

		if(Target[i].WindowTitle[0])
		{
			if(CornerMouse == FALSE)
			{
				TCHAR WindowTitle[256];
				GetWindowText(GetAncestor(hWnd, GA_ROOT), WindowTitle, 256);
				if(StrStrI(WindowTitle, Target[i].WindowTitle) == NULL)	//部分一致
					WindowTitleFlag = FALSE;
			}
			else
			{
				WindowTitleFlag = FALSE;
			}
		}

		if(FileNameFlag == TRUE && ClassNameFlag == TRUE && TargetIDFlag == TRUE && WindowTitleFlag == TRUE)
			FoundTargetNumber = i;
		i ++;
	}

	if(FoundTargetNumber == -1)
		FoundTargetNumber = (int)Target.size();

	return FoundTargetNumber;
}
//継承するターゲットの番号を取得する
void GetThruNumber(int TargetLocate, int *TargetNumber)
{
	int i, j;

	TargetNumber[0] = TargetLocate;

	for(i=1;i<=MAX_TARGET_THRU;i++)
	{
		if(TargetNumber[i-1] < (int)Target.size() && TargetNumber[i-1] >= 0)
		{
			switch(Target[TargetNumber[i-1]].DefaultThru)
			{
			case -1:
				TargetNumber[i] = -1;
				break;
			case 0:
				TargetNumber[i] = (int)Target.size();
				break;
			default:
				for(j=0;j<(int)Target.size();j++)
				{
					if(Target[TargetNumber[i-1]].DefaultThru == Target[j].Number)
						TargetNumber[i] = j;
				}
				break;
			}
		}
		else
		{
			TargetNumber[i] = -1;
		}
	}
}

int SearchAction(int FoundTargetNumber, int SearchCode)
{
	int i, j, k, l;
	int FoundActionNumber = -1;
	int TargetNumber[MAX_TARGET_THRU + 1];

	if(FoundTargetNumber == -1)
		return -1;

	GetThruNumber(FoundTargetNumber, TargetNumber);

	switch(SearchCode)
	{
	case 0:
		for(i=0;i<=MAX_TARGET_THRU;i++)
		{
			if(TargetNumber[i] >= 0 && FoundActionNumber < 0)
			{
				j = 0;
				while(j <= (int)Action.size()-1 && FoundActionNumber < 0 && TargetNumber[i] >= 0)
				{
					if(((TargetNumber[i] < (int)Target.size() && Action[j].TargetNumber == Target[TargetNumber[i]].Number) ||
						(TargetNumber[i] == (int)Target.size() && Action[j].TargetNumber == 0)) &&
						Action[j].Button == Gesture.Button &&
						(Action[j].Modifier & MODIFIER_DISABLE || Action[j].Modifier == Gesture.Modifier))
					{
						if(Action[j].Repeat > 0)
						{
							FoundActionNumber = j;
							for(k=i-1;k>=0;k--)
							{
								if(TargetNumber[k] >= 0 && FoundActionNumber >= 0)
								{
									for(l=0;l<(int)Action.size();l++)
									{
										if(Action[l].TargetNumber == Target[TargetNumber[k]].Number &&
											Action[l].Button == Action[j].Button &&
											Action[l].Modifier == Action[j].Modifier &&
											Action[l].Move[0] == Action[j].Move[0] &&
											Action[l].Move[1] == Action[j].Move[1] &&
											Action[l].Move[2] == Action[j].Move[2] &&
											Action[l].Move[3] == Action[j].Move[3] &&
											Action[l].Move[4] == Action[j].Move[4] &&
											Action[l].Move[5] == Action[j].Move[5] &&
											Action[l].Move[6] == Action[j].Move[6] &&
											Action[l].Move[7] == Action[j].Move[7] &&
											Action[l].Move[8] == Action[j].Move[8] &&
											Action[l].Move[9] == Action[j].Move[9] &&
											Action[l].Repeat == 0)
										{
											FoundActionNumber = -1;
										}
									}
								}
							}
						}
					}
					j ++;
				}
			}
		}
		break;
	case 1:
		for(i=0;i<=MAX_TARGET_THRU;i++)
		{
			if(TargetNumber[i] >= 0 && FoundActionNumber < 0)
			{
				j = 0;
				while(j <= (int)Action.size()-1 && FoundActionNumber < 0 && TargetNumber[i] >= 0)
				{
					if(((TargetNumber[i] < (int)Target.size() && Action[j].TargetNumber == Target[TargetNumber[i]].Number) ||
						(TargetNumber[i] == (int)Target.size() && Action[j].TargetNumber == 0)) &&
						Action[j].Button == Gesture.Button &&
						(Action[j].Modifier & MODIFIER_DISABLE || Action[j].Modifier == Gesture.Modifier))
					{
						if(Action[j].Move[0] == Gesture.Move[0] &&
							Action[j].Move[1] == Gesture.Move[1] &&
							Action[j].Move[2] == Gesture.Move[2] &&
							Action[j].Move[3] == Gesture.Move[3] &&
							Action[j].Move[4] == Gesture.Move[4] &&
							Action[j].Move[5] == Gesture.Move[5] &&
							Action[j].Move[6] == Gesture.Move[6] &&
							Action[j].Move[7] == Gesture.Move[7] &&
							Action[j].Move[8] == Gesture.Move[8] &&
							Action[j].Move[9] == Gesture.Move[9])
						{
							FoundActionNumber = j;
						}
					}
					j ++;
				}
			}
		}
		break;
	case 2://ボタンクリックのみ以外のアクションがないか検索
		for(i=0;i<=MAX_TARGET_THRU;i++)
		{
			if(TargetNumber[i] >= 0 && FoundActionNumber < 0)
			{
				j = 0;
				while(j <= (int)Action.size()-1 && FoundActionNumber < 0 && TargetNumber[i] >= 0)
				{
					if(((TargetNumber[i] < (int)Target.size() && Action[j].TargetNumber == Target[TargetNumber[i]].Number) ||
						(TargetNumber[i] == (int)Target.size() && Action[j].TargetNumber == 0)) &&
						Action[j].Button == Gesture.Button &&
						(Action[j].Modifier & MODIFIER_DISABLE || Action[j].Modifier == Gesture.Modifier))
					{
						if(Action[j].Move[0] != 0 && Action[j].Repeat > 0)
						{
							FoundActionNumber = j;
							for(k=i-1;k>=0;k--)
							{
								if(TargetNumber[k] >= 0 && FoundActionNumber >= 0)
								{
									for(l=0;l<(int)Action.size();l++)
									{
										if(Action[l].TargetNumber == Target[TargetNumber[k]].Number &&
											Action[l].Button == Action[j].Button &&
											Action[l].Modifier == Action[j].Modifier &&
											Action[l].Move[0] == Action[j].Move[0] &&
											Action[l].Move[1] == Action[j].Move[1] &&
											Action[l].Move[2] == Action[j].Move[2] &&
											Action[l].Move[3] == Action[j].Move[3] &&
											Action[l].Move[4] == Action[j].Move[4] &&
											Action[l].Move[5] == Action[j].Move[5] &&
											Action[l].Move[6] == Action[j].Move[6] &&
											Action[l].Move[7] == Action[j].Move[7] &&
											Action[l].Move[8] == Action[j].Move[8] &&
											Action[l].Move[9] == Action[j].Move[9] &&
											Action[l].Repeat == 0)
										{
											FoundActionNumber = -1;
										}
									}
								}
							}
						}
					}
					j ++;
				}
			}
		}
		break;
	}
	return FoundActionNumber;
}

BYTE GetGestureMove(int PosDiffx, int PosDiffy)
{
	int RealMove;

	if(GestureDirection == -4 && Gesture.Level > 0)
	{
		switch(Gesture.Move[0])
		{
		case MOVE_UP:
		case MOVE_DOWN:
		case MOVE_LEFT:
		case MOVE_RIGHT:
			if(abs(PosDiffx) < abs(PosDiffy))
				RealMove = (PosDiffy<0) ? MOVE_UP : MOVE_DOWN;
			else
				RealMove = (PosDiffx<0) ? MOVE_LEFT : MOVE_RIGHT;
			break;
		case MOVE_UPLEFT:
		case MOVE_UPRIGHT:
		case MOVE_DOWNLEFT:
		case MOVE_DOWNRIGHT:
			if(PosDiffy < 0)
				RealMove = (PosDiffx<0) ? MOVE_UPLEFT : MOVE_UPRIGHT;
			else
				RealMove = (PosDiffx<0) ? MOVE_DOWNLEFT : MOVE_DOWNRIGHT;
			break;
		}
	}
	else
	{
		if(abs(PosDiffx) < abs(PosDiffy))
		{
			if((GestureDirection == 8 || GestureDirection == -4 || (GestureDirection == -8 && Gesture.Level <= 1)) && abs(PosDiffx) > abs(PosDiffy) / 2)
			{
				if(PosDiffy < 0)
				{
					RealMove = (PosDiffx<0) ? MOVE_UPLEFT : MOVE_UPRIGHT;
					if(GestureDirection == -8 && Gesture.Level == 1 && Gesture.Move[0] != RealMove)
						RealMove = MOVE_UP;
				}
				else
				{
					RealMove = (PosDiffx<0) ? MOVE_DOWNLEFT : MOVE_DOWNRIGHT;
					if(GestureDirection == -8 && Gesture.Level == 1 && Gesture.Move[0] != RealMove)
						RealMove = MOVE_DOWN;
				}
			}
			else
			{
				RealMove = (PosDiffy<0) ? MOVE_UP : MOVE_DOWN;
			}
		}
		else
		{
			if((GestureDirection == 8 || GestureDirection == -4 || (GestureDirection == -8 && Gesture.Level <= 1)) && abs(PosDiffx) / 2 < abs(PosDiffy))
			{
				if(PosDiffx < 0)
				{
					RealMove = (PosDiffy<0) ? MOVE_UPLEFT : MOVE_DOWNLEFT;
					if(GestureDirection == -8 && Gesture.Level == 1 && Gesture.Move[0] != RealMove)
						RealMove = MOVE_LEFT;
				}
				else
				{
					RealMove = (PosDiffy<0) ? MOVE_UPRIGHT : MOVE_DOWNRIGHT;
					if(GestureDirection == -8 && Gesture.Level == 1 && Gesture.Move[0] != RealMove)
						RealMove = MOVE_RIGHT;
				}
			}
			else
			{
				RealMove = (PosDiffx<0) ? MOVE_LEFT : MOVE_RIGHT;
			}
		}
	}
	return RealMove;
}

void MyTaskTray(HWND hWnd)
{
	static NOTIFYICONDATA ni = {0};
	static HICON hIcon = NULL;

	if(hIcon==NULL)
		hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));

	if(hWnd != NULL)
	{
		ni.cbSize = sizeof(NOTIFYICONDATA);
		ni.hWnd = hWnd;
		ni.uID = ID_MYTRAY;
		ni.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		ni.uCallbackMessage = MYMSG_TRAY;
		ni.hIcon = hIcon;
		lstrcpyn(ni.szTip, OPENMAUSUJI_VERSION, 64);

		//http://support.microsoft.com/kb/418138/ja
		while(1)
		{
			if(Shell_NotifyIcon(NIM_ADD, &ni))
				break;

			if(GetLastError()==ERROR_TIMEOUT)
			{
				Sleep(500);
				if(Shell_NotifyIcon(NIM_MODIFY, &ni))
					break;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		Shell_NotifyIcon(NIM_DELETE, &ni);
		if(hIcon)
		{
			DestroyIcon(hIcon);
			hIcon = NULL;
		}
	}
}

void NaviRefresh(HWND hWnd)
{
	HDC hdc;
	HGDIOBJ obj;
	SIZE size;
	RECT rect;
	POINT apos;
	TCHAR NaviString[50 + MAX_COMMENT + 1];
	int x, y, width, height;

	if(NaviType == NaviTypeFixed || NaviType == NaviTypeFloat)
	{
		KillTimer(hWnd, ID_NAVITIMER);
		NaviDrawFlag = TRUE;

		//幅と高さ
		if(lstrlen(CommentString) > 0)
			wsprintf(NaviString, TEXT("%s %s"), GestureString, CommentString);
		else
			lstrcpy(NaviString, GestureString);
		if(lstrlen(NaviString)>0)	//表示する文字列がある
		{
			hdc = GetDC(NULL);
			obj = SelectObject(hdc, hFont);
			GetTextExtentPoint32(hdc, NaviString, lstrlen(NaviString), &size);
			SelectObject(hdc, obj);
			ReleaseDC(NULL, hdc);
			if(NaviWidth == 0)	//幅は可変
				width = size.cx + 7;
			else
				width = NaviWidth;
			height = size.cy+4;
		}
		else
		{
			width = 0;
			height = 0;
		}
		//位置
		GetWindowRect(hwndNavi, &rect);
		x = rect.left;
		y = rect.top;
		if(NaviType == NaviTypeFloat)
		{
			GetCursorPos(&apos);
			if(apos.x + TipLocate.x + width > GetSystemMetrics(SM_CXVIRTUALSCREEN))
				x = GetSystemMetrics(SM_CXVIRTUALSCREEN) - width;
			else
				x = apos.x + TipLocate.x;
			if(apos.y + TipLocate.y + height > GetSystemMetrics(SM_CYVIRTUALSCREEN))
				y = GetSystemMetrics(SM_CYVIRTUALSCREEN) - height;
			else
				y = apos.y + TipLocate.y;
		}
		//移動と表示
		MoveWindow(hwndNavi, x, y, width, height, TRUE);
		ShowWindow(hwndNavi, SW_SHOWNA);
		InvalidateRect(hwndNavi, NULL, TRUE);
	}
}

void ShowMauSujiNavi(HWND hWnd, BOOL Param)
{
	if(Param)
	{
		NaviRefresh(hWnd);
	}
	else
	{
		if(NaviType == NaviTypeFixed)
			GetWindowPosition(hwndNavi, &MauSujiNavi);
		ShowWindow(hwndNavi, SW_HIDE);
	}
}

void NaviDeleteProc(HWND hwnd, UINT id)
{
	lstrcpy(GestureString, NaviTitle);
	lstrcpy(CommentString, TEXT(""));
	if(PopupFlag == FALSE)
	{
		switch(NaviType)
		{
		case NaviTypeFixed:
			NaviRefresh(hwnd);
			break;
		case NaviTypeFloat:
			ShowMauSujiNavi(hwnd, FALSE);
			break;
		}
	}
	KillTimer(hwnd, ID_NAVITIMER);
	NaviDrawFlag = FALSE;
}

void NaviEditRefresh(HWND hWnd)
{
	switch(NaviType)
	{
	case NaviTypeFixed:
		lstrcpy(GestureString, NaviTitle);
		lstrcpy(CommentString, TEXT(""));
		NaviRefresh(hWnd);
		break;
	case NaviTypeFloat:
		NaviRefresh(hWnd);
		SetTimer(hWnd, ID_NAVITIMER, NaviDeleteTime, NULL);
		break;
	}
}

void NaviPaint(HWND hWnd)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;
	HBRUSH hBrush;
	HGDIOBJ obj;
	SIZE size;
	TCHAR NaviString[50 + MAX_COMMENT + 1];

	hdc = BeginPaint(hWnd, &ps);

	//枠と背景を描画
	GetClientRect(hWnd, &rc);

	hBrush = CreateSolidBrush(FrameColor);
	FillRect(hdc, &rc, hBrush);
	DeleteObject(hBrush);

	InflateRect(&rc, -1, -1);

	hBrush = CreateSolidBrush(BackColor);
	FillRect(hdc, &rc, hBrush);
	DeleteObject(hBrush);

	SetBkMode(hdc, TRANSPARENT);
	obj = SelectObject(hdc, hFont);

	//ジェスチャー文字列を描画
	SetTextColor(hdc, MainColor);
	TextOut(hdc, 4, 2, GestureString, lstrlen(GestureString));

	//コメント文字列があったらコメント文字列を描画
	if(lstrlen(CommentString) > 0)
	{
		wsprintf(NaviString, TEXT(" %s"), CommentString);
		GetTextExtentPoint32(hdc, GestureString, lstrlen(GestureString), &size);
		if(Gesture.Level >= 0 && Gesture.Level <= MAX_GESTURE_LEVEL &&
			Gesture.Move[0] != BUTTON_L && Gesture.Move[0] != BUTTON_M && Gesture.Move[0] != BUTTON_R &&
			Gesture.Move[0] != BUTTON_X1 && Gesture.Move[0] != BUTTON_X2 && ClickOnlyFlag == FALSE &&
			Gesture.Move[0] != WHEEL_UP && Gesture.Move[0] != WHEEL_DOWN && Gesture.Move[0] != WHEEL_LEFT && Gesture.Move[0] != WHEEL_RIGHT)
		{
			SetTextColor(hdc, SubColor);
		}
		TextOut(hdc, 4 + size.cx, 2, NaviString, lstrlen(NaviString));
	}

	SelectObject(hdc, obj);

	EndPaint(hWnd, &ps);
}

LRESULT CALLBACK WndProcNavi(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwndMain = NULL;

	switch(msg)
	{
	case WM_CREATE:
		hwndMain =(HWND)((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	case WM_PAINT:
		NaviPaint(hwnd);
		break;
	case WM_NCHITTEST:
		wParam = DefWindowProc(hwnd, msg, wParam, lParam);
		if(wParam==HTCLIENT)
			return HTCAPTION;
		return wParam;
	case WM_NCRBUTTONUP:
	case WM_RBUTTONUP:
		SendMessage(hwndMain, msg, wParam, lParam);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void UnShadeWindowAll()
{
	std::vector<SHADEWINDOW>::iterator it;

	it = shadewindowlist.begin();
	while( it != shadewindowlist.end() )
	{
		//シェード終了
		SetWindowPos(it->hwnd, NULL, NULL, NULL, it->rect.right-it->rect.left, it->rect.bottom-it->rect.top, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER);
		if(it->bMaximize)
			SendMessage(it->hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);	//最大化する
		it++;
	}
}

void SetShadeWindow(HWND hwnd)
{
	std::vector<SHADEWINDOW>::iterator it;
	SHADEWINDOW shadewindow;

	it = shadewindowlist.begin();
	while( it != shadewindowlist.end() )
	{
		if(hwnd == it->hwnd)
		{
			//シェード終了
			SetWindowPos(it->hwnd, NULL, NULL, NULL, it->rect.right-it->rect.left, it->rect.bottom-it->rect.top, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER);
			if(it->bMaximize)
				SendMessage(it->hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);	//最大化する
			shadewindowlist.erase(it);	//削除
			return;
		}
		it++;
	}

	//シェードする
	shadewindow.hwnd = hwnd;
	shadewindow.bMaximize = GetWindowLongPtr(hwnd, GWL_STYLE) & WS_MAXIMIZE;
	if(shadewindow.bMaximize)
		SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);	//最大化していたら元のサイズに戻す
	GetWindowRect(hwnd, &shadewindow.rect);
	SetWindowPos(hwnd, NULL, NULL, NULL, shadewindow.rect.right-shadewindow.rect.left, GetSystemMetrics(SM_CYCAPTION) + (GetSystemMetrics(SM_CYFRAME) * 2), SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOZORDER);
	shadewindowlist.push_back(shadewindow);	//追加
}

LRESULT CALLBACK LowLevelMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PMSLLHOOKSTRUCT mousehooks = (PMSLLHOOKSTRUCT)lParam;

	if(nCode==HC_ACTION)
	{
		switch(WindowHook)
		{
		case 1:
			if(wParam==WM_LBUTTONDOWN || wParam==WM_NCLBUTTONDOWN)
			{
				LONG_PTR TempID;

				WindowHook = 0;
				GetFileNameFromHwnd(WindowFromPoint(mousehooks->pt), TargetPath,_MAX_PATH);
				GetClassName(WindowFromPoint(mousehooks->pt), TargetClass, MAX_CLASSNAME);
				SetLastError(0);
				TempID = GetWindowLongPtr(WindowFromPoint(mousehooks->pt), GWLP_ID);
				if(!GetLastError())
					wsprintf(TargetID, TEXT("%d"), GetWindowLongPtr(WindowFromPoint(mousehooks->pt), GWLP_ID));
				else
					lstrcpy(TargetID, TEXT("\0"));
				GetWindowText(GetAncestor(WindowFromPoint(mousehooks->pt), GA_ROOT), TargetTitle, 256);
				PostMessage(CallDlg1, MAUHOOK_MSG, 0, 0);
			}
			break;
		case 2:
			if(wParam==WM_LBUTTONDOWN && SendMessage(WindowFromPoint(mousehooks->pt), WM_NCHITTEST, 0, MAKELPARAM(mousehooks->pt.x, mousehooks->pt.y))==HTCAPTION)
			{
				RECT TargetRect;
				TCHAR buffer[20];

				WindowHook = 0;
				GetWindowRect(GetRootWindow(WindowFromPoint(mousehooks->pt)), &TargetRect);
				wsprintf(buffer, TEXT("%d"), TargetRect.right - TargetRect.left);
				SendMessage(CallDlg1, WM_SETTEXT, 0, (LPARAM)buffer);
				wsprintf(buffer, TEXT("%d"), TargetRect.bottom - TargetRect.top);
				SendMessage(CallDlg2, WM_SETTEXT, 0, (LPARAM)buffer);
			}
			break;
		case 3:
			if(wParam==WM_LBUTTONDOWN && SendMessage(WindowFromPoint(mousehooks->pt), WM_NCHITTEST, 0, MAKELPARAM(mousehooks->pt.x, mousehooks->pt.y))==HTCAPTION)
			{
				RECT TargetRect;
				TCHAR buffer[20];

				WindowHook = 0;
				GetWindowRect(GetRootWindow(WindowFromPoint(mousehooks->pt)), &TargetRect);
				wsprintf(buffer, TEXT("%d"), TargetRect.left);
				SendMessage(CallDlg1, WM_SETTEXT, 0, (LPARAM)buffer);
				wsprintf(buffer, TEXT("%d"), TargetRect.top);
				SendMessage(CallDlg2, WM_SETTEXT, 0, (LPARAM)buffer);
			}
			break;
		case 4:
			if(wParam==WM_RBUTTONDOWN || wParam==WM_NCRBUTTONDOWN)
			{
				RECT TargetRect;
				TCHAR buffer[20];

				WindowHook = 0;
				GetWindowRect(GetRootWindow(WindowFromPoint(mousehooks->pt)), &TargetRect);
				wsprintf(buffer, TEXT("%d"), mousehooks->pt.x - TargetRect.left);
				SendMessage(CallDlg1, WM_SETTEXT, 0, (LPARAM)buffer);
				wsprintf(buffer, TEXT("%d"), mousehooks->pt.y - TargetRect.top);
				SendMessage(CallDlg2, WM_SETTEXT, 0, (LPARAM)buffer);
			}
			break;
		}
	}
	return CallNextHookEx(hHookMouse, nCode, wParam, lParam);
}

void SetWindowHook(HWND hDlg1, HWND hDlg2, int HookType)
{
	hHookMouse = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseHookProc, GetModuleHandle(NULL), 0);
	if(WindowHook == 0)
	{
		WindowHook = HookType;
		CallDlg1 = hDlg1;
		CallDlg2 = hDlg2;
	}
}

void WindowUnHook(void)
{
	UnhookWindowsHookEx(hHookMouse);
	WindowHook = 0;
}

void GetFileName(LPTSTR FileName, LPTSTR ClassName, LPTSTR ControlID, LPTSTR Title)
{
	lstrcpy(FileName, TargetPath);
	lstrcpy(ClassName, TargetClass);
	lstrcpy(ControlID, TargetID);
	lstrcpy(Title, TargetTitle);
}

BOOL CALLBACK SearchCommandTargetProc(HWND hWnd, LPARAM lParam)
{
	TCHAR ClassName[MAX_CLASSNAME + 1];
	SEARCHCHILDINFO *TargetInfo;
	TargetInfo = (SEARCHCHILDINFO *)lParam;
	int ClassFlag = 0x00000000;
	TCHAR WindowTitle[256];

	if(TargetInfo->ClassFlag & 0x00000001)
	{
		GetClassName(hWnd, ClassName, MAX_CLASSNAME);
		if(lstrcmpi(TargetInfo->ClassName, ClassName) == 0)
			ClassFlag = ClassFlag | 0x00000001;
	}
	if(TargetInfo->ClassFlag & 0x00000002)
	{
		SetLastError(0);
		if(GetWindowLongPtr(hWnd, GWLP_ID) == TargetInfo->ControlID && !GetLastError())
			ClassFlag = ClassFlag | 0x00000002;
	}
	if(TargetInfo->ClassFlag & 0x00000004)
	{
		GetWindowText(hWnd, WindowTitle, 256);
		if(lstrcmpi(TargetInfo->WindowTitle, WindowTitle) == 0)
			ClassFlag = ClassFlag | 0x00000004;
	}
	if(TargetInfo->ClassFlag == ClassFlag)
	{
		TargetInfo->hwTarget = hWnd;
	}
	return !TargetInfo->hwTarget;
}

HWND SearchCommandTarget(int CommandTarget)
{
	int i;
	TCHAR FilePath[_MAX_PATH + 1];
	TCHAR drive[_MAX_DRIVE + 1], Tdrive[_MAX_DRIVE + 1];
	TCHAR dir[_MAX_DIR + 1], Tdir[_MAX_DIR + 1];
	TCHAR fname[_MAX_FNAME + 1], Tfname[_MAX_FNAME + 1];
	TCHAR fext[_MAX_EXT + 1], Tfext[_MAX_EXT + 1];
	TCHAR TFileName[_MAX_PATH + 1];
	HWND hWnd;
	SEARCHCHILDINFO TargetInfo = {0, NULL, TEXT(""), 0, TEXT("")};

	for ( i = 0; i < (int)Target.size(); i++ )
	{
		if(Target[i].Number == CommandTarget)
		{
			lstrcpy(TFileName, Target[i].FileName);
			_tsplitpath_s( TFileName, Tdrive, _MAX_DRIVE, Tdir, _MAX_DIR, Tfname, _MAX_FNAME, Tfext, _MAX_EXT );
			if(Target[i].ClassName[0])
			{
				lstrcpy(TargetInfo.ClassName, Target[i].ClassName);
				TargetInfo.ClassFlag = TargetInfo.ClassFlag | 0x00000001;
			}
			if(Target[i].ControlID[0])
			{
				TargetInfo.ControlID = StrToInt(Target[i].ControlID);
				TargetInfo.ClassFlag = TargetInfo.ClassFlag | 0x00000002;
			}
			if(Target[i].WindowTitle[0])
			{
				lstrcpy(TargetInfo.WindowTitle, Target[i].WindowTitle);
				TargetInfo.ClassFlag = TargetInfo.ClassFlag | 0x00000004;
			}
		}
	}

	if(TFileName[0])
	{
		if(lstrcmpi(TFileName, TEXT("Desktop")) == 0)
		{
			TargetInfo.hwTarget = GetDesktopWindowHandle();
		}
		else
		{
			hWnd = GetTopWindow(NULL);
			while(TargetInfo.hwTarget == NULL && hWnd != NULL)
			{
				GetFileNameFromHwnd(hWnd, FilePath, _MAX_PATH);
				_tsplitpath_s( FilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT );
				if((!Tdrive[0] || !lstrcmpi(Tdrive, drive)) &&
					(!Tdir[0] || !lstrcmpi(Tdir, dir)) &&
					(!lstrcmpi(Tfname, fname) && !lstrcmpi(Tfext, fext)) &&
					GetParent(hWnd) == NULL)
				{
					if(TargetInfo.ClassFlag != 0)
						EnumChildWindows(hWnd, SearchCommandTargetProc, (LPARAM)&TargetInfo);
					else
						TargetInfo.hwTarget = hWnd;
				}
				hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
			}
		}
	}
	return TargetInfo.hwTarget;
}

void ImplementCommand(HWND hWnd, int FoundActionNumber, BOOL FinishAction)
{
	static int FinishCommandNumber = 0;
	HWND hwTarget;
	RECT TargetRect;
	int TargetSizeX, TargetSizeY;
	HGLOBAL hMem;
	TCHAR *p;
	UINT Message;
	TCHAR *HexEnd;
	WPARAM wParam;
	LPARAM lParam;
	int i, j;

	if(FinishAction == FALSE)
		i = Action[FoundActionNumber].BreakPoint;
	else
		i = FinishCommandNumber;
	Action[FoundActionNumber].BreakPoint = 0;
	FinishActionNumber = -1;
	FinishCommandNumber = 0;

	while(i < Action[FoundActionNumber].Repeat && Action[FoundActionNumber].BreakPoint == 0)
	{
		switch(Action[FoundActionNumber].command[i].CommandTarget)
		{
		case -1:
			hwTarget = GetRootWindow(GetTargetHandle());
			break;
		case 0:
			hwTarget = GetRootWindow(GetForegroundWindow());
			break;
		default:
			hwTarget = SearchCommandTarget(Action[FoundActionNumber].command[i].CommandTarget);
			break;
		}
		switch ( Action[FoundActionNumber].command[i].Case )
		{
		case 1:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			ExecutePlugin(hwTarget, StartPos, FinishPos, &CommandExt[Action[FoundActionNumber].command[i].Key]);
			break;
		case 10:
		case 11:
		case 12:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			ExecuteSendKey( Action[FoundActionNumber].command[i].Case, Action[FoundActionNumber].command[i].Key, ClickWait );
			break;
		case 20:
			if(FinishCommandNumber > 0 || hwTarget == NULL)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			PostMessage( hwTarget, WM_COMMAND, Action[FoundActionNumber].command[i].Key, 0 );
			break;
		case 25:
			hMem = GlobalAlloc(GHND, (lstrlen(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]) + 1)*sizeof(TCHAR));
			p = (TCHAR*)GlobalLock(hMem);
			lstrcpy(p, CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]);
			GlobalUnlock(hMem);
			OpenClipboard(NULL);
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hMem);
			CloseClipboard();
			break;
		case 30:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );

			//関連付けを実行
			if(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0][0])
			{
				TCHAR TargetFileName[_MAX_PATH + 1];
				TCHAR TargetDirectory[_MAX_PATH + 1];
				LPTSTR parameters;

				if(PathIsRelative(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]))
				{	//絶対パスではないとき、絶対パスに変換
					TCHAR szDir[MAX_PATH];
					TCHAR szPath[MAX_PATH];
					GetModuleFileName(NULL, szDir, MAX_PATH);
					PathRemoveFileSpec(szDir);
					wsprintf(szPath, TEXT("%s\\%s"), szDir, CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]);
					PathCanonicalize(TargetFileName, szPath);
				}
				else
				{
					lstrcpy(TargetFileName, CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]);
				}
				lstrcpy(TargetDirectory, TargetFileName);
				PathRemoveFileSpec(TargetDirectory);
				parameters = StrReplace(hwTarget, CommandExt[Action[FoundActionNumber].command[i].Key].Text[1]);
				ShellExecute(NULL, NULL, TargetFileName, parameters, TargetDirectory, SW_SHOWDEFAULT);
				HeapFree(GetProcessHeap(), 0, parameters);
			}
			break;
		case 35:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			PlaySound(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0], NULL, SND_ASYNC);
			break;
		case 40:
			if(FinishCommandNumber > 0 || hwTarget == NULL)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			switch ( Action[FoundActionNumber].command[i].Key )
			{
			case 0:
				SetWindowPos( hwTarget, HWND_TOP, NULL, NULL, NULL, NULL, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				break;
			case 1:
				SetWindowPos( hwTarget, HWND_BOTTOM, NULL, NULL, NULL, NULL, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				break;
			case 2:
				SetWindowPos( hwTarget, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				break;
			case 3:
				SetWindowPos( hwTarget, HWND_NOTOPMOST, NULL, NULL, NULL, NULL, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				break;
			case 4:
				if(GetWindowLongPtr(hwTarget, GWL_EXSTYLE) & WS_EX_TOPMOST)
					SetWindowPos( hwTarget, HWND_NOTOPMOST, NULL, NULL, NULL, NULL, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				else
					SetWindowPos( hwTarget, HWND_TOPMOST, NULL, NULL, NULL, NULL, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE );
				break;
			case 5:
				PostMessage( hwTarget, WM_SYSCOMMAND, SC_RESTORE, 0);
				break;
			case 6:
				PostMessage( hwTarget, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				break;
			case 7:
				PostMessage( hwTarget, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				break;
			case 8:
				PostMessage( hwTarget, WM_SYSCOMMAND, SC_CLOSE, 0);
				break;
			case 9:
				if(GetWindowLongPtr(hwTarget, GWL_STYLE) & WS_MINIMIZE)
					PostMessage( hwTarget, WM_SYSCOMMAND, SC_RESTORE, 0);
				else
					PostMessage( hwTarget, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				break;
			case 10:
				if(GetWindowLongPtr(hwTarget, GWL_STYLE) & WS_MAXIMIZE)
					PostMessage( hwTarget, WM_SYSCOMMAND, SC_RESTORE, 0);
				else
					PostMessage( hwTarget, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				break;
			case 11:
				if(GetWindowLongPtr(hwTarget, GWL_STYLE) & WS_MAXIMIZE)
					PostMessage( hwTarget, WM_SYSCOMMAND, SC_MINIMIZE, 0);
				else
					PostMessage( hwTarget, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
				break;
			case 12:
				SetShadeWindow(hwTarget);
				break;
			}
			break;
		case 41:
			if(FinishCommandNumber > 0 || hwTarget == NULL)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			SetWindowLongPtr(hwTarget, GWL_STYLE, GetWindowLongPtr(hwTarget, GWL_STYLE) & ~WS_MAXIMIZE);
			GetWindowRect(hwTarget, &TargetRect);
			TargetSizeX = (SHORT)HIWORD(Action[FoundActionNumber].command[i].Key);
			if(TargetSizeX < 0)
				TargetSizeX = TargetRect.right - TargetRect.left;
			TargetSizeY = (SHORT)LOWORD(Action[FoundActionNumber].command[i].Key);
			if(TargetSizeY < 0)
				TargetSizeY = TargetRect.bottom - TargetRect.top;
			SetWindowPos( hwTarget, NULL, NULL, NULL, TargetSizeX, TargetSizeY, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
			break;
		case 42:
			if(FinishCommandNumber > 0 || hwTarget == NULL)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			SetWindowLongPtr(hwTarget, GWL_STYLE, GetWindowLongPtr(hwTarget, GWL_STYLE) & ~WS_MAXIMIZE);
			SetWindowPos( hwTarget, NULL, (SHORT)HIWORD(Action[FoundActionNumber].command[i].Key), (SHORT)LOWORD(Action[FoundActionNumber].command[i].Key), NULL, NULL, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER );
			break;
		case 50:
			if(FinishCommandNumber > 0 || hwTarget == NULL)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			if((Action[FoundActionNumber].command[i].Key == 0 || GetWindowLongPtr(hwTarget, GWL_EXSTYLE) & WS_EX_LAYERED) &&
				!(Action[FoundActionNumber].command[i].Key < 0))
			{
				SetWindowLongPtr(hwTarget, GWL_EXSTYLE, GetWindowLongPtr(hwTarget, GWL_EXSTYLE) & ~WS_EX_LAYERED);
				SetLayeredWindowAttributes(hwTarget, 0, 255, LWA_ALPHA);
				RedrawWindow(hwTarget, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
			}
			else
			{
				SetWindowLongPtr(hwTarget, GWL_EXSTYLE, GetWindowLongPtr(hwTarget, GWL_EXSTYLE) | WS_EX_LAYERED);
				SetLayeredWindowAttributes(hwTarget, 0, 255 - (249 * (BYTE)Action[FoundActionNumber].command[i].Key / 100), LWA_ALPHA);
			}
			break;
		case 55:
			if(FinishCommandNumber > 0 || hwTarget == NULL)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			GetWindowRect(hwTarget, &TargetRect);
			SetCursorPos((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key) + TargetRect.left,
				(SHORT)LOWORD(Action[FoundActionNumber].command[i].Key) + TargetRect.top);
			break;
		case 58:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			SetCursorPos((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key) + StartPos.x,
				(SHORT)LOWORD(Action[FoundActionNumber].command[i].Key) + StartPos.y);
			break;
		case 59:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			SetCursorPos((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key) + FinishPos.x,
				(SHORT)LOWORD(Action[FoundActionNumber].command[i].Key) + FinishPos.y);
			break;
		case 60:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			switch(abs((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key)) - 1)
			{
			case SB_PAGELEFT:
			case SB_PAGERIGHT:
				if((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key) > 0)
					PostMessage(GetTargetHandle(), WM_VSCROLL, abs((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key)) - 1, (LPARAM)SearchScrollBar(GetTargetHandle(), SBS_VERT));
				else
					PostMessage(GetTargetHandle(), WM_HSCROLL, abs((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key)) - 1, (LPARAM)SearchScrollBar(GetTargetHandle(), SBS_HORZ));
				break;
			case SB_LINELEFT:
			case SB_LINERIGHT:
				for(j = 0; j < LOWORD(Action[FoundActionNumber].command[i].Key); j++)
				{
					if((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key) > 0)
						PostMessage(GetTargetHandle(), WM_VSCROLL, abs((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key)) - 1, (LPARAM)SearchScrollBar(GetTargetHandle(), SBS_VERT));
					else
						PostMessage(GetTargetHandle(), WM_HSCROLL, abs((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key)) - 1, (LPARAM)SearchScrollBar(GetTargetHandle(), SBS_HORZ));
				}
				break;
			}
			break;
		case 65:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			SetMouseScroll((SHORT)HIWORD(Action[FoundActionNumber].command[i].Key), (SHORT)LOWORD(Action[FoundActionNumber].command[i].Key));
			break;
		case 70:
		case 71:
			if(FinishCommandNumber > 0)
				break;
			Sleep( Action[FoundActionNumber].command[i].Wait );
			if(CommandExt[Action[FoundActionNumber].command[i].Key].Key[0] < 100)
			{
				switch(Action[FoundActionNumber].command[i].CommandTarget)
				{
				case -1:
					hwTarget = GetTargetHandle();
					break;
				case 0:
					hwTarget = GetWindowFocus(GetForegroundWindow());
					break;
				default:
					hwTarget = SearchCommandTarget(Action[FoundActionNumber].command[i].CommandTarget);
					break;
				}
			}
			switch(CommandExt[Action[FoundActionNumber].command[i].Key].Key[0])
			{
			case 0:
			case 100:
				Message = StrToInt(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]);
				break;
			case 1:
			case 101:
				Message = _tcstoul(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0], &HexEnd, 16);
				break;
			case 2:
			case 102:
				Message = RegisterWindowMessage(CommandExt[Action[FoundActionNumber].command[i].Key].Text[0]);
				break;
			}
			switch(CommandExt[Action[FoundActionNumber].command[i].Key].Key[1])
			{
			case 0:
				wParam = StrToInt(CommandExt[Action[FoundActionNumber].command[i].Key].Text[1]);
				break;
			case 1:
				wParam = _tcstoul(CommandExt[Action[FoundActionNumber].command[i].Key].Text[1], &HexEnd, 16);
				break;
			case 2:
				wParam = (WPARAM)CommandExt[Action[FoundActionNumber].command[i].Key].Text[1];
				break;
			}
			switch(CommandExt[Action[FoundActionNumber].command[i].Key].Key[2])
			{
			case 0:
				lParam = StrToInt(CommandExt[Action[FoundActionNumber].command[i].Key].Text[2]);
				break;
			case 1:
				lParam = _tcstoul(CommandExt[Action[FoundActionNumber].command[i].Key].Text[2], &HexEnd, 16);
				break;
			case 2:
				lParam = (LPARAM)CommandExt[Action[FoundActionNumber].command[i].Key].Text[2];
				break;
			}
			if(Action[FoundActionNumber].command[i].Case == 70)
				SendMessage(hwTarget, Message, wParam, lParam);
			else
				PostMessage(hwTarget, Message, wParam, lParam);
			break;
		case 99:
			if(FinishCommandNumber > 0)
				break;
			PostMessage(hWnd, WM_COMMAND, IDM_SHOWDIALOG, 0);
			break;
		case 95:
			Action[FoundActionNumber].BreakPoint = i + 1;
			break;
		case 96:
			if(FinishAction == TRUE)
				break;
			FinishActionNumber = FoundActionNumber;
			FinishCommandNumber = i + 1;
			break;
		}
		i++;
	}
}

void InsertTargetItem(HWND hLVT, int intIndex, int Insert)
{
	TCHAR FileName[_MAX_PATH + 1], ClassName[_MAX_PATH + 1];

	LV_ITEM item;

	item.mask = LVIF_TEXT;
	if(intIndex == (int)Target.size())
	{
		lstrcpy(FileName, TEXT("Default"));
	}
	else
	{
		lstrcpy(FileName, PathFindFileName(Target[intIndex].FileName));

		if(Target[intIndex].Comment[0])
		{
			lstrcpyn(FileName, Target[intIndex].Comment, 100);
		}
		else
		{
			lstrcpyn(ClassName, Target[intIndex].ClassName, 100);
			if(Target[intIndex].FileName[0] == 0 && Target[intIndex].ClassName[0])
			{
				lstrcpy(FileName, TEXT("--- "));
				lstrcat(FileName, ClassName);
			}
			else
			{
				lstrcat(FileName, TEXT(" "));
				lstrcat(FileName, ClassName);
			}
		}
	}
	item.pszText = FileName;
	item.iItem = intIndex;
	item.iSubItem = 0;
	if(ListView_GetItemCount(hLVT) - 1 > intIndex && Insert == 0)
		ListView_SetItem(hLVT, &item);
	else
		ListView_InsertItem(hLVT, &item);
}

void InsertActionItem(HWND hLVA, int i, int intIndex)
{
	TCHAR szText[70+MAX_COMMENT+1];
	TCHAR Code[70+MAX_COMMENT+1];
	LV_ITEM item;

	GetGestureString(&Action[i].Modifier, &Action[i].Button, Action[i].Move, szText, 70+MAX_COMMENT+1);
	if(!(Action[i].Modifier&MODIFIER_DISABLE) && !(Action[i].Modifier&MODIFIER_CONTROL) && !(Action[i].Modifier&MODIFIER_SHIFT))
		wsprintf(Code, TEXT("No Modifier %s"), szText);
	else
		lstrcpy(Code, szText);
	lstrcat( Code, TEXT("  ") );
	lstrcat( Code, Action[i].Comment );

	item.mask = LVIF_TEXT;
	item.pszText = Code;
	item.iItem = intIndex;
	item.iSubItem = 0;
	if(ListView_GetItemCount(hLVA) > intIndex)
		ListView_SetItem(hLVA, &item);
	else
		ListView_InsertItem(hLVA, &item);
}

void InsertCommandItem(HWND hLVC, int Case, LONG Key, int intIndex, int Insert)
{
	int FoundCaseNumber = 0, i;
	TCHAR CaseText[MAX_PATH],LVCommandText[1024];

	if(Case==1)
	{
		i = FindPlugin(CommandExt[Key].path);
		if(i==-1)
		{
			lstrcpyn(CaseText, CommandExt[Key].path, MAX_PATH);
			lstrcpy(LVCommandText, TEXT(""));
		}
		else
		{
			COMMANDARGV argv;
			lstrcpyn(CaseText, plugins[i].plugin->name, MAX_PATH);
			argv.nValue[0] = CommandExt[Key].Key[0];
			argv.nValue[1] = CommandExt[Key].Key[1];
			argv.nValue[2] = CommandExt[Key].Key[2];
			argv.nValue[3] = CommandExt[Key].Key[3];
			lstrcpy(argv.szText[0], CommandExt[Key].Text[0]);
			lstrcpy(argv.szText[1], CommandExt[Key].Text[1]);
			lstrcpy(argv.szText[2], CommandExt[Key].Text[2]);
			lstrcpy(argv.szText[3], CommandExt[Key].Text[3]);
			plugins[i].plugin->Description(&argv, LVCommandText, 1024);
		}
	}
	else
	{
		for(i=0; i<MAX_ACTION_CASE; i++)
			if(CommandList[i].Number == Case)
				FoundCaseNumber = i;
		lstrcpy(CaseText, CommandList[FoundCaseNumber].Name);

		switch(Case)
		{
		case 10:
		case 11:
		case 12:
			get_keyname(Key, LVCommandText, _MAX_PATH+1);
			break;
		case 20:
			wsprintf(LVCommandText, TEXT("%d"), Key);
			break;
		case 25:
			lstrcpy(LVCommandText, CommandExt[Key].Text[0]);
			break;
		case 30:
		case 35:
			lstrcpy(LVCommandText, PathFindFileName(CommandExt[Key].Text[0]));
			break;
		case 40:
			for(i=0; i<MAX_ZORDER; i++)
				if(ZorderList[i].Number == Key)
					lstrcpy(LVCommandText, ZorderList[i].Name);
			break;
		case 41:
		case 42:
		case 55:
		case 58:
		case 59:
			wsprintf(LVCommandText, TEXT("X : %d  Y : %d"), (SHORT)HIWORD(Key), (SHORT)LOWORD(Key));
			break;
		case 50:
			wsprintf(LVCommandText, TEXT("%d"), abs(Key));
			break;
		case 60:
			if((SHORT)HIWORD(Key) > 0)
				lstrcpy(LVCommandText, TEXT("縦"));
			else
				lstrcpy(LVCommandText, TEXT("横"));
			switch(abs((SHORT)HIWORD(Key)) - 1)
			{
			case SB_PAGELEFT:
				lstrcat(LVCommandText, TEXT("  -1 Page"));
				break;
			case SB_PAGERIGHT:
				lstrcat(LVCommandText, TEXT("  1 Page"));
				break;
			case SB_LINELEFT:
				wsprintf(LVCommandText, TEXT("%s  -%d Line"), LVCommandText, LOWORD(Key));
				break;
			case SB_LINERIGHT:
				wsprintf(LVCommandText, TEXT("%s  %d Line"), LVCommandText, LOWORD(Key));
				break;
			}
			break;
		case 65:
			wsprintf(LVCommandText, TEXT("Line : %d  Page : %d"), (SHORT)HIWORD(Key), (SHORT)LOWORD(Key));
			break;
		case 70:
		case 71:
			wsprintf(LVCommandText, TEXT("%s,%s,%s"), CommandExt[Key].Text[0], CommandExt[Key].Text[1], CommandExt[Key].Text[2]);
			break;
		default:
			lstrcpy(LVCommandText, TEXT(""));
			break;
		}
	}

	LV_ITEM item;

	item.mask = LVIF_TEXT;
	item.pszText = CaseText;
	item.iItem = intIndex;
	item.iSubItem = 0;
	if(ListView_GetItemCount(hLVC) > intIndex && Insert == 0)
		ListView_SetItem(hLVC, &item);
	else
		ListView_InsertItem(hLVC, &item);

	item.pszText = LVCommandText;
	item.iItem = intIndex;
	item.iSubItem = 1;
	ListView_SetItem(hLVC, &item);
}

UINT SendKeyCheck(UINT KeyCode)
{
	UINT modifiers = 0;

	if(KeyCode == VK_RWIN)
		KeyCode = VK_LWIN;
	if(GetKeyState( VK_SHIFT ) & 0x8000 && KeyCode != VK_SHIFT)
		modifiers |= HOTKEYF_SHIFT;
	if(GetKeyState( VK_CONTROL ) & 0x8000 && KeyCode != VK_CONTROL)
		modifiers |= HOTKEYF_CONTROL;
	if(GetKeyState( VK_MENU ) & 0x8000 && KeyCode != VK_MENU)
		modifiers |= HOTKEYF_ALT;
	if((GetKeyState( VK_LWIN ) & 0x8000 || GetKeyState( VK_RWIN ) & 0x8000) && KeyCode != VK_LWIN)
		modifiers |= HOTKEYF_WIN;

	return MAKEWORD(KeyCode, modifiers);
}

void EnableCommandEdit(HWND hDlg, int Case)
{
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_SENDKEY),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_SENDKEY),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_BUTTONCODE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_BUTTONCODE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BUTTONCAPTURE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COPY),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_COPY),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_FILEPATH),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_FULLPATH),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_FILEPATH),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_FILESEARCH),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_CMDLINE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_CMDLINE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ZORDER),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_ZORDER),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POS),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POSX),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POSY),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_POSX),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_POSY),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_WINDOWCAPTURE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ALPHA1),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ALPHA2),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ALPHA3),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_ALPHA),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_SLIDER_ALPHA),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECK_ALPHA),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_SCROLL),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_SCROLL),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_SLIDER_SCROLL),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_SCROLL),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_MESSAGE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_MESSAGE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_MESSAGE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WPARAM),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_WPARAM),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_WPARAM),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_LPARAM),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_LPARAM),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_LPARAM),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_CHECK_CONTROL),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COMMANDTARGET),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_COMBO_COMMANDTARGET),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_S_SCROLL),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_S_SCROLLLINE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_S_SCROLLLINE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_STATIC_S_SCROLLPAGE),FALSE);
	EnableWindow(GetDlgItem(hDlg,IDC_EDIT_S_SCROLLPAGE),FALSE);
	switch(Case)
	{
	case 1:
		break;
	case 10:
	case 11:
	case 12:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_SENDKEY),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_SENDKEY),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_COMBO_SENDKEY));
		break;
	case 20:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_BUTTONCODE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_BUTTONCODE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_BUTTONCAPTURE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COMMANDTARGET),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_COMMANDTARGET),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_BUTTONCODE));
		break;
	case 25:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COPY),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_COPY),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_COPY));
		break;
	case 30:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_FILEPATH),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_FULLPATH),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_FILEPATH),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_FILESEARCH),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_CMDLINE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_CMDLINE),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_FILEPATH));
		break;
	case 35:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_FILEPATH),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_FILEPATH),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_FILESEARCH),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_FILEPATH));
		break;
	case 40:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ZORDER),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_ZORDER),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COMMANDTARGET),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_COMMANDTARGET),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_COMBO_ZORDER));
		break;
	case 41:
	case 42:
	case 55:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POS),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POSX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POSY),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_POSX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_POSY),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_WINDOWCAPTURE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COMMANDTARGET),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_COMMANDTARGET),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_POSX));
		break;
	case 58:
	case 59:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POS),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POSX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_POSY),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_POSX),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_POSY),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_POSX));
		break;
	case 50:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ALPHA1),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ALPHA2),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_ALPHA3),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_ALPHA),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_SLIDER_ALPHA),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_CHECK_ALPHA),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COMMANDTARGET),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_COMMANDTARGET),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_SLIDER_ALPHA));
		break;
	case 60:
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_SCROLL),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_SCROLL),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_SLIDER_SCROLL),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_SCROLL),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_SLIDER_SCROLL));
		break;
	case 65:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_S_SCROLL),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_S_SCROLLLINE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_S_SCROLLLINE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_S_SCROLLPAGE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_S_SCROLLPAGE),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_STATIC_S_SCROLLLINE));
		break;
	case 70:
	case 71:
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_MESSAGE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_MESSAGE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_MESSAGE),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_WPARAM),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_WPARAM),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_WPARAM),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_LPARAM),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_EDIT_LPARAM),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_LPARAM),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_STATIC_COMMANDTARGET),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_COMBO_COMMANDTARGET),TRUE);
		EnableWindow(GetDlgItem(hDlg,IDC_CHECK_CONTROL),TRUE);
		SetFocus(GetDlgItem(hDlg,IDC_EDIT_MESSAGE));
		break;
	}
}

void ShowCommandDialogCtrl(HWND hDlg, BOOL bShow)
{
	int nCmdShow = bShow ? SW_SHOWNORMAL : SW_HIDE;
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_SENDKEY), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_COMBO_SENDKEY), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_BUTTONCODE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_BUTTONCODE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_BUTTONCAPTURE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_COPY), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_COPY), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_FILEPATH), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_FULLPATH), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_FILEPATH), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_FILESEARCH), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_CMDLINE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_CMDLINE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_ZORDER), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_COMBO_ZORDER), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_POS), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_POSX), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_POSY), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_POSX), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_POSY), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_WINDOWCAPTURE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_ALPHA1), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_ALPHA2), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_ALPHA3), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_ALPHA), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_SLIDER_ALPHA), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_CHECK_ALPHA), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_COMBO_SCROLL), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_SCROLL), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_SLIDER_SCROLL), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_SCROLL), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_MESSAGE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_MESSAGE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_COMBO_MESSAGE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_WPARAM), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_WPARAM), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_COMBO_WPARAM), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_LPARAM), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_LPARAM), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_COMBO_LPARAM), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_CHECK_CONTROL), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_S_SCROLL), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_S_SCROLLLINE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_S_SCROLLLINE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_STATIC_S_SCROLLPAGE), nCmdShow);
	ShowWindow(GetDlgItem(hDlg, IDC_EDIT_S_SCROLLPAGE), nCmdShow);
}
//コマンド設定ダイアログ
INT_PTR CALLBACK CommandDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bNew;
	static COMMAND *CommandEdit;
	static UINT SendKeyCode;
	static int AlphaScrPos, AlphaScrMax = 100, AlphaScrMin = 0;
	static int ScrollScrPos, ScrollScrMax = 21, ScrollScrMin = -21;
	static CONFIGPARAM *configparam;
	static HWND hDlgPrefs;
	int ComboKeyTable[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x0d,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x19, 0x1b, 0x1c, 0x1d,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2c, 0x2d, 0x2e,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
		0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5d, 0x5f,
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b,
		0x90, 0x91,
		0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
		0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
		0xc0,
		0xdb, 0xdc, 0xdd, -1};
	TCHAR FilePath[_MAX_PATH + 1];
	int i, FindCaseNumber = 0;
	TCHAR EditString[20], WaitString[10], KeyName[_MAX_PATH + 1];
	TCHAR TempMainText[_MAX_PATH + 1], TempSubText[_MAX_PATH + 1];
	COMMANDEXTINFO c = {0};

	switch (msg)
	{
	case WM_INITDIALOG:
		CommandEdit = (COMMAND *)lParam;
		bNew = (CommandEdit->Locate == CommandEdit->Repeat) ? TRUE : FALSE;
		SendKeyCode = 0;
		AlphaScrPos = 50;
		ScrollScrPos = 0;
		configparam = (CONFIGPARAM*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, plugins.size()*sizeof(CONFIGPARAM));
		if(!bNew)
		{
			if(CommandEdit->command[CommandEdit->Locate].Case==1)
			{
				i = FindPlugin(CommandExt[CommandEdit->command[CommandEdit->Locate].Key].path);
				if(i!=-1)
				{
					configparam[i].bInitialized = TRUE;
					configparam[i].argv.nValue[0] = CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[0];
					configparam[i].argv.nValue[1] = CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[1];
					configparam[i].argv.nValue[2] = CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[2];
					configparam[i].argv.nValue[3] = CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[3];
					lstrcpy(configparam[i].argv.szText[0], CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[0]);
					lstrcpy(configparam[i].argv.szText[1], CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[1]);
					lstrcpy(configparam[i].argv.szText[2], CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[2]);
					lstrcpy(configparam[i].argv.szText[3], CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[3]);
				}
			}
		}
		hDlgPrefs = NULL;

		//待ち時間
		SetDlgItemInt(hDlg, IDC_EDIT_WAIT, CommandEdit->command[CommandEdit->Locate].Wait, TRUE);
		//キーボードフック
		SetSendKeyComboHook(hDlg, GetWindow(GetDlgItem(hDlg, IDC_COMBO_SENDKEY), GW_CHILD));
		//コマンドのコンボボックス
		for ( i = 0; i < MAX_ACTION_CASE; i++ )
			SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND, CB_INSERTSTRING, (WPARAM)i, (LPARAM)CommandList[i].Name);
		for(i=0;i<(int)plugins.size();i++)
			SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND, CB_INSERTSTRING, (WPARAM)SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND,  CB_GETCOUNT, 0, 0), (LPARAM)plugins[i].plugin->name);
		if(CommandEdit->command[CommandEdit->Locate].Case==1)
		{
			i = FindPlugin(CommandExt[CommandEdit->command[CommandEdit->Locate].Key].path);
			if(i==-1)
			{
				//インストールされていないプラグイン
			}
			else
			{
				FindCaseNumber = MAX_ACTION_CASE+i;
			}
		}
		else
		{
			for ( i = 0; i < MAX_ACTION_CASE; i++ )
			{
				if(CommandEdit->command[CommandEdit->Locate].Case == CommandList[i].Number)
					FindCaseNumber = i;
			}
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND, CB_SETCURSEL, (WPARAM)FindCaseNumber, 0L);
		//キーのコンボボックス
		i = 0;
		while(ComboKeyTable[i]!=-1)
		{
			get_keyname(ComboKeyTable[i], KeyName, _MAX_PATH+1);
			SendDlgItemMessage(hDlg, IDC_COMBO_SENDKEY, CB_INSERTSTRING, SendDlgItemMessage(hDlg, IDC_COMBO_SENDKEY,  CB_GETCOUNT, 0, 0), (LPARAM)KeyName);
			i++;
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_SENDKEY, CB_SETCURSEL, 0L, 0L);
		//表示方法のコンボボックス
		for ( i = 0; i < MAX_ZORDER; i++ )
			SendDlgItemMessage(hDlg, IDC_COMBO_ZORDER, CB_INSERTSTRING, (WPARAM)i, (LPARAM)ZorderList[i].Name);
		SendDlgItemMessage(hDlg, IDC_COMBO_ZORDER, CB_SETCURSEL, 0L, 0L);
		//透明度
		SendDlgItemMessage(hDlg, IDC_SLIDER_ALPHA, TBM_SETRANGE, TRUE, MAKELPARAM(AlphaScrMin, AlphaScrMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_ALPHA, TBM_SETPAGESIZE, 0, 10);
		//スクロール
		SendDlgItemMessage(hDlg, IDC_COMBO_SCROLL, CB_INSERTSTRING, (WPARAM)0, (LPARAM)TEXT("縦"));
		SendDlgItemMessage(hDlg, IDC_COMBO_SCROLL, CB_INSERTSTRING, (WPARAM)1, (LPARAM)TEXT("横"));
		SendDlgItemMessage(hDlg, IDC_COMBO_SCROLL, CB_SETCURSEL, 0L, 0L);
		SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLL, TBM_SETRANGE, TRUE, MAKELPARAM(ScrollScrMin, ScrollScrMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLL, TBM_SETPAGESIZE, 0, 5);
		//Message
		SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_INSERTSTRING, (WPARAM)0, (LPARAM)TEXT("10進"));
		SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_INSERTSTRING, (WPARAM)1, (LPARAM)TEXT("16進"));
		SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_INSERTSTRING, (WPARAM)2, (LPARAM)TEXT("TEXT"));
		SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_SETCURSEL, 1, 0L);
		//wParam
		SendDlgItemMessage(hDlg, IDC_COMBO_WPARAM, CB_INSERTSTRING, (WPARAM)0, (LPARAM)TEXT("10進"));
		SendDlgItemMessage(hDlg, IDC_COMBO_WPARAM, CB_INSERTSTRING, (WPARAM)1, (LPARAM)TEXT("16進"));
		SendDlgItemMessage(hDlg, IDC_COMBO_WPARAM, CB_INSERTSTRING, (WPARAM)2, (LPARAM)TEXT("TEXT"));
		SendDlgItemMessage(hDlg, IDC_COMBO_WPARAM, CB_SETCURSEL, 1, 0L);
		//lParam
		SendDlgItemMessage(hDlg, IDC_COMBO_LPARAM, CB_INSERTSTRING, (WPARAM)0, (LPARAM)TEXT("10進"));
		SendDlgItemMessage(hDlg, IDC_COMBO_LPARAM, CB_INSERTSTRING, (WPARAM)1, (LPARAM)TEXT("16進"));
		SendDlgItemMessage(hDlg, IDC_COMBO_LPARAM, CB_INSERTSTRING, (WPARAM)2, (LPARAM)TEXT("TEXT"));
		SendDlgItemMessage(hDlg, IDC_COMBO_LPARAM, CB_SETCURSEL, 1, 0L);
		//スクロールに必要な移動量
		SetDlgItemInt(hDlg, IDC_EDIT_S_SCROLLLINE, 5, TRUE);
		SetDlgItemInt(hDlg, IDC_EDIT_S_SCROLLPAGE, 10, TRUE);
		//対象
		SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_INSERTSTRING, (WPARAM)0, (LPARAM)TEXT("アクティブウィンドウ"));
		SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_INSERTSTRING, (WPARAM)1, (LPARAM)TEXT("アクション開始ウィンドウ"));
		for ( i = 0; i < (int)Target.size(); i++ )
		{
			if(Target[i].Comment[0])
			{
				lstrcpyn(TempMainText, Target[i].Comment, 31);
			}
			else
			{
				lstrcpyn(TempMainText, Target[i].FileName, 31);
				if(TempMainText[0] == 0)
					lstrcpy(TempMainText, TEXT("---"));
				if(Target[i].ControlID[0])
				{
					if(Target[i].ClassName[0])
					{
						lstrcpyn(TempSubText, Target[i].ClassName, 17);
						wsprintf(TempSubText, TEXT("%s ID:%s"), TempSubText, Target[i].ControlID);
					}
					else
					{
						wsprintf(TempSubText, TEXT("ID:%s"), Target[i].ControlID);
					}
				}
				else
				{
					lstrcpyn(TempSubText, Target[i].ClassName, 31);
				}
				wsprintf(TempMainText, TEXT("%s %s"), TempMainText, TempSubText);
			}
			SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_INSERTSTRING, (WPARAM)i + 2, (LPARAM)TempMainText);
		}
		switch(CommandEdit->command[CommandEdit->Locate].CommandTarget)
		{
		case -1:
			SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_SETCURSEL, 1, 0L);
			break;
		case 0:
			SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_SETCURSEL, 0L, 0L);
			break;
		default:
			for(i = 0; i < (int)Target.size(); i++)
			{
				if(CommandEdit->command[CommandEdit->Locate].CommandTarget == Target[i].Number)
					SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_SETCURSEL, (WPARAM)i + 2, 0L);
			}
			break;
		}

		switch(CommandEdit->command[CommandEdit->Locate].Case)
		{
		case 1:
			break;
		case 10:
		case 11:
		case 12:
			SendKeyCode = CommandEdit->command[CommandEdit->Locate].Key;
			get_keyname(SendKeyCode, KeyName, _MAX_PATH+1);
			SendMessage(GetWindow(GetDlgItem(hDlg, IDC_COMBO_SENDKEY), GW_CHILD), WM_SETTEXT, 0L, (LPARAM)KeyName);
			SendMessage(GetWindow(GetDlgItem(hDlg, IDC_COMBO_SENDKEY), GW_CHILD), EM_SETSEL, lstrlen(KeyName), lstrlen(KeyName));
			break;
		case 20:
			SetDlgItemInt(hDlg, IDC_EDIT_BUTTONCODE, CommandEdit->command[CommandEdit->Locate].Key, TRUE);
			break;
		case 30:
			SetDlgItemText(hDlg, IDC_EDIT_FILEPATH, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[0]);
			SetDlgItemText(hDlg, IDC_EDIT_CMDLINE, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[1]);
			break;
		case 25:
			SetDlgItemText(hDlg, IDC_EDIT_COPY, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[0]);
			break;
		case 35:
			SetDlgItemText(hDlg, IDC_EDIT_FILEPATH, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[0]);
			break;
		case 40:
			for ( i = 0; i < MAX_ZORDER; i++ )
				if(ZorderList[i].Number == CommandEdit->command[CommandEdit->Locate].Key)
					SendDlgItemMessage(hDlg, IDC_COMBO_ZORDER, CB_SETCURSEL, (WPARAM)i, 0L);
			break;
		case 41:
		case 42:
		case 55:
		case 58:
		case 59:
			SetDlgItemInt(hDlg, IDC_EDIT_POSX, (SHORT)HIWORD(CommandEdit->command[CommandEdit->Locate].Key), TRUE);
			SetDlgItemInt(hDlg, IDC_EDIT_POSY, (SHORT)LOWORD(CommandEdit->command[CommandEdit->Locate].Key), TRUE);
			break;
		case 50:
			AlphaScrPos = abs(CommandEdit->command[CommandEdit->Locate].Key);
			break;
		case 60:
			if((SHORT)HIWORD(CommandEdit->command[CommandEdit->Locate].Key) < 0)
				SendDlgItemMessage(hDlg, IDC_COMBO_SCROLL, CB_SETCURSEL, 1, 0L);
			switch(abs((SHORT)HIWORD(CommandEdit->command[CommandEdit->Locate].Key)) - 1)
			{
			case SB_PAGELEFT:
				ScrollScrPos = ScrollScrMin;
				break;
			case SB_PAGERIGHT:
				ScrollScrPos = ScrollScrMax;
				break;
			case SB_LINELEFT:
				ScrollScrPos = LOWORD(CommandEdit->command[CommandEdit->Locate].Key) * -1;
				break;
			case SB_LINERIGHT:
				ScrollScrPos = LOWORD(CommandEdit->command[CommandEdit->Locate].Key);
				break;
			}
			break;
		case 65:
			SetDlgItemInt(hDlg, IDC_EDIT_S_SCROLLLINE, (SHORT)HIWORD(CommandEdit->command[CommandEdit->Locate].Key), TRUE);
			SetDlgItemInt(hDlg, IDC_EDIT_S_SCROLLPAGE, (SHORT)LOWORD(CommandEdit->command[CommandEdit->Locate].Key), TRUE);
			break;
		case 70:
		case 71:
			if(CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[0] < 100)
			{
				SendDlgItemMessage(hDlg, IDC_CHECK_CONTROL, BM_SETCHECK, 1, 0);
				SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_SETCURSEL, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[0], 0L);
			}
			else
			{
				SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_SETCURSEL, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[0] - 100, 0L);
			}
			SetDlgItemText(hDlg, IDC_EDIT_MESSAGE, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[0]);
			SetDlgItemText(hDlg, IDC_EDIT_WPARAM, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[1]);
			SendDlgItemMessage(hDlg, IDC_COMBO_WPARAM, CB_SETCURSEL, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[1], 0L);
			SetDlgItemText(hDlg, IDC_EDIT_LPARAM, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Text[2]);
			SendDlgItemMessage(hDlg, IDC_COMBO_LPARAM, CB_SETCURSEL, CommandExt[CommandEdit->command[CommandEdit->Locate].Key].Key[2], 0L);
			break;
		}

		//透明度
		if(CommandEdit->command[CommandEdit->Locate].Case == 50 && CommandEdit->command[CommandEdit->Locate].Key < 0)
			SendDlgItemMessage(hDlg, IDC_CHECK_ALPHA, BM_SETCHECK, 0, 0);
		else
			SendDlgItemMessage(hDlg, IDC_CHECK_ALPHA, BM_SETCHECK, 1, 0);
		SendDlgItemMessage(hDlg, IDC_SLIDER_ALPHA, TBM_SETPOS, TRUE, AlphaScrPos);
		SetDlgItemInt(hDlg, IDC_EDIT_ALPHA, AlphaScrPos, TRUE);
		//スクロール
		SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLL, TBM_SETPOS, TRUE, ScrollScrPos);
		if(ScrollScrPos == ScrollScrMin || ScrollScrPos == ScrollScrMax)
			wsprintf(EditString, TEXT("%d Page"), ScrollScrPos / abs(ScrollScrPos));
		else
			wsprintf(EditString, TEXT("%d Line"), ScrollScrPos);
		SetDlgItemText(hDlg, IDC_EDIT_SCROLL, EditString);
		//CBN_SELCHANGEを送る
		SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_COMBO_COMMAND, CBN_SELCHANGE), (LPARAM)GetDlgItem(hDlg, IDC_COMBO_COMMAND));
		return FALSE;
	case WM_DESTROY:
		if(hDlgPrefs)
		{
			DestroyWindow(hDlgPrefs);
			hDlgPrefs = NULL;
		}
		HeapFree(GetProcessHeap(), 0, configparam);
		ButtonSearchUnHook();
		SetSendKeyComboUnHook();
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_COMBO_COMMAND:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				//プラグインのダイアログがあったら破棄する
				if(hDlgPrefs)
				{
					PSHNOTIFY psn;
					psn.hdr.code = PSN_APPLY;
					SendMessage(hDlgPrefs, WM_NOTIFY, 0, (LPARAM)&psn);	//PSN_APPLYを送る
					DestroyWindow(hDlgPrefs);
					hDlgPrefs = NULL;
				}
				i = (int)SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND, CB_GETCURSEL, 0L, 0L);
				if(i<MAX_ACTION_CASE)
				{
					//コントロールを表示
					ShowCommandDialogCtrl(hDlg, TRUE);
					EnableCommandEdit(hDlg, CommandList[i].Number);
				}
				else
				{
					//プラグインのとき
					PREFS prefs;
					RECT rc;
					POINT pt;
					//コントロールを非表示
					ShowCommandDialogCtrl(hDlg, FALSE);
					EnableCommandEdit(hDlg, 1);
					if(plugins[i-MAX_ACTION_CASE].plugin->flags&0x80000000)	//対象を有効にする
					{
						EnableWindow(GetDlgItem(hDlg, IDC_STATIC_COMMANDTARGET), TRUE);
						EnableWindow(GetDlgItem(hDlg, IDC_COMBO_COMMANDTARGET), TRUE);
					}
					//ダイアログを作成
					ZeroMemory(&prefs, sizeof(PREFS));
					plugins[i-MAX_ACTION_CASE].plugin->Config(&prefs, &configparam[i-MAX_ACTION_CASE]);
					hDlgPrefs = CreateDialogParam(prefs.hInstance, MAKEINTRESOURCE(prefs.id), hDlg, prefs.DlgProc, prefs.lParam);
					configparam[i-MAX_ACTION_CASE].bInitialized = TRUE;
					//ダイアログを移動
					GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_BACKGROUND), &rc);
					pt.x = rc.left;
					pt.y = rc.top;
					ScreenToClient(hDlg, &pt);
					SetWindowPos(hDlgPrefs, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
				}
			}
			break;
		case IDC_COMBO_SENDKEY:
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				if(SendDlgItemMessage(hDlg, IDC_COMBO_SENDKEY, CB_GETCURSEL, 0L, 0L) > 0)
				{
					SendKeyCode = SendKeyCheck(ComboKeyTable[SendDlgItemMessage(hDlg, IDC_COMBO_SENDKEY, CB_GETCURSEL, 0L, 0L)]);
					PostMessage(hDlg, WM_COMBO_REFRESH, 0, 0);
				}
			}
			break;
		case IDC_BUTTON_BUTTONCAPTURE:
			SetDlgItemText(hDlg, IDC_EDIT_BUTTONCODE, TEXT("ツールバーボタンをクリック"));
			SetButtonSearchHook(GetDlgItem(hDlg,IDC_EDIT_BUTTONCODE));
			break;
		case IDC_BUTTON_WINDOWCAPTURE:
			switch(CommandList[SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND, CB_GETCURSEL, 0L, 0L)].Number)
			{
			case 41:
				SetDlgItemText(hDlg, IDC_EDIT_POSX, TEXT("ﾀｲﾄﾙﾊﾞｰ"));
				SetDlgItemText(hDlg, IDC_EDIT_POSY, TEXT("ｸﾘｯｸ"));
				SetWindowHook(GetDlgItem(hDlg,IDC_EDIT_POSX),GetDlgItem(hDlg,IDC_EDIT_POSY),2);
				break;
			case 42:
				SetDlgItemText(hDlg, IDC_EDIT_POSX, TEXT("ﾀｲﾄﾙﾊﾞｰ"));
				SetDlgItemText(hDlg, IDC_EDIT_POSY, TEXT("ｸﾘｯｸ"));
				SetWindowHook(GetDlgItem(hDlg,IDC_EDIT_POSX),GetDlgItem(hDlg,IDC_EDIT_POSY),3);
				break;
			case 55:
				SetDlgItemText(hDlg, IDC_EDIT_POSX, TEXT("ｲﾁｦｱﾜｾ"));
				SetDlgItemText(hDlg, IDC_EDIT_POSY, TEXT("R ｸﾘｯｸ"));
				SetWindowHook(GetDlgItem(hDlg,IDC_EDIT_POSX),GetDlgItem(hDlg,IDC_EDIT_POSY),4);
				break;
			}
			break;
		case IDC_BUTTON_FULLPATH:
			GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, FilePath, _MAX_PATH);
			if(!FilePath[0])
				break;
			if(PathIsRelative(FilePath))
			{	//絶対パスではないとき、絶対パスに変換
				TCHAR szDir[MAX_PATH];
				TCHAR szPath[MAX_PATH];
				GetModuleFileName(NULL, szDir, MAX_PATH);
				PathRemoveFileSpec(szDir);
				wsprintf(szPath, TEXT("%s\\%s"), szDir, FilePath);
				PathCanonicalize(FilePath, szPath);
			}
			else
			{
				//絶対パスを相対パスに変換
				TCHAR szExePath[MAX_PATH];
				TCHAR szPath[MAX_PATH];
				GetModuleFileName(NULL, szExePath, MAX_PATH);
				lstrcpy(szPath, FilePath);
				PathRelativePathTo(FilePath, szExePath, FILE_ATTRIBUTE_ARCHIVE, szPath, FILE_ATTRIBUTE_ARCHIVE);
			}
			SetDlgItemText(hDlg, IDC_EDIT_FILEPATH, FilePath);
			break;
		case IDC_BUTTON_FILESEARCH:
			GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, FilePath, _MAX_PATH);
			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(OPENFILENAME));    
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner   = hDlg;
			ofn.lpstrFile   = FilePath; //選択されたファイル名を受け取る(フルパス)
			ofn.nMaxFile    = _MAX_PATH;
			ofn.lpstrFilter = TEXT("all(*.*)\0*.*\0\0"); //フィルタ
			ofn.lpstrTitle  = TEXT("実行ファイル選択"); //ダイアログボックスのタイトル
			ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			GetOpenFileName(&ofn);
			SetDlgItemText(hDlg, IDC_EDIT_FILEPATH, FilePath);
			break;
		case IDOK:
			int index;
			if(hDlgPrefs)
			{
				PSHNOTIFY psn;
				psn.hdr.code = PSN_APPLY;
				SendMessage(hDlgPrefs, WM_NOTIFY, 0, (LPARAM)&psn);	//PSN_APPLYを送る
				DestroyWindow(hDlgPrefs);
				hDlgPrefs = NULL;
			}
			index = (int)SendDlgItemMessage(hDlg, IDC_COMBO_COMMAND, CB_GETCURSEL, 0L, 0L);
			if(index < 0)
				break;
			GetDlgItemText(hDlg, IDC_EDIT_WAIT, WaitString, 10);
			CommandEdit->command[CommandEdit->Locate].Wait = StrToInt(WaitString);
			if(index<MAX_ACTION_CASE)
				CommandEdit->command[CommandEdit->Locate].Case = CommandList[index].Number;
			else
				CommandEdit->command[CommandEdit->Locate].Case = 1;
			switch(CommandEdit->command[CommandEdit->Locate].Case)
			{
			case 1:
				CommandEdit->command[CommandEdit->Locate].Key = (LONG)CommandExt.size();
				lstrcpy(c.path, plugins[index-MAX_ACTION_CASE].szPath);
				c.Key[0] = configparam[index-MAX_ACTION_CASE].argv.nValue[0];
				c.Key[1] = configparam[index-MAX_ACTION_CASE].argv.nValue[1];
				c.Key[2] = configparam[index-MAX_ACTION_CASE].argv.nValue[2];
				c.Key[3] = configparam[index-MAX_ACTION_CASE].argv.nValue[3];
				lstrcpy(c.Text[0], configparam[index-MAX_ACTION_CASE].argv.szText[0]);
				lstrcpy(c.Text[1], configparam[index-MAX_ACTION_CASE].argv.szText[1]);
				lstrcpy(c.Text[2], configparam[index-MAX_ACTION_CASE].argv.szText[2]);
				lstrcpy(c.Text[3], configparam[index-MAX_ACTION_CASE].argv.szText[3]);
				CommandExt.push_back(c);
				break;
			case 10:
			case 11:
			case 12:
				CommandEdit->command[CommandEdit->Locate].Key = SendKeyCode;
				break;
			case 20:
				GetDlgItemText(hDlg, IDC_EDIT_BUTTONCODE, EditString, 11);
				CommandEdit->command[CommandEdit->Locate].Key = StrToInt(EditString);
				break;
			case 25:
				CommandEdit->command[CommandEdit->Locate].Key = (LONG)CommandExt.size();
				GetDlgItemText(hDlg, IDC_EDIT_COPY, c.Text[0], _MAX_PATH);
				CommandExt.push_back(c);
				break;
			case 30:
				CommandEdit->command[CommandEdit->Locate].Key = (LONG)CommandExt.size();
				GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, c.Text[0], _MAX_PATH);
				GetDlgItemText(hDlg, IDC_EDIT_CMDLINE, c.Text[1], _MAX_PATH);
				CommandExt.push_back(c);
				break;
			case 35:
				CommandEdit->command[CommandEdit->Locate].Key = (LONG)CommandExt.size();
				GetDlgItemText(hDlg, IDC_EDIT_FILEPATH, c.Text[0], _MAX_PATH);
				CommandExt.push_back(c);
				break;
			case 40:
				CommandEdit->command[CommandEdit->Locate].Key = ZorderList[(LONG)SendDlgItemMessage(hDlg, IDC_COMBO_ZORDER, CB_GETCURSEL, 0L, 0L)].Number;
				break;
			case 41:
			case 42:
			case 55:
			case 58:
			case 59:
				GetDlgItemText(hDlg, IDC_EDIT_POSX, EditString, 6);
				GetDlgItemText(hDlg, IDC_EDIT_POSY, WaitString, 6);
				CommandEdit->command[CommandEdit->Locate].Key = MAKELONG(StrToInt(WaitString),StrToInt(EditString));
				break;
			case 50:
				if(SendDlgItemMessage(hDlg, IDC_CHECK_ALPHA, BM_GETCHECK , 0L , 0L) == 0)
					CommandEdit->command[CommandEdit->Locate].Key = AlphaScrPos * -1;
				else
					CommandEdit->command[CommandEdit->Locate].Key = AlphaScrPos;
				break;
			case 60:
				if(ScrollScrPos == ScrollScrMin)
				{
					i = SB_PAGELEFT;
					ScrollScrPos = 0;
				}
				else if(ScrollScrPos == ScrollScrMax)
				{
					i = SB_PAGERIGHT;
					ScrollScrPos = 0;
				}
				else if(ScrollScrPos < 0)
				{
					i = SB_LINELEFT;
					ScrollScrPos = abs(ScrollScrPos);
				}
				else
				{
					i = SB_LINERIGHT;
					ScrollScrPos = abs(ScrollScrPos);
				}
				i++;
				if(SendDlgItemMessage(hDlg, IDC_COMBO_SCROLL, CB_GETCURSEL, 0L, 0L) > 0)
					i = i * -1;
				CommandEdit->command[CommandEdit->Locate].Key = MAKELONG(ScrollScrPos, i);
				break;
			case 65:
				GetDlgItemText(hDlg, IDC_EDIT_S_SCROLLLINE, EditString, 6);
				GetDlgItemText(hDlg, IDC_EDIT_S_SCROLLPAGE, WaitString, 6);
				CommandEdit->command[CommandEdit->Locate].Key = MAKELONG(StrToInt(WaitString),StrToInt(EditString));
				break;
			case 70:
			case 71:
				CommandEdit->command[CommandEdit->Locate].Key = (LONG)CommandExt.size();
				GetDlgItemText(hDlg, IDC_EDIT_MESSAGE, c.Text[0], _MAX_PATH);
				c.Key[0] = (int)SendDlgItemMessage(hDlg, IDC_COMBO_MESSAGE, CB_GETCURSEL, 0L, 0L);
				GetDlgItemText(hDlg, IDC_EDIT_WPARAM, c.Text[1], _MAX_PATH);
				c.Key[1] = (int)SendDlgItemMessage(hDlg, IDC_COMBO_WPARAM, CB_GETCURSEL, 0L, 0L);
				GetDlgItemText(hDlg, IDC_EDIT_LPARAM, c.Text[2], _MAX_PATH);
				c.Key[2] = (int)SendDlgItemMessage(hDlg, IDC_COMBO_LPARAM, CB_GETCURSEL, 0L, 0L);
				if(SendDlgItemMessage(hDlg, IDC_CHECK_CONTROL, BM_GETCHECK , 0L , 0L) == 0)
					c.Key[0] += 100;
				CommandExt.push_back(c);
				break;
			}
			switch(SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_GETCURSEL, 0L, 0L))
			{
			case 0:
				CommandEdit->command[CommandEdit->Locate].CommandTarget = 0;
				break;
			case 1:
				CommandEdit->command[CommandEdit->Locate].CommandTarget = -1;
				break;
			default:
				CommandEdit->command[CommandEdit->Locate].CommandTarget = Target[(int)SendDlgItemMessage(hDlg, IDC_COMBO_COMMANDTARGET, CB_GETCURSEL, 0L, 0L) - 2].Number;
				break;
			}
			if(bNew)	//追加
				CommandEdit->Repeat ++;	//コマンドの数を増やす
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	case WM_HSCROLL:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_SLIDER_ALPHA:
			AlphaScrPos = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_ALPHA, TBM_GETPOS, 0, 0);
			if (AlphaScrPos < AlphaScrMin)
				AlphaScrPos = AlphaScrMin;
			if (AlphaScrPos > AlphaScrMax)
				AlphaScrPos = AlphaScrMax;
			SetDlgItemInt(hDlg, IDC_EDIT_ALPHA, AlphaScrPos, TRUE);
			break;
		case IDC_SLIDER_SCROLL:
			ScrollScrPos = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLL, TBM_GETPOS, 0, 0);
			if (ScrollScrPos < ScrollScrMin)
				ScrollScrPos = ScrollScrMin;
			if (ScrollScrPos > ScrollScrMax)
				ScrollScrPos = ScrollScrMax;
			if(ScrollScrPos == ScrollScrMin || ScrollScrPos == ScrollScrMax)
				wsprintf(EditString, TEXT("%d Page"), ScrollScrPos / abs(ScrollScrPos));
			else
				wsprintf(EditString, TEXT("%d Line"), ScrollScrPos);
			SetDlgItemText(hDlg, IDC_EDIT_SCROLL, EditString);
			break;
		}
		break;
	case PSM_CHANGED:
		break;
	case WM_COMBO_REFRESH:
		get_keyname(SendKeyCode, KeyName, _MAX_PATH+1);
		SendMessage(GetWindow(GetDlgItem(hDlg, IDC_COMBO_SENDKEY), GW_CHILD), WM_SETTEXT, 0L, (LPARAM)KeyName);
		SendMessage(GetWindow(GetDlgItem(hDlg, IDC_COMBO_SENDKEY), GW_CHILD), EM_SETSEL, lstrlen(KeyName), lstrlen(KeyName));
		break;
	case WM_KEYBOARD_MSG:
		if(!(HIBYTE(SendKeyCode) & HOTKEYF_SHIFT && wParam == VK_SHIFT) &&
			!(HIBYTE(SendKeyCode) & HOTKEYF_CONTROL && wParam == VK_CONTROL) &&
			!(HIBYTE(SendKeyCode) & HOTKEYF_ALT && wParam == VK_MENU) &&
			!(HIBYTE(SendKeyCode) & HOTKEYF_WIN && (wParam == VK_LWIN || wParam == VK_RWIN)))
			SendKeyCode = SendKeyCheck((UINT)wParam);
		PostMessage(hDlg, WM_COMBO_REFRESH, 0, 0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void HighlightWindow(HWND hwnd)
{
	HDC hdc;
	RECT rc;
	HPEN hPen, hPenPre;
	HBRUSH hBrush, hBrushPre;

	hdc = GetWindowDC(hwnd);
	GetWindowRect(hwnd, &rc);
	OffsetRect(&rc, -rc.left, -rc.top);
	hPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 0));
	hPenPre = (HPEN)SelectObject(hdc, hPen);
	hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	hBrushPre = (HBRUSH)SelectObject(hdc, hBrush);
	SetROP2(hdc, R2_NOT);
	Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
	SelectObject(hdc, hPenPre);
	DeleteObject(hPen);
	SelectObject(hdc, hBrushPre);
	ReleaseDC(hwnd, hdc);
}
//スタティックコントロールのウィンドウハンドルを取得できるWindowFromPoint
HWND _WindowFromPoint(POINT pt)
{
	HWND hwnd;
	HWND hwndChild;

	hwnd = WindowFromPoint(pt);
	if(hwnd)
	{
		ScreenToClient(hwnd, &pt);
		hwndChild = ChildWindowFromPoint(hwnd, pt);
		if(hwndChild)
			hwnd = hwndChild;
	}
	return hwnd;
}

LRESULT CALLBACK LowLevelKeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code<0)
		return CallNextHookEx(hHookKeyboard, code, wParam, lParam);
	return -1;
}
//ファインダーツールアイコンのプロシージャ
LRESULT CALLBACK WndProcFinder(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bFile, bClass, bID, bWindowTitle;
	static HICON hIcon1;
	static HICON hIcon2;
	static HCURSOR hCursor1;
	static HCURSOR hCursor2;
	static HWND hwndCurrent = NULL;

	switch(msg)
	{
	case WM_DESTROY:
		if(hHookKeyboard)
			UnhookWindowsHookEx(hHookKeyboard);
		hHookKeyboard = NULL;
		break;
	case WM_MOUSEMOVE:
		if(GetCapture()==hwnd)
		{
			POINT pt;
			HWND hwndPoint;
			GetCursorPos(&pt);
			hwndPoint = _WindowFromPoint(pt);
			if(hwndPoint != hwndCurrent)
			{
				if(hwndCurrent)
					HighlightWindow(hwndCurrent);

				hwndCurrent = hwndPoint;
				HighlightWindow(hwndCurrent);
				if(bFile)
				{
					TCHAR szFileName[MAX_PATH];
					TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], fext[_MAX_EXT];
					GetFileNameFromHwnd(hwndCurrent, szFileName, MAX_PATH);
					_tsplitpath_s( szFileName, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT );
					switch(SendDlgItemMessage(GetParent(hwnd), IDC_COMBO_TARGETTYPE, CB_GETCURSEL, 0L, 0L))
					{
					case 0:
						_tmakepath_s( szFileName, _MAX_PATH, NULL, NULL, fname, fext );
						break;
					case 1:
						_tmakepath_s( szFileName, _MAX_PATH, drive, dir, fname, fext );
						break;
					case 2:
						_tmakepath_s( szFileName, _MAX_PATH, NULL, dir, fname, fext );
						break;
					case 3:
						_tmakepath_s( szFileName, _MAX_PATH, drive, NULL, fname, fext );
						break;
					}
					SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGETFILE, szFileName);
				}
				if(bClass)
				{
					TCHAR szClassName[256];
					GetClassName(hwndCurrent, szClassName, 256);
					SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGETCLASS, szClassName);
				}
				if(bID)
				{
					TCHAR szID[256];
					wsprintf(szID, TEXT("%d"), GetWindowLongPtr(hwndCurrent, GWLP_ID));
					SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGETID, szID);
				}
				if(bWindowTitle)
				{
					TCHAR szWindowText[256];
					GetWindowText(GetAncestor(hwndCurrent, GA_ROOT), szWindowText, 256);
					SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGEWINDOWTTITLE, szWindowText);
				}
			}
		}
		return 0;
	case WM_LBUTTONDOWN:
		SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGETFILE, TEXT(""));
		SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGEWINDOWTTITLE, TEXT(""));
		SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGETCLASS, TEXT(""));
		SetDlgItemText(GetParent(hwnd), IDC_EDIT_TARGETID, TEXT(""));
		bFile = (SendDlgItemMessage(GetParent(hwnd), IDC_CHECK_GETFILE, BM_GETCHECK, 0, 0) == BST_CHECKED);
		bClass = (SendDlgItemMessage(GetParent(hwnd), IDC_CHECK_GETCLASS, BM_GETCHECK, 0, 0) == BST_CHECKED);
		bID = (SendDlgItemMessage(GetParent(hwnd), IDC_CHECK_GETID, BM_GETCHECK, 0, 0) == BST_CHECKED);
		bWindowTitle = (SendDlgItemMessage(GetParent(hwnd), IDC_CHECK_GETWINDOWTITLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
		hIcon2 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FINDER2));
		hIcon1 = (HICON)SendMessage(hwnd, STM_SETICON, (WPARAM)hIcon2, 0);
		hCursor1 = LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FINDER));
		hCursor2 = SetCursor(hCursor1);
		SetCapture(hwnd);
		hHookKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	case WM_CAPTURECHANGED:
		UnhookWindowsHookEx(hHookKeyboard);
		hHookKeyboard = NULL;
		SetCursor(hCursor2);
		DestroyCursor(hCursor1);
		SendMessage(hwnd, STM_SETICON, (WPARAM)hIcon1, 0);
		DestroyIcon(hIcon2);
		HighlightWindow(hwndCurrent);
		hwndCurrent = NULL;
		return 0;
	}
	return CallWindowProc(WndProcFinderOld, hwnd, msg, wParam, lParam);
}
//ターゲット設定ダイアログ
INT_PTR CALLBACK TargetDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int nActiveIndex;
	int i, j;
	TCHAR FilePath[_MAX_PATH + 1];
	TCHAR drive[_MAX_DRIVE + 1];
	TCHAR dir[_MAX_DIR + 1];
	TCHAR fname[_MAX_FNAME + 1];
	TCHAR fext[_MAX_EXT + 1];
	TCHAR ClassName[MAX_CLASSNAME + 1];
	TCHAR ControlID[MAX_CLASSNAME + 1];
	TCHAR Title[256];
	static TCHAR TargetType[4][20] = {TEXT("ファイル"), TEXT("フルパス"), TEXT("ルートからのパス"), TEXT("ドライブとファイル")};
	static struct HitTestValue {
		TCHAR Value[30];
		TCHAR Name[30];
	} EtcTarget[12] = {
		{TEXT(""),TEXT("その他の領域")},
		{TEXT(""),TEXT("デスクトップ")},
		{TEXT("HitTestCaptionBar"),TEXT("タイトルバー")},
		{TEXT("HitTestScrollBar"),TEXT("スクロールバー")},
		{TEXT("HitTestBorder"),TEXT("境界線")},
		{TEXT("HitTestMenu"),TEXT("メニュー")},
		{TEXT("HitTestListViewIcon"),TEXT("リストビューアイコン")},
		{TEXT("HitTestTreeViewIcon"),TEXT("ツリービューアイコン")},
		{TEXT("HitTestSystemMenuButton"),TEXT("システムメニューボタン")},
		{TEXT("HitTestMinButton"),TEXT("最小化ボタン")},
		{TEXT("HitTestMaxButton"),TEXT("最大化ボタン")},
		{TEXT("HitTestCloseButton"),TEXT("クローズボタン")},
	};
	TARGETWINDOW t = {0};

	switch (msg)
	{
	case WM_INITDIALOG:
		nActiveIndex = (int)lParam;
		SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_INSERTSTRING, (WPARAM)0, (LPARAM)TEXT("継承しない"));
		SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_INSERTSTRING, (WPARAM)1, (LPARAM)TEXT("Default"));
		for ( i = 0; i < (int)Target.size(); i++ )
		{
			if(Target[i].Comment[0])
			{
				lstrcpyn(FilePath, Target[i].Comment, 15);
			}
			else
			{
				lstrcpyn(FilePath, Target[i].FileName, 15);
				if(FilePath[0] == 0)
					lstrcpy(FilePath, TEXT("---"));
				if(Target[i].ControlID[0])
				{
					if(Target[i].ClassName[0])
					{
						lstrcpyn(ClassName, Target[i].ClassName, 13);
						wsprintf(ClassName, TEXT("%s ID:%s"), ClassName, Target[i].ControlID);
					}
					else
					{
						wsprintf(ClassName, TEXT("ID:%s"), Target[i].ControlID);
					}
				}
				else
				{
					lstrcpyn(ClassName, Target[i].ClassName, 21);
				}
				wsprintf(FilePath, TEXT("%s %s"), FilePath, ClassName);
			}
			SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_INSERTSTRING, (WPARAM)i + 2, (LPARAM)FilePath);
		}
		if(nActiveIndex != -1)
		{
			SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, Target[nActiveIndex].FileName);
			switch(Target[nActiveIndex].DefaultThru)
			{
			case -1:
				SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_SETCURSEL, 0, 0L);
				break;
			case 0:
				SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_SETCURSEL, 1, 0L);
				break;
			default:
				for ( j = 0; j < (int)Target.size(); j++ )
				{
					if(Target[nActiveIndex].DefaultThru == Target[j].Number)
						SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_SETCURSEL, j + 2, 0L);
				}
				break;
			}
			if(Target[nActiveIndex].FileName[0])
				SendDlgItemMessage(hDlg, IDC_CHECK_GETFILE, BM_SETCHECK, 1, 0);
			if(Target[nActiveIndex].WindowTitle[0])
				SendDlgItemMessage(hDlg, IDC_CHECK_GETWINDOWTITLE, BM_SETCHECK, 1, 0);
			if(Target[nActiveIndex].ClassName[0])
				SendDlgItemMessage(hDlg, IDC_CHECK_GETCLASS, BM_SETCHECK, 1, 0);
			if(Target[nActiveIndex].ControlID[0])
				SendDlgItemMessage(hDlg, IDC_CHECK_GETID, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_HOOKTYPE, BM_SETCHECK, Target[nActiveIndex].HookType, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_WHEELREDIRECT, BM_SETCHECK, Target[nActiveIndex].WheelRedirect, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_FREESCROLL, BM_SETCHECK, Target[nActiveIndex].FreeScroll, 0);
			SetDlgItemText(hDlg, IDC_EDIT_TARGEWINDOWTTITLE, Target[nActiveIndex].WindowTitle);
			SetDlgItemText(hDlg, IDC_EDIT_TARGETCLASS, Target[nActiveIndex].ClassName);
			SetDlgItemText(hDlg, IDC_EDIT_TARGETID, Target[nActiveIndex].ControlID);
			SetDlgItemText(hDlg, IDC_EDIT_TARGETCOMMENT, Target[nActiveIndex].Comment);
		}
		else
		{
			SendDlgItemMessage(hDlg, IDC_CHECK_GETFILE, BM_SETCHECK, 1, 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_SETCURSEL, 1, 0L);
			SendDlgItemMessage(hDlg, IDC_CHECK_HOOKTYPE, BM_SETCHECK, Default.HookType, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_WHEELREDIRECT, BM_SETCHECK, Default.WheelRedirect, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_FREESCROLL, BM_SETCHECK, Default.FreeScroll, 0);
			nActiveIndex = (int)Target.size();
		}
		for ( i = 0; i <= 3; i++ )
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_INSERTSTRING, (WPARAM)i, (LPARAM)TargetType[i]);
		if(nActiveIndex == (int)Target.size())
			_tsplitpath_s( TEXT(""), drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
		else
			_tsplitpath_s( Target[nActiveIndex].FileName, drive, _MAX_DRIVE, dir, _MAX_DIR, NULL, 0, NULL, 0 );
		if(drive[0] && dir[0])
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_SETCURSEL, 1, 0L);	//フルパス
		else if(!drive[0] && dir[0])
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_SETCURSEL, 2, 0L);	//ルートからのパス
		else if(drive[0] && !dir[0])
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_SETCURSEL, 3, 0L);	//ドライブとファイル
		else
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_SETCURSEL, 0, 0L);	//ファイル
		for ( i = 0; i < 12; i++ )
			SendDlgItemMessage(hDlg, IDC_COMBO_ETCTARGET, CB_INSERTSTRING, (WPARAM)i, (LPARAM)EtcTarget[i].Name);
		SendDlgItemMessage(hDlg, IDC_COMBO_ETCTARGET, CB_SETCURSEL, 0, 0L);
		WndProcFinderOld = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_STATIC_FINDER), GWLP_WNDPROC, (LONG_PTR)WndProcFinder);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_COMBO_ETCTARGET:
			if(HIWORD(wParam)==CBN_SELCHANGE)
			{
				switch(SendDlgItemMessage(hDlg, IDC_COMBO_ETCTARGET, CB_GETCURSEL, 0L, 0L))
				{
				case 0:
					break;
				case 1:
					SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, TEXT("Desktop"));
					SetDlgItemText(hDlg, IDC_EDIT_TARGETCLASS, TEXT(""));
					break;
				default:
					GetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, FilePath, _MAX_PATH);
					if(lstrcmpi(FilePath, TEXT("Desktop")) == 0)
						SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, TEXT(""));
					SetDlgItemText(hDlg, IDC_EDIT_TARGETCLASS, EtcTarget[SendDlgItemMessage(hDlg, IDC_COMBO_ETCTARGET, CB_GETCURSEL, 0L, 0L)].Value);
					break;
				}
				SendDlgItemMessage(hDlg, IDC_COMBO_ETCTARGET, CB_SETCURSEL, 0, 0L);
			}
			break;
		case IDC_BUTTON_SETFILENAME:
			SetWindowHook(hDlg,NULL,1);
			if(SendDlgItemMessage(hDlg, IDC_CHECK_GETFILE, BM_GETCHECK , 0L , 0L) == 1)
				SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, TEXT("ウィンドウをクリック"));
			if(SendDlgItemMessage(hDlg, IDC_CHECK_GETCLASS, BM_GETCHECK , 0L , 0L) == 1)
				SetDlgItemText(hDlg, IDC_EDIT_TARGETCLASS, TEXT("ウィンドウをクリック"));
			if(SendDlgItemMessage(hDlg, IDC_CHECK_GETID, BM_GETCHECK , 0L , 0L) == 1)
				SetDlgItemText(hDlg, IDC_EDIT_TARGETID, TEXT("クリック"));
			if(SendDlgItemMessage(hDlg, IDC_CHECK_GETWINDOWTITLE, BM_GETCHECK , 0L , 0L) == 1)
				SetDlgItemText(hDlg, IDC_EDIT_TARGEWINDOWTTITLE, TEXT("ウィンドウをクリック"));
			break;
		case IDC_BUTTON_TARGETSEARCH:
			GetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, FilePath, _MAX_PATH);
			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(OPENFILENAME));    
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner   = hDlg;
			ofn.lpstrFile   = FilePath; //選択されたファイル名を受け取る(フルパス)
			ofn.nMaxFile    = _MAX_PATH;
			ofn.lpstrFilter = TEXT("all(*.*)\0*.*\0\0"); //フィルタ
			ofn.lpstrTitle  = TEXT("対象ファイル選択"); //ダイアログボックスのタイトル
			ofn.Flags       = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			GetOpenFileName(&ofn);
			_tsplitpath_s( FilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT );
			switch(SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_GETCURSEL, 0L, 0L))
			{
			case 0:
				_tmakepath_s( FilePath, _MAX_PATH, NULL, NULL, fname, fext );
				break;
			case 1:
				_tmakepath_s( FilePath, _MAX_PATH, drive, dir, fname, fext );
				break;
			case 2:
				_tmakepath_s( FilePath, _MAX_PATH, NULL, dir, fname, fext );
				break;
			case 3:
				_tmakepath_s( FilePath, _MAX_PATH, drive, NULL, fname, fext );
				break;
			}
			SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, FilePath);
			break;
		case IDOK:
			GetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, t.FileName, _MAX_PATH);
			GetDlgItemText(hDlg, IDC_EDIT_TARGEWINDOWTTITLE, t.WindowTitle, 256);
			GetDlgItemText(hDlg, IDC_EDIT_TARGETCLASS, t.ClassName, MAX_CLASSNAME);
			GetDlgItemText(hDlg, IDC_EDIT_TARGETID, t.ControlID, 19);
			GetDlgItemText(hDlg, IDC_EDIT_TARGETCOMMENT, t.Comment, MAX_COMMENT);
			switch(SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_GETCURSEL, 0L, 0L))
			{
			case 0:
				t.DefaultThru = -1;
				break;
			case 1:
				t.DefaultThru = 0;
				break;
			default:
				t.DefaultThru = Target[(int)SendDlgItemMessage(hDlg, IDC_COMBO_DEFAULTTHRU, CB_GETCURSEL, 0L, 0L) - 2].Number;
				break;
			}
			t.HookType = (BOOL)SendDlgItemMessage(hDlg, IDC_CHECK_HOOKTYPE, BM_GETCHECK , 0L , 0L);
			t.WheelRedirect = (BOOL)SendDlgItemMessage(hDlg, IDC_CHECK_WHEELREDIRECT, BM_GETCHECK , 0L , 0L);
			t.FreeScroll = (BOOL)SendDlgItemMessage(hDlg, IDC_CHECK_FREESCROLL, BM_GETCHECK , 0L , 0L);
			if(nActiveIndex == (int)Target.size())
			{
				//追加
				t.Number = MinTargetNumber();
				Target.push_back(t);
			}
			else
			{
				//編集
				t.Number = Target[nActiveIndex].Number;
				Target[nActiveIndex] = t;
			}

			IniChange = TRUE;
			WindowUnHook();
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			WindowUnHook();
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	case MAUHOOK_MSG:
		GetFileName( FilePath, ClassName, ControlID, Title );
		_tsplitpath_s( FilePath, drive, _MAX_DRIVE, dir, _MAX_DIR, fname, _MAX_FNAME, fext, _MAX_EXT );
		switch(SendDlgItemMessage(hDlg, IDC_COMBO_TARGETTYPE, CB_GETCURSEL, 0L, 0L))
		{
		case 0:
			_tmakepath_s( FilePath, _MAX_PATH, NULL, NULL, fname, fext );
			break;
		case 1:
			_tmakepath_s( FilePath, _MAX_PATH, drive, dir, fname, fext );
			break;
		case 2:
			_tmakepath_s( FilePath, _MAX_PATH, NULL, dir, fname, fext );
			break;
		case 3:
			_tmakepath_s( FilePath, _MAX_PATH, drive, NULL, fname, fext );
			break;
		}
		if(SendDlgItemMessage(hDlg, IDC_CHECK_GETFILE, BM_GETCHECK , 0L , 0L) == 1)
			SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, FilePath);
		else
			SetDlgItemText(hDlg, IDC_EDIT_TARGETFILE, TEXT(""));
		if(SendDlgItemMessage(hDlg, IDC_CHECK_GETCLASS, BM_GETCHECK , 0L , 0L) == 1)
			SetDlgItemText(hDlg, IDC_EDIT_TARGETCLASS, ClassName);
		if(SendDlgItemMessage(hDlg, IDC_CHECK_GETID, BM_GETCHECK , 0L , 0L) == 1)
			SetDlgItemText(hDlg, IDC_EDIT_TARGETID, ControlID);
		if(SendDlgItemMessage(hDlg, IDC_CHECK_GETWINDOWTITLE, BM_GETCHECK , 0L , 0L) == 1)
			SetDlgItemText(hDlg, IDC_EDIT_TARGEWINDOWTTITLE, Title);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void CommandDelete(COMMAND *CommandEdit)
{
	int i;
	for ( i = CommandEdit->Locate; i < (CommandEdit->Repeat - 1); i++ )
	{
		CommandEdit->command[i].Case = CommandEdit->command[i + 1].Case;
		CommandEdit->command[i].CommandTarget = CommandEdit->command[i + 1].CommandTarget;
		CommandEdit->command[i].Wait = CommandEdit->command[i + 1].Wait;
		CommandEdit->command[i].Key = CommandEdit->command[i + 1].Key;
	}
	CommandEdit->command[CommandEdit->Repeat - 1].Case = 0;
	CommandEdit->command[CommandEdit->Repeat - 1].CommandTarget = 0;
	CommandEdit->command[CommandEdit->Repeat - 1].Wait = 0;
	CommandEdit->command[CommandEdit->Repeat - 1].Key = 0;
	CommandEdit->Repeat --;
}

void EnableCommandItem(HWND hDlg, int nActiveIndexC, int MaxCommand)
{
	if(MaxCommand >= MAX_ACTION_REPEAT)
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDADD), FALSE);
	else
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDADD), TRUE);
	if(nActiveIndexC == -1)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDEDIT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDDELETE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDUP), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDDOWN), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDEDIT), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDDELETE), TRUE);
		if(nActiveIndexC <= 0 || nActiveIndexC == MaxCommand)
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDUP), FALSE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDUP), TRUE);
		if(nActiveIndexC == -1 || nActiveIndexC >= MaxCommand - 1)
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDDOWN), FALSE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_COMMANDDOWN), TRUE);
	}
}
//アクション設定ダイアログ
INT_PTR CALLBACK ActionDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static int Locate;
	static BOOL bNew;
	static COMMAND CommandEdit;
	static HWND hLVC;
	static int GestureButton, EditGestureLevel, EditMove[MAX_GESTURE_LEVEL];
	NMHDR *lpnmhdr;
	int nActiveIndexC;
	TCHAR Code[30], MoveS[12];
	int i, Move;
	EXECUTECOMMAND a = {0};
	RECT rc;
	LV_COLUMN lvcol;

	switch (msg)
	{
	case WM_INITDIALOG:
		//static変数の初期化
		Locate = LOWORD(lParam);	//LV_ActionLocate
		bNew = (Locate < (int)Action.size()) ? FALSE : TRUE;
		CommandEdit.Locate = 0;
		CommandEdit.Repeat = 0;
		for ( i = 0; i < MAX_ACTION_REPEAT; i++ )
		{
			CommandEdit.command[i].Case = 0;
			CommandEdit.command[i].CommandTarget = 0;
			CommandEdit.command[i].Wait = 0;
			CommandEdit.command[i].Key = 0;
		}
		if(bNew)
		{
			GestureButton = 3;
			EditGestureLevel = 0;
			for(i=0; i<MAX_GESTURE_LEVEL; i++)
				EditMove[i] = 0;
		}
		else
		{
			CommandEdit.Repeat = Action[Locate].Repeat;
			for ( i = 0; i < Action[Locate].Repeat; i++ )
			{
				CommandEdit.command[i].Case = Action[Locate].command[i].Case;
				CommandEdit.command[i].CommandTarget = Action[Locate].command[i].CommandTarget;
				CommandEdit.command[i].Wait = Action[Locate].command[i].Wait;
				CommandEdit.command[i].Key = Action[Locate].command[i].Key;
			}
			GestureButton = Action[Locate].Button;
			for(i=0; i<MAX_GESTURE_LEVEL; i++)
			{
				if(Action[Locate].Move[i] != 0)
					EditGestureLevel = i + 1;
				EditMove[i] = Action[Locate].Move[i];
			}
		}
		hLVC = GetDlgItem(hDlg, IDC_LIST_COMMAND);
		SetWindowTheme(hLVC, L"Explorer", NULL);

		//コントロールの初期化
		SetDlgItemText(hDlg, IDC_BUTTON_UP, MoveUp);
		SetDlgItemText(hDlg, IDC_BUTTON_DOWN, MoveDown);
		SetDlgItemText(hDlg, IDC_BUTTON_LEFT, MoveLeft);
		SetDlgItemText(hDlg, IDC_BUTTON_RIGHT, MoveRight);
		SetDlgItemText(hDlg, IDC_BUTTON_UPLEFT, MoveUpLeft);
		SetDlgItemText(hDlg, IDC_BUTTON_UPRIGHT, MoveUpRight);
		SetDlgItemText(hDlg, IDC_BUTTON_DOWNLEFT, MoveDownLeft);
		SetDlgItemText(hDlg, IDC_BUTTON_DOWNRIGHT, MoveDownRight);
		lstrcpy(MoveS, TEXT("～ W"));
		lstrcat(MoveS, MoveUp);
		SetDlgItemText(hDlg, IDC_BUTTON_TOWHEELUP, MoveS);
		lstrcpy(MoveS, TEXT("～ W"));
		lstrcat(MoveS, MoveDown);
		SetDlgItemText(hDlg, IDC_BUTTON_TOWHEELDOWN, MoveS);
		lstrcpy(MoveS, TEXT("～ W"));
		lstrcat(MoveS, MoveLeft);
		SetDlgItemText(hDlg, IDC_BUTTON_TOWHEELLEFT, MoveS);
		lstrcpy(MoveS, TEXT("～ W"));
		lstrcat(MoveS, MoveRight);
		SetDlgItemText(hDlg, IDC_BUTTON_TOWHEELRIGHT, MoveS);
		lstrcpy(MoveS, MoveUp);
		SetDlgItemText(hDlg, IDC_RADIO_WHEELUP, MoveS);
		lstrcpy(MoveS, MoveDown);
		SetDlgItemText(hDlg, IDC_RADIO_WHEELDOWN, MoveS);
		lstrcpy(MoveS, MoveLeft);
		SetDlgItemText(hDlg, IDC_RADIO_WHEELLEFT, MoveS);
		lstrcpy(MoveS, MoveRight);
		SetDlgItemText(hDlg, IDC_RADIO_WHEELRIGHT, MoveS);
		for ( i = 0; i < (int)Target.size(); i++ )
		{
			TCHAR TempMainText[_MAX_PATH + 1], TempSubText[_MAX_PATH + 1];

			if(Target[i].Comment[0])
			{
				lstrcpyn(TempMainText, Target[i].Comment, 15);
			}
			else
			{
				lstrcpyn(TempMainText, Target[i].FileName, 15);
				if(TempMainText[0] == 0)
					lstrcpy(TempMainText, TEXT("---"));
				if(Target[i].ControlID[0])
				{
					if(Target[i].ClassName[0])
					{
						lstrcpyn(TempSubText, Target[i].ClassName, 13);
						wsprintf(TempSubText, TEXT("%s ID:%s"), TempSubText, Target[i].ControlID);
					}
					else
					{
						wsprintf(TempSubText, TEXT("ID:%s"), Target[i].ControlID);
					}
				}
				else
				{
					lstrcpyn(TempSubText, Target[i].ClassName, 21);
				}
				wsprintf(TempMainText, TEXT("%s %s"), TempMainText, TempSubText);
			}
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGET, CB_INSERTSTRING, (WPARAM)i, (LPARAM)TempMainText);
		}
		SendDlgItemMessage(hDlg, IDC_COMBO_TARGET, CB_INSERTSTRING, (WPARAM)(int)Target.size(), (LPARAM)TEXT("Default"));
		if(HIWORD(lParam) == 0)	//SelectedTargetNumber
		{
			SendDlgItemMessage(hDlg, IDC_COMBO_TARGET, CB_SETCURSEL, (WPARAM)(int)Target.size(), 0L);
		}
		else
		{
			for(i = 0; i < (int)Target.size(); i++)
			{
				if(HIWORD(lParam) == Target[i].Number)
					SendDlgItemMessage(hDlg, IDC_COMBO_TARGET, CB_SETCURSEL, (WPARAM)i, 0L);
			}
		}

		ListView_SetExtendedListViewStyle(hLVC, ListView_GetExtendedListViewStyle(hLVC)|LVS_EX_FULLROWSELECT);
		GetClientRect(hLVC, &rc);
		lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcol.fmt = LVCFMT_LEFT;
		lvcol.cx = (rc.right - GetSystemMetrics(SM_CXVSCROLL) - 2) / 2;
		lvcol.pszText = TEXT("Case");
		lvcol.iSubItem = 0;
		ListView_InsertColumn(hLVC, 0, &lvcol);
		lvcol.cx = (rc.right - GetSystemMetrics(SM_CXVSCROLL) - 2) / 2;
		lvcol.pszText = TEXT("Value");
		lvcol.iSubItem = 1;
		ListView_InsertColumn(hLVC, 1, &lvcol);

		if(!bNew)
		{
			if(Action[Locate].Modifier & MODIFIER_DISABLE)
			{
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CONTROL), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SHIFT), FALSE);
			}
			else
			{
				SendDlgItemMessage(hDlg, IDC_CHECK_MODIFIERKEY, BM_SETCHECK, 1, 0);
				if(Action[Locate].Modifier & MODIFIER_CONTROL)
					SendDlgItemMessage(hDlg, IDC_CHECK_CONTROL, BM_SETCHECK, 1, 0);
				if(Action[Locate].Modifier & MODIFIER_SHIFT)
					SendDlgItemMessage(hDlg, IDC_CHECK_SHIFT, BM_SETCHECK, 1, 0);
			}
			for(i=0; i<CommandEdit.Repeat; i++)
				InsertCommandItem(hLVC, CommandEdit.command[i].Case, CommandEdit.command[i].Key, i, 0);
			SetDlgItemText(hDlg, IDC_EDIT_COMMENT, Action[Locate].Comment);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CONTROL), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SHIFT), FALSE);
		}

		switch(GestureButton)
		{
		case BUTTON_L:
			SendDlgItemMessage(hDlg, IDC_RADIO_L, BM_SETCHECK, 1, 0);
			break;
		case BUTTON_M:
			SendDlgItemMessage(hDlg, IDC_RADIO_M, BM_SETCHECK, 1, 0);
			break;
		case BUTTON_R:
			SendDlgItemMessage(hDlg, IDC_RADIO_R, BM_SETCHECK, 1, 0);
			break;
		case BUTTON_X1:
			SendDlgItemMessage(hDlg, IDC_RADIO_X1, BM_SETCHECK, 1, 0);
			break;
		case BUTTON_X2:
			SendDlgItemMessage(hDlg, IDC_RADIO_X2, BM_SETCHECK, 1, 0);
			break;
		case WHEEL_UP:
			SendDlgItemMessage(hDlg, IDC_RADIO_WHEELUP, BM_SETCHECK, 1, 0);
			break;
		case WHEEL_DOWN:
			SendDlgItemMessage(hDlg, IDC_RADIO_WHEELDOWN, BM_SETCHECK, 1, 0);
			break;
		case WHEEL_LEFT:
			SendDlgItemMessage(hDlg, IDC_RADIO_WHEELLEFT, BM_SETCHECK, 1, 0);
			break;
		case WHEEL_RIGHT:
			SendDlgItemMessage(hDlg, IDC_RADIO_WHEELRIGHT, BM_SETCHECK, 1, 0);
			break;
		case CORNER_TOP_A:
			SendDlgItemMessage(hDlg, IDC_RADIO_TOP_A, BM_SETCHECK, 1, 0);
			break;
		case CORNER_BOTTOM_A:
			SendDlgItemMessage(hDlg, IDC_RADIO_BOTTOM_A, BM_SETCHECK, 1, 0);
			break;
		case CORNER_LEFT_A:
			SendDlgItemMessage(hDlg, IDC_RADIO_LEFT_A, BM_SETCHECK, 1, 0);
			break;
		case CORNER_RIGHT_A:
			SendDlgItemMessage(hDlg, IDC_RADIO_RIGHT_A, BM_SETCHECK, 1, 0);
			break;
		case CORNER_TOPLEFT:
			SendDlgItemMessage(hDlg, IDC_RADIO_TOPLEFT, BM_SETCHECK, 1, 0);
			break;
		case CORNER_TOPRIGHT:
			SendDlgItemMessage(hDlg, IDC_RADIO_TOPRIGHT, BM_SETCHECK, 1, 0);
			break;
		case CORNER_BOTTOMLEFT:
			SendDlgItemMessage(hDlg, IDC_RADIO_BOTTOMLEFT, BM_SETCHECK, 1, 0);
			break;
		case CORNER_BOTTOMRIGHT:
			SendDlgItemMessage(hDlg, IDC_RADIO_BOTTOMRIGHT, BM_SETCHECK, 1, 0);
			break;
		case CORNER_TOP_B:
			SendDlgItemMessage(hDlg, IDC_RADIO_TOP_B, BM_SETCHECK, 1, 0);
			break;
		case CORNER_BOTTOM_B:
			SendDlgItemMessage(hDlg, IDC_RADIO_BOTTOM_B, BM_SETCHECK, 1, 0);
			break;
		case CORNER_LEFT_B:
			SendDlgItemMessage(hDlg, IDC_RADIO_LEFT_B, BM_SETCHECK, 1, 0);
			break;
		case CORNER_RIGHT_B:
			SendDlgItemMessage(hDlg, IDC_RADIO_RIGHT_B, BM_SETCHECK, 1, 0);
			break;
		case CORNER_TOP_C:
			SendDlgItemMessage(hDlg, IDC_RADIO_TOP_C, BM_SETCHECK, 1, 0);
			break;
		case CORNER_BOTTOM_C:
			SendDlgItemMessage(hDlg, IDC_RADIO_BOTTOM_C, BM_SETCHECK, 1, 0);
			break;
		case CORNER_LEFT_C:
			SendDlgItemMessage(hDlg, IDC_RADIO_LEFT_C, BM_SETCHECK, 1, 0);
			break;
		case CORNER_RIGHT_C:
			SendDlgItemMessage(hDlg, IDC_RADIO_RIGHT_C, BM_SETCHECK, 1, 0);
			break;
		}

		GetGestureString(NULL, &GestureButton, EditMove, Code, 30);

		SetDlgItemText(hDlg, IDC_EDIT_CODE, Code);
		nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
		EnableCommandItem(hDlg, nActiveIndexC, CommandEdit.Repeat);
		ListView_SetImageList(hLVC, ImageList_Create(1, GetSystemMetrics(SM_CYSMICON), ILC_COLOR, 0, 0), LVSIL_SMALL);
		break;
	case WM_DESTROY:
		ImageList_Destroy(ListView_GetImageList(hLVC, LVSIL_SMALL));
		break;
	case WM_NOTIFY:
		lpnmhdr = (NMHDR *)lParam;
		switch (lpnmhdr->code)
		{
		case NM_DBLCLK:
			if (lpnmhdr->hwndFrom == hLVC)
			{
				if(ListView_GetNextItem(hLVC, -1, LVNI_SELECTED) == -1)
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_COMMANDADD, 0);
				else
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_COMMANDEDIT, 0);
			}
			break;
		case NM_RCLICK:
			if(lpnmhdr->hwndFrom == hLVC)
			{
				int iItem;
				HMENU hMenu;
				POINT pt;
				iItem = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
				hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_COMMANDADD, TEXT("コマンドを追加(&A)..."));
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_COMMANDEDIT, TEXT("コマンドを編集(&E)..."));
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_COMMANDDELETE, TEXT("コマンドを削除(D&)..."));
				if(CommandEdit.Repeat >= MAX_ACTION_REPEAT)
					EnableMenuItem(hMenu, IDC_BUTTON_COMMANDADD, MF_GRAYED|MF_BYCOMMAND);
				if(iItem==-1)	//選択なし
				{
					EnableMenuItem(hMenu, IDC_BUTTON_COMMANDEDIT, MF_GRAYED|MF_BYCOMMAND);
					EnableMenuItem(hMenu, IDC_BUTTON_COMMANDDELETE, MF_GRAYED|MF_BYCOMMAND);
				}
				GetCursorPos(&pt);
				TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, hDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;
		case LVN_ITEMCHANGED:
			if(lpnmhdr->hwndFrom == hLVC)
			{
				nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
				EnableCommandItem(hDlg, nActiveIndexC, CommandEdit.Repeat);
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_RADIO_WHEELUP:
		case IDC_RADIO_WHEELDOWN:
		case IDC_RADIO_WHEELLEFT:
		case IDC_RADIO_WHEELRIGHT:
		case IDC_RADIO_TOP_A:
		case IDC_RADIO_TOP_B:
		case IDC_RADIO_TOP_C:
		case IDC_RADIO_BOTTOM_A:
		case IDC_RADIO_BOTTOM_B:
		case IDC_RADIO_BOTTOM_C:
		case IDC_RADIO_LEFT_A:
		case IDC_RADIO_LEFT_B:
		case IDC_RADIO_LEFT_C:
		case IDC_RADIO_RIGHT_A:
		case IDC_RADIO_RIGHT_B:
		case IDC_RADIO_RIGHT_C:
		case IDC_RADIO_TOPLEFT:
		case IDC_RADIO_TOPRIGHT:
		case IDC_RADIO_BOTTOMLEFT:
		case IDC_RADIO_BOTTOMRIGHT:
			if(GestureButton == BUTTON_L || GestureButton == BUTTON_M || GestureButton == BUTTON_R ||
				GestureButton == BUTTON_X1 || GestureButton == BUTTON_X2)
			{
				for(i=0; i<MAX_GESTURE_LEVEL; i++)
					EditMove[i] = 0;
				EditGestureLevel = 0;
			}
		case IDC_RADIO_L:
		case IDC_RADIO_M:
		case IDC_RADIO_R:
		case IDC_RADIO_X1:
		case IDC_RADIO_X2:
			switch (LOWORD(wParam))
			{
			case IDC_RADIO_L:
				GestureButton = BUTTON_L;
				break;
			case IDC_RADIO_M:
				GestureButton = BUTTON_M;
				break;
			case IDC_RADIO_R:
				GestureButton = BUTTON_R;
				break;
			case IDC_RADIO_X1:
				GestureButton = BUTTON_X1;
				break;
			case IDC_RADIO_X2:
				GestureButton = BUTTON_X2;
				break;
			case IDC_RADIO_WHEELUP:
				GestureButton = WHEEL_UP;
				break;
			case IDC_RADIO_WHEELDOWN:
				GestureButton = WHEEL_DOWN;
				break;
			case IDC_RADIO_WHEELLEFT:
				GestureButton = WHEEL_LEFT;
				break;
			case IDC_RADIO_WHEELRIGHT:
				GestureButton = WHEEL_RIGHT;
				break;
			case IDC_RADIO_TOP_A:
				GestureButton = CORNER_TOP_A;
				break;
			case IDC_RADIO_TOP_B:
				GestureButton = CORNER_TOP_B;
				break;
			case IDC_RADIO_TOP_C:
				GestureButton = CORNER_TOP_C;
				break;
			case IDC_RADIO_BOTTOM_A:
				GestureButton = CORNER_BOTTOM_A;
				break;
			case IDC_RADIO_BOTTOM_B:
				GestureButton = CORNER_BOTTOM_B;
				break;
			case IDC_RADIO_BOTTOM_C:
				GestureButton = CORNER_BOTTOM_C;
				break;
			case IDC_RADIO_LEFT_A:
				GestureButton = CORNER_LEFT_A;
				break;
			case IDC_RADIO_LEFT_B:
				GestureButton = CORNER_LEFT_B;
				break;
			case IDC_RADIO_LEFT_C:
				GestureButton = CORNER_LEFT_C;
				break;
			case IDC_RADIO_RIGHT_A:
				GestureButton = CORNER_RIGHT_A;
				break;
			case IDC_RADIO_RIGHT_B:
				GestureButton = CORNER_RIGHT_B;
				break;
			case IDC_RADIO_RIGHT_C:
				GestureButton = CORNER_RIGHT_C;
				break;
			case IDC_RADIO_TOPLEFT:
				GestureButton = CORNER_TOPLEFT;
				break;
			case IDC_RADIO_TOPRIGHT:
				GestureButton = CORNER_TOPRIGHT;
				break;
			case IDC_RADIO_BOTTOMLEFT:
				GestureButton = CORNER_BOTTOMLEFT;
				break;
			case IDC_RADIO_BOTTOMRIGHT:
				GestureButton = CORNER_BOTTOMRIGHT;
				break;
			}
			GetGestureString(NULL, &GestureButton, EditMove, Code, 30);
			SetDlgItemText(hDlg, IDC_EDIT_CODE, Code);
			break;
		case IDC_BUTTON_CLEAR:
			for(i=0; i<MAX_GESTURE_LEVEL; i++)
				EditMove[i] = 0;
			EditGestureLevel = 0;
			GetGestureString(NULL, &GestureButton, NULL, MoveS, 12);
			SetDlgItemText(hDlg, IDC_EDIT_CODE, MoveS);
			break;
		case IDC_BUTTON_UP:
		case IDC_BUTTON_DOWN:
		case IDC_BUTTON_LEFT:
		case IDC_BUTTON_RIGHT:
		case IDC_BUTTON_UPLEFT:
		case IDC_BUTTON_UPRIGHT:
		case IDC_BUTTON_DOWNLEFT:
		case IDC_BUTTON_DOWNRIGHT:
			if(GestureButton != BUTTON_L && GestureButton != BUTTON_M && GestureButton != BUTTON_R &&
				GestureButton != BUTTON_X1 && GestureButton != BUTTON_X2)
				break;
			if(EditMove[0] != MOVE_UP && EditMove[0] != MOVE_DOWN && EditMove[0] != MOVE_LEFT && EditMove[0] != MOVE_RIGHT &&
				EditMove[0] != MOVE_UPLEFT && EditMove[0] != MOVE_UPRIGHT && EditMove[0] != MOVE_DOWNLEFT && EditMove[0] != MOVE_DOWNRIGHT)
				EditGestureLevel = 0;
			switch (LOWORD(wParam))
			{
			case IDC_BUTTON_UP:
				Move = MOVE_UP;
				break;
			case IDC_BUTTON_DOWN:
				Move = MOVE_DOWN;
				break;
			case IDC_BUTTON_LEFT:
				Move = MOVE_LEFT;
				break;
			case IDC_BUTTON_RIGHT:
				Move = MOVE_RIGHT;
				break;
			case IDC_BUTTON_UPLEFT:
				Move = MOVE_UPLEFT;
				break;
			case IDC_BUTTON_UPRIGHT:
				Move = MOVE_UPRIGHT;
				break;
			case IDC_BUTTON_DOWNLEFT:
				Move = MOVE_DOWNLEFT;
				break;
			case IDC_BUTTON_DOWNRIGHT:
				Move = MOVE_DOWNRIGHT;
				break;
			}
			if(EditGestureLevel < MAX_GESTURE_LEVEL &&
				(EditMove[EditGestureLevel - 1] != Move ||
				EditGestureLevel == 0))
			{
				EditMove[EditGestureLevel] = Move;
				EditGestureLevel ++;
				GetGestureString(NULL, &GestureButton, EditMove, Code, 30);
				SetDlgItemText(hDlg, IDC_EDIT_CODE, Code);
			}
			break;
		case IDC_BUTTON_TOL:
		case IDC_BUTTON_TOM:
		case IDC_BUTTON_TOR:
		case IDC_BUTTON_TOX1:
		case IDC_BUTTON_TOX2:
		case IDC_BUTTON_TOWHEELUP:
		case IDC_BUTTON_TOWHEELDOWN:
		case IDC_BUTTON_TOWHEELLEFT:
		case IDC_BUTTON_TOWHEELRIGHT:
			if(GestureButton != BUTTON_L && GestureButton != BUTTON_M && GestureButton != BUTTON_R &&
				GestureButton != BUTTON_X1 && GestureButton != BUTTON_X2)
				break;
			for(i=1; i<MAX_GESTURE_LEVEL; i++)
				EditMove[i] = 0;
			EditGestureLevel = 0;
			switch (LOWORD(wParam))
			{
			case IDC_BUTTON_TOL:
				EditMove[0] = BUTTON_L;
				break;
			case IDC_BUTTON_TOM:
				EditMove[0] = BUTTON_M;
				break;
			case IDC_BUTTON_TOR:
				EditMove[0] = BUTTON_R;
				break;
			case IDC_BUTTON_TOX1:
				EditMove[0] = BUTTON_X1;
				break;
			case IDC_BUTTON_TOX2:
				EditMove[0] = BUTTON_X2;
				break;
			case IDC_BUTTON_TOWHEELUP:
				EditMove[0] = WHEEL_UP;
				break;
			case IDC_BUTTON_TOWHEELDOWN:
				EditMove[0] = WHEEL_DOWN;
				break;
			case IDC_BUTTON_TOWHEELLEFT:
				EditMove[0] = WHEEL_LEFT;
				break;
			case IDC_BUTTON_TOWHEELRIGHT:
				EditMove[0] = WHEEL_RIGHT;
				break;
			}
			GetGestureString(NULL, &GestureButton, EditMove, Code, 30);
			SetDlgItemText(hDlg, IDC_EDIT_CODE, Code);
			break;
		case IDC_CHECK_MODIFIERKEY:
			if(SendDlgItemMessage(hDlg, IDC_CHECK_MODIFIERKEY, BM_GETCHECK , 0L , 0L))
			{
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CONTROL), TRUE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SHIFT), TRUE);
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_CONTROL), FALSE);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_SHIFT), FALSE);
			}
			break;
		case IDC_BUTTON_COMMANDADD:	//コマンド追加
		case IDC_BUTTON_COMMANDEDIT:	//コマンド編集
			if(LOWORD(wParam)==IDC_BUTTON_COMMANDADD)	//コマンド追加
			{
				if(CommandEdit.Repeat >= MAX_ACTION_REPEAT)
					break;	//追加できない
				CommandEdit.Locate = CommandEdit.Repeat;
			}
			if(LOWORD(wParam)==IDC_BUTTON_COMMANDEDIT)	//コマンド編集
			{
				CommandEdit.Locate = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
				if(CommandEdit.Locate == -1)
					break;	//編集できない
			}

			if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_COMMAND), hDlg, CommandDialogProc, (LPARAM)&CommandEdit))
			{
				if(CommandEdit.command[CommandEdit.Locate].Case != 0)
				{
					InsertCommandItem(hLVC, CommandEdit.command[CommandEdit.Locate].Case, CommandEdit.command[CommandEdit.Locate].Key, CommandEdit.Locate, 0);
					if(LOWORD(wParam)==IDC_BUTTON_COMMANDADD)
						ListView_SetItemState(hLVC, ListView_GetItemCount(hLVC)-1, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);	//追加したコマンドを選択
					nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
					EnableCommandItem(hDlg, nActiveIndexC, CommandEdit.Repeat);
				}
			}
			break;
		case IDC_BUTTON_COMMANDDELETE:
			if(MessageBox(hDlg, TEXT("このコマンドを削除してもよろしいですか？"), TEXT("コマンドの削除"), MB_YESNO) != IDYES)
				break;
			CommandEdit.Locate = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
			if(CommandEdit.Locate == -1)
				break;
			CommandDelete(&CommandEdit);
			ListView_DeleteItem(hLVC, CommandEdit.Locate);
			if(ListView_GetItemCount(hLVC)>0)
			{
				int iItem = CommandEdit.Locate;
				if(iItem==ListView_GetItemCount(hLVC))
					iItem--;
				ListView_SetItemState(hLVC, iItem, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
			}
			nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
			EnableCommandItem(hDlg, nActiveIndexC, CommandEdit.Repeat);
			break;
		case IDC_BUTTON_COMMANDUP:
			nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
			if(nActiveIndexC <= 0 || nActiveIndexC == CommandEdit.Repeat)
				break;
			std::swap(CommandEdit.command[nActiveIndexC - 1].Case, CommandEdit.command[nActiveIndexC].Case);
			std::swap(CommandEdit.command[nActiveIndexC - 1].CommandTarget, CommandEdit.command[nActiveIndexC].CommandTarget);
			std::swap(CommandEdit.command[nActiveIndexC - 1].Wait, CommandEdit.command[nActiveIndexC].Wait);
			std::swap(CommandEdit.command[nActiveIndexC - 1].Key, CommandEdit.command[nActiveIndexC].Key);
			ListView_DeleteItem(hLVC, nActiveIndexC - 1);
			InsertCommandItem(hLVC, CommandEdit.command[nActiveIndexC].Case, CommandEdit.command[nActiveIndexC].Key, nActiveIndexC, 1);
			IniChange = TRUE;
			nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
			EnableCommandItem(hDlg, nActiveIndexC, CommandEdit.Repeat);
			break;
		case IDC_BUTTON_COMMANDDOWN:
			nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
			if(nActiveIndexC == -1 || nActiveIndexC >= CommandEdit.Repeat - 1)
				break;
			std::swap(CommandEdit.command[nActiveIndexC].Case, CommandEdit.command[nActiveIndexC + 1].Case);
			std::swap(CommandEdit.command[nActiveIndexC].CommandTarget, CommandEdit.command[nActiveIndexC + 1].CommandTarget);
			std::swap(CommandEdit.command[nActiveIndexC].Wait, CommandEdit.command[nActiveIndexC + 1].Wait);
			std::swap(CommandEdit.command[nActiveIndexC].Key, CommandEdit.command[nActiveIndexC + 1].Key);
			ListView_DeleteItem(hLVC, nActiveIndexC + 1);
			InsertCommandItem(hLVC, CommandEdit.command[nActiveIndexC].Case, CommandEdit.command[nActiveIndexC].Key, nActiveIndexC, 1);
			IniChange = TRUE;
			nActiveIndexC = ListView_GetNextItem(hLVC, -1, LVNI_SELECTED);
			EnableCommandItem(hDlg, nActiveIndexC, CommandEdit.Repeat);
			break;
		case IDOK:
			if(SendDlgItemMessage(hDlg, IDC_COMBO_TARGET, CB_GETCURSEL, 0L, 0L) < (int)Target.size())
				a.TargetNumber = Target[(int)SendDlgItemMessage(hDlg, IDC_COMBO_TARGET, CB_GETCURSEL, 0L, 0L)].Number;
			else
				a.TargetNumber = 0;
			a.Button = GestureButton;
			for(i=0; i<MAX_GESTURE_LEVEL; i++)
				a.Move[i] = EditMove[i];
			if(SendDlgItemMessage(hDlg, IDC_CHECK_MODIFIERKEY, BM_GETCHECK, 0L, 0L))
			{
				a.Modifier = 0;
				if(SendDlgItemMessage(hDlg, IDC_CHECK_CONTROL, BM_GETCHECK, 0L, 0L))
					a.Modifier |= MODIFIER_CONTROL;
				if(SendDlgItemMessage(hDlg, IDC_CHECK_SHIFT, BM_GETCHECK, 0L, 0L))
					a.Modifier |= MODIFIER_SHIFT;
			}
			else
			{
				a.Modifier = MODIFIER_DISABLE;
			}
			a.Repeat = CommandEdit.Repeat;
			for(i=0; i<CommandEdit.Repeat; i++)
			{
				a.command[i].Case = CommandEdit.command[i].Case;
				a.command[i].CommandTarget = CommandEdit.command[i].CommandTarget;
				a.command[i].Wait = CommandEdit.command[i].Wait;
				a.command[i].Key = CommandEdit.command[i].Key;
			}
			a.BreakPoint = 0;
			GetDlgItemText(hDlg, IDC_EDIT_COMMENT, a.Comment, MAX_COMMENT);
			if(bNew)
				Action.push_back(a);
			else
				Action[Locate] = a;

			IniChange = TRUE;
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void ActionDelete(int Locate)
{
	std::vector<EXECUTECOMMAND>::iterator it = Action.begin();
	it+=Locate;
	Action.erase(it);
}

void TargetDelete(int Locate)
{
	int i, j;

	for(i=0;i<(int)Target.size();i++)
	{
		if(Target[i].DefaultThru == Target[Locate].Number)
			Target[i].DefaultThru = -1;
	}

	for(i=(int)Action.size()-1;i>=0;i--)
	{
		if(Target[Locate].Number == Action[i].TargetNumber)
		{
			ActionDelete(i);
		}
		else
		{
			for(j=0;j<Action[i].Repeat;j++)
			{
				if(Action[i].command[j].CommandTarget == Target[Locate].Number)
					Action[i].command[j].CommandTarget = 0;
			}
		}
	}

	std::vector<TARGETWINDOW>::iterator it = Target.begin();
	it+=Locate;
	Target.erase(it);
}

void EnableActionItem(HWND hDlg, int nActiveIndexA)
{
	if((int)Action.size() >= MAX_ACTION_DATA)
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ACTIONADD), FALSE);
	else
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ACTIONADD), TRUE);
	if(nActiveIndexA == -1)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ACTIONEDIT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ACTIONDELETE), FALSE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ACTIONEDIT), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ACTIONDELETE), TRUE);
	}
}

void EnableTargetItem(HWND hDlg, int nActiveIndexT)
{
	if((int)Target.size() >= MAX_TARGET_DATA)
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETADD), FALSE);
	else
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETADD), TRUE);
	if(nActiveIndexT == -1)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETEDIT), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETDELETE), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETUP), FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETDOWN), FALSE);
	}
	else
	{
		if(nActiveIndexT < (int)Target.size())
		{
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETEDIT), TRUE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETDELETE), TRUE);
		}
		else
		{
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETEDIT), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETDELETE), FALSE);
		}
		if(nActiveIndexT <= 0 || nActiveIndexT == (int)Target.size())
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETUP), FALSE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETUP), TRUE);
		if(nActiveIndexT == -1 || nActiveIndexT >= (int)Target.size() - 1)
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETDOWN), FALSE);
		else
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_TARGETDOWN), TRUE);
	}
}
//マウ筋ナビ設定ダイアログ
INT_PTR CALLBACK NaviEditDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hWnd;
	CHOOSEFONT cf;
	CHOOSECOLOR cc;
	static int NaviWidthMax = GetSystemMetrics(SM_CXSCREEN);
	#define NaviWidthMin 0
	#define NaviDeleteTimeMax 3000
	#define NaviDeleteTimeMin 0
	#define TipLocateXMax 100
	#define TipLocateXMin -100
	#define TipLocateYMax 100
	#define TipLocateYMin -100
	static COLORREF CustColors[16];
	static HFONT hFont12px = NULL;	//プレビュー用12ピクセルのフォント
	static HBRUSH hBrushMain, hBrushSub, hBrushBack, hBrushFrame;
	HBRUSH hBrushOld;

	switch (msg)
	{
	case WM_INITDIALOG:
		hWnd = (HWND)lParam;
		SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIDELETETIME, TBM_SETRANGE, TRUE, MAKELPARAM(NaviDeleteTimeMin, NaviDeleteTimeMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIDELETETIME, TBM_SETPAGESIZE, 0, 50);
		SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIDELETETIME, TBM_SETPOS, TRUE, NaviDeleteTime);
		if(NaviDeleteTime == NaviDeleteTimeMin)
			SetDlgItemText(hDlg, IDC_EDIT_NAVIDELETETIME, TEXT("∞"));
		else
			SetDlgItemInt(hDlg, IDC_EDIT_NAVIDELETETIME, NaviDeleteTime, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIWIDTH, TBM_SETRANGE, TRUE, MAKELPARAM(NaviWidthMin, NaviWidthMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIWIDTH, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIWIDTH, TBM_SETPOS, TRUE, NaviWidth);
		if(NaviWidth == NaviWidthMin)
			SetDlgItemText(hDlg, IDC_EDIT_NAVIWIDTH, TEXT("∞"));
		else
			SetDlgItemInt(hDlg, IDC_EDIT_NAVIWIDTH, NaviWidth, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEX, TBM_SETRANGE, TRUE, MAKELPARAM(TipLocateXMin, TipLocateXMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEX, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEX, TBM_SETPOS, TRUE, TipLocate.x);
		SetDlgItemInt(hDlg, IDC_EDIT_TIPLOCATEX, TipLocate.x, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEY, TBM_SETRANGE, TRUE, MAKELPARAM(TipLocateYMin, TipLocateYMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEY, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEY, TBM_SETPOS, TRUE, TipLocate.y);
		SetDlgItemInt(hDlg, IDC_EDIT_TIPLOCATEY, TipLocate.y, TRUE);
		if(NaviType==NaviTypeNone)
			CheckRadioButton(hDlg, IDC_RADIO_NAVITYPE_NONE, IDC_RADIO_NAVITYPE_FLOAT, IDC_RADIO_NAVITYPE_NONE);
		else if(NaviType==NaviTypeFixed)
			CheckRadioButton(hDlg, IDC_RADIO_NAVITYPE_NONE, IDC_RADIO_NAVITYPE_FLOAT, IDC_RADIO_NAVITYPE_FIXED);
		else if(NaviType==NaviTypeFloat)
			CheckRadioButton(hDlg, IDC_RADIO_NAVITYPE_NONE, IDC_RADIO_NAVITYPE_FLOAT, IDC_RADIO_NAVITYPE_FLOAT);
		SetDlgItemText(hDlg, IDC_EDIT_NAVITITLE, NaviTitle);
		hFont12px = CreateFont(12, 0, 0, 0, lf.lfWeight, lf.lfItalic, 0, 0, lf.lfCharSet, 0, 0, 0, 0, lf.lfFaceName);
		SendMessage(GetDlgItem(hDlg, IDC_STATIC_FONT), WM_SETFONT, (WPARAM)hFont12px, MAKELPARAM(TRUE, 0));
		hBrushMain = CreateSolidBrush(MainColor);
		hBrushSub = CreateSolidBrush(SubColor);
		hBrushBack = CreateSolidBrush(BackColor);
		hBrushFrame = CreateSolidBrush(FrameColor);
		break;
	case WM_DESTROY:
		if(hFont12px)
			DeleteObject(hFont12px);
		DeleteObject(hBrushMain);
		DeleteObject(hBrushSub);
		DeleteObject(hBrushBack);
		DeleteObject(hBrushFrame);
		break;
	case WM_HSCROLL:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_SLIDER_NAVIDELETETIME:
			NaviDeleteTime = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIDELETETIME, TBM_GETPOS, 0, 0);
			if(NaviDeleteTime == NaviDeleteTimeMin)
				SetDlgItemText(hDlg, IDC_EDIT_NAVIDELETETIME, TEXT("∞"));
			else
				SetDlgItemInt(hDlg, IDC_EDIT_NAVIDELETETIME, NaviDeleteTime, TRUE);
			break;
		case IDC_SLIDER_NAVIWIDTH:
			NaviWidth = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_NAVIWIDTH, TBM_GETPOS, 0, 0);
			if(NaviWidth == NaviWidthMin)
				SetDlgItemText(hDlg, IDC_EDIT_NAVIWIDTH, TEXT("∞"));
			else
				SetDlgItemInt(hDlg, IDC_EDIT_NAVIWIDTH, NaviWidth, TRUE);
			NaviEditRefresh(hWnd);
			break;
		case IDC_SLIDER_TIPLOCATEX:
			TipLocate.x = (LONG)SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEX, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_TIPLOCATEX, TipLocate.x, TRUE);
			break;
		case IDC_SLIDER_TIPLOCATEY:
			TipLocate.y = (LONG)SendDlgItemMessage(hDlg, IDC_SLIDER_TIPLOCATEY, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_TIPLOCATEY, TipLocate.y, TRUE);
			break;
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			break;
		case PSN_RESET:
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_COMBO_NAVITYPE:
			break;
		case IDC_EDIT_NAVITITLE:
			if (HIWORD(wParam) == EN_UPDATE)
			{
				GetDlgItemText(hDlg, IDC_EDIT_NAVITITLE, NaviTitle, _MAX_PATH);
				NaviEditRefresh(hWnd);
			}
			break;
		case IDC_BUTTON_CHOOSEFONT:
			memset(&cf, 0, sizeof(CHOOSEFONT));    
			cf.lStructSize = sizeof(CHOOSEFONT);
			cf.hwndOwner   = hDlg;
			cf.lpLogFont   = &lf;
			cf.Flags       = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST | CF_NOVERTFONTS;
			if(ChooseFont(&cf))
			{
				if(hFont)
					DeleteObject(hFont);
				hFont = CreateFontIndirect(&lf);
				if(hFont12px)
					DeleteObject(hFont12px);
				hFont12px = CreateFont(12, 0, 0, 0, lf.lfWeight, lf.lfItalic, 0, 0, lf.lfCharSet, 0, 0, 0, 0, lf.lfFaceName);
				SendMessage(GetDlgItem(hDlg, IDC_STATIC_FONT), WM_SETFONT, (WPARAM)hFont12px, MAKELPARAM(TRUE, 0));
				NaviEditRefresh(hWnd);
			}
			break;
		case IDC_BUTTON_CHOOSEMAINCOLOR:
			memset(&cc, 0, sizeof(CHOOSECOLOR));    
			cc.lStructSize  = sizeof (CHOOSECOLOR);
			cc.hwndOwner    = hDlg;
			cc.rgbResult    = MainColor;
			cc.lpCustColors	= CustColors;
			cc.Flags        = CC_RGBINIT;
			if(ChooseColor(&cc))
			{
				MainColor = cc.rgbResult;
				DeleteObject(hBrushMain);
				hBrushMain = CreateSolidBrush(MainColor);
				InvalidateRect(hDlg, NULL, TRUE);
				NaviEditRefresh(hWnd);
			}
			break;
		case IDC_BUTTON_CHOOSESUBCOLOR:
			memset(&cc, 0, sizeof(CHOOSECOLOR));    
			cc.lStructSize  = sizeof (CHOOSECOLOR);
			cc.hwndOwner    = hDlg;
			cc.rgbResult    = SubColor;
			cc.lpCustColors	= CustColors;
			cc.Flags        = CC_RGBINIT;
			if(ChooseColor(&cc))
			{
				SubColor = cc.rgbResult;
				DeleteObject(hBrushSub);
				hBrushSub = CreateSolidBrush(SubColor);
				InvalidateRect(hDlg, NULL, TRUE);
				NaviEditRefresh(hWnd);
			}
			break;
		case IDC_BUTTON_CHOOSEBACKCOLOR:
			memset(&cc, 0, sizeof(CHOOSECOLOR));    
			cc.lStructSize  = sizeof (CHOOSECOLOR);
			cc.hwndOwner    = hDlg;
			cc.rgbResult    = BackColor;
			cc.lpCustColors	= CustColors;
			cc.Flags        = CC_RGBINIT;
			if(ChooseColor(&cc))
			{
				BackColor = cc.rgbResult;
				DeleteObject(hBrushBack);
				hBrushBack = CreateSolidBrush(BackColor);
				hBrushOld = (HBRUSH)SetClassLongPtr(hWnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(BackColor));
				DeleteObject(hBrushOld);
				InvalidateRect(hDlg, NULL, TRUE);
				NaviEditRefresh(hWnd);
			}
			break;
		case IDC_RADIO_NAVITYPE_NONE:
		case IDC_RADIO_NAVITYPE_FIXED:
		case IDC_RADIO_NAVITYPE_FLOAT:
			if(LOWORD(wParam)==IDC_RADIO_NAVITYPE_NONE)
				NaviType = NaviTypeNone;
			if(LOWORD(wParam)==IDC_RADIO_NAVITYPE_FIXED)
				NaviType = NaviTypeFixed;
			if(LOWORD(wParam)==IDC_RADIO_NAVITYPE_FLOAT)
				NaviType = NaviTypeFloat;
			NaviEditRefresh(hWnd);
			if(NaviType < NaviTypeFloat)
				ShowMauSujiNavi(hWnd, NaviType);
			break;
		case IDC_BUTTON_CHOOSEFRAMECOLOR:
			memset(&cc, 0, sizeof(CHOOSECOLOR));    
			cc.lStructSize  = sizeof (CHOOSECOLOR);
			cc.hwndOwner    = hDlg;
			cc.rgbResult    = FrameColor;
			cc.lpCustColors	= CustColors;
			cc.Flags        = CC_RGBINIT;
			if(ChooseColor(&cc))
			{
				FrameColor = cc.rgbResult;
				DeleteObject(hBrushFrame);
				hBrushFrame = CreateSolidBrush(FrameColor);
				InvalidateRect(hDlg, NULL, TRUE);
				NaviEditRefresh(hWnd);
			}
			break;
		}
		break;
	case WM_CTLCOLORSTATIC:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_STATIC_MAINCOLOR:
			return (INT_PTR)hBrushMain;
		case IDC_STATIC_SUBCOLOR:
			return (INT_PTR)hBrushSub;
		case IDC_STATIC_BACKCOLOR:
			return (INT_PTR)hBrushBack;
		case IDC_STATIC_FRAMECOLOR:
			return (INT_PTR)hBrushFrame;
		default:
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
//アクションリストを取得
int GetActionList(int *TargetNumber, int *ActionLocate)
{
	int i, j, k, l, m, n;

	m = 0;
	for ( i = 0; i <= MAX_TARGET_THRU; i++ )
	{
		if(TargetNumber[i] >= 0)
		{
			for(j = 0; j < (int)Action.size(); j++)
			{
				if((TargetNumber[i] < (int)Target.size() && Action[j].TargetNumber == Target[TargetNumber[i]].Number) ||
					(TargetNumber[i] == (int)Target.size() && Action[j].TargetNumber == 0))
				{
					n = 0;
					for ( k = i - 1; k >= 0; k--)
					{
						if(TargetNumber[k] >= 0 && n == 0)
						{
							for ( l = 0; l < (int)Action.size(); l++ )
							{
								if(Action[l].TargetNumber == Target[TargetNumber[k]].Number &&
									Action[l].Button == Action[j].Button &&
									Action[l].Move[0] == Action[j].Move[0] &&
									Action[l].Move[1] == Action[j].Move[1] &&
									Action[l].Move[2] == Action[j].Move[2] &&
									Action[l].Move[3] == Action[j].Move[3] &&
									Action[l].Move[4] == Action[j].Move[4] &&
									Action[l].Move[5] == Action[j].Move[5] &&
									Action[l].Move[6] == Action[j].Move[6] &&
									Action[l].Move[7] == Action[j].Move[7] &&
									Action[l].Move[8] == Action[j].Move[8] &&
									Action[l].Move[9] == Action[j].Move[9])
									n = 1;
							}
						}
					}
					if(n == 0)
					{
						ActionLocate[m] = j;
						m ++;
					}
				}
			}
		}
	}
	return m;
}

INT_PTR CALLBACK DialogProcTagetEditor(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static LPWSTR* xml;
	static SIZE sizeEdit;
	static SIZE sizeButtonOk;
	static SIZE sizeButtonCancel;
	static POINT ptMinTrackSize;
	RECT rc;
	RECT rc1;
	int cx;
	int cy;

	switch(msg)
	{
	case WM_INITDIALOG:
		xml = (LPWSTR*)lParam;
		GetClientRect(hDlg, &rc);

		GetWindowRect(GetDlgItem(hDlg, IDC_EDIT_TARGET), &rc1);
		sizeEdit.cx = (rc.right-rc.left)-(rc1.right-rc1.left);
		sizeEdit.cy = (rc.bottom-rc.top)-(rc1.bottom-rc1.top);

		GetWindowRect(GetDlgItem(hDlg, IDOK), &rc1);
		MapWindowPoints(NULL, hDlg, (LPPOINT)&rc1, 2);
		sizeButtonOk.cx = (rc.right-rc.left)-rc1.left;
		sizeButtonOk.cy = (rc.bottom-rc.top)-rc1.top;

		GetWindowRect(GetDlgItem(hDlg, IDCANCEL), &rc1);
		MapWindowPoints(NULL, hDlg, (LPPOINT)&rc1, 2);
		sizeButtonCancel.cx = (rc.right-rc.left)-rc1.left;
		sizeButtonCancel.cy = (rc.bottom-rc.top)-rc1.top;

		GetWindowRect(hDlg, &rc);
		ptMinTrackSize.x = rc.right-rc.left;
		ptMinTrackSize.y = rc.bottom-rc.top;
		if(*xml)
		{
			SetWindowText(hDlg, TEXT("ターゲット設定のエクスポート"));
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT_TARGET), *xml);
		}
		else
		{
			SetWindowText(hDlg, TEXT("ターゲット設定のインポート"));
			SetWindowText(GetDlgItem(hDlg, IDC_EDIT_TARGET), TEXT(""));
		}
		break;
	case WM_SIZE:
		cx = LOWORD(lParam);
		cy = HIWORD(lParam);

		SetWindowPos(GetDlgItem(hDlg, IDOK), NULL, cx-sizeButtonOk.cx, cy-sizeButtonOk.cy, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		InvalidateRect(GetDlgItem(hDlg, IDOK), NULL, TRUE);
		SetWindowPos(GetDlgItem(hDlg, IDCANCEL), NULL, cx-sizeButtonCancel.cx, cy-sizeButtonCancel.cy, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
		InvalidateRect(GetDlgItem(hDlg, IDCANCEL), NULL, TRUE);
		SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_TARGET), NULL, 0, 0, cx-sizeEdit.cx, cy-sizeEdit.cy, SWP_NOZORDER|SWP_NOMOVE);
		InvalidateRect(GetDlgItem(hDlg, IDC_EDIT_TARGET), NULL, TRUE);
		break;
	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize = ptMinTrackSize;
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			if(*xml==NULL)
			{
				int length;
				length = GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT_TARGET));
				*xml = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (length+1)*sizeof(WCHAR));
				GetWindowText(GetDlgItem(hDlg, IDC_EDIT_TARGET), *xml, length+1);
			}
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, FALSE);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
//感度設定ダイアログ
INT_PTR CALLBACK QuantityDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	#define CheckIntervalMax 50
	#define CheckIntervalMin 1
	#define MoveQuantityMax 100
	#define MoveQuantityMin 1
	#define GestureTimeOutMax 1000
	#define GestureTimeOutMin 0
	static int GestureStartQuantityMax;
	#define GestureStartQuantityMin 1
	#define CornerTimeMax 1000
	#define CornerTimeMin 0
	#define CornerPosXMax 500
	#define CornerPosXMin 0
	#define CornerPosYMax 500
	#define CornerPosYMin 0
	#define ClickWaitMax 100
	#define ClickWaitMin 0
	#define ScrollSensitivityMax 200
	#define ScrollSensitivityMin 5
	#define CircleRateMax 100
	#define CircleRateMin 1
	#define KuruKuruTimeOutMax 3000
	#define KuruKuruTimeOutMin 20

	switch (msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_SLIDER_CHECKINTERVAL, TBM_SETRANGE, TRUE, MAKELPARAM(CheckIntervalMin, CheckIntervalMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_CHECKINTERVAL, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CHECKINTERVAL, TBM_SETPOS, TRUE, CheckInterval);
		SetDlgItemInt(hDlg, IDC_EDIT_CHECKINTERVAL, CheckInterval, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_MOVEQUANTITY, TBM_SETRANGE, TRUE, MAKELPARAM(MoveQuantityMin, MoveQuantityMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_MOVEQUANTITY, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_MOVEQUANTITY, TBM_SETPOS, TRUE, MoveQuantity);
		SetDlgItemInt(hDlg, IDC_EDIT_MOVEQUANTITY, MoveQuantity, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETIMEOUT, TBM_SETRANGE, TRUE, MAKELPARAM(GestureTimeOutMin, GestureTimeOutMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETIMEOUT, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETIMEOUT, TBM_SETPOS, TRUE, GestureTimeOut);
		SetDlgItemInt(hDlg, IDC_EDIT_GESTURETIMEOUT, GestureTimeOut, TRUE);
		GestureStartQuantityMax = MoveQuantity;
		SendDlgItemMessage(hDlg, IDC_SLIDER_STARTQUANTITY, TBM_SETRANGE, TRUE, MAKELPARAM(GestureStartQuantityMin, GestureStartQuantityMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_STARTQUANTITY, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_STARTQUANTITY, TBM_SETPOS, TRUE, GestureStartQuantity);
		SetDlgItemInt(hDlg, IDC_EDIT_STARTQUANTITY, GestureStartQuantity, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERTIME, TBM_SETRANGE, TRUE, MAKELPARAM(CornerTimeMin, CornerTimeMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERTIME, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERTIME, TBM_SETPOS, TRUE, CornerTime);
		SetDlgItemInt(hDlg, IDC_EDIT_CORNERTIME, CornerTime, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSX, TBM_SETRANGE, TRUE, MAKELPARAM(CornerPosXMin, CornerPosXMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSX, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSX, TBM_SETPOS, TRUE, CornerPosX);
		SetDlgItemInt(hDlg, IDC_EDIT_CORNERPOSX, CornerPosX, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSY, TBM_SETRANGE, TRUE, MAKELPARAM(CornerPosYMin, CornerPosYMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSY, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSY, TBM_SETPOS, TRUE, CornerPosY);
		SetDlgItemInt(hDlg, IDC_EDIT_CORNERPOSY, CornerPosY, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CLICKWAIT, TBM_SETRANGE, TRUE, MAKELPARAM(ClickWaitMin, ClickWaitMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_CLICKWAIT, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CLICKWAIT, TBM_SETPOS, TRUE, ClickWait);
		SetDlgItemInt(hDlg, IDC_EDIT_CLICKWAIT, ClickWait, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLLSENSITIVITY, TBM_SETRANGE, TRUE, MAKELPARAM(ScrollSensitivityMin, ScrollSensitivityMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLLSENSITIVITY, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLLSENSITIVITY, TBM_SETPOS, TRUE, ScrollSensitivity);
		SetDlgItemInt(hDlg, IDC_EDIT_SCROLLSENSITIVITY, ScrollSensitivity, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_KURUKURUTIMEOUT, TBM_SETRANGE, TRUE, MAKELPARAM(KuruKuruTimeOutMin, KuruKuruTimeOutMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_KURUKURUTIMEOUT, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_KURUKURUTIMEOUT, TBM_SETPOS, TRUE, KuruKuruTimeOut);
		SetDlgItemInt(hDlg, IDC_EDIT_KURUKURUTIMEOUT, KuruKuruTimeOut, TRUE);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CIRCLERATE, TBM_SETRANGE, TRUE, MAKELPARAM(CircleRateMin, CircleRateMax));
		SendDlgItemMessage(hDlg, IDC_SLIDER_CIRCLERATE, TBM_SETPAGESIZE, 0, 10);
		SendDlgItemMessage(hDlg, IDC_SLIDER_CIRCLERATE, TBM_SETPOS, TRUE, CircleRate);
		SetDlgItemInt(hDlg, IDC_EDIT_CIRCLERATE, CircleRate, TRUE);
		break;
	case WM_HSCROLL:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_SLIDER_CHECKINTERVAL:
			CheckInterval = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_CHECKINTERVAL, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_CHECKINTERVAL, CheckInterval, TRUE);
			break;
		case IDC_SLIDER_MOVEQUANTITY:
			MoveQuantity = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_MOVEQUANTITY, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_MOVEQUANTITY, MoveQuantity, TRUE);
			GestureStartQuantityMax = MoveQuantity;
			if(GestureStartQuantity > GestureStartQuantityMax)
				GestureStartQuantity = GestureStartQuantityMax;
			SendDlgItemMessage(hDlg, IDC_SLIDER_STARTQUANTITY, TBM_SETRANGE, TRUE, MAKELPARAM(GestureStartQuantityMin, GestureStartQuantityMax));
			SendDlgItemMessage(hDlg, IDC_SLIDER_STARTQUANTITY, TBM_SETPOS, TRUE, GestureStartQuantity);
			SetDlgItemInt(hDlg, IDC_EDIT_STARTQUANTITY, GestureStartQuantity, TRUE);
			break;
		case IDC_SLIDER_GESTURETIMEOUT:
			GestureTimeOut = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETIMEOUT, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_GESTURETIMEOUT, GestureTimeOut, TRUE);
			break;
		case IDC_SLIDER_STARTQUANTITY:
			GestureStartQuantity = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_STARTQUANTITY, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_STARTQUANTITY, GestureStartQuantity, TRUE);
			break;
		case IDC_SLIDER_CORNERTIME:
			CornerTime = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERTIME, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_CORNERTIME, CornerTime, TRUE);
			break;
		case IDC_SLIDER_CORNERPOSX:
			CornerPosX = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSX, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_CORNERPOSX, CornerPosX, TRUE);
			break;
		case IDC_SLIDER_CORNERPOSY:
			CornerPosY = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_CORNERPOSY, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_CORNERPOSY, CornerPosY, TRUE);
			break;
		case IDC_SLIDER_CLICKWAIT:
			ClickWait = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_CLICKWAIT, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_CLICKWAIT, ClickWait, TRUE);
			break;
		case IDC_SLIDER_SCROLLSENSITIVITY:
			ScrollSensitivity = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_SCROLLSENSITIVITY, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_SCROLLSENSITIVITY, ScrollSensitivity, TRUE);
			break;
		case IDC_SLIDER_CIRCLERATE:
			CircleRate = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_CIRCLERATE, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_CIRCLERATE, CircleRate, TRUE);
			break;
		case IDC_SLIDER_KURUKURUTIMEOUT:
			KuruKuruTimeOut = (int)SendDlgItemMessage(hDlg, IDC_SLIDER_KURUKURUTIMEOUT, TBM_GETPOS, 0, 0);
			SetDlgItemInt(hDlg, IDC_EDIT_KURUKURUTIMEOUT, KuruKuruTimeOut, TRUE);
			break;
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			break;
		case PSN_RESET:
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}
//マウ筋設定ダイアログ
INT_PTR CALLBACK IniEditDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hLVT, hLVA;
	static int ActionLocate[MAX_ACTION_DATA], TargetNumber[MAX_TARGET_THRU + 1];
	NMHDR *lpnmhdr;
	int nActiveIndexT, nActiveIndexA, LV_TargetLocate, LV_ActionLocate;
	int i, j;
	int SelectedTargetNumber;
	int iItem;
	RECT rc;
	LV_COLUMN lvcol;

	switch(msg)
	{
	case WM_INITDIALOG:
		//ターゲットリストのリストビュー
		hLVT = GetDlgItem(hDlg, IDC_LIST_TARGET);
		SetWindowTheme(hLVT, L"Explorer", NULL);
		ListView_SetImageList(hLVT, ImageList_Create(1, GetSystemMetrics(SM_CYSMICON), ILC_COLOR, 0, 0), LVSIL_SMALL);
		ListView_SetExtendedListViewStyle(hLVT, ListView_GetExtendedListViewStyle(hLVT)|LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP);
		SetWindowPos(ListView_GetToolTips(hLVT), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
		GetClientRect(hLVT, &rc);
		lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcol.fmt = LVCFMT_LEFT;
		lvcol.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL) - 2;
		lvcol.pszText = TEXT("Target");
		lvcol.iSubItem = 0;
		ListView_InsertColumn(hLVT, 0, &lvcol);
		//アクションリストのリストビュー
		hLVA = GetDlgItem(hDlg, IDC_LIST_ACTION);
		SetWindowTheme(hLVA, L"Explorer", NULL);
		GetClientRect(hLVA, &rc);
		ListView_SetImageList(hLVA, ImageList_Create(1, GetSystemMetrics(SM_CYSMICON), ILC_COLOR, 0, 0), LVSIL_SMALL);
		ListView_SetExtendedListViewStyle(hLVA, ListView_GetExtendedListViewStyle(hLVA)|LVS_EX_FULLROWSELECT|LVS_EX_INFOTIP);
		SetWindowPos(ListView_GetToolTips(hLVA), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
		lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcol.fmt = LVCFMT_LEFT;
		lvcol.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL) - 2;
		lvcol.pszText = TEXT("Action");
		lvcol.iSubItem = 0;
		ListView_InsertColumn(hLVA, 0, &lvcol);
		//ターゲットアイテムリストを更新
		for(i=0; i<=(int)Target.size(); i++)
			InsertTargetItem(hLVT, i, 0);
		//0番目を選択
		ListView_SetItemState(hLVT, 0, LVIS_SELECTED, LVIS_SELECTED);
		break;
	case WM_DESTROY:
		ImageList_Destroy(ListView_GetImageList(hLVT, LVSIL_SMALL));
		ImageList_Destroy(ListView_GetImageList(hLVA, LVSIL_SMALL));
		break;
	case WM_NOTIFY:
		lpnmhdr = (NMHDR *)lParam;
		switch(lpnmhdr->code)
		{
		case NM_DBLCLK:
			if(lpnmhdr->hwndFrom == hLVT)
			{
				if(ListView_GetNextItem(hLVT, -1, LVNI_SELECTED)==-1)
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_TARGETADD, 0);
				else
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_TARGETEDIT,0);
			}
			if(lpnmhdr->hwndFrom == hLVA)
			{
				if(ListView_GetNextItem(hLVA, -1, LVNI_SELECTED)==-1)
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_ACTIONADD, 0);
				else
					PostMessage(hDlg, WM_COMMAND, IDC_BUTTON_ACTIONEDIT,0);
			}
			break;
		case NM_RCLICK:
			if(lpnmhdr->hwndFrom == hLVT)
			{
				int iItem;
				HMENU hMenu;
				POINT pt;
				iItem = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
				hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_TARGETADD, TEXT("ターゲットを追加(&A)..."));
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_TARGETEDIT, TEXT("ターゲットを編集(&E)..."));
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_TARGETDELETE, TEXT("ターゲットを削除(&D)..."));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_TARGETCOPY, TEXT("ターゲット設定のコピーを作成(&C)"));
//				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_TARGETIMPORT, TEXT("ターゲット設定をインポート(&I)..."));
//				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_TARGETEXPORT, TEXT("ターゲット設定をエクスポート(&X)..."));
				if((int)Target.size() >= MAX_TARGET_DATA)
					EnableMenuItem(hMenu, IDC_BUTTON_TARGETADD, MF_GRAYED|MF_BYCOMMAND);
				if(iItem==-1 || iItem==(int)Target.size())	//選択なしとDefault
				{
					EnableMenuItem(hMenu, IDC_BUTTON_TARGETEDIT, MF_GRAYED|MF_BYCOMMAND);
					EnableMenuItem(hMenu, IDC_BUTTON_TARGETDELETE, MF_GRAYED|MF_BYCOMMAND);
					EnableMenuItem(hMenu, IDC_BUTTON_TARGETCOPY, MF_GRAYED|MF_BYCOMMAND);
				}
				if(iItem==-1)
				{
					EnableMenuItem(hMenu, IDC_BUTTON_TARGETEXPORT, MF_GRAYED|MF_BYCOMMAND);
				}
				GetCursorPos(&pt);
				TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, hDlg, NULL);
				DestroyMenu(hMenu);
			}
			if(lpnmhdr->hwndFrom == hLVA)
			{
				int iItem;
				HMENU hMenu;
				POINT pt;
				iItem = ListView_GetNextItem(hLVA, -1, LVNI_SELECTED);
				hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_ACTIONADD, TEXT("アクションを追加(&A)..."));
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_ACTIONEDIT, TEXT("アクションを編集(&E)..."));
				AppendMenu(hMenu, MF_STRING, IDC_BUTTON_ACTIONDELETE, TEXT("アクションを削除(&D)..."));
				if((int)Action.size() >= MAX_ACTION_DATA)
					EnableMenuItem(hMenu, IDC_BUTTON_ACTIONADD, MF_GRAYED|MF_BYCOMMAND);
				if(iItem==-1)	//選択なし
				{
					EnableMenuItem(hMenu, IDC_BUTTON_ACTIONEDIT, MF_GRAYED|MF_BYCOMMAND);
					EnableMenuItem(hMenu, IDC_BUTTON_ACTIONDELETE, MF_GRAYED|MF_BYCOMMAND);
				}
				GetCursorPos(&pt);
				TrackPopupMenuEx(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, hDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;
		case NM_SETFOCUS:
			if(lpnmhdr->hwndFrom == hLVA)
			{
				nActiveIndexA = ListView_GetNextItem(hLVA, -1, LVNI_SELECTED);
				EnableActionItem(hDlg, nActiveIndexA);
			}
			break;
		case NM_CUSTOMDRAW:
			if(lpnmhdr->hwndFrom == hLVA)
			{
				LPNMLVCUSTOMDRAW lplvcd;
				lplvcd = (NMLVCUSTOMDRAW *)lParam;
				switch(lplvcd->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)CDRF_NOTIFYITEMDRAW);
					break;
				case CDDS_ITEMPREPAINT:
					nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
					if(nActiveIndexT < (int)Target.size() &&
						Action[ActionLocate[lplvcd->nmcd.dwItemSpec]].TargetNumber != Target[nActiveIndexT].Number)
					{
						lplvcd->clrText = GetSysColor(COLOR_GRAYTEXT);
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (LONG_PTR)CDRF_NEWFONT);
					}
					break;
				}
			}
			break;
		case LVN_ITEMCHANGED:
			if(lpnmhdr->hwndFrom == hLVT)
			{
				nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
				EnableTargetItem(hDlg, nActiveIndexT);
				//アクションアイテムリスト更新
				ListView_DeleteAllItems(hLVA);
				if(nActiveIndexT != -1)
				{
					SortAction();
					GetThruNumber(nActiveIndexT, TargetNumber);
					i = GetActionList(TargetNumber, ActionLocate);
					for(j = 0; j < i; j++)
						InsertActionItem(hLVA, ActionLocate[j], j);
				}
			}
			if(lpnmhdr->hwndFrom == hLVA)
			{
				nActiveIndexA = ListView_GetNextItem(hLVA, -1, LVNI_SELECTED);
				EnableActionItem(hDlg, nActiveIndexA);
			}
			break;
		case LVN_GETINFOTIP:
			if(lpnmhdr->hwndFrom == hLVT)
			{
				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)lParam;
				tstring str;

				if(pGetInfoTip->iItem==(int)Target.size())
				{
					str = TEXT("マウスメッセージを横取りする: ");
					str += Default.HookType ? TEXT("有効") : TEXT("無効");
					str += TEXT("\n");
					str += TEXT("非アクティブでもスクロール: ");
					str += Default.WheelRedirect ? TEXT("有効") : TEXT("無効");
					str += TEXT("\n");
					str += TEXT("フリースクロール: ");
					str += Default.FreeScroll ? TEXT("有効") : TEXT("無効");
				}
				else
				{
					TCHAR szBaseTargetName[1024];

					if(Target[pGetInfoTip->iItem].DefaultThru<0)
					{
						lstrcpy(szBaseTargetName, TEXT("継承しない"));
					}
					else if(Target[pGetInfoTip->iItem].DefaultThru==0)
					{
						lstrcpy(szBaseTargetName, TEXT("Default"));
					}
					else
					{
						int i;

						for(i=0;i<(int)Target.size();i++)
						{
							if(Target[i].Number==Target[pGetInfoTip->iItem].DefaultThru)
							{
								if(Target[i].Comment[0])
									lstrcpy(szBaseTargetName, Target[i].Comment);
								else
									wsprintf(szBaseTargetName, TEXT("%s %s"), Target[i].FileName[0] ? Target[i].FileName : TEXT("---"), Target[i].ClassName);
							}
						}
					}

					str = TEXT("ファイル名: ");
					str += Target[pGetInfoTip->iItem].FileName;
					str += TEXT("\n");
					str += TEXT("クラス名: ");
					str += Target[pGetInfoTip->iItem].ClassName;
					str += TEXT("\n");
					str += TEXT("タイトル:");
					str += Target[pGetInfoTip->iItem].WindowTitle;
					str += TEXT("\n");
					str += TEXT("ID: ");
					str += Target[pGetInfoTip->iItem].ControlID;
					str += TEXT("\n");
					str += TEXT("継承: ");
					str += szBaseTargetName;
					str += TEXT("\n");
					str += TEXT("マウスメッセージを横取りする: ");
					str += Target[pGetInfoTip->iItem].HookType ? TEXT("有効") : TEXT("無効");
					str += TEXT("\n");
					str += TEXT("非アクティブでもスクロール: ");
					str += Target[pGetInfoTip->iItem].WheelRedirect ? TEXT("有効") : TEXT("無効");
					str += TEXT("\n");
					str += TEXT("フリースクロール: ");
					str += Target[pGetInfoTip->iItem].FreeScroll ? TEXT("有効") : TEXT("無効");
					str += TEXT("\n");
					str += TEXT("コメント: ");
					str += Target[pGetInfoTip->iItem].Comment;
				}
				lstrcpyn(pGetInfoTip->pszText, str.c_str(), pGetInfoTip->cchTextMax);
			}
			if(lpnmhdr->hwndFrom == hLVA)
			{
				//todo:ツールチップで何か表示したい
//				LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)lParam;
//				TCHAR szText[1024];
//				wsprintf(szText, TEXT("%s"), Action[ActionLocate[pGetInfoTip->iItem]].Comment);
//				lstrcpyn(pGetInfoTip->pszText, szText, pGetInfoTip->cchTextMax);
			}
			break;
		case PSN_APPLY:
			break;
		case PSN_RESET:
			break;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_TARGETADD:	//ターゲット追加
		case IDC_BUTTON_TARGETEDIT:	//ターゲット編集
			if(LOWORD(wParam)==IDC_BUTTON_TARGETADD)	//ターゲット追加
			{
				if((int)Target.size() >= MAX_TARGET_DATA)
					break;
				LV_TargetLocate = -1;
			}
			if(LOWORD(wParam)==IDC_BUTTON_TARGETEDIT)	//ターゲット編集
			{
				LV_TargetLocate = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
				if(LV_TargetLocate == -1 || LV_TargetLocate == (int)Target.size())//Target:Defaultは編集しないため
					break;
			}

			if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_TARGET), hDlg, TargetDialogProc, (LPARAM)LV_TargetLocate))
			{
				int iItem;
				if(LOWORD(wParam)==IDC_BUTTON_TARGETADD)
				{
					InsertTargetItem(hLVT, (int)Target.size()-1, 0);
					iItem = ListView_GetItemCount(hLVT)-2;
				}
				if(LOWORD(wParam)==IDC_BUTTON_TARGETEDIT)
				{
					InsertTargetItem(hLVT, LV_TargetLocate, 0);
					iItem = LV_TargetLocate;
				}
				ListView_SetItemState(hLVT, -1, 0, LVIS_SELECTED);	//LVN_ITEMCHANGEDが通知されるように選択を解除しておく
				ListView_SetItemState(hLVT, iItem, LVIS_SELECTED, LVIS_SELECTED);	//ターゲットを選択
				ListView_EnsureVisible(hLVT, ListView_GetNextItem(hLVT, -1, LVNI_SELECTED), TRUE);
			}
			break;
		case IDC_BUTTON_ACTIONADD:	//アクション追加
		case IDC_BUTTON_ACTIONEDIT:	//アクション編集
			if(LOWORD(wParam)==IDC_BUTTON_ACTIONADD)	//アクション追加
			{
				if((int)Action.size() >= MAX_ACTION_DATA)
					break;

				LV_TargetLocate = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
				if(LV_TargetLocate == -1)
				{
					ListView_SetItemState(hLVT, ListView_GetItemCount(hLVT)-1, LVIS_SELECTED, LVIS_SELECTED);
					LV_TargetLocate = (int)Target.size();
				}

				LV_ActionLocate = (int)Action.size();
				if(LV_TargetLocate < (int)Target.size())
					SelectedTargetNumber = Target[LV_TargetLocate].Number;
				else
					SelectedTargetNumber = 0;
			}
			if(LOWORD(wParam)==IDC_BUTTON_ACTIONEDIT)	//アクション編集
			{
				nActiveIndexA = ListView_GetNextItem(hLVA, -1, LVNI_SELECTED);
				if(nActiveIndexA == -1)
					break;

				LV_TargetLocate = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
				if(LV_TargetLocate == -1)
					LV_TargetLocate = (int)Target.size();

				if(LV_TargetLocate < (int)Target.size() &&
					nActiveIndexA >= 0 && Action[ActionLocate[nActiveIndexA]].TargetNumber != Target[TargetNumber[0]].Number)
				{
					if(MessageBox(hDlg, TEXT("このアクションを編集すると、\n他の設定にも影響を与える可能性がありますがよろしいですか？"), TEXT("継承しているアクションを編集しようとしています"), MB_YESNO) != IDYES)
						break;
				}

				LV_ActionLocate = ActionLocate[nActiveIndexA];
				SelectedTargetNumber = Action[LV_ActionLocate].TargetNumber;
			}

			if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_ACTION), hDlg, ActionDialogProc, MAKELPARAM(LV_ActionLocate, SelectedTargetNumber)))
			{
				ListView_SetItemState(hLVT, -1, 0, LVIS_SELECTED);	//LVN_ITEMCHANGEDが通知されるように選択を解除しておく
				ListView_SetItemState(hLVT, LV_TargetLocate, LVIS_SELECTED, LVIS_SELECTED);	//ターゲットを選択
				//todo
				//アクションを追加/編集した後、それを選択した状態にしたい
				//とりあえず、選択がない状態にしておこう
			}
			break;
		case IDC_BUTTON_TARGETDELETE:
			if(MessageBox(hDlg, TEXT("このターゲットの登録を抹消してもよろしいですか？\nこのターゲットに対するアクションは全て削除されます。"), TEXT("ターゲットの削除"), MB_YESNO) != IDYES)
				break;
			nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
			if(nActiveIndexT == -1)
				break;
			iItem = nActiveIndexT;	//削除するアイテムの位置
			TargetDelete(nActiveIndexT);
			ListView_DeleteItem(hLVT, nActiveIndexT);
			if(ListView_GetItemCount(hLVT)>0)
			{
				if(iItem==ListView_GetItemCount(hLVT))
					iItem--;
				ListView_SetItemState(hLVT, iItem, LVIS_SELECTED, LVIS_SELECTED);
			}
			IniChange = TRUE;
			break;
		case IDC_BUTTON_ACTIONDELETE:
			nActiveIndexA = ListView_GetNextItem(hLVA, -1, LVNI_SELECTED);
			if(nActiveIndexA == -1)
				break;
			nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
			if(nActiveIndexT == -1)
				break;
			if(nActiveIndexT < (int)Target.size() &&
				Action[ActionLocate[nActiveIndexA]].TargetNumber != Target[TargetNumber[0]].Number)
			{
				if(MessageBox(hDlg, TEXT("このアクションを削除すると、\n他の設定にも影響を与える可能性がありますがよろしいですか？"), TEXT("継承しているアクションを削除しようとしています"), MB_YESNO) != IDYES)
					break;
			}
			else
			{
				if(MessageBox(hDlg, TEXT("このアクションを削除してもよろしいですか？"), TEXT("アクションの削除"), MB_YESNO) != IDYES)
					break;
			}
			iItem = nActiveIndexA;	//削除するアイテムの位置
			ActionDelete(ActionLocate[nActiveIndexA]);
			ListView_SetItemState(hLVT, -1, 0, LVIS_SELECTED);	//LVN_ITEMCHANGEDが通知されるように選択を解除しておく
			ListView_SetItemState(hLVT, nActiveIndexT, LVIS_SELECTED, LVIS_SELECTED);	//ターゲットを選択
			//アクションを選択
			if(ListView_GetItemCount(hLVA)>0)
			{
				if(iItem==ListView_GetItemCount(hLVA))
					iItem--;
				ListView_SetItemState(hLVA, iItem, LVIS_SELECTED, LVIS_SELECTED);
			}
			IniChange = TRUE;
			break;
		case IDC_BUTTON_TARGETUP:
			nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
			if(nActiveIndexT > 0 && nActiveIndexT < (int)Target.size())
			{
				std::swap(Target[nActiveIndexT - 1], Target[nActiveIndexT]);
				ListView_DeleteItem(hLVT, nActiveIndexT - 1);
				InsertTargetItem(hLVT, nActiveIndexT, 1);
				ListView_SetItemState(hLVT, -1, 0, LVIS_SELECTED);	//LVN_ITEMCHANGEDが通知されるように選択を解除しておく
				ListView_SetItemState(hLVT, nActiveIndexT-1, LVIS_SELECTED, LVIS_SELECTED);	//ターゲットを選択
				ListView_EnsureVisible(hLVT, ListView_GetNextItem(hLVT, -1, LVNI_SELECTED), TRUE);
				IniChange = TRUE;
			}
			break;
		case IDC_BUTTON_TARGETDOWN:
			nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
			if(nActiveIndexT >= 0 && nActiveIndexT < (int)Target.size() - 1)
			{
				std::swap(Target[nActiveIndexT], Target[nActiveIndexT + 1]);
				ListView_DeleteItem(hLVT, nActiveIndexT + 1);
				InsertTargetItem(hLVT, nActiveIndexT, 1);
				ListView_SetItemState(hLVT, -1, 0, LVIS_SELECTED);	//LVN_ITEMCHANGEDが通知されるように選択を解除しておく
				ListView_SetItemState(hLVT, nActiveIndexT+1, LVIS_SELECTED, LVIS_SELECTED);	//ターゲットを選択
				ListView_EnsureVisible(hLVT, ListView_GetNextItem(hLVT, -1, LVNI_SELECTED), TRUE);
				IniChange = TRUE;
			}
			break;
		case IDC_BUTTON_TARGETCOPY:
			nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
			if(nActiveIndexT!=-1 && nActiveIndexT<(int)Target.size())
			{
				CONFIG config;
				_GetConfig(&config);
				config.target[nActiveIndexT].Number = MinTargetNumber();
				_AddTarget(&config.target[nActiveIndexT]);
				//ターゲットアイテムリストを更新
				ListView_DeleteAllItems(hLVT);
				for(i=0;i<=(int)Target.size();i++)
					InsertTargetItem(hLVT, i, 0);
				//作成したコピーを選択
				ListView_SetItemState(hLVT, ListView_GetItemCount(hLVT)-2, LVIS_SELECTED, LVIS_SELECTED);
				ListView_EnsureVisible(hLVT, ListView_GetNextItem(hLVT, -1, LVNI_SELECTED), TRUE);
			}
			break;
		case IDC_BUTTON_TARGETIMPORT:
		{
			LPWSTR xml = NULL;
			if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_TAGETEDITOR), hDlg, DialogProcTagetEditor, (LPARAM)&xml)==IDOK)
			{
				if(xml)
				{
					CONFIG config;
					int i, ii, iii;
					ConfigReadXmlString(xml, &config, NULL);
					HeapFree(GetProcessHeap(), 0, xml);
					for(i=0;i<(int)config.target.size();i++)
					{
						config.target[i].DefaultThru = 0;	//継承を"default"に変更
						for(ii=0;ii<(int)config.target[i].action.size();ii++)
						{
							for(iii=0;iii<(int)config.target[i].action[ii].command.size();iii++)
							{
								config.target[i].action[ii].command[iii].CommandTarget = 0;	//対象を"アクティブウィンドウ"に変更
							}
						}
						config.target[i].Number = MinTargetNumber();
						_AddTarget(&config.target[i]);
					}
					//ターゲットアイテムリストを更新
					ListView_DeleteAllItems(hLVT);
					for(i=0;i<=(int)Target.size();i++)
						InsertTargetItem(hLVT, i, 0);
					//インポートしたターゲットを選択
					ListView_SetItemState(hLVT, ListView_GetItemCount(hLVT)-2, LVIS_SELECTED, LVIS_SELECTED);
					ListView_EnsureVisible(hLVT, ListView_GetNextItem(hLVT, -1, LVNI_SELECTED), TRUE);
				}
			}
			break;
		}
		case IDC_BUTTON_TARGETEXPORT:
			nActiveIndexT = ListView_GetNextItem(hLVT, -1, LVNI_SELECTED);
			if(nActiveIndexT!=-1)
			{
				CONFIG config;
				LPWSTR xml;
				_GetConfig(&config);
				ConfigWriteXmlString(&config, &xml, 0, 2, nActiveIndexT);	//BOMなし、XML宣言なし、指定したひとつのターゲット
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_TAGETEDITOR), hDlg, DialogProcTagetEditor, (LPARAM)&xml);
				HeapFree(GetProcessHeap(), 0, xml);
			}
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK GeneralDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwnd;
	BOOL IniChangeTemp;
	LPNMHDR pnmhdr;
	OPENFILENAME ofn;

	switch(msg)
	{
	case WM_INITDIALOG:
		hwnd = (HWND)lParam;
		if(MouseHookFlag)
			CheckDlgButton(hDlg, IDC_CHECKBOX_MOUSEHOOK, BST_CHECKED);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_WHEELREDIRECT), (MouseHookFlag)?TRUE:FALSE);
		EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_WHEELACTIVE), (MouseHookFlag&WheelRedirect)?TRUE:FALSE);
		if(WheelRedirect)
			CheckDlgButton(hDlg, IDC_CHECKBOX_WHEELREDIRECT, BST_CHECKED);
		if(CursorChange)
			CheckDlgButton(hDlg, IDC_CHECKBOX_CURSORCHANGE, BST_CHECKED);
		IniChangeTemp = IniChange;
		SetDlgItemText(hDlg, IDC_EDIT_GESTURECURSOR, GestureCursor);
		SetDlgItemText(hDlg, IDC_EDIT_WAITCURSOR, WaitCursor);
		IniChange = IniChangeTemp;
		if(ShowTaskTray)
			CheckDlgButton(hDlg, IDC_CHECKBOX_SHOWTASKTRAY, BST_CHECKED);
		if(Priority == 0x0080)
			CheckDlgButton(hDlg, IDC_CHECKBOX_HIGH_PRIORITY, BST_CHECKED);
		if(GestureDirection == 4)
			CheckRadioButton(hDlg, IDC_RADIO_GESTUREDIRECTION_4, IDC_RADIO_GESTUREDIRECTION_44, IDC_RADIO_GESTUREDIRECTION_4);
		else if(GestureDirection == 8)
			CheckRadioButton(hDlg, IDC_RADIO_GESTUREDIRECTION_4, IDC_RADIO_GESTUREDIRECTION_44, IDC_RADIO_GESTUREDIRECTION_8);
		else if(GestureDirection == -8)
			CheckRadioButton(hDlg, IDC_RADIO_GESTUREDIRECTION_4, IDC_RADIO_GESTUREDIRECTION_44, IDC_RADIO_GESTUREDIRECTION_84);
		else if(GestureDirection == -4)
			CheckRadioButton(hDlg, IDC_RADIO_GESTUREDIRECTION_4, IDC_RADIO_GESTUREDIRECTION_44, IDC_RADIO_GESTUREDIRECTION_44);
		if(TimeOutType)
			CheckDlgButton(hDlg, IDC_CHECKBOX_TIMEOUTTYPE, BST_CHECKED);
		if(WheelActive)
			CheckDlgButton(hDlg, IDC_CHECKBOX_WHEELACTIVE, BST_CHECKED);
		if(KuruKuruFlag)
			CheckDlgButton(hDlg, IDC_CHECKBOX_KURUKURU, BST_CHECKED);
		if(Default.HookType)
			CheckDlgButton(hDlg, IDC_CHECKBOX_DEFAULTHOOKTYPE, BST_CHECKED);
		if(Default.WheelRedirect)
			CheckDlgButton(hDlg, IDC_CHECKBOX_DEFAULTWHEELREDIRECT, BST_CHECKED);
		if(Default.FreeScroll)
			CheckDlgButton(hDlg, IDC_CHECKBOX_FREESCROLLMODE, BST_CHECKED);
		break;
	case WM_NOTIFY:
		pnmhdr = (LPNMHDR)lParam;
		switch(pnmhdr->code)
		{
		case PSN_APPLY:
			break;
		case PSN_RESET:
			break;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHECKBOX_MOUSEHOOK:
			MouseHookFlag = !MouseHookFlag;
			if(MouseHookFlag)
				SetMouseHook(hwnd);
			else
				MouseUnHook();
			CheckDlgButton(hDlg, IDC_CHECKBOX_MOUSEHOOK, MouseHookFlag ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_WHEELREDIRECT), (MouseHookFlag)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_WHEELACTIVE), (MouseHookFlag&WheelRedirect)?TRUE:FALSE);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_WHEELREDIRECT:
			WheelRedirect = !WheelRedirect;
			SetWheelRedirectFlag(WheelRedirect);
			CheckDlgButton(hDlg, IDC_CHECKBOX_WHEELREDIRECT, WheelRedirect ? BST_CHECKED : BST_UNCHECKED);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_WHEELREDIRECT), (MouseHookFlag)?TRUE:FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECKBOX_WHEELACTIVE), (MouseHookFlag&WheelRedirect)?TRUE:FALSE);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_CURSORCHANGE:
			CursorChange = !CursorChange;
			SetCursorHandle(GestureCursor, WaitCursor, CursorChange);
			CheckDlgButton(hDlg, IDC_CHECKBOX_CURSORCHANGE, CursorChange ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_EDIT_GESTURECURSOR:
			if(HIWORD(wParam)==EN_UPDATE)
			{
				GetDlgItemText(hDlg, IDC_EDIT_GESTURECURSOR, GestureCursor, _MAX_PATH);
				SetCursorHandle(GestureCursor, WaitCursor, CursorChange);
				IniChange = TRUE;
			}
			break;
		case IDC_BUTTON_GESTURECURSOR:
			memset(&ofn, 0, sizeof(OPENFILENAME));    
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = TEXT("ポインタ (*.ani, *.cur)\0*.ani;*.cur\0アニメーション ポインタ (*.ani)\0*.ani\0静的ポインタ (*.cur)\0*.cur\0すべてのファイル\0*.*\0"); //フィルタ
			ofn.lpstrFile = GestureCursor; //選択されたファイル名を受け取る(フルパス)
			ofn.nMaxFile = _MAX_PATH;
			ofn.lpstrTitle = TEXT("ジェスチャーカーソルの選択"); //ダイアログボックスのタイトル
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if(GetOpenFileName(&ofn))
				SetDlgItemText(hDlg, IDC_EDIT_GESTURECURSOR, GestureCursor);
			break;
		case IDC_EDIT_WAITCURSOR:
			if(HIWORD(wParam)==EN_UPDATE)
			{
				GetDlgItemText(hDlg, IDC_EDIT_WAITCURSOR, WaitCursor, _MAX_PATH);
				SetCursorHandle(GestureCursor, WaitCursor, CursorChange);
				IniChange = TRUE;
			}
			break;
		case IDC_BUTTON_WAITCURSOR:
			memset(&ofn, 0, sizeof(OPENFILENAME));    
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = TEXT("ポインタ (*.ani, *.cur)\0*.ani;*.cur\0アニメーション ポインタ (*.ani)\0*.ani\0静的ポインタ (*.cur)\0*.cur\0すべてのファイル\0*.*\0"); //フィルタ
			ofn.lpstrFile = WaitCursor; //選択されたファイル名を受け取る(フルパス)
			ofn.nMaxFile = _MAX_PATH;
			ofn.lpstrTitle = TEXT("ジェスチャー開始待ちカーソルの選択"); //ダイアログボックスのタイトル
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if(GetOpenFileName(&ofn))
				SetDlgItemText(hDlg, IDC_EDIT_WAITCURSOR, WaitCursor);
			break;
		case IDC_CHECKBOX_SHOWTASKTRAY:
			ShowTaskTray = !ShowTaskTray;
			if(ShowTaskTray)
				MyTaskTray(hwnd);
			else
				MyTaskTray(NULL);
			CheckDlgButton(hDlg, IDC_CHECKBOX_SHOWTASKTRAY, ShowTaskTray ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_HIGH_PRIORITY:
			if(Priority!=0x0080)
				Priority = 0x0080;
			else
				Priority = 0x0020;
			SetPriorityClass(GetCurrentProcess(), Priority);
			CheckDlgButton(hDlg, IDC_CHECKBOX_HIGH_PRIORITY, (Priority==0x0080) ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_RADIO_GESTUREDIRECTION_4:
			GestureDirection = 4;
			IniChange = TRUE;
			break;
		case IDC_RADIO_GESTUREDIRECTION_8:
			GestureDirection = 8;
			IniChange = TRUE;
			break;
		case IDC_RADIO_GESTUREDIRECTION_84:
			GestureDirection = -8;
			IniChange = TRUE;
			break;
		case IDC_RADIO_GESTUREDIRECTION_44:
			GestureDirection = -4;
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_TIMEOUTTYPE:
			TimeOutType = !TimeOutType;
			CheckDlgButton(hDlg, IDC_CHECKBOX_TIMEOUTTYPE, TimeOutType ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_WHEELACTIVE:
			WheelActive = !WheelActive;
			SetWheelActiveFlag(WheelActive);
			CheckDlgButton(hDlg, IDC_CHECKBOX_WHEELACTIVE, WheelActive ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_KURUKURU:
			KuruKuruFlag = !KuruKuruFlag;
			CheckDlgButton(hDlg, IDC_CHECKBOX_KURUKURU, KuruKuruFlag ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_DEFAULTHOOKTYPE:
			Default.HookType = !Default.HookType;
			CheckDlgButton(hDlg, IDC_CHECKBOX_DEFAULTHOOKTYPE, Default.HookType ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_DEFAULTWHEELREDIRECT:
			Default.WheelRedirect = !Default.WheelRedirect;
			CheckDlgButton(hDlg, IDC_CHECKBOX_DEFAULTWHEELREDIRECT, Default.WheelRedirect ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		case IDC_CHECKBOX_FREESCROLLMODE:
			Default.FreeScroll = !Default.FreeScroll;
			CheckDlgButton(hDlg, IDC_CHECKBOX_FREESCROLLMODE, Default.FreeScroll ? BST_CHECKED : BST_UNCHECKED);
			IniChange = TRUE;
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK GestureTrailDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR pnmhdr;
	TCHAR szText[1024];
	CHOOSECOLOR cc;
	static COLORREF CustColors[16];
	static HBRUSH hBrush;

	switch(msg)
	{
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETRAILWIDTH, TBM_SETRANGE, TRUE, MAKELPARAM(1, 20));
		if(MouseGestureTrail)
			CheckDlgButton(hDlg, IDC_CHECKBOX_GESTURETRAIL, BST_CHECKED);
		SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETRAILWIDTH, TBM_SETPOS, TRUE, MouseGestureTrailWidth);
		wsprintf(szText, TEXT("%d"), MouseGestureTrailWidth);
		SetDlgItemText(hDlg, IDC_EDIT_GESTURETRAILWIDTH, szText);
		if(MouseGestureTrailDrawMode==0)
			CheckRadioButton(hDlg, IDC_RADIO_GESTURETRAILMODE_AUTO, IDC_RADIO_GESTURETRAILMODE_LAYEREDWINDOW, IDC_RADIO_GESTURETRAILMODE_AUTO);
		else if(MouseGestureTrailDrawMode==1)
			CheckRadioButton(hDlg, IDC_RADIO_GESTURETRAILMODE_AUTO, IDC_RADIO_GESTURETRAILMODE_LAYEREDWINDOW, IDC_RADIO_GESTURETRAILMODE_DESKTOP);
		else if(MouseGestureTrailDrawMode==2)
			CheckRadioButton(hDlg, IDC_RADIO_GESTURETRAILMODE_AUTO, IDC_RADIO_GESTURETRAILMODE_LAYEREDWINDOW, IDC_RADIO_GESTURETRAILMODE_LAYEREDWINDOW);
		hBrush = CreateSolidBrush(MouseGestureTrailColor);
		break;
	case WM_DESTROY:
		DeleteObject(hBrush);
		break;
	case WM_NOTIFY:
		pnmhdr = (LPNMHDR)lParam;
		switch(pnmhdr->code)
		{
		case PSN_APPLY:
			break;
		case PSN_RESET:
			break;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_CHECKBOX_GESTURETRAIL:
			MouseGestureTrail = !MouseGestureTrail;
			break;
		case IDC_BUTTON_GESTURETRAILCOLOR:
			memset(&cc, 0, sizeof(CHOOSECOLOR));    
			cc.lStructSize  = sizeof (CHOOSECOLOR);
			cc.hwndOwner    = hDlg;
			cc.rgbResult    = MouseGestureTrailColor;
			cc.lpCustColors	= CustColors;
			cc.Flags        = CC_RGBINIT;
			if(ChooseColor(&cc))
			{
				MouseGestureTrailColor = cc.rgbResult;
				DeleteObject(hBrush);
				hBrush = CreateSolidBrush(MouseGestureTrailColor);
				InvalidateRect(hDlg, NULL, TRUE);
			}
			break;
		case IDC_RADIO_GESTURETRAILMODE_AUTO:
			MouseGestureTrailDrawMode = 0;
			SetMouseGestureTrailModeAuto();
			break;
		case IDC_RADIO_GESTURETRAILMODE_DESKTOP:
			MouseGestureTrailDrawMode = 1;
			SetMouseGestureTrailMode(0);
			break;
		case IDC_RADIO_GESTURETRAILMODE_LAYEREDWINDOW:
			MouseGestureTrailDrawMode = 2;
			SetMouseGestureTrailMode(1);
			break;
		}
		break;
	case WM_HSCROLL:
		if(GetDlgItem(hDlg, IDC_SLIDER_GESTURETRAILWIDTH)==(HWND)lParam)
		{
			MouseGestureTrailWidth = (DWORD)SendDlgItemMessage(hDlg, IDC_SLIDER_GESTURETRAILWIDTH, TBM_GETPOS, 0, 0);
			wsprintf(szText, TEXT("%d"), MouseGestureTrailWidth);
			SetDlgItemText(hDlg, IDC_EDIT_GESTURETRAILWIDTH, szText);
		}
		break;
	case WM_CTLCOLORSTATIC:
		switch(GetDlgCtrlID((HWND)lParam))
		{
		case IDC_STATIC_GESTURETRAILCOLOR:
			return (INT_PTR)hBrush;
		default:
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK DialogProcOption(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hwnd;
	#define MAX_PAGES 5
	static HWND hDlgPage[MAX_PAGES];
	static BOOL bChanged[MAX_PAGES];
	TV_INSERTSTRUCT is;
	RECT rc;
	POINT pt;
	LPNMHDR pnmhdr;
	int i;
	PSHNOTIFY psn;
	HMENU hMenu;

	switch(msg)
	{
	case WM_INITDIALOG:
		hwnd = (HWND)lParam;
		hDlgOption = hDlg;
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)));
		SetWindowTheme(GetDlgItem(hDlg, IDC_TREE_PAGE), L"Explorer", NULL);
		TreeView_SetItemHeight(GetDlgItem(hDlg, IDC_TREE_PAGE), 32);
		//ツリー
		is.hParent = TVI_ROOT;
		is.hInsertAfter = TVI_LAST;
		is.item.mask = TVIF_TEXT|TVIF_PARAM;
		is.item.pszText = TEXT("マウスジェスチャー");
		is.item.lParam = 0;
		TreeView_InsertItem(GetDlgItem(hDlg, IDC_TREE_PAGE), &is);
		is.item.pszText = TEXT("全般");
		is.item.lParam = 1;
		TreeView_InsertItem(GetDlgItem(hDlg, IDC_TREE_PAGE), &is);
		is.item.pszText = TEXT("マウ筋ナビ");
		is.item.lParam = 2;
		TreeView_InsertItem(GetDlgItem(hDlg, IDC_TREE_PAGE), &is);
		is.item.pszText = TEXT("マウス感度");
		is.item.lParam = 3;
		TreeView_InsertItem(GetDlgItem(hDlg, IDC_TREE_PAGE), &is);
		is.item.pszText = TEXT("軌跡");
		is.item.lParam = 4;
		TreeView_InsertItem(GetDlgItem(hDlg, IDC_TREE_PAGE), &is);
		//ページ
		GetWindowRect(GetDlgItem(hDlg, IDC_STATIC_PAGE), &rc);
		pt.x = rc.left;
		pt.y = rc.top;
		ScreenToClient(hDlg, &pt);
		hDlgPage[0] = CreateDialogParam(hInstance, MAKEINTATOM(IDD_DIALOG_INIEDIT), hDlg, IniEditDialogProc, (LPARAM)hwnd);
		SetWindowPos(hDlgPage[0], GetDlgItem(hDlg, IDC_TREE_PAGE), pt.x, pt.y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
		hDlgPage[1] = CreateDialogParam(hInstance, MAKEINTATOM(IDD_DIALOG_GENERAL), hDlg, GeneralDialogProc, (LPARAM)hwnd);
		SetWindowPos(hDlgPage[1], GetDlgItem(hDlg, IDC_TREE_PAGE), pt.x, pt.y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
		hDlgPage[2] = CreateDialogParam(hInstance, MAKEINTATOM(IDD_DIALOG_NAVIEDIT), hDlg, NaviEditDialogProc, (LPARAM)hwnd);
		SetWindowPos(hDlgPage[2], GetDlgItem(hDlg, IDC_TREE_PAGE), pt.x, pt.y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
		hDlgPage[3] = CreateDialogParam(hInstance, MAKEINTATOM(IDD_DIALOG_QUANTITY), hDlg, QuantityDialogProc, (LPARAM)hwnd);
		SetWindowPos(hDlgPage[3], GetDlgItem(hDlg, IDC_TREE_PAGE), pt.x, pt.y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
		hDlgPage[4] = CreateDialogParam(hInstance, MAKEINTATOM(IDD_DIALOG_GESTURETRAIL), hDlg, GestureTrailDialogProc, (LPARAM)hwnd);
		SetWindowPos(hDlgPage[4], GetDlgItem(hDlg, IDC_TREE_PAGE), pt.x, pt.y, rc.right-rc.left, rc.bottom-rc.top, SWP_NOACTIVATE);
		for(i=0;i<MAX_PAGES;i++)
			bChanged[i] = FALSE;
		//システムメニュー
		hMenu = GetSystemMenu(hDlg, FALSE);
		InsertMenu(hMenu, 0, MF_BYPOSITION|MF_SEPARATOR, 0, NULL);
		InsertMenu(hMenu, 0, MF_STRING|MF_BYPOSITION, 1, TEXT("マウ筋を終了(&X)"));
		InsertMenu(hMenu, 0, MF_STRING|MF_BYPOSITION, 2, TEXT("ヘルプを起動(&H)..."));
		InsertMenu(hMenu, 0, MF_STRING|MF_BYPOSITION, 3, TEXT("バージョン情報(&A)..."));
		break;
	case WM_DESTROY:
		for(i=0;i<MAX_PAGES;i++)
		{
			if(hDlgPage[i])
				DestroyWindow(hDlgPage[i]);
			hDlgPage[i] = NULL;
		}
		hDlgOption = NULL;
		break;
	case WM_NOTIFY:
		pnmhdr = (LPNMHDR)lParam;
		if(pnmhdr->idFrom==IDC_TREE_PAGE)
		{
			LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)lParam;
			int i;
			BOOL bExist = FALSE;
			switch(pnmtv->hdr.code)
			{
			case TVN_SELCHANGED:
				for(i=0;i<MAX_PAGES;i++)
				{
					if(pnmtv->itemNew.lParam==i)
						ShowWindow(hDlgPage[i], SW_SHOW);
					else
						ShowWindow(hDlgPage[i], SW_HIDE);
				}
				break;
			}
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_EXIT:
			PostMessage(hwnd, WM_CLOSE, 0, 0);	//メインウィンドウにWM_CLOSEをポストする
			break;
		case IDC_BUTTON_HELP:
			OpenHelp();
			break;
		case IDC_BUTTON_VERSION:
			ShowVersionInfo(hDlg);
			break;
		case IDOK:
		case 12321:	//12321=適用ボタン
			psn.hdr.code = PSN_APPLY;
			for(i=0;i<MAX_PAGES;i++)
			{
				if(hDlgPage[i])
					SendMessage(hDlgPage[i], WM_NOTIFY, 0, (LPARAM)&psn);
			}
			for(i=0;i<MAX_PAGES;i++)
				bChanged[i] = FALSE;
			EnableWindow(GetDlgItem(hDlg, 12321), FALSE);
			if(LOWORD(wParam)==IDOK)
				EndDialog(hDlg, IDOK);
			break;
		case IDCANCEL:
			psn.hdr.code = PSN_RESET;
			for(i=0;i<MAX_PAGES;i++)
			{
				if(hDlgPage[i])
					SendMessage(hDlgPage[i], WM_NOTIFY, 0, (LPARAM)&psn);
			}
			EndDialog(hDlg, IDCANCEL);
			break;
		}
		break;
	case WM_SYSCOMMAND:
		switch(LOWORD(wParam))
		{
		case 1:
			SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_EXIT, 0);
			break;
		case 2:
			SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_HELP, 0);
			break;
		case 3:
			SendMessage(hDlg, WM_COMMAND, IDC_BUTTON_VERSION, 0);
			break;
		default:
			return FALSE;
		}
		break;
	case PSM_CHANGED:
		for(i=0;i<MAX_PAGES;i++)
		{
			if(hDlgPage[i]==(HWND)wParam)
				bChanged[i] = TRUE;
		}
		EnableWindow(GetDlgItem(hDlg, 12321), TRUE);
		break;
	default:
		if(msg==WM_APP+WM_COMMAND)
		{
			switch(LOWORD(wParam))
			{
			case IDM_MOUSEHOOK:
				SendMessage(hDlgPage[1], WM_COMMAND, MAKEWPARAM(IDC_CHECKBOX_MOUSEHOOK, 0), 0);
				break;
			case IDM_WHEELREDIRECT:
				SendMessage(hDlgPage[1], WM_COMMAND, MAKEWPARAM(IDC_CHECKBOX_WHEELREDIRECT, 0), 0);
				break;
			case IDM_HIDEMAUSUJINAVI:
				CheckRadioButton(hDlgPage[2], IDC_RADIO_NAVITYPE_NONE, IDC_RADIO_NAVITYPE_FLOAT, IDC_RADIO_NAVITYPE_NONE);
				break;
			case IDM_SHOWMAUSUJINAVI:
				CheckRadioButton(hDlgPage[2], IDC_RADIO_NAVITYPE_NONE, IDC_RADIO_NAVITYPE_FLOAT, IDC_RADIO_NAVITYPE_FIXED);
				break;
			case IDM_TIPMAUSUJINAVI:
				CheckRadioButton(hDlgPage[2], IDC_RADIO_NAVITYPE_NONE, IDC_RADIO_NAVITYPE_FLOAT, IDC_RADIO_NAVITYPE_FLOAT);
				break;
			}
		}
		return FALSE;
	}
	return TRUE;
}

void MakeTrayMenu(HWND hWnd)
{
	HMENU hMenu, hSubMenu;
	POINT pt;

	PopupFlag = TRUE;
	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_TRAYMENU));
	hSubMenu = GetSubMenu(hMenu, 0);
	if(hDlgOption)
	{
		EnableMenuItem(hMenu, IDM_IMPORTSETTING, MF_BYCOMMAND|MF_GRAYED);
		EnableMenuItem(hMenu, IDM_EXPORTSETTING, MF_BYCOMMAND|MF_GRAYED);
	}
	if(NaviType==NaviTypeNone)
		CheckMenuRadioItem(hMenu, IDM_SHOWMAUSUJINAVI, IDM_TIPMAUSUJINAVI, IDM_HIDEMAUSUJINAVI, MF_BYCOMMAND);
	else if(NaviType==NaviTypeFixed)
		CheckMenuRadioItem(hMenu, IDM_SHOWMAUSUJINAVI, IDM_TIPMAUSUJINAVI, IDM_SHOWMAUSUJINAVI, MF_BYCOMMAND);
	else if(NaviType==NaviTypeFloat)
		CheckMenuRadioItem(hMenu, IDM_SHOWMAUSUJINAVI, IDM_TIPMAUSUJINAVI, IDM_TIPMAUSUJINAVI, MF_BYCOMMAND);
	if(MouseHookFlag == TRUE)
		CheckMenuItem( hMenu, IDM_MOUSEHOOK, MF_BYCOMMAND | MF_CHECKED );
	else
		EnableMenuItem( hMenu, IDM_WHEELREDIRECT, MF_BYCOMMAND | MF_GRAYED );
	if(WheelRedirect)
		CheckMenuItem( hMenu, IDM_WHEELREDIRECT, MF_BYCOMMAND | MF_CHECKED );
	GetCursorPos(&pt);
	_SetForegroundWindow(hWnd);
	TrackPopupMenuEx(hSubMenu, TPM_RIGHTBUTTON|TPM_BOTTOMALIGN, pt.x, pt.y, hWnd, NULL);
	DestroyMenu(hMenu);
	PostMessage(hWnd, WM_NULL, 0, 0);
	PopupFlag = FALSE;
	SetTimer(hWnd, ID_NAVITIMER, NaviDeleteTime, NULL);
}

BOOL CheckButtonUp(int ButtonCode)
{
	BOOL ButtonUpFlag = FALSE;

	switch(ButtonCode)
	{
	case BUTTON_L:
		if(!(GetSystemMetrics(SM_SWAPBUTTON)) && !(GetKeyState( VK_LBUTTON ) & 0x8000) ||
			(GetSystemMetrics(SM_SWAPBUTTON) && !(GetKeyState( VK_RBUTTON ) & 0x8000)))
			ButtonUpFlag = TRUE;
		break;
	case BUTTON_M:
		if(!(GetKeyState( VK_MBUTTON ) & 0x8000))
			ButtonUpFlag = TRUE;
		break;
	case BUTTON_R:
		if(!(GetSystemMetrics(SM_SWAPBUTTON)) && !(GetKeyState( VK_RBUTTON ) & 0x8000) ||
			(GetSystemMetrics(SM_SWAPBUTTON) && !(GetKeyState( VK_LBUTTON ) & 0x8000)))
			ButtonUpFlag = TRUE;
		break;
	case BUTTON_X1:
		if(!(GetKeyState( VK_XBUTTON1 ) & 0x8000))
			ButtonUpFlag = TRUE;
		break;
	case BUTTON_X2:
		if(!(GetKeyState( VK_XBUTTON2 ) & 0x8000))
			ButtonUpFlag = TRUE;
		break;
	}
	return ButtonUpFlag;
}

void DoubleClickProc(HWND hwnd, UINT id)
{
	DoubleClickFlag = -1;
	KillTimer(hwnd, ID_DBLCLKTIMER);
}

void HookCheckProc(HWND hwnd, UINT id)
{
	static POINT LastPos = {0,0};
	POINT TempPos;

	if(MouseHookFlag == TRUE && ImplementFlag == FALSE)
	{
		GetCursorPos(&TempPos);
		if((TempPos.x != LastPos.x || TempPos.y != LastPos.y) && HookCheckFlag == FALSE)
		{
			MouseUnHook();
			SetMouseHook(hwnd);
		}
		LastPos = TempPos;
		HookCheckFlag = FALSE;
	}
}

void KuruKuruTimerProc(HWND hwnd, UINT id)
{
	KuruKuruTimeoutFlag = TRUE;
	SetKuruKuruFlag(FALSE);
	KillTimer(hwnd, ID_KURUKURUTIMER);
}

void KuruKuruScroll(HWND hWnd, int CursorPosx, int CursorPosy)
{
	static POINT LastPos;
	static int LastScrollCourse = 0;
	static int StartCourse = -1;
	static int LastMove = -1;
	static int PosDiffx = 0, PosDiffy = 0;
	static BOOL ScrollFlag = FALSE;
	static int RealScrollSensitivity = 0;	
	static int MoveValuex[4] = {0,0,0,0};
	static int MoveValuey[4] = {0,0,0,0};
	int TotalMoveValuex, TotalMoveValuey, DiffMoveValuex, DiffMoveValuey;
	int ScrollCount;
	int RealMove = -1;
	static int LastPosx = 0, LastPosy = 0;
	DOUBLE TempCircleRate = CircleRate;

	if(KuruKuruTimeoutFlag == TRUE)
	{
		KuruKuruTimeoutFlag = FALSE;
		StartCourse = 0;
		ScrollFlag = FALSE;
		LastMove = -1;
		RealScrollSensitivity = 0;
		MoveValuex[S_MOVE_UP] = 0;
		MoveValuex[S_MOVE_RIGHT] = 0;
		MoveValuex[S_MOVE_DOWN] = 0;
		MoveValuex[S_MOVE_LEFT] = 0;
		MoveValuey[S_MOVE_UP] = 0;
		MoveValuey[S_MOVE_RIGHT] = 0;
		MoveValuey[S_MOVE_DOWN] = 0;
		MoveValuey[S_MOVE_LEFT] = 0;
		PosDiffx = 0;
		PosDiffy = 0;
		GetCursorPos(&LastPos);
	}
	if(LastPosx == CursorPosx && LastPosy == CursorPosy)
		return;
	LastPosx = CursorPosx;
	LastPosy = CursorPosy;
	PosDiffx += CursorPosx - LastPos.x;
	PosDiffy += CursorPosy - LastPos.y;
	if((PosDiffx * PosDiffx) + (PosDiffy * PosDiffy) > ((MoveQuantity < 5 ? MoveQuantity : 5) * (MoveQuantity < 5 ? MoveQuantity : 5))) 
	{
		if(abs(PosDiffx) < abs(PosDiffy))
		{
			if(PosDiffy < 0)
				RealMove = S_MOVE_UP;
			else
				RealMove = S_MOVE_DOWN;
		}
		else
		{
			if(PosDiffx < 0)
				RealMove = S_MOVE_LEFT;
			else
				RealMove = S_MOVE_RIGHT;
		}
		MoveValuex[RealMove] += PosDiffx;
		MoveValuey[RealMove] += PosDiffy;
		if(LastMove == -1)
		{
			StartCourse = RealMove;
			LastMove = RealMove;
		}
		else if(LastMove != RealMove)
		{
			if(RealMove == (LastMove + 1) % 4)//右回り
			{
				if(ScrollFlag == FALSE)
				{
					if(LastScrollCourse == -1)//前回と同じ回転方向であれば
					{
						if(StartCourse != -1 && (StartCourse + 1) % 4 == RealMove)
						{
							TotalMoveValuex = abs(MoveValuex[0]) + abs(MoveValuex[1]) + abs(MoveValuex[2]) + abs(MoveValuex[3]);
							TotalMoveValuey = abs(MoveValuey[0]) + abs(MoveValuey[1]) + abs(MoveValuey[2]) + abs(MoveValuey[3]);
							DiffMoveValuex = abs(MoveValuex[0] + MoveValuex[1] + MoveValuex[2] + MoveValuex[3]);
							DiffMoveValuey = abs(MoveValuey[0] + MoveValuey[1] + MoveValuey[2] + MoveValuey[3]);
							if(TotalMoveValuex <= TotalMoveValuey * (2 - (TempCircleRate / 100))
								&& TotalMoveValuex >= TotalMoveValuey * (TempCircleRate / 100)
								&& DiffMoveValuex <= TotalMoveValuex * (1 - (TempCircleRate / 100))
								&& DiffMoveValuey <= TotalMoveValuey * (1 - (TempCircleRate / 100)))
							{
								ScrollFlag = TRUE;
								GetCursorPos(&LastPos);
								SetKuruKuruFlag(TRUE);
							}
							else
							{
								MoveValuex[RealMove] = PosDiffx;
								MoveValuey[RealMove] = PosDiffy;
								StartCourse = RealMove;
							}
						}
					}
					else
					{
						LastScrollCourse = -1;
						StartCourse = LastMove;
						RealScrollSensitivity = 0;
					}
				}
				else
				{
					LastScrollCourse = -1;
				}
			}
			else if((RealMove + 1) % 4 == LastMove)//左回り
			{
				if(ScrollFlag == FALSE)
				{
					if(LastScrollCourse == 1)
					{
						if(StartCourse != -1 && StartCourse == (RealMove + 1) % 4)
						{
							TotalMoveValuex = abs(MoveValuex[0]) + abs(MoveValuex[1]) + abs(MoveValuex[2]) + abs(MoveValuex[3]);
							TotalMoveValuey = abs(MoveValuey[0]) + abs(MoveValuey[1]) + abs(MoveValuey[2]) + abs(MoveValuey[3]);
							DiffMoveValuex = abs(MoveValuex[0] + MoveValuex[1] + MoveValuex[2] + MoveValuex[3]);
							DiffMoveValuey = abs(MoveValuey[0] + MoveValuey[1] + MoveValuey[2] + MoveValuey[3]);
							if(TotalMoveValuex <= TotalMoveValuey * (2 - (TempCircleRate / 100))
								&& TotalMoveValuex >= TotalMoveValuey * (TempCircleRate / 100)
								&& DiffMoveValuex <= TotalMoveValuex * (1 - (TempCircleRate / 100))
								&& DiffMoveValuey <= TotalMoveValuey * (1 - (TempCircleRate / 100)))
							{
								ScrollFlag = TRUE;
								GetCursorPos(&LastPos);
								SetKuruKuruFlag(TRUE);
							}
							else
							{
								MoveValuex[RealMove] = PosDiffx;
								MoveValuey[RealMove] = PosDiffy;
								StartCourse = RealMove;
							}
						}
					}
					else
					{
						LastScrollCourse = 1;
						StartCourse = LastMove;
						RealScrollSensitivity = 0;
					}
				}
				else
				{
					LastScrollCourse = 1;
				}
			}
			LastMove = RealMove;
		}
		if(ScrollFlag == TRUE)
		{
			RealScrollSensitivity += abs(PosDiffy);
			RealScrollSensitivity += abs(PosDiffx);
			if(RealScrollSensitivity >= ScrollSensitivity)
			{
				ScrollCount = RealScrollSensitivity / ScrollSensitivity;
				RealScrollSensitivity -= ScrollSensitivity * ScrollCount;
				PostMessage(WindowFromPoint(LastPos), WM_MOUSEWHEEL, MAKEWPARAM(0, 120 * ScrollCount * LastScrollCourse), MAKELPARAM(CursorPosx, CursorPosy));
			}
		}
		else
		{
			LastPos.x = CursorPosx;
			LastPos.y = CursorPosy;
		}
		PosDiffx = 0;
		PosDiffy = 0;
	}
	SetTimer(hWnd, ID_KURUKURUTIMER, KuruKuruTimeOut, NULL);
}

void FreeScroll(int CursorPosx, int CursorPosy, BOOL ScrollInitFlag, HWND hWnd)
{
	static HWND hwTarget;
	static POINT LastPos;
	static int MoveValuex;
	static int MoveValuey;
	int ScrollCount;

	if(ScrollInitFlag == TRUE)
	{
		MoveValuex = 0;
		MoveValuey = 0;
		GetCursorPos(&LastPos);
		hwTarget = hWnd;
		return;
	}
	MoveValuex += CursorPosx - LastPos.x;
	MoveValuey += LastPos.y - CursorPosy;
	if(abs(MoveValuex) >= ScrollSensitivity)
	{
		ScrollCount = MoveValuex / ScrollSensitivity;
		MoveValuex -= ScrollSensitivity * ScrollCount;
		PostMessage(hwTarget, WM_MOUSEHWHEEL, MAKEWPARAM(0, 120 * ScrollCount), MAKELPARAM(CursorPosx, CursorPosy));
	}
	if(abs(MoveValuey) >= ScrollSensitivity)
	{
		ScrollCount = MoveValuey / ScrollSensitivity;
		MoveValuey -= ScrollSensitivity * ScrollCount;
		PostMessage(hwTarget, WM_MOUSEWHEEL, MAKEWPARAM(0, 120 * ScrollCount), MAKELPARAM(CursorPosx, CursorPosy));
	}
}

void TimeOutProc(HWND hwnd, UINT id)
{
	BOOL GestureTimeOutFlag = FALSE;
	UINT DownButton, UpButton, XButton = 0x0000;

	if(TimeOutFlag == FALSE)
		TimeOutFlag = TRUE;
	if(GestureTimeOutTimer == TRUE)
	{
		GestureTimeOutTimer = FALSE;
		GestureTimeOutFlag = TRUE;
	}
	KillTimer(hwnd, ID_TIMEOUTTIMER);
	if((Gesture.Level <= 0 && Gesture.Start == FALSE) ||
		(Gesture.Level <= MAX_GESTURE_LEVEL && GestureTimeOutFlag == TRUE))
	{
		GetGestureString(&Gesture.Modifier, &Gesture.Button, Gesture.Move, GestureString, 50);
		lstrcpy(CommentString, TEXT("Time-out"));
		Gesture.Level = MAX_GESTURE_LEVEL + 2;
		SetButtonFlag(FALSE);
		SetHookFlag(FALSE);
		SetCursorNumber(0);
		switch(Gesture.Button)
		{
		case BUTTON_L:
			DownButton = MOUSEEVENTF_LEFTDOWN;
			UpButton = MOUSEEVENTF_LEFTUP;
			break;
		case BUTTON_M:
			DownButton = MOUSEEVENTF_MIDDLEDOWN;
			UpButton = MOUSEEVENTF_MIDDLEUP;
			break;
		case BUTTON_R:
			DownButton = MOUSEEVENTF_RIGHTDOWN;
			UpButton = MOUSEEVENTF_RIGHTUP;
			break;
		case BUTTON_X1:
			DownButton = MOUSEEVENTF_XDOWN;
			UpButton = MOUSEEVENTF_XUP;
			XButton = XBUTTON1;
			break;
		case BUTTON_X2:
			DownButton = MOUSEEVENTF_XDOWN;
			UpButton = MOUSEEVENTF_XUP;
			XButton = XBUTTON2;
			break;
		default:
			return;
		}
		SetCursorPos(StartPos.x, StartPos.y);
		if(Gesture.Button == Gesture.Move[0] && DoubleClickFlag == -1)
		{
			mouse_event(DownButton,0,0,XButton,0);
			mouse_event(UpButton,0,0,XButton,0);
			Sleep(ClickWait * 2);
		}
		mouse_event(DownButton,0,0,XButton,0);
		Sleep(ClickWait);
		SetCursorPos(RealPos.x, RealPos.y);
		if(CheckButtonUp(Gesture.Button))
		{
			mouse_event(UpButton,0,0,XButton,0);
			Sleep(ClickWait);
		}
		NaviRefresh(hwnd);
	}
}

void ApplyConfig(HWND hwnd)
{
	SetCursorHandle(GestureCursor, WaitCursor, CursorChange);
	SetWheelRedirectFlag(WheelRedirect);
	SetWheelActiveFlag(WheelActive);
	SetPriorityClass(GetCurrentProcess(), Priority);
	if(ShowTaskTray)
		MyTaskTray(hwnd);
	else
		MyTaskTray(NULL);
	if(MouseGestureTrailDrawMode==1)
		SetMouseGestureTrailMode(0);
	else if(MouseGestureTrailDrawMode==2)
		SetMouseGestureTrailMode(1);
	else
		SetMouseGestureTrailModeAuto();

	if(hFont)
		DeleteObject(hFont);
	hFont = CreateFontIndirect(&lf);
	if(NaviType != NaviTypeNone)
	{
		ShowMauSujiNavi(hwnd, TRUE);
	}
	lstrcpy(GestureString, NaviTitle);
	NaviRefresh(hwnd);
	if(NaviType != NaviTypeNone)
	{
		ShowMauSujiNavi(hwnd, TRUE);
		SetTimer(hwnd, ID_NAVITIMER, NaviDeleteTime, NULL);
	}
}

void MouseGestureStringUpdate()
{
	GetGestureString(&Gesture.Modifier, &Gesture.Button, Gesture.Move, GestureString, 50);
}

void MouseGestureInit()
{
	int i;
	POINT apos;

	GetCursorPos(&apos);

	//Gestureを初期化
	Gesture.Level = 0;
	Gesture.Modifier = 0;
	if(GetKeyState( VK_SHIFT ) & 0x8000)
		Gesture.Modifier |= MODIFIER_SHIFT;
	if(GetKeyState( VK_CONTROL ) & 0x8000)
		Gesture.Modifier |= MODIFIER_CONTROL;
	for(i=0; i<MAX_GESTURE_LEVEL; i++)
		Gesture.Move[i] = 0;
	Gesture.Start = FALSE;
	//ジェスチャー文字列
	GetGestureString(&Gesture.Modifier, &Gesture.Button, NULL, GestureString, 50);
	//グローバル変数初期化
	StartPos = apos;
	RealPos = apos;
	DefPos = apos;
	LastPos = apos;
	//軌跡
	if(MouseGestureTrail)
	{
		MouseGestureTrailStart();
		MouseGestureTrailAddPos(apos);
	}
}

void MouseGestureTerm()
{
	if(MouseGestureTrail)
	{
		MouseGestureTrailEnd();
	}
}

void MouseGestureOnMouseMove()
{
	POINT apos;
	int PosDiffx, PosDiffy;

	GetCursorPos(&apos);

	PosDiffx = apos.x - LastPos.x;
	PosDiffy = apos.y - LastPos.y;

	if((PosDiffx * PosDiffx) + (PosDiffy * PosDiffy) >= ((MoveQuantity < 5 ? MoveQuantity : 5) * (MoveQuantity < 5 ? MoveQuantity : 5)))
	{
		if(Gesture.Level > 0 && Gesture.Move[Gesture.Level - 1] == GetGestureMove(PosDiffx, PosDiffy))
		{
			DefPos = apos;
		}
		LastPos = apos;
		//軌跡
		if(MouseGestureTrail)
		{
			MouseGestureTrailAddPos(LastPos);
			DrawTrail();
		}
	}
}

void MouseGesture(int FoundTargetNumber)
{
	POINT apos;

	GetCursorPos(&apos);

	if(Gesture.Level < MAX_GESTURE_LEVEL)
	{
		int PosDiffx, PosDiffy;
		int FoundActionNumber;

		PosDiffx = apos.x - DefPos.x;
		PosDiffy = apos.y - DefPos.y;

		if(Gesture.Start == FALSE && Gesture.Level == 0 &&
			((PosDiffx * PosDiffx) + (PosDiffy * PosDiffy) >= (GestureStartQuantity * GestureStartQuantity)))
		{
			Gesture.Start = TRUE;
			if(TimeOutType)
				GestureTimeOutTimer = TRUE;
			SetTarget();
			SetCursorNumber(2);
		}

		if(((PosDiffx * PosDiffx) + (PosDiffy * PosDiffy) >= (MoveQuantity * MoveQuantity)) &&
			Gesture.Button != Gesture.Move[0])
		{
			//ジェスチャーの方向を取得
			Gesture.Move[Gesture.Level] = GetGestureMove(PosDiffx, PosDiffy);
			if(Gesture.Level == 0 || Gesture.Move[Gesture.Level] != Gesture.Move[Gesture.Level - 1])
			{
				//ジェスチャー文字列
				GetGestureString(&Gesture.Modifier, &Gesture.Button, Gesture.Move, GestureString, 50);
				//ジェスチャーを入力した
				Gesture.Level ++;
				//コメント文字列
				FoundActionNumber = SearchAction(FoundTargetNumber, 1);
				if(FoundActionNumber >= 0)
					lstrcpy( CommentString, Action[FoundActionNumber].Comment);
				else
					lstrcpy( CommentString, TEXT(""));
			}
			else
			{
				Gesture.Move[Gesture.Level] = 0;	//0に戻す
			}
		}
	}
}

LRESULT OnMauHook(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	static int FoundTargetNumber, FoundActionNumber, LastClick = 0;
	static POINT FirstClickPos;
	static int ScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN) - 1;
	static int ScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN) - 1;
	POINT CursorPos;
	int i, CursorPosition;
	UINT UpButtonMessage, DownButton, UpButton, XButton = 0x0000;
	int TempGestureLevel;

	if (PopupFlag == TRUE || ImplementFlag == TRUE)
		return 0;

	TempGestureLevel = Gesture.Level;
	HookCheckFlag = TRUE;

	switch(Gesture.Button)
	{
	case CORNER_TOP_A:
	case CORNER_TOP_B:
	case CORNER_TOP_C:
	case CORNER_BOTTOM_A:
	case CORNER_BOTTOM_B:
	case CORNER_BOTTOM_C:
	case CORNER_LEFT_A:
	case CORNER_LEFT_B:
	case CORNER_LEFT_C:
	case CORNER_RIGHT_A:
	case CORNER_RIGHT_B:
	case CORNER_RIGHT_C:
	case CORNER_TOPLEFT:
	case CORNER_TOPRIGHT:
	case CORNER_BOTTOMLEFT:
	case CORNER_BOTTOMRIGHT:
	case 0: //何もボタンが有効でないときにボタンが押された場合の処理
		GetCursorPos(&CursorPos);
		switch(wParam)
		{
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_XBUTTON1DOWN:
		case WM_XBUTTON2DOWN:
			switch(wParam)
			{
			case WM_LBUTTONDOWN:
			case WM_NCLBUTTONDOWN:
				if(!(GetSystemMetrics(SM_SWAPBUTTON)))
					Gesture.Button = BUTTON_L;
				else
					Gesture.Button = BUTTON_R;
				break;
			case WM_MBUTTONDOWN:
			case WM_NCMBUTTONDOWN:
				Gesture.Button = BUTTON_M;
				break;
			case WM_RBUTTONDOWN:
			case WM_NCRBUTTONDOWN:
				if(!(GetSystemMetrics(SM_SWAPBUTTON)))
					Gesture.Button = BUTTON_R;
				else
					Gesture.Button = BUTTON_L;
				break;
			case WM_XBUTTON1DOWN:
				Gesture.Button = BUTTON_X1;
				break;
			case WM_XBUTTON2DOWN:
				Gesture.Button = BUTTON_X2;
				break;
			}

			MouseGestureInit();

			if(DoubleClickFlag == Gesture.Button &&
				abs(FirstClickPos.x - CursorPos.x) <= GetSystemMetrics(SM_CXDOUBLECLK) &&
				abs(FirstClickPos.y - CursorPos.y) <= GetSystemMetrics(SM_CYDOUBLECLK))
			{
				//ダブルクリック
				if(LastClick == Gesture.Button)
					DoubleClickFlag = 0;
				else
					DoubleClickFlag = -1;

				Gesture.Move[0] = Gesture.Button;	//ダブルクリック
				MouseGestureStringUpdate();
			}
			else
			{
				//ダブルクリック判定用タイマー
				DoubleClickFlag = Gesture.Button;
				FirstClickPos = CursorPos;
				SetTimer(hwnd, ID_DBLCLKTIMER, GetDoubleClickTime(), NULL);
			}
			break;
		case WM_LBUTTONDBLCLK:
		case WM_NCLBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_NCMBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_NCRBUTTONDBLCLK:
		case WM_XBUTTON1DBLCLK:
		case WM_XBUTTON2DBLCLK:
			switch(wParam)
			{
			case WM_LBUTTONDBLCLK:
			case WM_NCLBUTTONDBLCLK:
				if(!(GetSystemMetrics(SM_SWAPBUTTON)))
					Gesture.Button = BUTTON_L;
				else
					Gesture.Button = BUTTON_R;
				break;
			case WM_MBUTTONDBLCLK:
			case WM_NCMBUTTONDBLCLK:
				Gesture.Button = BUTTON_M;
				break;
			case WM_RBUTTONDBLCLK:
			case WM_NCRBUTTONDBLCLK:
				if(!(GetSystemMetrics(SM_SWAPBUTTON)))
					Gesture.Button = BUTTON_R;
				else
					Gesture.Button = BUTTON_L;
				break;
			case WM_XBUTTON1DBLCLK:
				Gesture.Button = BUTTON_X1;
				break;
			case WM_XBUTTON2DBLCLK:
				Gesture.Button = BUTTON_X2;
				break;
			}

			MouseGestureInit();

			if(DoubleClickFlag == Gesture.Button)
			{
				//ダブルクリック判定用タイマーを停止
				KillTimer(hwnd, ID_DBLCLKTIMER);
				DoubleClickFlag = -1;
			}

			Gesture.Move[0] = Gesture.Button;	//ダブルクリック
			MouseGestureStringUpdate();
			break;
		case WM_MOUSEWHEEL_UP:
		case WM_MOUSEWHEEL_DOWN:
		case WM_MOUSEWHEEL_LEFT:
		case WM_MOUSEWHEEL_RIGHT:
			if(wParam == WM_MOUSEWHEEL_UP)
				Gesture.Button = WHEEL_UP;
			else if(wParam == WM_MOUSEWHEEL_DOWN)
				Gesture.Button = WHEEL_DOWN;
			else if(wParam == WM_MOUSEWHEEL_LEFT)
				Gesture.Button = WHEEL_LEFT;
			else if(wParam == WM_MOUSEWHEEL_RIGHT)
				Gesture.Button = WHEEL_RIGHT;

			MouseGestureInit();

			if(Gesture.Button != 0)
				KillTimer(hwnd, ID_TIMER);
			lstrcpy(CommentString, TEXT(""));

			FoundTargetNumber = SearchTarget((HWND)lParam, FALSE);
			FoundActionNumber = SearchAction(FoundTargetNumber, 0);
			if(FoundActionNumber >= 0)
			{
				if((FoundTargetNumber < (int)Target.size() && Target[FoundTargetNumber].HookType) ||
					(FoundTargetNumber == (int)Target.size() && Default.HookType))
				{
					SetButtonFlag(TRUE);
				}
			}
			PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			return TRUE;
		default:
			//くるくるスクロール
			if(KuruKuruFlag)
				KuruKuruScroll(hwnd, CursorPos.x, CursorPos.y);
			//コーナーマウスを判定
			if(CursorPos.x == 0 || CursorPos.x == ScreenWidth || CursorPos.y == 0 || CursorPos.y == ScreenHeight)
			{
				if(CursorPos.x <= CornerPosX && CursorPos.y <= CornerPosY)
				{
					CursorPosition = CORNER_TOPLEFT;
				}
				else if(CursorPos.x <= CornerPosX && CursorPos.y >= ScreenHeight - CornerPosY)
				{
					CursorPosition = CORNER_BOTTOMLEFT;
				}
				else if(CursorPos.x >= ScreenWidth - CornerPosX && CursorPos.y <= CornerPosY)
				{
					CursorPosition = CORNER_TOPRIGHT;
				}
				else if(CursorPos.x >= ScreenWidth - CornerPosX && CursorPos.y >= ScreenHeight - CornerPosY)
				{
					CursorPosition = CORNER_BOTTOMRIGHT;
				}
				else if(CursorPos.y <= 0)
				{
					if(CursorPos.x <= (ScreenWidth - (CornerPosX * 2)) / 3 + CornerPosX)
						CursorPosition = CORNER_TOP_A;
					else if(CursorPos.x >= ScreenWidth - ((ScreenWidth - (CornerPosX * 2)) / 3 + CornerPosX))
						CursorPosition = CORNER_TOP_C;
					else
						CursorPosition = CORNER_TOP_B;
				}
				else if(CursorPos.y >= ScreenHeight)
				{
					if(CursorPos.x <= (ScreenWidth - (CornerPosX * 2)) / 3 + CornerPosX)
						CursorPosition = CORNER_BOTTOM_A;
					else if(CursorPos.x >= ScreenWidth - ((ScreenWidth - (CornerPosX * 2)) / 3 + CornerPosX))
						CursorPosition = CORNER_BOTTOM_C;
					else
						CursorPosition = CORNER_BOTTOM_B;
				}
				else if(CursorPos.x <= 0)
				{
					if(CursorPos.y <= (ScreenHeight - (CornerPosY * 2)) / 3 + CornerPosY)
						CursorPosition = CORNER_LEFT_A;
					else if(CursorPos.y >= ScreenHeight - ((ScreenHeight - (CornerPosY * 2)) / 3 + CornerPosY))
						CursorPosition = CORNER_LEFT_C;
					else
						CursorPosition = CORNER_LEFT_B;
				}
				else if(CursorPos.x >= ScreenWidth)
				{
					if(CursorPos.y <= (ScreenHeight - (CornerPosY * 2)) / 3 + CornerPosY)
						CursorPosition = CORNER_RIGHT_A;
					else if(CursorPos.y >= ScreenHeight - ((ScreenHeight - (CornerPosY * 2)) / 3 + CornerPosY))
						CursorPosition = CORNER_RIGHT_C;
					else
						CursorPosition = CORNER_RIGHT_B;
				}
				else
				{
					CursorPosition = 0;	//?
				}
			}
			else
			{
				CursorPosition = 0;	//マウスカーソルが画面端にない
			}

			if((Gesture.Level > -2 && Gesture.Button != CursorPosition) ||
				(Gesture.Level == -2 && CursorPosition == 0))
			{
				Gesture.Button = CursorPosition;
				Gesture.Level = -1;
				if(Gesture.Button == 0)
				{
					if(FinishActionNumber > -1)
					{
						Gesture.Level = MAX_GESTURE_LEVEL + 1;
						Gesture.Start = FALSE;
						Gesture.Button = CORNER_FINISH;
					}
					else
					{
						KillTimer(hwnd, ID_TIMER);
						SetCursorNumber(0);
						if(NaviDrawFlag == TRUE && NaviDeleteTime > 0 && FoundTargetNumber >= 0)
						{
							if(NaviType == NaviTypeNone)
							{
							}
							else
							{
								NaviDrawFlag = FALSE;
								SetTimer(hwnd, ID_NAVITIMER, NaviDeleteTime, NULL);
							}
						}
					}
				}
				else
				{
					lstrcpy(CommentString, TEXT(""));

					FoundTargetNumber = SearchTarget(GetForegroundWindow(), TRUE);
					MouseGestureInit();
					FoundActionNumber = SearchAction(FoundTargetNumber, 0);
					if(FoundActionNumber >= 0)
					{
						//コーナーマウス用タイマー
						lstrcpy(CommentString, Action[FoundActionNumber].Comment);
						NaviRefresh(hwnd);
						SetCursorNumber(1);
						SetTimer(hwnd, ID_TIMER, CornerTime, NULL);
					}
					else
					{
						KillTimer(hwnd, ID_TIMER);
						Gesture.Level = -1;
						if(NaviDrawFlag == TRUE && NaviDeleteTime > 0 && FoundTargetNumber >= 0)
						{
							if(NaviType == NaviTypeNone)
							{
							}
							else
							{
								NaviDrawFlag = FALSE;
								SetTimer(hwnd, ID_NAVITIMER, NaviDeleteTime, NULL);
							}
						}
					}
				}
			}
			else
			{
				if(Gesture.Level == MAX_GESTURE_LEVEL)
				{
					Gesture.Level = MAX_GESTURE_LEVEL + 1;
					Gesture.Start = TRUE;
					SetCursorNumber(0);
				}
			}
			break;
		}

		if(Gesture.Button == BUTTON_L || Gesture.Button == BUTTON_M || Gesture.Button == BUTTON_R ||
			Gesture.Button == BUTTON_X1 || Gesture.Button == BUTTON_X2)
		{
			if(Gesture.Button != 0)
				KillTimer(hwnd, ID_TIMER);

			lstrcpy(CommentString, TEXT(""));
			LastClick = 0;
			TimeOutFlag = TRUE;

			FoundTargetNumber = SearchTarget((HWND)lParam, FALSE);
			FoundActionNumber = SearchAction(FoundTargetNumber, 0);
			if(FoundActionNumber >= 0)
			{
				int Number;

				Number = SearchAction(FoundTargetNumber, 1);
				if(Number >= 0)
					lstrcpy(CommentString, Action[Number].Comment);

				if((FoundTargetNumber < (int)Target.size() && !Target[FoundTargetNumber].HookType) ||
					(FoundTargetNumber == (int)Target.size() && !Default.HookType))
				{
					Gesture.Start = TRUE;
				}
				else
				{
					SetButtonFlag(TRUE);
					SetCursorNumber(1);
					TimeOutFlag = FALSE;
					SetTimer(hwnd, ID_TIMEOUTTIMER, GestureTimeOut, NULL);
				}

				if(SearchAction(FoundTargetNumber, 2) == -1)
				{
					ClickOnlyFlag = TRUE;
					KillTimer(hwnd, ID_TIMEOUTTIMER);
					SetCursorNumber(2);
					TimeOutFlag = TRUE;
					Gesture.Start = TRUE;
					Gesture.Level = MAX_GESTURE_LEVEL + 1;
					SetTarget();
					NaviRefresh(hwnd);
				}
				else if(SearchAction(FoundTargetNumber, 1) >= 0)
				{
					NaviRefresh(hwnd);
				}
			}
			else
			{
				if(FoundTargetNumber < 0)
					lstrcpy(GestureString, NaviTitle);
			}
			SetTimer(hwnd, ID_TIMER, CheckInterval, NULL);
			return TRUE;
		}
		break;
	case BUTTON_L:
		UpButtonMessage = WM_LBUTTONUP;
		DownButton = MOUSEEVENTF_LEFTDOWN;
		UpButton = MOUSEEVENTF_LEFTUP;
		break;
	case BUTTON_M:
		UpButtonMessage = WM_MBUTTONUP;
		DownButton = MOUSEEVENTF_MIDDLEDOWN;
		UpButton = MOUSEEVENTF_MIDDLEUP;
		break;
	case BUTTON_R:
		UpButtonMessage = WM_RBUTTONUP;
		DownButton = MOUSEEVENTF_RIGHTDOWN;
		UpButton = MOUSEEVENTF_RIGHTUP;
		break;
	case BUTTON_X1:
		UpButtonMessage = WM_XBUTTON1UP;
		DownButton = MOUSEEVENTF_XDOWN;
		UpButton = MOUSEEVENTF_XUP;
		XButton = XBUTTON1;
		break;
	case BUTTON_X2:
		UpButtonMessage = WM_XBUTTON2UP;
		DownButton = MOUSEEVENTF_XDOWN;
		UpButton = MOUSEEVENTF_XUP;
		XButton = XBUTTON2;
		break;
	case WHEEL_UP:
	case WHEEL_DOWN:
	case WHEEL_LEFT:
	case WHEEL_RIGHT:
		Gesture.Start = TRUE;
		if(FoundActionNumber >= 0)
			Gesture.Level = MAX_GESTURE_LEVEL + 1;
		else
			Gesture.Level = MAX_GESTURE_LEVEL + 2;
		break;
	}

	switch(Gesture.Button)
	{
	case BUTTON_L:
	case BUTTON_M:
	case BUTTON_R:
	case BUTTON_X1:
	case BUTTON_X2:
		if(wParam == UpButtonMessage)
		{
			KillTimer(hwnd, ID_TIMER);
			if(TimeOutFlag == FALSE)
				KillTimer(hwnd, ID_TIMEOUTTIMER);

			SetCursorNumber(0);

			if(Gesture.Move[0] == Gesture.Button || Gesture.Move[0] == 0)
			{
				if(TimeOutFlag == FALSE && Gesture.Start == FALSE)
				{
					if(SearchAction(FoundTargetNumber, 1) < 0)
					{
						SetHookFlag(FALSE);
						//シングルクリック
						mouse_event(DownButton, 0, 0, XButton, 0);
						mouse_event(UpButton, 0, 0, XButton, 0);
						Sleep(ClickWait * 2);
						if(Gesture.Move[0] == Gesture.Button && DoubleClickFlag == -1)
						{
							//ダブルクリック
							mouse_event(DownButton, 0, 0, XButton, 0);
							mouse_event(UpButton, 0, 0, XButton, 0);
							Sleep(ClickWait * 2);
						}
						LastClick = Gesture.Button;
					}
					else
					{
						SetTarget();
						Gesture.Start = TRUE;
					}
				}

				if(ClickOnlyFlag == TRUE)
					Gesture.Level = MAX_GESTURE_LEVEL + 2;
				else
					Gesture.Level = MAX_GESTURE_LEVEL + 1;
			}
			else if(Gesture.Move[0] == BUTTON_L || Gesture.Move[0] == BUTTON_M || Gesture.Move[0] == BUTTON_R ||
				Gesture.Move[0] == BUTTON_X1 || Gesture.Move[0] == BUTTON_X2 ||
				Gesture.Move[0] == WHEEL_UP || Gesture.Move[0] == WHEEL_DOWN || Gesture.Move[0] == WHEEL_LEFT || Gesture.Move[0] == WHEEL_RIGHT)
			{
				Gesture.Level = MAX_GESTURE_LEVEL + 2;
				Gesture.Move[0] = 0;
			}
			else
			{
				Gesture.Level = MAX_GESTURE_LEVEL + 1;
			}
		}
		break;
	}

	if((TimeOutFlag == FALSE || Gesture.Start == TRUE) && Gesture.Level <= MAX_GESTURE_LEVEL)
	{
		if(Gesture.Button == BUTTON_L || Gesture.Button == BUTTON_M || Gesture.Button == BUTTON_R ||
			Gesture.Button == BUTTON_X1 || Gesture.Button == BUTTON_X2)
		{
			switch(wParam)
			{
			case WM_MOUSEWHEEL:
				if(lParam > 0)
					Gesture.Move[0] = WHEEL_UP;
				else if(lParam < 0)
					Gesture.Move[0] = WHEEL_DOWN;
				break;
			case WM_MOUSEHWHEEL:
				if(lParam < 0)
					Gesture.Move[0] = WHEEL_LEFT;
				else if(lParam > 0)
					Gesture.Move[0] = WHEEL_RIGHT;
				break;
			case WM_LBUTTONDOWN:
			case WM_NCLBUTTONDOWN:
				if((!(GetSystemMetrics(SM_SWAPBUTTON)) && Gesture.Button == BUTTON_L) ||
					(GetSystemMetrics(SM_SWAPBUTTON) && Gesture.Button == BUTTON_R))
					break;
				if(!(GetSystemMetrics(SM_SWAPBUTTON)))
					Gesture.Move[0] = BUTTON_L;
				else
					Gesture.Move[0] = BUTTON_R;
				break;
			case WM_MBUTTONDOWN:
			case WM_NCMBUTTONDOWN:
				if(Gesture.Button == BUTTON_M)
					break;
				Gesture.Move[0] = BUTTON_M;
				break;
			case WM_RBUTTONDOWN:
			case WM_NCRBUTTONDOWN:
				if((!(GetSystemMetrics(SM_SWAPBUTTON)) && Gesture.Button == BUTTON_R) ||
					(GetSystemMetrics(SM_SWAPBUTTON) && Gesture.Button == BUTTON_L))
					break;
				if(!(GetSystemMetrics(SM_SWAPBUTTON)))
					Gesture.Move[0] = BUTTON_R;
				else
					Gesture.Move[0] = BUTTON_L;
				break;
			case WM_XBUTTON1DOWN:
				if(Gesture.Button == BUTTON_X1)
					break;
				Gesture.Move[0] = BUTTON_X1;
				break;
			case WM_XBUTTON2DOWN:
				if(Gesture.Button == BUTTON_X2)
					break;
				Gesture.Move[0] = BUTTON_X2;
				break;
			}

			switch(wParam)
			{
			case WM_MOUSEWHEEL:
			case WM_LBUTTONDOWN:
			case WM_NCLBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_NCMBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_NCRBUTTONDOWN:
			case WM_XBUTTON1DOWN:
			case WM_XBUTTON2DOWN:
				//ロッカージェスチャー（Gesture.Buttonを押しながらGesture.Move[0]を押した）
				if(Gesture.Move[0] == 0)
					break;
				if(TimeOutFlag == FALSE)
				{
					SetCursorNumber(2);
					TimeOutFlag = TRUE;
					SetTarget();
				}
				Gesture.Start = TRUE;
				for(i=1; i<MAX_GESTURE_LEVEL; i++)
					Gesture.Move[i] = 0;
				Gesture.Level = MAX_GESTURE_LEVEL + 1;
				MouseGestureStringUpdate();
				break;
			default:
				MouseGestureOnMouseMove();
				if(Gesture.Level < MAX_GESTURE_LEVEL)
					MouseGesture(FoundTargetNumber);
				if(TimeOutType)
				{
					GetCursorPos(&CursorPos);
					if(GestureTimeOutTimer == TRUE && (RealPos.x != CursorPos.x || RealPos.y != CursorPos.y))
					{
						SetTimer(hwnd, ID_TIMEOUTTIMER, GestureTimeOut, NULL);
						RealPos = CursorPos;
					}
				}
				break;
			}
		}
	}

	if((Gesture.Level > MAX_GESTURE_LEVEL && Gesture.Start == TRUE) ||
		(Gesture.Level == MAX_GESTURE_LEVEL + 1 && Gesture.Start == FALSE))
	{
		GestureTimeOutTimer = FALSE;
		KillTimer(hwnd, ID_TIMER);
		MouseGestureTerm();

		if(Gesture.Level == MAX_GESTURE_LEVEL + 1 && Gesture.Start == TRUE)
		{
			FoundActionNumber = SearchAction(FoundTargetNumber, 1);
			if(FoundActionNumber >= 0)
			{
				//登録されているアクションが実行された
				lstrcpy(CommentString, Action[FoundActionNumber].Comment);
				NaviRefresh(hwnd);
				GetCursorPos(&FinishPos);
				ImplementFlag = TRUE;
				SetHookFlag(FALSE);
				ImplementCommand(hwnd, FoundActionNumber, FALSE);	//コマンドを実行
				SetHookFlag(TRUE);
				ImplementFlag = FALSE;
			}
			else
			{
				//登録されていないアクションが実行された
				lstrcpy(CommentString, TEXT("no entry"));
				NaviRefresh(hwnd);
			}
		}

		if(Gesture.Button == Gesture.Move[0])	//ダブルクリック
			Gesture.Move[0] = 0;

		switch(Gesture.Move[0])
		{
		case BUTTON_L:
		case BUTTON_M:
		case BUTTON_R:
		case BUTTON_X1:
		case BUTTON_X2:
		case WHEEL_UP:
		case WHEEL_DOWN:
		case WHEEL_LEFT:
		case WHEEL_RIGHT:
			//ロッカージェスチャー（R Lなど）
			Gesture.Level = MAX_GESTURE_LEVEL;
			SetTimer(hwnd, ID_TIMER, CheckInterval, NULL);
			break;
		default:
			if(Gesture.Level == MAX_GESTURE_LEVEL + 1 && ClickOnlyFlag == TRUE)
			{
				Gesture.Level = MAX_GESTURE_LEVEL;
				SetTimer(hwnd, ID_TIMER, CheckInterval, NULL);
				break;
			}

			if(FinishActionNumber > -1 &&
				(Gesture.Button == BUTTON_L || Gesture.Button == BUTTON_M || Gesture.Button == BUTTON_R ||
				Gesture.Button == BUTTON_X1 || Gesture.Button == BUTTON_X2 ||
				Gesture.Button == WHEEL_UP || Gesture.Button == WHEEL_DOWN ||
				Gesture.Button == WHEEL_LEFT || Gesture.Button == WHEEL_RIGHT || Gesture.Button == CORNER_FINISH))
			{
				GetCursorPos(&FinishPos);
				ImplementFlag = TRUE;
				SetHookFlag(FALSE);
				ImplementCommand(hwnd, FinishActionNumber, TRUE);	//コマンドを実行
				SetHookFlag(TRUE);
				ImplementFlag = FALSE;
			}

			ResetButtonFlag();
			Gesture.Button = 0;
			Gesture.Level = -2;
			ClickOnlyFlag = FALSE;

			if(NaviDrawFlag == TRUE && NaviDeleteTime > 0 && FoundTargetNumber >= 0)
			{
				if(NaviType == NaviTypeNone)
				{
				}
				else
				{
					NaviDrawFlag = FALSE;
					SetTimer(hwnd, ID_NAVITIMER, NaviDeleteTime, NULL);
				}
			}
			break;
		}
		return TRUE;
	}

	if(Gesture.Button > 0 && TempGestureLevel != Gesture.Level && Gesture.Start == TRUE)
		NaviRefresh(hwnd);

	return 0;
}

void OnTimer(HWND hwnd, UINT id)
{
	switch(id)
	{
	case ID_TIMER:
		switch(Gesture.Button)
		{
		case BUTTON_L:
			if(CheckButtonUp(Gesture.Button))
				PostMessage(hwnd, MAUHOOK_MSG, WM_LBUTTONUP, 0);
			else
				PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			break;
		case BUTTON_M:
			if(CheckButtonUp(Gesture.Button))
				PostMessage(hwnd, MAUHOOK_MSG, WM_MBUTTONUP, 0);
			else
				PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			break;
		case BUTTON_R:
			if(CheckButtonUp(Gesture.Button))
				PostMessage(hwnd, MAUHOOK_MSG, WM_RBUTTONUP, 0);
			else
				PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			break;
		case BUTTON_X1:
			if(CheckButtonUp(Gesture.Button))
				PostMessage(hwnd, MAUHOOK_MSG, WM_XBUTTON1UP, 0);
			else
				PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			break;
		case BUTTON_X2:
			if(CheckButtonUp(Gesture.Button))
				PostMessage(hwnd, MAUHOOK_MSG, WM_XBUTTON2UP, 0);
			else
				PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			break;
		case CORNER_TOP_A:
		case CORNER_TOP_B:
		case CORNER_TOP_C:
		case CORNER_BOTTOM_A:
		case CORNER_BOTTOM_B:
		case CORNER_BOTTOM_C:
		case CORNER_LEFT_A:
		case CORNER_LEFT_B:
		case CORNER_LEFT_C:
		case CORNER_RIGHT_A:
		case CORNER_RIGHT_B:
		case CORNER_RIGHT_C:
		case CORNER_TOPLEFT:
		case CORNER_TOPRIGHT:
		case CORNER_BOTTOMLEFT:
		case CORNER_BOTTOMRIGHT:
			//コーナーマウスのコマンドを実行する
			Gesture.Level = MAX_GESTURE_LEVEL;
			PostMessage(hwnd, MAUHOOK_MSG, 0, 0);
			break;
		}
		break;
	case ID_NAVITIMER:
		NaviDeleteProc(hwnd, id);
		break;
	case ID_TIMEOUTTIMER:
		TimeOutProc(hwnd, id);
		break;
	case ID_DBLCLKTIMER:
		DoubleClickProc(hwnd, id);
		break;
	case ID_KURUKURUTIMER:
		KuruKuruTimerProc(hwnd, id);
		break;
	case ID_HOOKCHECKTIMER:
		HookCheckProc(hwnd, id);
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static UINT WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	static BOOL IniEditFlag = FALSE;
	static CONFIG configTemp;
	int RedirectTarget;
	CONFIG config;

	switch( msg )
	{
	case WM_CREATE:
		hwndNavi = CreateWindowEx(
			WS_EX_TOPMOST|WS_EX_TOOLWINDOW,
			TEXT("MauSujiNavi"),
			NULL,
			WS_POPUP,
			0, 0, 0, 0,
			NULL, NULL, hInstance, hwnd);

		LoadConfig(&config);
		_SetConfig(&config);
		SetWindowPos(hwndNavi, NULL, MauSujiNavi.x, MauSujiNavi.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		ApplyConfig(hwnd);

		Gesture.Button = 0;
		Gesture.Level = -1;
		SetMouseHook(hwnd); //マウスフックを開始する
		SetTimer(hwnd, ID_HOOKCHECKTIMER, 5000, NULL);
		break;
	case WM_DESTROY:
		if(IniEditFlag)
		{
			SaveConfig(&configTemp);
		}
		else
		{
			if(NaviType == NaviTypeFixed)
				GetWindowPosition(hwndNavi, &MauSujiNavi);
			_GetConfig(&config);
			SaveConfig(&config);
		}

		if(hFont)
			DeleteObject(hFont);
		KillTimer(hwnd, ID_HOOKCHECKTIMER);
		MouseUnHook(); //フックを終了する
		MyTaskTray(NULL);
		UnShadeWindowAll();
		PostQuitMessage(0);
		break;
	case WM_QUERYENDSESSION:
		if(IniEditFlag)
		{
			SaveConfig(&configTemp);
		}
		else
		{
			if(NaviType == NaviTypeFixed)
				GetWindowPosition(hwndNavi, &MauSujiNavi);
			_GetConfig(&config);
			SaveConfig(&config);
			IniChange = FALSE;
		}
		KillTimer(hwnd, ID_HOOKCHECKTIMER);
		MouseUnHook(); //フックを終了する
		MyTaskTray(NULL);
		UnShadeWindowAll();
		return TRUE;
	case WM_ENDSESSION:
		if(wParam)
		{
		}
		break;
	case WM_NCRBUTTONUP:
	case WM_RBUTTONUP:
		MakeTrayMenu(hwnd);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDM_OPENHELP:
			OpenHelp();
			break;
		case IDM_VERSIONINFO:
			ShowVersionInfo(hwnd);
			break;
		case IDM_MOUSEHOOK:
			if(IniEditFlag)
			{
				SendMessage(hDlgOption, (WM_APP+WM_COMMAND), MAKEWPARAM(IDM_MOUSEHOOK, 0), 0);
			}
			else
			{
				MouseHookFlag = !MouseHookFlag;
				if(MouseHookFlag)
					SetMouseHook(hwnd);
				else
					MouseUnHook();
			}
			break;
		case IDM_WHEELREDIRECT:
			if(IniEditFlag)
			{
				SendMessage(hDlgOption, (WM_APP+WM_COMMAND), MAKEWPARAM(IDM_WHEELREDIRECT, 0), 0);
			}
			else
			{
				WheelRedirect = !WheelRedirect;
				SetWheelRedirectFlag(WheelRedirect);
				IniChange = TRUE;
			}
			break;
		case IDM_SHOWDIALOG:
			if(IniEditFlag)
			{
				_SetForegroundWindow(hDlgOption);
			}
			else
			{
				IniEditFlag = TRUE;

				//現在の設定を保存
				if(NaviType == NaviTypeFixed)
					GetWindowPosition(hwndNavi, &MauSujiNavi);
				configTemp.target.clear();
				_GetConfig(&configTemp);
				//設定ダイアログを開く
				IniChange = FALSE;
				if(DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_OPTION), NULL, DialogProcOption, (LPARAM)hwnd)!=IDOK)
				{
					//キャンセルしたとき、設定を読み込み
					_SetConfig(&configTemp);
					ApplyConfig(hwnd);
				}
				IniEditFlag = FALSE;
			}
			break;
		case IDM_HIDEMAUSUJINAVI:
			if(IniEditFlag)
			{
				SendMessage(hDlgOption, (WM_APP+WM_COMMAND), MAKEWPARAM(IDM_HIDEMAUSUJINAVI, 0), 0);
			}
			else
			{
				if(NaviType != NaviTypeNone)
				{
					NaviType = NaviTypeNone;
					ShowMauSujiNavi(hwnd, NaviType);
				}
			}
			break;
		case IDM_SHOWMAUSUJINAVI:
			if(IniEditFlag)
			{
				SendMessage(hDlgOption, (WM_APP+WM_COMMAND), MAKEWPARAM(IDM_SHOWMAUSUJINAVI, 0), 0);
			}
			else
			{
				if(NaviType != NaviTypeFixed)
				{
					NaviType = NaviTypeFixed;
					NaviEditRefresh(hwnd);
					ShowMauSujiNavi(hwnd, NaviType);
				}
			}
			break;
		case IDM_TIPMAUSUJINAVI:
			if(IniEditFlag)
			{
				SendMessage(hDlgOption, (WM_APP+WM_COMMAND), MAKEWPARAM(IDM_TIPMAUSUJINAVI, 0), 0);
			}
			else
			{
				if(NaviType != NaviTypeFloat)
				{
					NaviType = NaviTypeFloat;
					NaviEditRefresh(hwnd);
					ShowMauSujiNavi(hwnd, NaviType);
				}
			}
			break;
		case IDM_ENDMAUSUJI:
			//終了
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case IDM_IMPORTSETTING:
		case IDM_EXPORTSETTING:
			if(IniEditFlag)
			{
			}
			else
			{
				OPENFILENAME ofn;
				TCHAR szPath[MAX_PATH] = TEXT("MauSuji.ini");
				ZeroMemory(&ofn, sizeof(OPENFILENAME));    
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner   = hDlgOption;
				ofn.lpstrFile   = szPath;
				ofn.nMaxFile    = MAX_PATH;
				ofn.lpstrFilter = TEXT("iniファイル(*.ini)\0*.ini\0すべてのファイル(*.*)\0*.*\0\0");
				ofn.lpstrTitle  = TEXT("ファイル選択");
				ofn.Flags       = OFN_HIDEREADONLY|OFN_NOCHANGEDIR;
				if(LOWORD(wParam)==IDM_IMPORTSETTING)
				{
					if(MessageBox(hwnd, TEXT("設定をインポートすると現在の設定が破棄されます\n設定をインポートしますか？"), TEXT("設定をインポート"), MB_YESNO)==IDYES)
					{
						ofn.Flags |= OFN_FILEMUSTEXIST;
						if(GetOpenFileName(&ofn)!=0)
						{
							//インポート
							IniRead(szPath);
							SetWindowPos(hwnd, NULL, MauSujiNavi.x, MauSujiNavi.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
							ApplyConfig(hwnd);
						}
					}
				}
				else
				{
					ofn.Flags |= OFN_OVERWRITEPROMPT;
					if(GetSaveFileName(&ofn)!=0)
					{
						//エクスポート
						IniWrite(szPath);
					}
				}
			}
		}
		break;
	case WM_TIMER:
		OnTimer(hwnd, (UINT)wParam);
		break;
	case WM_DWMCOMPOSITIONCHANGED:
		if(MouseGestureTrailDrawMode==0)
			SetMouseGestureTrailModeAuto();
		break;

	case MYMSG_TRAY:
		if(wParam == ID_MYTRAY)
		{
			switch(lParam)
			{
			case WM_LBUTTONDOWN:
				PostMessage(hwnd, WM_COMMAND, IDM_SHOWDIALOG, 0);
				break;
			case WM_RBUTTONUP:
				PostMessage(hwnd, WM_RBUTTONUP, 0, 0);
				break;
			case WM_MBUTTONUP:
				SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDM_MOUSEHOOK, 0), 0);
				break;
			}
		}
		break;
	case MAUHOOK_MSG:
		return OnMauHook(hwnd, wParam, lParam);
	case WHEELREDIRECT_MSG:
		RedirectTarget = SearchTarget((HWND)wParam, FALSE);
		if(RedirectTarget >= 0 && RedirectTarget < (int)Target.size())
			return (Target[RedirectTarget].WheelRedirect)?TRUE:FALSE;
		else
			return (Default.WheelRedirect)?TRUE:FALSE;
		break;
	case NAVIMINIMIZE_MSG:
		break;
	case FREESCROLL_MSG:
		FreeScroll((INT)wParam, (INT)lParam, FALSE, NULL);
		break;
	case KURUKURUHOOK_MSG:
		KuruKuruScroll(hwnd, (INT)wParam, (INT)lParam);
		break;
	case FREESCROLLSEARCH_MSG:
		RedirectTarget = SearchTarget((HWND)wParam, FALSE);
		if(RedirectTarget >= 0 && RedirectTarget < (int)Target.size())
			return (Target[RedirectTarget].FreeScroll)?TRUE:FALSE;
		else
			return (Default.FreeScroll)?TRUE:FALSE;
		break;
	case FREESCROLLINIT_MSG:
		FreeScroll(0, 0, TRUE, (HWND)wParam);
		break;
	case GETTARGETFLAG_MSG:
		RedirectTarget = SearchTarget((HWND)wParam, FALSE);
		if(RedirectTarget >= 0 && RedirectTarget < (int)Target.size())
			return ((Target[RedirectTarget].WheelRedirect)?1:0) | ((Target[RedirectTarget].FreeScroll)?2:0);
		else
			return ((Default.WheelRedirect)?1:0) | ((Default.FreeScroll)?2:0);
		break;
	default:
		if(msg==WM_TASKBARCREATED && ShowTaskTray)
			MyTaskTray(hwnd);
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

BOOL CALLBACK FindWindowProc(HWND hWnd, LPARAM lParam)
{
	/* 見つかったウィンドウが、すでに起動しているアプリケーションのウィンドウか調べる */
	if(GetProp(hWnd, TEXT("MauSujiProp")))
	{
		PostMessage(hWnd, WM_COMMAND, IDM_SHOWDIALOG, 0);
		return FALSE; /* 列挙を中断 */
	}
	return TRUE;  /* 列挙を続ける */
}

static void StartCommandLine(LPTSTR StrCmdLine)
{
	TCHAR *p, *r;

	p = StrCmdLine;
	while(*p != '\0'){
		for(; *p == ' '; p++);
		if(*p == '\0') break;

		if(*p != '/'){
			if(*p == '"'){
				p++;
				for(p++; *p != '\0' && *p != '"'; p++);
			}else{
				for(; *p != '\0' && *p != ' '; p++);
			}
			if(*p != '\0') p++;
			continue;
		}

		for(p++; *p != '\0' && *p != ' '; p++){
			switch(*p)
			{
			case 'i':
			case 'I':
				p++;
				if(*p != ':') break;
				p++;
				if(*p == '"'){
					p++;
					for(r = p; *r != '\0' && *r != '"'; r++);
				}else{
					for(r = p; *r != '\0' && *r != ' '; r++);
				}
				if(r - p + 1<MAX_PATH)
					lstrcpyn(szConfigFilePath, p, (int)(r - p + 1));
				p = r;
				if(*p == '\"') p++;
				break;
			case 'l':
			case 'L':
				LeftyMode = TRUE;
				break;
			}
			if(*p == '\0') break;
		}
	}
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HANDLE hMutex;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	WNDCLASSEX wcex;
	HWND hWnd;
	MSG msg;

	hInstance = hInst;

	hMutex = CreateMutex(NULL, FALSE, TEXT("MPaIurSouCjhi")); //オブジェクト名は適当。ユニークである程良い
	if(GetLastError() == ERROR_ALREADY_EXISTS) //既に同じオブジェクトがあったら
	{ 
		EnumWindows(&FindWindowProc, NULL);
		CloseHandle(hMutex);
		return 0;
	}

	StartCommandLine(GetCommandLine());	//StartCommandLine(lpCmdLine);
	if(szConfigFilePath[0])
	{
		if(PathIsRelative(szConfigFilePath))	//相対パス
		{
			//絶対パスに変換
			TCHAR szConfigFilePathTemp[MAX_PATH];
			TCHAR szDir[MAX_PATH];
			TCHAR szPath[MAX_PATH];
			lstrcpy(szConfigFilePathTemp, szConfigFilePath);
			GetModuleFileName(NULL, szDir, MAX_PATH);
			PathRemoveFileSpec(szDir);
			wsprintf(szPath, TEXT("%s\\%s"), szDir, szConfigFilePathTemp);
			PathCanonicalize(szConfigFilePath, szPath);
		}
	}

	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, 0);

	InitMouseGestureTrail(hInst);

	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW|CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = TEXT("MauSuji");
	wcex.hIconSm       = NULL;
	RegisterClassEx( &wcex );

	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = 0;
	wcex.lpfnWndProc   = WndProcNavi;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = NULL;
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = TEXT("MauSujiNavi");
	wcex.hIconSm       = NULL;
	RegisterClassEx( &wcex );

	hWnd = CreateWindowEx(
		0,
		TEXT("MauSuji"),
		TEXT("MauSuji"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	SetProp(hWnd, TEXT("MauSujiProp"), (HANDLE)1);
	LoadPlugins(hWnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnloadPlugins();
	RemoveProp(hWnd, TEXT("MauSujiProp"));
	TermMouseGestureTrail(hInst);
	Gdiplus::GdiplusShutdown(gdiplusToken);
	CloseHandle(hMutex); //ミューテックスハンドルのクローズ

	return (int)msg.wParam;
}
