#include "symtab.hpp"

int SymTable::symtab_count = 0;

SymTable::SymTable() {
    symtab = { };
    parent_symtab = nullptr;
    index = ++symtab_count;
}

sym_info_t 
SymTable::get_sym_value(sym_name_t sym) {
    symtab_t::iterator it = symtab.find(sym);
    if (it != symtab.end()) {
        // symbol is in the current symtab
        return it->second;
    }
    else {
        // symbol not in the current symtab
        if (!parent_symtab) {
            // no parent block, which should not occur
            assert(false);
        }
        else {
            // search in parent block
            return parent_symtab->get_sym_value(sym);
        }
    }
}