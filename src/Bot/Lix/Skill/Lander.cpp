#include "Lander.hpp"
#include "Faller.hpp"
#include "../Lixxie.hpp"

#include <cassert>

void Lander::onBecome(const Job& old)
{
	if (old.activity == Activity::Faller)
	{
		auto faller = dynamic_cast<const Faller*>(&old);
		assert(faller);

		if (faller->frame < 3)
			frame = 1;
	}
}

void Lander::perform()
{
	if (m_lixxie.isLastFrame())
		m_lixxie.become(Activity::Walker);
	else
		m_lixxie.advanceFrame();
}
