#pragma once

#include <cassert>

#include <instruction.h>
#include <type.h>

namespace mycpu {

Instruction::Instruction(OpCode opcode, u8 field_a, u8 field_b, u8 field_c) {
	assert(field_a < 16);
	assert(field_b < 16);
	assert(field_c < 16);
	data = (static_cast<u16>(opcode) << 12) +
	       (static_cast<u16>(field_a) << 8) +
	       (static_cast<u16>(field_b) << 4) +
	       static_cast<u16>(field_c);
}

Instruction::Instruction(u16 data)
    : data(data) {
}

OpCode Instruction::get_opcode() const {
	return static_cast<OpCode>(data >> 12);
}

u8 Instruction::get_field_a() const {
	return static_cast<u8>((data >> 8) & 0xF);
}

u8 Instruction::get_field_b() const {
	return static_cast<u8>((data >> 4) & 0xF);
}

u8 Instruction::get_field_c() const {
	return static_cast<u8>(data & 0xF);
}

}  // namespace mycpu
