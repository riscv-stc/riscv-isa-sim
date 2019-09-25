// See LICENSE for license details.

#include "insn_template.h"

reg_t rv32_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 32;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  MCU_AUNIT();
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  RESTORE_AUNIT();
  return npc;
}

reg_t rv64_NAME(processor_t* p, insn_t insn, reg_t pc)
{
  int xlen = 64;
  reg_t npc = sext_xlen(pc + insn_length(OPCODE));
  MCU_AUNIT();
  #include "insns/NAME.h"
  trace_opcode(p, OPCODE, insn);
  RESTORE_AUNIT();
  return npc;
}
