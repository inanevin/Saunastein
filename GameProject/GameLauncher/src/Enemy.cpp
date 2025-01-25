#include "Enemy.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

namespace Lina
{
  Enemy::Enemy(EntityWorld* w, EntityTemplate* et, Player* p) : m_world(w), m_target(p)
	{
    m_entity = w->SpawnTemplate(et);
	}

	Enemy::~Enemy()
	{
	}

	void Enemy::Tick(float dt)
	{
  
	}
} // namespace Lina
