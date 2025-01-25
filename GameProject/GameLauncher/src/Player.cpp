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

#include "Player.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Common/System/SystemInfo.hpp"
#include "Weapon.hpp"
#include "Game.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

namespace Lina
{

	Player::Player(EntityWorld* world, BubbleManager* bm)
	{
		m_world = world;

		m_entity	= m_world->FindEntity("Player");
		m_cameraRef = m_world->FindEntity("CameraReference");

		JPH::Body*			body = m_entity->GetPhysicsBody();
		JPH::MassProperties mp;
		mp.mMass = m_entity->GetPhysicsSettings().mass;

		body->SetFriction(10.0f);
		body->SetRestitution(0.1f);
		body->GetMotionProperties()->SetLinearDamping(3.0f);

		body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ | JPH::EAllowedDOFs::RotationY, mp);
		m_entity->GetPhysicsBody()->SetAllowSleeping(false);

		m_movement.movementPower = 60.5f;
		m_movement.rotationSpeed = 30.0f;
		m_movement.rotationPower = 5.0f;

		m_movement.headbobPitchPower = 0.15f;
		m_movement.headbobPitchSpeed = 15.0f;

		m_movement.headbobYawPower = 0.3f;
		m_movement.headbobYawSpeed = 7.5f;

		m_movement.headSwayPower = 0.0f;
		m_weapon				 = new WeaponMelee(m_world, this, bm);
	}

	Player::~Player()
	{
		delete m_weapon;
	}

	void Player::UpdateHealth(float addition)
	{
		m_health += addition;
		LINA_TRACE("HEALTH {0}", m_health);
	}

	void Player::PreTick()
	{
	}

	void Player::Tick(float dt)
	{
		static float test = 0.0f;
		test += dt;

		const float	  verticalTarget   = m_world->GetInput().GetKey(LINAGX_KEY_W) ? 1.0f : (m_world->GetInput().GetKey(LINAGX_KEY_S) ? -1.0f : 0.0f);
		const float	  horizontalTarget = m_world->GetInput().GetKey(LINAGX_KEY_D) ? 1.0f : (m_world->GetInput().GetKey(LINAGX_KEY_A) ? -1.0f : 0.0f);
		const Vector2 input			   = Vector2(horizontalTarget, verticalTarget);
		Vector3		  direction		   = (m_cameraRef->GetRotation().GetForward() * verticalTarget + m_cameraRef->GetRotation().GetRight() * horizontalTarget);
		direction.y					   = 0.0f;
		m_runtime.velocity			   = direction;

		if (!Math::Equals(direction.x, 0.0f, 0.001f) || !Math::Equals(direction.y, 0.0f, 0.001f))
		{
			const Vector3 force = direction.Normalized() * m_movement.movementPower;
			m_entity->GetPhysicsBody()->AddForce(ToJoltVec3(force));
		}

		// Mouse camera movement.
		const Vector2 mouseDelta = m_world->GetInput().GetMouseDelta();
		m_runtime.cameraAngles.x += mouseDelta.x * dt * 10.0f;
		m_runtime.cameraAngles.y += mouseDelta.y * dt * 10.0f;
		m_runtime.cameraAngles.y = Math::Clamp(m_runtime.cameraAngles.y, -89.0f, 89.0f);
		m_runtime.cameraAngles.z = 0.0f;

		// Head-bob
		Vector3 headbob =
			Vector3(Math::Sin(SystemInfo::GetAppTimeF() * m_movement.headbobYawSpeed) * m_movement.headbobYawPower, Math::Cos(SystemInfo::GetAppTimeF() * m_movement.headbobPitchSpeed) * m_movement.headbobPitchPower, input.x * m_movement.headSwayPower);
		headbob *= direction.Magnitude() * 2.0f;

		const Quaternion qX				= Quaternion::AngleAxis(m_runtime.cameraAngles.x + headbob.x, Vector3::Up);
		const Quaternion qY				= Quaternion::AngleAxis(m_runtime.cameraAngles.y + headbob.y, Vector3::Right);
		const Quaternion qZ				= Quaternion::AngleAxis(m_runtime.cameraAngles.z + headbob.z, Vector3::Forward);
		const Quaternion cameraRotation = qX * qY * qZ;
		m_runtime.targetRotation		= Quaternion::Slerp(m_runtime.targetRotation, cameraRotation, dt * m_movement.rotationSpeed);

		m_cameraRef->SetRotation(m_runtime.targetRotation);
		m_cameraRef->SetPosition(m_entity->GetPosition() + m_entity->GetRotation().GetForward() * 0.1f);

		m_weapon->Tick(dt);

		// Camera.
		m_world->GetWorldCamera().SetPosition(m_cameraRef->GetPosition());
		m_world->GetWorldCamera().SetRotation(m_cameraRef->GetRotation());
		m_world->GetWorldCamera().Calculate(m_world->GetScreen().GetRenderSize());

		// DEBUG
		if (m_world->GetInput().GetKey(LINAGX_KEY_K))
			UpdateHealth(-10.0f);
	}
} // namespace Lina
