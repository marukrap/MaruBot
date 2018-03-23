#include "Mask.hpp"
#include "Help.hpp"

#include <cassert>

namespace
{
	std::vector<Mask> Masks;
}

Mask::Mask(const std::vector<std::string>& strs)
	: m_width(strs[0].size())
	, m_height(strs.size())
	, m_solid(m_width * m_height, false)
{
	assert(strs.size() > 0);
	assert(strs[0].size() > 0);

	for (std::size_t i = 1; i < strs.size(); ++i)
		assert(strs[i].size() == strs[0].size());

	bool offsetSet = false;

	for (int y = 0; y < m_height; ++y)
		for (int x = 0; x < m_width; ++x)
		{
			const CharOK cc = static_cast<CharOK>(strs[y][x]);
			assert(std::string("XN#.o").find(cc) != std::string::npos);

			m_solid[x + y * m_width] = (cc == CharOK::Solid
				|| cc == CharOK::SolidIgnore
				|| cc == CharOK::SolidOffset);

			if (cc == CharOK::SolidIgnore)
			{
				if (m_ignoreSteel.empty())
					m_ignoreSteel.resize(m_width * m_height, false);

				m_ignoreSteel[x + y * m_width] = true;
			}

			else if (cc == CharOK::SolidOffset || cc == CharOK::AirOffset)
			{
				assert(!offsetSet);
				m_offsetX = x;
				m_offsetY = y;
				offsetSet = true;
			}
		}
}

Mask::Mask(int radius, int offsetFromCenterY)
{
	m_width = m_height = 2 * radius + 2;
	m_solid.resize(m_width * m_height);

	const int midX = m_width / 2 - 1;
	const int midY = m_height / 2 - 1;

	m_offsetX = midX;
	m_offsetY = midY - offsetFromCenterY;

	for (int y = 0; y < m_height; ++y)
		for (int x = 0; x < m_width; ++x)
		{
			const int centralX = midX + (x > midX ? 1 : 0);
			const int centralY = midY + (y > midY ? 1 : 0);

			const bool solid = std::pow(radius + 0.5f, 2) >=
				std::pow(x - centralX, 2) + std::pow(y - centralY, 2);

			m_solid[x + y * m_width] = solid;
		}
}

Mask Mask::mirrored() const
{
	Mask mask;
	mask.m_width = m_width;
	mask.m_height = m_height;
	mask.m_solid.resize(m_solid.size(), false);
	mask.m_ignoreSteel.resize(m_ignoreSteel.size(), false);

	for (int y = 0; y < m_height; ++y)
		for (int x = 0; x < m_width; ++x)
		{
			const int mirrorX = m_width - 1 - x;

			mask.m_solid[x + y * m_width] = m_solid[mirrorX + y * m_width];

			if (!mask.m_ignoreSteel.empty())
				mask.m_ignoreSteel[x + y * m_width] = m_ignoreSteel[mirrorX + y * m_width];
		}

	mask.m_offsetX = even(m_width - 1 - m_offsetX);
	mask.m_offsetY = m_offsetY;

	return mask;
}

int Mask::getOffsetX() const
{
	return m_offsetX;
}

int Mask::getOffsetY() const
{
	return m_offsetY;
}

int Mask::getWidth() const
{
	return m_width;
}

int Mask::getHeight() const
{
	return m_height;
}

bool Mask::solid(int x, int y) const
{
	return m_solid[x + y * m_width];
}

bool Mask::ignoreSteel(int x, int y) const
{
	return m_ignoreSteel.empty() || m_ignoreSteel[x + y * m_width];
}

