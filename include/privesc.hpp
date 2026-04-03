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
char *get_myh_path(void);

bool elevate_runas();

#define path_reg "Software\\Classes\\ms-settings\\Shell\\Open\\command"
bool elevate_fodhelper();

int create_key(const char *path);
int set_value(const char *path, const char *name, const char *data);
int open_key(const char *path, HKEY *out);