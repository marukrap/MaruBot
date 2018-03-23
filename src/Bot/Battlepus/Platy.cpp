#include "Platy.hpp"
#include "../../Utility.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>

Platy::Platy(Type type, const std::string& id, const sf::Texture& texture, const sf::Vector2i& frameSize)
	: m_type(type)
	, m_id(id)
	, m_sprite(texture)
	, m_frameSize(frameSize)
{
	updateTextureRect();
	m_sprite.setOrigin(m_frameSize.x / 2.f, m_frameSize.y / 2.f);

	const int cols = texture.getSize().x / m_frameSize.x;
	const int rows = texture.getSize().y / m_frameSize.y;

	if (type == Type::Turret)
	{
		m_numFrames = 8;
		m_timePerFrame = sf::seconds(0.5f);
		m_hasAttackAnimation = true;
		m_hasSkillAnimation = (cols * rows) / 3 > 8;
	}

	else
	{
		m_numFrames = 4;
		m_timePerFrame = sf::seconds(0.1f);
		m_hasAttackAnimation = (cols * rows) / m_numFrames > 8;
		m_hasSkillAnimation = (cols * rows) / m_numFrames > 14;
	}
}

const std::string& Platy::getId() const
{
	return m_id;
}

const sf::String& Platy::getText() const
{
	return m_text.getString();
}

void Platy::setText(const std::string& str, const sf::Font& font, unsigned int fontSize)
{
	m_text.setFont(font);
	m_text.setCharacterSize(fontSize);
	m_text.setOutlineThickness(1.f);
	setText(str);
}

void Platy::setText(const std::string& str)
{
	m_text.setString(str);
	centerOrigin(m_text);
}

const sf::Color& Platy::getColor() const
{
	return m_text.getOutlineColor();
}

void Platy::setColor(const sf::Color& color)
{
	m_text.setOutlineColor(color);
}

Direction Platy::getDirection() const
{
	return m_direction;
}

void Platy::setDirection(Direction direction)
{
	m_direction = direction;
	updateTextureRect();
}

void Platy::setAlpha(int alpha)
{
	assert(alpha >= 0 && alpha <= 255);

	sf::Color spriteColor = m_sprite.getColor();
	spriteColor.a = alpha;
	m_sprite.setColor(spriteColor);

	sf::Color textFillColor = m_text.getFillColor();
	textFillColor.a = alpha;
	m_text.setFillColor(textFillColor);

	sf::Color textOutlineColor = m_text.getOutlineColor();
	textOutlineColor.a = alpha;
	m_text.setOutlineColor(textOutlineColor);
}

void Platy::setScale(float scale)
{
	m_sprite.setScale(scale, scale);
	m_text.setScale(scale, scale);
	m_text.setPosition(0.f, -m_frameSize.y * scale / 2);
	centerOrigin(m_text);
}

const sf::Vector2i& Platy::getTileSize() const
{
	return m_frameSize;
}

sf::FloatRect Platy::getBoundingRect() const
{
	return getTransform().transformRect(m_sprite.getGlobalBounds());
}

