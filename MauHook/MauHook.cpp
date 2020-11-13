
#define WINVER          0x0501
#define _WIN32_WINNT    0x0501
#define _WIN32_IE       0x0501

#include <windows.h>
#include <commctrl.h>

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL                  0x020E
#endif

#include "MauHook.h"
#include "../MauSuji/main.h"

#pragma data_seg(".mshared")

HHOOK NextMouseHook = NULL;	//フックプロシージャのハンドル
HHOOK NextMessageHook = NULL;
HHOOK NextButtonSearchHook = NULL;
HHOOK KeyBoardHook = NULL;
HHOOK NextLowLevelHook = NULL;
HWND CallWnd = NULL;	//呼び出し元のウィンドウハンドル
HWND CallDlg1 = NULL;
HWND hwTarget = NULL;
HWND hCommandDialog = NULL;
HWND hSendKeyCombo = NULL;
HCURSOR hcursor_gesture = NULL;
HCURSOR hcursor_wait = NULL;
BOOL ButtonSearchHook = FALSE;
int Cursor = 3;
BOOL ButtonFlag = FALSE;
BOOL HookFlag = TRUE;	//TRUE:フック有効 FALSE:フック無効
BOOL SetTargetFlag = FALSE;
BOOL GetWindowFocusFlag = FALSE;
HWND WindowFocus = NULL;
BOOL WheelRedirectFlag = FALSE;
BOOL X_Button_1 = FALSE;
BOOL X_Button_2 = FALSE;
POINT MousePos = {0, 0};
BOOL ScrollFlag = FALSE;
BOOL WheelActiveFlag = FALSE;
BOOL MessageRedirectFlag = FALSE;
BOOL KuruKuruFlag = FALSE;
BOOL FreeScrollFlag = FALSE;
int ScrollLine = 0;
int ScrollPage = 0;
UINT uMsgSetTarget = 0;
UINT uMsgGetFocus = 0;
#ifdef _WIN64
#else
BOOL Wow64Process = FALSE;
#endif	//_WIN64

#pragma data_seg()


HINSTANCE hInstDLL;		//DLLのインスタンスハンドル

//DLLメイン
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwNotification, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(lpReserved);

	hInstDLL = hInstance;
	return TRUE;
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

HWND SearchScrollBar(HWND hWnd, LONG_PTR BarStyle)
{
	TCHAR szClassName[256];
	LONG_PTR style;

	hWnd = GetTopWindow(hWnd);//子の最初のウィンドを取得
	while(hWnd != NULL)
	{
		GetClassName(hWnd, szClassName, 256);
		if(lstrcmpi(szClassName, TEXT("ScrollBar")) == 0)
		{
			style = GetWindowLongPtr(hWnd, GWL_STYLE);
			if((style & 0x0001) == BarStyle && (style & WS_VISIBLE) && !(style & WS_DISABLED))
			{//スクロールバーウィンドがある
				return hWnd;
			}
		}
		hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
	}
	return NULL;
}

