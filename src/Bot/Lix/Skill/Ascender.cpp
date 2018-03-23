#include "Ascender.hpp"
#include "../Lixxie.hpp"

#include <cassert>

bool Ascender::blockable() const
{
	return false;
}

void Ascender::onBecome(const Job& old)
{
	auto solidPixelWithAirAbove = [this] (int y)
	{
		return m_lixxie.isSolid(0, y) && !m_lixxie.isSolid(0, y - 1);
	};

	constexpr int checkBelowHeight = 26;
	int swh = 0;

	while (swh < checkBelowHeight && !solidPixelWithAirAbove(2 - swh))
		++swh;

	if (swh == checkBelowHeight)
	{
		m_lixxie.become(Activity::Faller);
		return;
	}

	frame = std::clamp(6 - (swh / 2), 0, 5);
	const int swhLeftToAscendDuringPerform = 10 - frame * 2;
	assert(swh >= swhLeftToAscendDuringPerform);
	m_lixxie.moveUp(swh - swhLeftToAscendDuringPerform);
}

void Ascender::perform()
{
	if (frame != 5)
		m_lixxie.moveUp(2);

	if (m_lixxie.isLastFrame())
		m_lixxie.become(Activity::Walker);
	else
		m_lixxie.advanceFrame();
}
