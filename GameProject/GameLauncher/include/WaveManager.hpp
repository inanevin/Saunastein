#pragma once

#include "Common/Math/Vector.hpp"
#include "Common/Math/Quaternion.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Game.hpp"

namespace LinaGX
{

} // namespace LinaGX

namespace Lina
{
	class Game;

	class WaveManager
	{
	public:
		WaveManager(Game* game);

		void PreTick();
		void Tick(float dt);
		~WaveManager();

		void UpdateWaves();
    void HandleContact(Entity* e1, Entity* e2);

		Game*			m_game = nullptr;
		Vector<Enemy*>	m_currentEnemies;
    Vector<Enemy*>  m_deadEnemies;
		Vector<Entity*> m_enemySpawns;
		uint32_t		m_waveCounter				= 0;
		float			m_globalTimer				= 0;
		float			m_currentWaveStartAt		= 0;
		bool			m_currentWaveHasWaited		= 0;
		float			m_currentWaveLastSpawnedAt	= 0;
		uint32_t		m_currentWaveEnemiesSpawned = 0;
    uint32_t        m_entitySpawnerCounter = 0;
	};
} // namespace Lina