void MouseScroll(int PosX, int PosY) //その場でスクロール
{
	POINT RealPos;

	GetCursorPos(&RealPos);

	if(abs(MousePos.x - PosX) >= ScrollLine)
	{
		if(abs(MousePos.x - PosX) >= ScrollPage)
		{
			if(MousePos.x > PosX)
				PostMessage(hwTarget, WM_HSCROLL, SB_PAGELEFT, (LPARAM)SearchScrollBar(hwTarget, SBS_HORZ));
			else
				PostMessage(hwTarget, WM_HSCROLL, SB_PAGERIGHT, (LPARAM)SearchScrollBar(hwTarget, SBS_HORZ));
		}
		else
		{
			if(MousePos.x > PosX)
				PostMessage(hwTarget, WM_HSCROLL, SB_LINELEFT, (LPARAM)SearchScrollBar(hwTarget, SBS_HORZ));
			else
				PostMessage(hwTarget, WM_HSCROLL, SB_LINERIGHT, (LPARAM)SearchScrollBar(hwTarget, SBS_HORZ));
		}
		MousePos.x = RealPos.x;
	}
	else
	{
		MousePos.x += (MousePos.x - PosX);
	}

	if(abs(MousePos.y - PosY) >= ScrollLine)
	{
		if(abs(MousePos.y - PosY) >= ScrollPage)
		{
			if(MousePos.y > PosY)
				PostMessage(hwTarget, WM_VSCROLL, SB_PAGEUP, (LPARAM)SearchScrollBar(hwTarget, SBS_VERT));
			else
				PostMessage(hwTarget, WM_VSCROLL, SB_PAGEDOWN, (LPARAM)SearchScrollBar(hwTarget, SBS_VERT));
		}
		else
		{
			if(MousePos.y > PosY)
				PostMessage(hwTarget, WM_VSCROLL, SB_LINEUP, (LPARAM)SearchScrollBar(hwTarget, SBS_VERT));
			else
				PostMessage(hwTarget, WM_VSCROLL, SB_LINEDOWN, (LPARAM)SearchScrollBar(hwTarget, SBS_VERT));
		}
		MousePos.y = RealPos.y;
	}
	else
	{
		MousePos.y += (MousePos.y - PosY);
	}
}
//マウスフックのプロシージャ
LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PMOUSEHOOKSTRUCT mousehooks = (PMOUSEHOOKSTRUCT)lParam;
	BYTE Press_X_Button = 0;
	BOOL New_X_Button_1, New_X_Button_2;
	UINT X_Button_Code = 0x0000;
#ifdef _WIN64
#else
	BOOL x86APP;
#endif

	if(nCode==HC_ACTION)
	{
		switch(wParam)
		{
		case WM_XBUTTONDOWN:
		case WM_NCXBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_NCXBUTTONDBLCLK:
		case WM_XBUTTONUP:
		case WM_NCXBUTTONUP:
			New_X_Button_1 = GetAsyncKeyState( VK_XBUTTON1 ) & 0x8000;
			New_X_Button_2 = GetAsyncKeyState( VK_XBUTTON2 ) & 0x8000;
			if(!X_Button_1 && New_X_Button_1)
				Press_X_Button = 1;
			else if(!X_Button_2 && New_X_Button_2)
				Press_X_Button = 2;
			else if(New_X_Button_1)
				Press_X_Button = 1;
			else if(New_X_Button_2)
				Press_X_Button = 2;
			X_Button_1 = New_X_Button_1;
			X_Button_2 = New_X_Button_2;
			break;
		}

		if(HookFlag)
		{
			switch(wParam)
			{
			case WM_LBUTTONDOWN:
			case WM_NCLBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_NCMBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_NCRBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
			case WM_NCLBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
			case WM_NCMBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_NCRBUTTONDBLCLK:
				if(hwTarget == NULL)
				{
#ifdef _WIN64
#else
					if(Wow64Process)//wow64からのメッセージを殺すととりあえずいいみたい
					{
						if(IsWow64Process(mousehooks->hwnd, &x86APP))
						{
						}
						else
						{
							if(x86APP == FALSE)
								return CallNextHookEx(NextMouseHook, nCode, wParam, lParam);
						}
					}
#endif	//_WIN64
					hwTarget = mousehooks->hwnd;
					SendMessage(CallWnd, MAUHOOK_MSG, wParam, (LPARAM)hwTarget);
				}
				else
				{
					PostMessage(CallWnd, MAUHOOK_MSG, wParam, 0);
				}
			case WM_LBUTTONUP:
			case WM_NCLBUTTONUP:
			case WM_MBUTTONUP:
			case WM_NCMBUTTONUP:
			case WM_RBUTTONUP:
			case WM_NCRBUTTONUP:
				if(ButtonFlag)
					return TRUE;
				break;
			case WM_XBUTTONDOWN:
			case WM_NCXBUTTONDOWN:
			case WM_XBUTTONDBLCLK:
			case WM_NCXBUTTONDBLCLK:
				switch(wParam)
				{
				case WM_XBUTTONDOWN:
				case WM_NCXBUTTONDOWN:
					if(Press_X_Button == 1)
						X_Button_Code = WM_XBUTTON1DOWN;
					else if(Press_X_Button == 2)
						X_Button_Code = WM_XBUTTON2DOWN;
					break;
				case WM_XBUTTONDBLCLK:
				case WM_NCXBUTTONDBLCLK:
					if(Press_X_Button == 1)
						X_Button_Code = WM_XBUTTON1DBLCLK;
					else if(Press_X_Button == 2)
						X_Button_Code = WM_XBUTTON2DBLCLK;
					break;
				}

				if(hwTarget == NULL)
				{
					hwTarget = mousehooks->hwnd;
					SendMessage(CallWnd, MAUHOOK_MSG, X_Button_Code, (LPARAM)hwTarget);
				}
				else
				{
					PostMessage(CallWnd, MAUHOOK_MSG, X_Button_Code, 0);
				}
			case WM_XBUTTONUP:
			case WM_NCXBUTTONUP:
				if(ButtonFlag)
					return TRUE;
				break;
			default:
				break;
			}
		}
	}
	return CallNextHookEx(NextMouseHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	MSG *msg = (MSG *)lParam;

	if(nCode==HC_ACTION)
	{
		switch(Cursor)
		{
		case 0:
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			Cursor = 3;
			break;
		case 1:
			SetCursor(hcursor_wait);
			break;
		case 2:
			SetCursor(hcursor_gesture);
			break;
		}

		if(HookFlag)
		{
			if(msg->message == uMsgSetTarget)
			{
				SetTargetFlag = FALSE;
				SetFocus(msg->hwnd);
				msg->message = WM_NULL;
				msg->wParam = 0;
				msg->lParam = 0;
				return FALSE;
			}
			if(msg->message == uMsgGetFocus)
			{
				GetWindowFocusFlag = FALSE;
				WindowFocus = GetFocus();
				msg->message = WM_NULL;
				msg->wParam = 0;
				msg->lParam = 0;
				return FALSE;
			}
		}
	}
	return CallNextHookEx(NextMessageHook, nCode, wParam, lParam);
}

