#include <cassert>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#include <doctest/doctest.h>
#pragma clang diagnostic pop

#include <vm.h>

namespace mycpu {

// helpers for test
void load16(VirtualMachine &vm, u8 reg_idx, u16 imm) {
	assert(reg_idx != 14 && "This register has been reserved as a temp register");
	assert(reg_idx != 15 && "This register has been reserved as a temp register");
	vm.step(Instruction(OpCode::LI, reg_idx, (imm >> 4) & 0xF, imm & 0xF));
	vm.step(Instruction(OpCode::LI, 15, imm >> 12, (imm >> 8) & 0xF));
	vm.step(Instruction(OpCode::LI, 14, 0, 8));
	vm.step(Instruction(OpCode::SLL, 15, 15, 14));
	vm.step(Instruction(OpCode::ADD, reg_idx, reg_idx, 15));
}

}  // namespace mycpu

TEST_SUITE("helpers") {
	TEST_CASE("load16") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 1, 0xDEAD);
		load16(vm, 3, 0xBEEF);
		CHECK_EQ(vm.get_reg(1), 0xDEAD);
		CHECK_EQ(vm.get_reg(3), 0xBEEF);
	}
}

TEST_CASE("x0 should be always 0") {
	using namespace mycpu;
	VirtualMachine vm;

	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 0, 0, 1));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::ADD, 0, 1, 1));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::SUB, 0, 1, 1));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::SLL, 0, 1, 1));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 4));
	vm.step(Instruction(OpCode::LI, 2, 0, 1));
	vm.step(Instruction(OpCode::SRL, 0, 1, 2));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 4));
	vm.step(Instruction(OpCode::LI, 2, 0, 1));
	vm.step(Instruction(OpCode::SRA, 0, 1, 2));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::LI, 2, 0, 2));
	vm.step(Instruction(OpCode::SLT, 0, 1, 2));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::LI, 2, 0, 2));
	vm.step(Instruction(OpCode::SLTU, 0, 1, 2));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::LI, 2, 0, 2));
	vm.step(Instruction(OpCode::XOR, 0, 1, 2));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::LI, 2, 0, 2));
	vm.step(Instruction(OpCode::OR, 0, 1, 2));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::AND, 0, 1, 1));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 7));
	vm.step(Instruction(OpCode::LI, 2, 0, 2));
	vm.step(Instruction(OpCode::STR, 1, 2, 0));
	vm.step(Instruction(OpCode::LDR, 0, 2, 0));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 8));
	vm.step(Instruction(OpCode::JAL, 0, 1, 0));
	CHECK_EQ(vm.get_reg(0), 0);

	vm.step(Instruction(OpCode::LI, 1, 0, 1));
	vm.step(Instruction(OpCode::LI, 2, 0, 8));
	vm.step(Instruction(OpCode::BAL, 1, 2, 0));
	CHECK_EQ(vm.get_reg(0), 0);
}

