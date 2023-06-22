#pragma once

#include "koopa.h"
#include <vector>

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
void Visit(const koopa_raw_store_t &store);
void Visit(const koopa_raw_load_t &load);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_jump_t &jump);

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
void load_value(const koopa_raw_value_t &value);
void new_riscv_tempreg();
std::string current_tempreg();
void riscv_by_koopa(const koopa_raw_value_t &value);
int offset_by_koopa(const koopa_raw_value_t &value);
void generate_bin_riscv(std::string riscv, const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs);
std::string set_visit_mode (std::string new_mode);
void reset_visit_mode (std::string old_mode);