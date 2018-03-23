#include "Stunner.hpp"
#include "Tumbler.hpp"
#include "../Lixxie.hpp"

#include <cassert>

void Stunner::onBecome(const Job& old)
{
	m_lixxie.playSound("ouch");
}

void Stunner::perform()
{
	bool considerBecomingWalker = false;

	if (frame == 8)
	{
		++m_stayedInFrame8;

		if (m_stayedInFrame8 == 17)
			m_lixxie.advanceFrame();
	}

	else if (m_lixxie.isLastFrame())
		considerBecomingWalker = true;

	else
		m_lixxie.advanceFrame();

	auto hollowBelow = [this] (int y)
	{
		return !m_lixxie.isSolid(0, 2 + y);
	};

	constexpr int maxDown = 4;

	int moveDownBy = 0;

	for (int y = 0; y < maxDown; ++y)
	{
		if (hollowBelow(y))
			++moveDownBy;
		else
			break;
	}

	if (moveDownBy == maxDown && hollowBelow(moveDownBy))
	{
		m_lixxie.moveDown(1);
		m_lixxie.become(Activity::Tumbler);

		auto tumbling = dynamic_cast<Tumbler*>(&m_lixxie.job);
		assert(tumbling);
		tumbling->speedX = 0;
		tumbling->speedY = 2;
	}

	else if (moveDownBy > 0)
		m_lixxie.moveDown(moveDownBy);

	if (this == &m_lixxie.job && considerBecomingWalker)
		m_lixxie.become(Activity::Walker);
}