LRESULT CALLBACK ButtonSearchHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	CWPSTRUCT *pcwp = (CWPSTRUCT *)lParam;
	TCHAR buffer[20];

	if(nCode==HC_ACTION)
	{
		if(pcwp->message==WM_COMMAND)
		{
			UnhookWindowsHookEx(NextButtonSearchHook);
			ButtonSearchHook = FALSE;
			wsprintf(buffer, TEXT("%d"), LOWORD(pcwp->wParam));
			SendMessage(CallDlg1, WM_SETTEXT, 0, (LPARAM)buffer);
		}
	}
	return CallNextHookEx(NextButtonSearchHook, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	PMSLLHOOKSTRUCT mousehooks = (PMSLLHOOKSTRUCT)lParam;
	HWND RedirectTarget;
	UINT WheelMessage;

	if(nCode==HC_ACTION)
	{
		switch(wParam)
		{
		case WM_LBUTTONDOWN:
		case WM_NCLBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_NCMBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_NCRBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_NCLBUTTONDBLCLK:
		case WM_MBUTTONDBLCLK:
		case WM_NCMBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
		case WM_NCRBUTTONDBLCLK:
			break;
		case WM_LBUTTONUP:
		case WM_NCLBUTTONUP:
		case WM_MBUTTONUP:
		case WM_NCMBUTTONUP:
		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP:
			FreeScrollFlag = FALSE;
			break;
		case WM_XBUTTONDOWN:
		case WM_NCXBUTTONDOWN:
		case WM_XBUTTONDBLCLK:
		case WM_NCXBUTTONDBLCLK:
		case WM_XBUTTONUP:
		case WM_NCXBUTTONUP:
			FreeScrollFlag = FALSE;
			break;
		case WM_NCMOUSEMOVE:
		case WM_MOUSEMOVE:
			if(KuruKuruFlag)
			{
				SendMessage(CallWnd, KURUKURUHOOK_MSG, mousehooks->pt.x, mousehooks->pt.y);
				return TRUE;
			}
			else if(FreeScrollFlag)
			{
				SendMessage(CallWnd, FREESCROLL_MSG, mousehooks->pt.x, mousehooks->pt.y);
				return TRUE;
			}
			else if(ScrollFlag)
			{
				if(mousehooks->pt.x != MousePos.x || mousehooks->pt.y != MousePos.y)
					MouseScroll(mousehooks->pt.x, mousehooks->pt.y);
				return TRUE;
			}
			else
			{
//				HitTestCheck(MAKELPARAM(mousehooks->pt.x, mousehooks->pt.y));
			}
			if(HookFlag)
				SendMessage(CallWnd, MAUHOOK_MSG, 0, 0);
			break;
		case WM_MOUSEWHEEL:
			if((SHORT)HIWORD(mousehooks->mouseData) > 0)
				WheelMessage = WM_MOUSEWHEEL_UP;
			else
				WheelMessage = WM_MOUSEWHEEL_DOWN;

			if(hwTarget==NULL)
			{
				hwTarget = WindowFromPoint(mousehooks->pt);
				SendMessage(CallWnd, MAUHOOK_MSG, WheelMessage, (LPARAM)hwTarget);
			}
			else
			{
				PostMessage(CallWnd, MAUHOOK_MSG, WM_MOUSEWHEEL, (LPARAM)(SHORT)HIWORD(mousehooks->mouseData));
			}

			if(ButtonFlag)
			{
				return TRUE;
			}
			else
			{
				if(WheelRedirectFlag)	//スクロールを監視する
				{
					int flags;
					RedirectTarget = WindowFromPoint(mousehooks->pt);
					flags = (int)SendMessage(CallWnd, GETTARGETFLAG_MSG, (WPARAM)RedirectTarget, 0);
					if(flags&2)	//フリースクロール
					{
						if(WheelActiveFlag)	//ホイール回転でアクティブ
						{
							if(GetForegroundWindow()!=GetAncestor(RedirectTarget, GA_ROOT))
							{
								_SetForegroundWindow(CallWnd);
								BringWindowToTop(GetRootWindow(RedirectTarget));
								SetTargetFlag = TRUE;
								PostMessage(RedirectTarget, uMsgSetTarget, 0, 0);
							}
						}

						if(WheelMessage==WM_MOUSEWHEEL_DOWN)
						{
							//フリースクロール開始
							FreeScrollFlag = TRUE;
							SendMessage(CallWnd, FREESCROLLINIT_MSG, (WPARAM)RedirectTarget, 0);
						}
						else
						{
							FreeScrollFlag = FALSE;
						}
						return TRUE;
					}

					if(flags&1)	//非アクティブでもスクロール
					{
						if(WheelActiveFlag)	//ホイール回転でアクティブ
						{
							if(GetForegroundWindow()!=GetAncestor(RedirectTarget, GA_ROOT))
							{
								_SetForegroundWindow(CallWnd);
								BringWindowToTop(GetRootWindow(RedirectTarget));
								SetTargetFlag = TRUE;
								PostMessage(RedirectTarget, uMsgSetTarget, 0, 0);
							}
						}
						//非アクティブでもスクロール
						WORD key;
						key = ( (GetAsyncKeyState( VK_LBUTTON )  < 0) ? MK_LBUTTON  : 0) |
							( (GetAsyncKeyState( VK_RBUTTON )  < 0) ? MK_RBUTTON  : 0) |
							( (GetAsyncKeyState( VK_SHIFT )    < 0) ? MK_SHIFT    : 0) |
							( (GetAsyncKeyState( VK_CONTROL )  < 0) ? MK_CONTROL  : 0) |
							( (GetAsyncKeyState( VK_MBUTTON )  < 0) ? MK_MBUTTON  : 0) |
							( (GetAsyncKeyState( VK_XBUTTON1 ) < 0) ? MK_XBUTTON1 : 0) |
							( (GetAsyncKeyState( VK_XBUTTON2 ) < 0) ? MK_XBUTTON2 : 0);
						PostMessage(RedirectTarget, WM_MOUSEWHEEL, MAKEWPARAM(key, HIWORD(mousehooks->mouseData)), MAKELPARAM(mousehooks->pt.x, mousehooks->pt.y));
						return TRUE;
					}
				}
			}
			break;
		case WM_MOUSEHWHEEL:
			if((SHORT)HIWORD(mousehooks->mouseData) < 0)
				WheelMessage = WM_MOUSEWHEEL_LEFT;
			else
				WheelMessage = WM_MOUSEWHEEL_RIGHT;

			if(hwTarget==NULL)
			{
				hwTarget = WindowFromPoint(mousehooks->pt);
				SendMessage(CallWnd, MAUHOOK_MSG, WheelMessage, (LPARAM)hwTarget);
			}
			else
			{
				PostMessage(CallWnd, MAUHOOK_MSG, WM_MOUSEHWHEEL, (LPARAM)(SHORT)HIWORD(mousehooks->mouseData));
			}

			if(ButtonFlag)
			{
				return TRUE;
			}
			else
			{
				if(WheelRedirectFlag)	//スクロールを監視する
				{
					RedirectTarget = WindowFromPoint(mousehooks->pt);

					if(SendMessage(CallWnd, WHEELREDIRECT_MSG, (WPARAM)RedirectTarget, 0))	//非アクティブでもスクロール
					{
						if(WheelActiveFlag)	//ホイール回転でアクティブ
						{
							if(GetForegroundWindow()!=GetAncestor(RedirectTarget, GA_ROOT))
							{
								_SetForegroundWindow(CallWnd);
								BringWindowToTop(GetRootWindow(RedirectTarget));
								SetTargetFlag = TRUE;
								PostMessage(RedirectTarget, uMsgSetTarget, 0, 0);
							}
						}
						//非アクティブでもスクロール
						WORD key;
						key = ( (GetAsyncKeyState( VK_LBUTTON )  < 0) ? MK_LBUTTON  : 0) |
							( (GetAsyncKeyState( VK_RBUTTON )  < 0) ? MK_RBUTTON  : 0) |
							( (GetAsyncKeyState( VK_SHIFT )    < 0) ? MK_SHIFT    : 0) |
							( (GetAsyncKeyState( VK_CONTROL )  < 0) ? MK_CONTROL  : 0) |
							( (GetAsyncKeyState( VK_MBUTTON )  < 0) ? MK_MBUTTON  : 0) |
							( (GetAsyncKeyState( VK_XBUTTON1 ) < 0) ? MK_XBUTTON1 : 0) |
							( (GetAsyncKeyState( VK_XBUTTON2 ) < 0) ? MK_XBUTTON2 : 0);
						PostMessage(RedirectTarget, WM_MOUSEHWHEEL, MAKEWPARAM(key, HIWORD(mousehooks->mouseData)), MAKELPARAM(mousehooks->pt.x, mousehooks->pt.y));
						return TRUE;
					}
				}
			}
			break;
		}
	}
	return CallNextHookEx(NextLowLevelHook, nCode, wParam, lParam);
}

