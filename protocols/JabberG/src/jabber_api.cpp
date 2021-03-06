/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2012-18 Miranda NG team

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

// Jabber API functions
INT_PTR __cdecl CJabberProto::JabberGetApi(WPARAM, LPARAM lParam)
{
	IJabberInterface **ji = (IJabberInterface**)lParam;
	if (!ji)
		return -1;
	*ji = this;
	return 0;
}

DWORD CJabberProto::GetFlags() const
{
	return JIF_UNICODE;
}

int CJabberProto::GetVersion() const
{
	return 1;
}

DWORD CJabberProto::GetJabberVersion() const
{
	return PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM);
}

MCONTACT CJabberProto::ContactFromJID(LPCTSTR jid)
{
	return HContactFromJID(jid);
}

LPTSTR CJabberProto::ContactToJID(MCONTACT hContact)
{
	return getWStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : "jid");
}

LPTSTR CJabberProto::GetBestResourceName(LPCTSTR jid)
{
	if (jid == nullptr)
		return nullptr;
	LPCTSTR p = wcschr(jid, '/');
	if (p == nullptr) {
		mir_cslock lck(m_csLists);
		return mir_wstrdup(ListGetBestClientResourceNamePtr(jid));
	}
	return mir_wstrdup(jid);
}

LPTSTR CJabberProto::GetResourceList(LPCTSTR jid)
{
	if (jid == nullptr)
		return nullptr;

	mir_cslock lck(m_csLists);
	JABBER_LIST_ITEM *item = nullptr;
	if ((item = ListGetItemPtr(LIST_VCARD_TEMP, jid)) == nullptr)
		item = ListGetItemPtr(LIST_ROSTER, jid);
	if (item == nullptr)
		return nullptr;

	if (!item->arResources.getCount())
		return nullptr;

	CMStringW res;
	for (int i=0; i < item->arResources.getCount(); i++) {
		res.Append(item->arResources[i]->m_tszResourceName);
		res.AppendChar(0);
	}
	res.AppendChar(0);

	return mir_wstrndup(res, res.GetLength());
}

char *CJabberProto::GetModuleName() const
{
	return m_szModuleName;
}

int CJabberProto::SendXmlNode(HXML node)
{
	return m_ThreadInfo->send(node);
}

typedef struct
{
	JABBER_HANDLER_FUNC Func;
	void *pUserData;
} sHandlerData;

void CJabberProto::ExternalTempIqHandler(HXML node, CJabberIqInfo *pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	d->Func(this, node, d->pUserData);
	free(d); // free IqHandlerData allocated in CJabberProto::AddIqHandler below
}

