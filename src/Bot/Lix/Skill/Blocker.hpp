#pragma once

#include "../Job.hpp"

class Blocker : public Job
{
public:
	using Job::Job;

	PhysicsUpdateOrder updateOrder() const override;
	bool blockable() const override;

	AfterAssignment onManualAssignment(Job& old) override;
	void perform() override;

private:
	void blockOtherLix();
	void blockSingleLix(Lixxie& target);

public:
	static constexpr int ForceFieldXlEachSide = 14;
	static constexpr int ForceFieldYlAbove = 16;
	static constexpr int ForceFieldYlBelow = 8;
};
