#include "Faller.hpp"
#include "../Lixxie.hpp"

#include <cassert>

void Faller::becomeAndFallPixels(Lixxie& lixxie, int fallY)
{
	lixxie.moveDown(fallY);
	lixxie.become(Activity::Faller);

	auto faller = dynamic_cast<Faller*>(&lixxie.job);
	assert(faller);
	faller->pixelsFallen = fallY;
}

void Faller::perform()
{
	int ySpeedThisFrame = 0;

	for (; ySpeedThisFrame <= ySpeed; ++ySpeedThisFrame)
	{
		if (m_lixxie.isSolid(0, ySpeedThisFrame + 2))
		{
			m_lixxie.moveDown(ySpeedThisFrame);
			pixelsFallen += ySpeedThisFrame;

			auto hasFallenVeryLittle = [this] ()
			{
				return pixelsFallen <= 9 && frame < 1
					|| pixelsFallen == 0
					|| frame < 2;
			};

			if (pixelsFallen > PixelsSafeToFall && !m_lixxie.abilityToFloat)
				m_lixxie.become(Activity::Splatter);
			else if (hasFallenVeryLittle())
				m_lixxie.become(Activity::Walker);
			else
				m_lixxie.become(Activity::Lander);

			return;
		}
	}

	ySpeedThisFrame = std::min(ySpeedThisFrame, ySpeed);

	m_lixxie.moveDown(ySpeedThisFrame);
	pixelsFallen += ySpeedThisFrame;

	if (ySpeed < YSpeedTerminal)
		++ySpeed;

	if (m_lixxie.isLastFrame())
		frame -= 1;
	else
		m_lixxie.advanceFrame();

	if (m_lixxie.abilityToFloat && pixelsFallen >= PixelsFallenToBecomeFloater)
		m_lixxie.become(Activity::Floater);
}
