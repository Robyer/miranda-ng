/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (�) 2012-16 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "file.h"

#define MAX_MRU_DIRS    5

static BOOL CALLBACK ClipSiblingsChildEnumProc(HWND hwnd, LPARAM)
{
	SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE)|WS_CLIPSIBLINGS);
	return TRUE;
}

static void GetLowestExistingDirName(const wchar_t *szTestDir, wchar_t *szExistingDir, int cchExistingDir)
{
	DWORD dwAttributes;
	wchar_t *pszLastBackslash;

	mir_wstrncpy(szExistingDir, szTestDir, cchExistingDir);
	while ((dwAttributes = GetFileAttributes(szExistingDir)) != INVALID_FILE_ATTRIBUTES && !(dwAttributes&FILE_ATTRIBUTE_DIRECTORY)) {
		pszLastBackslash = wcsrchr(szExistingDir, '\\');
		if (pszLastBackslash == NULL) { *szExistingDir = '\0'; break; }
		*pszLastBackslash = '\0';
	}
	if (szExistingDir[0] == '\0')
		GetCurrentDirectory(cchExistingDir, szExistingDir);
}

static const wchar_t InvalidFilenameChars[] = L"\\/:*?\"<>|";
void RemoveInvalidFilenameChars(wchar_t *tszString)
{
	size_t i;
	if (tszString) {
		for (i = wcscspn(tszString, InvalidFilenameChars); tszString[i]; i+=wcscspn(tszString+i+1, InvalidFilenameChars)+1)
			tszString[i] = '_';
	}
}

static const wchar_t InvalidPathChars[] = L"*?\"<>|"; // "\/:" are excluded as they are allowed in file path
void RemoveInvalidPathChars(wchar_t *tszString)
{
	if (tszString)
		for (size_t i = wcscspn(tszString, InvalidPathChars); tszString[i]; i += wcscspn(tszString + i + 1, InvalidPathChars) + 1)
			tszString[i] = '_';
}

static INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData)
{
	wchar_t szDir[MAX_PATH];
	switch (uMsg) {
	case BFFM_INITIALIZED:
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, pData);
		break;
	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList((LPITEMIDLIST)lp, szDir))
			SendMessage(hwnd, BFFM_SETSTATUSTEXT, 0, (LPARAM)szDir);
		break;
	}
	return 0;
}

int BrowseForFolder(HWND hwnd, wchar_t *szPath)
{
	BROWSEINFO bi = { 0 };
	bi.hwndOwner = hwnd;
	bi.pszDisplayName = szPath;
	bi.lpszTitle = TranslateT("Select folder");
	bi.ulFlags = BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)szPath;

	LPITEMIDLIST pidlResult = SHBrowseForFolder(&bi);
	if (pidlResult) {
		SHGetPathFromIDList(pidlResult, szPath);
		mir_wstrcat(szPath, L"\\");
		CoTaskMemFree(pidlResult);
	}
	return pidlResult != NULL;
}

static REPLACEVARSARRAY sttVarsToReplace[] =
{
	{ "///", "//" },
	{ "//", "/" },
	{ "()", "" },
	{ NULL, NULL }
};

static void patchDir(wchar_t *str, size_t strSize)
{
	wchar_t *result = Utils_ReplaceVarsT(str, 0, sttVarsToReplace);
	if (result) {
		wcsncpy(str, result, strSize);
		mir_free(result);
	}

	size_t len = mir_wstrlen(str);
	if (len + 1 < strSize && str[len - 1] != '\\')
		mir_wstrcpy(str + len, L"\\");
}

