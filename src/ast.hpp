#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include "symtab.hpp"

/*
Current EBNF:
CompUnit      ::= FuncDef;

Decl          ::= ConstDecl | VarDecl;
ConstDecl     ::= "const" BType ConstDefList ";"
ConstDefList  ::= ConstDef | ConstDefList "," ConstDef
BType         ::= "int";
ConstDef      ::= IDENT "=" ConstInitVal;
ConstInitVal  ::= ConstExp;
VarDecl       ::= BType VarDefList ";";
VarDefList    ::= VarDef | VarDefList "," VarDef
VarDef        ::= IDENT | IDENT "=" InitVal;
InitVal       ::= Exp;

FuncDef       ::= FuncType IDENT "(" ")" Block;
FuncType      ::= "int";

Block         ::= "{" BlockItemList "}"; 
BlockItemList ::= | BlockItemList BlockItem
BlockItem     ::= Decl | Stmt;
Stmt          ::= MatchedStmt
                  | UnmatchedStmt
MatchedStmt   ::= LVal "=" Exp ";"
                  | [Exp] ";"
                  | Block
                  | "return" [Exp] ";"
                  | "while" "(" Exp ")" MatchedStmt
                  | "if" "(" Exp ")" MatchedStmt "else" MatchedStmt
UnmatchedStmt ::= "if" "(" Exp ")" Stmt
                  | "if" "(" Exp ")" MatchedStmt "else" UnmatchedStmt

Exp           ::= LOrExp;
LVal          ::= IDENT;
PrimaryExp    ::= "(" Exp ")" | LVal | Number;
Number        ::= INT_CONST;
UnaryExp      ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp       ::= "+" | "-" | "!";
MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp       ::= EqExp | LAndExp "&&" EqExp;
LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
ConstExp      ::= Exp;
*/

// Base class for all ASTs
class BaseAST {
    public:
        // virtual methods
        virtual ~BaseAST() = default;
        virtual void Dump() const = 0;
        virtual void GenKoopa() = 0;
        virtual int GetValue() {
            return 0;
        };
        virtual std::string GetOp() {
            return "";
        };

        // member methods
        std::string get_koopa_symbol();
        void new_koopa_symbol();
        std::string new_koopa_block(std::string block_type);
        void update_current_symtab(sym_name_t sym, sym_info_t info);

        // static memeber variables
        static int sym_num; // next koopa symbol number
        // static bool sym_type; // next koopa symbol name type
        static std::pair<bool, int> proc_const; // bool: processing const or not; int: const value
        static std::string var_mode; // "load" or "store" for different LVal koopa code
        static std::shared_ptr<SymTable> current_symtab; // current block symbol table
        static std::vector<int> wentry_bn_stack; // stack of all while entry name index
        static std::vector<int> wend_bn_stack; // stack of all while end name index
        // static std::stringstream cout_bin;
};

// CompUnit AST
class CompUnitAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_def;

        void Dump() const override {
            std::cout << "CompUnitAST {\n";
            func_def->Dump();
            std::cout << "\n}\n";
        }

        void GenKoopa() override {
            func_def->GenKoopa();
        }
};

// FuncDef AST
class FuncDefAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> block;

        void Dump() const override {
            std::cout << "FuncDefAST {\n";
            func_type->Dump();
            std::cout << ", " << ident << ", ";
            block->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            std::cout << "fun @" << ident << "(): ";
            func_type->GenKoopa();

            // create first block symtab
            auto sym_table = std::make_shared<SymTable>();
            current_symtab = sym_table;

            // first block is entry block
            std::cout << " {\n";
            std::cout << "\%entry:\n";

            auto old_symtab = current_symtab;
            block->GenKoopa();
            current_symtab = old_symtab;

            std::cout << "\tret 0\n";
            std::cout << "}";
        }
};

// FuncType AST
class FuncTypeAST : public BaseAST {
    public:
        const std::string ast_str = "int";
        const std::string koopa_str = "i32";

        void Dump() const override {
            std::cout << "FuncTypeAST {" << ast_str << "}";
        }

        void GenKoopa() override {
            std::cout << koopa_str;
        }
};

// Block AST
class BlockAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> block_item_lst;

        void Dump() const override {
            std::cout << "BlockAST {\n";
            block_item_lst->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            block_item_lst->GenKoopa();
        }
};

// BlockItemList AST
class BlockItemListAST_emp : public BaseAST {
    public:

        void Dump() const override {

        } 

