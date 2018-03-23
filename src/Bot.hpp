#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

class Client;
struct PRIVMSG;

class Bot
{
public:
	using Ptr = std::unique_ptr<Bot>;

public:
	explicit Bot(Client& client);
	virtual ~Bot() = default;

	Bot(const Bot&) = delete;
	Bot& operator=(const Bot&) = delete;

	virtual void handleEvent(const sf::Event& event) = 0;
	virtual void update(sf::Time dt) = 0;
	virtual void draw(sf::RenderTarget& target) = 0;

	virtual void handlePRIVMSG(const PRIVMSG& priv) = 0;

protected:
	bool isAdmin(const std::string& username) const;
	void sendPRIVMSG(const std::string& msg);

private:
	Client& m_client;
};

std::pair<std::string, std::string> splitCommand(const std::string& command);
