#include "Enemy.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include "Player.hpp"

namespace Lina
{
	Enemy::Enemy(EntityWorld* w, EntityTemplate* et, Player* p, Vector3 position, Quaternion rotation) : m_world(w), m_target(p)
	{
		m_entity = w->SpawnTemplate(et, position, rotation);

		JPH::Body*			body = m_entity->GetPhysicsBody();
		JPH::MassProperties mp;
		mp.mMass = m_entity->GetPhysicsSettings().mass;

		body->SetFriction(0.1f);
		body->SetRestitution(0.1f);
		body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ | JPH::EAllowedDOFs::RotationY, mp);
		m_entity->GetPhysicsBody()->SetAllowSleeping(false);

		for (Entity* child : m_entity->GetChildren())
		{
			if (child->GetName().contains("Sprite"))
			{
				m_sprites.push_back(child);
			}
		}

		LINA_INFO("Got {0} sprites", m_sprites.size());
	}

	Enemy::~Enemy()
	{
    m_world->DestroyEntity(m_entity);
	}
  
  bool Enemy::IsAlive() {
//    return true;
    return !m_dead;
//    return m_timer < 20.0f;
  }
  
void Enemy::TakeDamage(int amount) {
  m_health -= amount;
  m_hitFrameTime = 0.5f;
}
	
  void Enemy::Tick(float dt)
	{
		m_timer += dt;
    m_hitFrameTime -= dt;
    
    if (m_health <= 0) m_dead = true;
    
		JPH::Body* physicsBody		   = m_entity->GetPhysicsBody();
		float	   currentPhysicsSpeed = physicsBody->GetLinearVelocity().Length();
		float	   animFrameTime	   = 0.1f + (0.5f / currentPhysicsSpeed);

		int animFrame = ((int)(m_timer / animFrameTime) % 2);
    if (m_dead) animFrame = 2;
    else if (m_hitFrameTime > 0.0f) {
      animFrame = 3;
    }
    
		m_currentSpriteIdx = animFrame;

    Vector3 selfPosition  = m_entity->GetPosition();
    selfPosition.y      = 0.0f;
    Vector3 targetPosition  = m_target->m_entity->GetPosition();
    targetPosition.y    = 0.0f;
    
    if (!m_dead) {
      float  forceFactor    = Math::Clamp(1.0f / currentPhysicsSpeed, 0.0f, 1.0f);
      float  force      = 8.0f * forceFactor;

      Vector3 targetDirection = (targetPosition - selfPosition).Normalized();
      physicsBody->AddForce(ToJoltVec3(targetDirection * force));
    }
		
		for (uint32_t spriteIdx = 0; spriteIdx < m_sprites.size(); ++spriteIdx)
		{
			bool currentSprite = spriteIdx == m_currentSpriteIdx;
			m_sprites[spriteIdx]->SetVisible(currentSprite);

			if (currentSprite)
			{
				m_sprites[spriteIdx]->SetRotation(Quaternion::LookAt(selfPosition, targetPosition, Vector3::Up));
			}
		}
	}
} // namespace Lina