        void GenKoopa() override {

        }
};
class BlockItemListAST_lst : public BaseAST {
    public:
        std::unique_ptr<BaseAST> block_item_lst;
        std::unique_ptr<BaseAST> block_item;

        void Dump() const override {

        } 

        void GenKoopa() override {
            block_item_lst->GenKoopa();
            block_item->GenKoopa();
        }
};

// BlockItem AST
class BlockItemAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> some_block;

        void Dump() const override {

        } 

        void GenKoopa() override {
            some_block->GenKoopa();
        }
};

// Stmt AST
class MatchedStmtAST_ret : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp_list;

        void Dump() const override {

        }

        void GenKoopa() override {
            exp_list->GenKoopa();

            // ret code
            std::string sym_name = get_koopa_symbol();
            std::cout << "\tret " << sym_name << "\n";

            // no more code should be generated
            // std::cout.rdbuf(cout_bin.rdbuf());

            // add a unreachable basic block
            std::string unreached_bn = new_koopa_block("unreached");
            std::cout << "\n" << unreached_bn << ":\n";
        }
};
class MatchedStmtAST_var : public BaseAST {
    public:
        std::unique_ptr<BaseAST> l_val;
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {

        }

        void GenKoopa() override {
            exp->GenKoopa();

            var_mode = "store";
            l_val->GenKoopa();
            var_mode = "none";
        }
};
class MatchedStmtAST_blk : public BaseAST {
    public:
        std::unique_ptr<BaseAST> block;
    
        void Dump() const override {

        }

        void GenKoopa() override {
            // save old symtab
            auto old_symtab = current_symtab;

            // create empty block symtab
            auto sym_table = std::make_shared<SymTable>();
            sym_table->parent_symtab = old_symtab;

            // entering block
            current_symtab = sym_table;
            block->GenKoopa();
            current_symtab = old_symtab;
        }
};
class MatchedStmtAST_lst : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp_list;
    
        void Dump() const override {

        }

        void GenKoopa() override {
            exp_list->GenKoopa();
        }
};
class MatchedStmtAST_ifelse : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> matched_stmt_if;
        std::unique_ptr<BaseAST> matched_stmt_else;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // calculate if exp
            exp->GenKoopa();

            // assign block names
            std::string then_bn = new_koopa_block("then");
            std::string else_bn = new_koopa_block("else");
            std::string end_bn = new_koopa_block("end");

            // br
            std::cout << "\tbr " << get_koopa_symbol();
            std::cout << ", " << then_bn << ", " << else_bn << "\n";

            // then block
            std::cout << "\n" << then_bn << ":\n";
            matched_stmt_if->GenKoopa();
            std::cout << "\tjump " << end_bn << "\n";

            // else block
            std::cout << "\n" << else_bn << ":\n";
            matched_stmt_else->GenKoopa();
            std::cout << "\tjump " << end_bn << "\n";

            // end block
            std::cout << "\n" << end_bn << ":\n";
        }
};
class MatchedStmtAST_while : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> matched_stmt;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // assign block names
            std::string wentry_bn = new_koopa_block("while_entry");
            std::string wbody_bn = new_koopa_block("while_body");
            std::string wend_bn = new_koopa_block("while_end");

            // jump to while entry
            std::cout << "\tjump " << wentry_bn << "\n";

            // while entry block
            std::cout << "\n" << wentry_bn << ":\n";
            exp->GenKoopa();
            std::cout << "\tbr " << get_koopa_symbol();
            std::cout << ", " << wbody_bn << ", " << wend_bn << "\n";

            // while body block
            std::cout << "\n" << wbody_bn << ":\n";
            matched_stmt->GenKoopa();
            std::cout << "\tjump " << wentry_bn << "\n";

            // end block
            std::cout << "\n" << wend_bn << ":\n";

            // pop stack info
            wentry_bn_stack.pop_back();
            wend_bn_stack.pop_back();
        }
};
class MatchedStmtAST_break : public BaseAST {
    public:

        void Dump() const override {

        } 

        void GenKoopa() override {
            // get current while end block name
            std::string curr_end_bn = "\%while_end" + std::to_string(wend_bn_stack.back());

            // jump to current end
            std::cout << "\tjump " << curr_end_bn << "\n";

            // add a unreachable basic block
            std::string unreached_bn = new_koopa_block("unreached");
            std::cout << "\n" << unreached_bn << ":\n";
        }
};
class MatchedStmtAST_continue : public BaseAST {
    public:
    
        void Dump() const override {

        } 

