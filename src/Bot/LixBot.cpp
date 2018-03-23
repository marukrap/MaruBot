#include "LixBot.hpp"
#include "Lix/Mask.hpp"
#include "../Client.hpp"
#include "../Utility.hpp"

#include <cassert>

LixBot::LixBot(Client& client, const sf::RenderTarget& target)
	: Bot(client)
	, m_mapSize(static_cast<sf::Vector2i>(target.getSize()))
	, m_world(m_mapSize)
{
	m_renderTexture.create(m_mapSize.x, m_mapSize.y);

	loadResources();
	initializeCommands();
}

void LixBot::handleEvent(const sf::Event& event)
{
	bool removePickedLixxie = false;

	if (event.type == sf::Event::KeyPressed && !m_pickedLixxie)
	{
		if (event.key.code == sf::Keyboard::LControl)
		{
			m_pickedLixxie = getLixxie(static_cast<sf::Vector2f>(sf::Mouse::getPosition()));

			if (m_pickedLixxie)
				m_picking = true;
		}

		else
		{
			auto assignActivity = [this] (Activity activity)
			{
				if (Lixxie* lixxie = getLixxie(static_cast<sf::Vector2f>(sf::Mouse::getPosition())))
				{
					if (lixxie->priorityForNewAc(activity) > 1)
					{
						lixxie->assignManually(activity);
						m_world.playSound("assign");
					}
				}
			};

			switch (event.key.code)
			{
			case sf::Keyboard::Space:
				addLixxie("");
				m_world.playSound("lets_go");
				break;

			case sf::Keyboard::Num1: assignActivity(Activity::Runner);     break; // (shift) run faster than walkers.
			case sf::Keyboard::Num2: assignActivity(Activity::Climber);    break; // (z) climb all vertical walls.
			case sf::Keyboard::Num3: assignActivity(Activity::Floater);    break; // (q) float to survive all falls.

			case sf::Keyboard::Q:    assignActivity(Activity::Walker);     break; // (d) walk back or cancel blockers.
			case sf::Keyboard::W:    assignActivity(Activity::Jumper);     break; // (r) jump once. Runners jump further.
			case sf::Keyboard::E:    assignActivity(Activity::Batter);     break; // (c) bat other lix, flinging them.
			case sf::Keyboard::R:    assignActivity(Activity::Blocker);    break; // (x) block other lix, turning them.

			case sf::Keyboard::A:    assignActivity(Activity::Builder);    break; // (a) build a diagonal staircase.
			case sf::Keyboard::S:    assignActivity(Activity::Platformer); break; // (t) platform a horizontal bridge.
			case sf::Keyboard::D:    assignActivity(Activity::Cuber);      break; // (b) cube to generate earth quickly.
			case sf::Keyboard::F:    assignActivity(Activity::Imploder);   break; // (v) implode to destroy earth/cancel blockers.

			case sf::Keyboard::Z:    assignActivity(Activity::Basher);     break; // (e) bash a horizontal tunnel.
			case sf::Keyboard::X:    assignActivity(Activity::Miner);      break; // (g) mine a diagonal tunnel.
			case sf::Keyboard::C:    assignActivity(Activity::Digger);     break; // (w) dig a vertical hole.
			case sf::Keyboard::V:    assignActivity(Activity::Exploder);   break; // (v) explode to fling other lix.

			case sf::Keyboard::F4: // nuke.
				if (!m_nuking)
				{
					m_nuking = true;
					m_world.playSound("nuke");
				}
				break;
			}
		}
	}

	else if (event.type == sf::Event::KeyReleased && m_pickedLixxie)
	{
		if (event.key.code == sf::Keyboard::LControl)
			removePickedLixxie = true;
	}

	else if (event.type == sf::Event::LostFocus && m_pickedLixxie)
		removePickedLixxie = true;

	if (removePickedLixxie)
	{
		const sf::Vector2i pos = sf::Mouse::getPosition();

		if (pos.x < 0 || pos.x >= m_mapSize.x || pos.y < 0 || pos.y >= m_mapSize.y)
		{
			removeLixxie(*m_pickedLixxie);
			m_world.playSound("oblivion"); // "cant_win"
		}

		else
		{
			if (m_pickedLixxie->ex != pos.x)
				m_pickedLixxie->ex = pos.x;
			if (m_pickedLixxie->ey != pos.y)
				m_pickedLixxie->ey = pos.y;
		}

		m_pickedLixxie = nullptr;
		m_picking = false;
	}
}

