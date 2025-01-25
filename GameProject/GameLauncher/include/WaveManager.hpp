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

		void Tick(float dt);
		~WaveManager();
    
    void SpawnWave();
    
    Game* m_game = nullptr;
    Vector<Enemy*> m_enemies;
    Vector<Entity*> m_enemySpawns;
    uint32_t m_waveCounter = 0;
	};
} // namespace Lina