void SetCursorNumber(int CursorNumber)
{
	if(Cursor < 4)
		Cursor = CursorNumber;
}

void SetCursorHandle(LPTSTR GestureCursor, LPTSTR WaitCursor, BOOL CursorChange)
{
	if(CursorChange)
	{	
		if(hcursor_gesture != NULL)
			DestroyCursor(hcursor_gesture);

		if(hcursor_wait != NULL)
			DestroyCursor(hcursor_wait);

		hcursor_gesture = LoadCursorFromFile(GestureCursor);
		if(hcursor_gesture==NULL)
			hcursor_gesture = LoadCursor(NULL, IDC_CROSS);

		hcursor_wait = LoadCursorFromFile(WaitCursor);
		if(hcursor_wait==NULL)
			hcursor_wait = LoadCursor(NULL, IDC_APPSTARTING);
		Cursor = 3;
	}
	else
		Cursor = 4;
}

void SetButtonFlag(BOOL Flag)
{
	ButtonFlag = Flag;
}

void SetHookFlag(BOOL fEnable)
{
	HookFlag = fEnable;
}

void ResetButtonFlag()
{
	HookFlag = TRUE;
	hwTarget = NULL;
	ButtonFlag = FALSE;
	ScrollFlag = FALSE;
}
//フックの開始
void SetMouseHook(HWND hWnd)
{
	CallWnd = hWnd;

	//誤動作を防止するためとりあえず初期化
	HookFlag = TRUE;
	hwTarget = NULL;
	ButtonFlag = FALSE;

	X_Button_1 = GetAsyncKeyState( VK_XBUTTON1 ) & 0x8000;
	X_Button_2 = GetAsyncKeyState( VK_XBUTTON2 ) & 0x8000;

	uMsgSetTarget = RegisterWindowMessage(TEXT("MauSujiMsgSetTarget"));
	uMsgGetFocus = RegisterWindowMessage(TEXT("MauSujiMsgGetFocus"));

	//フックを開始する
	NextMouseHook = SetWindowsHookEx(WH_MOUSE, MouseHookProc, hInstDLL, 0);
	NextMessageHook = SetWindowsHookEx(WH_GETMESSAGE, MessageHookProc, hInstDLL, 0);
	NextLowLevelHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseHookProc, hInstDLL, 0);

