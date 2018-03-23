#include "AnswerBot.hpp"
#include "../Client.hpp"
#include "../Utility.hpp"

#include <fstream>
#include <iostream>
#include <cassert>

AnswerBot::AnswerBot(Client& client)
	: Bot(client)
{
	if (std::ifstream ifs("Answers.txt"); ifs)
	{
		for (std::string line; std::getline(ifs, line); )
		{
			const auto [key, value] = parseLine(line);

			if (!key.empty())
			{
				const auto inserted = answerMap.emplace(key, value);
				assert(inserted.second);
			}
		}
	}

	else
		std::cout << "[AnswerBot] Unable to load answers!\n";
}

AnswerBot::~AnswerBot()
{
	if (std::ofstream ofs("Answers.txt"); ofs)
	{
		for (const auto& [key, value] : answerMap)
		{
			if (!value.empty())
				ofs << key << " = " << value << '\n';
		}
	}

	else
		std::cout << "[AnswerBot] Unable to save answers!\n";
}

void AnswerBot::handleEvent(const sf::Event& event)
{
}

void AnswerBot::update(sf::Time dt)
{
}

void AnswerBot::draw(sf::RenderTarget& target)
{
}

void AnswerBot::handlePRIVMSG(const PRIVMSG& priv)
{
	const auto [first, second] = splitCommand(priv.message);

	if (isAdmin(priv.username))
	{
		if (first == "!answer" && !second.empty())
		{
			auto [question, answer] = splitCommand(second);

			if (question[0] != '!')
				question = '!' + question;

			if (!answer.empty())
			{
				answerMap[question] = answer;
				sendPRIVMSG('@' + priv.username + " Added new command: " + question);
			}

			else
			{
				if (const auto found = answerMap.find(question); found != answerMap.end())
				{
					answerMap.erase(found);
					sendPRIVMSG('@' + priv.username + " Removed command: " + question);
				}
			}
		}
	}

	if (const auto found = answerMap.find(first); found != answerMap.end())
		sendPRIVMSG('@' + priv.username + ' ' + found->second);
}

std::pair<std::string, std::string> AnswerBot::parseLine(const std::string& line) const
{
	std::string key, value;

	if (const std::size_t equal = line.find('='); equal != std::string::npos)
	{
		key = line.substr(0, equal);
		value = line.substr(equal + 1);

		key = trim(key);
		value = trim(value);
	}

	return { key, value };
}
