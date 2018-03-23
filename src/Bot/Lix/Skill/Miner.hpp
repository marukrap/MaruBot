#pragma once

#include "../Job.hpp"

class Miner : public Job
{
public:
	using Job::Job;

	PhysicsUpdateOrder updateOrder() const override;

	void perform()override;

private:
	void antiShock(int resiliance);
	int antiShockMoveDown(int maxDepth);
	void removeEarth();
	void checkFutureGround();
	void normalMovement();
	void becomeFallerWithAlreadyFallenPixels(int downThisFrame);

private:
	static constexpr int FutureLength = 4;
	static constexpr int MaxGapDepth = 3;

	int m_movedDownSinceSwing = 0;
	bool m_futureGroundIsSolid[FutureLength] = { false };
};
