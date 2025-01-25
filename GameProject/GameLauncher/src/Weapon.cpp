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

namespace Lina
{
	Weapon::Weapon(Player* player, EntityWorld* w)
	{
        m_player = player;
		m_world	 = w;
		m_entity = w->FindEntity("WeaponQuad");
	}

	Weapon::~Weapon()
	{
	}

    void Weapon::Tick(float dt)
    {
        if(!m_entity)
            return;
        
       // const Vector3& camPosition = m_player->m_cameraRef->GetPosition();
       // const Quaternion& camRotation = m_player->m_cameraRef->GetRotation();
       //
       // m_entity->SetPosition(camPosition + camRotation.GetForward() * 0.5f);
       // m_entity->SetRotation(Quaternion::LookAt(m_entity->GetPosition(), -camPosition, Vector3::Up));
    }

    void WeaponMelee::Tick(float dt)
    {
        Weapon::Tick(dt);
        
        if(!m_entity)
            return;
    }
} // namespace Lina
