#include "Builder.hpp"
#include "Walker.hpp"
#include "../World.hpp"
#include "../Mask.hpp"

#include <cassert>

PhysicsUpdateOrder BrickCounter::updateOrder() const
{
	return PhysicsUpdateOrder::Adder;
}

AfterAssignment BrickCounter::onManualAssignment(Job& old)
{
	if (activity != old.activity)
		return AfterAssignment::BecomeNormally;

	else
	{
		auto counter = dynamic_cast<BrickCounter*>(&old);
		assert(counter);
		counter->m_skillsQueued = counter->m_skillsQueued + 1;
		return AfterAssignment::DoNotBecome;
	}
}

void BrickCounter::onBecome(const Job& old)
{
	m_bricksLeft = BricksAtStart;
	frame = startFrame(old);
}

void BrickCounter::perform()
{
	if (m_lixxie.turnedByBlocker)
		buildBrickForFree();

	onPerform();
}

/*
void BrickCounter::returnSkillsDontCallLixxieInHere(Tribe& tribe)
{
	tribe.returnSkills(activity, skillsQueued);
	m_skillsQueued = 0;
}
*/

void BrickCounter::buildBrick()
{
	assert(m_bricksLeft > 0);
	--m_bricksLeft;

	if (m_bricksLeft < 3 && m_skillsQueued == 0)
		m_lixxie.playSound("brick");

	onBuildingBrick();
}

void BrickCounter::buildBrickForFree()
{
	onBuildingBrick();
}

Job::BecomeCalled BrickCounter::maybeBecomeShrugger(Activity shruggingAc)
{
	assert(m_bricksLeft >= 0);
	assert(m_skillsQueued >= 0);

	if (m_bricksLeft > 0)
		return BecomeCalled::No;

	else if (m_skillsQueued > 0)
	{
		--m_skillsQueued;
		m_bricksLeft += BricksAtStart;
		return BecomeCalled::No;
	}

	else
	{
		m_lixxie.become(shruggingAc);
		return BecomeCalled::Yes;
	}
}

int Builder::startFrame(const Job& old) const
{
	return 6;
}

void Builder::onPerform()
{
	m_lixxie.advanceFrame();

	if (frame == 0)
		maybeBecomeShrugger(Activity::Shrugger);

	else if (frame == 8)
	{
		m_fullyInsideTerrain = m_lixxie.solidWallHeight(0, 2) > Walker::HighestStepUp;

		m_lixxie.moveUp(2);
		buildBrick();
	}

	else if (frame == 12)
		bumpAgainstTerrain();

	else if (frame == 13 || frame == 14)
		m_lixxie.moveAhead();
}

void Builder::onBuildingBrick()
{
	TerrainAddition tc;
	tc.update = m_lixxie.world.update;
	tc.type   = TerrainAddition::Build;
	tc.style  = m_lixxie.style;
	tc.x      = m_lixxie.facingRight ? m_lixxie.ex : m_lixxie.ex - 10;
	tc.y      = m_lixxie.ey + 2;

	m_lixxie.world.physicsDrawer.add(std::move(tc));
}

Job::BecomeCalled Builder::bumpAgainstTerrain()
{
	const bool wallNearFoot = (m_lixxie.isSolid(4, 1) && m_lixxie.isSolid(4, -2))
						   || (m_lixxie.isSolid(2, 1) && m_lixxie.isSolid(2, -2));
	const bool insideThinHorizontalBeam =
		m_lixxie.isSolid(4, 1) && m_lixxie.isSolid(2, 1) && m_lixxie.isSolid(0, 1);
	const bool hitHead = m_lixxie.isSolid(4, -16);

	if (wallNearFoot || insideThinHorizontalBeam || hitHead)
	{
		m_lixxie.turn();

		if (m_fullyInsideTerrain)
			m_lixxie.moveDown(2);

		m_lixxie.become(Activity::Walker);
		return BecomeCalled::Yes;
	}

	else
		return BecomeCalled::No;
}

