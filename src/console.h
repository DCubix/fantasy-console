#ifndef CONSOLE_H
#define CONSOLE_H

#if defined(__MINGW32__) || defined(__MINGW64__)
#include "SDL2/SDL.h"
#else
#include "SDL2.h"
#endif

#include "ram.h"
#include "video.h"

#include <stack>
#include <vector>

/**
 * Memory Layout
 * +--------------------+ <- 12KB (0x0000 - 0x2FFF)
 * |                    |
 * |                    |
 * |    PROGRAM         |
 * |                    |
 * |                    |
 * +--------------------+ <- 9KB (0x3000 - 0x53FF)
 * |                    |
 * |                    |
 * |    VIDEO RAM       |
 * |                    |
 * |                    |
 * |                    |
 * +--------------------+ <- 2.5KB (0x5400 - 0x5DFF)
 * |                    |
 * |    DATA STORAGE    |
 * |                    |
 * +--------------------+ <- 512 bytes (0x5E00 - 0x5FFF)
 * |                    |
 * |    CONSOLE OPTS    |
 * |                    |
 * +--------------------+
*/

constexpr int ConsoleScreenWidth = 96;
constexpr int ConsoleScreenHeight = 96;
constexpr int PixelSize = 2;

constexpr uint16_t ProgramSize = 12288;
constexpr uint16_t VideoSize = 9216;
constexpr uint16_t DataSize = 2560;
constexpr uint16_t OptsSize = 512;

#define LEN(x) (sizeof(x) / sizeof(x[0]))

enum OpCode {
	/* BASIC */
	OpHalt = 0,			// Stops execution
	OpPush,				// Pushes a LITERAL to the stack
	OpPushM,			// Pushes a MEMORY addr to the stack
	OpPop,				// Pops a value to MEMORY
	OpWait,				// Waits N ticks (pops N from the stack)

	/* MATH */
	OpAdd,				// Pops 2 values from the stack, adds them and pushes the result to it
	OpAddM,				// Pops 2 values from the stack, adds them and set it to a MEMORY ADDR
	OpSub,				// Pops 2 values from the stack, subtracts them and pushes the result to it
	OpSubM,				// Pops 2 values from the stack, subtracts them and set it to a MEMORY ADDR
	OpMul,				// Pops 2 values from the stack, multiplies them and pushes the result to it
	OpMulM,				// Pops 2 values from the stack, multiplies them and set it to a MEMORY ADDR
	OpDiv,				// Pops 2 values from the stack, divides them and pushes the result to it
	OpDivM,				// Pops 2 values from the stack, divides them and set it to a MEMORY ADDR
	OpLsh,				// Pops a value from the stack, shifts it to the left and pushes the result to it
	OpLshM,				// Pops a value from the stack, shifts it to the left and set it to a MEMORY ADDR
	OpRsh,				// Pops a value from the stack, shifts it to the right and pushes the result to it
	OpRshM,				// Pops a value from the stack, shifts it to the right and set it to a MEMORY ADDR
	OpAnd,				// Pops 2 values from the stack, bitwise ANDs them and pushes the result to it
	OpAndM,				// Pops 2 values from the stack, bitwise ANDs them and set it to a MEMORY ADDR
	OpOr,				// Pops 2 values from the stack, bitwise ORs them and pushes the result to it
	OpOrM,				// Pops 2 values from the stack, bitwise ORs them and set it to a MEMORY ADDR
	OpXor,				// Pops 2 values from the stack, bitwise XORs them and pushes the result to it
	OpXorM,				// Pops 2 values from the stack, bitwise XORs them and set it to a MEMORY ADDR
	OpNot,				// Pops a value from the stack, bitwise NOTs them and pushes the result to it
	OpNotM,				// Pops a value from the stack, bitwise NOTs them and set it to a MEMORY ADDR

	/*  COMPARISON */
	OpCmp,				// Compares a value in MEMORY to a LITERAL
	OpCmpM,				// Compares a value in MEMORY to another value in MEMORY

	/* FLOW CONTROL */
	OpJmp,				// Jumps to a point in the program
	OpJeq,				// Jumps if equal
	OpJne,				// Jumps if not equal
	OpJgt,				// Jumps if greater
	OpJlt,				// Jumps if less
	OpJge,				// Jumps if greater or equal
	OpJle,				// Jumps if less or equal

	/* SUBROUTINES */
	OpCall,				// Call a subroutine
	OpRet,				// Return from a subroutine

	/* DRAWING */
	OpPutP,				// Draws a single pixel (pops 2 values from the stack. X and Y)
	OpPutPM,			// Draws a single pixel (pops 2 values from the stack. X and Y, and the color value from a MEMORY ADDR)
	OpPutS,				// Draws an 8x8 sprite (pops 2 values from the stack. X and Y)

	OpSys,				// System call

	OpNoop
};

enum SystemCall {
	SysNone = 0,
	SysClearScreen = 0xF0,	// Pops a color from the stack and clears the screen, if the stack is empty, 0 is used.
	SysFlip,				// Flips the backbuffer to the screen
};

class Console {
public:
	Console() = default;
	~Console() = default;

	void init();

	Byte* prog() { return &m_ram[0x0000u]; }
	Byte* vram() { return &m_ram[0x3000u]; }
	Byte* data() { return &m_ram[0x5400u]; }
	Byte* opts() { return &m_ram[0x5E00u]; }

	void tick();

private:
	void flip();

	Byte next();

	struct Value {
		Byte val{ 0 };
		enum Type { Unknown = 0, Literal, MemoryAddr } type{ Unknown };

		Value() = default;
		Value(Byte v, int type) : val(v), type(Type(type)) {}
	};

	enum CmpResult {
		CmpEquals = 0,
		CmpGreater,
		CmpLess
	};

	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
	SDL_Texture *m_buffer;

	// Console components
	RAM<24> m_ram; // 24KB
	Video m_video;

	Byte m_pc, m_waitTimer = 0;
	CmpResult m_cmpResult;

	std::stack<Value> m_stack;
	std::stack<Byte> m_callStack;

	bool m_halted;
};

#endif // CONSOLE_H
