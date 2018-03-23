#include "PhysicsDrawer.hpp"
#include "Map.hpp"
#include "Mask.hpp"
#include "Skill/Cuber.hpp"
#include "Skill/Digger.hpp"

#include <cassert>

PhysicsDrawer::PhysicsDrawer(Map& map)
	: m_map(map)
{
}

void PhysicsDrawer::add(TerrainAddition&& tc)
{
	m_addsForPhymap.emplace_back(std::move(tc));
}

void PhysicsDrawer::add(TerrainDeletion&& tc)
{
	m_delsForPhymap.emplace_back(std::move(tc));
}

void PhysicsDrawer::applyChangesToPhymap()
{
	deletionsToPhymap();
	additionsToPhymap();
}

void PhysicsDrawer::applyChangesToLand(Phyu upd)
{
	assert(m_delsForPhymap.empty() && m_addsForPhymap.empty());

	while (!m_delsForLand.empty() || !m_addsForLand.empty())
	{
		deletionsToLand();
		additionsToLand();
	}
}

sf::IntRect PhysicsDrawer::additionsDefs(const TerrainAddition& tc) const
{
	const bool build = (tc.type == TerrainAddition::Build);
	const bool plaLo = (tc.type == TerrainAddition::PlatformLong);
	const bool plaSh = (tc.type == TerrainAddition::PlatformShort);

	sf::IntRect rect;
	rect.height = (build || plaLo || plaSh) ? BrickYl
											: tc.cubeYl;
	rect.top = build ? 0
			 : plaLo ? 1 * BrickYl
			 : plaSh ? 2 * BrickYl
			 : CubeY + Cuber::CubeSize - rect.height;
	rect.width = build ? BuilderBrickXl
			   : plaLo ? PlatformLongXl
			   : plaSh ? PlatformShortXl
			   : Cuber::CubeSize;
	rect.left = rect.width * tc.style;

	return rect;
}

bool PhysicsDrawer::diggerAntiRazorsEdge(const TerrainDeletion& tc) const
{
	assert(tc.type == TerrainDeletion::Dig);

	bool result = false;
	sf::Vector2i p;

	for (p.y = 0; p.y < tc.digYl; ++p.y)
	{
		constexpr int half = Digger::TunnelWidth / 2;
		const sf::Vector2i loc(tc.x, tc.y);

		for (p.x = half - 1; p.x >= 0; --p.x)
		{
			if (m_map.setAirCountSteel(loc + p))
			{
				result = true;
				break;
			}
		}

		for (p.x = half; p.x < 2 * half; ++p.x)
		{
			if (m_map.setAirCountSteel(loc + p))
			{
				result = true;
				break;
			}
		}
	}

	return result;
}

bool PhysicsDrawer::diggerAntiRazorsEdge(const FlaggedDeletion& fc) const
{
	assert(fc.tc.type == TerrainDeletion::Dig);

	bool result = false;
	sf::Vector2i p;

	for (p.y = 0; p.y < fc.tc.digYl; ++p.y)
	{
		constexpr int half = Digger::TunnelWidth / 2;
		const sf::Vector2i loc(fc.tc.x, fc.tc.y);

		for (p.x = half - 1; p.x >= 0; --p.x)
		{
			if (m_map.getSteel(loc + p))
				break;

			const sf::Vector2i wrapped = m_map.wrap(loc + p);
			// TODO: al_draw_pixel(wrapped.x + 0.5f, wrapped.y + 0.5f, color.white);
		}

		for (p.x = half; p.x < 2 * half; ++p.x)
		{
			if (m_map.getSteel(loc + p))
				break;

			const sf::Vector2i wrapped = m_map.wrap(loc + p);
			// TODO: al_draw_pixel(wrapped.x + 0.5f, wrapped.y + 0.5f, color.white);
		}
	}

	return result;
}

void PhysicsDrawer::additionsToPhymap()
{
	for (auto& tc : m_addsForPhymap)
	{
		const sf::IntRect rect = additionsDefs(tc);
		assert(rect.height > 0);

		FlaggedAddition fc = { std::move(tc) };

		for (int y = 0; y < rect.height; ++y)
			for (int x = 0; x < rect.width; ++x)
			{
				const sf::Vector2i target(fc.tc.x + x, fc.tc.y + y);

				if (m_map.getSolid(target))
					fc.drawPerPixelDueToExistingTerrain = true;

				else
				{
					m_map.add(target, Map::Terrain);
					fc.needsColoring[y][x] = true;
				}
			}

		m_addsForLand.emplace_back(std::move(fc));
	}

	m_addsForPhymap.clear();
}

void PhysicsDrawer::deletionsToPhymap()
{
	for (auto& tc : m_delsForPhymap)
	{
		int steelHit = 0;

		if (tc.type == TerrainDeletion::Dig)
			steelHit = diggerAntiRazorsEdge(tc);
		else
			steelHit += m_map.setAirCountSteelEvenWhereMaskIgnores(
				{ tc.x, tc.y }, Mask::get(tc.type));

		FlaggedDeletion fc = { std::move(tc) };
		fc.drawPerPixelDueToSteel = steelHit > 0;
		m_delsForLand.emplace_back(std::move(fc));
	}

	m_delsForPhymap.clear();
}

void PhysicsDrawer::additionsToLand()
{
	if (m_addsForLand.empty())
		return;

	if (!m_delsForLand.empty() && m_delsForLand[0].tc.update < m_addsForLand[0].tc.update)
		return;

	const Phyu upd = m_addsForLand[0].tc.update;

	while (!m_addsForLand.empty() && m_addsForLand[0].tc.update == upd)
	{
		const FlaggedAddition& fc = m_addsForLand[0];
		const sf::IntRect rect = additionsDefs(fc.tc);

		// TODO: const bool allAtOnce = !fc.drawPerPixelDueToExistingTerrain;

		m_map.draw({ fc.tc.x, fc.tc.y }, rect);

		m_addsForLand.pop_front();
	}
}

void PhysicsDrawer::deletionsToLand()
{
	if (m_delsForLand.empty())
		return;

	if (!m_addsForLand.empty() && m_addsForLand[0].tc.update < m_delsForLand[0].tc.update)
		return;

	const Phyu upd = m_delsForLand[0].tc.update;

	while (!m_delsForLand.empty() && m_delsForLand[0].tc.update == upd)
	{
		const FlaggedDeletion& fc = m_delsForLand[0];

		if (fc.tc.type != TerrainDeletion::Dig)
		{
			// LIXBOT:
			m_map.draw({ fc.tc.x, fc.tc.y }, fc.tc.type);
		}

		else if (fc.drawPerPixelDueToSteel)
			diggerAntiRazorsEdge(fc);

		else
		{
			assert(fc.tc.type == TerrainDeletion::Dig);
			assert(fc.tc.digYl > 0);

			// LIXBOT:
			m_map.draw({ fc.tc.x, fc.tc.y }, fc.tc.type, fc.tc.digYl);
		}

		m_delsForLand.pop_front();
	}
}
