/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ifileoperation.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 05:12:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 05:12:00 by zcadinot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "privesc.hpp"
#include <objbase.h>
#include <shlobj.h>

static const CLSID g_clsid_fileoperation =
    {0x3AD05575, 0x8857, 0x4850,
     {0x92, 0x77, 0x11, 0xB8, 0x5B, 0xDB, 0x8E, 0x09}};

typedef HRESULT (__stdcall *SetOperationFlags_t)(void *, DWORD);
typedef HRESULT (__stdcall *Release_t)(void *);

bool elevate_ifileoperation(void)
{
	void *op = NULL;
	HRESULT hr;

	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr))
		return (false);
	hr = CoCreateInstance(g_clsid_fileoperation, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnknown, &op);
	if (FAILED(hr))
	{
		printf("CoCreateInstance(FileOperation) failed: 0x%08lX\n",
			(unsigned long)hr);
		fflush(stdout);
		CoUninitialize();
		return (false);
	}
	printf("FileOperation elevated object acquired\n");
	fflush(stdout);
	((IUnknown *)op)->Release();
	CoUninitialize();
	return (true);
}
