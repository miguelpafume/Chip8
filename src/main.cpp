#include "Chip8App.hpp"

#include <iostream>

int main() {
	Chip8App app;

	try {
		app.run();
	} catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}