void GetContactReceivedFilesDir(MCONTACT hContact, wchar_t *szDir, int cchDir, BOOL patchVars)
{
	wchar_t tszTemp[MAX_PATH];

	ptrW tszRecvPath(db_get_wsa(NULL, "SRFile", "RecvFilesDirAdv"));
	if (tszRecvPath)
		wcsncpy_s(tszTemp, tszRecvPath, _TRUNCATE);
	else
		mir_snwprintf(tszTemp, L"%%mydocuments%%\\%s\\%%userid%%", TranslateT("My received files"));

	if (hContact) {
		hContact = db_mc_tryMeta(hContact);

		REPLACEVARSARRAY rvaVarsToReplace[4];
		rvaVarsToReplace[0].key.w = L"nick";
		rvaVarsToReplace[0].value.w = mir_wstrdup((wchar_t *)pcli->pfnGetContactDisplayName(hContact, 0));
		rvaVarsToReplace[1].key.w = L"userid";
		rvaVarsToReplace[1].value.w = GetContactID(hContact);
		rvaVarsToReplace[2].key.w = L"proto";
		rvaVarsToReplace[2].value.w = mir_a2u(GetContactProto(hContact));
		rvaVarsToReplace[3].key.w = NULL;
		rvaVarsToReplace[3].value.w = NULL;
		for (int i = 0; i < (_countof(rvaVarsToReplace) - 1); i++)
			RemoveInvalidFilenameChars(rvaVarsToReplace[i].value.w);

		wchar_t *result = Utils_ReplaceVarsT(tszTemp, hContact, rvaVarsToReplace);
		if (result) {
			wcsncpy(tszTemp, result, _countof(tszTemp));
			mir_free(result);
			for (int i = 0; i < (_countof(rvaVarsToReplace) - 1); i++)
				mir_free(rvaVarsToReplace[i].value.w);
		}
	}

	if (patchVars)
		patchDir(tszTemp, _countof(tszTemp));
	RemoveInvalidPathChars(tszTemp);
	mir_wstrncpy(szDir, tszTemp, cchDir);
}

void GetReceivedFilesDir(wchar_t *szDir, int cchDir)
{
	wchar_t tszTemp[MAX_PATH];

	ptrW tszRecvPath(db_get_wsa(NULL, "SRFile", "RecvFilesDirAdv"));
	if (tszRecvPath)
		wcsncpy_s(tszTemp, tszRecvPath, _TRUNCATE);
	else
		mir_snwprintf(tszTemp, L"%%mydocuments%%\\%s\\%%userid%%", TranslateT("My received files"));

	patchDir(tszTemp, _countof(tszTemp));
	RemoveInvalidPathChars(tszTemp);
	mir_wstrncpy(szDir, tszTemp, cchDir);
}

