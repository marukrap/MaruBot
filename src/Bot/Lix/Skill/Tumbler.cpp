#include "Tumbler.hpp"
#include "Walker.hpp"
#include "../Lixxie.hpp"
#include "../Help.hpp"

#include <cassert>

void BallisticFlyer::perform()
{
	assert(speedX >= 0);

	if (speedX % 2 != 0)
		++speedX;

	if (moveSeveralTimes() == BecomeCalled::Yes)
		return;

	speedY += accel(speedY);
	selectFrame();

	if (speedY >= SpeedYToFloat)
	{
		if (m_lixxie.abilityToFloat)
			m_lixxie.become(Activity::Floater);
		else if (activity == Activity::Jumper)
			m_lixxie.become(Activity::Tumbler);
	}
}

void BallisticFlyer::copyFrom(const BallisticFlyer& rhs)
{
	speedX = rhs.speedX;
	speedY = rhs.speedY;
	pixelsFallen = rhs.pixelsFallen;
}

bool BallisticFlyer::wall(int y) const
{
	return m_lixxie.isSolid(0, y);
}

bool BallisticFlyer::behind(int y) const
{
	return wall(y) && m_lixxie.isSolid(-2, y);
}

int BallisticFlyer::accel(int ysp)
{
	return (ysp <= BiggestLargeAccelSpeedY) ? 2 : (ysp < 32) ? 1 : 0;
}

bool BallisticFlyer::splatUpsideDown() const
{
	return false;
}

Job::BecomeCalled BallisticFlyer::landOnFloor()
{
	if (pixelsFallen > PixelsSafeToFall && !m_lixxie.abilityToFloat)
	{
		const bool sud = splatUpsideDown();
		m_lixxie.become(Activity::Splatter);

		if (sud)
			m_lixxie.frame = 10;

		return BecomeCalled::Yes;
	}

	else
		return onLandingWithoutSplatting();
}

Job::BecomeCalled BallisticFlyer::moveSeveralTimes()
{
	const int ySgn = speedY >= 0 ? 1 : -1;
	const int yAbs = std::abs(speedY);

	const int maxSteps = std::max(yAbs, speedX);

	// LIXBOT: unused label
	// BREAK_MOTION:
	for (int step = 0; step < maxSteps; ++step)
	{
		const int oldEx = m_lixxie.ex;
		const int oldEy = m_lixxie.ey;
		const int oldFallen = pixelsFallen;
		const Map::Flags oldEncBody = m_lixxie.bodyEncounters;
		const Map::Flags oldEncFoot = m_lixxie.footEncounters;

		auto moveDownCounting = [this] (int by)
		{
			m_lixxie.moveDown(by);

			if (by >= 0)
				pixelsFallen += by;
			else
				pixelsFallen = 0;
		};

		if (yAbs >= speedX)
		{
			const int halfX = (step + 1) * speedX / 2 / yAbs
							- (step)     * speedX / 2 / yAbs;
			assert(halfX == 0 || halfX == 1);
			m_lixxie.moveAhead(2 * halfX);
			moveDownCounting(ySgn);
		}

		else
		{
			moveDownCounting(ySgn * ( (step + 1) * yAbs / speedX
									- (step)     * yAbs / speedX));
			m_lixxie.moveAhead(2 * (step & 1));
		}

		const Collision col = collision();

		if (col.pleaseDo == PleaseDo::Nothing ||
			col.pleaseDo == PleaseDo::ResetPositionIfMovedX && m_lixxie.ex == oldEx)
		{
		}

		else if (col.pleaseDo == PleaseDo::StopMovement)
			return col.becomeCalled;

		else if (col.pleaseDo == PleaseDo::ResetEncounters)
		{
			m_lixxie.forceBodyAndFootEncounters(oldEncBody, oldEncFoot);
 			m_lixxie.ey = m_lixxie.ey; // re-check encounters here
			return col.becomeCalled;
		}

		else if (col.pleaseDo == PleaseDo::ResetPosition
			  || col.pleaseDo == PleaseDo::ResetPositionIfMovedX && m_lixxie.ex != oldEx)
		{
			m_lixxie.forceBodyAndFootEncounters(oldEncBody, oldEncFoot);
			m_lixxie.ex = oldEx;
			m_lixxie.ey = oldEy;
			pixelsFallen = oldFallen;

			if (m_lixxie.isSolid(0, 1))
			{
				m_lixxie.become(Activity::Stunner);
				return BecomeCalled::Yes;
			}

			return col.becomeCalled;
		}

		else
			assert(false);
	}

	return BecomeCalled::No;
}

