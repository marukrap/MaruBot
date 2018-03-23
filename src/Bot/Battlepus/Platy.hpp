#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>

#include <memory>

enum class Direction
{
	S, SW, W, NW, N, NE, E, SE
};

class Platy : public sf::Drawable, public sf::Transformable
{
public:
	using Ptr = std::unique_ptr<Platy>;

	enum class Type
	{
		Character,
		Vehicle,
		Turret,
		Typecount,
		SwashBucklerShip, // exception
	};

public:
	Platy(Type type, const std::string& id, const sf::Texture& texture, const sf::Vector2i& frameSize);

	Platy(const Platy&) = delete;
	Platy& operator=(const Platy&) = delete;

	const std::string& getId() const;

	const sf::String& getText() const;
	void setText(const std::string& str, const sf::Font& font, unsigned int fontSize);
	void setText(const std::string& str);

	const sf::Color& getColor() const;
	void setColor(const sf::Color& color);

	Direction getDirection() const;
	void setDirection(Direction direction);

	void setAlpha(int alpha); // [0-255]
	void setScale(float scale); // [1-4]

	const sf::Vector2i& getTileSize() const;
	sf::FloatRect getBoundingRect() const;

	void update(sf::Time dt);

private:
	void updateTextureRect();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	Type m_type;
	std::string m_id; // username
	sf::Sprite m_sprite;
	sf::Vector2i m_frameSize;
	sf::Text m_text;
	Direction m_direction = Direction::S;

	// animation
	sf::Time m_elapsedTime;
	sf::Time m_timePerFrame;
	int m_currentFrame = 0;
	int m_numFrames = 0;

	bool m_hasAttackAnimation = false;
	bool m_hasSkillAnimation = false;
	bool m_attacking = false;
	bool m_casting = false;
};
