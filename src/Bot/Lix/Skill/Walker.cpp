#include "Walker.hpp"
#include "Builder.hpp"
#include "Faller.hpp"
#include "../Lixxie.hpp"

#include <cassert>

AfterAssignment Walker::onManualAssignment(Job& old)
{
	// LIXBOT: if > switch
	switch (old.activity)
	{
	case Activity::Walker:
	case Activity::Runner:
	case Activity::Lander:
		m_lixxie.turn();
		old.frame = -1;
		return AfterAssignment::DoNotBecome;

	case Activity::Stunner:
	case Activity::Ascender:
		m_lixxie.become(Activity::Walker);
		m_lixxie.turn();
		m_lixxie.frame = -1;
		return AfterAssignment::WeAlreadyBecame;

	case Activity::Blocker:
		if (old.frame < 20)
			old.frame = 20; // LIXBOT: 21 > 20
		else
			m_lixxie.turn();
		return AfterAssignment::DoNotBecome;

	case Activity::Platformer:
		if (old.frame > 5)
		{
			Platformer::abortAndStandUp(m_lixxie);
			return AfterAssignment::WeAlreadyBecame;
		}
		break;

	case Activity::Shrugger:
	case Activity::Shrugger2:
		m_lixxie.become(Activity::Walker);
		m_lixxie.turn();
		m_lixxie.frame = -1;
		return AfterAssignment::WeAlreadyBecame;
	}

	m_lixxie.become(Activity::Walker);
	m_lixxie.frame = -1;
	return AfterAssignment::WeAlreadyBecame;
}

void Walker::onBecome(const Job& old)
{
	if (m_lixxie.abilityToRun)
		m_lixxie.become(Activity::Runner);
	else
		setFrameAfterShortFallTo(old, 8);
}

void Walker::perform()
{
	if (m_lixxie.isLastFrame())
		frame = 3;
	else
		m_lixxie.advanceFrame();

	performWalkingOrRunning();
}

void Walker::performWalkingOrRunning()
{
	const int oldEx = m_lixxie.ex;
	const int oldEy = m_lixxie.ey;
	const Map::Flags oldEncBody = m_lixxie.bodyEncounters;
	const Map::Flags oldEncFoot = m_lixxie.footEncounters;

	if (frame != 0)
		m_lixxie.moveAhead();

	const bool turnAfterAll = handleWallOrPitHere();

	if (turnAfterAll)
	{
		m_lixxie.ex = oldEx;
		m_lixxie.ey = oldEy;
		m_lixxie.forceBodyAndFootEncounters(oldEncBody, oldEncFoot);

		// LIXBOT: unused variable
		// bool climbedAfterAll = false;

		if (m_lixxie.abilityToClimb)
		{
			bool enoughSpaceToClimb = true;

			for (int i = 1; i <= HighestStepUp; ++i)
			{
				if (m_lixxie.isSolid(0, -i))
				{
					enoughSpaceToClimb = false;
					break;
				}
			}

			if (enoughSpaceToClimb)
			{
				m_lixxie.become(Activity::Climber);
				return;
			}
		}

		m_lixxie.turn();
		handleWallOrPitHere();
	}
}

void Walker::setFrameAfterShortFallTo(const Job& old, int targetFrame)
{
	if (old.activity == Activity::Faller)
	{
		auto faller = dynamic_cast<const Faller*>(&old);
		assert(faller);

		if ((faller->pixelsFallen <= 9 && faller->frame < 1) ||
			faller->pixelsFallen == 0)
			frame = targetFrame;

		else if (faller->frame < 2)
			frame = 0;
	}
}

bool Walker::handleWallOrPitHere() const
{
	if (m_lixxie.isSolid() || m_lixxie.isSolid(0, 1))
	{
		const int upBy = m_lixxie.solidWallHeight(0);

		if (upBy > HighestStepUp)
			return true;
		else if (upBy >= 6)
			m_lixxie.become(Activity::Ascender);
		else
			m_lixxie.moveUp(upBy);
	}

	else
	{
		assert(!m_lixxie.isSolid(0, 1) && !m_lixxie.isSolid(0, 2));

		constexpr int spaceBelowForAnyFalling = 7;
		constexpr int spaceBelowForNormalFalling = 9;
		int spaceBelow = 1;

		while (spaceBelow < spaceBelowForNormalFalling
			&& !m_lixxie.isSolid(0, spaceBelow + 2))
			++spaceBelow;

		if (spaceBelow >= spaceBelowForNormalFalling)
			Faller::becomeAndFallPixels(m_lixxie, 2);
		else if (spaceBelow >= spaceBelowForAnyFalling)
			Faller::becomeAndFallPixels(m_lixxie, spaceBelow - 4);
		else
			m_lixxie.moveDown(spaceBelow);
	}

	return false;
}

AfterAssignment Runner::onManualAssignment(Job& old)
{
	// LIXBOT: turn off ability to run
	if (m_lixxie.abilityToRun)
	{
		m_lixxie.abilityToRun = false;

		if (old.activity == Activity::Runner)
		{
			m_lixxie.become(Activity::Walker);
			m_lixxie.frame = 3;
			return AfterAssignment::WeAlreadyBecame;
		}

		return AfterAssignment::DoNotBecome;
	}
	//

	assert(!m_lixxie.abilityToRun);
	m_lixxie.abilityToRun = true;

	if (old.activity == Activity::Walker)
	{
		m_lixxie.become(Activity::Runner);
		m_lixxie.frame = 2;
		return AfterAssignment::WeAlreadyBecame;
	}

	else
		return AfterAssignment::DoNotBecome;
}

void Runner::onBecome(const Job& old)
{
	assert(m_lixxie.abilityToRun);
	setFrameAfterShortFallTo(old, 6);
}

void Runner::perform()
{
	if (m_lixxie.isLastFrame())
		frame = 1;
	else
		m_lixxie.advanceFrame();

	const int oldDir = m_lixxie.direction;

	performWalkingOrRunning();

	if (m_lixxie.activity == Activity::Runner && oldDir == m_lixxie.direction)
		performWalkingOrRunning();
}
