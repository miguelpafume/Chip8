#include "Chip8.hpp"

Chip8::Chip8() : m_randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    m_pc = START_ADDRESS;

    // Load fonts into memory
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		m_memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

    m_randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    for (size_t i = 0; i <= 0xE; i++) {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    for (size_t i = 0; i <= 0x65; i++) {
        tableF[i] = &Chip8::OP_NULL;
    }

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::Cycle() {
	if (m_pc + 1 >= 4096) return;

    m_opcode = (m_memory[m_pc] << 8u) | m_memory[m_pc + 1];

    m_pc += 2;

    ((*this).*(table[(m_opcode & 0xF000u) >> 12u]))();
}

void Chip8::TickTimers() {
    if (m_delayTimer > 0) {
        --m_delayTimer;
    }

    if (m_soundTimer > 0) {
        --m_soundTimer;
    }
}

void Chip8::LoadROM(const std::string& filename) {
    std::vector<char> file = MAGE::readFile(filename);

    for (long i = 0; i < file.size(); ++i) {
        m_memory[START_ADDRESS + i] = file[i];
    }
}

void Chip8::getPixelsRGBA(uint8_t *pixels) const {
	for (size_t i = 0; i < VIDEO_WIDTH * VIDEO_HEIGHT; i++) {
		uint8_t value = m_video[i] ? 255 : 0;
		pixels[4 * i + 0] = value; // R
		pixels[4 * i + 1] = value; // G
		pixels[4 * i + 2] = value; // B
		pixels[4 * i + 3] = 255;   // A
	}
}

void Chip8::OP_NULL() {}

void Chip8::Table0() {
    ((*this).*(table0[m_opcode & 0x000Fu]))();
}

void Chip8::Table8() {
    ((*this).*(table8[m_opcode & 0x000Fu]))();
}

void Chip8::TableE() {
    ((*this).*(tableE[m_opcode & 0x000Fu]))();
}

void Chip8::TableF() {
    ((*this).*(tableF[m_opcode & 0x00FFu]))();
}

// Clear the display
void Chip8::OP_00E0()
{
    m_video.fill(0);
}

// Return from a subroutine
void Chip8::OP_00EE()
{
	if(m_sp == 0) return;

	--m_sp;
	m_pc = m_stack[m_sp];
}

// Jump to address NNN
void Chip8::OP_1nnn()
{
	uint16_t address = m_opcode & 0x0FFFu;

	m_pc = address;
}

// Call subroutine at NNN
void Chip8::OP_2nnn()
{
	if(m_sp >= 16) return;

	uint16_t address = m_opcode & 0x0FFFu;

	m_stack[m_sp] = m_pc;
	++m_sp;
	m_pc = address;
}

// Skip next instruction if Vx == kk
void Chip8::OP_3xkk()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t byte = m_opcode & 0x00FFu;

	if (m_registers[Vx] == byte)
	{
		m_pc += 2;
	}
}

// Skip next instruction if Vx != kk
void Chip8::OP_4xkk()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t byte = m_opcode & 0x00FFu;

	if (m_registers[Vx] != byte)
	{
		m_pc += 2;
	}
}

// Skip next instruction if Vx == Vy
void Chip8::OP_5xy0()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	if (m_registers[Vx] == m_registers[Vy])
	{
		m_pc += 2;
	}
}

// Set Vx = kk
void Chip8::OP_6xkk()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t byte = m_opcode & 0x00FFu;

	m_registers[Vx] = byte;
}

// Set Vx = Vx + kk
void Chip8::OP_7xkk()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t byte = m_opcode & 0x00FFu;

	m_registers[Vx] += byte;
}

// Set Vx = Vy
void Chip8::OP_8xy0()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	m_registers[Vx] = m_registers[Vy];
}

// Set Vx = Vx OR Vy
void Chip8::OP_8xy1()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	m_registers[Vx] |= m_registers[Vy];
}

// Set Vx = Vx AND Vy
void Chip8::OP_8xy2()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	m_registers[Vx] &= m_registers[Vy];
}

// Set Vx = Vx XOR Vy
void Chip8::OP_8xy3()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	m_registers[Vx] ^= m_registers[Vy];
}

// Set Vx = Vx + Vy, set VF = carry
void Chip8::OP_8xy4()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	uint16_t sum = m_registers[Vx] + m_registers[Vy];

	if (sum > 255U)
	{
		m_registers[0xF] = 1;
	}
	else
	{
		m_registers[0xF] = 0;
	}

	m_registers[Vx] = sum & 0xFFu;
}

// Set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::OP_8xy5()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	if (m_registers[Vx] > m_registers[Vy])
	{
		m_registers[0xF] = 1;
	}
	else
	{
		m_registers[0xF] = 0;
	}

	m_registers[Vx] -= m_registers[Vy];
}

// Set Vx = Vx SHR 1
void Chip8::OP_8xy6()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	// Save LSB in VF
	m_registers[0xF] = (m_registers[Vx] & 0x1u);

	m_registers[Vx] >>= 1;
}

