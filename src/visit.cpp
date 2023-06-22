#include <cassert>
#include <fstream>
#include <vector>
#include <map>
#include "visit.hpp"
#include "koopa.h"
#include "iostream"

// TODO: figure out logic for releasing tempregs

// constant
#define TEMPREG_NUM 15
#define S_ALIGNMENT 16

// temp register list
std::vector<std::string>
tempreg_lst = {"t0", "t1", "t2", "t3", "t4", "t5", "t6",
  "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
int used_tempreg_count = 0;

// map: koopa value -> risv temp reg name
std::map<koopa_raw_value_t, std::string>
koopa_tempreg_map = { };

// map: koopa value -> stack offset
std::map<koopa_raw_value_t, int>
koopa_offset_map = { };

// stack space needed to alloc for current funtion
int stack_s = 0;

// visit mode: 
//  1. "riscv" (visit koopa to generate riscv code); 
//  2. "stack" (visit koopa to calculate stack space allocated)
std::string visit_mode = "none";

// visit raw program
void Visit(const koopa_raw_program_t &program) {
  std::cout << "\t.text\n";

  std::string old_mode = set_visit_mode("riscv");
  Visit(program.values);
  Visit(program.funcs);
  reset_visit_mode(old_mode);
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
  // entry point
  std::cout << "\t.globl " << (func->name)+1 << "\n";
  std::cout << (func->name)+1 << ":\n";

  // calculate stack space for prologue
  stack_s = 0;
  std::string old_mode;
  old_mode = set_visit_mode("stack");
  Visit(func->bbs);
  reset_visit_mode(old_mode);

  // 16 bytes alignment (round up)
  stack_s = (stack_s + S_ALIGNMENT - 1) & ~(S_ALIGNMENT - 1);
  
  // generate prologue
  // TODO: handle outside of [-2048, 2047]
  std::cout << "\taddi sp, sp, -" << stack_s << "\n";

  // generate riscv code for basic blocks
  old_mode = set_visit_mode("riscv");
  Visit(func->bbs);
  reset_visit_mode(old_mode);
}

// visit basic blocks
void Visit(const koopa_raw_basic_block_t &bb) {
  if (visit_mode == "riscv") {
    std::string bbname = ((bb->name)+1);
    if (bbname != "entry") {
      std::cout << "\n" << (bb->name)+1 << ":\n";
    }
  }
  Visit(bb->insts);
}

// visit values
void Visit(const koopa_raw_value_t &value) {
  if (visit_mode == "riscv") {
    const auto &kind = value->kind;
    int load_offset = -1;
    switch (kind.tag) {
      case KOOPA_RVT_RETURN:
        Visit(kind.data.ret);
        break;
      case KOOPA_RVT_INTEGER:
        Visit(kind.data.integer);
        break;
      case KOOPA_RVT_BINARY:
        Visit(kind.data.binary);
        // directly save calculated value to stack
        load_offset = offset_by_koopa(value);
        std::cout << "\tsw " << current_tempreg() << ", " << load_offset << "(sp)\n";
        // release tempreg
        used_tempreg_count = used_tempreg_count - 1;
        break;
      case KOOPA_RVT_STORE:
        Visit(kind.data.store);
        break;
      case KOOPA_RVT_LOAD:
        Visit(kind.data.load);
        // directly save loaded value to stack
        load_offset = offset_by_koopa(value);
        std::cout << "\tsw " << current_tempreg() << ", " << load_offset << "(sp)\n";
        // release tempreg
        used_tempreg_count = used_tempreg_count - 1;
        break;
      case KOOPA_RVT_ALLOC:
        break;
      case KOOPA_RVT_BRANCH:
        Visit(kind.data.branch);
        break;
      case KOOPA_RVT_JUMP:
        Visit(kind.data.jump);
        break;
      default:
        std::cout << "[unexpected value kind: " << kind.tag << " at visit mode: " << visit_mode << "]\n";
        // assert(false);
    }
  }
  else if (visit_mode == "stack") {
    // allocate 4 for non-void value
    if (value->ty->tag == KOOPA_RTT_INT32) {
      koopa_offset_map[value] = stack_s;
      stack_s = stack_s + 4;
    }

    // allocate 4 for "alloc"
    if (value->kind.tag == KOOPA_RVT_ALLOC) {
      koopa_offset_map[value] = stack_s;
      stack_s = stack_s + 4;
    }
  }
}

// return
void Visit(const koopa_raw_return_t &ret) {
  if (visit_mode == "riscv") {
    // put return value to register a0, according to value type
    if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
    {
      std::cout << "\tli a0, " << ret.value->kind.data.integer.value;
      std::cout << "\n";
    }
    else {
      std::cout << "\tlw a0, " << offset_by_koopa(ret.value) << "(sp)\n";
      std::cout << "\n";
    }

    // generate epilogue
    // TODO: handle outside of [-2048, 2047]
    std::cout << "\taddi sp, sp, " << stack_s << "\n";

    // ret instruction
    std::cout << "\tret\n";
  }
}

// integer
void Visit(const koopa_raw_integer_t &integer) {
  if (visit_mode == "riscv") {
    if (integer.value == 0) {
      std::cout << "x0";
    }
    else {
      std::cout << integer.value;
    }
  }
}

// binary
void Visit(const koopa_raw_binary_t &binary) {
  if (visit_mode == "riscv") {
    const auto &lhs = binary.lhs;
    const auto &rhs = binary.rhs;
    switch (binary.op) {
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
      case KOOPA_RBO_LT:
        generate_lt(lhs, rhs);
        break;
      case KOOPA_RBO_GT:
        generate_gt(lhs, rhs);
        break;
      case KOOPA_RBO_LE:
        generate_le(lhs, rhs);
        break;
      case KOOPA_RBO_GE:
        generate_ge(lhs, rhs);
        break;
      case KOOPA_RBO_EQ:
        generate_eq(lhs, rhs);
        break;
      case KOOPA_RBO_NOT_EQ:
        generate_neq(lhs, rhs);
        break;
      case KOOPA_RBO_AND:
        generate_and(lhs, rhs);
        break;
      case KOOPA_RBO_OR:
        generate_or(lhs, rhs);
        break;
      default:
        assert(false);
    }
  }
}

// store
void Visit(const koopa_raw_store_t &store) {
  if (visit_mode == "riscv") {
    const auto &value = store.value;
    const auto &dest = store.dest;

    // load value to a new tempreg
    load_value(value);

    // find stack offset of dest
    int dest_offset = offset_by_koopa(dest);

    // generate sw instruction, directly use current tempreg to save the result
    std::cout << "\tsw " << current_tempreg() << ", " << dest_offset << "(sp)\n";

    // release tempreg
    used_tempreg_count = used_tempreg_count - 1;
  }
}

// load
void Visit(const koopa_raw_load_t &load) {
  if (visit_mode == "riscv") {
    const auto &src = load.src;
    
    // load value to a new tempreg
    load_value(src);
  }
}

// branch
void Visit(const koopa_raw_branch_t &branch) {
  const auto &cond = branch.cond;
  const auto &true_bb = branch.true_bb;
  const auto &false_bb = branch.false_bb;

  if (visit_mode == "riscv") {
    // load cond value from stack to tempreg and release the tempreg
    load_value(cond);
    used_tempreg_count = used_tempreg_count - 1;

    // branch = bnez + j
    std::cout << "\tbnez ";
    riscv_by_koopa(cond);
    std::cout << ", " << (true_bb->name)+1 << "\n";
    std::cout << "\tj " << (false_bb->name)+1 << "\n";
  }
}

// jump
void Visit(const koopa_raw_jump_t &jump) {
  const auto &target = jump.target;

  if (visit_mode == "riscv") {
    // jump = j
    std::cout << "\tj " << (target->name)+1 << "\n";
  }
}

// generate riscv code for binary ops
void generate_eq(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  if (rhs->kind.tag == KOOPA_RVT_INTEGER && rhs->kind.data.integer.value == 0) {
      load_value(lhs);

      // release tempreg
      used_tempreg_count = used_tempreg_count - 1;

      std::cout << "\txor ";
      new_riscv_tempreg();
      std::cout << ", ";
      riscv_by_koopa(lhs);
      std::cout << ", x0" << "\n";
      std::cout << "\tseqz " << current_tempreg() << ", " << current_tempreg() << "\n";
  }
  else {
    generate_bin_riscv("xor", lhs, rhs);
    std::cout << "\tseqz " << current_tempreg() << ", " << current_tempreg() << "\n";
  }
}

void generate_neq(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  if (rhs->kind.tag == KOOPA_RVT_INTEGER && rhs->kind.data.integer.value == 0) {
      // '!' operator for constant
      load_value(lhs);

      // release tempreg
      used_tempreg_count = used_tempreg_count - 1;

      std::cout << "\txor ";
      new_riscv_tempreg();
      std::cout << ", ";
      riscv_by_koopa(lhs);
      std::cout << ", x0" << "\n";
      std::cout << "\tsnez " << current_tempreg() << ", " << current_tempreg() << "\n";
  }
  else {
    generate_bin_riscv("xor", lhs, rhs);
    std::cout << "\tsnez " << current_tempreg() << ", " << current_tempreg() << "\n";
  }
}

void generate_mul(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("mul", lhs, rhs);
}

void generate_div(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("div", lhs, rhs);
}

void generate_mod(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("rem", lhs, rhs);
}

void generate_add(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs){
  generate_bin_riscv("add", lhs, rhs);
}

void generate_sub(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("sub", lhs, rhs);
}

void generate_lt(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("slt", lhs, rhs);
}

void generate_gt(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("sgt", lhs, rhs);
}

void generate_le(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("sgt", lhs, rhs);
  std::cout << "\tseqz " << current_tempreg() << ", " << current_tempreg() << "\n";
}

void generate_ge(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("slt", lhs, rhs);
  std::cout << "\tseqz " << current_tempreg() << ", " << current_tempreg() << "\n";
}

void generate_and(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("and", lhs, rhs);
}

void generate_or(const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  generate_bin_riscv("or", lhs, rhs);
}

// helper functions
void load_value(const koopa_raw_value_t &value) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    int constant = value->kind.data.integer.value;
    std::cout << "\tli ";
    new_riscv_tempreg();
    std::cout << ", " << constant << "\n";
    koopa_tempreg_map[value] = current_tempreg();
  }
  else {
    int offset = offset_by_koopa(value);
    std::cout << "\tlw ";
    new_riscv_tempreg();
    std::cout << ", " << offset << "(sp)\n";
    koopa_tempreg_map[value] = current_tempreg();
  }
}

