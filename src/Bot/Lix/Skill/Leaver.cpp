#include "Leaver.hpp"
#include "../Lixxie.hpp"

#include <cassert>

bool RemovedLix::blockable() const
{
	return false;
}

void RemovedLix::onBecome(const Job& old)
{
	assert(old.activity != Activity::Nothing);

	/*
	if (JobUnion.healthy(old.ac))
		outsideWorld.tribe.recordOutToLeaver(lixxie.outsideWorld.state.update);

	outsideWorld.tribe.recordLeaverDone();
	*/

	m_lixxie.ploderTimer = 0;
}

bool Leaver::blockable() const
{
	return false;
}

void Leaver::onBecome(const Job& old)
{
	// lixxie.outsideWorld.tribe.recordOutToLeaver(lixxie.outsideWorld.state.update);

	onBecomeLeaver(old);
}

void Leaver::perform()
{
	advanceFrameAndLeave();
}

void Leaver::onBecomeLeaver(const Job& old)
{
}

void Leaver::advanceFrameAndLeave()
{
	if (m_lixxie.isLastFrame())
		m_lixxie.become(Activity::Nothing);
	else
		m_lixxie.advanceFrame();
}

void Splatter::onBecomeLeaver(const Job& old)
{
	m_lixxie.playSound("splat");
}

void Burner::onBecomeLeaver(const Job& old)
{
	m_lixxie.playSound("fire");
}

void Drowner::onBecomeLeaver(const Job& old)
{
	m_lixxie.playSound("water");

	if (old.activity == Activity::Tumbler && old.frame >= 9)
		frame = 6;
}
