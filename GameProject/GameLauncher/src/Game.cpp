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

#include "Game.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include "WaveManager.hpp"
#include "BubbleManager.hpp"
#include "HudManager.hpp"
#include "AudioManager.hpp"

#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/World/Components/CompWidget.hpp"
#include "Core/GUI/Widgets/Primitives/Button.hpp"
#include "GameLauncher.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Graphics/Resource/Material.hpp"
#include "Core/Application.hpp"
#include "Core/World/Components/CompAudio.hpp"
#include "Core/World/Components/CompLight.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include <LinaGX/Core/InputMappings.hpp>

namespace Lina
{
	static LINAGX_STRING FormatString(const char* fmt, va_list args)
	{
		// Determine the required size
		va_list args_copy;
		va_copy(args_copy, args);
		int size = vsnprintf(nullptr, 0, fmt, args_copy) + 1; // +1 for the null terminator
		va_end(args_copy);

		// Allocate a buffer and format the string
		std::vector<char> buffer(size);
		vsnprintf(buffer.data(), size, fmt, args);

		return std::string(buffer.data());
	}
	static LINAGX_STRING FormatString(const char* fmt, ...)
	{
		// Initialize a variable argument list
		va_list args;
		va_start(args, fmt);

		// Call the existing FormatString that takes va_list
		va_list args_copy;
		va_copy(args_copy, args);
		int size = vsnprintf(nullptr, 0, fmt, args_copy) + 1; // +1 for the null terminator
		va_end(args_copy);

		// Allocate a buffer and format the string
		std::vector<char> buffer(size);
		vsnprintf(buffer.data(), size, fmt, args);

		va_end(args);

		return LINAGX_STRING(buffer.data());
	}

	void Game::OnKey(uint32 keycode, int32 scancode, LinaGX::InputAction inputAction)
	{
		if (keycode == LINAGX_KEY_ESCAPE && inputAction == LinaGX::InputAction::Pressed)
		{
			m_mouseLocked = false;
			m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(true);
			m_mouseVisible = true;
		}
	}

	void Game::OnMouse(uint32 button, LinaGX::InputAction inputAction)
	{
		if (m_gameState != GameState::Running)
			return;

		m_mouseLocked = true;

		if (inputAction == LinaGX::InputAction::Pressed)
		{
			m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(false);
			m_mouseVisible = false;
		}
	}

	void Game::OnMouseWheel(float amt)
	{
	}

	void Game::OnMouseMove(const LinaGX::LGXVector2&)
	{
	}

	void Game::OnGameBegin(EntityWorld* world, GameLauncher* gl, Application* app)
	{
		m_gameLauncher = gl;
		m_world		   = world;

		m_bubbleManager = new BubbleManager(m_world);
		m_hudManager	= new HudManager(this);
		m_audioManager	= new AudioManager(m_world);
		m_player		= new Player(m_world, m_bubbleManager, app, m_audioManager);
		m_mouseLocked	= true;

		// Find resources
		Entity* res = m_world->FindEntity("Resources");

		if (res == NULL)
		{
			LINA_ERR("Resources entity not found");
			return;
		}

		for (auto param : res->GetParams().params)
		{
			m_resources[param.name] = param;
			LINA_INFO("Resource: {0}", param.name);
		}

		Entity* light = m_world->FindEntity("SunLight");
		m_sunLight	  = m_world->GetComponent<CompLight>(light);

		m_waveManager = new WaveManager(this);
		std::random_device rd;
		m_rng = std::mt19937(rd());

		m_gameLostScreen = m_world->FindEntity("GUIGameLost");
		m_gameWonScreen	 = m_world->FindEntity("GUIGameWon");
		m_fireVisuals	 = m_world->FindEntity("Fire");

		if (m_gameLostScreen)
		{
			CompWidget* w = m_world->GetComponent<CompWidget>(m_gameLostScreen);
			if (w)
			{
				WidgetManager& wm	   = w->GetWidgetManager();
				Widget*		   restart = wm.GetRoot()->FindChildWithDebugName("Restart");
				if (restart)
				{
					static_cast<Button*>(restart)->GetProps().onClicked = [this]() { m_gameLauncher->Restart(); };
				}

				Widget* quit = wm.GetRoot()->FindChildWithDebugName("Quit");
				if (quit)
				{

					static_cast<Button*>(quit)->GetProps().onClicked = [this]() { m_gameLauncher->Quit(); };
				}
			}
		}

		Material* skyMat = m_world->GetResourceManager()->GetIfExists<Material>(m_world->GetGfxSettings().skyMaterial);
		if (skyMat)
		{
			m_skyTopColor	  = *skyMat->GetProperty<Vector3>("skyColor"_hs);
			m_skyHorizonColor = *skyMat->GetProperty<Vector3>("horizonColor"_hs);
		}
		m_world->GetPhysicsWorld()->AddContactListener(this);
	}

	void Game::OnGameEnd()
	{
		m_world->GetPhysicsWorld()->RemoveContactListener(this);

		delete m_audioManager;
		delete m_waveManager;
		delete m_player;
		delete m_bubbleManager;
	}

	void Game::OnGamePreTick()
	{

		if (m_mouseLocked)
		{
			m_world->GetScreen().GetOwnerWindow()->ConfineMouseToCenter();
		}
		else
		{
			m_world->GetScreen().GetOwnerWindow()->FreeMouse();
		}

		if (m_gameState != GameState::Running)
			return;

		m_player->PreTick();
		m_waveManager->PreTick();
		m_bubbleManager->PreTick();
	}

