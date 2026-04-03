#include "privesc.hpp"
#include <windows.h>

bool is_admin()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(
        &ntAuthority,
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0,
        &adminGroup))
        return (false);

    CheckTokenMembership(NULL, adminGroup, &isAdmin);
    FreeSid(adminGroup);

    return (isAdmin);
}

char *get_myh_path(void)
{
	char	buffer[MAX_PATH];
	DWORD	len;
	char	*path;

	len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
	if (len == 0 || len == MAX_PATH)
		return (NULL);
	path = (char *)malloc(len + 1);
	if (!path)
		return (NULL);
	lstrcpyA(path, buffer);
	return (path);
}