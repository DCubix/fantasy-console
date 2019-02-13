#ifndef RAM_H
#define RAM_H

#include <array>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>

using Byte = uint32_t;

struct DataBlock {
	uint16_t address, size;
	Byte* data;
};

template <uint16_t SizeKB>
class RAM {
public:
	RAM()
		: m_dataPtr(0u)
	{ m_data.fill(0u); }

	~RAM() = default;

	Byte* map(uint16_t addr) { return &m_data[addr]; }

	uint16_t alloc(uint16_t size) {
		if (size == 0) return -1;

		DataBlock block;
		auto pos = std::find_if(m_unused.begin(), m_unused.end(), [=](const DataBlock& d){
			return d.size >= size;
		});
		if (!m_unused.empty() && pos != m_unused.end()) {
			block = m_unused.back();
			m_unused.pop_back();
		} else {
			block = DataBlock();
			block.size = size;
			block.address = m_dataPtr;
			block.data = map(block.address);
			m_dataPtr += size;
			m_inuse.push_back(block);
		}
		return block.address;
	}

	void free(uint16_t addr) {
		auto pos = std::find_if(m_inuse.begin(), m_inuse.end(), [=](const DataBlock& d) {
			return d.address == addr;
		});
		if (pos != m_inuse.end()) {
			DataBlock block = m_inuse.back();
			block.data = nullptr;
			m_inuse.pop_back();
			m_unused.push_back(block);
		}
	}

	Byte& operator [](uint16_t addr) { return m_data[addr]; }
	const Byte& operator [](uint16_t addr) const { return m_data[addr]; }

	std::array<Byte, SizeKB * 1024> data() const { return m_data; }

private:
	std::array<Byte, SizeKB * 1024> m_data;
	std::vector<DataBlock> m_unused, m_inuse;
	uint16_t m_dataPtr;
};

#endif // RAM_H
