#include "privesc.hpp"

int main()
{
    elevate_privileges(RUNAS);
    printf("HelloWorld");

    while (true)
    {
        Sleep(1000);
    }
    return (0);
}