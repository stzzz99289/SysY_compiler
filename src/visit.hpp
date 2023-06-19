#pragma once

#include "koopa.h"
#include <vector>

// temp register list
// std::vector<std::string> tempreg_lst;
// int used_tempreg_count;

// basic visit
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);

// value visit
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_binary_t &binary);

// generate riscv code for binary ops
void generate_eq(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_neq(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_mul(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_div(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_mod(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_add(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_sub(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_lt(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_gt(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_le(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_ge(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_and(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
void generate_or(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);

// helper functions
void load_constant(const koopa_raw_value_t &value);
void new_riscv_tempreg();
std::string current_tempreg();
void riscv_by_koopa(const koopa_raw_value_t &value);