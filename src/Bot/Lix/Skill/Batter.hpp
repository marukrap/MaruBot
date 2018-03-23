#pragma once

#include "../Job.hpp"

class Batter : public Job
{
public:
	using Job::Job;

public:
	PhysicsUpdateOrder updateOrder() const override;
	bool blockable() const override;

	void perform() override;

private:
	bool flingIfCloseTo(Lixxie& target, int cx, int cy) const;

private:
	static constexpr int FlingAfterFrame = 2;
	static constexpr int RectHalfXl = 12;
	static constexpr int RectHalfYl = 12;
	static constexpr int ExtraXRangeForBlockers = 4;
	static constexpr int FlingSpeedX = 10;
	static constexpr int FlingSpeedY = -12;
};
