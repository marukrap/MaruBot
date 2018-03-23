#include "Floater.hpp"
#include "Faller.hpp"
#include "Tumbler.hpp"
#include "../Lixxie.hpp"

#include <cassert>

AfterAssignment Floater::onManualAssignment(Job& old)
{
	// LIXBOT: turn off ability to Float
	if (m_lixxie.abilityToFloat)
	{
		m_lixxie.abilityToFloat = false;

		if (old.activity == Activity::Floater)
		{
			m_lixxie.become(Activity::Faller);
			return AfterAssignment::WeAlreadyBecame;
		}

		return AfterAssignment::DoNotBecome;
	}
	//

	assert(!m_lixxie.abilityToFloat);
	m_lixxie.abilityToFloat = true;
	return AfterAssignment::DoNotBecome;
}

void Floater::onBecome(const Job& old)
{
	if (old.activity == Activity::Faller)
	{
		auto faller = dynamic_cast<const Faller*>(&old);
		assert(faller);
		m_speedY = faller->ySpeed;
	}

	else if (old.activity == Activity::Jumper || old.activity == Activity::Tumbler)
	{
		auto flyer = dynamic_cast<const BallisticFlyer*>(&old);
		assert(flyer);
		m_speedX = flyer->speedX;
		m_speedY = flyer->speedY;
		m_accelerateY = true;
	}
}

void Floater::perform()
{
	adjustFrame();
	adjustSpeed();
	move();
}

void Floater::adjustFrame()
{
	if (m_lixxie.isLastFrame())
		frame = 9;
	else
		m_lixxie.advanceFrame();
}

void Floater::adjustSpeed()
{
	assert(m_speedX >= 0);
	assert(m_speedX % 2 == 0);

	if (frame == 7)
		m_speedY = 4;

	else if (frame == 6)
		m_speedY = 2;

	else if (frame == 4)
	{
		m_speedX = 0;
		m_speedY = 0;
	}

	else if (frame < 4)
	{
		if (m_accelerateY)
		{
			if (m_speedY <= 12)
				m_speedY += 2;
			else if (m_speedY < 32)
				m_speedY += 1;
		}

		else
			m_speedY = (frame == 1 ? 6 : frame == 3 ? 2 : m_speedY);
	}
}

void Floater::move()
{
	int flownAhead = 0;
	int flownDown = 0;
	assert(m_speedX >= 0);

	while (flownAhead < m_speedX || flownDown < m_speedY)
	{
		if (m_lixxie.isSolid(0, 2))
		{
			m_lixxie.become(Activity::Lander);
			return;
		}

		else if (m_lixxie.isSolid(2, 0))
		{
			m_speedX = 0;
			flownAhead = 0;

			if (flownAhead >= m_speedY)
				break;
		}

		const bool ahead = flownAhead >= m_speedX ? false
						 : flownDown >= m_speedY ? true
						 : flownAhead == 0 && flownDown == 0 ? m_speedX >= m_speedY
						 : m_speedX * flownDown >= flownAhead * m_speedY;

		if (ahead)
		{
			m_lixxie.moveAhead();
			flownAhead += 2;
		}

		else
		{
			m_lixxie.moveDown(1);
			flownDown += 1;
		}
	}
}
