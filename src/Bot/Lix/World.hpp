#pragma once

#include "Map.hpp"
#include "PhysicsDrawer.hpp"
#include "Lixxie.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>

#include <unordered_map>
#include <memory>

struct Debris
{
	enum Type
	{
		Arrow,
		FlyingTool,
		Implosion,
		Explosion,
		Particle,
	};

	Type type;
	sf::Sprite sprite;
	int numFrames = 0;
	int currentFrame = 0;
	int speedX = 0;
	int speedY = 0;
	float rotCw = 0.f;
};

class World
{
public:
	using Tribe = std::vector<std::unique_ptr<Lixxie>>;

public:
	explicit World(const sf::Vector2i& mapSize);

	World(const World&) = delete;
	World& operator=(const World&) = delete;

	Phyu getUpdateCount() const;
	void setUpdateCount(Phyu count);

	Map& getLookup();
	const Map& getLookup() const;

	PhysicsDrawer& getPhysicsDrawer();
	const PhysicsDrawer& getPhysicsDrawer() const;

	Tribe& getTribe();
	const Tribe& getTribe() const;

	std::vector<Debris>& getDebris();
	const std::vector<Debris>& getDebris() const;

	void applyChangesToLand();

	// effects
	void addPickaxe(int ex, int ey, int dir);
	void addDigHammer(int ex, int ey, int dir);
	void addImplosion(int ex, int ey);
	void addExplosion(int ex, int ey);

	void updateDebris();

	void loadSounds(const std::string& directory);
	void playSound(const std::string& id);
	void removeStoppedSounds();

private:
	std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
	std::list<sf::Sound> m_sounds;
	float m_volume = 75.f;

	Phyu m_update = 0;
	Map m_map;
	PhysicsDrawer m_physicsDrawer;
	Tribe m_tribe;
	std::vector<Debris> m_debris;

	sf::Texture m_debrisTexture;
	sf::Texture m_implosion;
	sf::Texture m_explosion;

public:
	__declspec(property(get = getUpdateCount, put = setUpdateCount)) Phyu update;
	__declspec(property(get = getLookup)) Map& lookup;
	__declspec(property(get = getPhysicsDrawer)) PhysicsDrawer& physicsDrawer;
	__declspec(property(get = getTribe)) Tribe& tribe;
	__declspec(property(get = getDebris)) std::vector<Debris>& debris;
};
