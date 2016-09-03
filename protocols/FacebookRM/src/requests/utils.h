/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2011-16 Robert Pösel

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

#ifndef _FACEBOOK_REQUEST_UTILS_H_
#define _FACEBOOK_REQUEST_UTILS_H_

// refreshing captcha dialog (changing captcha type)
class RefreshCaptchaRequest : public HttpRequest
{
public:
	RefreshCaptchaRequest(facebook_client *fc, const char *captchaPersistData) :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/captcha/refresh_ajax.php")
	{
		Url
			<< "__a=1"
			<< "new_captcha_type=TFBCaptcha"
			<< CHAR_VALUE("skipped_captcha_data", captchaPersistData)
			<< CHAR_VALUE("__dyn", fc->__dyn())
			<< CHAR_VALUE("__req", fc->__req())
			<< CHAR_VALUE("__rev", fc->__rev())
			<< CHAR_VALUE("__user", fc->self_.user_id.c_str());
	}
};

// getting data for given url (for sending/posting reasons)
class LinkScraperRequest : public HttpRequest
{
public:
	LinkScraperRequest(facebook_client *fc, status_data *status) :
		HttpRequest(REQUEST_POST, FACEBOOK_SERVER_REGULAR "/ajax/composerx/attachment/link/scraper/")
	{
		Url
			<< "__a=1"
			<< "composerurihash=2"
			<< CHAR_VALUE("scrape_url", ptrA(mir_urlEncode(status->url.c_str())));

		Body
			<< CHAR_VALUE("fb_dtsg", fc->dtsg_.c_str())
			<< CHAR_VALUE("targetid", status->user_id.empty() ? fc->self_.user_id.c_str() : status->user_id.c_str())
			<< CHAR_VALUE("xhpc_targetid", status->user_id.empty() ? fc->self_.user_id.c_str() : status->user_id.c_str())
			<< "istimeline=1"
			<< "composercontext=composer"
			<< "onecolumn=1"
			<< "nctr[_mod]=pagelet_timeline_recent"
			<< "__a=1"
			<< CHAR_VALUE("ttstamp", fc->ttstamp_.c_str())
			<< CHAR_VALUE("__user", status->isPage && !status->user_id.empty() ? status->user_id.c_str() : fc->self_.user_id.c_str())
			<< "loaded_components[0]=maininput"
			<< "loaded_components[1]=backdateicon"
			<< "loaded_components[2]=withtaggericon"
			<< "loaded_components[3]=cameraicon"
			<< "loaded_components[4]=placetaggericon"
			<< "loaded_components[5]=mainprivacywidget"
			<< "loaded_components[6]=withtaggericon"
			<< "loaded_components[7]=backdateicon"
			<< "loaded_components[8]=placetaggericon"
			<< "loaded_components[9]=cameraicon"
			<< "loaded_components[10]=mainprivacywidget"
			<< "loaded_components[11]=maininput"
			<< "loaded_components[12]=explicitplaceinput"
			<< "loaded_components[13]=hiddenplaceinput"
			<< "loaded_components[14]=placenameinput"
			<< "loaded_components[15]=hiddensessionid"
			<< "loaded_components[16]=withtagger"
			<< "loaded_components[17]=backdatepicker"
			<< "loaded_components[18]=placetagger"
			<< "loaded_components[19]=citysharericon";
	}
};

// getting owned/admined pages list
class GetPagesRequest : public HttpRequest
{
public:
	GetPagesRequest() :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_REGULAR "/bookmarks/pages")
	{

	}
};

#endif //_FACEBOOK_REQUEST_UTILS_H_
