#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

/*
Current EBNF:
CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt      ::= "return" Exp ";";

Exp         ::= UnaryExp | AddExp;
PrimaryExp  ::= "(" Exp ")" | Number;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp;
UnaryOp     ::= "+" | "-" | "!";
MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;

Number      ::= INT_CONST;
*/

/*
lv3.3 EBNF:
Exp         ::= LOrExp;
PrimaryExp  ::= ...;
Number      ::= ...;
UnaryExp    ::= ...;
UnaryOp     ::= ...;
MulExp      ::= ...;
AddExp      ::= ...;
RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
*/

// Base class for all ASTs
class BaseAST {
    public:
        virtual ~BaseAST() = default;

        virtual void Dump() const = 0;
        virtual void GenKoopa() = 0;

        std::string get_koopa_symbol() {
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

        void new_koopa_symbol() {
            std::cout << "\t\%" << sym_num;
            sym_num = sym_num + 1;
        }

        static int sym_num; // next symbol number
        static int lsym_num; // symbol number where stores right op
        static int rsym_num; // symbol number where stores left op
        static std::pair<bool, int> proc_const; // bool: processing const or not; int: const value
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
            block->GenKoopa();
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
        std::unique_ptr<BaseAST> stmt;

        void Dump() const override {
            std::cout << "BlockAST {\n";
            stmt->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            std::cout << " {\n";
            std::cout << "\%entry:\n";
            stmt->GenKoopa();
            std::cout << "\n}";
        }
};

// Stmt AST
class StmtAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> exp;

        void Dump() const override {
            std::cout << "StmtAST {\n";
            exp->Dump();
            std::cout << "\n}";
        }

        void GenKoopa() override {
            exp->GenKoopa();

            std::string sym_name = get_koopa_symbol();
            std::cout << "\tret " << sym_name;
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
