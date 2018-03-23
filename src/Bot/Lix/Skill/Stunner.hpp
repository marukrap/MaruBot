#pragma once

#include "../Job.hpp"

class Stunner : public Job
{
public:
	using Job::Job;

	void onBecome(const Job& old) override;
	void perform() override;

private:
	int m_stayedInFrame8 = 0;
};
