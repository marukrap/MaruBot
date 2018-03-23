#pragma once

#include "../Job.hpp"

class Floater : public Job
{
public:
	using Job::Job;

	AfterAssignment onManualAssignment(Job& old) override;
	void onBecome(const Job& old) override;
	void perform() override;

private:
	void adjustFrame();
	void adjustSpeed();
	void move();

private:
	int m_speedX = 0;
	int m_speedY = 0;
	bool m_accelerateY = false;
};
