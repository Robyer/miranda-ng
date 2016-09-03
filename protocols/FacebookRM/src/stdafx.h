/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2009-11 Michal Zelinka, 2011-16 Robert P�sel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#pragma warning(disable:4996)

#define _CRT_RAND_S

#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <algorithm>

#include <windows.h>
#include <time.h>

#include <win2k.h>
#include <newpluginapi.h>
#include <m_avatars.h>
#include <m_chat.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_idle.h>
#include <m_ignore.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_icolib.h>
#include <m_hotkeys.h>
#include <m_folders.h>
#include <m_smileyadd.h>
#include <m_toptoolbar.h>
#include <m_json.h>
#include <m_imgsrvc.h>
#include <m_http.h>
#include <m_messagestate.h>
#include <m_gui.h>

class FacebookProto;

#include "../../utils/std_string_utils.h"

#include "constants.h"
#include "entities.h"
#include "http.h"
#include "http_request.h"
#include "list.hpp"
#include "client.h"
#include "proto.h"
#include "json.h"
#include "db.h"
#include "dialogs.h"
#include "theme.h"
#include "resource.h"
#include "version.h"

#include "requests/channel.h"
#include "requests/login.h"
#include "requests/notifications.h"
#include "requests/profile.h"
#include "requests/search.h"

extern HINSTANCE g_hInstance;
extern std::string g_strUserAgent;
extern DWORD g_mirandaVersion;

class ScopedLock
{
public:
	ScopedLock(HANDLE h, int t = INFINITE) : handle_(h), timeout_(t)
	{
		WaitForSingleObject(handle_,timeout_);
	}
	~ScopedLock()
	{
		if(handle_)
			ReleaseMutex(handle_);
	}
	void Unlock()
	{
		ReleaseMutex(handle_);
		handle_ = 0;
	}
private:
	HANDLE handle_;
	int timeout_;
};