        void GenKoopa() override {
            // get current while end block name
            std::string curr_wentry_bn = "\%while_entry" + std::to_string(wentry_bn_stack.back());

            // jump to current entry
            std::cout << "\tjump " << curr_wentry_bn << "\n";

            // add a unreachable basic block
            std::string unreached_bn = new_koopa_block("unreached");
            std::cout << "\n" << unreached_bn << ":\n";
        }
};

class UnmatchedStmtAST_if : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> stmt;

        void Dump() const override {
        } 

        void GenKoopa() override {
            // calculate if exp
            exp->GenKoopa();

            // assign block names
            std::string then_bn = new_koopa_block("then");
            std::string end_bn = new_koopa_block("end");

            // br
            std::cout << "\tbr " << get_koopa_symbol();
            std::cout << ", " << then_bn << ", " << end_bn << "\n";

            // then block
            std::cout << "\n" << then_bn << ":\n";
            stmt->GenKoopa();
            std::cout << "\tjump " << end_bn << "\n";

            // end block
            std::cout << "\n" << end_bn << ":\n";
        }
};
class UnmatchedStmtAST_ifelse : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;
        std::unique_ptr<BaseAST> matched_stmt;
        std::unique_ptr<BaseAST> unmatched_stmt;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // calculate if exp
            exp->GenKoopa();

            // assign block names
            std::string then_bn = new_koopa_block("then");
            std::string else_bn = new_koopa_block("else");
            std::string end_bn = new_koopa_block("end");

            // br
            std::cout << "\tbr " << get_koopa_symbol();
            std::cout << ", " << then_bn << ", " << else_bn << "\n";

            // then block
            std::cout << "\n" << then_bn << ":\n";
            matched_stmt->GenKoopa();
            std::cout << "\tjump " << end_bn << "\n";

            // else block
            std::cout << "\n" << else_bn << ":\n";
            unmatched_stmt->GenKoopa();
            std::cout << "\tjump " << end_bn << "\n";

            // end block
            std::cout << "\n" << end_bn << ":\n";
        }
};

class StmtAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> some_stmt;

        void Dump() const override {

        } 

        void GenKoopa() override {
            some_stmt->GenKoopa();
        }
};

// ConstExp AST
class ConstExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {

        } 

        void GenKoopa() override {
            exp->GenKoopa();
        }

        int GetValue() override {
            return exp->GetValue();
        }
};

// ExpList AST
class ExpListAST_emp : public BaseAST {
    public:

        void Dump() const override {

        } 

        void GenKoopa() override {

        }
};
class ExpListAST_lst : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp_list;
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {

        } 

        void GenKoopa() override {
            exp_list->GenKoopa();
            exp->GenKoopa();
        }
};

// Exp AST
class ExpAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> some_exp;

        void Dump() const override {
            std::cout << "ExpAST {\n";
            some_exp->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            some_exp->GenKoopa();
        }

        int GetValue() override {
            return some_exp->GetValue();
        }
};

// PrimaryExp AST
class PrimaryExpAST_exp : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Dump () const override {
            std::cout << "PrimaryExpAST_exp {\n";
            exp->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            exp->GenKoopa();
        }

        int GetValue() override {
            return exp->GetValue();
        }
};
class PrimaryExpAST_num : public BaseAST {
    public:
        int number;

        void Dump () const override {
            std::cout << "PrimaryExpAST_num {\n";
            std::cout << number;
            std::cout << "\n}";
        }

        void GenKoopa() override {
            proc_const.first = true;
            proc_const.second = number;
        }

        int GetValue() override {
            return number;
        }
};
class PrimaryExpAST_val : public BaseAST {
    public:
        std::unique_ptr<BaseAST> l_val;

        void Dump() const override {

        } 

        void GenKoopa() override {
            var_mode = "load";
            l_val->GenKoopa();
            var_mode = "none";
        }

        int GetValue() override {
            return l_val->GetValue();
        }
};

// UnaryExp AST
class UnaryExpAST_pri : public BaseAST {
    public:
        std::unique_ptr<BaseAST> primary_exp;

        void Dump () const override {
            std::cout << "UnaryExpAST_pri {\n";
            primary_exp->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            primary_exp->GenKoopa();
        }

        int GetValue() override {
            return primary_exp->GetValue();
        }
};
class UnaryExpAST_uop : public BaseAST {
    public:
        std::unique_ptr<BaseAST> unary_op;
        std::unique_ptr<BaseAST> unary_exp;

