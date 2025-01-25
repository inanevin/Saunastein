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
#include "Game.hpp"
#include "BubbleManager.hpp"

#include "Core/World/EntityWorld.hpp"
#include "Common/System/SystemInfo.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Common/Data/Vector.hpp"
#include "Core/Application.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

namespace Lina
{
	void WeaponAnimation::AddTextureID(ResourceID id)
	{
		m_textureIDs.push_back(id);
	}

	void WeaponAnimation::Tick(float dt, Material* material)
	{
		m_frameCtr++;

		if (m_frameCtr > m_displayFrames)
		{
			m_frameCtr = 0;
			m_index	   = (m_index + 1) % static_cast<uint32>(m_textureIDs.size());
		}

		if (m_textureIDs.empty())
			return;

		m_textureID = m_textureIDs.at(m_index);
		UpdateMaterial(material);
	}

	void WeaponAnimation::UpdateMaterial(Material* mat)
	{
		if (mat->HasProperty<LinaTexture2D>("txtAlbedo"_hs))
		{
			LinaTexture2D* prop = mat->GetProperty<LinaTexture2D>("txtAlbedo"_hs);
			if (prop->texture != m_textureID)
			{
				prop->texture = m_textureID;
				m_app->GetGfxContext().MarkBindlessDirty();
			}
		}
	}

	Weapon::Weapon(EntityWorld* world, Player* player, BubbleManager* bm, Application* app)
	{
		m_app			= app;
		m_world			= world;
		m_player		= player;
		m_bubbleManager = bm;

		m_entity = m_world->FindEntity("WeaponQuad");

		m_movement.bobPower	  = 0.7f;
		m_movement.bobSpeed	  = 12.0f;
		m_movement.swaySpeed  = 0.8f;
		m_movement.swayPowerX = 0.001f;
		m_movement.swayPowerY = -0.001f;

		if (!m_entity)
			return;

		m_runtime.startLocalPos	  = m_entity->GetLocalPosition();
		m_runtime.startLocalEuler = m_entity->GetLocalRotationAngles();
	}

	Weapon::~Weapon()
	{
	}

	WeaponMelee::WeaponMelee(EntityWorld* world, Player* player, BubbleManager* bm, Application* app) : Weapon(world, player, bm, app)
	{
		Entity* idle = m_world->FindEntity("Weapon0_Idle");
		Entity* fire = m_world->FindEntity("Weapon0_Fire");

		m_idleAnim = {};
		m_fireAnim = {};

		m_idleAnim.SetApp(app);
		m_fireAnim.SetApp(app);

		if (idle)
		{
			for (const EntityParameter& p : idle->GetParams().params)
			{
				if (p.type != EntityParameterType::ResourceID)
					continue;
				m_idleAnim.AddTextureID(p.valRes);
			}
		}

		if (fire)
		{
			for (const EntityParameter& p : fire->GetParams().params)
			{
				if (p.type != EntityParameterType::ResourceID)
					continue;
				m_fireAnim.AddTextureID(p.valRes);
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

		if (m_world->GetInput().GetMouseButtonDown(LINAGX_MOUSE_0))
		{
			Fire();
		}
	}

	void WeaponMelee::Tick(float dt)
	{
		Weapon::Tick(dt);
		if (!m_entity)
			return;
	}

	void WeaponMelee::Fire()
	{
		const Vector3&	  camPosition	= m_player->m_cameraRef->GetPosition();
		const Quaternion& camRotation	= m_player->m_cameraRef->GetRotation();
		const Vector3	  spawnPosition = camPosition + camRotation.GetForward() * 5.5f;
		const Quaternion  spawnRotation = Quaternion::LookAt(spawnPosition, camPosition, Vector3::Up);
		const Vector3	  shootForce	= camRotation.GetForward() * 500.0f;
		m_bubbleManager->SpawnBubble(shootForce, spawnPosition, spawnRotation);
	}
} // namespace Lina
