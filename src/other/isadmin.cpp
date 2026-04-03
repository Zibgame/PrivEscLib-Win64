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