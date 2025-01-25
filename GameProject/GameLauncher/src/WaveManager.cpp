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
	struct WaveEnemyDesc
	{
		String	resourceKey;
		int32_t health;
		int32_t speed;
		String	projectileKey;
		float	waitTimeBefore;
	};

	struct WaveDesc
	{
		float				  waitTimeBefore;
		String				  name;
		Vector<WaveEnemyDesc> enemies;
	};

	static Vector<WaveDesc> waves = {
		WaveDesc{
			.name			= "Get ready",
			.waitTimeBefore = 1.0f,
		},
		WaveDesc{.waitTimeBefore = 3.0f,
				 .name			 = "Wave 1",
				 .enemies =
					 {
             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.0f},
					 }},
		WaveDesc{
			.waitTimeBefore = 0.5f,
			.name			= "Well done!",
		},
		WaveDesc{.waitTimeBefore = 3.0f,
				 .name			 = "Wave 2",
				 .enemies =
					 {
             WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
						 WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
//             WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 100, .speed = 5, .waitTimeBefore = 0.1f},
					 }},
	};

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
	}

	WaveManager::~WaveManager()
	{
	}

	void WaveManager::UpdateWaves()
	{
		EntityWorld* world = m_game->m_world;

		if (m_waveCounter >= waves.size())
			return;
		uint32_t currentWaveIdx = m_waveCounter;

		float currentWaveTime = m_globalTimer - m_currentWaveStartAt;

		WaveDesc& currentWave = waves[currentWaveIdx];

		bool currentWaveAllSpawned = m_currentWaveEnemiesSpawned >= currentWave.enemies.size();
		bool currentWaveHasActiveEnemies = m_enemies.size() > 0;
		bool currentWaveIsWaiting = currentWaveTime < currentWave.waitTimeBefore;
		bool currentWaveIsDone = !currentWaveIsWaiting && currentWaveAllSpawned && !currentWaveHasActiveEnemies;

		if (!currentWaveIsWaiting && !m_currentWaveHasWaited)
		{
			LINA_INFO("[{0}] Start Wave {1}", m_globalTimer, m_waveCounter);
			m_currentWaveHasWaited	   = true;
			m_currentWaveLastSpawnedAt = m_globalTimer;
      m_game->OnWaveSpawned(m_waveCounter, currentWave.name);
			return;
		}

		// next wave when current wave is done
		if (currentWaveIsDone)
		{
			LINA_INFO("[{0}] End Wave {1}", m_globalTimer, m_waveCounter);
			m_waveCounter++;

			std::shuffle(m_enemySpawns.begin(), m_enemySpawns.end(), m_game->m_rng);
			m_currentWaveEnemiesSpawned = 0;
			m_currentWaveStartAt		= m_globalTimer;
			m_currentWaveLastSpawnedAt	= m_globalTimer;
			m_currentWaveHasWaited		= false;

			return;
		}

		if (currentWaveIsWaiting)
		{
			// Waiting...
		}
		else if (!currentWaveAllSpawned)
		{
			// Spawning...
			if (!currentWaveAllSpawned)
			{
				uint32_t	   nextEnemyIdx	  = m_currentWaveEnemiesSpawned;
				WaveEnemyDesc& enemyDesc	  = currentWave.enemies[nextEnemyIdx];
				float		   sinceLastSpawn = m_globalTimer - m_currentWaveLastSpawnedAt;

				if (sinceLastSpawn >= enemyDesc.waitTimeBefore)
				{
					// Span enemy
					LINA_INFO("[{0}] Spawn {1} from wave {2} (sinceLastSpawn: {3})", m_globalTimer, m_currentWaveEnemiesSpawned, m_waveCounter, sinceLastSpawn);

					m_currentWaveEnemiesSpawned++;
					m_currentWaveLastSpawnedAt = m_globalTimer;
          EntityTemplate* templ = m_game->GetEntityTemplate(enemyDesc.resourceKey);
          if (templ != nullptr)
          {
            Entity* spawn = m_enemySpawns[m_entitySpawnerCounter++%m_enemySpawns.size()];
            Enemy* enemy = new Enemy(world, templ, m_game->m_player, spawn->GetPosition(), spawn->GetRotation());
            m_enemies.push_back(enemy);
            m_game->OnEnemySpawned(enemy);
          }
				}
			}
		}
		else
		{
			// Everything spawned.. waiting for enemies to be cleared...
		}
	}

	void WaveManager::PreTick()
	{
    UpdateWaves();
	}

	void WaveManager::Tick(float dt)
	{
		m_globalTimer += dt;
		EntityWorld* world = m_game->m_world;
    
    Vector<Enemy*> deadEnemies;
		for (Enemy* enemy : m_enemies)
		{
			enemy->Tick(dt);
      if (!enemy->IsAlive()) {
        deadEnemies.push_back(enemy);
      }
		}
    
    uint32_t enemyCount = m_enemies.size();
    
    for (Enemy* deadEnemy : deadEnemies) {
      delete deadEnemy;
      m_enemies.erase(std::remove(m_enemies.begin(), m_enemies.end(), deadEnemy), m_enemies.end());
    }
    
    if (deadEnemies.size() > 0) {
      LINA_INFO("Destroyed {0} enemies. Had: {1}, Has: {2}", deadEnemies.size(), enemyCount, m_enemies.size());
    }
	}
} // namespace Lina