void LixBot::update(sf::Time dt)
{
	static const int updatePerSecond = 15;
	static const sf::Time timePerUpdate = sf::seconds(1.f / updatePerSecond);

	m_elapsedTime += dt;

	if (m_elapsedTime >= timePerUpdate)
	{
		m_elapsedTime -= timePerUpdate;

		++m_world.update;
		updateNuke();
		updateLixxies();
		removeDestroyedLixxies();
	}

	m_world.applyChangesToLand();
	m_world.updateDebris();
	m_world.removeStoppedSounds();

	if (m_picking)
	{
		const sf::Vector2i pos = sf::Mouse::getPosition();

		if (m_pickedLixxie->ex != pos.x)
			m_pickedLixxie->ex = pos.x;
		if (m_pickedLixxie->ey != pos.y)
			m_pickedLixxie->ey = pos.y;
	}
}

void LixBot::draw(sf::RenderTarget& target)
{
	m_renderTexture.clear(sf::Color::Transparent);

	m_renderTexture.draw(sf::Sprite(m_world.lookup.getTexture()));

	for (const auto& lixxie : m_world.tribe)
		m_renderTexture.draw(*lixxie);

	for (const auto& debris : m_world.debris)
		m_renderTexture.draw(debris.sprite);

	m_renderTexture.display();

	target.draw(sf::Sprite(m_renderTexture.getTexture()));
}

void LixBot::handlePRIVMSG(const PRIVMSG& priv)
{
	auto [first, second] = splitCommand(priv.message);
	toLower(second);

	if (first == "!command" && (second == "lix" || second == "lixbot"))
		sendPRIVMSG('@' + priv.username + " Available commands: !join, !part, !walk, !jump, !run, !climb, !float, !bat, !implode, !explode, !block, !cube, !build, !platform, !bash, !mine, !dig");

	else if (auto found = m_commandBinding.find(first); found != m_commandBinding.end())
	{
		auto lixxie = getLixxie(priv.username);

		if (!lixxie && found->second != Command::Join && found->second != Command::Part)
		{
			sendPRIVMSG('@' + priv.username + " Try !join first.");
			return;
		}

		auto assignActivity = [this] (Lixxie& lixxie, Activity activity)
		{
			if (lixxie.priorityForNewAc(activity) > 1)
			{
				lixxie.assignManually(activity);
				m_world.playSound("assign");
			}

			// else
				// TODO: sendPRIVMSG
		};

		switch (found->second)
		{
		case Command::Join:
			if (isAdmin(priv.username) && !second.empty() && isdigit(second[0]))
			{
				const int count = std::stoi(second);

				for (int i = 0; i < count; ++i)
					addLixxie("");
			}

			else // if (!lixxie)
			{
				lixxie = addLixxie(priv.username);

				if (lixxie)
					m_world.playSound("lets_go");
			}
			break;

		case Command::Part:
			if (lixxie)
			{
				removeLixxie(*lixxie);
				m_world.playSound("oblivion");
				lixxie = nullptr;
			}
			break;

		case Command::Walk:     assignActivity(*lixxie, Activity::Walker);     break;
		case Command::Jump:     assignActivity(*lixxie, Activity::Jumper);     break;
		case Command::Run:      assignActivity(*lixxie, Activity::Runner);     break;
		case Command::Climb:    assignActivity(*lixxie, Activity::Climber);    break;
		case Command::Float:    assignActivity(*lixxie, Activity::Floater);    break;
		case Command::Bat:      assignActivity(*lixxie, Activity::Batter);     break;
		case Command::Implode:  assignActivity(*lixxie, Activity::Imploder);   break;
		case Command::Explode:  assignActivity(*lixxie, Activity::Exploder);   break;
		case Command::Block:    assignActivity(*lixxie, Activity::Blocker);    break;
		case Command::Cube:     assignActivity(*lixxie, Activity::Cuber);      break;
		case Command::Build:    assignActivity(*lixxie, Activity::Builder);    break;
		case Command::Platform: assignActivity(*lixxie, Activity::Platformer); break;
		case Command::Bash:     assignActivity(*lixxie, Activity::Basher);     break;
		case Command::Mine:     assignActivity(*lixxie, Activity::Miner);      break;
		case Command::Dig:      assignActivity(*lixxie, Activity::Digger);     break;
		}

		if (lixxie && priv.color != 0)
			lixxie->setColor(sf::Color((priv.color << 8) | 0xff));
	}
}

