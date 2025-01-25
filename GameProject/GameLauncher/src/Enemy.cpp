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
  Enemy::Enemy(EntityWorld* w, EntityTemplate* et, Player* p) : m_world(w), m_target(p)
	{
    m_entity = w->SpawnTemplate(et);
    
    JPH::Body* body = m_entity->GetPhysicsBody();
    JPH::MassProperties mp;
    mp.mMass = m_entity->GetPhysicsSettings().mass;

    body->SetFriction(0.1f);
    body->SetRestitution(0.1f);

    body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ | JPH::EAllowedDOFs::RotationY, mp);
    m_entity->GetPhysicsBody()->SetAllowSleeping(false);
    
    for (Entity* child : m_entity->GetChildren()) {
      if (child->GetName().compare("Quad") == 0) {
        m_sprite = child;
        break;
      }
    }
    
	}

	Enemy::~Enemy()
	{
	}

	void Enemy::Tick(float dt)
	{
    const float speed = 5.0f;
    Vector3 selfPosition = m_entity->GetPosition();
    Vector3 targetPosition = m_target->m_entity->GetPosition();
    Vector3 targetDirection = (targetPosition - selfPosition).Normalized();
    
    m_entity->GetPhysicsBody()->SetLinearVelocity(ToJoltVec3(targetDirection * speed));
    
    m_sprite->SetRotation(Quaternion::LookAt(selfPosition, targetPosition, Vector3::Up));
	}
} // namespace Lina