        void Dump () const override {
            std::cout << "UnaryExpAST_uop {\n";
            unary_op->Dump();
            std::cout << ", ";
            unary_exp->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            unary_exp->GenKoopa();
            unary_op->GenKoopa();
        }

        int GetValue() override {
            std::string op = unary_op->GetOp();
            if (op == "+") {
                return unary_exp->GetValue();
            }
            else if (op == "-") {
                return -unary_exp->GetValue();
            }
            else if (op == "!") {
                return !unary_exp->GetValue();
            }
            assert(false);
            return 0;
        }
};

// UnaryOp AST
class UnaryOpAST : public BaseAST {
    public:
        std::string op;

        void Dump () const override {
            std::cout << "UnaryOpAST { " << op << "}";
        }

        void GenKoopa() override {
            if (op == "+") {
                return;
            }

            std::string sym_name = get_koopa_symbol();
            if (op == "+") {
            }
            else if (op == "-") {
                new_koopa_symbol();
                std::cout << " = sub 0, " << sym_name << "\n";
            }
            else if (op == "!") {
                new_koopa_symbol();
                std::cout << " = eq " << sym_name << ", 0\n";
            }
        }

        std::string GetOp() override {
            return op;
        }
};

// MulExp AST
class MulExpAST_una : public BaseAST {
    public:
        std::unique_ptr<BaseAST> unary_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            unary_exp->GenKoopa();
        }

        int GetValue() override {
            return unary_exp->GetValue();
        }
};
class MulExpAST_mul : public BaseAST {
    public:
        std::unique_ptr<BaseAST> mul_exp;
        std::string op;
        std::unique_ptr<BaseAST> unary_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            // right exp
            unary_exp->GenKoopa();
            std::string rsym_name = get_koopa_symbol();

            // left exp
            mul_exp->GenKoopa();
            std::string lsym_name = get_koopa_symbol();

            // combine two exps
            if (op == "*") {
                new_koopa_symbol();
                std::cout << " = mul " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == "/") {
                new_koopa_symbol();
                std::cout << " = div " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == "%") {
                new_koopa_symbol();
                std::cout << " = mod " << lsym_name << ", " << rsym_name << "\n";
            }
        }

        int GetValue() override {
            if (op == "*") {
                return mul_exp->GetValue() * unary_exp->GetValue();
            }
            else if (op == "/") {
                return mul_exp->GetValue() / unary_exp->GetValue();
            }
            else if (op == "%") {
                return mul_exp->GetValue() % unary_exp->GetValue();
            }
            assert(false);
            return 0;
        }
};

// AddExp AST
class AddExpAST_mul : public BaseAST {
    public:
        std::unique_ptr<BaseAST> mul_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            mul_exp->GenKoopa();
        }

        int GetValue() override {
            return mul_exp->GetValue();
        }
};
class AddExpAST_add : public BaseAST {
    public:
        std::unique_ptr<BaseAST> add_exp;
        std::string op;
        std::unique_ptr<BaseAST> mul_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            // right exp
            mul_exp->GenKoopa();
            std::string rsym_name = get_koopa_symbol();

            // left exp
            add_exp->GenKoopa();
            std::string lsym_name = get_koopa_symbol();

            if (op == "+") {
                new_koopa_symbol();
                std::cout << " = add " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == "-") {
                new_koopa_symbol();
                std::cout << " = sub " << lsym_name << ", " << rsym_name << "\n";
            }
        }

        int GetValue() override {
            if (op == "+") {
                return add_exp->GetValue() + mul_exp->GetValue();
            }
            else if (op == "-") {
                return add_exp->GetValue() - mul_exp->GetValue();
            }
            assert(false);
            return 0;
        }
};

// RelExp AST
class RelExpAST_add : public BaseAST {
    public:
        std::unique_ptr<BaseAST> add_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            add_exp->GenKoopa();
        }

        int GetValue() override {
            return add_exp->GetValue();
        }
};
class RelExpAST_rel : public BaseAST {
    public:
        std::unique_ptr<BaseAST> rel_exp;
        std::string op;
        std::unique_ptr<BaseAST> add_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            // right exp
            add_exp->GenKoopa();
            std::string rsym_name = get_koopa_symbol();

            // left exp
            rel_exp->GenKoopa();
            std::string lsym_name = get_koopa_symbol();

