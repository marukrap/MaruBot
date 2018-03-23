#pragma once

#include "TerrainChange.hpp"

#include <SFML/Graphics/Texture.hpp>

#include <vector>

class Mask;

class Map
{
public:
	using Point = sf::Vector2i;

	enum Flags
	{
		None    = 0,
		Terrain = 1 << 0,
		Steel   = 1 << 1,
		Goal    = 1 << 2,
		Fire    = 1 << 3,
		Water   = 1 << 4,
		Trap    = 1 << 5,
		Fling   = 1 << 6,
		All     = ~0,
	};

public:
	Map(int width, int height, bool torusX = false, bool torusY = false, bool smoothlyScalable = false);

	const sf::Texture& getTexture() const;

	void draw(Point point, sf::IntRect rect);
	void draw(Point point, TerrainDeletion::Type type, int digYl = 0); // deletions

	int getWidth() const;
	int getHeight() const;

	int getTorusX() const;
	int getTorusY() const;

	Flags get(const Point& point) const;
	bool get(const Point& point, Flags flag) const;

	bool getSolid(const Point& point) const;
	bool getSolidEven(const Point& point) const;

	bool getSteel(const Point& point) const;
	bool getSteelUnlessMaskIgnores(const Point& point, const Mask& mask) const;

	void add(const Point& point, Flags flag);

	bool setAirCountSteel(const Point& point);
	int setAirCountSteelEvenWhereMaskIgnores(const Point& topLeft, const Mask& mask);

	//
	int distanceX(int x1, int x2) const;
	int distanceY(int y1, int y2) const;

	Point wrap(const Point& point) const;

private:
	bool inside(const Point& point) const;

	Point clamp(const Point& point) const;
	Map::Flags at(const Point& point) const;

private:
	// TODO: improve performance (sf::Image > sf::RenderTarget/sf::BlendMode)
	sf::Image m_image;
	sf::Image m_textureImage;
	std::vector<sf::Image> m_subImages;
	sf::Texture m_texture;
	std::vector<Flags> m_flags; // lookup table
	int m_width;
	int m_height;
	bool m_torusX;
	bool m_torusY;
	bool m_smoothlyScalable;

public:
	__declspec(property(get = getWidth)) int width;
	__declspec(property(get = getHeight)) int height;
	__declspec(property(get = getTorusX)) int torusX;
	__declspec(property(get = getTorusY)) int torusY;
};

Map::Flags operator~(Map::Flags flag);
Map::Flags operator|(Map::Flags lhs, Map::Flags rhs);
Map::Flags operator&(Map::Flags lhs, Map::Flags rhs);
Map::Flags& operator|=(Map::Flags& lhs, Map::Flags rhs);
Map::Flags& operator&=(Map::Flags& lhs, Map::Flags rhs);
