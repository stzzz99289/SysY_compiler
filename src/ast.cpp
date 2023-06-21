#include "ast.hpp"

// definition of static member variables
int BaseAST::sym_num = 0;
std::pair<bool, int> BaseAST::proc_const(false, 0);
std::string BaseAST::var_mode = "none";
std::shared_ptr<SymTable> BaseAST::current_symtab = nullptr; // current block symbol table

// definition of member functions
std::string 
BaseAST::get_koopa_symbol() {
    std::stringstream ele; // element name (constant or symbol)
    if (proc_const.first) {
        ele << proc_const.second;
        proc_const.first = false;
    }
    else {
        ele << "\%" << sym_num-1;
    }

    return ele.str();
}

void 
BaseAST::new_koopa_symbol() {
    std::cout << "\t\%" << sym_num;
    sym_num = sym_num + 1;
}

void
BaseAST::update_current_symtab(sym_name_t sym, sym_info_t info) {
    current_symtab->symtab[sym] = info;
}