#include "ast.hpp"

int BaseAST::sym_num = 0;
int BaseAST::lsym_num = 0;
int BaseAST::rsym_num = 0;
std::pair<bool, int> BaseAST::proc_const(false, 0);