void Mask::initialize()
{
	using Type = TerrainDeletion::Type;

	Masks.resize(Type::Max);

	Masks[Type::BashRight] = Mask(
		{
			"NNNNNNNNNNNN....",
			"NNNNNNNNNNNNNN..",
			"XXXXXXXXXXXXXXX.",
			"XXXXXXXXXXXXXXXX", // 1 // repeat(12)
			"XXXXXXXXXXXXXXXX", // 2
			"XXXXXXXXXXXXXXXX", // 3
			"XXXXXXXXXXXXXXXX", // 4
			"XXXXXXXXXXXXXXXX", // 5
			"XXXXXXXXXXXXXXXX",	// 6
			"XXXXXXXXXXXXXXXX",	// 7
			"XXXXXXXXXXXXXXXX",	// 8
			"XXXXXXXXXXXXXXXX",	// 9
			"XXXXXXXXXXXXXXXX",	// 10
			"XXXXXXXXXXXXXXXX",	// 11
			"XXXXXXXXXXXXXXXX",	// 12
			"XXXXXXXXXXXXXXX.",
			"#XXXXXXXXXXXXX..",
			"XXXXXXXXXXXX....",
		});

	Masks[Type::BashLeft] = Masks[Type::BashRight].mirrored();

	Masks[Type::BashNoRelicsRight] = Mask(
		{
			"NNNNNNNNNNNNNNNN", // 1 // repeat(2)
			"NNNNNNNNNNNNNNNN", // 2
			"XXXXXXXXXXXXXXXX", // 1 // repeat(14)
			"XXXXXXXXXXXXXXXX", // 2
			"XXXXXXXXXXXXXXXX", // 3
			"XXXXXXXXXXXXXXXX", // 4
			"XXXXXXXXXXXXXXXX", // 5
			"XXXXXXXXXXXXXXXX", // 6
			"XXXXXXXXXXXXXXXX", // 7
			"XXXXXXXXXXXXXXXX", // 8
			"XXXXXXXXXXXXXXXX", // 9
			"XXXXXXXXXXXXXXXX", // 10
			"XXXXXXXXXXXXXXXX", // 11
			"XXXXXXXXXXXXXXXX", // 12
			"XXXXXXXXXXXXXXXX", // 13
			"XXXXXXXXXXXXXXXX", // 14
			"#XXXXXXXXXXXXXXX",
			"XXXXXXXXXXXXXXXX",
		});

	Masks[Type::BashNoRelicsLeft] = Masks[Type::BashNoRelicsRight].mirrored();

	Masks[Type::MineRight] = Mask(
		{
			"...NNNNN..........",
			".NNNNNNNNN........",
			"NNXXXXXXNNNN......",
			"NNXXXXXXXXXXNN....",
			"NNXXXXXXXXXXXXN...",
			"NNXXXXXXXXXXXXXX..",
			"NNXXXXXXXXXXXXXXX.",
			"NNXXXXXXXXXXXXXXX.",
			"NNXXXXXXXXXXXXXXXX", // 1 // repeat(12)
			"NNXXXXXXXXXXXXXXXX", // 2
			"NNXXXXXXXXXXXXXXXX", // 3
			"NNXXXXXXXXXXXXXXXX", // 4
			"NNXXXXXXXXXXXXXXXX", // 5
			"NNXXXXXXXXXXXXXXXX", // 6
			"NNXXXXXXXXXXXXXXXX", // 7
			"NNXXXXXXXXXXXXXXXX", // 8
			"NNXXXXXXXXXXXXXXXX", // 9
			"NNXXXXXXXXXXXXXXXX", // 10
			"NNXXXXXXXXXXXXXXXX", // 11
			"NNXXXXXXXXXXXXXXXX", // 12
			"#XXXXXXXXXXXXXXXX.",
			"XXXXXXXXXXXXXXXXX.",
			"..XXXXXXXXXXXXXX..",
			"....XXXXXXXXXXX...",
			"......XXXXXXXX....",
			"........XXXX......",
		});

	Masks[Type::MineLeft] = Masks[Type::MineRight].mirrored();

	Masks[Type::Explode] = Mask(22, ExplodeMaskOffsetY);

	Masks[Type::Implode] = Mask(
		{
			"..............XXXXXX..............",
			"...........XXXXXXXXXXXX...........",
			".........XXXXXXXXXXXXXXXX.........",
			"........XXXXXXXXXXXXXXXXXX........",
			".......XXXXXXXXXXXXXXXXXXXX.......",
			"......XXXXXXXXXXXXXXXXXXXXXX......",
			".....XXXXXXXXXXXXXXXXXXXXXXXX.....", // 1 // repeat(2)
			".....XXXXXXXXXXXXXXXXXXXXXXXX.....", // 2
			"....XXXXXXXXXXXXXXXXXXXXXXXXXX....", // 1 // repeat(2)
			"....XXXXXXXXXXXXXXXXXXXXXXXXXX....", // 2
			"...XXXXXXXXXXXXXXXXXXXXXXXXXXXX...", // 1 // repeat(2)
			"...XXXXXXXXXXXXXXXXXXXXXXXXXXXX...", // 2
			"..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..", // 1 // repeat(3)
			"..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..", // 2
			"..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..", // 3
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 1 // repeat(5)
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 2
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 3
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 4
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 5
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 1 // repeat(6)
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 2
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 3
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 4
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 5
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 6
			"XXXXXXXXXXXXXXXX#XXXXXXXXXXXXXXXXX", // 1 // repeat(1)
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 1 // repeat(5)
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 2
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 3
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 4
			"XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", // 5
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 1 // repeat(3)
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 2
			".XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.", // 3
			"..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..", // 1 // repeat(2)
			"..XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..", // 2
			"...XXXXXXXXXXXXXXXXXXXXXXXXXXXX...",
			"....XXXXXXXXXXXXXXXXXXXXXXXXXX....",
			".....XXXXXXXXXXXXXXXXXXXXXXXX.....",
			"......XXXXXXXXXXXXXXXXXXXXXX......",
			".......XXXXXXXXXXXXXXXXXXXX.......",
			".........XXXXXXXXXXXXXXXX.........",
			"............XXXXXXXXXX............",
		});
}

const Mask& Mask::get(TerrainDeletion::Type type)
{
	return Masks[type];
}