void Platy::update(sf::Time dt)
{
	m_elapsedTime += dt;

	if (m_elapsedTime < m_timePerFrame)
		return;

	m_elapsedTime -= m_timePerFrame;
	m_currentFrame = (m_currentFrame + 1) % m_numFrames;

	if (m_type != Type::Turret && !m_attacking && !m_casting)
	{
		float velocity = 1.f;

		if (m_type == Type::Vehicle || m_type == Type::SwashBucklerShip)
			velocity += 1.f;

		switch (m_direction)
		{
		case Direction::S:  move(      0.f,  velocity); break;
		case Direction::SW: move(-velocity,  velocity); break;
		case Direction::W:  move(-velocity,       0.f); break;
		case Direction::NW: move(-velocity, -velocity); break;
		case Direction::N:  move(      0.f, -velocity); break;
		case Direction::NE: move( velocity, -velocity); break;
		case Direction::E:  move( velocity,       0.f); break;
		case Direction::SE: move( velocity,  velocity); break;
		}
	}

	if (m_type == Type::SwashBucklerShip && m_hasAttackAnimation && m_currentFrame == 0)
	{
		if (!m_attacking && randomInt(10) == 0)
		{
			m_attacking = true;
			m_numFrames = 16;
			m_timePerFrame = sf::seconds(0.15f);
			m_elapsedTime = sf::Time::Zero;
		}

		else if (m_attacking && randomInt(5) > 0)
		{
			m_attacking = false;
			m_numFrames = 4;
			m_timePerFrame = sf::seconds(0.1f);
			m_elapsedTime = sf::Time::Zero;
		}
	}

	else if (m_type != Type::Turret && m_hasAttackAnimation && m_currentFrame == 0)
	{
		if (!m_attacking && randomInt(10) == 0)
		{
			if (m_hasSkillAnimation && randomInt(2) == 0)
				m_casting = true;
			else
				m_attacking = true;

			m_numFrames = 3;
			m_timePerFrame = sf::seconds(0.15f);
			m_elapsedTime = sf::Time::Zero;
		}

		else if ((m_attacking || m_casting) && randomInt(5) == 0)
		{
			m_attacking = false;
			m_casting = false;
			m_numFrames = 4;
			m_timePerFrame = sf::seconds(0.1f);
			m_elapsedTime = sf::Time::Zero;
		}
	}

	else if (m_type == Type::Turret)
	{
		if (!m_attacking && randomInt(10) == 0)
		{
			if (m_hasSkillAnimation && randomInt(2) == 0)
				m_casting = true;
			else
				m_attacking = true;

			m_numFrames = 3;
			m_direction = static_cast<Direction>(m_currentFrame);
			m_currentFrame = 0;
			m_timePerFrame = sf::seconds(0.15f);
			m_elapsedTime = sf::Time::Zero;
		}

		else if ((m_attacking || m_casting) && m_currentFrame == 0 && randomInt(5) == 0)
		{
			m_attacking = false;
			m_casting = false;
			m_numFrames = 8;
			m_currentFrame = static_cast<int>(m_direction);
			m_timePerFrame = sf::seconds(0.5f);
			m_elapsedTime = sf::Time::Zero;
		}
	}

	// change direction
	if (m_type != Type::Turret && m_currentFrame == 0 && randomInt(100) == 0)
		m_direction = static_cast<Direction>(randomInt(8));

	updateTextureRect();
}

void Platy::updateTextureRect()
{
	const sf::Texture& texture = *m_sprite.getTexture();
	const int tileNumber = static_cast<int>(m_direction);

	int tileBegin = 0;

	if (m_type == Type::SwashBucklerShip)
	{
		if (m_attacking)
			tileBegin = 8 * 4 + m_currentFrame;
		else
			tileBegin = tileNumber * m_numFrames + m_currentFrame;
	}

	else if (m_type == Type::Turret)
	{
		if (m_casting)
			tileBegin = 8 * 3 + tileNumber + m_numFrames + m_currentFrame;
		else if (m_attacking)
			tileBegin = tileNumber * m_numFrames + m_currentFrame;
		else
			tileBegin = m_currentFrame * 3;
	}

	else
	{
		if (m_casting)
			tileBegin = 8 * 4 + 8 * 3 + tileNumber * m_numFrames + m_currentFrame;
		else if (m_attacking)
			tileBegin = 8 * 4 + tileNumber * m_numFrames + m_currentFrame;
		else
			tileBegin = tileNumber * m_numFrames + m_currentFrame;
	}

	const int tu = tileBegin % (texture.getSize().x / m_frameSize.x);
	const int tv = tileBegin / (texture.getSize().x / m_frameSize.x);

	m_sprite.setTextureRect({ tu * m_frameSize.x, tv * m_frameSize.y, m_frameSize.x, m_frameSize.y });
}

void Platy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(m_sprite, states);
	target.draw(m_text, states);
}
