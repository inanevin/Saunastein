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

		m_runtime.startLocalPos	  = m_entity->GetLocalPosition();
		m_runtime.startLocalEuler = m_entity->GetLocalRotationAngles();
	}

	Weapon::~Weapon()
	{
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
	}

	void WeaponMelee::Tick(float dt)
	{
		Weapon::Tick(dt);

		if (!m_entity)
			return;
	}
} // namespace Lina
