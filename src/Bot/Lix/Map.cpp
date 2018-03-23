#include "Map.hpp"
#include "Mask.hpp"
#include "Lixxie.hpp" // REMOVE:
#include "Help.hpp"

// TODO: move to PhysicsDrawer
#include "Skill/Cuber.hpp"
#include "Skill/Digger.hpp"

#include <cassert>

Map::Map(int width, int height, bool torusX, bool torusY, bool smoothlyScalable)
	: m_flags(width * height, Flags::None)
	, m_width(width)
	, m_height(height)
	, m_torusX(torusX)
	, m_torusY(torusY)
	, m_smoothlyScalable(smoothlyScalable)
{
	m_textureImage.create(m_width, m_height, sf::Color::Transparent);
	m_texture.loadFromImage(m_textureImage);
	// m_texture.create(m_width, m_height);

	// TODO: move to PhysicsDrawer
	Mask::initialize();

	sf::Image recol;
	recol.loadFromFile("Lix/Images/lixrecol.I.png");

	using Type = TerrainDeletion::Type;

	assert(BuilderBrickXl >= PlatformLongXl);
	assert(BuilderBrickXl >= PlatformShortXl);
	m_image.create(0x100, 0x80);
	m_subImages.resize(TerrainDeletion::Max);

	auto drawRectangle = [this] (int x1, int y1, int x2, int y2, const sf::Color& color)
	{
		for (int y = y1; y < y2; ++y)
			for (int x = x1; x < x2; ++x)
				m_image.setPixel(x, y, color);
	};

	auto drawBrick = [&] (int x, int y, int width, const sf::Color& light, const sf::Color& medium, const sf::Color& dark)
	{
		const int height = BrickYl;

		drawRectangle(x,             y,              x + width - 1, y + 1,      light);
		drawRectangle(x + 1,         y + height - 1, x + width,     y + height, dark);
		drawRectangle(x,             y + height - 1, x + 1,         y + height, medium);
		drawRectangle(x + width - 1, y,              x + width,     y + 1,      medium);
	};

	auto drawCube = [&] (int x, int y, const sf::Color& light, const sf::Color& medium, const sf::Color& dark)
	{
		constexpr int length = Cuber::CubeSize;
		assert(length >= 10);

		drawRectangle(x, y, x + length, y + length, medium);

		auto symmetrical = [&] (int ax, int ay, int axl, int ayl, const sf::Color& color)
		{
			drawRectangle(x + ax, y + ay, x + ax + axl, y + ay + ayl, color);
			drawRectangle(x + ay, y + ax, x + ay + ayl, y + ax + axl, color);
		};

		symmetrical(0, 0, length - 1, 1, light);
		symmetrical(0, 1, length - 2, 1, light);

		symmetrical(2, length - 2, length - 2, 1, dark);
		symmetrical(1, length - 1, length - 1, 1, dark);

		constexpr int o  = 4;
		constexpr int ol = length - 2 * o - 1;

		symmetrical(o,     o,              ol, 1, dark);
		symmetrical(o + 1, length - o - 1, ol, 1, light);
	};

	const int recolWidth = recol.getSize().x;
	const int recolHeight = recol.getSize().y;

	// PhysicsDrawer
	constexpr int buiY     = 0;
	constexpr int cubeY    = 3 * BrickYl;
	constexpr int remY     = cubeY + Cuber::CubeSize;
	constexpr int remYl    = 32;
	constexpr int ploY     = remY + remYl;
	const     int ploYl    = Mask::get(Type::Implode).width;
	constexpr int bashX    = Digger::TunnelWidth + 1;
	const     int bashXl   = Mask::get(Type::BashRight).width + 1;
	const     int mineX    = bashX + 4 * bashXl;
	const     int mineXl   = Mask::get(Type::MineRight).width + 1;
	constexpr int implodeX = 0;
	const     int explodeX = Mask::get(Type::Implode).width + 1;
	//

	for (int i = 0; i < Lixxie::Style::Max && i < recolHeight + 1; ++i)
	{
		sf::Color light  = recol.getPixel(recolWidth - 3, i + 1);
		sf::Color medium = recol.getPixel(recolWidth - 2, i + 1);
		sf::Color dark   = recol.getPixel(recolWidth - 1, i + 1);

		drawBrick(i * BuilderBrickXl, 0, BuilderBrickXl, light, medium, dark);
		drawBrick(i * PlatformLongXl, BrickYl, PlatformLongXl, light, medium, dark);
		drawBrick(i * PlatformShortXl, 2 * BrickYl, PlatformShortXl, light, medium, dark);
		drawCube(i * Cuber::CubeSize, cubeY, light, medium, dark);
	}

	// digger swing
	drawRectangle(0, remY, Digger::TunnelWidth, remY + remYl, sf::Color::White);
	m_subImages[Type::Dig].create(Digger::TunnelWidth, remYl);
	m_subImages[Type::Dig].copy(m_image, 0, 0, { 0, remY, Digger::TunnelWidth, remY + remYl });
	// TODO: ^ unused image?

	// basher and miner swings
	auto drawSwing = [this] (int startX, int startY, Type type)
	{
		const Mask& mask = Mask::get(type);

		for (int y = 0; y < mask.height; ++y)
			for (int x = 0; x < mask.width; ++x)
			{
				if (mask.solid(x, y))
					m_image.setPixel(startX + x, startY + y, sf::Color::White); // transparent?
			}

		m_subImages[type].create(mask.width, mask.height);
		m_subImages[type].copy(m_image, 0, 0, { startX, startY, mask.width, mask.height });
	};

	drawSwing(bashX,              remY, Type::BashRight);
	drawSwing(bashX + bashXl,     remY, Type::BashLeft);
	drawSwing(bashX + 2 * bashXl, remY, Type::BashNoRelicsRight);
	drawSwing(bashX + 3 * bashXl, remY, Type::BashNoRelicsLeft);
	drawSwing(mineX,              remY, Type::MineRight);
	drawSwing(mineX + mineXl,     remY, Type::MineLeft);

	// imploder, exploder
	drawSwing(implodeX, ploY, Type::Implode);
	drawSwing(explodeX, ploY, Type::Explode);
}

