#pragma once

#include "Bot.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

class Application
{
public:
	explicit Application(Client& client);

	void run();

private:
	void processInput();
	void update(sf::Time dt);
	void render();

	void handlePRIVMSG(const PRIVMSG& priv);

private:
	Client& m_client;
	sf::RenderWindow m_window;
	std::vector<Bot::Ptr> m_bots;
};
