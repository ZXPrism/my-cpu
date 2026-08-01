#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <random>
#include <ranges>
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
}

void VirtualMachine::dump_regs() const {
	std::cout << "=======\n";
	for (int i = 0; i < REG_COUNT; i++) {
		std::cout << std::format("REG #{:02}: {:5}\n", i, reg_file[i]);
	}
	std::cout << "=======\n\n";
}

StepStatus VirtualMachine::exec(Instruction inst) {
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
		if (tmp % 2 == 1) {
			throw std::runtime_error("Target address for JAL instruction should be even");
		}

		if (link != 0) {
			reg_file[link] = program_counter + 2;
		}
		program_counter = tmp;

		return StepStatus::JUMP;

		break;
	case OpCode::BAL:
		condition = field_a;
		jmp_target = field_b;
		link = field_c;

		if (reg_file[condition] == 1) {
			tmp = reg_file[jmp_target];
			if (tmp % 2 == 1) {
				throw std::runtime_error("Target address for BAL instruction should be even");
			}

			if (link != 0) {
				reg_file[link] = program_counter + 2;
			}
			program_counter = tmp;

			return StepStatus::JUMP;
		}

		break;
	case OpCode::INT:
		service_id = field_a;
		if (reg_file[service_id] == 0) {  // test: print
			std::cout << reg_file[15] << '\n';
		} else if (reg_file[service_id] == 1) {  // test: halt
			return StepStatus::HALT;
		}

		break;
	}

	return StepStatus::CONTINUE;
}

void VirtualMachine::run() {
	while (true) {
		u16 instruction_data =
		    static_cast<u16>(mem[program_counter]) +
		    (static_cast<u16>(mem[program_counter + 1]) << 8);
		auto curr_inst = Instruction(instruction_data);

		auto exec_status = exec(curr_inst);
		if (exec_status == StepStatus::CONTINUE) {
			program_counter += 2;
		} else if (exec_status == StepStatus::HALT) {
			break;
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

void VirtualMachine::load_from_dram(const std::vector<u8> &bytecode) {
	auto n_bytes = bytecode.size();
	if (n_bytes % 2 == 1) {
		throw std::runtime_error(std::format("Bytecode is invalid since byte count is {}, which is odd", n_bytes));
	}
	if (n_bytes > MEM_SIZE_BYTES) {
		throw std::runtime_error(std::format("Too many instructions, given {}, max allowed {}", n_bytes / 2, MEM_SIZE_BYTES / 2));
	}
	std::ranges::copy(bytecode, mem.begin());
}

void VirtualMachine::load_from_file(const std::string &file_path) {
	std::ifstream fin(file_path, std::ios::binary | std::ios::ate);
	if (!fin) {
		throw std::runtime_error(std::format("Could not open bytecode file at {}", file_path));
	}

	auto n_bytes = static_cast<std::size_t>(fin.tellg());
	if (n_bytes % 2 == 1) {
		throw std::runtime_error(std::format("Bytecode is invalid since byte count is {}, which is odd", n_bytes));
	}
	if (n_bytes > MEM_SIZE_BYTES) {
		throw std::runtime_error(std::format("Too many instructions, given {}, max allowed {}", n_bytes / 2, MEM_SIZE_BYTES / 2));
	}

	fin.seekg(0);
	fin.read(reinterpret_cast<char *>(mem.data()), n_bytes);

	fin.close();
}

}  // namespace mycpu
