#include "stdafx.h"

int hLangpack;
HINSTANCE g_hInst;
HANDLE hAddOptionService;

PLUGININFOEX pluginInfo = 
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {78975601-C869-4BEE-84EE-5D79E662F6D9}
	{ 0x78975601, 0xc869, 0x4bee, { 0x84, 0xee, 0x5d, 0x79, 0xe6, 0x62, 0xf6, 0xd9 } }
};

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInst = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	hAddOptionService = CreateServiceFunction(MODULENAME "/AddOption", AddOptionService);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if (hAddOptionService)
		DestroyServiceFunction(hAddOptionService);

	return 0;
}