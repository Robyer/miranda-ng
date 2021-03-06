#include "stdafx.h"
#include "MraMPop.h"

//	MPOP_SESSION
struct MRA_MPOP_SESSION_QUEUE : public FIFO_MT
{
	bool    bKeyValid;	/* lpszMPOPKey contain valid key. */
	LPSTR   lpszMPOPKey;	/* Key for web auth on mail.ru services. */
	size_t  dwMPOPKeySize;	/* Key size. */
};

struct MRA_MPOP_SESSION_QUEUE_ITEM : public FIFO_MT_ITEM
{
	LPSTR   lpszUrl;	/* Url to open. */
	size_t  dwUrlSize;
};

DWORD MraMPopSessionQueueInitialize(HANDLE *phMPopSessionQueue)
{
	if (!phMPopSessionQueue)
		return ERROR_INVALID_HANDLE;
	if ((*phMPopSessionQueue))
		return ERROR_ALREADY_INITIALIZED;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = new MRA_MPOP_SESSION_QUEUE();
	if (!pmpsqMPopSessionQueue)
		return GetLastError();

	*phMPopSessionQueue = (HANDLE)pmpsqMPopSessionQueue;
	return NO_ERROR;
}

void MraMPopSessionQueueClear(HANDLE hQueue)
{
	if (!hQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hQueue;
	pmpsqMPopSessionQueue->bKeyValid = false;
	mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
	pmpsqMPopSessionQueue->lpszMPOPKey = nullptr;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;

	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;
	while ( !FifoMTItemPop(pmpsqMPopSessionQueue, nullptr, (LPVOID*)&pmpsqi))
		mir_free(pmpsqi);
}

void MraMPopSessionQueueDestroy(HANDLE hQueue)
{
	if (!hQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hQueue;
	MraMPopSessionQueueClear(hQueue);
	delete pmpsqMPopSessionQueue;
}

DWORD CMraProto::MraMPopSessionQueueAddUrl(HANDLE hQueue, const CMStringA &lpszUrl)
{
	if (!hQueue)
		return ERROR_INVALID_HANDLE;
	if (lpszUrl.IsEmpty())
		return ERROR_INVALID_DATA;
	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	if (!getByte("AutoAuthOnWebServices", MRA_DEFAULT_AUTO_AUTH_ON_WEB_SVCS) || !m_bLoggedIn) { /* Open without web auth. / Not loggedIn. */
		Utils_OpenUrl(lpszUrl);
		return NO_ERROR;
	}
	/* Add to queue. */
	pmpsqi = (MRA_MPOP_SESSION_QUEUE_ITEM*)mir_calloc((sizeof(MRA_MPOP_SESSION_QUEUE_ITEM) + lpszUrl.GetLength() + sizeof(size_t)));
	if (!pmpsqi)
		return GetLastError();

	pmpsqi->dwUrlSize = lpszUrl.GetLength();
	pmpsqi->lpszUrl = (LPSTR)(pmpsqi + 1);
	memcpy(pmpsqi->lpszUrl, lpszUrl, lpszUrl.GetLength());
	FifoMTItemPush(pmpsqMPopSessionQueue, pmpsqi, (LPVOID)pmpsqi);
	MraMPopSessionQueueStart(hQueue);
	return NO_ERROR;
}

DWORD CMraProto::MraMPopSessionQueueAddUrlAndEMail(HANDLE hQueue, const CMStringA &lpszUrl, CMStringA &szEmail)
{
	if (!hQueue)
		return ERROR_INVALID_HANDLE;
	if (lpszUrl.IsEmpty() || szEmail.IsEmpty())
		return ERROR_INVALID_DATA;

	szEmail.MakeLower();

	int iStart = 0;
	CMStringA szUser = szEmail.Tokenize("@", iStart);
	CMStringA szDomain = szEmail.Tokenize("@", iStart);

	CMStringA szUrl;
	szUrl.Format("%s/%s/%s", lpszUrl.c_str(), szDomain.c_str(), szUser.c_str());
	return MraMPopSessionQueueAddUrl(hQueue, szUrl);
}

void CMraProto::MraMPopSessionQueueStart(HANDLE hQueue)
{
	if (!hQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	if (!getByte("AutoAuthOnWebServices", MRA_DEFAULT_AUTO_AUTH_ON_WEB_SVCS) || !m_bLoggedIn) { /* Open without web auth. / Not loggedIn. */
		MraMPopSessionQueueFlush(hQueue);
		return;
	}

	while ( FifoMTGetCount(pmpsqMPopSessionQueue)) {
		if (!pmpsqMPopSessionQueue->bKeyValid) { /* We have no key, try to get one. */
			if (0 == MraSendCMD(MRIM_CS_GET_MPOP_SESSION, nullptr, 0))	/* Fail to send. */
				MraMPopSessionQueueFlush(hQueue);
			return;
		}

		if ( FifoMTItemPop(pmpsqMPopSessionQueue, nullptr, (LPVOID*)&pmpsqi) == NO_ERROR) {
			CMStringA szUrl, szEmail;
			if (mraGetStringA(NULL, "e-mail", szEmail)) {
				pmpsqMPopSessionQueue->bKeyValid = false;
				szEmail.MakeLower();
				szUrl.Format(MRA_MPOP_AUTH_URL, szEmail.c_str(), pmpsqMPopSessionQueue->lpszMPOPKey, pmpsqi->lpszUrl);
				Utils_OpenUrl(szUrl);
				debugLogA("Opening URL: %s\n", szUrl.c_str());
			}
			mir_free(pmpsqi);
		}
	}
}

void CMraProto::MraMPopSessionQueueFlush(HANDLE hQueue)
{
	if (!hQueue)
		return;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hQueue;
	MRA_MPOP_SESSION_QUEUE_ITEM *pmpsqi;

	while (FifoMTItemPop(pmpsqMPopSessionQueue, nullptr, (LPVOID*)&pmpsqi) == NO_ERROR) {
		Utils_OpenUrl(pmpsqi->lpszUrl);
		mir_free(pmpsqi);
	}
}

DWORD MraMPopSessionQueueSetNewMPopKey(HANDLE hQueue, const CMStringA &szKey)
{
	if (!hQueue)
		return ERROR_INVALID_HANDLE;

	MRA_MPOP_SESSION_QUEUE *pmpsqMPopSessionQueue = (MRA_MPOP_SESSION_QUEUE*)hQueue;
	if (pmpsqMPopSessionQueue->dwMPOPKeySize < (size_t)szKey.GetLength() || szKey.IsEmpty()) {
		mir_free(pmpsqMPopSessionQueue->lpszMPOPKey);
		pmpsqMPopSessionQueue->lpszMPOPKey = (LPSTR)mir_calloc(szKey.GetLength() + sizeof(size_t));
	}

	if (pmpsqMPopSessionQueue->lpszMPOPKey) {
		pmpsqMPopSessionQueue->bKeyValid = true;
		pmpsqMPopSessionQueue->dwMPOPKeySize = szKey.GetLength();
		memcpy(pmpsqMPopSessionQueue->lpszMPOPKey, szKey, szKey.GetLength());
		*(pmpsqMPopSessionQueue->lpszMPOPKey + szKey.GetLength()) = 0;
		return NO_ERROR;
	}

	pmpsqMPopSessionQueue->bKeyValid = false;
	pmpsqMPopSessionQueue->lpszMPOPKey = nullptr;
	pmpsqMPopSessionQueue->dwMPOPKeySize = 0;
	return GetLastError();
}
