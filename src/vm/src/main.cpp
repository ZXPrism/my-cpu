#include <iostream>
#include <optional>

#include <CLI/CLI.hpp>

#include <vm.h>

int main(int argc, char **argv) {
	CLI::App app{ "vm" };
	std::optional<std::string> bytecode_file_path;
	app.add_option("-i,--input", bytecode_file_path, "Bytecode file path");
	CLI11_PARSE(app, argc, argv);

	using namespace mycpu;
	mycpu::VirtualMachine vm;

	if (bytecode_file_path.has_value()) {
		vm.load_from_file(*bytecode_file_path);
	} else {
		// test program: fib sequence
		std::vector<u8> bytecode{ 0x11, 0x71, 0x1, 0xc2,
			                      0x1, 0xc3, 0x44, 0x74,
			                      0x1, 0xc5, 0x66, 0x76,
			                      0x10, 0xc7, 0xa, 0xc8,
			                      0x23, 0x4, 0x3, 0x2,
			                      0x4, 0x3, 0x4, 0xf,
			                      0x0, 0xf0, 0x15, 0x1,
			                      0x18, 0x66, 0x70, 0xe6,
			                      0x1, 0xc1, 0x0, 0xf1 };
		vm.load_from_dram(bytecode);
	}

	vm.run();

	std::cout << "Exiting.." << std::endl;

	return 0;
}
