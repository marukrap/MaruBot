#include "BattlepusBot.hpp"
#include "../Client.hpp"
#include "../Utility.hpp"

#include <SFML/Audio/Sound.hpp>

#include <filesystem>
#include <cctype>
#include <cassert>

namespace fs = std::experimental::filesystem;

BattlepusBot::BattlepusBot(Client& client, const sf::RenderTarget& target)
	: Bot(client)
	, m_mapSize(static_cast<sf::Vector2i>(target.getSize()))
{
	m_renderTexture.create(m_mapSize.x, m_mapSize.y);

	loadResources();
	initializeCommands();
}

void BattlepusBot::handleEvent(const sf::Event& event)
{
	auto getMousePosition = [] () { return static_cast<sf::Vector2f>(sf::Mouse::getPosition()); };

	bool removePickedPlaty = false;

	if (event.type == sf::Event::KeyPressed && !m_pickedPlaty)
	{
		if (event.key.code == sf::Keyboard::LControl)
		{
			m_pickedPlaty = getPlaty(getMousePosition());

			if (m_pickedPlaty)
				m_picking = true;
		}

		else if (event.key.code == sf::Keyboard::LAlt)
		{
			if (Platy* platy = getPlaty(getMousePosition()))
				changePlaty(*platy);
		}
	}

	else if (event.type == sf::Event::KeyReleased && m_pickedPlaty)
	{
		if (event.key.code == sf::Keyboard::LControl)
			removePickedPlaty = true;
	}

	else if (event.type == sf::Event::LostFocus && m_pickedPlaty)
		removePickedPlaty = true;

	if (removePickedPlaty)
	{
		const sf::Vector2f pos = getMousePosition();

		if (pos.x < 0.f || pos.x >= m_mapSize.x || pos.y < 0.f || pos.y >= m_mapSize.y)
		{
			removePlaty(*m_pickedPlaty);
			playSound("platypus3");
		}

		else
			m_pickedPlaty->setPosition(pos);

		m_pickedPlaty = nullptr;
		m_picking = false;
	}
}

void BattlepusBot::update(sf::Time dt)
{
	for (auto& platy : m_platies)
	{
		platy->update(dt);

		sf::Vector2f pos = platy->getPosition();
		const sf::Vector2i size = platy->getTileSize();

		if (pos.x + size.x / 2.f < 0.f)
			pos.x = m_mapSize.x + size.x / 2.f;
		else if (pos.x > m_mapSize.x + size.x / 2.f)
			pos.x = -size.x / 2.f;

		if (pos.y + size.x / 2.f < 0.f)
			pos.y = m_mapSize.y + size.y / 2.f;
		else if (pos.y > m_mapSize.y + size.y / 2.f)
			pos.y = -size.y / 2.f;

		platy->setPosition(pos);
	}

	removeStoppedSounds();

	if (m_picking)
		m_pickedPlaty->setPosition(static_cast<sf::Vector2f>(sf::Mouse::getPosition()));
}

void BattlepusBot::draw(sf::RenderTarget& target)
{
	m_renderTexture.clear(sf::Color::Transparent);

	for (const auto& platy : m_platies)
		m_renderTexture.draw(*platy);

	m_renderTexture.display();

	target.draw(sf::Sprite(m_renderTexture.getTexture()));
}

