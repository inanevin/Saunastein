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

#include "Core/World/EntityWorld.hpp"
#include "Core/Physics/PhysicsContactListener.hpp"

namespace LinaGX
{
	class Window;
	enum class InputAction;
	struct LGXVector2;

} // namespace LinaGX

namespace Lina
{
	class EntityWorld;
	class Player;
	class Enemy;
	class GameLauncher;
	class CompAudio;
	class WaveManager;
	class BubbleManager;
	class HudManager;
	class CompLight;
	class Application;
	class AudioManager;

	enum class GameState
	{
		Running,
		Lost,
		Won
	};

	class Game : public PhysicsContactListener
	{
	public:
		void OnGameBegin(EntityWorld* world, GameLauncher* gl, Application* app);
		void OnGameEnd();
		void OnGamePreTick();
		void OnGameTick(float dt);

		void OnKey(uint32 keycode, int32 scancode, LinaGX::InputAction inputAction);
		void OnMouse(uint32 button, LinaGX::InputAction inputAction);
		void OnMouseWheel(float amt);
		void OnMouseMove(const LinaGX::LGXVector2&);
		void OnWindowFocus(bool focus);

		void OnEnemySpawned(Enemy* enemy);
		void OnWaveSpawned(uint32_t index, String name);

		void UpdateHeat(float addition);
		void SetHeat(float heat);

		virtual void OnContactBegin(Entity* e1, Entity* e2, const Vector3& p1, const Vector3& p2) override;
		virtual void OnContact(Entity* e1, Entity* e2, const Vector3& p1, const Vector3& p2) override;
		virtual void OnContactEnd(Entity* e1, Entity* e2) override;

		EntityTemplate* GetEntityTemplate(String key);

		AudioManager*					 m_audioManager	 = nullptr;
		BubbleManager*					 m_bubbleManager = nullptr;
		WaveManager*					 m_waveManager	 = nullptr;
		EntityWorld*					 m_world		 = nullptr;
		Player*							 m_player		 = nullptr;
		HudManager*						 m_hudManager	 = nullptr;
		HashMap<String, EntityParameter> m_resources;
		bool							 m_mouseLocked	= false;
		bool							 m_mouseVisible = true;
		std::mt19937					 m_rng;

		GameLauncher* m_gameLauncher;
		Entity*		  m_gameLostScreen = nullptr;
		Entity*		  m_gameWonScreen  = nullptr;
		GameState	  m_gameState	   = GameState::Running;
		Entity*		  m_fireVisuals	   = nullptr;

		float	   m_heatLevel		 = 50.0f;
		Vector3	   m_skyTopColor	 = Vector3::Zero;
		Vector3	   m_skyHorizonColor = Vector3::Zero;
		CompLight* m_sunLight		 = nullptr;
		float	   m_heatDangerRatio = 0.0f;
	};
} // namespace Lina
