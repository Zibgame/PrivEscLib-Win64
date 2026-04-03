#include "privesc.hpp"
#include <windows.h>

bool elevate_runas()
{
    char path[MAX_PATH];
    HINSTANCE ret;

    if (!GetModuleFileNameA(NULL, path, MAX_PATH))
        return (false);

    ret = ShellExecuteA(NULL, "runas", path, NULL, NULL, SW_SHOW);
    if ((INT_PTR)ret <= 32)
        return (false);
    ExitProcess(0);
    return (true);
}