#pragma once

#include "TerrainChange.hpp"
#include "Activity.hpp" // HACK: BrickYl

#include <SFML/Graphics/Rect.hpp>

#include <vector>
#include <deque>

class Map;

class PhysicsDrawer
{
public:
	explicit PhysicsDrawer(Map& map);

	void add(TerrainAddition&& tc);
	void add(TerrainDeletion&& tc);

	void applyChangesToPhymap();
	void applyChangesToLand(Phyu upd);

private:
	sf::IntRect additionsDefs(const TerrainAddition& tc) const;

	bool diggerAntiRazorsEdge(const TerrainDeletion& tc) const;
	bool diggerAntiRazorsEdge(const FlaggedDeletion& fc) const;

	void deletionsToPhymap();
	void additionsToPhymap();

	void deletionsToLand();
	void additionsToLand();

public:
	static constexpr int CubeY = 3 * BrickYl;

private:
	Map& m_map; // LIXBOT: Phymap + Torbit(Land)

	std::vector<TerrainAddition> m_addsForPhymap;
	std::vector<TerrainDeletion> m_delsForPhymap;

	std::deque<FlaggedAddition> m_addsForLand;
	std::deque<FlaggedDeletion> m_delsForLand;
};
