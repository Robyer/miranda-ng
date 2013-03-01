/* Copyright (C) Miklashevsky Roman, sss, elzor
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "headers.h"



BOOL gbDosServiceExist = 0;
BOOL gbVarsServiceExist = 0;

DWORD gbMaxQuestCount = 5;
BOOL gbInfTalkProtection = 0;
BOOL gbAddPermanent = 0;
BOOL gbHandleAuthReq = 1;
BOOL gbSpecialGroup = 0;
BOOL gbHideContacts = 1;
BOOL gbIgnoreContacts = 0;
BOOL gbExclude = 1;
BOOL gbDelExcluded = 0;
BOOL gbDelAllTempory = 0;
BOOL gbHistoryLog = 0;
BOOL gbDosServiceIntegration = 0;
BOOL gbCaseInsensitive = 0;
BOOL gbRegexMatch = 0;
BOOL gbInvisDisable = 0;
BOOL gbIgnoreURL = 1;
BOOL gbLogToFile=0;
BOOL gbAutoAuth=0;
BOOL gbAutoAddToServerList=0;
BOOL gbAutoReqAuth=1;
BOOL gbMathExpression = 0;

HANDLE hStopSpamLogDirH=0;

tstring gbSpammersGroup = _T("Spammers");
tstring gbAutoAuthGroup	= _T("NotSpammers");

tstring gbQuestion;
tstring gbAnswer;
tstring gbCongratulation;
std::wstring gbAuthRepl;
extern char * pluginDescription;
extern TCHAR const * defQuestion;
extern int RemoveTmp(WPARAM,LPARAM);



/////////////////////////////////////////////////////////////////////////////////////////
// returns plugin's extended information


PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	pluginName" mod",
	PLUGIN_MAKE_VERSION(0, 0, 2, 0),
	pluginDescription,
	"Roman Miklashevsky, sss, Elzor",
	"sss123next@list.ru",
	"� 2004-2012 Roman Miklashevsky, A. Petkevich, Kosh&chka, sss, Elzor",
	"http://sss.chaoslab.ru/tracker/mim_plugs/",
	UNICODE_AWARE,
	// {553811EE-DEB6-48b8-8902-A8A00C1FD679}
	{ 0x553811ee, 0xdeb6, 0x48b8, { 0x89, 0x2, 0xa8, 0xa0, 0xc, 0x1f, 0xd6, 0x79 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern tstring DBGetContactSettingStringPAN(HANDLE hContact, char const * szModule, char const * szSetting, tstring errorValue);

void InitVars()
{
	gbDosServiceIntegration = db_get_b(NULL, pluginName, "DOSIntegration", 0);
	gbSpammersGroup = DBGetContactSettingStringPAN(NULL, pluginName, "SpammersGroup", _T("Spammers"));
	gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", _T("nospam"));
	gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", _T("Congratulations! You just passed human/robot test. Now you can write me a message."));
	gbInfTalkProtection = db_get_b(NULL, pluginName, "infTalkProtection", 0);
	gbAddPermanent = db_get_b(NULL, pluginName, "addPermanent", 0);
	gbMaxQuestCount = db_get_dw(NULL, pluginName, "maxQuestCount", 5);
	gbHandleAuthReq = db_get_b(NULL, pluginName, "handleAuthReq", 1);
	gbQuestion = DBGetContactSettingStringPAN(NULL, pluginName, "question", defQuestion);
	gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", _T("nospam"));
	gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", _T("Congratulations! You just passed human/robot test. Now you can write me a message."));
	gbAuthRepl = DBGetContactSettingStringPAN(NULL, pluginName, "authrepl", _T("StopSpam: send a message and reply to a anti-spam bot question."));
	gbSpecialGroup = db_get_b(NULL, pluginName, "SpecialGroup", 0);
	gbHideContacts = db_get_b(NULL, pluginName, "HideContacts", 0);
	gbIgnoreContacts = db_get_b(NULL, pluginName, "IgnoreContacts", 0);
	gbExclude = db_get_b(NULL, pluginName, "ExcludeContacts", 1);
	gbDelExcluded = db_get_b(NULL, pluginName, "DelExcluded", 0);
	gbDelAllTempory = db_get_b(NULL, pluginName, "DelAllTempory", 0);
	gbCaseInsensitive = db_get_b(NULL, pluginName, "CaseInsensitive", 0);
	gbRegexMatch = db_get_b(NULL, pluginName, "RegexMatch", 0);
	gbInvisDisable = db_get_b(NULL, pluginName, "DisableInInvis", 0);
	gbIgnoreURL = db_get_b(NULL, pluginName, "IgnoreURL", 0);
	gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL, pluginName, "AutoAuthGroup", _T("Not Spammers"));
	gbAutoAuth = db_get_b(NULL, pluginName, "AutoAuth", 0);
	gbAutoAddToServerList = db_get_b(NULL, pluginName, "AutoAddToServerList", 0);
	gbAutoReqAuth = db_get_b(NULL, pluginName, "AutoReqAuth", 0);
	gbLogToFile = db_get_b(NULL, pluginName, "LogSpamToFile", 0);
	gbHistoryLog = db_get_b(NULL, pluginName, "HistoryLog", 0);
	gbMathExpression = db_get_b(NULL, pluginName, "MathExpression", 0);

}

static int OnSystemModulesLoaded(WPARAM wParam,LPARAM lParam)
{
/*	if (ServiceExists(MS_DOS_SERVICE))
		gbDosServiceExist = TRUE; */
	if (ServiceExists(MS_VARS_FORMATSTRING))
		gbVarsServiceExist = TRUE;
	InitVars();
	void CleanThread();
	if(gbDelAllTempory || gbDelExcluded)
		boost::thread *thr = new boost::thread(&CleanThread);
	
	// Folders plugin support
	hStopSpamLogDirH = FoldersRegisterCustomPath("StopSpam", "StopSpam Logs", PROFILE_PATH "\\" CURRENT_PROFILE "\\StopSpamLog");
	return 0;
}

HANDLE hEventFilter = 0, hOptInitialise = 0, hSettingChanged = 0;


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	/*if(DLL_PROCESS_ATTACH == fdwReason)
		hInst=hinstDLL;
	return TRUE;*/
	hInst = hinstDLL;
	return TRUE;
}


int hLangpack = 0;

extern "C" int __declspec(dllexport) Load()
{
	mir_getLP(&pluginInfoEx);

	CreateServiceFunction("/RemoveTmp", (MIRANDASERVICE)RemoveTmp);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnSystemModulesLoaded);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position=-0x7FFFFFFF;
	mi.flags=0;
	mi.hIcon=LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	mi.pszName=LPGEN("Remove Temporary Contacts");
	mi.pszService="/RemoveTmp";

	Menu_AddMainMenuItem(&mi);

	miranda::EventHooker::HookAll();
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	miranda::EventHooker::UnhookAll();
	return 0;
}
