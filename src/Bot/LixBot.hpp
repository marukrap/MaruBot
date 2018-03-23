#pragma once

#include "../Bot.hpp"
#include "Lix/World.hpp"

#include <SFML/Graphics/RenderTexture.hpp>

class LixBot : public Bot
{
public:
	enum class Command
	{
		Join,
		Part,
		Walk,
		Jump,
		Run,
		Climb,
		Float,
		Bat,
		Implode,
		Explode,
		Block,
		Cube,
		Build,
		Platform,
		Bash,
		Mine,
		Dig,
	};

public:
	LixBot(Client& client, const sf::RenderTarget& target);

	void handleEvent(const sf::Event& event) override;
	void update(sf::Time dt) override;
	void draw(sf::RenderTarget& target) override;

	void handlePRIVMSG(const PRIVMSG& priv) override;

private:
	void loadResources();
	void initializeCommands();

	void updateNuke();
	void updateLixxies();
	void removeDestroyedLixxies();

	// TODO: move to World
	Lixxie* getLixxie(const std::string& id) const;
	Lixxie* getLixxie(const sf::Vector2f& position) const;

	Lixxie* addLixxie(const std::string& id);

	void removeLixxie(Lixxie& lixxie);
	void removeAllLixxiees(const std::string& id);

	void relocate(Lixxie& lixxie);
	void relocateLixxies();

	void clearLixxies();
	void clearWorld();
	//

private:
	std::unordered_map<std::string, Command> m_commandBinding;

	// resources
	std::vector<std::unique_ptr<sf::Texture>> m_textures;
	sf::Font m_font;
	unsigned int m_fontSize = 0;

	sf::Vector2i m_mapSize;
	sf::RenderTexture m_renderTexture;
	sf::Time m_elapsedTime;
	World m_world;
	Lixxie* m_pickedLixxie = nullptr;
	bool m_picking = false;
	bool m_nuking = false;
	// TODO: m_drawing & m_erasing
};
