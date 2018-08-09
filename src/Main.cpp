#include "Client.hpp"
#include "Application.hpp"

#include <Windows.h>

#include <iostream>

int main()
{
	SetWindowPos(GetConsoleWindow(), nullptr, 2315, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	Client client;

	if (client.connect("Config.txt"))
	{
		Application app(client);
		app.run();
	}

	else
	{
		std::cout << "Press Enter to exit... ";
		std::cin.get();
	}

	client.disconnect();

	return 0;
}
