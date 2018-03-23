#pragma once

// #include "Lixxie.hpp"

using Phyu = int; // physics update count

struct TerrainAddition
{
	enum Type
	{
		Build,
		PlatformLong,
		PlatformShort,
		Cube,
		Max,
	};

	Phyu update = 0;
	Type type;
	int x = 0;
	int y = 0;

	int style; // Lixxie::Style style;
	int cubeYl = 0;
};

struct TerrainDeletion
{
	enum Type
	{
		Implode,
		Explode,
		BashLeft,
		BashRight,
		BashNoRelicsLeft,
		BashNoRelicsRight,
		MineLeft,
		MineRight,
		Dig,
		Max,
	};

	Phyu update = 0;
	Type type;
	int x = 0;
	int y = 0;

	int digYl = 0;
};

struct FlaggedAddition
{
	TerrainAddition tc; // terrainChange;

	bool drawPerPixelDueToExistingTerrain = false;
	bool needsColoring[16][16] = { false };
};

struct FlaggedDeletion
{
	TerrainDeletion tc; // terrainChange;

	bool drawPerPixelDueToSteel = false;
};
