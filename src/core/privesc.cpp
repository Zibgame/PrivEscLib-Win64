#include "privesc.hpp"

bool elevate_privileges(method meto)
{
    if (is_admin())
    {
        return (true);
    }
    if (meto == RUNAS)
    {
        return (elevate_runas());
    }
    if (meto == FODHELPER)
    {
        return (true);
    }
    return (true);
}