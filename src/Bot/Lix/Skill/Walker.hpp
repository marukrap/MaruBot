#pragma once

#include "../Job.hpp"

class Walker : public Job
{
public:
	using Job::Job;

	AfterAssignment onManualAssignment(Job& old) override;
	void onBecome(const Job& old) override;
	void perform() override;

protected:
	void performWalkingOrRunning();
	void setFrameAfterShortFallTo(const Job& old, int targetFrame);

private:
	bool handleWallOrPitHere() const;

public:
	static constexpr int HighestStepUp = 12;
};

class Runner : public Walker
{
public:
	using Walker::Walker;

	AfterAssignment onManualAssignment(Job& old) override;
	void onBecome(const Job& old) override;
	void perform() override;
};
