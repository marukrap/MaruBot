#include "Exiter.hpp"
// #include "../Lixxie.hpp"

void Exiter::perform()
{
	const int change = (m_xOffsetFromGoal < 0 ? 1 : m_xOffsetFromGoal > 0 ? -1 : 0);

	spriteOffsetX = spriteOffsetX + change;
	m_xOffsetFromGoal += change;

	advanceFrameAndLeave();

}

/*
void Exiter::scoreForTribe(Tribe& tribe)
{
	tribe.addSaved(this.style, outsideWorld.state.update);
}

void Exiter::determineSidewaysMotion(const Goal& goal)
{
	xOffsetFromGoal = env.distanceX(
		goal.x + goal.tile.trigger.x + goal.tile.triggerXl / 2, lixxie.ex);

	if (xOffsetFromGoal % 2 == 0)
		xOffsetFromGoal += 1;
}

void Exiter::playSound(const Goal& goal)
{
	if (goal.hasTribe(style))
		lixxie.playSound(Sound.GOAL);

	else
	{
		lixxie.playSound(Sound.GOAL_BAD);
		foreach (tr; goal.tribes)
			outsideWorld.effect.addSound(outsideWorld.state.update, tr,
				outsideWorld.lixID,
				Sound.GOAL);
	}
}
*/