BallisticFlyer::Collision BallisticFlyer::collision()
{
	int wallCount = 0;
	int wallCountT = 0;
	int swh = 0;
	int lowestFloor = -999;
	int behindCount = 0;

	for (int i = 1; i > -16; --i)
	{
		if (wall(i))
		{
			++wallCount;

			if (i <= -1 && i > -11)
				++wallCountT;
		}
	}

	for (int i = 1; i > -16; --i)
	{
		if (wall(i))
			++swh;
		else
			break;
	}

	for (int i = -1; i > -9; --i)
		behindCount += behind(i);

	for (int i = -1; i > -15; --i)
	{
		if (!wall(i - 1) && wall(i))
		{
			lowestFloor = i;
			break;
		}
	}

	const bool down = (speedY > 0);

	if ((swh <= 2 && m_lixxie.isSolid(0, 1) && (m_lixxie.isSolid(2, 0) || down)) ||
		(swh <= 2 && m_lixxie.isSolid(0, 2) && (m_lixxie.isSolid(2, 1) || down)))
	{
		while (m_lixxie.isSolid(0, 1))
			m_lixxie.moveUp(1);

		return { landOnFloor(), PleaseDo::ResetEncounters };
	}

	if (lowestFloor != -999 && activity == Activity::Jumper &&
		(lowestFloor > -9 || (m_lixxie.abilityToClimb && !behind(lowestFloor))))
	{
		m_lixxie.become(Activity::Ascender);
		return { BecomeCalled::Yes, PleaseDo::ResetEncounters };
	}

	if ((behindCount > 0 && speedY < 2) ||
		(wall(-12) && !wallCountT && speedY < 0))
	{
		BecomeCalled bec = BecomeCalled::No;

		if (activity != Activity::Tumbler)
		{
			m_lixxie.become(Activity::Tumbler);
			bec = BecomeCalled::Yes;
		}

		auto tumbling = dynamic_cast<BallisticFlyer*>(&m_lixxie.job);
		assert(tumbling);
		tumbling->speedY = 4;
		tumbling->speedX = speedX / 2;

		return { bec, m_lixxie.isSolid(0, 1) ? PleaseDo::ResetPosition : PleaseDo::StopMovement };
	}

	if (wallCountT)
		return onHittingWall();

	return { BecomeCalled::No, PleaseDo::Nothing };
}

AfterAssignment Jumper::onManualAssignment(Job& old)
{
	// LIXBOT: jump while climbing
	if (old.activity == Activity::Climber)
		m_lixxie.turn();
	//

	return AfterAssignment::BecomeNormally;
}

void Jumper::onBecome(const Job& old)
{
	if (m_lixxie.abilityToRun)
	{
		speedX = 8;
		speedY = -12;
		frame = 13;
	}

	else
	{
		speedX = 6;
		speedY = -8;
	}

	for (int i = -4; i > -16; --i)
	{
		if (m_lixxie.isSolid(0, i))
		{
			m_lixxie.become(Activity::Stunner);
			return;
		}
	}
}

Job::BecomeCalled Jumper::onLandingWithoutSplatting()
{
	const int soft = speedY < 12;

	m_lixxie.become(Activity::Lander);

	if (soft)
		m_lixxie.advanceFrame();

	return BecomeCalled::Yes;
}

BallisticFlyer::Collision Jumper::onHittingWall()
{
	if (m_lixxie.abilityToClimb)
	{
		m_lixxie.moveAhead(-2);
		m_lixxie.become(Activity::Climber);
		return { BecomeCalled::Yes, PleaseDo::StopMovement };
	}

	else
	{
		m_lixxie.turn();
		return { BecomeCalled::No, PleaseDo::ResetPosition };
	}
}

