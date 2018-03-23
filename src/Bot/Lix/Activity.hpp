#pragma once

namespace
{
	constexpr int SkillInfinity   = -1;
	constexpr int SkillNumberMax  = 999;

	constexpr int BuilderBrickXl  = 12;
	constexpr int PlatformLongXl  = 8;
	constexpr int PlatformShortXl = 6;
	constexpr int BrickYl         = 2;
}

enum class PhysicsUpdateOrder
{
	Peaceful,
	Adder,
	Remover,
	Blocker,
	Flinger,
};

enum class Activity
{
	Nothing,
	Faller,
	Tumbler,
	Stunner,
	Lander,
	Splatter,
	Burner,
	Drowner,
	Exiter,
	Walker,
	Runner,

	Climber,
	Ascender,
	Floater,
	Imploder,
	Exploder,
	Blocker,
	Builder,
	Shrugger,
	Platformer,
	Shrugger2,
	Basher,
	Miner,
	Digger,

	Jumper,
	Batter,
	Cuber,

	Max
};
