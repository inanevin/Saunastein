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

		Entity* bubbleEquip = m_world->FindEntity("BubbleEquip");
		Entity* pistolEquip = m_world->FindEntity("PistolEquip");
		Entity* pistolFire	= m_world->FindEntity("PistolFire");
		Entity* bubbleFire	= m_world->FindEntity("BubbleFire");
		Entity* chillMusic	= m_world->FindEntity("ChillMusic");
		Entity* metalMusic	= m_world->FindEntity("MetalMusic");
		Entity* humpfh		= m_world->FindEntity("Humpfh");

		if (bubbleEquip)
			m_bubbleEquip = m_world->GetComponent<CompAudio>(bubbleEquip);

		if (pistolEquip)
			m_pistolEquip = m_world->GetComponent<CompAudio>(pistolEquip);

		if (pistolFire)
			m_pistolFire = m_world->GetComponent<CompAudio>(pistolFire);

		if (bubbleFire)
			m_bubbleFire = m_world->GetComponent<CompAudio>(bubbleFire);

		if (humpfh)
			m_humpfh = m_world->GetComponent<CompAudio>(humpfh);

		if (chillMusic)
		{
			m_chillMusic = m_world->GetComponent<CompAudio>(chillMusic);
			m_chillMusic->SetGain(1.0f);
			m_chillMusic->Rewind();
			m_chillMusic->Play();
		}

		if (metalMusic)
		{
			m_metalMusic = m_world->GetComponent<CompAudio>(metalMusic);
			m_metalMusic->SetGain(0.0f);
			m_metalMusic->SetupProperties();
		}
	}

	void AudioManager::Play(CompAudio* audio, float varyPitch)
	{
		if (!audio)
			return;

		const float pitch = Math::RandF(1.0f - varyPitch, 1.0f + varyPitch);
		audio->SetPitch(pitch);
		audio->Rewind();
		audio->Play();
	}

	void AudioManager::Tick(float dt, float dangerRatio)
	{
		if (m_metalMusic)
		{
			m_metalMusic->SetGain(Math::Lerp(0.0f, 3.0f, dangerRatio));
			m_metalMusic->SetupProperties();
		}

		if (m_chillMusic)
		{
			if (dangerRatio > 0.2f && m_chillMusic->GetGain() > 0.1f)
			{
				m_chillMusic->SetGain(Math::Lerp(m_chillMusic->GetGain(), 0.0f, dt * 5.0f));
				m_chillMusic->SetupProperties();
			}

			// if(dangerRatio > 0.3f)
			// m_chillMusic->SetGain(Math::Lerp(1.0f, 0.0f, dangerRatio));
			// else
			// m_chillMusic->SetGain(1.0f);

			/// m_chillMusic->SetupProperties();
		}
	}

	void AudioManager::OnResetHeat()
	{
		if (m_chillMusic && m_chillMusic->GetGain() < 0.1f)
		{
			m_chillMusic->Rewind();
			m_chillMusic->SetGain(0.75f);
			m_chillMusic->Play();
		}

		// if(m_chillMusic)
		// {
		//     m_chillMusic->Rewind();
		//     m_chillMusic->SetGain(1.0f);
		//     m_chillMusic->SetupProperties();
		// }
	}

} // namespace Lina