void new_riscv_tempreg() {
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
      std::map<koopa_raw_value_t, std::string>::iterator it;
      it = koopa_tempreg_map.find(value);
      if (it == koopa_tempreg_map.end()) {
        assert(false);
      }

      std::cout << it->second;
    }
  }
  else {
    std::map<koopa_raw_value_t, std::string>::iterator it;
    it = koopa_tempreg_map.find(value);
    if (it == koopa_tempreg_map.end()) {
      assert(false);
    }

    std::cout << it->second;
  }
}

int offset_by_koopa(const koopa_raw_value_t &value) {
  std::map<koopa_raw_value_t, int>::iterator it;
  it = koopa_offset_map.find(value);
  if (it == koopa_offset_map.end()) {
    assert(false);
    return 0;
  }
  return it->second;
}

void generate_bin_riscv(std::string riscv, const koopa_raw_value_t &lhs, const koopa_raw_value_t &rhs) {
  // constant/variables needs to be loaded to tempreg
  load_value(lhs);
  load_value(rhs);

  // release all tempregs
  used_tempreg_count = used_tempreg_count - 2;

  // riscv code for binary op
  std::cout << "\t" << riscv << " ";
  new_riscv_tempreg();
  std::cout << ", ";
  riscv_by_koopa(lhs);
  std::cout << ", ";
  riscv_by_koopa(rhs);
  std::cout << "\n";
}

std::string set_visit_mode (std::string new_mode) {
  std::string old_mode = visit_mode;
  visit_mode = new_mode;
  return old_mode;
}

void reset_visit_mode (std::string old_mode) {
  visit_mode = old_mode;
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