INT_PTR CALLBACK DlgProcRecvFile(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	FileDlgData *dat = (FileDlgData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			wchar_t szPath[450];
			CLISTEVENT* cle = (CLISTEVENT*)lParam;

			dat = (FileDlgData*)mir_calloc(sizeof(FileDlgData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->hContact = cle->hContact;
			dat->hDbEvent = cle->hDbEvent;
			dat->hNotifyEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_RECVEVENT);
			dat->hPreshutdownEvent = HookEventMessage(ME_SYSTEM_PRESHUTDOWN, hwndDlg, M_PRESHUTDOWN);
			dat->dwTicks = GetTickCount();

			EnumChildWindows(hwndDlg, ClipSiblingsChildEnumProc, 0);

			Window_SetSkinIcon_IcoLib(hwndDlg, SKINICON_EVENT_FILE);
			Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View user's history"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User menu"));

			wchar_t *contactName = pcli->pfnGetContactDisplayName(dat->hContact, 0);
			SetDlgItemText(hwndDlg, IDC_FROM, contactName);
			GetContactReceivedFilesDir(dat->hContact, szPath, _countof(szPath), TRUE);
			SetDlgItemText(hwndDlg, IDC_FILEDIR, szPath);
			SHAutoComplete(GetWindow(GetDlgItem(hwndDlg, IDC_FILEDIR), GW_CHILD), 1);

			for (int i = 0; i < MAX_MRU_DIRS; i++) {
				char idstr[32];
				mir_snprintf(idstr, "MruDir%d", i);

				DBVARIANT dbv;
				if (db_get_ws(NULL, "SRFile", idstr, &dbv))
					break;
				SendDlgItemMessage(hwndDlg, IDC_FILEDIR, CB_ADDSTRING, 0, (LPARAM)dbv.ptszVal);
				db_free(&dbv);
			}

			db_event_markRead(dat->hContact, dat->hDbEvent);

			DBEVENTINFO dbei = { sizeof(dbei) };
			dbei.cbBlob = db_event_getBlobSize(dat->hDbEvent);
			if (dbei.cbBlob > 4 && dbei.cbBlob <= 8196) {
				dbei.pBlob = (PBYTE)alloca(dbei.cbBlob + 1);
				db_event_get(dat->hDbEvent, &dbei);
				dbei.pBlob[dbei.cbBlob] = 0;
				dat->fs = cle->lParam ? (HANDLE)cle->lParam : (HANDLE)*(PDWORD)dbei.pBlob;

				char *str = (char*)dbei.pBlob + 4;
				ptrW ptszFileName(DbGetEventStringT(&dbei, str));
				SetDlgItemText(hwndDlg, IDC_FILENAMES, ptszFileName);

				unsigned len = (unsigned)mir_strlen(str) + 1;
				if (len + 4 < dbei.cbBlob) {
					str += len;
					ptrW pwszDescription(DbGetEventStringT(&dbei, str));
					SetDlgItemText(hwndDlg, IDC_MSG, pwszDescription);
				}
			}
			else DestroyWindow(hwndDlg);

			wchar_t datetimestr[64];
			TimeZone_PrintTimeStamp(NULL, dbei.timestamp, L"t d", datetimestr, _countof(datetimestr), 0);
			SetDlgItemText(hwndDlg, IDC_DATE, datetimestr);

			ptrW info(Contact_GetInfo(CNF_UNIQUEID, dat->hContact));
			SetDlgItemText(hwndDlg, IDC_NAME, (info) ? info : contactName);

			if (db_get_b(dat->hContact, "CList", "NotOnList", 0)) {
				RECT rcBtn1, rcBtn2, rcDateCtrl;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_ADD), &rcBtn1);
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rcBtn2);
				GetWindowRect(GetDlgItem(hwndDlg, IDC_DATE), &rcDateCtrl);
				SetWindowPos(GetDlgItem(hwndDlg, IDC_DATE), 0, 0, 0, rcDateCtrl.right - rcDateCtrl.left - (rcBtn2.left - rcBtn1.left), rcDateCtrl.bottom - rcDateCtrl.top, SWP_NOZORDER | SWP_NOMOVE);
			}
			else if (db_get_b(NULL, "SRFile", "AutoAccept", 0)) {
				//don't check auto-min here to fix BUG#647620
				PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDOK, BN_CLICKED), (LPARAM)GetDlgItem(hwndDlg, IDOK));
			}
			if (!db_get_b(dat->hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
		}
		return TRUE;

	case WM_MEASUREITEM:
		return Menu_MeasureItem((LPMEASUREITEMSTRUCT)lParam);

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_PROTOCOL)) {
				char *szProto = GetContactProto(dat->hContact);
				if (szProto) {
					HICON hIcon = (HICON)CallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
					if (hIcon) {
						DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);
						DestroyIcon(hIcon);
					}
				}
			}
		}
		return Menu_DrawItem((LPDRAWITEMSTRUCT)lParam);

	case WM_COMMAND:
		if (CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact))
			break;

		switch (LOWORD(wParam)) {
		case IDC_FILEDIRBROWSE:
			{
				wchar_t szDirName[MAX_PATH], szExistingDirName[MAX_PATH];

				GetDlgItemText(hwndDlg, IDC_FILEDIR, szDirName, _countof(szDirName));
				GetLowestExistingDirName(szDirName, szExistingDirName, _countof(szExistingDirName));
				if (BrowseForFolder(hwndDlg, szExistingDirName))
					SetDlgItemText(hwndDlg, IDC_FILEDIR, szExistingDirName);
			}
			break;

		case IDOK:
			{	//most recently used directories
				wchar_t szRecvDir[MAX_PATH], szDefaultRecvDir[MAX_PATH];
				GetDlgItemText(hwndDlg, IDC_FILEDIR, szRecvDir, _countof(szRecvDir));
				RemoveInvalidPathChars(szRecvDir);
				GetContactReceivedFilesDir(NULL, szDefaultRecvDir, _countof(szDefaultRecvDir), TRUE);
				if (wcsnicmp(szRecvDir, szDefaultRecvDir, mir_wstrlen(szDefaultRecvDir))) {
					char idstr[32];
					int i;
					DBVARIANT dbv;
					for (i = MAX_MRU_DIRS-2;i>=0;i--) {
						mir_snprintf(idstr, "MruDir%d", i);
						if (db_get_ws(NULL, "SRFile", idstr, &dbv)) continue;
						mir_snprintf(idstr, "MruDir%d", i+1);
						db_set_ws(NULL, "SRFile", idstr, dbv.ptszVal);
						db_free(&dbv);
					}
					db_set_ws(NULL, "SRFile", idstr, szRecvDir);
				}
			}
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILENAMES), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_MSG), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIR), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIRBROWSE), FALSE);

			GetDlgItemText(hwndDlg, IDC_FILEDIR, dat->szSavePath, _countof(dat->szSavePath));
			GetDlgItemText(hwndDlg, IDC_FILE, dat->szFilenames, _countof(dat->szFilenames));
			GetDlgItemText(hwndDlg, IDC_MSG, dat->szMsg, _countof(dat->szMsg));
			dat->hwndTransfer = FtMgr_AddTransfer(dat);
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			//check for auto-minimize here to fix BUG#647620
			if (db_get_b(NULL, "SRFile", "AutoAccept", 0) && db_get_b(NULL, "SRFile", "AutoMin", 0)) {
				ShowWindow(hwndDlg, SW_HIDE);
				ShowWindow(hwndDlg, SW_SHOWMINNOACTIVE);
			}
			DestroyWindow(hwndDlg);
			break;

		case IDCANCEL:
			if (dat->fs) ProtoChainSend(dat->hContact, PSS_FILEDENY, (WPARAM)dat->fs, (LPARAM)TranslateT("Canceled"));
			dat->fs = NULL; /* the protocol will free the handle */
			DestroyWindow(hwndDlg);
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hContact = dat->hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = "";
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
				if (!db_get_b(dat->hContact, "CList", "NotOnList", 0))
					ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
			}
			break;

		case IDC_USERMENU:
			{
				RECT rc;
				GetWindowRect((HWND)lParam, &rc);
				HMENU hMenu = Menu_BuildContactMenu(dat->hContact);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)dat->hContact, 0);
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)dat->hContact, 0);
			break;
		}
		break;

	case HM_RECVEVENT:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if ((ack == NULL) || (ack->hProcess != dat->fs) || (ack->type != ACKTYPE_FILE) || (ack->hContact != dat->hContact))
				break;

			if (ack->result == ACKRESULT_DENIED || ack->result == ACKRESULT_FAILED) {
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIR), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_FILEDIRBROWSE), FALSE);
				SetDlgItemText(hwndDlg, IDC_MSG, TranslateT("This file transfer has been canceled by the other side"));
				SkinPlaySound("FileDenied");
				FlashWindow(hwndDlg, TRUE);
			}
			else if (ack->result != ACKRESULT_FILERESUME)
			{
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), (LPARAM)GetDlgItem(hwndDlg, IDCANCEL));
			}
		}
		break;

	case WM_DESTROY:
		Window_FreeIcon_IcoLib(hwndDlg);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_HISTORY);
		Button_FreeIcon_IcoLib(hwndDlg, IDC_USERMENU);

		FreeFileDlgData(dat);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		break;
	}
	return FALSE;
}
