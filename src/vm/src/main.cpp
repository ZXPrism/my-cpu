#include <iostream>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#pragma clang diagnostic pop

#include <vm.h>

int main(int argc, char **argv) {
	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int result = context.run();
	if (context.shouldExit()) {
		return result;
	}

	mycpu::VirtualMachine vm;
	vm.run();

	std::cout << "Exiting.." << std::endl;

	return result;
}
