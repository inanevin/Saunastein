/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Weapon.hpp"
#include "Player.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/System/SystemInfo.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Common/Data/Vector.hpp"
#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

namespace Lina
{
	Weapon::Weapon(Player* player, EntityWorld* w)
	{
		m_player = player;
		m_world	 = w;
		m_entity = w->FindEntity("WeaponQuad");

		m_movement.bobPower	  = 0.7f;
		m_movement.bobSpeed	  = 12.0f;
		m_movement.swaySpeed  = 0.8f;
		m_movement.swayPowerX = 0.001f;
		m_movement.swayPowerY = -0.001f;

		if (!m_entity)
			return;

		m_runtime.startLocalPos	  = m_entity->GetLocalPosition();
		m_runtime.startLocalEuler = m_entity->GetLocalRotationAngles();

		Entity* bubbleRes = m_world->FindEntity("InanResources");
		if (bubbleRes)
		{
			const ResourceID id = bubbleRes->GetParameter("Bubble"_hs)->valRes;
			m_bubbleTemplate	= m_world->GetResourceManager()->GetIfExists<EntityTemplate>(id);
		}
	}

	Weapon::~Weapon()
	{
	}

	void Weapon::PreTick()
	{
		if (!m_entity)
			return;

		if (m_world->GetInput().GetMouseButtonDown(LINAGX_MOUSE_0))
		{
			Fire();
		}

		Vector<Entity*> killList;
		for (const BubbleData& data : m_bubbles)
		{
			if (data._kill)
				killList.push_back(data.e);
		}

		for (Entity* e : killList)
		{
			auto it = linatl::find_if(m_bubbles.begin(), m_bubbles.end(), [e](const BubbleData& data) -> bool { return data.e == e; });
			if (it != m_bubbles.end())
			{
				LINA_TRACE("Killing bubble");
				m_world->DestroyEntity(it->e);
				m_bubbles.erase(it);
			}
		}
	}

	void Weapon::Tick(float dt)
	{
		if (!m_entity)
			return;

		float bob = Math::Sin(SystemInfo::GetAppTimeF() * m_movement.bobSpeed) * m_movement.bobPower;
		bob *= m_player->m_runtime.velocity.Magnitude();

		const Vector2 md = m_world->GetInput().GetMouseDelta();

		const Vector3 targetLocalPositionOffset = Vector3(md.x * m_movement.swayPowerX, md.y * m_movement.swayPowerY, 0.0f);
		m_runtime.localPositionOffset			= Math::Lerp(m_runtime.localPositionOffset, targetLocalPositionOffset, m_movement.swaySpeed * dt);
		m_entity->SetLocalPosition(m_runtime.startLocalPos + m_runtime.localPositionOffset);
		m_entity->SetLocalRotation(Quaternion::PitchYawRoll(Vector3(m_runtime.startLocalEuler.x, 0.0f, bob)));

		const Vector3&	  camPos = m_player->m_cameraRef->GetPosition();
		const Quaternion& camRot = m_player->m_cameraRef->GetRotation();

		for (BubbleData& bubble : m_bubbles)
		{
			if (bubble._counter > bubble.destroyIn)
			{
				bubble._kill = true;
				continue;
			}

			if (!bubble._inited)
			{
				const Vector3 force = camRot.GetForward() * bubble.force;
				bubble.e->GetPhysicsBody()->AddForce(ToJoltVec3(force));
				LINA_TRACE("Adding force for bubble");
				bubble._inited = true;
				continue;
			}

			bubble._counter += dt;
		}
	}

	Entity* Weapon::SpawnBuble()
	{
		if (!m_bubbleTemplate)
			return nullptr;

		const Vector3&	  camPos = m_player->m_cameraRef->GetPosition();
		const Quaternion& camRot = m_player->m_cameraRef->GetRotation();

		const Vector3	 bubblePos = camPos + camRot.GetForward() * 5.2f;
		const Quaternion bubbleRot = Quaternion::LookAt(bubblePos, camPos, Vector3::Up);

		Entity* spawnedBubble = m_world->SpawnTemplate(m_bubbleTemplate, bubblePos, bubbleRot);
		spawnedBubble->GetPhysicsBody()->SetAllowSleeping(false);

		JPH::Body*			body = spawnedBubble->GetPhysicsBody();
		JPH::MassProperties mp;
		mp.mMass = spawnedBubble->GetPhysicsSettings().mass;

		body->SetFriction(10.0f);
		body->SetRestitution(0.1f);
		body->GetMotionProperties()->SetLinearDamping(3.0f);

		body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ, mp);
		spawnedBubble->GetPhysicsBody()->SetAllowSleeping(false);

		const BubbleData data = {
			.e		   = spawnedBubble,
			.force	   = 1000.0f,
			.destroyIn = 3.0f,
		};
		m_bubbles.push_back(data);

		// m_world->GetPhysicsWorld()->GetPhysicsSystem().GetBodyInterface().AddImpulse(spawnedBubble->GetPhysicsBody()->GetID(), ToJoltVec3(force));
		return spawnedBubble;
	}

	void WeaponMelee::Tick(float dt)
	{
		Weapon::Tick(dt);

		if (!m_entity)
			return;
	}

	void WeaponMelee::Fire()
	{
		Entity* bubble = SpawnBuble();
		if (!bubble)
			return;
	}
} // namespace Lina
