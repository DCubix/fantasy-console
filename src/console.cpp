#include "console.h"

#include <thread>
#include <mutex>
#include <fstream>
#include <iostream>

constexpr uint8_t PALETTE[][3] = {
	{  21,  25,  26 },
	{ 138,  76,  88 },
	{ 217,  98, 117 },
	{ 230, 184, 193 },
	{  69, 107, 115 },
	{  75, 151, 166 },
	{ 165, 189, 194 },
	{ 255, 245, 247 }
};

Byte Console::next() {
	return prog()[m_pc++];
}

void Console::tick() {
#define unpack(v) (v.type == Value::Literal ? v.val : data()[v.val])
#define mop(name, op) \
case name: { \
	Byte a = unpack(m_stack.top()); m_stack.pop(); \
	Byte b = unpack(m_stack.top()); m_stack.pop(); \
	m_stack.push(Value(a op b, Value::Literal)); \
} break;

	if (m_waitTimer > 0) {
		m_waitTimer--;
	} else {
		OpCode op = OpCode(next());
		switch (op) {
			case OpHalt: m_halted = true; break;
			case OpPush: m_stack.push(Value(next(), Value::Literal)); break;
			case OpPushM: m_stack.push(Value(next(), Value::MemoryAddr)); break;
			case OpPop: {
				Byte value = unpack(m_stack.top());
				data()[next()] = value;
				m_stack.pop();
			} break;
			case OpWait: {
				Byte N = unpack(m_stack.top()); m_stack.pop();
				m_waitTimer = N * 512;
			} break;

			mop(OpAdd, +)
			mop(OpSub, -)
			mop(OpMul, *)
			mop(OpDiv, /)
			mop(OpAnd, &)
			mop(OpOr, |)
			mop(OpXor, ^)

			case OpInc: data()[next()]++; break;
			case OpDec: data()[next()]--; break;

			case OpRsh: {
				Byte a = unpack(m_stack.top()); m_stack.pop();
				Byte n = unpack(m_stack.top()); m_stack.pop();
				m_stack.push(Value(a >> n, Value::Literal));
			} break;

			case OpLsh: {
				Byte a = unpack(m_stack.top()); m_stack.pop();
				Byte n = unpack(m_stack.top()); m_stack.pop();
				m_stack.push(Value(a << n, Value::Literal));
			} break;
			
			case OpNot: {
				Byte a = unpack(m_stack.top()); m_stack.pop();
				m_stack.push(Value(~a, Value::Literal));
			} break;

			case OpCmp: {
				Byte mem = data()[next()];
				Byte lit = next();
				if (mem == lit) m_cmpResult = CmpEquals;
				else if (mem > lit) m_cmpResult = CmpGreater;
				else if (mem < lit) m_cmpResult = CmpLess;
			} break;
			case OpCmpM: {
				Byte a = data()[next()];
				Byte b = data()[next()];
				if (a == b) m_cmpResult = CmpEquals;
				else if (a > b) m_cmpResult = CmpGreater;
				else if (a < b) m_cmpResult = CmpLess;
			} break;
			case OpJmp: m_pc = next(); break;
			case OpJeq: { Byte pos = next(); if (m_cmpResult == CmpEquals) m_pc = pos; } break;
			case OpJne: { Byte pos = next(); if (m_cmpResult != CmpEquals) m_pc = pos; } break;
			case OpJgt: { Byte pos = next(); if (m_cmpResult == CmpGreater) m_pc = pos; } break;
			case OpJlt: { Byte pos = next(); if (m_cmpResult == CmpLess) m_pc = pos; } break;
			case OpJge: { Byte pos = next(); if (m_cmpResult == CmpGreater || m_cmpResult == CmpEquals) m_pc = pos; } break;
			case OpJle: { Byte pos = next(); if (m_cmpResult == CmpLess || m_cmpResult == CmpEquals) m_pc = pos; } break;
			case OpCall: m_callStack.push(m_pc); m_pc = next(); break;
			case OpRet: m_pc = m_callStack.top(); m_callStack.pop(); next(); break;
			case OpPutP: {
				Byte y = unpack(m_stack.top()); m_stack.pop();
				Byte x = unpack(m_stack.top()); m_stack.pop();
				m_video.put(x, y, next());
			} break;
			case OpPutPM: {
				Byte y = unpack(m_stack.top()); m_stack.pop();
				Byte x = unpack(m_stack.top()); m_stack.pop();
				m_video.put(x, y, data()[next()]);
			} break;
			case OpPutS: {
				Byte y = unpack(m_stack.top()); m_stack.pop();
				Byte x = unpack(m_stack.top()); m_stack.pop();
				Byte frame = 0;
				if (!m_stack.empty()) {
					frame = unpack(m_stack.top()); m_stack.pop();
				}
				m_video.sprite(x, y, &data()[next() + 64 * frame]);
			} break;
			case OpNoop: break;
			case OpSys: {
				Byte sc = SystemCall(next());
				switch (sc) {
					case SysClearScreen: {
						Byte color = 0;
						if (!m_stack.empty()) {
							color = unpack(m_stack.top()); m_stack.pop();
						}
						m_video.clear(color);
					} break;
				}
			} break;
			default: break;
		}
	}
}

