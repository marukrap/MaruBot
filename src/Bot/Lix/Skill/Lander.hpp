#pragma once

#include "../Job.hpp"

class Lander : public Job
{
public:
	using Job::Job;

	void onBecome(const Job& old) override;
	void perform() override;
};
