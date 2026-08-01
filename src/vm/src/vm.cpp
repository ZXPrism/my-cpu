#include <cassert>
#include <format>
#include <iostream>
#include <random>
#include <stdexcept>

#include <vm.h>

namespace mycpu {

VirtualMachine::VirtualMachine() {
	reg_file[0] = 0;

	// randomly fill reg file
	std::random_device rd;
	std::mt19937_64 engine(rd());
	std::uniform_int_distribution<u32> dist16(0, (1 << 16) - 1);
	for (int i = 1; i < REG_COUNT; i++) {
		reg_file[i] = static_cast<u16>(dist16(engine));
	}

	// randomly fill memory
	std::uniform_int_distribution<u32> dist8(0, (1 << 8) - 1);
	for (int i = 0; i < MEM_SIZE_BYTES; i++) {
		mem[i] = static_cast<u8>(dist8(engine));
	}

	// test program: fib sequence
	load_instruction(Instruction(OpCode::XOR, 1, 1, 1), 0);   // counter
	load_instruction(Instruction(OpCode::LI, 2, 0, 1), 2);    // a
	load_instruction(Instruction(OpCode::LI, 3, 0, 1), 4);    // b
	load_instruction(Instruction(OpCode::XOR, 4, 4, 4), 6);   // c
	load_instruction(Instruction(OpCode::LI, 5, 0, 1), 8);    // "1"
	load_instruction(Instruction(OpCode::XOR, 6, 6, 6), 10);  // condition=False
	load_instruction(Instruction(OpCode::LI, 7, 1, 0), 12);   // jmp target
	load_instruction(Instruction(OpCode::LI, 8, 0, 10), 14);  // "10"

	load_instruction(Instruction(OpCode::ADD, 4, 2, 3), 16);   // c = a + b
	load_instruction(Instruction(OpCode::ADD, 2, 0, 3), 18);   // a = b
	load_instruction(Instruction(OpCode::ADD, 3, 0, 4), 20);   // b = c
	load_instruction(Instruction(OpCode::ADD, 15, 0, 4), 22);  // mov c to result
	load_instruction(Instruction(OpCode::INT, 0, 0, 0), 24);
	load_instruction(Instruction(OpCode::ADD, 1, 1, 5), 26);   // counter += 1
	load_instruction(Instruction(OpCode::SLTU, 6, 1, 8), 28);  // counter < 10?
	load_instruction(Instruction(OpCode::BAL, 6, 7, 0), 30);   // jmp if counter < 10
}

void VirtualMachine::dump_regs() const {
	std::cout << "=======\n";
	for (int i = 0; i < REG_COUNT; i++) {
		std::cout << std::format("REG #{:02}: {:5}\n", i, reg_file[i]);
	}
	std::cout << "=======\n\n";
}

bool VirtualMachine::exec(Instruction inst) {
	u8 field_a = inst.get_field_a();
	u8 field_b = inst.get_field_b();
	u8 field_c = inst.get_field_c();

	u8 imm = 42;
	u8 dest = REG_COUNT;
	u8 src = REG_COUNT;
	u8 src_1 = REG_COUNT;
	u8 src_2 = REG_COUNT;
	u8 shift = REG_COUNT;
	u8 link = REG_COUNT;
	u8 jmp_target = REG_COUNT;
	u8 condition = REG_COUNT;
	u8 service_id = REG_COUNT;

	u16 tmp = 42;

	switch (inst.get_opcode()) {
	case OpCode::ADD:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src_1] + reg_file[src_2];
		}

		break;
	case OpCode::SUB:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src_1] - reg_file[src_2];
		}

		break;
	case OpCode::SLL:
		dest = field_a;
		src = field_b;
		shift = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src] << (reg_file[shift] & 0xF);
		}

		break;
	case OpCode::SRL:
		dest = field_a;
		src = field_b;
		shift = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src] >> (reg_file[shift] & 0xF);
		}

		break;
	case OpCode::SRA:
		dest = field_a;
		src = field_b;
		shift = field_c;

		if (dest != 0) {
			reg_file[dest] = static_cast<u16>(static_cast<i16>(reg_file[src]) >> (reg_file[shift] & 0xF));
		}

		break;
	case OpCode::SLT:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			if (static_cast<i16>(reg_file[src_1]) < static_cast<i16>(reg_file[src_2])) {
				reg_file[dest] = 1;
			} else {
				reg_file[dest] = 0;
			}
		}

		break;
	case OpCode::SLTU:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			if (reg_file[src_1] < reg_file[src_2]) {
				reg_file[dest] = 1;
			} else {
				reg_file[dest] = 0;
			}
		}

		break;
	case OpCode::XOR:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src_1] ^ reg_file[src_2];
		}

		break;
	case OpCode::OR:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src_1] | reg_file[src_2];
		}

		break;
	case OpCode::AND:
		dest = field_a;
		src_1 = field_b;
		src_2 = field_c;

		if (dest != 0) {
			reg_file[dest] = reg_file[src_1] & reg_file[src_2];
		}

		break;
	case OpCode::STR:
		src = field_a;
		dest = field_b;

		if (reg_file[dest] % 2 == 1) {
			throw std::runtime_error("Destination address for STR instruction should be even");
		}

		mem[reg_file[dest]] = reg_file[src] & 0xFF;
		mem[reg_file[dest] + 1] = (reg_file[src] >> 8) & 0xFF;

		break;
	case OpCode::LDR:
		dest = field_a;
		src = field_b;

		if (reg_file[src] % 2 == 1) {
			throw std::runtime_error("Source address for LDR instruction should be even");
		}

		if (dest != 0) {
			reg_file[dest] = mem[reg_file[src]] + (static_cast<u16>(mem[reg_file[src] + 1]) << 8);
		}

		break;
	case OpCode::LI:
		dest = field_a;
		imm = static_cast<u8>((field_b << 4) + field_c);

		if (dest != 0) {
			reg_file[dest] = imm;
		}

		break;
	case OpCode::JAL:
		link = field_a;
		jmp_target = field_b;

		tmp = reg_file[jmp_target];
		if (link != 0) {
			reg_file[link] = program_counter + 2;
		}
		program_counter = tmp;

		return false;

		break;
	case OpCode::BAL:
		condition = field_a;
		jmp_target = field_b;
		link = field_c;

		if (reg_file[condition] == 1) {
			tmp = reg_file[jmp_target];
			if (link != 0) {
				reg_file[link] = program_counter + 2;
			}
			program_counter = tmp;

			return false;
		}

		break;
	case OpCode::INT:
		service_id = field_a;
		if (reg_file[service_id] == 0) {
			std::cout << reg_file[15] << '\n';
		}

		break;
	}

	return true;
}

void VirtualMachine::run() {
	while (program_counter < 31) {
		u16 instruction_data =
		    static_cast<u16>(mem[program_counter]) +
		    (static_cast<u16>(mem[program_counter + 1]) << 8);
		auto curr_inst = Instruction(instruction_data);
		if (exec(curr_inst)) {
			program_counter += 2;
		}
	}
}

u16 VirtualMachine::get_reg(u8 reg_idx) const {
	assert(reg_idx < REG_COUNT);
	return reg_file[reg_idx];
}

u8 VirtualMachine::get_mem(u16 addr) const {
	return mem[addr];
}

u16 VirtualMachine::get_pc() const {
	return program_counter;
}

void VirtualMachine::load_instruction(Instruction inst, u16 addr) {
	assert(addr % 2 == 0);
	mem[addr] = inst.data & 0xFF;
	mem[addr + 1] = (inst.data >> 8) & 0xFF;
}

}  // namespace mycpu
