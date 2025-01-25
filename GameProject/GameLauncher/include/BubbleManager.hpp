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

	class EntityWorld;
	class Entity;
	class Player;
	class EntityTemplate;
	class Game;

	class BubbleManager
	{
	public:
		struct BubbleData
		{
			Vector3	   shootForce	 = Vector3::Zero;
			Vector3	   spawnPosition = Vector3::Zero;
			Quaternion spawnRotation = Quaternion::Identity();
			float	   destroyIn	 = 3.0f;
			float	   _counter		 = 0.0f;
			bool	   _inited		 = false;
			bool	   _spawned		 = false;
			bool	   _kill		 = false;
			Entity*	   _entity		 = nullptr;
		};

		BubbleManager(EntityWorld* world);

		void PreTick();
		void Tick(float dt);
		void SpawnBubble(const Vector3& shootForce, const Vector3& position, const Quaternion& rotation, float destroyIn = 3.0f);

		EntityTemplate*	   m_bubbleTemplate = nullptr;
		Vector<BubbleData> m_bubbles;
		EntityWorld*	   m_world = nullptr;
	};

} // namespace Lina
