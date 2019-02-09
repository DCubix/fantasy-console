#include "video.h"

#include <cstring>

Video::Video(Byte* vram, uint16_t vramSize, int videoWidth, int videoHeight)
	: m_vram(vram), m_vramSize(vramSize), m_videoWidth(videoWidth), m_videoHeight(videoHeight)
{
	assert(vram != nullptr && "Invalid VRAM");
	m_viewport[0] = 0;
	m_viewport[1] = 0;
	m_viewport[2] = videoWidth;
	m_viewport[3] = videoHeight;
}

void Video::clear(uint8_t color) {
	std::memset(m_vram, color, m_vramSize * sizeof(Byte));
}

void Video::viewport(int x, int y, int w, int h) {
	m_viewport[0] = x;
	m_viewport[1] = y;
	m_viewport[2] = w;
	m_viewport[3] = h;
}

void Video::viewportReset() {
	m_viewport[0] = 0;
	m_viewport[1] = 0;
	m_viewport[2] = m_videoWidth;
	m_viewport[3] = m_videoHeight;
}

void Video::put(int x, int y, uint8_t color) {
	if (x < m_viewport[0] || x >= m_viewport[2] ||
		y < m_viewport[1] || y >= m_viewport[3])
		return;
	m_vram[x + y * m_videoWidth] = color;
}

void Video::sprite(int x, int y, Byte* data) {

}
