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
#include "AudioManager.hpp"
#include "Weapon.hpp"
#include "Game.hpp"

#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Common/System/SystemInfo.hpp"

#include <LinaGX/Core/InputMappings.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

namespace Lina
{

	Player::Player(EntityWorld* world, BubbleManager* bm, Application* app, AudioManager* audioManager, Game* game)
	{
		m_world		 = world;
		m_audManager = audioManager;
		m_game		 = game;

		m_entity		= m_world->FindEntity("Player");
		m_cameraRef		= m_world->FindEntity("CameraReference");
		m_app			= app;
		m_bubbleManager = bm;

		JPH::Body*			body = m_entity->GetPhysicsBody();
		JPH::MassProperties mp;
		mp.mMass = m_entity->GetPhysicsSettings().mass;

		body->SetFriction(2.0f);
		body->SetRestitution(0.1f);
		body->GetMotionProperties()->SetLinearDamping(3.0f);
		body->GetMotionProperties()->SetGravityFactor(5.0f);

		body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ | JPH::EAllowedDOFs::RotationY, mp);
		m_entity->GetPhysicsBody()->SetAllowSleeping(false);

		m_movement.movementPower	 = 50.5f;
		m_movement.rotationSpeed	 = 30.0f;
		m_movement.rotationPower	 = 5.0f;
		m_movement.recoilMove		 = Vector3(0.0f, 0.0f, -0.2f);
		m_movement.recoilEuler		 = Vector3(15.0f, 0.0f, 0.0f);
		m_movement.headbobPitchPower = 0.15f;
		m_movement.headbobPitchSpeed = 15.0f;

		m_movement.headbobYawPower = 0.3f;
		m_movement.headbobYawSpeed = 7.5f;

		m_movement.headSwayPower = 0.0f;

		m_weapon = new Weapon(m_world, this, m_bubbleManager, m_app, m_audManager);

