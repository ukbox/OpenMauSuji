
#include <windows.h>
#include "../../../plugin.h"
#include "resource.h"

BOOL Init(void);
void Quit(void);
void Config(PREFS* prefs, CONFIGPARAM* configparam);
void Command(COMMANDPARAM* param);
void Description(COMMANDARGV* argv, LPWSTR lpszText, int nLength);

PLUGIN plugin = {
	PLUGIN_VERSION,
	0x80000000,
	L"sample plugin",
	NULL,
	NULL,
	Init,
	Quit,
	Config,
	Command,
	Description
};

EXTERN_C PLUGIN * WINAPI GetPlugin(void)
{
	return &plugin;
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static CONFIGPARAM* configparam;

	switch(msg)
	{
	case WM_INITDIALOG:
		configparam = (CONFIGPARAM*)lParam;
		if(!configparam->bInitialized)
		{
			configparam->argv.nValue[0] = 1;
			configparam->argv.nValue[1] = 2;
			configparam->argv.nValue[2] = 3;
			configparam->argv.nValue[3] = 4;
			lstrcpy(configparam->argv.szText[0], TEXT("sample1"));
			lstrcpy(configparam->argv.szText[1], TEXT("sample2"));
			lstrcpy(configparam->argv.szText[2], TEXT("sample3"));
			lstrcpy(configparam->argv.szText[3], TEXT("sample4"));
		}
		SetDlgItemInt(hwnd, IDC_EDIT1, configparam->argv.nValue[0], TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT2, configparam->argv.nValue[1], TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT3, configparam->argv.nValue[2], TRUE);
		SetDlgItemInt(hwnd, IDC_EDIT4, configparam->argv.nValue[3], TRUE);
		SetDlgItemText(hwnd, IDC_EDIT5, configparam->argv.szText[0]);
		SetDlgItemText(hwnd, IDC_EDIT6, configparam->argv.szText[1]);
		SetDlgItemText(hwnd, IDC_EDIT7, configparam->argv.szText[2]);
		SetDlgItemText(hwnd, IDC_EDIT8, configparam->argv.szText[3]);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			configparam->argv.nValue[0] = GetDlgItemInt(hwnd, IDC_EDIT1, NULL, TRUE);
			configparam->argv.nValue[1] = GetDlgItemInt(hwnd, IDC_EDIT2, NULL, TRUE);
			configparam->argv.nValue[2] = GetDlgItemInt(hwnd, IDC_EDIT3, NULL, TRUE);
			configparam->argv.nValue[3] = GetDlgItemInt(hwnd, IDC_EDIT4, NULL, TRUE);
			GetDlgItemText(hwnd, IDC_EDIT5, configparam->argv.szText[0], MAX_PATH);
			GetDlgItemText(hwnd, IDC_EDIT6, configparam->argv.szText[1], MAX_PATH);
			GetDlgItemText(hwnd, IDC_EDIT7, configparam->argv.szText[2], MAX_PATH);
			GetDlgItemText(hwnd, IDC_EDIT8, configparam->argv.szText[3], MAX_PATH);
			break;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_EDIT1:
		case IDC_EDIT2:
		case IDC_EDIT3:
		case IDC_EDIT4:
		case IDC_EDIT5:
		case IDC_EDIT6:
		case IDC_EDIT7:
		case IDC_EDIT8:
			if(HIWORD(wParam)==EN_CHANGE)
				PropSheet_Changed(GetParent(hwnd), hwnd);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL Init(void)
{
	return TRUE;
}

void Quit(void)
{
}

void Config(PREFS* prefs, CONFIGPARAM* configparam)
{
	prefs->hInstance = plugin.hDllInstance;
	prefs->id = IDD_DIALOG;
	prefs->DlgProc = DlgProc;
	prefs->lParam = (LPARAM)configparam;
}

void Command(COMMANDPARAM* param)
{
	WCHAR szText[1024];

	wsprintfW(szText, L"%d%:%d:%d:%d\n%s\n%s\n%s\n%s",
		param->argv.nValue[0], param->argv.nValue[1], param->argv.nValue[2], param->argv.nValue[3],
		param->argv.szText[0], param->argv.szText[1], param->argv.szText[2], param->argv.szText[3]);
	MessageBoxW(param->hwnd, szText, L"sample plugin", MB_OK);
}

void Description(COMMANDARGV* argv, LPWSTR lpszText, int nLength)
{
	WCHAR szText[1024];

	wsprintfW(szText, L"%d%:%d:%d:%d %s %s %s %s",
		argv->nValue[0], argv->nValue[1], argv->nValue[2], argv->nValue[3],
		argv->szText[0], argv->szText[1], argv->szText[2], argv->szText[3]);
	lstrcpynW(lpszText, szText, nLength);
}
