#pragma once

#include <map>
#include <string>
#include <cassert>
#include <variant>

typedef std::string sym_name_t;
typedef std::variant<int, std::string> sym_info_t;

void insert_sym(sym_name_t sym, sym_info_t info);
sym_info_t get_sym_value(sym_name_t sym);
bool sym_exists(sym_name_t sym);