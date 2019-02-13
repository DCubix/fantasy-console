#ifndef VIDEO_H
#define VIDEO_H

#include "ram.h"

#include <cstdint>
#include <cassert>
#include <queue>

constexpr uint32_t SpriteSize = 8;

class Video {
public:
	Video(Byte* vram, uint16_t vramSize, int videoWidth, int videoHeight);

	Video() = default;
	~Video() = default;

	void clear(uint8_t color = 0);

	void put(int x, int y, uint8_t color);
	void sprite(int x, int y, Byte* data);

	void viewport(int x, int y, int w, int h);
	void viewportReset();

private:
	Byte* m_vram;
	uint16_t m_vramSize;

	int m_videoWidth, m_videoHeight;
	int m_viewport[4];
};

#endif // VIDEO_H
