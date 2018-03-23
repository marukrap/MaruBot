#include "Batter.hpp"
#include "../World.hpp"

PhysicsUpdateOrder Batter::updateOrder() const
{
	if (frame == FlingAfterFrame)
		return PhysicsUpdateOrder::Flinger;
	else
		return PhysicsUpdateOrder::Peaceful;
}

bool Batter::blockable() const
{
	return false;
}

void Batter::perform()
{
	const bool batNow = (updateOrder() == PhysicsUpdateOrder::Flinger);

	if (!m_lixxie.isSolid())
		m_lixxie.become(Activity::Faller);
	else if (m_lixxie.isLastFrame())
		m_lixxie.become(Activity::Walker);
	else
		m_lixxie.advanceFrame();

	if (batNow)
	{
		bool hit = false;

		/*
		foreach (Tribe battedTribe; outsideWorld.state.tribes)
			foreach (id, Lixxie batted; battedTribe.lixvec.enumerate!int)
				if (flingIfCloseTo(batted, lixxie.ex + 6 * lixxie.dir, lixxie.ey - 4))
				{
					hit = true;

					if (lixxie.outsideWorld.effect)
						lixxie.outsideWorld.effect.addSound(
							lixxie.outsideWorld.state.update, batted.style,
							id, Sound.BATTER_HIT);
				}
		*/

		for (auto& batted : m_lixxie.world.tribe)
		{
			if (flingIfCloseTo(*batted, m_lixxie.ex + 6 * m_lixxie.direction, m_lixxie.ey - 4))
				hit = true;
		}

		m_lixxie.playSound(hit ? "bat_hit" : "bat_miss");
	}
}

bool Batter::flingIfCloseTo(Lixxie& target, int cx, int cy) const
{
	if (!target.healthy())
		return false;

	const bool sameTribe = (target.style == m_lixxie.style);

	if (m_lixxie.flingNew && sameTribe &&
		target.activity == Activity::Batter && target.frame == frame)
		return false;

	const bool blo = (target.activity == Activity::Blocker);
	const int ch = (blo ? cx + ExtraXRangeForBlockers * m_lixxie.direction: cx);
	const int dx = std::abs(m_lixxie.env.distanceX(ch, target.ex));
	const int dy = std::abs(m_lixxie.env.distanceY(cy, target.ey));
	const bool fling = (dx <= RectHalfXl + (blo ? ExtraXRangeForBlockers : 0)
					 && dy <= RectHalfYl && &target != &m_lixxie);

	if (fling)
		target.addFling(FlingSpeedX * m_lixxie.direction, FlingSpeedY, sameTribe);

	return fling;
}