void Console::flip() {
	m_lock.lock();
	Uint8 *pixels;
	int pitch;
	int w, h;
	Uint32 format;
	SDL_QueryTexture(m_buffer, &format, nullptr, &w, &h);

	SDL_LockTexture(m_buffer, nullptr, (void**) &pixels, &pitch);
	SDL_PixelFormat fmt;
	fmt.format = format;

	for (uint32_t y = 0; y < ConsoleScreenHeight; y++) {
		for (uint32_t x = 0; x < ConsoleScreenWidth; x++) {
			uint32_t i = x + y * ConsoleScreenWidth;
			uint32_t j = i * 3;

			uint8_t col = vram()[i];
			pixels[j + 0] = PALETTE[col][0];
			pixels[j + 1] = PALETTE[col][1];
			pixels[j + 2] = PALETTE[col][2];
		}
	}
	SDL_UnlockTexture(m_buffer);

	SDL_RenderClear(m_renderer);
	SDL_Rect dst = { 0, 0, ConsoleScreenWidth * PixelSize, ConsoleScreenHeight * PixelSize };
	SDL_RenderCopy(m_renderer, m_buffer, nullptr, &dst);
	SDL_RenderPresent(m_renderer);

	m_video.markAsNotDirty();

	m_lock.unlock();
}

void Console::init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		return;
	}

	m_window = SDL_CreateWindow(
		"Console",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		ConsoleScreenWidth * PixelSize, ConsoleScreenHeight * PixelSize,
		SDL_WINDOW_SHOWN
	);

	if (m_window == nullptr) {
		SDL_Quit();
		return;
	}

	m_renderer = SDL_CreateRenderer(
		m_window,
		-1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (m_renderer == nullptr) {
		SDL_Quit();
		return;
	}

	m_buffer = SDL_CreateTexture(
		m_renderer,
		SDL_PIXELFORMAT_RGB24,
		SDL_TEXTUREACCESS_STREAMING,
		ConsoleScreenWidth, ConsoleScreenHeight
	);

	m_video = Video(vram(), VideoSize, ConsoleScreenWidth, ConsoleScreenHeight);

	m_halted = false;

	std::thread cpu([](Console* console){
		while (!console->m_halted) {
			if (!console->m_video.dirty()) {
				console->tick();
				// wait
				for (int i = 0; i < 512; i++);
			}
		}
	}, this);

	SDL_Event evt{};
	while (!m_halted) {
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
				case SDL_QUIT: m_halted = true; break;
				case SDL_KEYDOWN: {
					if (evt.key.keysym.sym == SDLK_F10) {
						m_lock.lock();
						std::ofstream fs("memory.dat", std::ios::binary | std::ios::ate);
						if (fs.good()) {
							fs.write(reinterpret_cast<char*>(m_ram.data().data()), sizeof(Byte) * m_ram.data().size());
							fs.close();
							std::cout << "Saved memory dump" << std::endl;
						}
						m_lock.unlock();
					}
				} break;
				default: break;
			}
		}

		if (m_video.dirty()) {
			flip();
		}
	}

	cpu.join();

	SDL_DestroyTexture(m_buffer);
	SDL_DestroyRenderer(m_renderer);
	SDL_DestroyWindow(m_window);
	SDL_Quit();
}
