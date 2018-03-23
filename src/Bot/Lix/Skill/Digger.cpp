#include "Digger.hpp"
#include "../World.hpp"
#include "../Mask.hpp"

PhysicsUpdateOrder Digger::updateOrder() const
{
	return PhysicsUpdateOrder::Remover;
}

void Digger::perform()
{
	if (m_lixxie.isLastFrame())
		frame = 4;
	else
		m_lixxie.advanceFrame();

	bool weWillFall = false;

	if (frame != 16)
		weWillFall = shouldWeFallHere();

	else
	{
		int rowsToDig = 0;

		while (rowsToDig < 4)
		{
			if (hitEnoughSteel())
				break;

			++rowsToDig;
			m_lixxie.moveDown(1);
			weWillFall = shouldWeFallHere();

			if (weWillFall)
				break;
		}

		if (rowsToDig > 0)
		{
			const int plusUpstroke = (m_upstrokeDone ? 0 : 4);

			removeRowsYInterval(2 - rowsToDig - plusUpstroke,
									rowsToDig + plusUpstroke);
		}

		m_upstrokeDone = true;
	}

	if (weWillFall)
		m_lixxie.become(Activity::Faller);
}

bool Digger::hitEnoughSteel()
{
	constexpr int midLoRes = 5;
	const bool enoughSteel = m_lixxie.countSteel(1 - midLoRes, 2, midLoRes, 2) > 0;

	if (enoughSteel)
	{
		/*
		if (outsideWorld.effect)
			outsideWorld.effect.addDigHammer(outsideWorld.state.update,
				style, outsideWorld.lixID, ex, ey, dir);
		*/

		m_lixxie.world.addDigHammer(m_lixxie.ex, m_lixxie.ey, m_lixxie.direction);

		m_lixxie.become(Activity::Walker);
	}

	return enoughSteel;
}

bool Digger::shouldWeFallHere() const
{
	return !m_lixxie.isSolid(-2, 2) && !m_lixxie.isSolid(0, 2) && !m_lixxie.isSolid(2, 2);
}

void Digger::removeRowsYInterval(int y, int yl)
{
	TerrainDeletion tc;
	tc.update = m_lixxie.world.update;
	tc.type   = TerrainDeletion::Dig;
	tc.x      = m_lixxie.ex - 8;
	tc.y      = m_lixxie.ey + y;
	tc.digYl  = yl;

	m_lixxie.world.physicsDrawer.add(std::move(tc));
}