            if (op == "<") {
                new_koopa_symbol();
                std::cout << " = lt " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == ">") {
                new_koopa_symbol();
                std::cout << " = gt " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == "<=") {
                new_koopa_symbol();
                std::cout << " = le " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == ">=") {
                new_koopa_symbol();
                std::cout << " = ge " << lsym_name << ", " << rsym_name << "\n";
            }
        }

        int GetValue() override {
            if (op == "<") {
                return rel_exp->GetValue() < add_exp->GetValue();
            }
            else if (op == ">") {
                return rel_exp->GetValue() > add_exp->GetValue();
            }
            else if (op == "<=") {
                return rel_exp->GetValue() <= add_exp->GetValue();
            }
            else if (op == ">=") {
                return rel_exp->GetValue() >= add_exp->GetValue();
            }
            assert(false);
            return 0;
        }
};

// EqExp AST
class EqExpAST_rel : public BaseAST {
    public:
        std::unique_ptr<BaseAST> rel_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            rel_exp->GenKoopa();
        }

        int GetValue() override {
            return rel_exp->GetValue();
        }
};
class EqExpAST_eq : public BaseAST {
    public:
        std::unique_ptr<BaseAST> eq_exp;
        std::string op;
        std::unique_ptr<BaseAST> rel_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            // right exp
            rel_exp->GenKoopa();
            std::string rsym_name = get_koopa_symbol();

            // left exp
            eq_exp->GenKoopa();
            std::string lsym_name = get_koopa_symbol();

            if (op == "==") {
                new_koopa_symbol();
                std::cout << " = eq " << lsym_name << ", " << rsym_name << "\n";
            }
            else if (op == "!=") {
                new_koopa_symbol();
                std::cout << " = ne " << lsym_name << ", " << rsym_name << "\n";
            }
        }

        int GetValue() override {
            if (op == "==") {
                return eq_exp->GetValue() == rel_exp->GetValue();
            }
            else if (op == "!=") {
                return eq_exp->GetValue() != rel_exp->GetValue();
            }
            assert(false);
            return 0;
        }
};

// LAndExp AST
class LAndExpAST_eq : public BaseAST {
    public:
        std::unique_ptr<BaseAST> eq_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            eq_exp->GenKoopa();
        }

        int GetValue() override {
            return eq_exp->GetValue();
        }
};
class LAndExpAST_and : public BaseAST {
    public:
        std::unique_ptr<BaseAST> land_exp;
        std::unique_ptr<BaseAST> eq_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            // right exp
            eq_exp->GenKoopa();
            std::string rsym_name = get_koopa_symbol();

            // left exp
            land_exp->GenKoopa();
            std::string lsym_name = get_koopa_symbol();

            new_koopa_symbol();
            std::cout << " = ne " << lsym_name << ", 0\n";
            lsym_name = get_koopa_symbol();

            new_koopa_symbol();
            std::cout << " = ne " << rsym_name << ", 0\n";
            rsym_name = get_koopa_symbol();

            new_koopa_symbol();
            std::cout << " = and " << lsym_name << ", " << rsym_name << "\n";
        }

        int GetValue() override {
            return land_exp->GetValue() && eq_exp->GetValue();
        }
};

// LorExp AST
class LOrExpAST_and : public BaseAST {
    public:
        std::unique_ptr<BaseAST> land_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            land_exp->GenKoopa();
        }

        int GetValue() override {
            return land_exp->GetValue();
        }
};
class LOrExpAST_or : public BaseAST {
    public:
        std::unique_ptr<BaseAST> lor_exp;
        std::unique_ptr<BaseAST> land_exp;

        void Dump() const override {
        }

        void GenKoopa() override {
            // right exp
            land_exp->GenKoopa();
            std::string rsym_name = get_koopa_symbol();

            // left exp
            lor_exp->GenKoopa();
            std::string lsym_name = get_koopa_symbol();

            new_koopa_symbol();
            std::cout << " = ne " << lsym_name << ", 0\n";
            lsym_name = get_koopa_symbol();

            new_koopa_symbol();
            std::cout << " = ne " << rsym_name << ", 0\n";
            rsym_name = get_koopa_symbol();

            new_koopa_symbol();
            std::cout << " = or " << lsym_name << ", " << rsym_name << "\n";
        }

        int GetValue() override {
            return lor_exp->GetValue() || land_exp->GetValue();
        }
};

// Decl AST
class DeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> some_decl;

        void Dump() const override {

        } 

        void GenKoopa() override {
            some_decl->GenKoopa();
        }
};

// ConstDecl AST
class ConstDeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> const_def_list;

        void Dump() const override {

        } 

        void GenKoopa() override {
            const_def_list->GenKoopa();
        }
};

