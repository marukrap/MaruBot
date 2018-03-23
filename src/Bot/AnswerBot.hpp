#pragma once

#include "../Bot.hpp"

#include <map>

class AnswerBot : public Bot
{
public:
	explicit AnswerBot(Client& client);
	~AnswerBot();

	void handleEvent(const sf::Event& event) override;
	void update(sf::Time dt) override;
	void draw(sf::RenderTarget& target) override;

	void handlePRIVMSG(const PRIVMSG& priv) override;

private:
	std::pair<std::string, std::string> parseLine(const std::string& line) const;

private:
	std::map<std::string, std::string> answerMap;
};
