#pragma once

#include "../Job.hpp"

class RemovedLix : public Job
{
public:
	using Job::Job;

	bool blockable() const override;

	void onBecome(const Job& old) override;
};

class Leaver : public Job
{
public:
	bool blockable() const override;

	void onBecome(const Job& old) final;
	void perform() override;

protected:
	using Job::Job;

	virtual void onBecomeLeaver(const Job& old);

	void advanceFrameAndLeave();
};

class Splatter : public Leaver
{
public:
	using Leaver::Leaver;

private:
	void onBecomeLeaver(const Job& old) override;
};

class Burner : public Leaver
{
public:
	using Leaver::Leaver;

private:
	void onBecomeLeaver(const Job& old) override;
};

class Drowner : public Leaver
{
public:
	using Leaver::Leaver;

private:
	void onBecomeLeaver(const Job& old) override;
};