BOOL CJabberProto::ExternalIqHandler(HXML node, CJabberIqInfo *pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

BOOL CJabberProto::ExternalMessageHandler(HXML node, ThreadData*, CJabberMessageInfo* pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

BOOL CJabberProto::ExternalPresenceHandler(HXML node, ThreadData*, CJabberPresenceInfo* pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

BOOL CJabberProto::ExternalSendHandler(HXML node, ThreadData*, CJabberSendInfo* pInfo)
{
	sHandlerData *d = (sHandlerData*)pInfo->GetUserData();
	return d->Func(this, node, d->pUserData);
}

HJHANDLER CJabberProto::AddPresenceHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_presenceManager.AddPermanentHandler(&CJabberProto::ExternalPresenceHandler, d, free, iPriority);
}

HJHANDLER CJabberProto::AddMessageHandler(JABBER_HANDLER_FUNC Func, int iMsgTypes, LPCTSTR szXmlns, LPCTSTR szTag, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_messageManager.AddPermanentHandler(&CJabberProto::ExternalMessageHandler, iMsgTypes, 0, szXmlns, FALSE, szTag, d, free, iPriority);
}

HJHANDLER CJabberProto::AddIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, LPCTSTR szXmlns, LPCTSTR szTag, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_iqManager.AddPermanentHandler(&CJabberProto::ExternalIqHandler, iIqTypes, 0, szXmlns, FALSE, szTag, d, free, iPriority);
}

HJHANDLER CJabberProto::AddTemporaryIqHandler(JABBER_HANDLER_FUNC Func, int iIqTypes, int iIqId, void *pUserData, DWORD dwTimeout, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	CJabberIqInfo *pInfo = AddIQ(&CJabberProto::ExternalTempIqHandler, iIqTypes, nullptr, 0, iIqId, d, iPriority);
	if (pInfo && dwTimeout > 0)
		pInfo->SetTimeout(dwTimeout);
	return (HJHANDLER)pInfo;
}

HJHANDLER CJabberProto::AddSendHandler(JABBER_HANDLER_FUNC Func, void *pUserData, int iPriority)
{
	sHandlerData *d = (sHandlerData*)malloc(sizeof(sHandlerData));
	d->Func = Func;
	d->pUserData = pUserData;
	return (HJHANDLER)m_sendManager.AddPermanentHandler(&CJabberProto::ExternalSendHandler, d, free, iPriority);
}

int CJabberProto::RemoveHandler(HJHANDLER hHandler)
{
	return m_sendManager.DeletePermanentHandler((CJabberSendPermanentInfo*)hHandler) ||
		m_presenceManager.DeletePermanentHandler((CJabberPresencePermanentInfo*)hHandler) ||
		m_messageManager.DeletePermanentHandler((CJabberMessagePermanentInfo*)hHandler) ||
		m_iqManager.DeletePermanentHandler((CJabberIqPermanentInfo*)hHandler) ||
		m_iqManager.DeleteHandler((CJabberIqInfo*)hHandler);
}

JabberFeatCapPairDynamic *CJabberProto::FindFeature(LPCTSTR szFeature)
{
	for (int i=0; i < m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		if (!mir_wstrcmp(m_lstJabberFeatCapPairsDynamic[i]->szFeature, szFeature))
			return m_lstJabberFeatCapPairsDynamic[i];

	return nullptr;
}

int CJabberProto::RegisterFeature(LPCTSTR szFeature, LPCTSTR szDescription)
{
	if (!szFeature)
		return false;

	// check for this feature in core features, and return false if it's present, to prevent re-registering a core feature
	int i;
	for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
		if (!mir_wstrcmp(g_JabberFeatCapPairs[i].szFeature, szFeature))
			return false;

	mir_cslock lck(m_csLists);
	JabberFeatCapPairDynamic *fcp = FindFeature(szFeature);
	if (!fcp) { // if the feature is not registered yet, allocate new bit for it
		JabberCapsBits jcb = JABBER_CAPS_OTHER_SPECIAL; // set all bits not included in g_JabberFeatCapPairs

		// set all bits occupied by g_JabberFeatCapPairs
		for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
			jcb |= g_JabberFeatCapPairs[i].jcbCap;

		// set all bits already occupied by external plugins
		for (i=0; i < m_lstJabberFeatCapPairsDynamic.getCount(); i++)
			jcb |= m_lstJabberFeatCapPairsDynamic[i]->jcbCap;

		// Now get first zero bit. The line below is a fast way to do it. If there are no zero bits, it returns 0.
		jcb = (~jcb) & (JabberCapsBits)(-(__int64)(~jcb));

		// no more free bits
		if (!jcb)
			return false;

		// remove unnecessary symbols from szFeature to make the string shorter, and use it as szExt
		LPTSTR szExt = mir_wstrdup(szFeature);
		LPTSTR pSrc, pDst;
		for (pSrc = szExt, pDst = szExt; *pSrc; pSrc++)
			if (wcschr(L"bcdfghjklmnpqrstvwxz0123456789", *pSrc))
				*pDst++ = *pSrc;
		*pDst = 0;
		m_clientCapsManager.SetOwnCaps(JABBER_CAPS_MIRANDA_NODE, szExt, jcb);

		fcp = new JabberFeatCapPairDynamic();
		fcp->szExt = szExt; // will be deallocated along with other values of JabberFeatCapPairDynamic in CJabberProto destructor
		fcp->szFeature = mir_wstrdup(szFeature);
		fcp->szDescription = szDescription ? mir_wstrdup(szDescription) : nullptr;
		fcp->jcbCap = jcb;
		m_lstJabberFeatCapPairsDynamic.insert(fcp);
	}
	else if (szDescription) { // update description
		if (fcp->szDescription)
			mir_free(fcp->szDescription);
		fcp->szDescription = mir_wstrdup(szDescription);
	}
	return true;
}

int CJabberProto::AddFeatures(LPCTSTR szFeatures)
{
	if (!szFeatures)
		return false;

	mir_cslockfull lck(m_csLists);
	BOOL ret = true;
	LPCTSTR szFeat = szFeatures;
	while (szFeat[0]) {
		JabberFeatCapPairDynamic *fcp = FindFeature(szFeat);
		// if someone is trying to add one of core features, RegisterFeature() will return false, so we don't have to perform this check here
		if (!fcp) { // if the feature is not registered yet
			if (!RegisterFeature(szFeat, nullptr))
				ret = false;
			else
				fcp = FindFeature(szFeat); // update fcp after RegisterFeature()
		}
		if (fcp)
			m_uEnabledFeatCapsDynamic |= fcp->jcbCap;
		else
			ret = false;
		szFeat += mir_wstrlen(szFeat) + 1;
	}
	lck.unlock();

	if (m_bJabberOnline)
		SendPresence(m_iStatus, true);

	return ret;
}

int CJabberProto::RemoveFeatures(LPCTSTR szFeatures)
{
	if (!szFeatures)
		return false;

	mir_cslockfull lck(m_csLists);
	BOOL ret = true;
	LPCTSTR szFeat = szFeatures;
	while (szFeat[0]) {
		JabberFeatCapPairDynamic *fcp = FindFeature(szFeat);
		if (fcp)
			m_uEnabledFeatCapsDynamic &= ~fcp->jcbCap;
		else
			ret = false; // indicate that there was an error removing at least one of the specified features

		szFeat += mir_wstrlen(szFeat) + 1;
	}
	lck.unlock();

	if (m_bJabberOnline)
		SendPresence(m_iStatus, true);

	return ret;
}

LPTSTR CJabberProto::GetResourceFeatures(LPCTSTR jid)
{
	JabberCapsBits jcb = GetResourceCapabilities(jid);
	if (jcb & JABBER_RESOURCE_CAPS_ERROR)
		return nullptr;

	mir_cslockfull lck(m_csLists);
	int i;
	size_t iLen = 1; // 1 for extra zero terminator at the end of the string
	// calculate total necessary string length
	for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
		if (jcb & g_JabberFeatCapPairs[i].jcbCap)
			iLen += mir_wstrlen(g_JabberFeatCapPairs[i].szFeature) + 1;

	for (i=0; i < m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		if (jcb & m_lstJabberFeatCapPairsDynamic[i]->jcbCap)
			iLen += mir_wstrlen(m_lstJabberFeatCapPairsDynamic[i]->szFeature) + 1;

	// allocate memory and fill it
	LPTSTR str = (LPTSTR)mir_alloc(iLen * sizeof(wchar_t));
	LPTSTR p = str;
	for (i=0; g_JabberFeatCapPairs[i].szFeature; i++)
		if (jcb & g_JabberFeatCapPairs[i].jcbCap) {
			mir_wstrcpy(p, g_JabberFeatCapPairs[i].szFeature);
			p += mir_wstrlen(g_JabberFeatCapPairs[i].szFeature) + 1;
		}

	for (i=0; i < m_lstJabberFeatCapPairsDynamic.getCount(); i++)
		if (jcb & m_lstJabberFeatCapPairsDynamic[i]->jcbCap) {
			mir_wstrcpy(p, m_lstJabberFeatCapPairsDynamic[i]->szFeature);
			p += mir_wstrlen(m_lstJabberFeatCapPairsDynamic[i]->szFeature) + 1;
		}

	*p = 0; // extra zero terminator
	return str;
}

HNETLIBUSER CJabberProto::GetHandle()
{
	return m_hNetlibUser;
}