TEST_SUITE("Instructions") {
	TEST_CASE("ADD") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 4));
		vm.step(Instruction(OpCode::LI, 3, 0, 5));
		vm.step(Instruction(OpCode::ADD, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 9);

		vm.step(Instruction(OpCode::LI, 2, 0, 15));
		vm.step(Instruction(OpCode::LI, 3, 0, 14));
		vm.step(Instruction(OpCode::ADD, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 29);
	}

	TEST_CASE("SUB") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 9));
		vm.step(Instruction(OpCode::LI, 3, 0, 5));
		vm.step(Instruction(OpCode::SUB, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 4);

		vm.step(Instruction(OpCode::LI, 2, 0, 3));
		vm.step(Instruction(OpCode::LI, 3, 0, 7));
		vm.step(Instruction(OpCode::SUB, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0xFFFC);  // 3 - 7 = -4
	}

	TEST_CASE("SLL") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 9));
		vm.step(Instruction(OpCode::LI, 3, 0, 3));
		vm.step(Instruction(OpCode::SLL, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 72);
	}

	TEST_CASE("SRL") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 9));
		vm.step(Instruction(OpCode::LI, 3, 0, 2));
		vm.step(Instruction(OpCode::SRL, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 2);
	}

	TEST_CASE("SRA") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 9));
		vm.step(Instruction(OpCode::LI, 3, 0, 2));
		vm.step(Instruction(OpCode::SRA, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 2);

		// R[2] = -9
		vm.step(Instruction(OpCode::LI, 2, 0xF, 6));
		vm.step(Instruction(OpCode::LI, 4, 0xF, 0xF));
		vm.step(Instruction(OpCode::LI, 5, 0, 8));
		vm.step(Instruction(OpCode::SLL, 4, 4, 5));  // << 8
		vm.step(Instruction(OpCode::ADD, 2, 2, 4));  // fill high 8 bits with 0xFF
		// R[3] = 1
		vm.step(Instruction(OpCode::LI, 3, 0, 1));
		vm.step(Instruction(OpCode::SRA, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0xFFFB);  // -9 >> 1 = -5
	}

	TEST_CASE("SLT") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 9));
		vm.step(Instruction(OpCode::LI, 3, 0, 2));
		vm.step(Instruction(OpCode::SLT, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0);  // 9 < 2 --> F

		vm.step(Instruction(OpCode::LI, 2, 0, 3));
		vm.step(Instruction(OpCode::LI, 3, 0, 4));
		vm.step(Instruction(OpCode::SLT, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 1);  // 3 < 4 --> T

		vm.step(Instruction(OpCode::LI, 2, 0, 6));
		vm.step(Instruction(OpCode::LI, 3, 0, 6));
		vm.step(Instruction(OpCode::SLT, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0);  // 6 < 6 --> F

		// R[2] = -5
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 0, 5));
		vm.step(Instruction(OpCode::SUB, 2, 4, 5));
		vm.step(Instruction(OpCode::LI, 3, 0, 6));
		vm.step(Instruction(OpCode::SLT, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 1);  // -5 < 6 --> T

		// R[2] = -5
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 0, 5));
		vm.step(Instruction(OpCode::SUB, 2, 4, 5));
		vm.step(Instruction(OpCode::LI, 3, 0, 6));
		vm.step(Instruction(OpCode::SLT, 1, 3, 2));
		CHECK_EQ(vm.get_reg(1), 0);  // 6 < -5 --> F

		// R[2] = -5
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 0, 5));
		vm.step(Instruction(OpCode::SUB, 2, 4, 5));
		// R[3] = -17
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 1, 1));
		vm.step(Instruction(OpCode::SUB, 3, 4, 5));
		vm.step(Instruction(OpCode::SLT, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0);  // -5 < -17 --> F

		// R[2] = -26
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 1, 10));
		vm.step(Instruction(OpCode::SUB, 2, 4, 5));
		// R[3] = -19
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 1, 3));
		vm.step(Instruction(OpCode::SUB, 3, 4, 5));
		vm.step(Instruction(OpCode::SLT, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 1);  // -26 < -19 --> T
	}

	TEST_CASE("SLTU") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 2, 0, 9));
		vm.step(Instruction(OpCode::LI, 3, 0, 2));
		vm.step(Instruction(OpCode::SLTU, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0);  // 9 < 2 --> F

		vm.step(Instruction(OpCode::LI, 2, 0, 3));
		vm.step(Instruction(OpCode::LI, 3, 0, 4));
		vm.step(Instruction(OpCode::SLTU, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 1);  // 3 < 4 --> T

		vm.step(Instruction(OpCode::LI, 2, 0, 6));
		vm.step(Instruction(OpCode::LI, 3, 0, 6));
		vm.step(Instruction(OpCode::SLTU, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0);  // 6 < 6 --> F

		// R[2] = 65510 (-26)
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 1, 10));
		vm.step(Instruction(OpCode::SUB, 2, 4, 5));
		// R[3] = 65517 (-19)
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 1, 3));
		vm.step(Instruction(OpCode::SUB, 3, 4, 5));
		vm.step(Instruction(OpCode::SLTU, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 1);  // 65510 < 65517 --> T (-26 < -19 --> T)

		// R[2] = 65510 (-26)
		vm.step(Instruction(OpCode::LI, 4, 0, 0));
		vm.step(Instruction(OpCode::LI, 5, 1, 10));
		vm.step(Instruction(OpCode::SUB, 2, 4, 5));
		// R[3] = 9 (9)
		vm.step(Instruction(OpCode::LI, 3, 0, 9));
		vm.step(Instruction(OpCode::SLTU, 1, 2, 3));
		CHECK_EQ(vm.get_reg(1), 0);  // 65510 < 9 --> F (-26 < 9 --> T)
	}

	TEST_CASE("XOR") {
		using namespace mycpu;
		VirtualMachine vm;

		// id est CLR
		vm.step(Instruction(OpCode::LI, 14, 0, 13));
		vm.step(Instruction(OpCode::XOR, 14, 14, 14));
		CHECK_EQ(vm.get_reg(14), 0);

		vm.step(Instruction(OpCode::LI, 3, 0, 12));
		vm.step(Instruction(OpCode::LI, 4, 0x4, 0xD));
		vm.step(Instruction(OpCode::XOR, 7, 3, 4));
		CHECK_EQ(vm.get_reg(7), 12 ^ 0x4D);
	}

	TEST_CASE("OR") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 3, 0, 12));
		vm.step(Instruction(OpCode::LI, 4, 0x4, 0xD));
		vm.step(Instruction(OpCode::OR, 7, 3, 4));
		CHECK_EQ(vm.get_reg(7), 12 | 0x4D);
	}

	TEST_CASE("AND") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 3, 0, 12));
		vm.step(Instruction(OpCode::LI, 4, 0x4, 0xD));
		vm.step(Instruction(OpCode::AND, 7, 3, 4));
		CHECK_EQ(vm.get_reg(7), 12 & 0x4D);
	}

	TEST_CASE("STR") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 1, 0xABCD);  // src value
		load16(vm, 2, 0XFFFE);  // dest mem addr
		vm.step(Instruction(OpCode::STR, 1, 2, 0));
		CHECK_EQ(vm.get_mem(0xFFFE), 0xCD);
		CHECK_EQ(vm.get_mem(0xFFFF), 0xAB);

		load16(vm, 1, 0xDEAD);  // src value
		load16(vm, 2, 0XBEEE);  // dest mem addr
		vm.step(Instruction(OpCode::STR, 1, 2, 0));
		CHECK_EQ(vm.get_mem(0xBEEE), 0xAD);
		CHECK_EQ(vm.get_mem(0xBEEF), 0xDE);
	}

	TEST_CASE("STR / Odd Addr") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 1, 0xABCD);  // src value
		load16(vm, 2, 0XFFF9);  // dest mem addr
		CHECK_THROWS(vm.step(Instruction(OpCode::STR, 1, 2, 0)));
	}

	TEST_CASE("LDR") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 1, 0xDEAD);  // src value
		load16(vm, 2, 0XBEEE);  // dest mem addr
		vm.step(Instruction(OpCode::STR, 1, 2, 0));
		vm.step(Instruction(OpCode::LDR, 1, 2, 0));
		CHECK_EQ(vm.get_reg(1), 0xDEAD);
	}

	TEST_CASE("LDR / Odd Addr") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 2, 0XBEEF);  // dest mem addr
		CHECK_THROWS(vm.step(Instruction(OpCode::LDR, 1, 2, 0)));
	}

	TEST_CASE("LI") {
		using namespace mycpu;
		VirtualMachine vm;

		vm.step(Instruction(OpCode::LI, 1, 0, 11));
		CHECK_EQ(vm.get_reg(1), 11);

		vm.step(Instruction(OpCode::LI, 1, 0xA, 0xB));
		CHECK_EQ(vm.get_reg(1), 0xAB);
	}

	TEST_CASE("JAL") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 2, 0xDCBA);
		auto curr_pc = vm.get_pc();
		vm.step(Instruction(OpCode::JAL, 1, 2, 0));
		CHECK_EQ(vm.get_reg(1), curr_pc + 2);
		CHECK_EQ(vm.get_pc(), 0XDCBA);
	}

	TEST_CASE("JAL-2") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 1, 0xDCBA);
		auto curr_pc = vm.get_pc();
		vm.step(Instruction(OpCode::JAL, 1, 1, 0));
		CHECK_EQ(vm.get_reg(1), curr_pc + 2);
		CHECK_EQ(vm.get_pc(), 0XDCBA);
	}

	TEST_CASE("JAL / Odd Addr") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 1, 0xDCBB);
		CHECK_THROWS(vm.step(Instruction(OpCode::JAL, 1, 1, 0)));
	}

	TEST_CASE("BAL") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 2, 0xDCBA);
		load16(vm, 3, 1);
		auto curr_pc = vm.get_pc();
		vm.step(Instruction(OpCode::BAL, 3, 2, 1));
		CHECK_EQ(vm.get_reg(1), curr_pc + 2);
		CHECK_EQ(vm.get_pc(), 0XDCBA);

		load16(vm, 2, 0xBEEE);
		load16(vm, 3, 1);
		curr_pc = vm.get_pc();
		vm.step(Instruction(OpCode::BAL, 3, 2, 1));
		CHECK_EQ(vm.get_reg(1), curr_pc + 2);
		CHECK_EQ(vm.get_pc(), 0XBEEE);
	}

	TEST_CASE("BAL-2") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 2, 0xDCBA);
		load16(vm, 3, 1);
		auto curr_pc = vm.get_pc();
		vm.step(Instruction(OpCode::BAL, 3, 2, 2));
		CHECK_EQ(vm.get_reg(2), curr_pc + 2);
		CHECK_EQ(vm.get_pc(), 0XDCBA);
	}

	TEST_CASE("BAL / Odd Addr") {
		using namespace mycpu;
		VirtualMachine vm;

		load16(vm, 2, 0x1123);
		load16(vm, 3, 1);
		CHECK_THROWS(vm.step(Instruction(OpCode::BAL, 3, 2, 4)));
	}

	// INT can't be tested for now
	// Because I have not decided the spec for that
}
