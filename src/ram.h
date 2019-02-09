#ifndef RAM_H
#define RAM_H

#include <array>
#include <vector>
#include <cstdint>
#include <cassert>

using Byte = uint32_t;

template <uint16_t SizeKB>
class RAM {
public:
	RAM()
	{ m_data.fill(0u); }

	~RAM() = default;

	Byte* map(uint16_t addr) { return &m_data[addr]; }

	Byte& operator [](uint16_t addr) { return m_data[addr]; }
	const Byte& operator [](uint16_t addr) const { return m_data[addr]; }

private:
	std::array<Byte, SizeKB * 1024> m_data;
};

#endif // RAM_H