int Platformer::startFrame(const Job& old) const
{
	if (old.activity == Activity::Shrugger2 && old.frame < StandingUpFrame)
		return 16;
	else
		return 0;
}

void Platformer::onPerform()
{
	constexpr int loopBackToFrame = 10;
	bool loopCompleted = false;

	if (m_lixxie.isLastFrame())
	{
		assert(frame == 25);
		frame = loopBackToFrame;
		loopCompleted = true;
	}

	else
		m_lixxie.advanceFrame();

	if (frame == 2)
		buildBrick();

	else if (frame == 5)
		planNextBrickFirstCycle();

	else if (frame == 7)
	{
		moveUpAndCollide();
		moveAheadAndCollide();
	}

	else if (frame == 8)
		moveAheadAndCollide();

	else if (frame == loopBackToFrame)
	{
		if (loopCompleted && maybeBecomeShrugger(Activity::Shrugger2) == BecomeCalled::No)
			planNextBrickSubsequentCycles();
	}

	else if (frame == 18)
		buildBrick();

	else if (frame >= 22 && frame < 25)
		moveAheadAndCollide();
}

void Platformer::abortAndStandUp(Lixxie& lixxie)
{
	lixxie.become(Activity::Shrugger2);
	assert(lixxie.job.activity == Activity::Shrugger2);
	lixxie.job.frame = StandingUpFrame;
}

void Platformer::onBuildingBrick()
{
	const bool firstCycle = (frame == 2);

	TerrainAddition tc;
	tc.update = m_lixxie.world.update;
	tc.type   = firstCycle ? TerrainAddition::PlatformLong
						   : TerrainAddition::PlatformShort;
	tc.style  = m_lixxie.style;
	tc.y      = firstCycle ? m_lixxie.ey : m_lixxie.ey + 2;
	tc.x      = firstCycle ? (m_lixxie.facingRight ? m_lixxie.ex : m_lixxie.ex - 6)
						   : (m_lixxie.facingRight ? m_lixxie.ex + 4 : m_lixxie.ex - 8);

	m_lixxie.world.physicsDrawer.add(std::move(tc));
}

bool Platformer::platformerTreatsAsSolid(int x, int y)
{
	if (!m_lixxie.isSolid(x, y))
		return false;

	if (m_lixxie.isSolid(x + 2, y) && m_lixxie.isSolid(x + 4, y))
		return true;

	assert(m_lixxie.isSolid(x, y));

	return m_lixxie.isSolid(x + 2, y - 2)
		|| m_lixxie.isSolid(x    , y - 2)
		|| m_lixxie.isSolid(x - 2, y - 2);
}

void Platformer::planNextBrickFirstCycle()
{
	if (platformerTreatsAsSolid(6, -1) &&
		platformerTreatsAsSolid(8, -1) &&
		platformerTreatsAsSolid(10, -1))
		m_lixxie.become(Activity::Walker);
}

void Platformer::planNextBrickSubsequentCycles()
{
	assert(this == &m_lixxie.job);

	if (platformerTreatsAsSolid(2, 1) &&
		platformerTreatsAsSolid(4, 1) &&
		platformerTreatsAsSolid(6, 1))
		abortAndStandUp(m_lixxie);
}

void Platformer::moveUpAndCollide()
{
	const bool airAbove = !m_lixxie.isSolid(0, -1);

	if (airAbove)
		m_lixxie.moveUp(2);
	else
		abortAndStandUp(m_lixxie);
}

void Platformer::moveAheadAndCollide()
{
	if (!platformerTreatsAsSolid(2, 1))
		m_lixxie.moveAhead();
	else
		abortAndStandUp(m_lixxie);
}

void Shrugger::perform()
{
	if (m_lixxie.isLastFrame())
		m_lixxie.become(Activity::Walker);
	else
		m_lixxie.advanceFrame();
}
