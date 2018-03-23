#include "Blocker.hpp"
#include "../World.hpp"

#include <cassert>

PhysicsUpdateOrder Blocker::updateOrder() const
{
	return PhysicsUpdateOrder::Blocker;
}

bool Blocker::blockable() const
{
	return false;
}

AfterAssignment Blocker::onManualAssignment(Job& old)
{
	// LIXBOT: turn off blocker
	if (old.activity == Activity::Blocker)
	{
		if (old.frame < 20)
			old.frame = 20;
		else
			m_lixxie.turn();

		return AfterAssignment::DoNotBecome;
	}
	//

	return AfterAssignment::BecomeNormally;
}

void Blocker::perform()
{
	if (!m_lixxie.isSolid())
	{
		m_lixxie.become(Activity::Faller);
		return;
	}

	else if (frame == 19)
		frame = 4;

	else if (m_lixxie.isLastFrame())
	{
		m_lixxie.become(Activity::Walker);
		return;
	}

	else
		m_lixxie.advanceFrame();

	assert(m_lixxie.activity == Activity::Blocker);
	blockOtherLix();
}

void Blocker::blockOtherLix()
{
	/*
	foreach (Tribe tribe; outsideWorld.state.tribes)
		foreach (Lixxie li; tribe.lixvec)
			if (li.job.blockable)
				blockSingleLix(li);
	*/

	for (auto& lix : m_lixxie.world.tribe)
		if (lix->job.blockable())
			blockSingleLix(*lix);
}

void Blocker::blockSingleLix(Lixxie& target)
{
	const int dx = m_lixxie.env.distanceX(target.ex, m_lixxie.ex);
	const int dy = m_lixxie.env.distanceX(target.ey, m_lixxie.ey);

	if (std::abs(dx) < ForceFieldXlEachSide &&
		dy > -ForceFieldYlBelow &&
		dy <  ForceFieldYlAbove)
	{
		const int blockedByL = target.facingRight && dx > 0;
		const int blockedByR = target.facingLeft  && dx < 0;

		if ((blockedByL || blockedByR) && !target.turnedByBlocker)
		{
			target.turn();
			target.turnedByBlocker = true;
		}

		target.inBlockerFieldLeft  = target.inBlockerFieldLeft  || blockedByL;
		target.inBlockerFieldRight = target.inBlockerFieldRight || blockedByR;
	}
}
