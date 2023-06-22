#include "ast.hpp"

// definition of static member variables
int BaseAST::sym_num = 0;
std::pair<bool, int> BaseAST::proc_const(false, 0);
std::string BaseAST::var_mode = "none";
std::shared_ptr<SymTable> BaseAST::current_symtab = nullptr;
// std::stringstream BaseAST::cout_bin("");
std::vector<int> BaseAST::wentry_bn_stack = { }; // stack of all while entry name index
std::vector<int> BaseAST::wend_bn_stack = { }; // stack of all while end name index

// definition of member functions
std::string 
BaseAST::get_koopa_symbol() {
    std::stringstream ele; // element name (constant or symbol)
    if (proc_const.first) {
        ele << proc_const.second;
        proc_const.first = false;
    }
    else {
        ele << "\%" << std::max(sym_num-1, 0);
    }

    return ele.str();
}

void 
BaseAST::new_koopa_symbol() {
    std::cout << "\t\%" << sym_num;
    sym_num = sym_num + 1;
}

std::string
BaseAST::new_koopa_block(std::string block_type) {
    // block types: {%then, %else, %end, %unreachable}
    static int then_n = 0;
    static int else_n = 0;
    static int end_n = 0;
    static int unreached_n = 0;
    static int wentry_n = 0;
    static int wbody_n = 0;
    static int wend_n = 0;

    if (block_type == "then")
        return "\%" + block_type + std::to_string(then_n++);
    else if (block_type == "else")
        return "\%" + block_type + std::to_string(else_n++);
    else if (block_type == "end")
        return "\%" + block_type + std::to_string(end_n++);
    else if (block_type == "unreached")
        return "\%" + block_type + std::to_string(unreached_n++);
    else if (block_type == "while_entry") {
        wentry_bn_stack.push_back(wentry_n);
        return "\%" + block_type + std::to_string(wentry_n++);
    }
    else if (block_type == "while_body")
        return "\%" + block_type + std::to_string(wbody_n++);
    else if (block_type == "while_end") {
        wend_bn_stack.push_back(wend_n);
        return "\%" + block_type + std::to_string(wend_n++);
    }  
    else
        return "\%wrong_block_type";
}

void
BaseAST::update_current_symtab(sym_name_t sym, sym_info_t info) {
    current_symtab->symtab[sym] = info;
}