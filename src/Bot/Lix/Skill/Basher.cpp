#include "Basher.hpp"
#include "Faller.hpp"
#include "../World.hpp"
#include "../Mask.hpp"

#include <cassert>

PhysicsUpdateOrder Basher::updateOrder() const
{
	return PhysicsUpdateOrder::Remover;
}

void Basher::onBecome(const Job& old)
{
	frame = 2;
}

void Basher::perform()
{
	m_lixxie.advanceFrame();

	if (frame == 7)
		performSwing();

	else if (frame == 10 && (m_steelWasHit || nothingMoreToBash()))
	{
		if (m_steelWasHit)
			m_lixxie.turn();

		m_lixxie.become(Activity::Walker);
		return;
	}

	else if (frame >= 11 && frame < 16)
		m_lixxie.moveAhead();

	stopIfMovedDownTooFar();
}

bool Basher::nothingMoreToBash()
{
	const int earth = m_lixxie.countSolid(16, -14, 23, -3);

	if (earth < 15)
	{
		for (int x = 16; x <= 23; x += 2)
			if (m_lixxie.isSolid(x, -12))
			{
				m_swingsLeft = 0;
				return false;
			}

		// LIXBOT: swingsLeft
		if (m_swingsLeft > 0)
			return false;

		return true;
	}

	m_swingsLeft = 0;
	return false;
}

void Basher::performSwing()
{
	auto omitRelics = [this] ()
	{
		const int earthAfter = m_lixxie.countSolid(16, -16, 17, 1);
		const int pathClear = nothingMoreToBash();

		return earthAfter == 0 && pathClear;
	};

	TerrainDeletion tc;
	tc.update = m_lixxie.world.update;

	if (omitRelics())
	{
		if (m_lixxie.facingRight)
			tc.type = TerrainDeletion::BashNoRelicsRight;
		else
			tc.type = TerrainDeletion::BashNoRelicsLeft;
	}

	else
	{
		if (m_lixxie.facingRight)
			tc.type = TerrainDeletion::BashRight;
		else
			tc.type = TerrainDeletion::BashLeft;
	}

	const Mask& mask = Mask::get(tc.type);
	tc.x = m_lixxie.ex - mask.offsetX;
	tc.y = m_lixxie.ey - mask.offsetY;

	m_lixxie.world.physicsDrawer.add(std::move(tc));

	if (m_lixxie.wouldHitSteel(mask))
	{
		m_lixxie.playSound("steel");
		m_steelWasHit = true;
	}

	// LIXBOT: swingsLeft
	if (m_swingsLeft > 0)
		--m_swingsLeft;
}

void Basher::stopIfMovedDownTooFar()
{
	const int stepSize = [this] ()
	{
		assert(this == &m_lixxie.job);
		assert(m_halfPixelsMovedDown < HalfPixelsToFall);

		for (int y = 0; 2 * y < HalfPixelsToFall - m_halfPixelsMovedDown; ++y)
			if (m_lixxie.isSolid(0, 2 + y))
				return y;

		return -1;
	}();

	if (stepSize >= 0)
	{
		m_lixxie.moveDown(stepSize);
		m_halfPixelsMovedDown += 2 * stepSize;
		assert(m_halfPixelsMovedDown < HalfPixelsToFall);

		if (m_halfPixelsMovedDown > 0)
			--m_halfPixelsMovedDown;
	}

	else
	{
		constexpr int fallUpTo = 2;
		int y = 0;

		while (!m_lixxie.isSolid(0, 2 + y) && y < fallUpTo)
			++y;

		if (m_lixxie.isSolid(0, 2 + y))
		{
			m_lixxie.moveDown(y);
			m_lixxie.become(Activity::Walker);
		}

		else
			Faller::becomeAndFallPixels(m_lixxie, y);
	}
}