void BattlepusBot::handlePRIVMSG(const PRIVMSG& priv)
{
	auto [first, second] = splitCommand(priv.message);
	toLower(second);

	if (first == "!command" && (second == "battlepus" || second == "battlepusbot"))
		sendPRIVMSG('@' + priv.username + " Available commands: !join, !part, !change, !s, !sw, !w, !nw, !n, !ne, !e, !se");

	else if (auto found = m_commandBinding.find(first); found != m_commandBinding.end())
	{
		auto platy = getPlaty(priv.username);

		if (!platy && found->second != Command::Join && found->second != Command::Part)
		{
			sendPRIVMSG('@' + priv.username + " Try !join first.");
			return;
		}

		switch (found->second)
		{
		case Command::Join:
			if (isAdmin(priv.username) && !second.empty())
			{
				if (second == "all")
					addAllPlatyes(priv.username);

				else if (std::isdigit(second[0]))
				{
					const int count = std::stoi(second);

					for (int i = 0; i < count; ++i)
						addPlaty("");
				}
			}

			else if (!platy)
			{
				platy = addPlaty(priv.username, second);

				if (platy)
					playSound("platypus1");
				else
					sendPRIVMSG('@' + priv.username + " Unrecognized name: " + second);
			}
			break;

		case Command::Part:
			if (isAdmin(priv.username) && !second.empty() && second == "all")
			{
				removeAllPlatyes(priv.username);
				platy = nullptr;
			}

			else if (platy)
			{
				removePlaty(*platy);
				playSound("platypus3");
				platy = nullptr;
			}
			break;

		case Command::Change:
			platy = changePlaty(*platy, second);

			if (platy)
				playSound("platypus1");
			else
				sendPRIVMSG('@' + priv.username + " Unrecognized name: " + second);
			break;

		case Command::MoveS:  platy->setDirection(Direction::S);  break;
		case Command::MoveSW: platy->setDirection(Direction::SW); break;
		case Command::MoveW:  platy->setDirection(Direction::W);  break;
		case Command::MoveNW: platy->setDirection(Direction::NW); break;
		case Command::MoveN:  platy->setDirection(Direction::N);  break;
		case Command::MoveNE: platy->setDirection(Direction::NE); break;
		case Command::MoveE:  platy->setDirection(Direction::E);  break;
		case Command::MoveSE: platy->setDirection(Direction::SE); break;

		case Command::Alpha:
			if (isAdmin(priv.username) && !second.empty() && std::isdigit(second[0]))
			{
				if (const int value = std::stoi(second); value >= 0 && value <= 255)
				{
					m_alpha = value;

					for (auto& p : m_platies)
						p->setAlpha(m_alpha);
				}
			}
			break;

		case Command::Scale:
			if (isAdmin(priv.username) && !second.empty() && std::isdigit(second[0]))
			{
				if (const float value = std::stof(second); value >= 1.f && value <= 4.f)
				{
					m_scale = value;

					for (auto& p : m_platies)
						p->setScale(m_scale);
				}
			}
			break;
		}

		if (platy)
			platy->setColor(sf::Color((priv.color << 8) | m_alpha));
	}
}

void BattlepusBot::loadResources()
{
	// load textures
	const std::vector<std::string> directories =
	{
		"Battlepus/Images/Characters",
		"Battlepus/Images/Vehicles",
		"Battlepus/Images/Turrets",
	};

	for (std::size_t i = 0; i < directories.size(); ++i)
		for (const auto& entry : fs::directory_iterator(directories[i]))
		{
			if (!fs::is_regular_file(entry) || entry.path().extension() != ".png")
				continue;

			auto texture = std::make_unique<Texture>();
			texture->type = static_cast<Platy::Type>(i);
			texture->texture.loadFromFile(entry.path().string());
			texture->frameSize = { 64, 64 };

			std::string id = entry.path().stem().string();
			toLower(id);

			if (id == "platycarrier")
				texture->frameSize = { 74, 74 };

			else if (id == "platyspiderbot")
				texture->frameSize = { 96, 96 };

			else if (id == "platyswashbucklership")
				texture->type = Platy::Type::SwashBucklerShip;

			else if (texture->type == Platy::Type::Character)
				texture->frameSize = { 32, 32 };

			const auto inserted = m_textures.emplace(id, std::move(texture));
			assert(inserted.second);
		}

	// load font
	m_font.loadFromFile("Battlepus/Fonts/Magero.ttf");
	m_fontSize = 8;
	const_cast<sf::Texture&>(m_font.getTexture(m_fontSize)).setSmooth(false);

	// load sounds
	for (const auto& entry : fs::directory_iterator("Battlepus/Sounds"))
	{
		if (!fs::is_regular_file(entry) || entry.path().extension() != ".wav")
			continue;

		auto sound = std::make_unique<sf::SoundBuffer>();
		sound->loadFromFile(entry.path().string());

		std::string id = entry.path().stem().string();
		toLower(id);

		const auto inserted = m_soundBuffers.emplace(id, std::move(sound));
		assert(inserted.second);
	}
}

