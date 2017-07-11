// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project1.h"

#include "GuiMain.h"

// Global Variables:
class CGuiMain GuiMain;

// Forward declarations of functions included in this code module:
INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgProc(
	HWND hDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		// GUI初期化
		GuiMain.Initialize(hDlg);
		break;

	default:
		// メッセージ分配
		if ( GuiMain.DispatchMessage(uMsg, wParam, lParam))
		{	// 処理した
			break;
		}
		return 1;
	}

	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

	DialogBox(hInstance,
		MAKEINTRESOURCE(IDD_DIALOG1),
		NULL, (DLGPROC)DlgProc);
	return TRUE;
}