// ConstDefList AST
class ConstDefListAST_def : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_def;

        void Dump() const override {

        } 

        void GenKoopa() override {
            const_def->GenKoopa();
        }
};
class ConstDefListAST_lst : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_def_list;
        std::unique_ptr<BaseAST> const_def;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // from left to right
            const_def_list->GenKoopa();
            const_def->GenKoopa();
        }
};

// BType AST
class BTypeAST : public BaseAST {
    public:
        const std::string koopa_string = "i32";

        void Dump() const override {

        } 

        void GenKoopa() override {

        }
};

// ConstDef AST
class ConstDefAST : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> const_init_val;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // insert new const symbol to symtab
            sym_info_t info = const_init_val->GetValue();
            update_current_symtab(ident, info);
        }
};

// ConstInitVal AST
class ConstInitValAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> const_exp;

        void Dump() const override {

        } 

        void GenKoopa() override {
            const_exp->GenKoopa();
        }

        int GetValue() override {
            return const_exp->GetValue();
        }
};

// LVal AST
class LValAST : public BaseAST {
    public:
        std::string ident;

        void Dump() const override {

        } 

        void GenKoopa() override {
            sym_info_t info = current_symtab->get_sym_value(ident);
            if (var_mode == "store") {
                // generate koopa code for store to variable
                if (info.index() == 1) { // var
                    std::string sym_name = get_koopa_symbol();
                    std::cout << "\tstore " << sym_name << ", " << std::get<std::string>(info) << "\n";
                }
                else { // store to const, wrong senmatics
                    assert(false);
                }
            }
            else if (var_mode == "load") {
                // generate koopa code for load to variable
                if (info.index() == 0) { // const
                    proc_const.first = true;
                    proc_const.second = std::get<int>(info);
                }
                else if (info.index() == 1){ // var
                    new_koopa_symbol();
                    std::cout << " = load " << std::get<std::string>(info) << "\n";
                }
                else {
                    assert(false);
                }
            }
            
        }

        int GetValue() override {
            // search from symbol table by ident to get value
            sym_info_t info = current_symtab->get_sym_value(ident);
            if (info.index() == 0) {
                int value = std::get<int>(info);
                return value;
            }
            else {
                assert(false);
                return 0;
            }
        }
};

// VarDecl AST
class VarDeclAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> b_type;
        std::unique_ptr<BaseAST> var_def_list;

        void Dump() const override {

        } 

        void GenKoopa() override {
            var_def_list->GenKoopa();
        }
};

// VarDefList AST
class VarDefListAST_def : public BaseAST {
    public:
        std::unique_ptr<BaseAST> var_def;

        void Dump() const override {

        } 

        void GenKoopa() override {
            var_def->GenKoopa();
        }
};
class VarDefListAST_lst : public BaseAST {
    public:
        std::unique_ptr<BaseAST> var_def_list;
        std::unique_ptr<BaseAST> var_def;

        void Dump() const override {

        } 

        void GenKoopa() override {
            var_def_list->GenKoopa();
            var_def->GenKoopa();
        }
};
// VarDef AST
class VarDefAST_dec : public BaseAST {
    public:
        std::string ident;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // insert alloc name to symtab
            std::string alloc_name = "@" + ident + "_" + std::to_string(current_symtab->index);
            sym_info_t info = alloc_name;
            update_current_symtab(ident, info);

            // generate koopa code
            std::cout << alloc_name << " = alloc i32\n";
        }
};
class VarDefAST_def : public BaseAST {
    public:
        std::string ident;
        std::unique_ptr<BaseAST> init_val;

        void Dump() const override {

        } 

        void GenKoopa() override {
            // insert alloc name to symtab
            std::string alloc_name = "@" + ident + "_" + std::to_string(current_symtab->index);
            sym_info_t info = alloc_name;
            update_current_symtab(ident, info);

            // generate koopa code for alloc
            std::cout << "\t" << alloc_name << " = alloc i32\n";

            // generate koopa code for init_val calculation
            init_val->GenKoopa();

            // store result
            std::cout << "\tstore " << get_koopa_symbol() << ", " << alloc_name << "\n";
            proc_const.first = false;
        }
};
// InitVal AST
class InitValAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {

        } 

        void GenKoopa() override {
            exp->GenKoopa();
        }

        int GetValue() override {
            return exp->GetValue();
        }
};

// Number is not an AST type in this implementation
// class NumberAST : public BaseAST {
//     public:
//         int int_const;

//         void Dump() const override {
//             std::cout << int_const;
//         }

//         void GenKoopa() override {

//         }
// };
