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
	class CompAudio;
	class EntityWorld;
	class Entity;
	class Player;
	class EntityTemplate;
	class Game;

	class AudioManager
	{
	public:
		AudioManager(EntityWorld* world);

		void Play(CompAudio* audio);
		void Tick(float dt, float dangerRatio);

		EntityWorld* m_world	   = nullptr;
		CompAudio*	 m_pistolEquip = nullptr;
		CompAudio*	 m_bubbleEquip = nullptr;
		CompAudio*	 m_pistolRun   = nullptr;
		CompAudio*	 m_pistolFire  = nullptr;
		CompAudio*	 m_bubbleFire  = nullptr;
		CompAudio*	 m_chillMusic  = nullptr;
		CompAudio*	 m_metalMusic  = nullptr;
	};

} // namespace Lina
