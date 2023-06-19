#include "symtab.hpp"

std::map<sym_name_t, sym_info_t> symtab = {};

void insert_sym(sym_name_t sym, sym_info_t info) {
    symtab[sym] = info;
}

sym_info_t get_sym_value(sym_name_t sym) {
    if (sym_exists(sym)) {
        std::map<sym_name_t, sym_info_t>::iterator it = symtab.find(sym);
        return it->second;
    }
    else {
        assert(false);
        return 0;
    }
}

bool sym_exists(sym_name_t sym) {
    std::map<sym_name_t, sym_info_t>::iterator it = symtab.find(sym);

    return (it != symtab.end());
}