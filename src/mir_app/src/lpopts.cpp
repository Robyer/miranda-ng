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
#include "langpack.h"

static void SetDlgItemText_CP(HWND hwndDlg, int ctrlID, LPCSTR str)
{
	SetDlgItemText(hwndDlg, ctrlID, ptrW(mir_utf8decodeW(str)));
}

static void DisplayPackInfo(HWND hwndDlg, const LANGPACK_INFO *pack)
{
	/* locale string */
	if (!(pack->flags & LPF_NOLOCALE)) {
		wchar_t szLocaleName[256], szLanguageName[128], szContryName[128];

		if (!GetLocaleInfo(pack->Locale, WINVER >= _WIN32_WINNT_WIN7 ? LOCALE_SENGLISHLANGUAGENAME : LOCALE_SENGLANGUAGE, szLanguageName, _countof(szLanguageName)))
			szLanguageName[0] = '\0';
		if (!GetLocaleInfo(pack->Locale, WINVER >= _WIN32_WINNT_WIN7 ? LOCALE_SENGLISHCOUNTRYNAME : LOCALE_SENGCOUNTRY, szContryName, _countof(szContryName)))
			szContryName[0] = '\0';
		
		/* add some note if its incompatible */
		if (szLanguageName[0] && szContryName[0]) {
			mir_snwprintf(szLocaleName, L"%s (%s)", TranslateW(szLanguageName), TranslateW(szContryName));
			if (!IsValidLocale(pack->Locale, LCID_INSTALLED)) {
				wchar_t *pszIncompat;
				pszIncompat = TranslateT("(incompatible)");
				szLocaleName[_countof(szLocaleName) - mir_wstrlen(pszIncompat) - 1] = 0;
				mir_wstrcat(mir_wstrcat(szLocaleName, L" "), pszIncompat);
			}
			SetDlgItemText(hwndDlg, IDC_LANGLOCALE, szLocaleName);
		}
		else SetDlgItemText(hwndDlg, IDC_LANGLOCALE, TranslateT("Unknown"));
	}
	else SetDlgItemText(hwndDlg, IDC_LANGLOCALE, TranslateT("Unknown"));
	
	/* file date */
	SYSTEMTIME stFileDate;
	wchar_t szDate[128]; szDate[0] = 0;
	if (FileTimeToSystemTime(&pack->ftFileDate, &stFileDate))
		GetDateFormat(Langpack_GetDefaultLocale(), DATE_SHORTDATE, &stFileDate, NULL, szDate, _countof(szDate));
	SetDlgItemText(hwndDlg, IDC_LANGDATE, szDate);
	
	/* general */
	SetDlgItemText_CP(hwndDlg, IDC_LANGMODUSING, pack->szLastModifiedUsing);
	SetDlgItemText_CP(hwndDlg, IDC_LANGAUTHORS, pack->szAuthors);
	SetDlgItemText_CP(hwndDlg, IDC_LANGEMAIL, pack->szAuthorEmail);
	SetDlgItemText(hwndDlg, IDC_LANGINFOFRAME, TranslateW(pack->tszLanguage));
}

static BOOL InsertPackItemEnumProc(LANGPACK_INFO *pack, WPARAM wParam, LPARAM)
{
	LANGPACK_INFO *pack2 = new LANGPACK_INFO();
	*pack2 = *pack;

	/* insert */
	wchar_t tszName[512];
	mir_snwprintf(tszName, L"%s [%s]",
		TranslateW(pack->tszLanguage),
		pack->flags & LPF_DEFAULT ? TranslateT("built-in") : pack->tszFileName);
	UINT message = pack->flags & LPF_DEFAULT ? CB_INSERTSTRING : CB_ADDSTRING;
	int idx = SendMessage((HWND)wParam, message, 0, (LPARAM)tszName);
	SendMessage((HWND)wParam, CB_SETITEMDATA, idx, (LPARAM)pack2);
	if (pack->flags & LPF_ENABLED) {
		SendMessage((HWND)wParam, CB_SETCURSEL, idx, 0);
		DisplayPackInfo(GetParent((HWND)wParam), pack);
		EnableWindow(GetDlgItem(GetParent((HWND)wParam), IDC_RELOAD), !(pack->flags & LPF_DEFAULT));
	}

	return TRUE;
}

static void CALLBACK OpenOptions(void*)
{
	OPENOPTIONSDIALOG ood = { sizeof(ood) };
	ood.pszGroup = "Customize";
	ood.pszPage = "Languages";
	Options_Open(&ood);
}

static void ReloadOptions(void *hWnd)
{
	while (IsWindow((HWND)hWnd))
		Sleep(50);

	CallFunctionAsync(OpenOptions, 0);
}

INT_PTR CALLBACK DlgLangpackOpt(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LANGUAGES);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		ComboBox_ResetContent(hwndList);
		EnumLangpacks(InsertPackItemEnumProc, (WPARAM)hwndList, (LPARAM)0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LANGEMAIL:
			{
				char buf[512];
				mir_strcpy(buf, "mailto:");
				if (GetDlgItemTextA(hwndDlg, LOWORD(wParam), &buf[7], _countof(buf) - 7))
					Utils_OpenUrl(buf);
			}
			break;

		case IDC_MORELANG:
			Utils_OpenUrl("http://wiki.miranda-ng.org/index.php?title=Langpacks/en#Download");
			break;

		case IDC_LANGUAGES:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int idx = ComboBox_GetCurSel(hwndList);
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, idx);
				DisplayPackInfo(hwndDlg, pack);
				if (!(pack->flags & LPF_ENABLED))
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RELOAD), (pack->flags & LPF_ENABLED) && !(pack->flags & LPF_DEFAULT));
			}
			break;

		case IDC_RELOAD:
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_RELOAD), FALSE);
				int idx = ComboBox_GetCurSel(hwndList);
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, idx);
				ReloadLangpack(pack->tszFullPath);
				DisplayPackInfo(hwndDlg, pack);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RELOAD), TRUE);
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (LPNMHDR(lParam)->code == PSN_APPLY) {
			wchar_t tszPath[MAX_PATH]; tszPath[0] = 0;
			int idx = ComboBox_GetCurSel(hwndList);
			int count = ComboBox_GetCount(hwndList);
			for (int i = 0; i < count; i++) {
				LANGPACK_INFO *pack = (LANGPACK_INFO*)ComboBox_GetItemData(hwndList, i);
				if (i == idx) {
					db_set_ws(NULL, "Langpack", "Current", pack->tszFileName);
					mir_wstrcpy(tszPath, pack->tszFullPath);
					pack->flags |= LPF_ENABLED;
				}
				else pack->flags &= ~LPF_ENABLED;
			}

			if (tszPath[0]) {
				ReloadLangpack(tszPath);

				if (LPPSHNOTIFY(lParam)->lParam == IDC_APPLY) {
					HWND hwndParent = GetParent(hwndDlg);
					PostMessage(hwndParent, WM_CLOSE, 1, 0);
					mir_forkthread(ReloadOptions, hwndParent);
				}
			}
		}
		break;

	case WM_DESTROY:
		int count = ListBox_GetCount(hwndList);
		for (int i = 0; i < count; i++)
			delete (LANGPACK_INFO*)ListBox_GetItemData(hwndList, i);
		ComboBox_ResetContent(hwndList);
		return TRUE;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LangpackOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = g_hInst;
	odp.pfnDlgProc = DlgLangpackOpt;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_LANGUAGES);
	odp.position = -1300000000;
	odp.pszTitle = LPGEN("Languages");
	odp.pszGroup = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}
