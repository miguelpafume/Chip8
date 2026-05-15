#pragma once 

#include <cstdint>
#include <array>
#include <chrono>
#include <random>
#include <cstring>

#include "Util.hpp"

class Chip8 {
public:
	static inline constexpr unsigned int VIDEO_WIDTH = 64;
	static inline constexpr unsigned int VIDEO_HEIGHT = 32;
	
	const unsigned int START_ADDRESS = 0x200;

	const unsigned int FONTSET_START_ADDRESS = 0x50;
	const static unsigned int FONTSET_SIZE = 80;
	
	const float CYCLE_RATE = 960.0f;
	const float CYCLE_PERIOD = 1.0f / CYCLE_RATE;
	const float TIMER_RATE = 60.0f;
	const float TIMER_PERIOD = 1.0f / TIMER_RATE;

    Chip8();
    void Cycle();
	void TickTimers();
    void LoadROM(const std::string& filename);
    
    std::array<uint8_t, 16> m_keypad {};
    std::array<uint32_t, VIDEO_WIDTH * VIDEO_HEIGHT> m_video {};

	void getPixelsRGBA(uint8_t* pixels) const;

private:
    std::array<uint8_t, 4096> m_memory {};
    std::array<uint8_t, 16> m_registers {};
    std::array<uint16_t, 16> m_stack {};
    uint16_t m_index {};
    uint16_t m_pc {};
    uint8_t m_sp {};
    uint8_t m_delayTimer {};
    uint8_t m_soundTimer {};
    uint16_t m_opcode;

    std::default_random_engine m_randGen;
    std::uniform_int_distribution<uint8_t> m_randByte;

    using Chip8Func = void(Chip8::*)();
    Chip8Func table[0xF + 1];
    Chip8Func table0[0xE + 1];
    Chip8Func table8[0xE + 1];
    Chip8Func tableE[0xE + 1];
    Chip8Func tableF[0x65 + 1];

    void Table0();
    void Table8();
    void TableE();
    void TableF();

    void OP_NULL();

	void OP_00E0();
	void OP_00EE();

	void OP_1nnn();

	void OP_2nnn();

	void OP_3xkk();

	void OP_4xkk();

	void OP_5xy0();

	void OP_6xkk();

	void OP_7xkk();

	void OP_8xy0();
	void OP_8xy1();
	void OP_8xy2();
	void OP_8xy3();
	void OP_8xy4();
	void OP_8xy5();
	void OP_8xy6();
	void OP_8xy7();
	void OP_8xyE();

	void OP_9xy0();

	void OP_Annn();

	void OP_Bnnn();

	void OP_Cxkk();

	void OP_Dxyn();

	void OP_Ex9E();
	void OP_ExA1();

	void OP_Fx07();
	void OP_Fx0A();
	void OP_Fx15();
	void OP_Fx18();
	void OP_Fx1E();
	void OP_Fx29();
	void OP_Fx33();
	void OP_Fx55();
	void OP_Fx65();

	uint8_t fontset[FONTSET_SIZE] = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
};