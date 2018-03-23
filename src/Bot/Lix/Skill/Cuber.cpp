#include "Cuber.hpp"
#include "../World.hpp"
#include "../Mask.hpp"

#include <cassert>

PhysicsUpdateOrder Cuber::updateOrder() const
{
	return PhysicsUpdateOrder::Adder;
}

void Cuber::perform()
{
	if (frame >= 2)
	{
		TerrainAddition tc;
		tc.update = m_lixxie.world.update;
		tc.type   = TerrainAddition::Cube;
		tc.style  = m_lixxie.style;
		tc.x      = m_lixxie.ex - CubeSize / 2;

		assert(m_lixxie.isLastFrame() == (frame == 5));
		tc.cubeYl = m_lixxie.isLastFrame() ? CubeSize : 2 * frame - 2;
		assert(tc.cubeYl > 0);

		tc.y = m_lixxie.ey - tc.cubeYl + 2;

		m_lixxie.world.physicsDrawer.add(std::move(tc));
	}

	advanceFrameAndLeave();
}

void Cuber::onBecomeLeaver(const Job& old)
{
	if (m_lixxie.facingLeft)
	{
		m_lixxie.turn();
		m_lixxie.moveAhead();
	}
}
