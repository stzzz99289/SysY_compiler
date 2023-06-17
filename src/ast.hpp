#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <sstream>

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
        virtual void GenKoopa() const = 0;
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

        void GenKoopa() const override {
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
            std::cout << "FuncDefAST { ";
            func_type->Dump();
            std::cout << ", " << ident << ", ";
            block->Dump();
            std::cout << " }";
        }

        void GenKoopa() const override {
            std::cout << "fun @" << ident << "(): ";
            func_type->GenKoopa();
            block->GenKoopa();
        }
};

// FuncType AST
class FuncTypeAST : public BaseAST {
    public:
        void Dump() const override {
            std::cout << "FuncTypeAST { int }";
        }

        void GenKoopa() const override {
            std::cout << "i32";
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

        void GenKoopa() const override {
            std::cout << " {\n";
            std::cout << "\%entry:\n";
            stmt->GenKoopa();
            std::cout << "\n}";
        }
};

// Stmt AST
class StmtAST : public BaseAST {
    public:
        int number;

        void Dump() const override {
            std::cout << "StmtAST { " << number << " }";
        }

        void GenKoopa() const override {
            std::cout << "\tret " << number;
        }
};

// Number is not an AST type in this implementation
// class NumberAST : public BaseAST {
//     public:
//         int int_const;

//         void Dump() const override {
//             std::cout << int_const;
//         }
// };