// Set Vx = Vy - Vx, set VF = NOT borrow
void Chip8::OP_8xy7()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	if (m_registers[Vy] > m_registers[Vx])
	{
		m_registers[0xF] = 1;
	}
	else
	{
		m_registers[0xF] = 0;
	}

	m_registers[Vx] = m_registers[Vy] - m_registers[Vx];
}

// Set Vx = Vx SHL 1
void Chip8::OP_8xyE()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	// Save MSB in VF
	m_registers[0xF] = (m_registers[Vx] & 0x80u) >> 7u;

	m_registers[Vx] <<= 1;
}

// Skip next instruction if Vx != Vy
void Chip8::OP_9xy0()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;

	if (m_registers[Vx] != m_registers[Vy])
	{
		m_pc += 2;
	}
}

// Set I = NNN
void Chip8::OP_Annn()
{
	uint16_t address = m_opcode & 0x0FFFu;

	m_index = address;
}

// Jump to address NNN + V0
void Chip8::OP_Bnnn()
{
	uint16_t address = m_opcode & 0x0FFFu;

	m_pc = m_registers[0] + address;
}

// Set Vx = random byte AND kk
void Chip8::OP_Cxkk()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t byte = m_opcode & 0x00FFu;

	m_registers[Vx] = m_randByte(m_randGen) & byte;
}

// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void Chip8::OP_Dxyn()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (m_opcode & 0x00F0u) >> 4u;
	uint8_t height = m_opcode & 0x000Fu;

	// Wrap if going beyond screen boundaries
	uint8_t xPos = m_registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = m_registers[Vy] % VIDEO_HEIGHT;

	m_registers[0xF] = 0;

	for (unsigned int col = 0; col < height; ++col) {
		if (yPos + col >= VIDEO_HEIGHT) break;

		uint8_t spriteByte = m_memory[m_index + col];

		for (unsigned int row = 0; row < 8; ++row) {
			if (xPos + row >= VIDEO_WIDTH) break;

			uint8_t spritePixel = spriteByte & (0x80u >> row);
			uint32_t* screenPixel = &m_video[(yPos + col) * VIDEO_WIDTH + (xPos + row)];

			// Sprite pixel is on
			if (spritePixel) {
				if (*screenPixel == 0xFFFFFFFF) {
					m_registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_Ex9E()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	uint8_t key = m_registers[Vx];

	if (key > 0xF) return;

	if (m_keypad[key]) {
		m_pc += 2;
	}
}

void Chip8::OP_ExA1()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	uint8_t key = m_registers[Vx];

	if (key > 0xF) return;

	if (!m_keypad[key]) {
		m_pc += 2;
	}
}

void Chip8::OP_Fx07()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	m_registers[Vx] = m_delayTimer;
}

void Chip8::OP_Fx0A()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
    
	if (m_keypad[0]) {
		m_registers[Vx] = 0;
	}
	else if (m_keypad[1]) {
		m_registers[Vx] = 1;
	}
	else if (m_keypad[2]) {
		m_registers[Vx] = 2;
	}
	else if (m_keypad[3]) {
		m_registers[Vx] = 3;
	}
	else if (m_keypad[4]) {
		m_registers[Vx] = 4;
	}
	else if (m_keypad[5]) {
		m_registers[Vx] = 5;
	}
	else if (m_keypad[6]) {
		m_registers[Vx] = 6;
	}
	else if (m_keypad[7]) {
		m_registers[Vx] = 7;
	}
	else if (m_keypad[8]) {
		m_registers[Vx] = 8;
	}
	else if (m_keypad[9]) {
		m_registers[Vx] = 9;
	}
	else if (m_keypad[10]) {
		m_registers[Vx] = 10;
	}
	else if (m_keypad[11]) {
		m_registers[Vx] = 11;
	}
	else if (m_keypad[12]) {
		m_registers[Vx] = 12;
	}
	else if (m_keypad[13]) {
		m_registers[Vx] = 13;
	}
	else if (m_keypad[14]) {
		m_registers[Vx] = 14;
	}
	else if (m_keypad[15]) {
		m_registers[Vx] = 15;
	}
	else {
		m_pc -= 2;
	}
}

void Chip8::OP_Fx15()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	m_delayTimer = m_registers[Vx];
}

void Chip8::OP_Fx18()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	m_soundTimer = m_registers[Vx];
}

void Chip8::OP_Fx1E()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	m_index += m_registers[Vx];
}

void Chip8::OP_Fx29()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t digit = m_registers[Vx];

	m_index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;
	uint8_t value = m_registers[Vx];

	// Ones-place
	m_memory[m_index + 2] = value % 10;
	value /= 10;

	// Tens-place
	m_memory[m_index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	m_memory[m_index] = value % 10;
}

void Chip8::OP_Fx55()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		m_memory[m_index + i] = m_registers[i];
	}
}

void Chip8::OP_Fx65()
{
	uint8_t Vx = (m_opcode & 0x0F00u) >> 8u;

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		m_registers[i] = m_memory[m_index + i];
	}
}