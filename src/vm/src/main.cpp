#include <iostream>

#include <CLI/CLI.hpp>

#include <vm.h>

int main(int argc, char **argv) {
	CLI::App app{ "vm" };

	std::string bytecode_file_path;
	app.add_option("-i,--input", bytecode_file_path, "Bytecode file path")
	    ->required();

	bool dump_memory;
	app.add_flag("-d,--dump-memory", dump_memory, "Dump memory to memory.bin after the run");

	CLI11_PARSE(app, argc, argv);

	using namespace mycpu;
	mycpu::VirtualMachine vm;

	vm.load_from_file(bytecode_file_path);
	vm.run();

	if (dump_memory) {
		vm.dump_memory("memory.bin");
	}

	std::cout << "Exiting.." << std::endl;

	return 0;
}
