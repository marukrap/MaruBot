#include "Exploder.hpp"
#include "../World.hpp"
#include "../Mask.hpp"
#include "../Help.hpp"

#include <cassert>

PhysicsUpdateOrder Ploder::updateOrder() const
{
	return PhysicsUpdateOrder::Flinger;
}

bool Ploder::blockable() const
{
	return false;
}

AfterAssignment Ploder::onManualAssignment(Job& old)
{
	assert(m_lixxie.ploderTimer == 0);
	m_lixxie.ploderTimer = m_lixxie.ploderTimer + 1;
	m_lixxie.ploderIsExploder = (activity == Activity::Exploder);
	return AfterAssignment::DoNotBecome;
}

void Ploder::perform()
{
	changeTerrain();
	flingOtherLix();
	makeEffect();
	m_lixxie.become(Activity::Nothing);
}

void Ploder::onBecomeLeaver(const Job& old)
{
}

void Ploder::flingOtherLix()
{
}

void Ploder::changeTerrain()
{
	assert(activity == Activity::Imploder || activity == Activity::Exploder);

	TerrainDeletion tc;
	tc.update = m_lixxie.world.update;
	tc.type   = (activity == Activity::Exploder) ? TerrainDeletion::Explode
												 : TerrainDeletion::Implode;

	const Mask& mask = Mask::get(tc.type);
	tc.x = -mask.offsetX + m_lixxie.ex;
	tc.y = -mask.offsetY + m_lixxie.ey;

	m_lixxie.world.physicsDrawer.add(std::move(tc));
}

void Imploder::makeEffect()
{
	/*
	if (outsideWorld.effect)
		outsideWorld.effect.addImplosion(outsideWorld.state.update, style,
										 outsideWorld.lixID, ex, ey);
	*/

	m_lixxie.world.addImplosion(m_lixxie.ex, m_lixxie.ey);
}

void Exploder::makeEffect()
{
	/*
	if (outsideWorld.effect)
		outsideWorld.effect.addExplosion(outsideWorld.state.update, style,
										 outsideWorld.lixID, ex, ey);
	*/

	m_lixxie.world.addExplosion(m_lixxie.ex, m_lixxie.ey);
}

void Exploder::flingOtherLix()
{
	/*
	foreach (targetTribe; outsideWorld.state.tribes)
		foreach (target; targetTribe.lixvec)
		if (target.healthy)
			flingOtherLix(target, targetTribe.style == this.style);
	*/

	for (auto& target : m_lixxie.world.tribe)
	{
		if (target->healthy())
			// flingOtherLix(*target, target->style == m_lixxie.style);
			flingOtherLix(*target, true);
	}
}

void Exploder::flingOtherLix(Lixxie& target, bool targetTribeIsOurTribe)
{
	const int dx = m_lixxie.env.distanceX(m_lixxie.ex, target.ex);
	const int dy = m_lixxie.env.distanceY(m_lixxie.ey + 4, target.ey);

	const double distSquared = dx * dx + dy * dy;

	constexpr double range = 23 * 2.5 + 0.5;
	constexpr double rangeSquared = range * range; // range^2

	if (distSquared <= rangeSquared)
	{
		double sx = 0;
		double sy = 0;

		if (distSquared > 0)
		{
			constexpr int strengthX = 350;
			constexpr int strengthY = 330;
			constexpr int centerConst = 20;

			const double dist = std::sqrt(distSquared);

			sx = strengthX * dx / (dist * (dist + centerConst));
			sy = strengthY * dy / (dist * (dist + centerConst));
		}

		if (sy > -10)
			sy += -5;
		else if (sy > -20)
			sy += (-20 - sy) / 2;

		target.addFling(roundInt(sx), roundInt(sy), targetTribeIsOurTribe);
	}
}
