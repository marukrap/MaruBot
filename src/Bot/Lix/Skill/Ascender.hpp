#pragma once

#include "../Job.hpp"

class Ascender : public Job
{
public:
	using Job::Job;

	bool blockable() const override;

	void onBecome(const Job& old) override;
	void perform() override;
};
