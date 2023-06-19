#pragma once

#include <map>
#include <string>
#include <cassert>

void insert_sym(std::string sym, int info);
int get_sym_value(std::string sym);
bool sym_exists(std::string sym);