#pragma once

#include "Leaver.hpp"

class Cuber : public Leaver
{
public:
	using Leaver::Leaver;

	PhysicsUpdateOrder updateOrder() const override;

	void perform() override;

private:
	void onBecomeLeaver(const Job& old) override;

public:
	static constexpr int CubeSize = 16;
};
