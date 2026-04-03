#include "privesc.hpp"
#include <windows.h>


int open_key(const char *path, HKEY *out)
{
	LONG	res;

	res = RegCreateKeyExA(
		HKEY_CURRENT_USER,
		path,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		out,
		NULL
	);
	if (res != ERROR_SUCCESS)
		return (1);
	return (0);
}

int create_key(const char *path)
{
	HKEY	hkey;

	if (!path)
		return (1);
	if (open_key(path, &hkey) != 0)
		return (1);
	RegCloseKey(hkey);
	return (0);
}

int set_value(const char *path, const char *name, const char *data)
{
	HKEY	hkey;
	DWORD	size;
	LONG	res;

	if (!path)
		return (1);
	if (open_key(path, &hkey) != 0)
		return (1);
	if (!data)
		size = 1;
	else
		size = (DWORD)(lstrlenA(data) + 1);
	res = RegSetValueExA(
		hkey,
		name,
		0,
		REG_SZ,
		(const BYTE *)data,
		size
	);
	if (res != ERROR_SUCCESS)
	{
		RegCloseKey(hkey);
		return (1);
	}
	RegCloseKey(hkey);
	return (0);
}