		SetupWeapon(PlayerWeaponType::Melee, 0.0f);
	}

	Player::~Player()
	{
		delete m_weapon;
	}

	void Player::UpdateHealth(float addition)
	{
		m_health += addition;
		LINA_TRACE("HEALTH {0}", m_health);
		m_invincibilityTimer = 1.0f;
	}

	void Player::PreTick()
	{
	}

	void Player::Recoil()
	{
		m_runtime.recoilAlpha  = 1.0f;
		m_movement.recoilMove  = Vector3(0.0f, 0.0f, Math::RandF(-0.1f, -0.5f));
		m_movement.recoilEuler = Vector3(0.0f, -Math::RandF(2.0f, 5.0f), Math::RandF(-2.0f, 2.0f));
	}
	void Player::Tick(float dt)
	{
		m_runtime.recoilAlpha = Math::Lerp(m_runtime.recoilAlpha, 0.0f, dt * 10.0f);

		const bool running = m_world->GetInput().GetKey(LINAGX_KEY_LSHIFT);
		m_invincibilityTimer -= dt;

		const float	  verticalTarget   = m_world->GetInput().GetKey(LINAGX_KEY_W) ? 1.0f : (m_world->GetInput().GetKey(LINAGX_KEY_S) ? -1.0f : 0.0f);
		const float	  horizontalTarget = m_world->GetInput().GetKey(LINAGX_KEY_D) ? 1.0f : (m_world->GetInput().GetKey(LINAGX_KEY_A) ? -1.0f : 0.0f);
		const Vector2 input			   = Vector2(horizontalTarget, verticalTarget);
		Vector3		  direction		   = (m_cameraRef->GetRotation().GetForward() * verticalTarget + m_cameraRef->GetRotation().GetRight() * horizontalTarget);
		direction.y					   = 0.0f;
		m_runtime.velocity			   = direction;

		const bool	jump	  = m_world->GetInput().GetKeyDown(LINAGX_KEY_SPACE) && m_entity->GetPosition().y < 1.2f;
		const float jumpPower = 20.0f * (jump ? 1.0f : 0.0f);

		m_entity->GetPhysicsBody()->AddImpulse(ToJoltVec3(Vector3::Up * jumpPower));

		if (!Math::Equals(direction.x, 0.0f, 0.001f) || !Math::Equals(direction.y, 0.0f, 0.001f))
		{
			const Vector3 force = direction.Normalized() * m_movement.movementPower * (running ? m_movement.runningMultiplier : 1.0f);
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
		headbob *= direction.Magnitude() * 2.0f * (running ? 1.8f : 1.0f);

		const Vector3 recoilOffset	  = m_movement.recoilMove * m_runtime.recoilAlpha;
		const Vector3 recoilEulOffset = m_movement.recoilEuler * m_runtime.recoilAlpha;

		const Quaternion qX				= Quaternion::AngleAxis(m_runtime.cameraAngles.x + headbob.x + recoilEulOffset.x, Vector3::Up);
		const Quaternion qY				= Quaternion::AngleAxis(m_runtime.cameraAngles.y + headbob.y + recoilEulOffset.y, Vector3::Right);
		const Quaternion qZ				= Quaternion::AngleAxis(m_runtime.cameraAngles.z + headbob.z + recoilEulOffset.z, Vector3::Forward);
		const Quaternion cameraRotation = qX * qY * qZ;
		m_runtime.targetRotation		= Quaternion::Slerp(m_runtime.targetRotation, cameraRotation, dt * m_movement.rotationSpeed);

		m_cameraRef->SetRotation(m_runtime.targetRotation);

		m_cameraRef->SetPosition(m_entity->GetPosition() + m_entity->GetRotation().GetForward() * 0.1f + Vector3::Up * 0.5f + recoilOffset);

		m_weapon->m_runtime.isRunning = running;
		m_weapon->Tick(dt);

		// Camera.
		m_world->GetWorldCamera().SetPosition(m_cameraRef->GetPosition());
		m_world->GetWorldCamera().SetRotation(m_cameraRef->GetRotation());
		m_world->GetWorldCamera().Calculate(m_world->GetScreen().GetRenderSize());

		static constexpr float switchSpeed = 160.0f;

		// AAA quality animation system.
		if (m_switchingWeapon)
		{
			if (m_switchingWeaponStage1)
			{
				m_weapon->m_movement.holsterAlpha += 0.05f;

				if (m_weapon->m_movement.holsterAlpha > 0.99f)
				{
					m_switchingWeaponStage1 = false;
					SetupWeapon(m_switchWeaponTo, 1.0f);
				}
			}
			else
			{
				m_weapon->m_movement.holsterAlpha -= 0.05f;

				if (m_weapon->m_movement.holsterAlpha < 0.02f)
				{
					m_weapon->m_movement.holsterAlpha = 0.0f;
					m_switchingWeapon				  = false;
				}
			}
		}

		// DEBUG
		if (m_world->GetInput().GetKey(LINAGX_KEY_K))
			UpdateHealth(-10.0f);

		if (m_weapon->m_isPistolHack && m_game->m_heatLevel < 50.0f)
		{
			SwitchWeapon(PlayerWeaponType::Melee);
		}
		else if (!m_weapon->m_isPistolHack && m_game->m_heatLevel > 50.0f)
		{
			SwitchWeapon(PlayerWeaponType::Pistol);
		}

		// if (m_world->GetInput().GetKeyDown(LINAGX_KEY_1))
		//	SwitchWeapon(PlayerWeaponType::Melee);
		//
		// if (m_world->GetInput().GetKeyDown(LINAGX_KEY_2))
		//	SwitchWeapon(PlayerWeaponType::Pistol);
	}

	void Player::SwitchWeapon(PlayerWeaponType type)
	{
		if (type == m_weaponType)
			return;

		if (m_switchingWeapon)
			return;

		m_switchWeaponTo		= type;
		m_switchingWeapon		= true;
		m_switchingWeaponStage1 = true;
	}

	void Player::CreateWeapon(PlayerWeaponType type, float holsterAlpha)
	{
		if (m_weapon)
			delete m_weapon;
	}

	void Player::SetupWeapon(PlayerWeaponType type, float holsterAlpha)
	{
		m_weaponType					  = type;
		m_weapon->m_movement.holsterAlpha = holsterAlpha;

		if (type == PlayerWeaponType::Melee)
		{
			m_weapon->m_movement.bobPower		 = 0.7f;
			m_weapon->m_movement.bobSpeed		 = 12.0f;
			m_weapon->m_movement.swaySpeed		 = 0.8f;
			m_weapon->m_movement.swayPowerX		 = 0.001f;
			m_weapon->m_movement.swayPowerY		 = -0.001f;
			m_weapon->m_movement.localOffset	 = Vector3(0.0f, 0.0f, 0.5f);
			m_weapon->m_movement.holsterOffset	 = Vector3(0.0f, -0.5f, 0.0f);
			m_weapon->m_movement.runningOffset	 = Vector3(0.0f, -0.0f, 0.0f);
			m_weapon->m_animation->m_fireDisplay = 6;
			m_weapon->m_movement.spawnLight		 = false;
			m_weapon->SetAnim("Weapon0_Idle", "Weapon0_Fire", "Weapon0_Idle");
			m_weapon->m_isPistolHack = false;
			m_audManager->Play(m_audManager->m_bubbleEquip, 0.0f);
		}
		else
		{
			m_weapon->m_movement.bobPower		 = 0.7f;
			m_weapon->m_movement.bobSpeed		 = 12.0f;
			m_weapon->m_movement.swaySpeed		 = 0.8f;
			m_weapon->m_movement.swayPowerX		 = 0.001f;
			m_weapon->m_movement.swayPowerY		 = -0.001f;
			m_weapon->m_movement.localOffset	 = Vector3(0.08f, -0.01f, 0.5f);
			m_weapon->m_movement.holsterOffset	 = Vector3(0.0f, -0.5f, 0.0f);
			m_weapon->m_movement.runningOffset	 = Vector3(0.0f, -0.0f, 0.0f);
			m_weapon->m_animation->m_fireDisplay = 2;
			m_weapon->m_movement.spawnLight		 = true;
			m_weapon->SetAnim("Weapon1_Idle", "Weapon1_Fire", "Weapon1_Run");
			m_weapon->m_isPistolHack = true;
			m_audManager->Play(m_audManager->m_pistolEquip, 0.0f);
		}

		m_weapon->m_runtime.localPositionOffset = m_weapon->CalculateTargetPosition(Vector2::Zero);
	}

} // namespace Lina
