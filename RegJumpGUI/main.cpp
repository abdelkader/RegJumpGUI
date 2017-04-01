#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"
#include "Utils.h"

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg)
  {
  case WM_INITDIALOG:
	  CenterWindow(hDlg);
	  SetIcon(hDlg);
	  DisplayText(hDlg);
	  ChangeStatusButton(hDlg);
	  break;

  case WM_COMMAND:
    switch(LOWORD(wParam))
    {
    case IDC_CLOSE:  onCancel(hDlg); return TRUE;
	case IDC_JUMP:   onJump(hDlg);   return TRUE;
	case IDC_EDITJUMP:
		if (HIWORD(wParam) == EN_CHANGE)
		{
			ChangeStatusButton(hDlg);
		}

		break;
    }
    break;

  case WM_CLOSE: DestroyWindow(hDlg); return TRUE;
  case WM_DESTROY: PostQuitMessage(0); return TRUE;
  }

  return FALSE;
}

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE h0, LPTSTR lpCmdLine, int nCmdShow)
{
  MSG msg;
  BOOL ret;

  InitCommonControls();
  HWND hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
  
  ShowWindow(hDlg, nCmdShow);

  while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
    if(ret == -1)
      return -1;

    if(!IsDialogMessage(hDlg, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return 0;
}