#ifdef _WIN64
#else
	// OS がどの CPU アーキテクチャ用なのか判定
	IsWow64Process(GetCurrentProcess(), &Wow64Process);
#endif	//_WIN64
}

BOOL CALLBACK PostNullProc(HWND hWnd, LPARAM lParam)
{
	PostMessage(hWnd, WM_NULL, 0, 0);
	return TRUE;
}
//フックの終了
void MouseUnHook(void)
{
	if(NextMouseHook)
		UnhookWindowsHookEx(NextMouseHook);
	if(NextMessageHook)
		UnhookWindowsHookEx(NextMessageHook);
	if(NextLowLevelHook)
		UnhookWindowsHookEx(NextLowLevelHook);
	EnumWindows(PostNullProc, NULL);
}

void SetButtonSearchHook(HWND hDlgItem)
{
	if(!ButtonSearchHook)
	{
		ButtonSearchHook = TRUE;
		CallDlg1 = hDlgItem;
		NextButtonSearchHook = SetWindowsHookEx(WH_CALLWNDPROC, ButtonSearchHookProc, hInstDLL, 0);
	}
}

void ButtonSearchUnHook(void)
{
	if(NextButtonSearchHook)
		UnhookWindowsHookEx(NextButtonSearchHook);
}

LRESULT CALLBACK KeyBoardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode >= 0 && GetFocus() == hSendKeyCombo)
	{
		if(GetKeyState( (int)wParam ) & 0x8000)
			SendMessage(hCommandDialog, WM_KEYBOARD_MSG, wParam, lParam);
		return TRUE;
	}
	return CallNextHookEx(KeyBoardHook, nCode, wParam, lParam);
}