void BattlepusBot::initializeCommands()
{
	m_commandBinding["!join"]   = Command::Join;
	m_commandBinding["!part"]   = Command::Part;
	m_commandBinding["!change"] = Command::Change;

	m_commandBinding["!s"]      = Command::MoveS;
	m_commandBinding["!sw"]     = Command::MoveSW;
	m_commandBinding["!w"]      = Command::MoveW;
	m_commandBinding["!nw"]     = Command::MoveNW;
	m_commandBinding["!n"]      = Command::MoveN;
	m_commandBinding["!ne"]     = Command::MoveNE;
	m_commandBinding["!e"]      = Command::MoveE;
	m_commandBinding["!se"]     = Command::MoveSE;

	m_commandBinding["!d"]      = Command::MoveS;
	m_commandBinding["!dl"]     = Command::MoveSW;
	m_commandBinding["!l"]      = Command::MoveW;
	m_commandBinding["!ul"]     = Command::MoveNW;
	m_commandBinding["!u"]      = Command::MoveN;
	m_commandBinding["!ur"]     = Command::MoveNE;
	m_commandBinding["!r"]      = Command::MoveE;
	m_commandBinding["!dr"]     = Command::MoveSE;

	m_commandBinding["!alpha"]  = Command::Alpha;
	m_commandBinding["!scale"]  = Command::Scale;
}

Platy* BattlepusBot::getPlaty(const std::string& id) const
{
	auto it = std::find_if(m_platies.rbegin(), m_platies.rend(),
		[&] (const auto& p) { return p->getId() == id; });

	if (it != m_platies.rend())
		return it->get();

	return nullptr;
}

Platy* BattlepusBot::getPlaty(const sf::Vector2f& position) const
{
	auto it = std::find_if(m_platies.rbegin(), m_platies.rend(),
		[&] (const auto& p) { return p->getBoundingRect().contains(position); });

	if (it != m_platies.rend())
		return it->get();

	return nullptr;
}

Platy* BattlepusBot::addPlaty(const std::string& id, const std::string& platyName)
{
	Texture* texture = nullptr;

	if (!platyName.empty())
	{
		if (auto found = m_textures.find(platyName); found != m_textures.end())
			texture = found->second.get();
		else
			return nullptr;
	}

	else
	{
		const auto it = std::next(m_textures.begin(), randomInt(m_textures.size()));
		texture = it->second.get();
	}

	m_platies.emplace_back(std::make_unique<Platy>(texture->type, id, texture->texture, texture->frameSize));
	Platy* platy = m_platies.back().get();
	platy->setText(id, m_font, m_fontSize);

	const float x = static_cast<float>(randomInt(m_mapSize.x));
	const float y = static_cast<float>(randomInt(m_mapSize.y));

	platy->setPosition(x, y);
	platy->setDirection(static_cast<Direction>(randomInt(8)));
	platy->setAlpha(m_alpha);
	platy->setScale(m_scale);

	return platy;
}

Platy* BattlepusBot::changePlaty(Platy& platy, const std::string& platyName)
{
	const sf::Vector2f pos = platy.getPosition();
	const Direction dir = platy.getDirection();
	const std::string text = platy.getText();
	const sf::Color color = platy.getColor();

	Platy* newPlaty = addPlaty(platy.getId(), platyName);

	if (newPlaty)
	{
		newPlaty->setPosition(pos);
		newPlaty->setDirection(dir);
		newPlaty->setText(text);
		newPlaty->setColor(color);

		removePlaty(platy);
	}

	return newPlaty;
}

void BattlepusBot::removePlaty(Platy& platy)
{
	auto it = std::find_if(m_platies.begin(), m_platies.end(),
		[&] (const auto& p) { return p.get() == &platy; });

	assert(it != m_platies.end());
	m_platies.erase(it);
}

void BattlepusBot::addAllPlatyes(const std::string& id)
{
	for (const auto& pair : m_textures)
	{
		Platy* platy = addPlaty(id, pair.first);
		platy->setText(pair.first);
	}
}

void BattlepusBot::removeAllPlatyes(const std::string& id)
{
	auto it = std::remove_if(m_platies.begin(), m_platies.end(),
		[&] (const auto& p) { return p->getId() == id; });

	m_platies.erase(it, m_platies.end());
}

void BattlepusBot::playSound(const std::string& id)
{
	const auto found = m_soundBuffers.find(id);
	assert(found != m_soundBuffers.end());

	m_sounds.emplace_back(*found->second);
	m_sounds.back().setVolume(m_volume);
	m_sounds.back().play();
}

void BattlepusBot::removeStoppedSounds()
{
	m_sounds.remove_if([] (const auto& s) { return s.getStatus() == sf::Sound::Stopped; });
}
