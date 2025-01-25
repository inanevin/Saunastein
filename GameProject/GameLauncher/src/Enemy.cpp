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
	}

	Enemy::~Enemy()
	{
	}

	void Enemy::Tick(float dt)
	{
		m_timer += dt;
		int animFrame = ((int)(m_timer / 0.2f) % 2);

		m_currentSpriteIdx = animFrame;

		const float speed		 = 5.0f;
		Vector3		selfPosition = m_entity->GetPosition();
		selfPosition.y			 = 0.0f;
		Vector3 targetPosition	 = m_target->m_entity->GetPosition();
		targetPosition.y		 = 0.0f;
		Vector3 targetDirection	 = (targetPosition - selfPosition).Normalized();

		m_entity->GetPhysicsBody()->SetLinearVelocity(ToJoltVec3(targetDirection * speed));

		for (uint32_t spriteIdx = 0; spriteIdx < m_sprites.size(); ++spriteIdx)
		{
			bool currentSprite = spriteIdx == m_currentSpriteIdx;
			m_sprites[spriteIdx]->SetVisible(currentSprite);
			if (currentSprite)
			{
				m_sprites[spriteIdx]->SetRotation(Quaternion::LookAt(selfPosition, targetPosition, Vector3::Up));
			}
		}

		//    LINA_INFO("spriteFlipScale: {0}", spriteFlipScale);
		//    Vector3 spriteScale = m_sprite->GetLocalScale();
		//    spriteScale.x = m_origSpriteScale * spriteFlipScale;
		//    m_sprite->SetLocalScale(spriteScale);
	}
} // namespace Lina
