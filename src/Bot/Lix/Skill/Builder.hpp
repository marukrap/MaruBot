#pragma once

#include "../Job.hpp"

class BrickCounter : public Job
{
public:
	using Job::Job;

	PhysicsUpdateOrder updateOrder() const override;

	AfterAssignment onManualAssignment(Job& old) override;
	void onBecome(const Job& old) override;
	void perform() final;
	// void returnSkillsDontCallLixxieInHere(Tribe& tribe) override;

protected:
	virtual int startFrame(const Job& old) const = 0;
	virtual void onPerform() = 0;

	void buildBrick();
	void buildBrickForFree();
	virtual void onBuildingBrick() = 0;

	BecomeCalled maybeBecomeShrugger(Activity shruggingAc);

private:
	static constexpr int BricksAtStart = 12;

	int m_skillsQueued = 0;
	int m_bricksLeft = 0;
};

class Builder : public BrickCounter
{
public:
	using BrickCounter::BrickCounter;

private:
	int startFrame(const Job& old) const override;
	void onPerform()override;

	void onBuildingBrick()override;

	BecomeCalled bumpAgainstTerrain();

private:
	bool m_fullyInsideTerrain = false;
};

class Platformer : public BrickCounter
{
public:
	using BrickCounter::BrickCounter;

	static void abortAndStandUp(Lixxie& lixxie);

private:
	int startFrame(const Job& old) const override;
	void onPerform() override;

	void onBuildingBrick() override;

	bool platformerTreatsAsSolid(int x, int y);
	void planNextBrickFirstCycle();
	void planNextBrickSubsequentCycles();
	void moveUpAndCollide();
	void moveAheadAndCollide();

private:
	static constexpr int StandingUpFrame = 9;
};

class Shrugger : public Job
{
public:
	using Job::Job;

	void perform() override;
};
