#pragma once

#include <map>
#include <string>
#include <cassert>
#include <variant>
#include <vector>
#include <memory>

typedef std::string sym_name_t;
typedef std::variant<int, std::string> sym_info_t;
typedef std::map<sym_name_t, sym_info_t> symtab_t;

class SymTable {
    public:
        symtab_t symtab;
        std::shared_ptr<SymTable> parent_symtab;
        int index;
        static int symtab_count;

        SymTable();
        sym_info_t get_sym_value(sym_name_t sym);
};