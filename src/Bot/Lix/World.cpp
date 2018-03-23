#include "World.hpp"
#include "Mask.hpp"
#include "../../Utility.hpp"

#include <filesystem>
#include <algorithm>
#include <cassert>

namespace fs = std::experimental::filesystem;

World::World(const sf::Vector2i& mapSize)
	: m_map(mapSize.x, mapSize.y)
	, m_physicsDrawer(m_map)
{
	// HACK: effect textures
	sf::Image debris;
	debris.loadFromFile("Lix/Images/debris.I.png");
	debris.createMaskFromColor(sf::Color(255, 0, 255));
	m_debrisTexture.loadFromImage(debris);

	sf::Image implode;
	implode.loadFromFile("Lix/Images/implode.I.png");
	implode.createMaskFromColor(sf::Color(255, 0, 255));
	m_implosion.loadFromImage(implode);

	sf::Image explode;
	explode.loadFromFile("Lix/Images/explode.I.png");
	explode.createMaskFromColor(sf::Color(255, 0, 255));
	m_explosion.loadFromImage(explode);
	//
}

Phyu World::getUpdateCount() const
{
	return m_update;
}

void World::setUpdateCount(Phyu count)
{
	m_update = count;
}

Map& World::getLookup()
{
	return m_map;
}

const Map& World::getLookup() const
{
	return m_map;
}

PhysicsDrawer& World::getPhysicsDrawer()
{
	return m_physicsDrawer;
}

const PhysicsDrawer& World::getPhysicsDrawer() const
{
	return m_physicsDrawer;
}

World::Tribe& World::getTribe()
{
	return m_tribe;
}

const World::Tribe& World::getTribe() const
{
	return m_tribe;
}

std::vector<Debris>& World::getDebris()
{
	return m_debris;
}

const std::vector<Debris>& World::getDebris() const
{
	return m_debris;
}

void World::applyChangesToLand()
{
	m_physicsDrawer.applyChangesToLand(m_update);
}

void World::addPickaxe(int ex, int ey, int dir)
{
	auto uniform = [] (int min, int max)
	{
		return randomInt(min, max - 1);
	};

	sf::Sprite sprite(m_debrisTexture);
	const float x = static_cast<float>(ex + 10 * dir);
	const float y = static_cast<float>(ey);
	sprite.setPosition(x, y);
	const float rotCw = dir < 0 /* && toolFrame == 0 */ ? 1.f : 0.f;
	m_debris.push_back({ Debris::FlyingTool, std::move(sprite), uniform(40, 90), 0,
		uniform(1, 6) * dir, uniform(-11, -7), rotCw });
	playSound("steel");
}

void World::addDigHammer(int ex, int ey, int dir)
{
	playSound("steel");
}

void World::addImplosion(int ex, int ey)
{
	sf::Sprite sprite(m_implosion);
	const int frameSize = sprite.getTexture()->getSize().y - 2;
	const float x = static_cast<float>(ex - frameSize / 2);
	const float y = static_cast<float>(ey - frameSize / 2 + Mask::ExplodeMaskOffsetY);
	sprite.setPosition(x, y);
	m_debris.push_back({ Debris::Implosion, std::move(sprite), 11, -1 });
	playSound("pop");
}

void World::addExplosion(int ex, int ey)
{
	sf::Sprite sprite(m_explosion);
	const int frameSize = sprite.getTexture()->getSize().y - 2;
	const float x = static_cast<float>(ex - frameSize / 2);
	const float y = static_cast<float>(ey - frameSize / 2 + Mask::ExplodeMaskOffsetY);
	sprite.setPosition(x, y);
	m_debris.push_back({ Debris::Explosion, std::move(sprite), 8 + 2, -1 });
	playSound("pop");
}

void World::updateDebris()
{
	for (auto& debris : m_debris)
	{
		++debris.currentFrame;

		// HACK: pickaxe
		if (debris.type == Debris::FlyingTool)
		{
			// moveThenAccelerateByGravity()
			debris.sprite.move(static_cast<float>(debris.speedX), static_cast<float>(debris.speedY));

			if (debris.currentFrame % 2 == 0)
				debris.speedY += 1;
			//

			// calcFlyingTool()
			debris.rotCw = std::fmodf(4 + debris.rotCw + debris.speedX * 0.03125f, 4.f);
			//

			const int frameSize = 15;
			const int maxFrame = 5;

			const int tu = std::clamp(debris.currentFrame / 4, 0, maxFrame - 1);
			const int tv = 0;

			debris.sprite.setTextureRect({ tu * (frameSize + 1) + 1, tv * (frameSize + 1) + 1, frameSize, frameSize });
			debris.sprite.setRotation(debris.rotCw);

			continue;
		}
		//

		const int frameSize = debris.sprite.getTexture()->getSize().y - 2;
		const int maxFrame = debris.sprite.getTexture()->getSize().x / (frameSize + 1);

		const int tu = std::clamp(debris.currentFrame, 0, maxFrame - 1);
		const int tv = 0;

		debris.sprite.setTextureRect({ tu * (frameSize + 1) + 1, tv * (frameSize + 1) + 1, frameSize, frameSize });
	}

	auto it = std::remove_if(m_debris.begin(), m_debris.end(),
		[] (const auto& d) { return d.currentFrame >= d.numFrames; });

	m_debris.erase(it, m_debris.end());
}

void World::loadSounds(const std::string& directory)
{
	for (const auto& entry : fs::directory_iterator(directory))
	{
		if (!fs::is_regular_file(entry) || entry.path().extension() != ".ogg")
			continue;

		auto sound = std::make_unique<sf::SoundBuffer>();
		sound->loadFromFile(entry.path().string());

		std::string id = entry.path().stem().string();
		toLower(id);

		const auto inserted = m_soundBuffers.emplace(id, std::move(sound));
		assert(inserted.second);
	}
}

void World::playSound(const std::string& id)
{
	const auto found = m_soundBuffers.find(id);
	assert(found != m_soundBuffers.end());

	m_sounds.emplace_back(*found->second);
	m_sounds.back().setVolume(m_volume);
	m_sounds.back().play();
}

void World::removeStoppedSounds()
{
	m_sounds.remove_if([] (const auto& s) { return s.getStatus() == sf::Sound::Stopped; });
}