void LixBot::loadResources()
{
	// load texture
	m_textures = Lixxie::generateTextures();

	// load font
	m_font.loadFromFile("Lix/Fonts/Magero.ttf");
	m_fontSize = 8;
	const_cast<sf::Texture&>(m_font.getTexture(m_fontSize)).setSmooth(false);

	// load sounds
	m_world.loadSounds("Lix/sounds");
}

void LixBot::initializeCommands()
{
	m_commandBinding["!join"]     = Command::Join;
	m_commandBinding["!part"]     = Command::Part;
	m_commandBinding["!walk"]     = Command::Walk;
	m_commandBinding["!jump"]     = Command::Jump;
	m_commandBinding["!run"]      = Command::Run;
	m_commandBinding["!climb"]    = Command::Climb;
	m_commandBinding["!float"]    = Command::Float;
	m_commandBinding["!bat"]      = Command::Bat;
	m_commandBinding["!implode"]  = Command::Implode;
	m_commandBinding["!explode"]  = Command::Explode;
	m_commandBinding["!block"]    = Command::Block;
	m_commandBinding["!cube"]     = Command::Cube;
	m_commandBinding["!build"]    = Command::Build;
	m_commandBinding["!platform"] = Command::Platform;
	m_commandBinding["!bash"]     = Command::Bash;
	m_commandBinding["!mine"]     = Command::Mine;
	m_commandBinding["!dig"]      = Command::Dig;
}

void LixBot::updateNuke()
{
	if (!m_nuking)
		return;

	for (auto& lixxie : m_world.tribe)
	{
		if (!lixxie->healthy() || lixxie->ploderTimer > 0)
			continue;

		lixxie->assignManually(Activity::Exploder);
		break;
	}

	if (m_world.tribe.empty())
		m_nuking = false;
}

