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
    bool IsAlive();
		~Enemy();

		//	private:
		EntityWorld*	m_world			   = nullptr;
		Entity*			m_entity		   = nullptr;
		Player*			m_target		   = nullptr;
		uint32_t		m_currentSpriteIdx = 0;
		Vector<Entity*> m_sprites;
		float			m_timer	 = 0.0f;
		uint32_t		m_health = 0;
    bool        m_dead = false;
	};
} // namespace Lina
