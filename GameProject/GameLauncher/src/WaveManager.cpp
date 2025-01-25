#include "WaveManager.hpp"

#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include "Player.hpp"
#include "Enemy.hpp"

#include <algorithm>
#include <random>

namespace Lina
{

	WaveManager::WaveManager(Game* game) : m_game(game)
	{
		EntityWorld* world = game->m_world;

		// Find spawns
		world->ViewEntities([&](Entity* e, uint32 index) -> bool {
			if (e->GetName().compare("EnemySpawn") == 0)
			{
				this->m_enemySpawns.push_back(e);
			}
			return false;
		});
    
    SpawnWave();
	}

	WaveManager::~WaveManager()
	{
	}

	void WaveManager::SpawnWave()
	{
		uint32_t waveIndex = m_waveCounter++;
		EntityWorld* world = m_game->m_world;

		EntityTemplate* enemyTemplate = m_game->GetEntityTemplate("Enemy_1");
    EntityTemplate* bossTemplate = m_game->GetEntityTemplate("Enemy_2");

		std::shuffle(m_enemySpawns.begin(), m_enemySpawns.end(), m_game->m_rng);
		//    m_enemySpawns

//		if (enemyTemplate != nullptr)
//		{
//			Entity* spawn = m_enemySpawns[0];
//			Enemy*	enemy = new Enemy(world, enemyTemplate, m_game->m_player, spawn->GetPosition(), spawn->GetRotation());
//			m_enemies.push_back(enemy);
//		}
    
    if (bossTemplate != nullptr)
    {
      Entity* spawn = m_enemySpawns[1];
      Enemy*  enemy = new Enemy(world, bossTemplate, m_game->m_player, spawn->GetPosition(), spawn->GetRotation());
      m_enemies.push_back(enemy);
    }

		m_game->OnEnemyWaveSpawned(waveIndex);
	}

	void WaveManager::Tick(float dt)
	{
		EntityWorld* world = m_game->m_world;
		for (Enemy* enemy : m_enemies)
		{
			enemy->Tick(dt);
		}
	}
} // namespace Lina
