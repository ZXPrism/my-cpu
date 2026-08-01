#pragma once

#include <array>

#include <instruction.h>
#include <type.h>

namespace mycpu {

constexpr int REG_COUNT = 16;
constexpr int MEM_SIZE_BYTES = 1 << 16;

enum class StepStatus {
	CONTINUE,
	JUMP,
	HALT,
};

class VirtualMachine {
public:
	VirtualMachine();

	void dump_regs() const;

	StepStatus step(Instruction inst);
	void run();

	[[nodiscard]] u16 get_reg(u8 reg_idx) const;
	[[nodiscard]] u8 get_mem(u16 addr) const;
	[[nodiscard]] u16 get_pc() const;

	void load_instruction(Instruction inst, u16 addr);

private:
	u16 program_counter = 0;
	std::array<u16, REG_COUNT> reg_file;
	std::array<u8, MEM_SIZE_BYTES> mem;
};

}  // namespace mycpu
