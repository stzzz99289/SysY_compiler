#include <cassert>
#include <fstream>
#include <vector>
#include <map>
#include "visit.hpp"
#include "koopa.h"
#include "iostream"

// constant
#define TEMPREG_NUM 15

// temp register list
std::vector<std::string>
tempreg_lst = {"t0", "t1", "t2", "t3", "t4", "t5", "t6",
  "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
int used_tempreg_count = 0;

// map: koopa value -> risv temp reg name
std::map<koopa_raw_value_t, std::string>
koopa_tempreg_map;

// visit raw program
void Visit(const koopa_raw_program_t &program) {
  std::cout << "\t.text\n";

  Visit(program.values);
  Visit(program.funcs);
}

// visit raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        assert(false);
    }
  }
}

// visit function
void Visit(const koopa_raw_function_t &func) {
  std::cout << "\t.globl " << (func->name)+1 << "\n";
  std::cout << (func->name)+1 << ":\n";

  Visit(func->bbs);
}

// visit basic blocks
void Visit(const koopa_raw_basic_block_t &bb) {
  Visit(bb->insts);
}

// visit values
void Visit(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      Visit(kind.data.binary);
      koopa_tempreg_map[value] = current_tempreg();
      break;
    default:
      std::cout << kind.tag << "\n";
      // assert(false);
  }
}

// return
void Visit(const koopa_raw_return_t &ret) {
  // put return value to register a0, according to value type
  if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
  {
    std::cout << "\tli a0, " << ret.value->kind.data.integer.value;
    std::cout << "\n";
  }
  else {
    std::cout << "\tmv a0, " << current_tempreg();
    std::cout << "\n";
  }

  // ret instruction
  std::cout << "\tret\n";
}

// integer
void Visit(const koopa_raw_integer_t &integer) {
  if (integer.value == 0) {
    std::cout << "x0";
  }
  else {
    std::cout << integer.value;
  }
}

// binary
void Visit(const koopa_raw_binary_t &binary) {
  const auto &lhs = binary.lhs;
  const auto &rhs = binary.rhs;
  switch (binary.op) {
    case KOOPA_RBO_EQ:
      generate_eq(lhs, rhs);
      break;
    case KOOPA_RBO_MUL:
      generate_mul(lhs, rhs);
      break;
    case KOOPA_RBO_DIV:
      generate_div(lhs, rhs);
      break;
    case KOOPA_RBO_MOD:
      generate_mod(lhs, rhs);
      break;
    case KOOPA_RBO_ADD:
      generate_add(lhs, rhs);
      break;
    case KOOPA_RBO_SUB:
      generate_sub(lhs, rhs);
      break;
    default:
      assert(false);
  }
}

// generate riscv code for binary ops
void generate_eq(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  if (rhs->kind.tag == KOOPA_RVT_INTEGER && rhs->kind.data.integer.value == 0) {
      // '!' operator for constant
      load_constant(lhs);
      std::cout << "\txor ";
      new_riscv_tempreg();
      std::cout << ", ";
      riscv_by_koopa(lhs);
      std::cout << ", x0" << "\n";
      std::cout << "\tseqz " << current_tempreg() << ", " << current_tempreg() << "\n";
  }
  else {
    // not implemented yet
    assert(false);
  }
}

void generate_mul(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  load_constant(lhs);
  load_constant(rhs);
  std::cout << "\tmul ";
  new_riscv_tempreg();
  std::cout << ", ";
  riscv_by_koopa(lhs);
  std::cout << ", ";
  riscv_by_koopa(rhs);
  std::cout << "\n";
}

void generate_div(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  load_constant(lhs);
  load_constant(rhs);
  std::cout << "\tdiv ";
  new_riscv_tempreg();
  std::cout << ", ";
  riscv_by_koopa(lhs);
  std::cout << ", ";
  riscv_by_koopa(rhs);
  std::cout << "\n";
}

void generate_mod(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  load_constant(lhs);
  load_constant(rhs);
  std::cout << "\trem ";
  new_riscv_tempreg();
  std::cout << ", ";
  riscv_by_koopa(lhs);
  std::cout << ", ";
  riscv_by_koopa(rhs);
  std::cout << "\n";
}

void generate_add(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs){
  load_constant(lhs);
  load_constant(rhs);
  std::cout << "\tadd ";
  new_riscv_tempreg();
  std::cout << ", ";
  riscv_by_koopa(lhs);
  std::cout << ", ";
  riscv_by_koopa(rhs);
  std::cout << "\n";
}

void generate_sub(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  load_constant(lhs);
  load_constant(rhs);
  std::cout << "\tsub ";
  new_riscv_tempreg();
  std::cout << ", ";
  riscv_by_koopa(lhs);
  std::cout << ", ";
  riscv_by_koopa(rhs);
  std::cout << "\n";
}

// helper functions
void load_constant(const koopa_raw_value_t &value) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    int constant = value->kind.data.integer.value;
    // if (constant == 0) {
    //   std::cout << "\tli ";
    //   new_riscv_tempreg();
    //   std::cout << ", x0" << "\n";
    //   return;
    // }
    std::cout << "\tli ";
    new_riscv_tempreg();
    std::cout << ", " << constant << "\n";
    koopa_tempreg_map[value] = current_tempreg();
  }
  else {
    return;
  }
}

void new_riscv_tempreg() {
  // TODO: better temp reg allocation method
  assert(used_tempreg_count < TEMPREG_NUM);
  std::cout << tempreg_lst.at(used_tempreg_count);
  used_tempreg_count = used_tempreg_count + 1;
}

std::string current_tempreg() {
  assert(used_tempreg_count >= 1);
  return tempreg_lst.at(used_tempreg_count - 1);
}

void riscv_by_koopa(const koopa_raw_value_t &value) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    if (value->kind.data.integer.value == 0) {
      std::cout << "x0";
    }
    else {
      std::cout << koopa_tempreg_map.find(value)->second;
    }
  }
  else if (value->kind.tag == KOOPA_RVT_BINARY) {
    std::cout << koopa_tempreg_map.find(value)->second;
  }
  else {
    assert(false);
  }
}

// [example]
// handle raw program
// for (size_t i = 0; i < raw.funcs.len; ++i) {
//   assert(raw.funcs.kind == KOOPA_RSIK_FUNCTION);
//   koopa_raw_function_t func = (koopa_raw_function_t) raw.funcs.buffer[i];

//   for (size_t j = 0; j < func->bbs.len; ++j) {
//     assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
//     koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t) func->bbs.buffer[j];

//     for (size_t k = 0; k < bb->insts.len; ++k) {
//       assert(bb->insts.kind == KOOPA_RSIK_VALUE);

//       koopa_raw_value_t value = (koopa_raw_value_t) bb->insts.buffer[k];
//       assert(value->kind.tag == KOOPA_RVT_RETURN);

//       koopa_raw_value_t ret_value = value->kind.data.ret.value;
//       assert(ret_value->kind.tag == KOOPA_RVT_INTEGER);

//       int32_t int_val = ret_value->kind.data.integer.value;
//       assert(int_val == 0);
//     }
//   }
// }