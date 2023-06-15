#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <fstream>

/*
lv1 EBNF:
CompUnit  ::= FuncDef;

FuncDef   ::= FuncType IDENT "(" ")" Block;
FuncType  ::= "int";

Block     ::= "{" Stmt "}";
Stmt      ::= "return" Number ";";
Number    ::= INT_CONST;
*/

// Base class for all ASTs
class BaseAST {
    public:
        virtual ~BaseAST() = default;

        virtual void Dump() const = 0;
        virtual void GenKoopa(std::ofstream &outfile) const = 0;
};

// CompUnit AST
class CompUnitAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_def;

        void Dump() const override {
            std::cout << "CompUnitAST { ";
            func_def->Dump();
            std::cout << " }";
        }

        void GenKoopa(std::ofstream &outfile) const override {
            func_def->GenKoopa(outfile);
        }
};

// FuncDef AST
class FuncDefAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> func_type;
        std::string ident;
        std::unique_ptr<BaseAST> block;

        void Dump() const override {
            std::cout << "FuncDefAST { ";
            func_type->Dump();
            std::cout << ", " << ident << ", ";
            block->Dump();
            std::cout << " }";
        }

        void GenKoopa(std::ofstream &outfile) const override {
            outfile << "fun @" << ident << "(): ";
            func_type->GenKoopa(outfile);
            block->GenKoopa(outfile);
        }
};

// FuncType AST
class FuncTypeAST : public BaseAST {
    public:
        void Dump() const override {
            std::cout << "FuncTypeAST { int }";
        }

        void GenKoopa(std::ofstream &outfile) const override {
            outfile << "i32";
        }
};

// Block AST
class BlockAST : public BaseAST {
    public:
        std::unique_ptr<BaseAST> stmt;

        void Dump() const override {
            std::cout << "BlockAST { ";
            stmt->Dump();
            std::cout << " }";
        }

        void GenKoopa(std::ofstream &outfile) const override {
            outfile << " {\n";
            outfile << "\%entry:\n";
            stmt->GenKoopa(outfile);
            outfile << "\n}";
        }
};

// Stmt AST
class StmtAST : public BaseAST {
    public:
        int number;

        void Dump() const override {
            std::cout << "StmtAST { " << number << " }";
        }

        void GenKoopa(std::ofstream &outfile) const override {
            outfile << "\tret " << number;
        }
};

// ...
// class NumberAST : public BaseAST {
//     public:
//         int int_const;

//         void Dump() const override {
//             std::cout << int_const;
//         }
// };
