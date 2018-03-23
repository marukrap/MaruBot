#pragma once

#include "../Job.hpp"

class Faller : public Job
{
public:
	using Job::Job;

	static void becomeAndFallPixels(Lixxie& lixxie, int fallY);

	void perform() override;

public:
	static constexpr int YSpeedTerminal = 8;
	static constexpr int PixelsSafeToFall = 126;
	static constexpr int PixelsFallenToBecomeFloater = 60;

	int ySpeed = 4;
	int pixelsFallen = 0;
};
