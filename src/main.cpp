#include <iostream>
#include <cstdlib>
#include <cstring>

#include "console.h"
#include "asm.h"

int main(int argc, char** argv) {
	Console con{};

	ASM comp(R"(
			 let x, 11
			 let dirx, 0
			 let y, 37
			 let diry, 1
			 let spr, [
				0, 0, 7, 7, 7, 7, 0, 0,
				0, 7, 7, 7, 7, 7, 7, 0,
				7, 7, 7, 7, 7, 7, 5, 7,
				7, 7, 7, 7, 7, 7, 5, 7,
				7, 7, 7, 7, 7, 5, 5, 7,
				7, 7, 7, 7, 5, 5, 5, 7,
				0, 7, 5, 5, 5, 5, 7, 0,
				0, 0, 7, 7, 7, 7, 0, 0
			 ]

		_start:
			 call _incx

			 cmp &x, 88
			 jge _swapx

			 call _incy
			 cmp &y, 88
			 jge _swapy

			 sys 0xF0
			 pushm &x
			 pushm &y
			 puts &spr

			 jmp _start

		_decx:
			 cmp &dirx, 0
			 jne _incx
			 dec &x
			 ret

		_incx:
			 cmp &dirx, 1
			 jne _decx
			 inc &x
			 ret

		_decy:
			 cmp &diry, 0
			 jne _incy
			 dec &y
			 ret

		_incy:
			 cmp &diry, 1
			 jne _decy
			 inc &y
			 ret

		_swapx:
			 pushm &dirx
			 push 1
			 xor
			 pop &dirx
			 jmp _start

		_swapy:
			 pushm &diry
			 push 1
			 xor
			 pop &diry
			 jmp _start

	)", &con);
	ByteList code = comp.compile();

	std::memcpy(con.prog(), code.data(), sizeof(Byte) * code.size());

	con.init();
	return 0;
}
