#include "WaveManager.hpp"
#include "AudioManager.hpp"

#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include "Player.hpp"
#include "Enemy.hpp"
#include "BubbleManager.hpp"
#include "Common/System/SystemInfo.hpp"

#include <algorithm>
#include <random>

namespace Lina
{
	struct WaveEnemyDesc
	{
		String	resourceKey;
		int32_t health;
		int32_t speed;
		int32_t score;
		String	projectileKey;
		float	waitTimeBefore;
	};

	struct WaveDesc
	{
		float				  waitTimeBefore;
		float				  heatAddition = 0.0f;
		String				  name;
		Vector<WaveEnemyDesc> enemies;
	};

	static Vector<WaveDesc> waves = {
		WaveDesc{
			.name			= "Get ready",
			.waitTimeBefore = 2.0f,
		},

		WaveDesc{.name			 = "Wave 1",
				 .waitTimeBefore = 2.0f,
				 .heatAddition	 = 1.5f,
				 .enemies =
					 {
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
					 }},
		WaveDesc{.name			 = "Wave 2",
				 .waitTimeBefore = 3.0f,
				 .heatAddition	 = 2.5f,
				 .enemies =
					 {
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
						 WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 6, .speed = 5, .waitTimeBefore = 0.1f, .score = 300},
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
					 }},

