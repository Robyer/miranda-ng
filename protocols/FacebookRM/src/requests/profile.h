/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright � 2011-16 Robert P�sel

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

#ifndef _FACEBOOK_REQUEST_PROFILE_H_
#define _FACEBOOK_REQUEST_PROFILE_H_

// getting own name, avatar, ...
class HomeRequest : public HttpRequest
{
public:
	HomeRequest() :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_MOBILE "/profile.php")
	{
		Url
			<< "v=info";
	}
};

// getting fb_dtsg
class DtsgRequest : public HttpRequest
{
public:
	DtsgRequest() :
		HttpRequest(REQUEST_GET, FACEBOOK_SERVER_MOBILE "/editprofile.php")
	{
		Url
			<< "edit=current_city"
			<< "type=basic";
	}
};

#endif //_FACEBOOK_REQUEST_PROFILE_H_