const sf::Texture& Map::getTexture() const
{
	return m_texture;
}

void Map::draw(Point point, sf::IntRect rect)
{
	// HACK: clamp rect
	if (point.x < 0)
	{
		rect.left -= point.x;
		rect.width += point.x;
		point.x = 0;
	}

	if (point.y < 0)
	{
		rect.top -= point.y;
		rect.height += point.y;
		point.y = 0;
	}

	if (point.x + rect.width > m_width)
		rect.width = std::max(0, m_width - point.x);

	if (point.y + rect.height > m_height)
		rect.height = std::max(0, m_height - point.y);
	//

	sf::Image image;
	image.create(rect.width, rect.height);
	image.copy(m_image, 0, 0, { rect.left, rect.top, rect.width, rect.height });

	m_textureImage.copy(image, point.x, point.y);
	m_texture.update(image, point.x, point.y);
}

void Map::draw(Point point, TerrainDeletion::Type type, int digYl)
{
	const sf::Image& subImage = m_subImages[type];
	sf::IntRect rect(0, 0, subImage.getSize().x, subImage.getSize().y);

	if (type == TerrainDeletion::Dig)
		rect.height = digYl;

	// HACK: clamp rect
	if (point.x < 0)
	{
		rect.left -= point.x;
		rect.width += point.x;
		point.x = 0;
	}

	if (point.y < 0)
	{
		rect.top -= point.y;
		rect.height += point.y;
		point.y = 0;
	}

	if (point.x + rect.width > m_width)
		rect.width = std::max(0, m_width - point.x);

	if (point.y + rect.height > m_height)
		rect.height = std::max(0, m_height - point.y);
	//

	sf::Image image;
	image.create(rect.width, rect.height);
	image.copy(m_textureImage, 0, 0, { point.x, point.y, rect.width, rect.height });

	for (int y = rect.top; y < rect.top + rect.height; ++y)
		for (int x = rect.left; x < rect.left + rect.width; ++x)
		{
			if (subImage.getPixel(x, y) == sf::Color::White)
				image.setPixel(x - rect.left, y - rect.top, sf::Color::Transparent);
		}

	m_textureImage.copy(image, point.x, point.y);
	m_texture.update(image, point.x, point.y);
}

int Map::getWidth() const
{
	return m_width;
}

int Map::getHeight() const
{
	return m_height;
}

int Map::getTorusX() const
{
	return m_torusX;
}

int Map::getTorusY() const
{
	return m_torusY;
}

Map::Flags Map::get(const Point& point) const
{
	// LIXBOT: out of bounds
	if (point.x < 0 || point.x >= m_width || point.y < 0 || point.y >= m_height)
		return Flags::Terrain | Flags::Steel;

	// return at(clamp(p));
	return at(point);
}

bool Map::get(const Point& point, Flags flag) const
{
	return (get(point) & flag) != 0;
}

