#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include "resource.h"

bool IsUserAdmin();
BOOL CenterWindow(HWND hwndWindow);
void RegeditJump( HWND hWnd, char * RegPath, char * RegValue);
char* GetClipboardText();
void SetIcon(HWND hDlg);
void DisplayText(HWND hDlg);
void ChangeStatusButton(HWND hDlg);


void onCancel(HWND hDlg);
void onClose(HWND hDlg);
void onJump(HWND hDlg);
