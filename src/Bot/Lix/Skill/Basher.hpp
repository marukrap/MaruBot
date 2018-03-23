#pragma once

#include "../Job.hpp"

class Basher : public Job
{
public:
	using Job::Job;

	PhysicsUpdateOrder updateOrder() const override;

	void onBecome(const Job& old) override;
	void perform() override;

private:
	bool nothingMoreToBash();
	void performSwing();
	void stopIfMovedDownTooFar();

private:
	static constexpr int HalfPixelsToFall = 9;

	int m_halfPixelsMovedDown = 0;
	bool m_steelWasHit = false;

	// LIXBOT: swingsLeft
	int m_swingsLeft = 12;
};
