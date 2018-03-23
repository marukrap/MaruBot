#pragma once

#include "Activity.hpp"

#include <memory>

enum class AfterAssignment
{
	BecomeNormally,
	DoNotBecome,
	WeAlreadyBecame,
};

class Lixxie;

class Job
{
public:
	using Ptr = std::shared_ptr<Job>;

public:
	explicit Job(Lixxie& lixxie);
	virtual ~Job() = default;

	Activity getActivity() const;

	int getFrame() const;
	void setFrame(int frame);

	int getSpriteOffsetX() const;
	void setSpriteOffsetX(int offsetX);

	virtual PhysicsUpdateOrder updateOrder() const; // property
	virtual bool blockable() const; // property

	virtual AfterAssignment onManualAssignment(Job& old);
	virtual void onBecome(const Job& old);
	virtual void perform();
	// TODO: virtual void returnSkillsDontCallLixxieInHere(Tribe& tribe);

	static bool healthy(Activity activity);
	static Job::Ptr createJob(Activity activity, Lixxie& lixxie);

protected:
	enum class BecomeCalled : bool
	{
		No = false,
		Yes = true,
	};

public:
	__declspec(property(get = getActivity)) Activity activity;
	__declspec(property(get = getFrame, put = setFrame)) int frame;
	__declspec(property(get = getSpriteOffsetX, put = setSpriteOffsetX)) int spriteOffsetX;

protected:
	Lixxie& m_lixxie;

private:
	Activity m_activity = Activity::Nothing;
	int m_frame = 0;
	int m_spriteOffsetX = 0;
};
