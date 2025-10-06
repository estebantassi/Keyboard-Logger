#ifndef TYPES_HPP
#define TYPES_HPP

#include <chrono>

struct MouseInput {
	USHORT flag;
	bool state;
	char* name;
};

struct Input {
	RAWINPUT raw;
	double time;
};

#endif