#include <Windows.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <strsafe.h>
#include "resource.h"

#pragma region Mehtodes

#pragma warning(disable:4996)

//http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
char *str_replace(char *orig, char *rep, char *with) {
	char *result; // the return string
	char *ins;    // the next insert point
	char *tmp;    // varies
	int len_rep;  // length of rep (the string to remove)
	int len_with; // length of with (the string to replace rep with)
	int len_front; // distance between rep and end of last rep
	int count;    // number of replacements

	// sanity checks and initialization
	if (!orig || !rep)
		return NULL;
	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; // empty rep causes infinite loop during count
	if (!with)
		with = "";
	len_with = strlen(with);

	// count the number of replacements needed
	ins = orig;
	for (count = 0; tmp = strstr(ins, rep); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	// first time through the loop, all the variable are set correctly
	// from here on,
	//    tmp points to the end of the result string
	//    ins points to the next occurrence of rep in orig
	//    orig points to the remainder of orig after "end of rep"
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

char* HarmonizeString(char * RegPath)
{
	char * stringCopy = (char*)calloc(1, strlen(RegPath) + 1);

	char *token = NULL;        // containt the first part of the key
	char *next_token = NULL;   // the last part
	char *root = NULL;         // the new root key. 


	StringCchCopyA(stringCopy, strlen(RegPath), RegPath); // Keep a copy.
	token = strtok_s(stringCopy, "\\", &next_token);

	if (NULL == token)
		return NULL;

	if (strcmp(token, "HKCR") == 0)
		root = "HKEY_CLASSES_ROOT";
	else if (strcmp(token, "HKCU") == 0)
		root = "HKEY_CURRENT_USER";
	else if (strcmp(token, "HKLM") == 0)
		root = "HKEY_LOCAL_MACHINE";
	else if (strcmp(token, "HKU") == 0)
		root = "HKEY_USERS";
	else if (strcmp(token, "HKCC") == 0)
		root = "HKEY_CURRENT_CONFIG";
	else
		root = token;

	delete stringCopy;

	return str_replace(RegPath, token, root);
}


/*
https://msdn.microsoft.com/en-us/library/aa376389(v=vs.85).aspx
Routine Description: This routine returns TRUE if the caller's
process is a member of the Administrators local group. Caller is NOT
expected to be impersonating anyone and is expected to be able to
open its own process and process token.
Arguments: None.
Return Value:
TRUE - Caller has Administrators local group.
FALSE - Caller does not have Administrators local group. --
*/

bool IsUserAdmin()
{
	BOOL b;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup);
	if (b)
	{
		if (!CheckTokenMembership(NULL, AdministratorsGroup, &b))
		{
			b = FALSE;
		}
		FreeSid(AdministratorsGroup);
	}

	return(b);
}

BOOL CenterWindow(HWND hwndWindow)
{
	HWND hwndParent;
	RECT rectWindow, rectParent;

	// make the window relative to its parent
	if ((hwndParent = GetDesktopWindow()) != NULL)
	{
		GetWindowRect(hwndWindow, &rectWindow);
		GetWindowRect(hwndParent, &rectParent);

		int nWidth = rectWindow.right - rectWindow.left;
		int nHeight = rectWindow.bottom - rectWindow.top;

		int nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
		int nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

		int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
		int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

		// make sure that the dialog box never moves outside of the screen
		if (nX < 0) nX = 0;
		if (nY < 0) nY = 0;
		if (nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
		if (nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;

		MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);

		return TRUE;
	}

	return FALSE;
}

//https://forum.sysinternals.com/regjump-sysinternals-way_topic5864.html
void RegeditJump(HWND hWnd, char * RegPath, char * RegValue)
{
	char *hRegPath = HarmonizeString(RegPath);

	// Open RegEdit
	HWND regeditMainHwnd = FindWindow(L"RegEdit_RegEdit", NULL);
	if (regeditMainHwnd == NULL)  {
		SHELLEXECUTEINFO info;
		memset(&info, 0, sizeof info);
		info.cbSize = sizeof info;
		info.fMask = SEE_MASK_NOCLOSEPROCESS;
		info.lpVerb = L"open";
		info.lpFile = L"regedit.exe";
		info.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&info);
		WaitForInputIdle(info.hProcess, INFINITE);
		regeditMainHwnd = FindWindow(L"RegEdit_RegEdit", NULL);
	}
	if (regeditMainHwnd == NULL) {
		MessageBox(hWnd, L"Unable to launch Regedit.", L"RegJumpGUI", MB_OK | MB_ICONERROR);
		return;
	}
	ShowWindow(regeditMainHwnd, SW_SHOW);
	SetForegroundWindow(regeditMainHwnd);

	// Get treeview
	HWND regeditHwnd = FindWindowEx(regeditMainHwnd, NULL, L"SysTreeView32", NULL);
	SetForegroundWindow(regeditHwnd);
	SetFocus(regeditHwnd);


	DWORD  ProcessID = 0;
	GetWindowThreadProcessId(regeditHwnd, &ProcessID);
	HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, ProcessID);

	// Close it up
	for (int pos = 0; pos < 30; ++pos)  {
		SendMessage(regeditHwnd, WM_KEYDOWN, VK_LEFT, 0);
	}

	SendMessage(regeditHwnd, WM_KEYDOWN, VK_RIGHT, 0);

	// wait for slow displays
	WaitForInputIdle(hProcess, INFINITE);

	// Open path
	for (CHAR * ch = hRegPath; *ch; ++ch)  {
		if (*ch == '\\')  {

			UINT vk = VK_RIGHT;
			SendMessage(regeditHwnd, WM_KEYDOWN, vk, 0);

			// wait for slow displays  
			WaitForInputIdle(hProcess, INFINITE);

		}
		else {

			UINT vk = toupper(*ch);
			SendMessage(regeditHwnd, WM_CHAR, vk, 0);
		}
	}

	if (RegValue != NULL)
	{
		// Get listview
		regeditHwnd = FindWindowEx(regeditMainHwnd, NULL, L"SysListView32", NULL);
		SetForegroundWindow(regeditHwnd);
		SetFocus(regeditHwnd);

		// wait for slow displays  
		WaitForInputIdle(hProcess, INFINITE);

		for (CHAR * ch = RegValue; *ch; ++ch) {
			UINT vk = toupper(*ch);
			SendMessage(regeditHwnd, WM_CHAR, vk, 0);
		}
	}

	SetForegroundWindow(regeditMainHwnd);
	SetFocus(regeditMainHwnd);

	CloseHandle(hProcess);
}


