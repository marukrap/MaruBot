#include "Climber.hpp"
#include "../Lixxie.hpp"

#include <cassert>

bool Climber::blockable() const
{
	return false;
}

AfterAssignment Climber::onManualAssignment(Job& old)
{
	// LIXBOT: turn off ability to climb
	if (m_lixxie.abilityToClimb)
	{
		m_lixxie.abilityToClimb = false;

		if (old.activity == Activity::Climber)
		{
			stopAndBecomeWalker();
			return AfterAssignment::WeAlreadyBecame;
		}

		return AfterAssignment::DoNotBecome;
	}
	//

	assert(!m_lixxie.abilityToClimb);
	m_lixxie.abilityToClimb = true;
	return AfterAssignment::DoNotBecome;
}

void Climber::onBecome(const Job& old)
{
	if (old.activity == Activity::Jumper)
		m_lixxie.playSound("climber");
	else
		frame = 3;

	stickSpriteToWall();
	maybeBecomeAscenderImmediatelyOnBecome();
}

void Climber::perform()
{
	if (m_lixxie.isLastFrame())
		frame = 4;
	else
		m_lixxie.advanceFrame();

	const int upwardsMovementThisFrame = [this] ()
	{
		switch ((frame - 4) % 8)
		{
		case 5:  return 2;
		case 6:  return 4;
		case 7:  return 2;
		default: return 0;
		}
	}();

	for (int i = 0; i < upwardsMovementThisFrame; ++i)
	{
		if (m_lixxie.isSolid(0, CeilingY))
		{
			m_lixxie.turn();
			m_lixxie.become(Activity::Faller);
			return;
		}

		else if (ascendHoistableLedge())
			return;

		else
			m_lixxie.moveUp(1);
	}

	assert(this == &m_lixxie.job);

	if (ascendHoistableLedge())
		return;
	else
		stickSpriteToWall();
}

void Climber::stickSpriteToWall()
{
	spriteOffsetX = m_lixxie.direction * (m_lixxie.facingRight && !m_lixxie.isSolidSingle(2, -6)
									   || m_lixxie.facingLeft  && !m_lixxie.isSolidSingle(1, -6));
}

void Climber::maybeBecomeAscenderImmediatelyOnBecome()
{
	for (int i = 8; i < 18; ++i)
	{
		if (m_lixxie.isSolid(0, -i))
		{
			stopAndBecomeWalker();
			break;
		}

		else if (i > 9 && !m_lixxie.isSolid(2, -i))
		{
			m_lixxie.moveAhead();
			m_lixxie.become(Activity::Ascender);
			break;
		}
	}
}

bool Climber::ascendHoistableLedge()
{
	if (!m_lixxie.isSolid(2, CeilingY))
	{
		m_lixxie.moveAhead();
		m_lixxie.become(Activity::Ascender);
		return true;
	}

	else
		return false;
}

void Climber::stopAndBecomeWalker()
{
	m_lixxie.turn();

	if (m_lixxie.isSolid())
	{
		const int oldWalkerFrame =
			(m_lixxie.activity == Activity::Walker || m_lixxie.activity == Activity::Runner)
			? m_lixxie.frame : -999;

		m_lixxie.become(Activity::Walker);

		if (oldWalkerFrame >= 0)
			m_lixxie.frame = oldWalkerFrame;
	}

	else
		m_lixxie.become(Activity::Faller);
}
