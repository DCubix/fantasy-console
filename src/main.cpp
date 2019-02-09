#include <iostream>
#include <cstdlib>
#include <cstring>

#include "console.h"
#include "asm.h"

int main(int argc, char** argv) {
	Console con{};

	ASM comp(R"(
			 let x
			 let y
			 let col, 7

			 sys 0xF0	;; Clear screen
		_start:
			 pushm @x
			 pushm @y
			 putpm @col

			 pushm @x
			 push 1
			 addm @x

			 cmp @x, 96
			 jge next_row

			 pushm @col
			 push 1
			 addm @col

			 cmp @col, 7
			 jgt reset_col

			 jmp _start

		reset_col:
			 push 0
			 pop @col
			 jmp _start

		next_row:
			 push 0
			 pop @x
			 pushm @y
			 push 1
			 addm @y

			 cmp @y, 96
			 jge _end

			 jmp _start

		_end:
			 push 1000
			 wait
			 halt

	)", &con);
	ByteList code = comp.compile();

	std::memcpy(con.prog(), code.data(), sizeof(Byte) * code.size());

	con.init();
	return 0;
}
