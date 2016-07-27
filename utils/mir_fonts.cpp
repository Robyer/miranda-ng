#include <windows.h>

#include <m_system.h>
#include <m_fontservice.h>
#include <win2k.h>

#include "mir_fonts.h"

int FontService_RegisterFont(const char *pszDbModule, const char *pszDbName, const wchar_t *pszSection, const wchar_t *pszDescription, const wchar_t *pszBackgroundGroup, const wchar_t *pszBackgroundName, int position, BOOL bAllowEffects, LOGFONT *plfDefault, COLORREF clrDefault)
{
	FontIDT fid = { 0 };
	fid.cbSize = sizeof(fid);
	mir_strncpy(fid.dbSettingsGroup, pszDbModule, sizeof(fid.dbSettingsGroup)); /* buffer safe */
	mir_strncpy(fid.prefix, pszDbName, sizeof(fid.prefix)); /* buffer safe */
	mir_wstrncpy(fid.group, pszSection, _countof(fid.group)); /* buffer safe */
	mir_wstrncpy(fid.name, pszDescription, _countof(fid.name)); /* buffer safe */
	mir_wstrncpy(fid.backgroundGroup, pszBackgroundGroup, _countof(fid.backgroundGroup)); /* buffer safe */
	mir_wstrncpy(fid.backgroundName, pszBackgroundName, _countof(fid.backgroundName)); /* buffer safe */
	fid.flags = FIDF_ALLOWREREGISTER;
	if (bAllowEffects) fid.flags |= FIDF_ALLOWEFFECTS;
	fid.order = position;
	if (plfDefault != NULL) {
		fid.flags |= FIDF_DEFAULTVALID;
		fid.deffontsettings.colour = clrDefault;
		fid.deffontsettings.size = (char)plfDefault->lfHeight;
		if (plfDefault->lfItalic) fid.deffontsettings.style |= DBFONTF_ITALIC;
		if (plfDefault->lfWeight != FW_NORMAL) fid.deffontsettings.style |= DBFONTF_BOLD;
		if (plfDefault->lfUnderline) fid.deffontsettings.style |= DBFONTF_UNDERLINE;
		if (plfDefault->lfStrikeOut) fid.deffontsettings.style |= DBFONTF_STRIKEOUT;
		fid.deffontsettings.charset = plfDefault->lfCharSet;
		mir_wstrncpy(fid.deffontsettings.szFace, plfDefault->lfFaceName, _countof(fid.deffontsettings.szFace)); /* buffer safe */
	}
	FontRegisterT(&fid);
	return 0;
}

int FontService_GetFont(const wchar_t *pszSection, const wchar_t *pszDescription, COLORREF *pclr, LOGFONT *plf)
{
	FontIDT fid = { 0 };
	mir_wstrncpy(fid.group, pszSection, _countof(fid.group)); /* buffer sfae */
	mir_wstrncpy(fid.name, pszDescription, _countof(fid.name)); /* buffer safe */
	*pclr = (COLORREF)CallService(MS_FONT_GETT, (WPARAM)&fid, (LPARAM)plf); /* uses fallback font on error */
	return (int)*pclr == -1;
}

int FontService_RegisterColor(const char *pszDbModule, const char *pszDbName, const wchar_t *pszSection, const wchar_t *pszDescription, COLORREF clrDefault)
{
	ColourIDT cid = { 0 };
	cid.cbSize = sizeof(cid);
	cid.defcolour = clrDefault;
	mir_strncpy(cid.dbSettingsGroup, pszDbModule, sizeof(cid.dbSettingsGroup)); /* buffer safe */
	mir_strncpy(cid.setting, pszDbName, sizeof(cid.setting)); /* buffer safe */
	mir_wstrncpy(cid.group, pszSection, _countof(cid.group)); /* buffer safe */
	mir_wstrncpy(cid.name, pszDescription, _countof(cid.name)); /* buffer safe */
	ColourRegisterT(&cid);
	return 0;
}

int FontService_GetColor(const wchar_t *pszSection, const wchar_t *pszDescription, COLORREF *pclr)
{
	ColourIDT cid = { 0 };
	cid.cbSize = sizeof(cid);
	wcsncpy_s(cid.group, pszSection, _TRUNCATE);
	wcsncpy_s(cid.name, pszDescription, _TRUNCATE);
	*pclr = (COLORREF)CallService(MS_COLOUR_GETT, (WPARAM)&cid, 0);
	return (int)*pclr == -1;
}