bool Map::getSolid(const Point& point) const
{
	return get(point, Flags::Terrain);
}

bool Map::getSolidEven(const Point& point) const
{
	assert(m_width % 2 == 0);

	const Point p1(point.x & ~1, point.y);
	const Point p2(point.x | 1, point.y);

	return ((get(p1) | get(p2)) & Flags::Terrain) != 0;
}

bool Map::getSteel(const Point& point) const
{
	const bool steel = get(point, Flags::Steel);

	assert(!steel || getSolid(point));

	return steel;
}

bool Map::getSteelUnlessMaskIgnores(const Point& point, const Mask& mask) const
{
	const Point offset(mask.offsetX, mask.offsetY);

	for (int y = 0; y < mask.height; ++y)
		for (int x = 0; x < mask.width; ++x)
		{
			const Point p(x, y);

			if (mask.solid(x, y) && !mask.ignoreSteel(x, y) && getSteel(point + p - offset))
				return true;
		}

	return false;
}

void Map::add(const Point& point, Flags flag)
{
	if (inside(point))
	{
		// addAt(wrap(point), flag);
		const Point wrapped = wrap(point);
		m_flags[wrapped.x + wrapped.y * m_width] |= flag;
	}
}

bool Map::setAirCountSteel(const Point & point)
{
	const Point wrapped = wrap(point);

	if (!inside(point))
		return getSteel(point);

	else if (at(wrapped) & Flags::Steel)
	{
		assert(at(wrapped) & Flags::Terrain);
		return true;
	}

	else
	{
		m_flags[wrapped.x + wrapped.y * m_width] &= ~Flags::Terrain;
		return false;
	}
}

int Map::setAirCountSteelEvenWhereMaskIgnores(const Point& topLeft, const Mask& mask)
{
	// assert(mask.solid);
	assert(mask.width > 0 && mask.height > 0);

	int steelHit = 0;

	for (int y = 0; y < mask.height; ++y)
	{
		Point p(0, topLeft.y + y);

		for (int x = 0; x < mask.width; ++x)
		{
			p.x = topLeft.x + x;

			if (mask.solid(x, y))
				steelHit += setAirCountSteel(p);
		}
	}
	return steelHit;
}

int Map::distanceX(int x1, int x2) const
{
	if (!m_torusX)
		return x2 - x1;

	const int possible[3] = { x2 - x1, x2 - x1 - m_width, x2 - x1 + m_width };

	int minPos = 0;

	for (int i = 1; i < 3; ++i)
	{
		if (std::abs(possible[i]) < std::abs(possible[minPos]))
			minPos = i;
	}

	return possible[minPos];
}

int Map::distanceY(int y1, int y2) const
{
	if (!m_torusX)
		return y2 - y1;

	const int possible[3] = { y2 - y1, y2 - y1 - m_width, y2 - y1 + m_width };

	int minPos = 0;

	for (int i = 1; i < 3; ++i)
	{
		if (std::abs(possible[i]) < std::abs(possible[minPos]))
			minPos = i;
	}

	return possible[minPos];
}

Map::Point Map::wrap(const Point& point) const
{
	const int x = m_torusX ? positiveMod(point.x, m_width) : point.x;
	const int y = m_torusY ? positiveMod(point.y, m_height) : point.y;

	return { x, y };
}

bool Map::inside(const Point& point) const
{
	if (!m_torusX && (point.x < 0 || point.x >= m_width))
		return false;
	if (!m_torusY && (point.y < 0 || point.y >= m_height))
		return false;

	return true;
}

Map::Point Map::clamp(const Point& point) const
{
	const int x = m_torusX ? positiveMod(point.x, m_width) : std::clamp(point.x, 0, m_width - 1);
	const int y = m_torusY ? positiveMod(point.x, m_height) : std::clamp(point.y, 0, m_height - 1);

	return { x, y };
}

Map::Flags Map::at(const Point& point) const
{
	return m_flags[point.x + point.y * m_width];
}

Map::Flags operator~(Map::Flags flag)
{
	return static_cast<Map::Flags>(~static_cast<int>(flag));
}

Map::Flags operator|(Map::Flags lhs, Map::Flags rhs)
{
	return static_cast<Map::Flags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

Map::Flags operator&(Map::Flags lhs, Map::Flags rhs)
{
	return static_cast<Map::Flags>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

Map::Flags& operator|=(Map::Flags& lhs, Map::Flags rhs)
{
	return lhs = lhs | rhs;
}

Map::Flags& operator&=(Map::Flags& lhs, Map::Flags rhs)
{
	return lhs = lhs & rhs;
}
