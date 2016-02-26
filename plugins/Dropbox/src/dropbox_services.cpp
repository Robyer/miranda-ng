#include "stdafx.h"

HANDLE CDropbox::CreateProtoServiceFunctionObj(const char *szService, MIRANDASERVICEOBJ serviceProc, void *obj)
{
	char str[MAXMODULELABELLENGTH];
	mir_snprintf(str, "%s%s", MODULE, szService);
	str[MAXMODULELABELLENGTH - 1] = 0;
	return CreateServiceFunctionObj(str, serviceProc, obj);
}

INT_PTR CDropbox::ProtoGetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_FILESEND;
	case PFLAGNUM_2:
		return PF2_ONLINE;
	case PFLAGNUM_4:
		return PF4_OFFLINEFILES;
	}

	return 0;
}

INT_PTR CDropbox::ProtoGetName(WPARAM wParam, LPARAM lParam)
{
	if (lParam) {
		mir_strncpy((char *)lParam, MODULE, wParam);
		return 0;
	}

	return 1;
}

INT_PTR CDropbox::ProtoLoadIcon(WPARAM wParam, LPARAM)
{
	return (LOWORD(wParam) == PLI_PROTOCOL) ? (INT_PTR)CopyIcon(LoadIconEx(IDI_DROPBOX)) : 0;
}

INT_PTR CDropbox::ProtoGetStatus(WPARAM, LPARAM)
{
	return HasAccessToken() ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE;
}

INT_PTR CDropbox::ProtoSendFile(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	if (!HasAccessToken()) {
		ProtoBroadcastAck(MODULE, pccsd->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send files when you are not authorized.");
		return 0;
	}

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.flags |= PFTS_SENDING;
	ftp->pfts.hContact = pccsd->hContact;
	ftp->hContact = (hTransferContact) ? hTransferContact : pccsd->hContact;
	hTransferContact = 0;

	ftp->SetDescription((TCHAR*)pccsd->wParam);

	TCHAR **paths = (TCHAR**)pccsd->lParam;
	ftp->SetWorkingDirectory(paths[0]);
	for (int i = 0; paths[i]; i++) {
		if (PathIsDirectory(paths[i]))
			continue;
		ftp->AddFile(paths[i]);
	}

	ULONG fileId = InterlockedIncrement(&hFileProcess);
	ftp->hProcess = (HANDLE)fileId;
	transfers.insert(ftp);

	mir_forkthreadowner(CDropbox::SendFilesAndReportAsync, this, ftp, 0);

	return fileId;
}

INT_PTR CDropbox::ProtoSendFileInterceptor(WPARAM wParam, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	const char *proto = GetContactProto(pccsd->hContact);
	const char *interceptedProtos = db_get_sa(NULL, MODULE, "InterceptedProtos");
	if (interceptedProtos == NULL || strstr(interceptedProtos, proto) == NULL)
		return CALLSERVICE_NOTFOUND;
	
	return ProtoSendFile(wParam, lParam);
}

INT_PTR CDropbox::ProtoCancelFile(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	HANDLE hTransfer = (HANDLE)pccsd->wParam;
	FileTransferParam *ftp = transfers.find((FileTransferParam*)&hTransfer);
	if (ftp == NULL)
		return 0;

	ftp->isTerminated = true;

	return 0;
}

INT_PTR CDropbox::ProtoSendMessage(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	if (!HasAccessToken()) {
		ProtoBroadcastAck(MODULE, pccsd->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, (LPARAM)"You cannot send messages when you are not authorized.");
		return 0;
	}

	char *szMessage = (char*)pccsd->lParam;
	if (*szMessage == '/') {
		// parse commands
		char *sep = strchr(szMessage, ' ');
		if (sep != NULL) *sep = 0;

		struct
		{
			const char *szCommand;
			pThreadFunc pHandler;
		}
		static commands[] =
		{
			{ "help", &CDropbox::CommandHelp },
			{ "list", &CDropbox::CommandContent },
			{ "share", &CDropbox::CommandShare },
			{ "delete", &CDropbox::CommandDelete }
		};

		for (int i = 0; i < _countof(commands); i++) {
			if (!mir_strcmp(szMessage + 1, commands[i].szCommand)) {
				ULONG messageId = InterlockedIncrement(&hMessageProcess);

				CommandParam *param = new CommandParam();
				param->instance = this;
				param->hContact = pccsd->hContact;
				param->hProcess = (HANDLE)messageId;
				param->data = (sep ? sep + 1 : NULL);

				mir_forkthread(commands[i].pHandler, param);

				return messageId;
			}
		}
	}

	char help[1024];
	mir_snprintf(help, Translate("\"%s\" is not valid.\nUse \"/help\" for more info."), szMessage);
	CallContactService(GetDefaultContact(), PSR_MESSAGE, 0, (LPARAM)help);
	return 0;
}

INT_PTR CDropbox::ProtoReceiveMessage(WPARAM, LPARAM lParam)
{
	CCSDATA *pccsd = (CCSDATA*)lParam;

	char *message = (char*)pccsd->lParam;

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.flags = DBEF_UTF;
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)mir_strlen(message);
	dbei.pBlob = (PBYTE)mir_strdup(message);
	db_event_add(pccsd->hContact, &dbei);

	return 0;
}

INT_PTR CDropbox::SendFileToDropbox(WPARAM hContact, LPARAM lParam)
{
	if (!HasAccessToken())
		return 0;

	if (hContact == NULL)
		hContact = GetDefaultContact();

	TCHAR *filePath = (TCHAR*)lParam;

	FileTransferParam *ftp = new FileTransferParam();
	ftp->pfts.hContact = hContact;
	ftp->pfts.totalFiles = 1;
	ftp->hContact = (hTransferContact) ? hTransferContact : hContact;
	hTransferContact = 0;

	int length = _tcsrchr(filePath, '\\') - filePath;
	ftp->pfts.tszWorkingDir = (TCHAR*)mir_alloc(sizeof(TCHAR) * (length + 1));
	mir_tstrncpy(ftp->pfts.tszWorkingDir, filePath, length + 1);
	ftp->pfts.tszWorkingDir[length] = '\0';

	ftp->pfts.ptszFiles = (TCHAR**)mir_alloc(sizeof(TCHAR*) * (ftp->pfts.totalFiles + 1));
	ftp->pfts.ptszFiles[0] = mir_wstrdup(filePath);
	ftp->pfts.ptszFiles[ftp->pfts.totalFiles] = NULL;

	ULONG fileId = InterlockedIncrement(&hFileProcess);
	ftp->hProcess = (HANDLE)fileId;

	mir_forkthreadowner(CDropbox::SendFilesAndEventAsync, this, ftp, 0);

	return fileId;
}
