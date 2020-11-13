
#ifdef __cplusplus
extern "C" {
#endif

void SetMouseHook(HWND);
void MouseUnHook(void);
void SetButtonSearchHook(HWND);
void ButtonSearchUnHook(void);
void SetSendKeyComboHook(HWND, HWND);
void SetSendKeyComboUnHook(void);
void SetButtonFlag(BOOL);
void SetHookFlag(BOOL);
void ResetButtonFlag(void);
void SetCursorNumber(int);
void SetCursorHandle(LPTSTR, LPTSTR, BOOL);
void SetTarget(void);
HWND GetTargetHandle(void);
HWND GetWindowFocus(HWND);
void SetWheelRedirectFlag(BOOL);
HWND GetRootWindow(HWND);
void SetMouseScroll(int, int);
HWND SearchScrollBar(HWND, LONG_PTR);
void SetWheelActiveFlag(BOOL);
void SetKuruKuruFlag(BOOL);

#ifdef __cplusplus
}
#endif
