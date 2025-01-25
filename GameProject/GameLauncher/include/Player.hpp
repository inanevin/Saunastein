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

#pragma once

#include "Common/Math/Vector.hpp"
#include "Common/Math/Quaternion.hpp"

namespace LinaGX
{

} // namespace LinaGX

namespace Lina
{

	class Weapon;
	class EntityWorld;
	class Entity;

	class Player
	{
	public:
		struct Movement
		{
			float movementPower = 10.5f;
			float rotationSpeed = 20.0f;
			float rotationPower = 5.0f;

			float headbobPitchPower = 4.0f;
			float headbobYawPower	= 2.0f;
			float headbobPitchSpeed = 4.0f;
			float headbobYawSpeed	= 2.0f;
			float headSwayPower		= 0.5f;
		};

		struct Runtime
		{
			Quaternion targetRotation = Quaternion::Identity();
			Vector3	   cameraAngles	  = Vector2::Zero;
			Vector3	   velocity		  = Vector3::Zero;
		};

		Player(EntityWorld* ew);
		~Player();

		void Tick(float dt);

		EntityWorld* m_world	 = nullptr;
		Entity*		 m_entity	 = nullptr;
		Entity*		 m_cameraRef = nullptr;
		Weapon*		 m_weapon	 = nullptr;

		Movement m_movement = {};
		Runtime	 m_runtime	= {};
	};
} // namespace Lina