void LixBot::updateLixxies()
{
	bool anyFlingers = false;

	// LixD/fuse.d
	auto handlePloderTimer = [] (Lixxie& lixxie)
	{
		assert(lixxie.activity != Activity::Imploder);
		assert(lixxie.activity != Activity::Exploder);

		if (lixxie.ploderTimer == 0)
			return;

		if (lixxie.healthy())
		{
			// if (ow.state.numTribes <= 1 || lixxie.ploderTimer == Lixxie::PloderDelay)
			if (true)
				lixxie.becomePloder();
			else
				lixxie.ploderTimer = lixxie.ploderTimer + 1;
		}

		else
		{
			lixxie.ploderTimer = lixxie.ploderTimer + lixxie.frame + 2;

			if (lixxie.ploderTimer > Lixxie::PloderDelay)
				lixxie.ploderTimer = 0;
		}
	};
	//

	auto performFlingersUnmarkOthers = [&] ()
	{
		for (auto& lixxie : m_world.tribe)
		{
			lixxie->setNoEncountersNoBlockerFlags();

			if (lixxie->ploderTimer != 0)
				handlePloderTimer(*lixxie);

			if (lixxie->updateOrder == PhysicsUpdateOrder::Flinger)
			{
				lixxie->marked = true;
				anyFlingers = true;
				lixxie->perform();
			}

			else
				lixxie->marked = false;
		}
	};

	auto applyFlinging = [&] ()
	{
		if (!anyFlingers)
			return;

		for (auto& lixxie : m_world.tribe)
			lixxie->applyFlingXY();
	};

	auto performUnmarked = [&] (PhysicsUpdateOrder uo)
	{
		for (auto& lixxie : m_world.tribe)
		{
			if (!lixxie->marked && lixxie->updateOrder == uo)
			{
				lixxie->marked = true;
				lixxie->perform();
			}
		}
	};

	performFlingersUnmarkOthers();
	applyFlinging();
	m_world.physicsDrawer.applyChangesToPhymap();

	performUnmarked(PhysicsUpdateOrder::Blocker);
	performUnmarked(PhysicsUpdateOrder::Remover);
	m_world.physicsDrawer.applyChangesToPhymap();

	performUnmarked(PhysicsUpdateOrder::Adder);
	m_world.physicsDrawer.applyChangesToPhymap();

	performUnmarked(PhysicsUpdateOrder::Peaceful);
}

void LixBot::removeDestroyedLixxies()
{
	auto it = std::remove_if(m_world.tribe.begin(), m_world.tribe.end(),
		[] (const auto& l) { return l->activity == Activity::Nothing; });

	m_world.tribe.erase(it, m_world.tribe.end());
}

Lixxie* LixBot::getLixxie(const std::string& id) const
{
	auto it = std::find_if(m_world.tribe.rbegin(), m_world.tribe.rend(),
		[&] (const auto& l) { return l->getId() == id; });

	if (it != m_world.tribe.rend())
		return it->get();

	return nullptr;
}

Lixxie* LixBot::getLixxie(const sf::Vector2f& position) const
{
	auto it = std::find_if(m_world.tribe.rbegin(), m_world.tribe.rend(),
		[&] (const auto& l) { return l->getBoundingRect().contains(position); });

	if (it != m_world.tribe.rend())
		return it->get();

	return nullptr;
}

Lixxie* LixBot::addLixxie(const std::string& id)
{
	Lixxie::Style style = Lixxie::Garden;

	if (randomInt(9) > 0)
		style = static_cast<Lixxie::Style>(randomInt(Lixxie::Red, Lixxie::Black));

	m_world.tribe.emplace_back(std::make_unique<Lixxie>(m_world, style, id));
	Lixxie* lixxie = m_world.tribe.back().get();
	lixxie->setText(id, m_font, m_fontSize);

	lixxie->ex = randomInt(m_mapSize.x);
	lixxie->ey = randomInt(m_mapSize.y);

	return lixxie;
}

void LixBot::removeLixxie(Lixxie& lixxie)
{
	// TODO: 바로 제거하지 않고 게임의 상황을 이용하여 제거

	auto it = std::find_if(m_world.tribe.begin(), m_world.tribe.end(),
		[&] (const auto& l) { return l.get() == &lixxie; });

	assert(it != m_world.tribe.end());
	m_world.tribe.erase(it);
}

void LixBot::removeAllLixxiees(const std::string& id)
{
	auto it = std::remove_if(m_world.tribe.begin(), m_world.tribe.end(),
		[&] (const auto& l) { return l->getId() == id; });

	m_world.tribe.erase(it, m_world.tribe.end());
}

void LixBot::relocate(Lixxie& lixxie)
{
}

void LixBot::relocateLixxies()
{
}

void LixBot::clearLixxies()
{
	m_world.tribe.clear();
}

void LixBot::clearWorld()
{
}