char* GetClipboardText()
{
	// Try opening the clipboard
	if (!OpenClipboard(nullptr))
		return NULL;

	// Get handle of clipboard object for ANSI text
	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == nullptr)
		return NULL;

	// Lock the handle to get the actual text pointer
	char * pszText = static_cast<char*>(GlobalLock(hData));
	if (pszText == nullptr)
		return NULL;

	// Release the lock
	GlobalUnlock(hData);

	// Release the clipboard
	CloseClipboard();

	return pszText;
}

void SetIcon(HWND hDlg)
{
	HICON hicon = LoadIcon(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_ICON1));
	SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hicon);

}

void DisplayText(HWND hDlg)
{
	char*  text = GetClipboardText();
	if (text != NULL)
	{
		const char *res = strstr(text, "HKEY");
		if (res != NULL)
		{
			HWND edit = GetDlgItem(hDlg, IDC_EDITJUMP);
			SetWindowTextA(edit, text);

			HWND jump = GetDlgItem(hDlg, IDC_JUMP);
			SetFocus(jump);
		}


	}


}

void ChangeStatusButton(HWND hDlg)
{
	HWND edit = GetDlgItem(hDlg, IDC_EDITJUMP);
	HWND jump = GetDlgItem(hDlg, IDC_JUMP);

	int length = GetWindowTextLength(edit);

	EnableWindow(jump, length != 0);

}
#pragma endregion


#pragma region Events
void onCancel(HWND hDlg)
{
	SendMessage(hDlg, WM_CLOSE, 0, 0);
}

void onJump(HWND hDlg)
{
	char buff[1024];

	HWND edit = GetDlgItem(hDlg, IDC_EDITJUMP);
	GetWindowTextA(edit, buff, 1024);

	RegeditJump(hDlg, buff, NULL);
}
#pragma endregion