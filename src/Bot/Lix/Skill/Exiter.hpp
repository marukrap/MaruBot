#pragma once

#include "Leaver.hpp"

class Exiter : public Leaver
{
public:
	using Leaver::Leaver;

	void perform() override;

private:
	// void scoreForTribe(Tribe& tribe);
	// void determineSidewaysMotion(const Goal& goal);
	// void playSound(const Goal& goal);

private:
	int m_xOffsetFromGoal = 0;
};
