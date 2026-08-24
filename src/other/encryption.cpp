/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   encryption.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 02:19:23 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 02:19:24 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "privesc.hpp"
#include <windows.h>

static void	print_byte(unsigned char byte)
{
	const char	*hex;

	hex = "0123456789ABCDEF";
	std::cout << "\\x";
	std::cout << hex[(byte >> 4) & 0x0F];
	std::cout << hex[byte & 0x0F];
}

void	print_encrypted(const std::string &str)
{
	size_t	i;

	i = 0;
	while (i < str.size())
	{
		print_byte(static_cast<unsigned char>(str[i]));
		i++;
	}
	std::cout << std::endl;
}

std::string key_xor(const std::string &str, const std::string &key)
{
    if (key.empty())
		return (str);

    size_t	i = 0;
	std::string	res = "";

    while (i < str.size())
    {
        res += str[i] ^ key[i % key.size()];
        i++;
    }
    return (res);
}