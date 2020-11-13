
#ifdef _WIN64
#define OPENMAUSUJI_VERSION		TEXT("OpenMauSuji ver. 1.33\r\nx64")
#else
#define OPENMAUSUJI_VERSION		TEXT("OpenMauSuji ver. 1.33\r\nx86")
#endif	//_WIN64
#define MAUSUJI_VERSION		TEXT("É}ÉEãÿ  ver. 1.41 beta 3\r\nby ÉsÉç")
#define INI_VERSION			11

#define MAX_TARGET_DATA		500
#define MAX_ACTION_DATA		3000
#define MAX_ACTION_REPEAT	30
#define MAX_GESTURE_LEVEL	10
#define MAX_ACTION_CASE		21
#define MAX_TARGET_THRU		10
#define MAX_ZORDER			13

#define MAX_COMMENT			100
#define MAX_SHADEWINDOW		500
#define MAX_CLASSNAME		128

#define ID_MYTRAY			101
#define ID_TIMER			102
#define ID_NAVITIMER		103
#define ID_TIMEOUTTIMER		104
#define ID_DBLCLKTIMER		105
#define ID_KURUKURUTIMER	106
#define ID_HOOKCHECKTIMER	109

#define MYMSG_TRAY			(WM_APP + 50)
#define MAUHOOK_MSG			(WM_APP + 100)
#define NAVIMINIMIZE_MSG	(WM_APP + 108)
#define WM_COMBO_REFRESH	(WM_APP + 109)	//CommandDialogProc
#define WHEELREDIRECT_MSG	(WM_APP + 107)
#define WM_KEYBOARD_MSG		(WM_APP + 110)	//CommandDialogProc
#define FREESCROLL_MSG		(WM_APP + 111)
#define KURUKURUHOOK_MSG	(WM_APP + 112)
#define FREESCROLLSEARCH_MSG	(WM_APP + 113)
#define FREESCROLLINIT_MSG	(WM_APP + 114)
#define GETTARGETFLAG_MSG	(WM_APP + 115)

#define WM_XBUTTON1DOWN		(WM_APP + 121)
#define WM_XBUTTON1UP		(WM_APP + 122)	//
#define WM_XBUTTON1DBLCLK	(WM_APP + 123)
#define WM_XBUTTON2DOWN		(WM_APP + 124)
#define WM_XBUTTON2UP		(WM_APP + 125)	//
#define WM_XBUTTON2DBLCLK	(WM_APP + 126)
#define WM_MOUSEWHEEL_UP	(WM_APP + 127)
#define WM_MOUSEWHEEL_DOWN	(WM_APP + 128)
#define WM_MOUSEWHEEL_LEFT	(WM_APP + 129)
#define WM_MOUSEWHEEL_RIGHT	(WM_APP + 130)

#define HOTKEYF_WIN			0x10

#define MODIFIER_CONTROL	0x02
#define MODIFIER_SHIFT		0x04
#define MODIFIER_DISABLE	0x80

#define BUTTON_L			1
#define BUTTON_M			2
#define BUTTON_R			3
#define BUTTON_X1			4
#define BUTTON_X2			5
#define MOVE_UP				11
#define MOVE_DOWN			12
#define MOVE_LEFT			13
#define MOVE_RIGHT			14
#define MOVE_UPLEFT			15
#define MOVE_UPRIGHT		16
#define MOVE_DOWNLEFT		17
#define MOVE_DOWNRIGHT		18
#define WHEEL_UP			21
#define WHEEL_DOWN			22
#define WHEEL_LEFT			23
#define WHEEL_RIGHT			24
#define CORNER_TOP_A		31
#define CORNER_BOTTOM_A		32
#define CORNER_LEFT_A		33
#define CORNER_RIGHT_A		34
#define CORNER_TOPLEFT		35
#define CORNER_TOPRIGHT		36
#define CORNER_BOTTOMLEFT	37
#define CORNER_BOTTOMRIGHT	38
#define CORNER_TOP_B		39
#define CORNER_BOTTOM_B		40
#define CORNER_LEFT_B		41
#define CORNER_RIGHT_B		42
#define CORNER_TOP_C		43
#define CORNER_BOTTOM_C		44
#define CORNER_LEFT_C		45
#define CORNER_RIGHT_C		46
#define CORNER_FINISH		47

#define S_MOVE_UP			0
#define S_MOVE_RIGHT		1
#define S_MOVE_DOWN			2
#define S_MOVE_LEFT			3
