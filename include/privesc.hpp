#pragma once

#include <windows.h>
#include <stdio.h>

enum method
{
    RUNAS,
    FODHELPER
};

bool elevate_privileges(method meto);
bool is_admin();

bool elevate_runas();