#include "symtab.hpp"

std::map<std::string, int> symtab = {};

void insert_sym(std::string sym, int info) {
    symtab[sym] = info;
}

int get_sym_value(std::string sym) {
    if (sym_exists(sym)) {
        std::map<std::string, int>::iterator it = symtab.find(sym);
        return it->second;
    }
    else {
        assert(false);
        return 0;
    }
}

bool sym_exists(std::string sym) {
    std::map<std::string, int>::iterator it = symtab.find(sym);

    return (it != symtab.end());
}