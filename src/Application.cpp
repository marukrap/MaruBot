#include "Application.hpp"
#include "Client.hpp"
#include "Utility.hpp"

// bots
#include "Bot/Jukebox.hpp"
#include "Bot/AnswerBot.hpp"
#include "Bot/LixBot.hpp"
#include "Bot/BattlepusBot.hpp"

#include <Windows.h>
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")

Application::Application(Client& client)
	: m_client(client)
{
	// TODO: https://stackoverflow.com/questions/9283430/how-to-set-icon-in-a-windows-application-using-visual-studio

	sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	sf::Vector2i windowPos;

	if (HWND taskbar = FindWindow(L"Shell_TrayWnd", nullptr))
	{
		RECT rect;
		GetWindowRect(taskbar, &rect);

		const int width = rect.right - rect.left;
		const int height = rect.bottom - rect.top;

		if (width > height)
		{
			mode.height -= height;

			if (rect.top == 0)
				windowPos.y = rect.bottom;
		}

		else
		{
			mode.width -= width;

			if (rect.left == 0)
				windowPos.x = rect.right;
		}
	}

	m_window.create(mode, "MaruBot", sf::Style::None);
	m_window.setPosition(windowPos);
	m_window.setFramerateLimit(60);

	HWND hWnd = m_window.getSystemHandle();

	// always on top
	// https://en.sfml-dev.org/forums/index.php?topic=2271.0
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// transparent
	// https://gist.github.com/Alia5/5d8c48941d1f73c1ef14967a5ffe33d5
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(hWnd, &margins);
	SetWindowLong(hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

	// click-through
	// https://stackoverflow.com/questions/31313624/click-through-transparent-window-no-dragging-allowed-c
	SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT);

	m_bots.emplace_back(std::make_unique<Jukebox>(m_client));
	m_bots.emplace_back(std::make_unique<AnswerBot>(m_client));
	m_bots.emplace_back(std::make_unique<LixBot>(m_client, m_window));
	// m_bots.emplace_back(std::make_unique<BattlepusBot>(m_client, m_window));
}

void Application::run()
{
	const sf::Time timePerFrame = sf::seconds(1.f / 60);

	sf::Clock clock;
	sf::Time timeSinceLastUpdate;

	while (m_window.isOpen())
	{
		const sf::Time dt = clock.restart();
		timeSinceLastUpdate += dt;

		if (timeSinceLastUpdate >= timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;

			processInput();
			update(timePerFrame);
		}

		render();
	}
}

void Application::processInput()
{
	sf::Event event;

	while (m_window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			m_window.close();

		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Escape)
				m_window.close();
		}

		for (auto& bot : m_bots)
			bot->handleEvent(event);
	}

	while (!m_client.isMessageQueueEmpty())
	{
		PRIVMSG priv = m_client.popMessage();

		// HACK: auto join/part
		if (priv.message == "JOIN")
			priv.message = "!join";
		else if (priv.message == "PART")
			priv.message = "!part";

		if (!priv.message.empty() && priv.message[0] == '!')
		{
			handlePRIVMSG(priv);

			for (auto& bot : m_bots)
				bot->handlePRIVMSG(priv);
		}
	}
}

void Application::update(sf::Time dt)
{
	for (auto& bot : m_bots)
		bot->update(dt);
}

void Application::render()
{
	m_window.clear(sf::Color::Transparent);

	for (const auto& bot : m_bots)
		bot->draw(m_window);

	m_window.display();
}

void Application::handlePRIVMSG(const PRIVMSG& priv)
{
	if (!m_client.isAdmin(priv.username))
		return;

	auto [first, second] = splitCommand(priv.message);
	toLower(second);

	if (first == "!quit")
		m_window.close();

	else if (first == "!addbot" && !second.empty())
	{
		bool exists = false;

		for (const auto& bot : m_bots)
		{
			std::string name = typeid(*bot).name();
			toLower(name);

			if (endsWith(name, second))
			{
				exists = true;
				break;
			}
		}

		if (exists)
			m_client.sendPRIVMSG('@' + priv.username + ' ' + second + " is already running.");

		else
		{
			Bot::Ptr bot = nullptr;

			if (second == "jukebox")
				bot = std::make_unique<Jukebox>(m_client);
			else if (second == "answerbot")
				bot = std::make_unique<AnswerBot>(m_client);
			else if (second == "lixbot")
				bot = std::make_unique<LixBot>(m_client, m_window);
			else if (second == "battlepusbot")
				bot = std::make_unique<BattlepusBot>(m_client, m_window);

			if (bot)
			{
				m_bots.emplace_back(std::move(bot));
				m_client.sendPRIVMSG('@' + priv.username + " Added bot: " + second);
			}

			else
				m_client.sendPRIVMSG('@' + priv.username + " Unrecognized bot name: " + second);
		}
	}

	else if (first == "!removebot" && !second.empty())
	{
		bool removed = false;

		for (auto it = m_bots.begin(); it != m_bots.end(); ++it)
		{
			std::string name = typeid(*(*it)).name();
			toLower(name);

			if (endsWith(name, second))
			{
				removed = true;
				m_bots.erase(it);
				break;
			}
		}

		if (removed)
			m_client.sendPRIVMSG('@' + priv.username + " Removed bot: " + second);
		else
			m_client.sendPRIVMSG('@' + priv.username + " Unrecognized bot name: " + second);
	}
}
