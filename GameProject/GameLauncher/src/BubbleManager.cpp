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

#include "BubbleManager.hpp"
#include "Player.hpp"
#include "Game.hpp"
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
	BubbleManager::BubbleManager(EntityWorld* world)
	{
		m_world = world;

		Entity* bubbleRes = m_world->FindEntity("InanResources");
		if (bubbleRes)
		{
			const ResourceID id = bubbleRes->GetParameter("Bubble"_hs)->valRes;
			m_bubbleTemplate	= m_world->GetResourceManager()->GetIfExists<EntityTemplate>(id);
		}
	}

	void BubbleManager::PreTick()
	{
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
				m_world->DestroyEntity(it->e);
				m_bubbles.erase(it);
			}
		}
	}

	void BubbleManager::Tick(float dt)
	{
		for (BubbleData& bubble : m_bubbles)
		{
			if (bubble._kill)
				continue;

			if (bubble._counter > bubble.destroyIn)
			{
				bubble._kill = true;
				continue;
			}

			if (!bubble._inited)
			{
				bubble.e->GetPhysicsBody()->AddForce(ToJoltVec3(bubble.shootForce));
				bubble._inited = true;
				continue;
			}

			bubble._counter += dt;
		}
	}

	void BubbleManager::SpawnBubble(const Vector3& shootForce, const Vector3& position, const Quaternion& rotation, float destroyIn)
	{
		if (!m_bubbleTemplate)
			return nullptr;

		Entity* spawnedBubble = m_world->SpawnTemplate(m_bubbleTemplate, position, rotation);

		JPH::Body*			body = spawnedBubble->GetPhysicsBody();
		JPH::MassProperties mp;
		mp.mMass = spawnedBubble->GetPhysicsSettings().mass;

		body->SetFriction(10.0f);
		body->SetRestitution(0.1f);
		body->GetMotionProperties()->SetLinearDamping(3.0f);

		body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ, mp);
		spawnedBubble->GetPhysicsBody()->SetAllowSleeping(false);

		const BubbleData data = {
			.e			= spawnedBubble,
			.shootForce = shootForce,
			.destroyIn	= destroyIn,
		};
		m_bubbles.push_back(data);
	}

} // namespace Lina