void Jumper::selectFrame()
{
	if (m_lixxie.isLastFrame())
		frame = (m_lixxie.abilityToRun ? 12 : frame - 1);
	else
		m_lixxie.advanceFrame();
}

void Tumbler::applyFlingXY(Lixxie& lixxie)
{
	if (!lixxie.flingNew)
		return;

	const int wantFlingX = lixxie.flingX;
	const int wantFlingY = lixxie.flingY;
	lixxie.resetFlingNew();

	// assert(lixxie.outsideWorld);

	if (wantFlingX != 0)
		lixxie.direction = wantFlingX;

	lixxie.become(Activity::Tumbler);

	if (lixxie.activity == Activity::Tumbler)
	{
		auto tumbling = dynamic_cast<Tumbler*>(&lixxie.job);
		assert(tumbling);
		tumbling->speedX = std::abs(wantFlingX);
		tumbling->speedY = wantFlingY;
		tumbling->initPixelsFallen();
		tumbling->selectFrame();
	}

	else
		assert(lixxie.activity == Activity::Stunner);
}

void Tumbler::onBecome(const Job& old)
{
	if (m_lixxie.isSolid(0, 1) && old.activity == Activity::Ascender)
		for (int dist = 1; dist <= Walker::HighestStepUp; ++dist)
		{
			if (!m_lixxie.isSolid(0, 1 - dist))
			{
				m_lixxie.moveUp(dist);
				break;
			}

			else if (!m_lixxie.isSolid(-even(dist), 1))
			{
				m_lixxie.moveAhead(-even(dist));
				break;
			}
		}

	if (m_lixxie.isSolid(0, 1))
		m_lixxie.become(Activity::Stunner);

	else if (old.activity == Activity::Jumper)
	{
		copyFrom(static_cast<const Jumper&>(old));
		frame = 3;
	}

	else
		selectFrame();
}

bool Tumbler::splatUpsideDown() const
{
	return frame >= 9;
}

Job::BecomeCalled Tumbler::onLandingWithoutSplatting()
{
	m_lixxie.become(Activity::Stunner);
	return BecomeCalled::Yes;
}

BallisticFlyer::Collision Tumbler::onHittingWall()
{
	if (wall(1) || (wall(0) && !behind(0)) ||
		(wall(-1) && !behind(-1)) ||
		(wall(-2) && !behind(-2)))
	{
		m_lixxie.turn();
		return { BecomeCalled::No, PleaseDo::ResetPositionIfMovedX };
	}

	return { BecomeCalled::No, PleaseDo::Nothing };
}

void Tumbler::selectFrame()
{
	assert(speedX >= 0);

	const int tan = speedY * 12 / std::max(2, speedX);

	struct Result
	{
		int targetFrame = 0;
		bool anim = false;
	};

	Result res = tan >  18 ? Result{ 13, true }
			   : tan >   9 ? Result{ 11, true }
			   : tan >   3 ? Result{  9, true }
			   : tan >   1 ? Result{  8 }
			   : tan >  -1 ? Result{  7 }
			   : tan >  -4 ? Result{  6 }
			   : tan > -10 ? Result{  5 }
			   : tan > -15 ? Result{  4 }
			   : tan > -30 ? Result{  3 }
			   : tan > -42 ? Result{  2 }
			   :             Result{  0, true };

	if (frame > 0)
		res.targetFrame = std::min(res.targetFrame, frame + (res.anim ? 2 : 1));

	frame = res.targetFrame
		+ ((res.targetFrame == frame && res.anim) ? 1 : 0);
}

void Tumbler::initPixelsFallen()
{
	pixelsFallen = 0;

	if (speedY < 0)
		return;

	for (int ysp = speedY <= BiggestLargeAccelSpeedY + 1 ? speedY % 2 : 0;
		ysp < speedY && accel(ysp) > 0;
		ysp += accel(ysp))
		pixelsFallen += ysp;
}
