#include "privesc.hpp"

int	main(void)
{
	int	admin;

	elevate_privileges(FODHELPER);
	admin = is_admin();
	if (!admin)
		printf("Admin: False\n");
	else
		printf("Admin: True\n");
	while (1)
		Sleep(1000);
	return (0);
}