		WaveDesc{.name			 = "Wave 3",
				 .waitTimeBefore = 3.0f,
				 .heatAddition	 = 3.5f,
				 .enemies =
					 {
						 WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 6, .speed = 5, .waitTimeBefore = 0.1f, .score = 300},
						 WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 6, .speed = 5, .waitTimeBefore = 0.1f, .score = 300},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
					 }},
		WaveDesc{.name			 = "Wave 4",
				 .waitTimeBefore = 3.0f,
				 .heatAddition	 = 5.5f,
				 .enemies =
					 {
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
						 WaveEnemyDesc{.resourceKey = "Enemy_1", .health = 3, .speed = 5, .waitTimeBefore = 0.1f, .score = 100},
						 WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 6, .speed = 5, .waitTimeBefore = 0.1f, .score = 300},
						 WaveEnemyDesc{.resourceKey = "Enemy_2", .health = 6, .speed = 5, .waitTimeBefore = 0.1f, .score = 300},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
					 }},
		WaveDesc{
			.waitTimeBefore = 0.5f,
			.name			= "Well done!",
		},

		WaveDesc{.waitTimeBefore = 10.0f,
				 .name			 = "FINAL WAVE",
				 .heatAddition	 = 50.0f,
				 .enemies =
					 {
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
						 WaveEnemyDesc{.resourceKey = "Enemy_3", .health = 10, .speed = 5, .waitTimeBefore = 0.1f, .score = 500},
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
		for (Enemy* enemy : m_deadEnemies)
		{
			delete enemy;
		}

		for (Enemy* enemy : m_currentEnemies)
		{
			delete enemy;
		}
	}

	void WaveManager::UpdateWaves()
	{
		EntityWorld* world = m_game->m_world;

		if (m_waveCounter >= waves.size())
			return;
		uint32_t currentWaveIdx = m_waveCounter;

		float currentWaveTime = m_globalTimer - m_currentWaveStartAt;

		WaveDesc& currentWave = waves[currentWaveIdx];

		bool currentWaveAllSpawned		 = m_currentWaveEnemiesSpawned >= currentWave.enemies.size();
		bool currentWaveHasActiveEnemies = m_currentEnemies.size() > 0;
		bool currentWaveIsWaiting		 = currentWaveTime < currentWave.waitTimeBefore;
		bool currentWaveIsDone			 = !currentWaveIsWaiting && currentWaveAllSpawned && !currentWaveHasActiveEnemies;

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
			m_game->SetHeat(0.0f);
			m_game->m_audioManager->OnResetHeat();

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
					EntityTemplate* templ	   = m_game->GetEntityTemplate(enemyDesc.resourceKey);
					if (templ != nullptr)
					{
						Entity* spawn	= m_enemySpawns[m_entitySpawnerCounter++ % m_enemySpawns.size()];
						Enemy*	enemy	= new Enemy(world, templ, m_game->m_player, spawn->GetPosition(), spawn->GetRotation());
						enemy->m_health = enemyDesc.health;
						enemy->m_score	= enemyDesc.score;

						m_currentEnemies.push_back(enemy);
						m_game->OnEnemySpawned(enemy);
					}
				}
			}
		}
		else
		{
			// Everything spawned.. waiting for enemies to be cleared...
			m_game->UpdateHeat(currentWave.heatAddition * static_cast<float>(SystemInfo::GetDeltaTime()));
		}

		// Update Fire
	}

	void WaveManager::PreTick()
	{
		UpdateWaves();
	}

  static void HandleEnemyFrying(WaveManager* wm, Enemy* enemy) {
    if (!enemy->m_dead) return;
    enemy->m_isFried = true;
    wm->m_game->AddScore(100);
//    LINA_INFO("HandleEnemyFrying {0} isDead {1}", enemy->m_entity->GetName(), enemy->IsAlive());
  }

	static void HandleEnemyDamage(WaveManager* wm, BubbleManager::BubbleData* bubble, Enemy* enemy)
	{
		if (!enemy->m_dead && enemy->m_hitFrameTime <= 0.00001f)
		{
			enemy->TakeDamage(1);
			LINA_INFO("ENEMY TAKE HIT! Has {0}", enemy->m_health);
		}

		wm->m_game->m_audioManager->Play(wm->m_game->m_audioManager->m_humpfh, 0.25f);
		wm->m_game->m_bubbleManager->KillBubble(bubble->_entity);
	}

	static void HandlePlayerDamage(WaveManager* wm, Player* player, Enemy* enemy)
	{
		if (enemy->m_dead)
			return;

		if (player->m_invincibilityTimer > 0.0f)
			return;

		player->UpdateHealth(-5.0f);
		LINA_INFO("PLAYER TAKE HIT! Has {0}", player->m_health);
	}

	void WaveManager::HandleContact(Entity* e1, Entity* e2)
	{
		Player* player = m_game->m_player;

		Enemy* enemy1 = nullptr;
		Enemy* enemy2 = nullptr;

		Player* player1 = nullptr;
		Player* player2 = nullptr;

		BubbleManager::BubbleData* bubble1 = nullptr;
		BubbleManager::BubbleData* bubble2 = nullptr;
  
      
		if (e1 == player->m_entity)
			player1 = player;
		if (e2 == player->m_entity)
			player2 = player;

		for (Enemy* enemy : m_currentEnemies)
		{
			if (enemy->m_entity == e1)
				enemy1 = enemy;
			if (enemy->m_entity == e2)
				enemy2 = enemy;
		}

    for (Enemy* enemy : m_deadEnemies)
    {
      if (enemy->m_entity == e1)
        enemy1 = enemy;
      if (enemy->m_entity == e2)
        enemy2 = enemy;
    }
    
		for (uint32_t i = 0; i < m_game->m_bubbleManager->m_bubbles.size(); ++i)
		{
			auto* bubble = &m_game->m_bubbleManager->m_bubbles[i];
			if (bubble->_entity == e1)
				bubble1 = bubble;
			if (bubble->_entity == e2)
				bubble2 = bubble;
		}

		if (enemy1 != nullptr && bubble2 != nullptr)
			HandleEnemyDamage(this, bubble2, enemy1);
		if (enemy2 != nullptr && bubble1 != nullptr)
			HandleEnemyDamage(this, bubble1, enemy2);

		if (enemy1 != nullptr && player2 != nullptr)
			HandlePlayerDamage(this, player2, enemy1);

		if (enemy2 != nullptr && player1 != nullptr)
			HandlePlayerDamage(this, player1, enemy2);
    
    if (enemy1 != nullptr && e2->GetName().contains("Fire")) {
      HandleEnemyFrying(this, enemy1);
    }
    
    if (enemy2 != nullptr && e1->GetName().contains("Fire")) {
      HandleEnemyFrying(this, enemy2);
    }
	}

	void WaveManager::Tick(float dt)
	{
		m_globalTimer += dt;
		EntityWorld* world = m_game->m_world;

    Vector<Enemy*> fried;
		for (Enemy* enemy : m_currentEnemies)
		{
			enemy->Tick(dt);

			if (!enemy->IsAlive())
			{
				m_game->AddScore(enemy->m_score);
				m_deadEnemies.push_back(enemy);
			}
		}
    
		for (Enemy* deadEnemy : m_deadEnemies)
		{
			deadEnemy->Tick(dt);
			m_currentEnemies.erase(std::remove(m_currentEnemies.begin(), m_currentEnemies.end(), deadEnemy), m_currentEnemies.end());
      
      if (deadEnemy->m_isFried) {
        fried.push_back(deadEnemy);
      }
		}
    
    for (Enemy* friedEnemy: fried) {
      m_deadEnemies.erase(std::remove(m_deadEnemies.begin(), m_deadEnemies.end(), friedEnemy), m_deadEnemies.end());
      delete friedEnemy;
    }
	}
} // namespace Lina
