#pragma once

#include "Leaver.hpp"

class Ploder : public Leaver
{
public:
	using Leaver::Leaver;

	PhysicsUpdateOrder updateOrder() const override;
	bool blockable() const override;

	AfterAssignment onManualAssignment(Job& old) final;
	void perform() final;

protected:
	void onBecomeLeaver(const Job& old) final;

	virtual void flingOtherLix();
	virtual void makeEffect() = 0;

private:
	void changeTerrain();
};

class Imploder : public Ploder
{
public:
	using Ploder::Ploder;

protected:
	void makeEffect() override;
};

class Exploder : public Ploder
{
public:
	using Ploder::Ploder;

protected:
	void makeEffect() override;
	void flingOtherLix() override;

private:
	void flingOtherLix(Lixxie& target, bool targetTribeIsOurTribe);
};
