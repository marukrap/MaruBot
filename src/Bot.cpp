#include "Bot.hpp"
#include "Client.hpp"
#include "Utility.hpp"

Bot::Bot(Client& client)
	: m_client(client)
{
}

bool Bot::isAdmin(const std::string& username) const
{
	return m_client.isAdmin(username);
}

void Bot::sendPRIVMSG(const std::string& msg)
{
	m_client.sendPRIVMSG(msg);
}

std::pair<std::string, std::string> splitCommand(const std::string& command)
{
	std::string first, second;

	if (const std::size_t space = command.find(' '); space != std::string::npos)
	{
		first = command.substr(0, space);
		second = command.substr(space + 1);
	}

	else
		first = command;

	toLower(first);
	// toLower(second);

	return { first, second };
}