	void Game::OnGameTick(float dt)
	{
		m_audioManager->Tick(dt, m_heatDangerRatio);

		if (m_gameState != GameState::Running)
			return;

		m_player->Tick(dt);
		m_bubbleManager->Tick(dt);
		m_waveManager->Tick(dt);
		m_hudManager->Tick(dt);

		if (m_player->m_health < 0.0f && m_gameLostScreen != nullptr)
		{
			m_gameState = GameState::Lost;
			m_gameLostScreen->SetVisible(true);
			m_mouseLocked = false;

			if (!m_mouseVisible)
			{
				m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(true);
				m_mouseVisible = true;
			}
		}

		// DEBUG
		if (m_world->GetInput().GetKeyDown(LINAGX_KEY_H))
		{
			UpdateHeat(10.0f);
		}

		if (m_world->GetInput().GetKeyDown(LINAGX_KEY_N))
		{
			UpdateHeat(-10.0f);
		}
    
    m_hudManager->SetTopRight(FormatString("Score: %i", m_score));
        
    if (m_fireVisuals)
    {
      Vector3 scale = m_fireVisuals->GetLocalScale();
      m_fireTargetScale = Math::Lerp(0.0f, 1.0f, m_dangerRatio);
      scale.y      = Math::Lerp(scale.y, m_fireTargetScale, 10*dt);
      m_fireVisuals->SetLocalScale(scale);
    }

	}

	EntityTemplate* Game::GetEntityTemplate(String key)
	{
		if (m_resources.contains(key))
		{
			EntityParameter param = m_resources[key];
			if (param.type != EntityParameterType::ResourceID)
			{
				LINA_ERR("Parameter {0} is not a resource", key);
				return nullptr;
			}

			ResourceID		   rid			   = m_resources[key].valRes;
			ResourceManagerV2* rm			   = m_world->GetResourceManager();
			EntityTemplate*	   entity_template = rm->GetIfExists<EntityTemplate>(rid);

			if (entity_template == nullptr)
			{
				LINA_ERR("Failed to get EntityTemplate for key {0} ", key);
				return nullptr;
			}

			return entity_template;
		}
		else
		{
			LINA_ERR("No resource {0} found", key);
			return nullptr;
		}
	}

	void Game::OnWindowFocus(bool focus)
	{
		if (!m_world)
			return;

		if (focus)
		{
			// m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(false);
			// m_world->GetScreen().GetOwnerWindow()->SetWrapMouse(true);
		}
		else
		{
			// m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(true);
			// m_world->GetScreen().GetOwnerWindow()->SetWrapMouse(false);
		}
	}

	void Game::OnEnemySpawned(Enemy* enemy)
	{
	}

	void Game::OnWaveSpawned(uint32_t index, String name)
	{
		m_hudManager->SetMainText(name);
		m_hudManager->SetTopRight(name);
		//		LINA_TRACE("OnEnemyWaveSpawned: {0} {1}", index, name);
	}

	void Game::AddScore(uint32_t score)
	{
		m_score += score;
	}

	void Game::UpdateHeat(float addition)
	{
		SetHeat(m_heatLevel + addition);
	}

	void Game::SetHeat(float heat)
	{
		m_heatLevel = heat;
		m_heatLevel = Math::Clamp(m_heatLevel, 0.0f, 100.0f);
//		LINA_TRACE("HeatLevel {0}", m_heatLevel);

		const float	  dangerRatio				   = Math::Clamp(Math::Remap(m_heatLevel, 00.0f, 100.0f, 0.0f, 1.0f), 0.0f, 1.0f);
		const Vector4 ambientCold				   = Vector4(1, 1, 1, 1);
		const Vector4 ambientHot				   = Vector4(0.9f, 0.1f, 0.1f, 1.0f);
		const Vector4 ambient					   = Math::Lerp(ambientCold, ambientHot, dangerRatio);
		m_world->GetGfxSettings().ambientTop	   = ambient;
		m_world->GetGfxSettings().ambientBot	   = ambient;
		m_world->GetGfxSettings().ambientMid	   = ambient;
		m_world->GetGfxSettings().ambientIntensity = Math::Lerp(0.2f, 1.0f, dangerRatio);
    
    m_dangerRatio = dangerRatio;
		
    if (m_sunLight)
		{
			m_sunLight->SetColor(Math::Lerp(Color::White, Color(1, 0, 0, 1), dangerRatio));
			m_sunLight->SetIntensity(Math::Lerp(0.45f, 2.0f, dangerRatio));
		}

		Material* skyMat = m_world->GetResourceManager()->GetIfExists<Material>(m_world->GetGfxSettings().skyMaterial);
		if (skyMat)
		{
			const Vector3 skyColor	   = Math::Lerp(m_skyTopColor, Vector3(0.3f, 0.01f, 0.01f), dangerRatio);
			const Vector3 horizonColor = Math::Lerp(m_skyTopColor, Vector3(3.0f, 0.01f, 0.01f), dangerRatio);
			skyMat->SetProperty<Vector3>("skyColor"_hs, skyColor);
			skyMat->SetProperty<Vector3>("horizonColor"_hs, horizonColor);
			m_gameLauncher->GetApp()->GetGfxContext().MarkBindlessDirty();
		}

		m_heatDangerRatio = dangerRatio;
	}

	void Game::OnContactBegin(Entity* e1, Entity* e2, const Vector3& p1, const Vector3& p2)
	{
		m_waveManager->HandleContact(e1, e2);
	}

	void Game::OnContact(Entity* e1, Entity* e2, const Vector3& p1, const Vector3& p2)
	{
    m_waveManager->HandleContact(e1, e2);
	}

	void Game::OnContactEnd(Entity* e1, Entity* e2)
	{
	}

} // namespace Lina
