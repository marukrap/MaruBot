#pragma once

#include "../Job.hpp"

class Climber : public Job
{
public:
	using Job::Job;

	bool blockable() const override;

	AfterAssignment onManualAssignment(Job& old) override;
	void onBecome(const Job& old) override;
	void perform() override;

private:
	void stickSpriteToWall();
	void maybeBecomeAscenderImmediatelyOnBecome();
	// int upwardsMovementThisFrame() const;
	bool ascendHoistableLedge();
	void stopAndBecomeWalker();

private:
	static constexpr int CeilingY = -16;
};
