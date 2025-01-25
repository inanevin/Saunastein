#pragma once

#include "Common/Math/Vector.hpp"
#include "Common/Math/Quaternion.hpp"
#include "Core/World/EntityWorld.hpp"


namespace LinaGX
{

} // namespace LinaGX

namespace Lina
{
	class EntityWorld;
	class Entity;
  class Player;
  
	class Enemy
	{
	public:
		Enemy(EntityWorld* ew, EntityTemplate* et, Player* p, Vector3 position, Quaternion rotation);

		void Tick(float dt);
		~Enemy();
    
//	private:
    EntityWorld* m_world = nullptr;
		Entity* m_entity = nullptr;
    Player* m_target = nullptr;
    Entity* m_sprite = nullptr;
    float m_timer = 0;
    float m_origSpriteScale = 1;
	};
} // namespace Lina
