#include "define.h"
#include "Overlay.h"
#include "Xorstr.h"

DWORD InstallHook()
{
	char wnd_title[256];
	HWND foreground = GetForegroundWindow();
	for (;;)
	{
		foreground = GetForegroundWindow();
		GetWindowTextA(foreground, wnd_title, sizeof(wnd_title));
		if (strstr(wnd_title, "Infestation : Survivor Stories SEA")) //  Game Title  Infestation ASIA     Infestation LastZ Survivor
		{
			g_hWindow = foreground;
			break;
		}
		std::this_thread::sleep_for(
			std::chrono::milliseconds(250));
	}

	/*AllocConsole();
	SetConsoleTitle("Debug Console");

	freopen("CONOUT$", "wb", stdout);
	freopen("CONOUT$", "wb", stderr);

	EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_GRAYED);
	*/
	return NULL;
}

DWORD WINAPI MainThread(LPVOID hModule)
{
	InstallHook();
	Overlay::GetInstance()->WinMain(HINSTANCE(), NULL, NULL, NULL);
	return 0;
}

extern __declspec(dllexport)void WINAPI OnDllAttach(LPVOID pParam)
{
}

bool _stdcall DllMain(HMODULE hModule, std::uintptr_t uReason, void* lpReserved)
{
	if (uReason == DLL_PROCESS_ATTACH)
	{
		///AllocConsole();
		hGameBase = GetModuleHandleA(NULL);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)OnDllAttach, hModule, 0, 0);
		CreateThread(NULL, NULL, MainThread, hModule, NULL, nullptr);
		//CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Delete, hModule, 0, 0);
	}

	return true;
}