void SetSendKeyComboHook(HWND hDlg, HWND hCmb)
{
	hCommandDialog = hDlg;
	hSendKeyCombo = hCmb;
	KeyBoardHook = SetWindowsHookEx(WH_KEYBOARD, KeyBoardHookProc, hInstDLL, 0);
}

void SetSendKeyComboUnHook(void)
{
	if(KeyBoardHook)
		UnhookWindowsHookEx(KeyBoardHook);
}

void SetTarget()
{
	SetTargetFlag = TRUE;
	PostMessage(hwTarget, uMsgSetTarget, 0, 0);
	Sleep(0);
}

HWND GetTargetHandle()
{
	return hwTarget;
}

HWND GetWindowFocus(HWND hWnd)
{
	GetWindowFocusFlag = TRUE;
	PostMessage(hWnd, uMsgGetFocus, 0, 0);
	Sleep(0);
	return WindowFocus;
}

void SetWheelRedirectFlag(BOOL Flag)
{
	WheelRedirectFlag = Flag;
}

HWND GetRootWindow(HWND hWnd)
{
	return GetAncestor(hWnd, GA_ROOT);
}

void SetMouseScroll(int S_Line, int S_Page)
{
	ScrollLine = S_Line;
	ScrollPage = S_Page;
	ScrollFlag = TRUE;
	GetCursorPos(&MousePos);
	return;
}

void SetWheelActiveFlag(BOOL Flag)
{
	WheelActiveFlag = Flag;
}

void SetKuruKuruFlag(BOOL Flag)
{
	KuruKuruFlag = Flag;
}
