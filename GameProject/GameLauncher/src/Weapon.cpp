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
#include "AudioManager.hpp"

#include "Core/World/EntityWorld.hpp"
#include "Common/System/SystemInfo.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Common/Data/Vector.hpp"
#include "Core/Application.hpp"
#include "Core/World/Components/CompModel.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

namespace Lina
{

	WeaponAnimation::WeaponAnimation(EntityWorld* world, Application* app)
	{
		m_app						   = app;
		Entity*					 wep   = world->FindEntity("WeaponQuad");
		CompModel*				 model = world->GetComponent<CompModel>(wep);
		const Vector<ResourceID> mats  = model->GetMaterials();

		if (!mats.empty())
			m_material = m_app->GetResourceManager().GetIfExists<Material>(mats.at(0));

		m_used = m_idle;
		UpdateMaterial();
	}

	void WeaponAnimation::Tick(float dt)
	{
		if (m_used == m_fire)
		{
			if (m_ctr > m_fireDisplay)
			{
				m_ctr = 0;
				Idle();
			}
			m_ctr++;
		}

		if (m_used == m_idle && m_isRunning)
			m_used = m_run;

		if (m_used == m_run && !m_isRunning)
			m_used = m_idle;

		UpdateMaterial();
	}

	void WeaponAnimation::UpdateMaterial()
	{
		if (!m_material)
			return;

		if (m_material->HasProperty<LinaTexture2D>("txtAlbedo"_hs))
		{
			LinaTexture2D* prop = m_material->GetProperty<LinaTexture2D>("txtAlbedo"_hs);
			if (prop->texture != m_used && m_used != 0)
			{
				prop->texture = m_used;
				m_app->GetGfxContext().MarkBindlessDirty();
			}
		}
	}

	Weapon::Weapon(EntityWorld* world, Player* player, BubbleManager* bm, Application* app, AudioManager* am)
	{
		m_app			= app;
		m_world			= world;
		m_player		= player;
		m_bubbleManager = bm;
		m_audioManager	= am;

		m_entity = m_world->FindEntity("WeaponQuad");
		m_light	 = m_world->FindEntity("WeaponLight");

		if (!m_entity)
			return;

		m_runtime.localPositionOffset = CalculateTargetPosition(Vector2::Zero);
		m_runtime.startLocalEuler	  = m_entity->GetLocalRotationAngles();

		m_animation = new WeaponAnimation(m_world, app);
	}

	Weapon::~Weapon()
	{
		if (m_animation)
			delete m_animation;
	}

	Vector3 Weapon::CalculateTargetPosition(const Vector2& md)
	{
		const Vector3 holster = m_movement.holsterOffset * m_movement.holsterAlpha;
		const Vector3 running = m_movement.runningOffset * (m_runtime.isRunning ? 1.0f : 0.0f);
		Vector3		  targetLocalPositionOffset =
			Vector3(md.x * m_movement.swayPowerX + m_movement.localOffset.x + holster.x + running.x, md.y * m_movement.swayPowerY + m_movement.localOffset.y + holster.y + running.y, m_movement.localOffset.z + holster.z + running.z);
		return targetLocalPositionOffset;
	}

	void Weapon::Tick(float dt)
	{
		if (!m_entity)
			return;

		float bob = Math::Sin(SystemInfo::GetAppTimeF() * m_movement.bobSpeed) * m_movement.bobPower;
		bob *= m_player->m_runtime.velocity.Magnitude();

		const Vector2 md			  = m_world->GetInput().GetMouseDelta();
		m_runtime.localPositionOffset = Math::Lerp(m_runtime.localPositionOffset, CalculateTargetPosition(md), m_movement.swaySpeed * dt * 10.0f);

		m_entity->SetLocalPosition(m_runtime.localPositionOffset);
		m_entity->SetLocalRotation(Quaternion::PitchYawRoll(Vector3(m_runtime.startLocalEuler.x, 0.0f, bob)));

		m_animation->m_isRunning = m_runtime.isRunning;
		m_animation->Tick(dt);

		if (m_light && m_light->GetVisible())
		{
			if (m_lightCtr > 3)
			{
				m_light->SetVisible(false);
				m_lightCtr = 0;
			}
			m_lightCtr++;
		}
		if (m_world->GetInput().GetMouseButtonDown(LINAGX_MOUSE_0))
		{
			Fire();
		}
	}

	void Weapon::Fire()
	{
		if (m_isPistolHack)
			m_player->Recoil();

		const Vector3&	  camPosition	= m_player->m_cameraRef->GetPosition();
		const Quaternion& camRotation	= m_player->m_cameraRef->GetRotation();
		const Vector3	  spawnPosition = camPosition + camRotation.GetForward() * 1.0f;
		const Quaternion  spawnRotation = Quaternion::LookAt(spawnPosition, camPosition, Vector3::Up);
		const Vector3	  shootForce	= (camRotation.GetForward() * 700.0f + -camRotation.GetUp() * 100) * (m_isPistolHack ? 1.8f : 1.0f);
		m_bubbleManager->SpawnBubble(shootForce, spawnPosition, spawnRotation);
		m_animation->Fire();

		if (m_light && m_movement.spawnLight)
		{
			m_light->SetVisible(true);
			m_lightCtr = 0;
		}

		m_audioManager->Play(m_isPistolHack ? m_audioManager->m_pistolFire : m_audioManager->m_bubbleFire, 0.5f);
	}

	void Weapon::SetAnim(const String& animIdle, const String& animFire, const String& animRun)
	{
		Entity* idle = m_world->FindEntity(animIdle);
		Entity* fire = m_world->FindEntity(animFire);
		Entity* run	 = m_world->FindEntity(animRun);
		if (idle)
		{
			for (const EntityParameter& p : idle->GetParams().params)
			{
				m_animation->m_idle = p.valRes;
			}
		}

		if (fire)
		{
			for (const EntityParameter& p : fire->GetParams().params)
			{
				m_animation->m_fire = p.valRes;
			}
		}

		if (run)
		{
			for (const EntityParameter& p : run->GetParams().params)
			{
				m_animation->m_run = p.valRes;
			}
		}

		m_animation->m_used = m_animation->m_idle;
		m_animation->UpdateMaterial();
	}

} // namespace Lina
