#include "Miner.hpp"
#include "Faller.hpp"
#include "../World.hpp"
#include "../Mask.hpp"

#include <cassert>

PhysicsUpdateOrder Miner::updateOrder() const
{
	return PhysicsUpdateOrder::Remover;
}

void Miner::perform()
{
	m_lixxie.advanceFrame();

	if (frame == 1)
	{
		checkFutureGround();
		antiShock(MaxGapDepth);
	}

	else if (frame == 2)
	{
		removeEarth();
		m_movedDownSinceSwing = 0;
		antiShock(MaxGapDepth + 1);
	}

	else if (frame >= 3 && frame < 7)
		antiShock(MaxGapDepth + 1);

	else if (frame >= 7 && frame < 12)
		normalMovement();

	else if (frame >= 12 || frame == 0)
		antiShock(MaxGapDepth);
}

void Miner::antiShock(int resiliance)
{
	const int downThisFrame = antiShockMoveDown(resiliance);

	if (!m_lixxie.isSolid() || m_movedDownSinceSwing > resiliance)
		becomeFallerWithAlreadyFallenPixels(downThisFrame);
}

int Miner::antiShockMoveDown(int maxDepth)
{
	int downThisFrame = 0;

	while (downThisFrame < maxDepth && !m_lixxie.isSolid(0, 2 + downThisFrame))
	{
		++downThisFrame;
		++m_movedDownSinceSwing;
	}

	if (m_lixxie.isSolid(0, 2 + downThisFrame))
	{
		m_lixxie.moveDown(downThisFrame);
		return downThisFrame;
	}

	else
		return 0;
}

void Miner::removeEarth()
{
	TerrainDeletion tc;
	tc.update = m_lixxie.world.update;
	tc.type   = m_lixxie.facingRight ? TerrainDeletion::MineRight
									 : TerrainDeletion::MineLeft;

	const Mask& mask = Mask::get(tc.type);
	tc.x = m_lixxie.ex - mask.offsetX;
	tc.y = m_lixxie.ey - mask.offsetY;

	m_lixxie.world.physicsDrawer.add(std::move(tc));

	if (m_lixxie.wouldHitSteel(mask))
	{
		/*
		if (outsideWorld.effect)
			outsideWorld.effect.addPickaxe(outsideWorld.state.update,
				style, outsideWorld.lixID, ex, ey, dir);
		*/

		m_lixxie.world.addPickaxe(m_lixxie.ex, m_lixxie.ey, m_lixxie.direction);

		m_lixxie.turn();
		m_lixxie.become(Activity::Walker);
	}
}

void Miner::checkFutureGround()
{
	for (int j = 0; j < FutureLength; ++j)
		m_futureGroundIsSolid[j] = m_lixxie.isSolid(2 * j + 2,
			2 + (j + 1) + std::max(m_movedDownSinceSwing - j, 0));
}

void Miner::normalMovement()
{
	int downThisFrame = 0;

	if (frame != 9)
	{
		m_lixxie.moveAhead();

		if (m_movedDownSinceSwing == 0)
		{
			m_lixxie.moveDown(1);
			downThisFrame = 1;
		}

		else
			m_movedDownSinceSwing -= 1;
	}

	assert(frame >= 7 && frame < 12);

	const int future = frame == 7 ? 0
					 : frame == 8 || frame == 9 ? 1
					 : frame == 10 ? 2 : 3;

	if (m_futureGroundIsSolid[future])
		downThisFrame += antiShockMoveDown(MaxGapDepth);

	const bool downTooFar = m_movedDownSinceSwing > MaxGapDepth;
	const bool solid = m_lixxie.isSolid(0, 2) || m_futureGroundIsSolid[future];
	const bool leeway = (frame == 7 || frame == 10) && m_lixxie.isSolid(0, 3);

	if (downTooFar || (!solid && !leeway))
		becomeFallerWithAlreadyFallenPixels(downThisFrame);
}

void Miner::becomeFallerWithAlreadyFallenPixels(int downThisFrame)
{
	assert(this == &m_lixxie.job);
	m_lixxie.become(Activity::Faller);

	auto faller = dynamic_cast<Faller*>(&m_lixxie.job);
	assert(faller);
	faller->pixelsFallen = downThisFrame;
}
