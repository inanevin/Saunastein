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
<<<<<<< HEAD
#include "Enemy.hpp"

#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
=======
#include "Core/World/EntityWorld.hpp"
#include <LinaGX/Core/InputMappings.hpp>
>>>>>>> origin/main

namespace Lina
{
	void Game::OnKey(uint32 keycode, int32 scancode, LinaGX::InputAction inputAction)
	{
		if (keycode == LINAGX_KEY_ESCAPE && inputAction == LinaGX::InputAction::Pressed)
		{
			m_mouseLocked = false;
			m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(true);
			LINA_TRACE("MOUSE UNLOCKED");
		}
	}

	void Game::OnMouse(uint32 button, LinaGX::InputAction inputAction)
	{
		m_mouseLocked = true;

		if (inputAction == LinaGX::InputAction::Pressed)
			m_world->GetScreen().GetOwnerWindow()->SetMouseVisible(false);

		LINA_TRACE("MOUSE LOCKED");
	}

	void Game::OnMouseWheel(float amt)
	{
	}

	void Game::OnMouseMove(const LinaGX::LGXVector2&)
	{
	}

	void Game::OnGameBegin(EntityWorld* world)
	{
		m_world	 = world;
		m_player = new Player(m_world);
    m_mouseLocked = true;
    // Find resources
    Entity *res = m_world->FindEntity("Resources");
    
    if (res == NULL) {
      LINA_ERR("Resources entity not found");
      return;
    }
    
    for (auto param : res->GetParams().params) {
      m_resources[param.name] = param;
      LINA_INFO("Resource: {0}", param.name);
    }
    
    // Find enemy spawns
    m_world->ViewEntities([&](Entity* e, uint32 index) -> bool {
      LINA_INFO("Entity: {0}", e->GetName());
      if (e->GetName().compare("EnemySpawn") == 0) {
        this->m_enemySpawns.push_back(e);
      }
      return false;
    });
    
    LINA_INFO("EnemySpawns: {0}", m_enemySpawns.size());
    
    // Placeholder: spawn a single enemy at each enemy spawner
    EntityTemplate* enemyTemplate = GetEntityTemplate("Enemy_1");
    if (enemyTemplate != nullptr) {
      for (Entity* spawn : m_enemySpawns) {
        Enemy* enemy = new Enemy(m_world, enemyTemplate, m_player);
        enemy->m_entity->SetPosition(spawn->GetPosition());
        enemy->m_entity->SetRotation(spawn->GetRotation());
      }
    }
    
//    if (m_resources.contains("Enemy_1")) {
////      ResourceID rid = m_resources["Enemy_1"].valRes;
////      EntityTemplate* enemyTemplate = m_resources["Enemy_1"].valRes;
//      for (Entity* spawn : m_enemySpawns) {
//        Enemy* enemy = new Enemy(m_world, enemyTemplate);
//      }
//    } else {
//      LINA_ERROR("No Enemy_1 template resource found");
//    }
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
		m_player->Tick(dt);
	}
  
  EntityTemplate* Game::GetEntityTemplate(String key) {
    if (m_resources.contains(key)) {
      EntityParameter param = m_resources[key];
      if (param.type != EntityParameterType::ResourceID) {
        LINA_ERR("Parameter {0} is not a resource", key);
        return nullptr;
      }
      
      ResourceID rid = m_resources[key].valRes;
      ResourceManagerV2 *rm = m_world->GetResourceManager();
      EntityTemplate *entity_template = rm->GetIfExists<EntityTemplate>(rid);
      
      if (entity_template == nullptr) {
        LINA_ERR("Failed to get EntityTemplate for key {0} ", key);
        return nullptr;
      }
      
      return entity_template;
    } else {
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

} // namespace Lina
