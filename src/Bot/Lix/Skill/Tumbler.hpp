#pragma once

#include "Faller.hpp"

enum class PleaseDo
{
	Nothing,
	StopMovement,
	ResetEncounters,
	ResetPosition,
	ResetPositionIfMovedX,
};

class BallisticFlyer : public Job
{
public:
	using Job::Job;

	void perform() final;

protected:
	struct Collision
	{
		BecomeCalled becomeCalled;
		PleaseDo pleaseDo;
	};

	void copyFrom(const BallisticFlyer& rhs);

	bool wall(int y) const;
	bool behind(int y) const;

	int accel(int ysp);

	virtual bool splatUpsideDown() const;

	virtual BecomeCalled onLandingWithoutSplatting() = 0;
	virtual Collision onHittingWall() = 0;
	virtual void selectFrame() = 0;

private:
	BecomeCalled landOnFloor();
	BecomeCalled moveSeveralTimes();
	Collision collision();

public:
	static constexpr int SpeedYToFloat = 15;
	static constexpr int PixelsSafeToFall = Faller::PixelsSafeToFall;

	int speedX = 0;
	int speedY = 0;
	int pixelsFallen = 0;

protected:
	static constexpr int BiggestLargeAccelSpeedY = 12;
};

class Jumper : public BallisticFlyer
{
public:
	using BallisticFlyer::BallisticFlyer;

	AfterAssignment onManualAssignment(Job& old) override;
	void onBecome(const Job& old) override;

protected:
	BecomeCalled onLandingWithoutSplatting() override;
	Collision onHittingWall() override;
	void selectFrame() override;
};

class Tumbler : public BallisticFlyer
{
public:
	using BallisticFlyer::BallisticFlyer;

	static void applyFlingXY(Lixxie& lixxie);

	void onBecome(const Job& old) override;

protected:
	bool splatUpsideDown() const override;

	BecomeCalled onLandingWithoutSplatting() override;
	Collision onHittingWall() override;
	void selectFrame() override;

private:
	void initPixelsFallen();
};
