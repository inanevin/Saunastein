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

#include "AudioManager.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/World/Components/CompAudio.hpp"
#include "Common/Math/Math.hpp"

namespace Lina
{
	AudioManager::AudioManager(EntityWorld* world)
	{
		m_world = world;

		CompAudio* m_bubbleEquip = nullptr;
		CompAudio* m_pistolRun	 = nullptr;
		CompAudio* m_pistolFire	 = nullptr;
		CompAudio* m_bubbleFire	 = nullptr;
		CompAudio* m_chillMusic	 = nullptr;
		CompAudio* m_metalMusic	 = nullptr;

		Entity* pistolEquip = m_world->FindEntity("PistolEquip");
		Entity* pistolRun	= m_world->FindEntity("PistolRun");
		Entity* pistolFire	= m_world->FindEntity("PistolFire");
		Entity* bubbleFire	= m_world->FindEntity("BubbleFire");
		Entity* chillMusic	= m_world->FindEntity("ChillMusic");
		Entity* metalMusic	= m_world->FindEntity("MetalMusic");

		if (pistolEquip)
			m_pistolEquip = m_world->GetComponent<CompAudio>(pistolEquip);

		if (pistolRun)
			m_pistolRun = m_world->GetComponent<CompAudio>(pistolRun);

		if (pistolFire)
			m_pistolFire = m_world->GetComponent<CompAudio>(pistolFire);

		if (bubbleFire)
			m_bubbleFire = m_world->GetComponent<CompAudio>(bubbleFire);

		if (chillMusic)
			m_chillMusic = m_world->GetComponent<CompAudio>(chillMusic);

		if (metalMusic)
			m_metalMusic = m_world->GetComponent<CompAudio>(metalMusic);
	}

	void AudioManager::Play(CompAudio* audio)
	{
		if (!audio)
			return;

		audio->Rewind();
		audio->Play();
	}

	void AudioManager::Tick(float dt, float dangerRatio)
	{
		if (m_metalMusic)
		{
			m_metalMusic->SetGain(Math::Lerp(0.0f, 1.0f, dangerRatio));
			m_metalMusic->SetupProperties();
		}

		if (m_chillMusic)
		{
			m_chillMusic->SetGain(Math::Lerp(1.0f, 0.0f, dangerRatio));
			m_chillMusic->SetupProperties();
		}
	}

} // namespace Lina
