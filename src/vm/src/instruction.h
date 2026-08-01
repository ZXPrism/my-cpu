#pragma once

#include <type.h>

namespace mycpu {

enum class OpCode : u8 {
	ADD,
	SUB,
	SLL,
	SRL,
	SRA,
	SLT,
	SLTU,
	XOR,
	OR,
	AND,
	STR,
	LDR,
	LI,
	JAL,
	BAL,
	INT,
};

struct Instruction {
public:
	Instruction(OpCode opcode, u8 field_a, u8 field_b, u8 field_c);
	explicit Instruction(u16 data);

	[[nodiscard]] OpCode get_opcode() const;
	[[nodiscard]] u8 get_field_a() const;
	[[nodiscard]] u8 get_field_b() const;
	[[nodiscard]] u8 get_field_c() const;

	u16 data;
};

}  // namespace mycpu
