#ifndef _GLOBAL_H_
#define _GLOBAL_H_

static LIST<void> arMonitoredWindows(3, PtrKeySortT);

const enum VALUE_TYPE {
	TYPE_BYTE,
	TYPE_WORD,
	TYPE_DWORD,
	TYPE_STRING,
	TYPE_UNICODE
};

typedef struct {
	int cbSize;                    // must be equal to sizeof(HiddenOptionData)
	char *szModule;
	char *szKey;
	char *szDescription;
	VALUE_TYPE type;	
	union {
		int defaultByte;
		int defaultWord;
		DWORD defaultDword;
		char *defaultString;
		wchar_t *defaultUnicode;
	};
} HiddenOptionData;

int OnModulesLoaded(WPARAM, LPARAM);
INT_PTR AddOptionService(WPARAM, LPARAM);


#endif //_GLOBAL_H_