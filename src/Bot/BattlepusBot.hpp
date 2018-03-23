#pragma once

#include "../Bot.hpp"
#include "Battlepus/Platy.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>

#include <unordered_map>

class BattlepusBot : public Bot
{
public:
	enum class Command
	{
		Join,
		Part,
		Change,
		MoveS,
		MoveSW,
		MoveW,
		MoveNW,
		MoveN,
		MoveNE,
		MoveE,
		MoveSE,
		Alpha,
		Scale,
	};

public:
	BattlepusBot(Client& client, const sf::RenderTarget& target);

	void handleEvent(const sf::Event& event) override;
	void update(sf::Time dt) override;
	void draw(sf::RenderTarget& target) override;

	void handlePRIVMSG(const PRIVMSG& priv) override;

private:
	struct Texture
	{
		Platy::Type type;
		sf::Texture texture;
		sf::Vector2i frameSize;
	};

	void loadResources();
	void initializeCommands();

	Platy* getPlaty(const std::string& id) const;
	Platy* getPlaty(const sf::Vector2f& position) const;

	Platy* addPlaty(const std::string& id, const std::string& platyName = "");
	Platy* changePlaty(Platy& platy, const std::string& platyName = "");
	void removePlaty(Platy& platy);

	void addAllPlatyes(const std::string& id);
	void removeAllPlatyes(const std::string& id);

	void playSound(const std::string& id);
	void removeStoppedSounds();

private:
	std::unordered_map<std::string, Command> m_commandBinding;

	// resources
	std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
	std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
	sf::Font m_font;
	unsigned int m_fontSize = 0;

	sf::Vector2i m_mapSize;
	sf::RenderTexture m_renderTexture;
	std::vector<Platy::Ptr> m_platies;
	std::list<sf::Sound> m_sounds;
	Platy* m_pickedPlaty = nullptr;
	bool m_picking = false;
	int m_alpha = 255;
	float m_scale = 1.f;
	float m_volume = 75.f;
};
