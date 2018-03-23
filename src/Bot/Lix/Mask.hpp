#pragma once

#include "TerrainChange.hpp"

#include <vector>

class Mask
{
public:
	enum CharOK : char
	{
		Solid       = 'X',
		SolidIgnore = 'N',
		SolidOffset = '#',
		Air         = '.',
		AirOffset   = 'o',
	};

public:
	Mask() = default;
	explicit Mask(const std::vector<std::string>& strs);
	Mask(int radius, int offsetFromCenterY);

	Mask mirrored() const;

	int getOffsetX() const;
	int getOffsetY() const;

	int getWidth() const;
	int getHeight() const;

	bool solid(int x, int y) const;
	bool ignoreSteel(int x, int y) const;

	static void initialize();
	static const Mask& get(TerrainDeletion::Type type);

public:
	static constexpr int ExplodeMaskOffsetY = -6;

private:
	int m_offsetX = 0;
	int m_offsetY = 0;
	int m_width = 0;
	int m_height = 0;
	std::vector<bool> m_solid;
	std::vector<bool> m_ignoreSteel;

public:
	__declspec(property(get = getOffsetX)) int offsetX;
	__declspec(property(get = getOffsetY)) int offsetY;
	__declspec(property(get = getWidth)) int width;
	__declspec(property(get = getHeight)) int height;
};
