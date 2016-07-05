#include "stdafx.h"

#define STATUS_TITLE_MAX 64
#define STATUS_DESC_MAX  250

static std::vector<int> xstatusIconsValid;
static std::map<int, int> xstatusIcons;

void CSteamProto::DownloadGameIcon(int gameId)
{
	// download icon from:  http://cdn.akamai.steamstatic.com/steamcommunity/public/images/apps/%appid%/%icon%.ico
							https://steamcdn-a.akamaihd.net/steamcommunity/public/images/apps/%appid%/%icon%.ico


	/* There are few ways how to get icon hash from:
	1) https://github.com/DoctorMcKay/steam-pics-api via https://steampics-mckay.rhcloud.com/info?apps=%appid%&prettyprint=0
	then parse "clienticon": "1a8d50f6078b5d023582ea1793b0e53813d57b7f",
	and use in http://cdn.akamai.steamstatic.com/steamcommunity/public/images/apps/%appid%/%icon%.ico to download icon

	2) get clienticon hash from https://steamdb.info/app/%appid%/info/
	and parse from
	<td class="span3">clienticon</td>
	<td><a href="https://steamcdn-a.akamaihd.net/steamcommunity/public/images/apps/550/1a8d50f6078b5d023582ea1793b0e53813d57b7f.ico" rel="nofollow">1a8d50f6078b5d023582ea1793b0e53813d57b7f</a></td>
	*/


	PushRequest(
		new GetAvatarRequest(avatarUrl),
		&CSteamProto::OnGotAvatar,
		(void*)pai->hContact);


	if ()
}

int RegisterGameIcon(int status)
{
	


	// icons
	wchar_t filePath[MAX_PATH];
	GetModuleFileName(g_hInstance, filePath, MAX_PATH);

	wchar_t sectionName[100];
	mir_sntprintf(sectionName, _T("%s/%s"), LPGENT("Protocols"), LPGENT(MODULE));

	char settingName[100];
	mir_snprintf(settingName, "%s_%s", MODULE, "main");

	SKINICONDESC sid = { 0 };
	sid.flags = SIDF_ALL_TCHAR;
	sid.defaultFile.t = filePath;
	sid.pszName = settingName;
	sid.section.t = sectionName;
	sid.description.t = LPGENT("Protocol icon");
	sid.iDefaultIndex = -IDI_STEAM;
	IcoLib_AddIcon(&sid);

	mir_snprintf(settingName, "%s_%s", MODULE, "gaming");
	sid.description.t = LPGENT("Gaming icon");
	sid.iDefaultIndex = -IDI_GAMING;
	IcoLib_AddIcon(&sid);
}

int OnReloadIcons(WPARAM, LPARAM)
{
	xstatusIconsValid.clear();
	return 0;
}

int CSteamProto::GetContactXStatus(MCONTACT hContact)
{
	return getDword(hContact, "XStatusId", 0);
}

void SetContactExtraIcon(MCONTACT hContact, int status)
{
	HANDLE hImage = NULL;

	if (status > 0) {
		

		char iconName[100];
		mir_snprintf(iconName, "%s_%d", "SteamGames", status);
		hImage = IcoLib_GetIconHandle(iconName);

		if (hImage == NULL) {
			// We don't have icon for this game, use generic icon
			mir_snprintf(iconName, "%s_%s", MODULE, "gaming");
			hImage = IcoLib_GetIconHandle(iconName);
		}
	}

	ExtraIcon_SetIcon(hExtraXStatus, hContact, hImage);
}

INT_PTR CSteamProto::OnGetXStatusEx(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = (MCONTACT)wParam;

	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;
	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1;

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = GetContactXStatus(hContact);

	// fill status name member
	if (pData->flags & CSSF_MASK_NAME)
	{
		int status = (pData->wParam == NULL) ? GetContactXStatus(hContact) : *pData->wParam;
		if (status < 1)
			return 1;

		ptrT title;
		if (pData->flags & CSSF_DEFAULT_NAME)
			title = mir_tstrdup(TranslateT("Playing"));
		else
			title = getTStringA(hContact, "XStatusName");

		if (pData->flags & CSSF_UNICODE)
			mir_tstrncpy(pData->ptszName, title, STATUS_TITLE_MAX);
		else
			mir_strncpy(pData->pszName, _T2A(title), STATUS_TITLE_MAX);
	}

	// fill status message member
	if (pData->flags & CSSF_MASK_MESSAGE) {
		ptrT message(getTStringA(hContact, "XStatusMsg"));
		
		if (pData->flags & CSSF_UNICODE)
			mir_tstrncpy(pData->ptszMessage, message, STATUS_DESC_MAX);
		else
			mir_strncpy(pData->pszMessage, _T2A(message), STATUS_DESC_MAX);
	}

	// disable menu
	if (pData->flags & CSSF_DISABLE_MENU)
		if (pData->wParam)
			*pData->wParam = true;

	// disable ui
	if (pData->flags & CSSF_DISABLE_UI)
		if (pData->wParam)
			*pData->wParam = true;

	// number of xstatuses
	if (pData->flags & CSSF_STATUSES_COUNT)
		if (pData->wParam)
			*pData->wParam = 1; // TODO: how to solve unknown count of games?

	// data sizes
	if (pData->flags & CSSF_STR_SIZES) {
		if (pData->wParam) *pData->wParam = STATUS_TITLE_MAX;
		if (pData->lParam) *pData->lParam = STATUS_DESC_MAX;
	}

	return 0;
}

HICON CSteamProto::GetXStatusIcon(int status, UINT flags)
{
	if (status < 1)
		return 0;

	char iconName[100];
	mir_snprintf(iconName, "%s_%s", MODULE, "gaming");

	HICON icon = IcoLib_GetIcon(iconName, (flags & LR_BIGICON) != 0);
	return (flags & LR_SHARED) ? icon : CopyIcon(icon);
}

INT_PTR CSteamProto::OnGetXStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if (!wParam)
		wParam = GetContactXStatus(NULL);

	if (wParam < 1)
		return 0;

	return (INT_PTR)GetXStatusIcon(wParam, lParam);
}

INT_PTR CSteamProto::OnRequestAdvStatusIconIdx(WPARAM wParam, LPARAM)
{
	int status = GetContactXStatus(wParam);
	if (status)
	{
		if (std::find(xstatusIconsValid.begin(), xstatusIconsValid.end(), status) == xstatusIconsValid.end())
		{
			// adding/updating icon
			HIMAGELIST clistImageList = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
			if (clistImageList)
			{
				HICON hXStatusIcon = GetXStatusIcon(status, LR_SHARED);

				std::map<int, int>::iterator it = xstatusIcons.find(status);
				if (it != xstatusIcons.end() && it->second > 0)
					ImageList_ReplaceIcon(clistImageList, it->first, hXStatusIcon);
				else
					xstatusIcons.insert(std::make_pair(status, ImageList_AddIcon(clistImageList, hXStatusIcon)));

				// mark icon index in the array as valid
				xstatusIconsValid.push_back(status);

				IcoLib_ReleaseIcon(hXStatusIcon);
			}
		}

		if (std::find(xstatusIconsValid.begin(), xstatusIconsValid.end(), status) != xstatusIconsValid.end())
		{
			std::map<int, int>::iterator it = xstatusIcons.find(status);
			if (it != xstatusIcons.end())
				return ((INT_PTR) it->second & 0xFFFF) << 16;
		}
	}

	return -1;
}