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

#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/World/Components/CompWidget.hpp"
#include <LinaGX/Core/InputMappings.hpp>

namespace Lina
{
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

	void Game::OnGameBegin(EntityWorld* world)
	{
		m_world		  = world;
		m_player	  = new Player(m_world);
		m_mouseLocked = true;
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

		m_waveManager = new WaveManager(this);
		std::random_device rd;
		m_rng = std::mt19937(rd());

		m_gameLostScreen = m_world->FindEntity("GUIGameLost");
		m_gameWonScreen	 = m_world->FindEntity("GUIGameWon");

		if (m_gameLostScreen)
		{
			CompWidget* w = m_world->GetComponent<CompWidget>(m_gameLostScreen);
			if (w)
			{
				Widget* w = w->GetWidgetManager()->GetRoot()->FindChildWithDebugName("Restart");
			}
		}
	}

	void Game::OnGameEnd()
	{
		delete m_player;
	}

	void Game::OnGamePreTick(float dt)
	{
		if (m_mouseLocked)
		{
			m_world->GetScreen().GetOwnerWindow()->ConfineMouseToCenter();
		}
		else
		{
			m_world->GetScreen().GetOwnerWindow()->FreeMouse();
		}
	}

	void Game::OnGameTick(float dt)
	{
		if (m_gameState != GameState::Running)
			return;

		m_player->Tick(dt);
		m_waveManager->Tick(dt);

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

	void Game::OnEnemyWaveSpawned(uint32_t index)
	{
		LINA_TRACE("OnEnemyWaveSpawned: {0}", index);
	}

} // namespace Lina
