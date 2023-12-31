#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <string>
#include <string.h>
#include "ast.hpp"
#include "koopa.h"
#include "visit.hpp"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

int main(int argc, const char *argv[]) {
  // compiler [mode] [infile] -o [outfile]
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // infile -> lexer
  yyin = fopen(input, "r");
  assert(yyin);

  // parser -> AST
  unique_ptr<BaseAST> ast;
  auto ret_parser = yyparse(ast);
  assert(!ret_parser);

  // (debug) AST -> EBNF str
  // ast->Dump();

  // AST -> Koopa str
  stringstream koopa_stream;
  auto coutbuf = std::cout.rdbuf(koopa_stream.rdbuf());
  ast->sym_num = 0;
  ast->GenKoopa();
  std::cout.rdbuf(coutbuf);

  // -koopa mode
  if (strcmp(mode, "-koopa") == 0) {
    ofstream koopa_outfile(output);
    koopa_outfile << koopa_stream.rdbuf();
    koopa_outfile.close();

    return 0;
  }

  // Koopa IR string -> Koopa IR char array
  // std::ifstream koopa_file(output);
  // koopa_sstr << koopa_file.rdbuf();
  std::string koopa_str = koopa_stream.str();
  const char *koopa_chararr = koopa_str.c_str();

  // Koopa IR char array -> Koopa IR program
  koopa_program_t program;
  koopa_error_code_t ret_koopa = koopa_parse_from_string(koopa_chararr, &program);
  assert(ret_koopa == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);

  // Koopa IR program -> RISC-V program
  ofstream riscv_outfile(output);
  coutbuf = std::cout.rdbuf(riscv_outfile.rdbuf());
  Visit(raw);
  std::cout.rdbuf(coutbuf);

  // release memory of raw program builder
  koopa_delete_raw_program_builder(builder);

  return 0;
}
