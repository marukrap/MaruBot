#pragma once

#include "../Job.hpp"

class Digger : public Job
{
public:
	using Job::Job;

	PhysicsUpdateOrder updateOrder() const override;

	void perform() override;

private:
	bool hitEnoughSteel();
	bool shouldWeFallHere() const;
	void removeRowsYInterval(int y, int yl);

public:
	static constexpr int TunnelWidth = 18;

private:
	bool m_upstrokeDone = false;
};
