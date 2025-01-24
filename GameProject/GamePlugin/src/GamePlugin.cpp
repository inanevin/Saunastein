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

#include "GamePlugin.hpp"
#include "Core/System/PluginInterface.hpp"
#include "Common/Reflection/ReflectionSystem.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "LinaGX/Core/InputMappings.hpp"
#include "Common/Math/Vector.hpp"

namespace Lina
{
    SystemInitializationInfo Lina_GetInitInfo()
    {
        return {};
    }

	void GamePlugin::OnAttached()
	{
		Log::SetLog(m_interface->GetLog());
		LINA_TRACE("GamePlugin -> Attached.");
	}

	void GamePlugin::OnDetached()
	{
		LINA_TRACE("GamePlugin -> Detaching.");
	}
    
    void GamePlugin::OnSetPlayMode(EntityWorld* world, PlayMode mode)
    {
        LINA_TRACE("OnSetPlayMode");

        if (mode == PlayMode::None && this->saunastein != NULL) {
            world->RemoveListener(this->saunastein);
            delete this->saunastein;
        }
        
        if (mode == PlayMode::Play) {
            this->saunastein = new Saunastein(world);
            world->AddListener(this->saunastein);
        }
    }
    
    Saunastein::Saunastein(EntityWorld* world): m_world(world) {
        Entity* resman = m_world->FindEntity("ResourceManager");
        
        if (resman == NULL) {
            LINA_ERR("ResourceManager not found");
            return;
        }
        
        for (auto param : resman->GetParams().params) {
            m_resources[param.name] = param;
            LINA_INFO("Resource: {0}", param.name);
        }
        
        EntityTemplate* templ = GetEntityTemplate("suzanne");
        if (templ == NULL) {
            LINA_ERR("suzanne not found");
            return;
        }
        
        // Span suzannes
        EntityTemplate* entity_template = GetEntityTemplate("suzanne");
        if (entity_template == NULL) return;
        
        for (uint32_t x = 0; x < 8; ++x) {
            for (uint32_t y = 0; y < 8; ++y) {
                for (uint32_t z = 0; z < 8; ++z) {
                    Entity* entity = SpawnEntity(entity_template);
                    if (entity == NULL) break;
                    entity->SetPosition(Vector3(x * 4.0f, y * 4.0f, z * 4.0f));
                    m_entities.push_back(entity);
                }
            }
        }
    }
    
    EntityTemplate* Saunastein::GetEntityTemplate(String key) {
        EntityParameter param = m_resources[key];
        if (param.type != EntityParameterType::ResourceID) {
            LINA_ERR("Resource {0} is not an ResourceID", key);
            return NULL;
        }
        
        ResourceID rid = m_resources[key].valRes;
        ResourceManagerV2* rm = m_world->GetResourceManager();
        EntityTemplate* entity_template = rm->GetIfExists<EntityTemplate>(rid);
        
        if (entity_template == NULL) {
            LINA_ERR("Failed to get EntityTemplate for {0}", key);
            return NULL;
        }
        
        return entity_template;
    }
    
    Entity* Saunastein::SpawnEntity(EntityTemplate* entity_template) {
        return m_world->SpawnTemplate(entity_template);
    }

    Entity* Saunastein::SpawnEntity(String key) {
        EntityTemplate* entity_template = GetEntityTemplate(key);
        
        if (entity_template == NULL) {
            LINA_ERR("Failed to get EntityTemplate for {0}", key);
            return NULL;
        }
        
        return SpawnEntity(entity_template);
    }
    
    void Saunastein::OnComponentAdded(Component* c) {
    
    }

    void Saunastein::OnComponentRemoved(Component* c) {
        
    }

    void Saunastein::OnWorldTick(float delta, PlayMode playmode) {
//        for (auto entity : m_entities) {
//            entity->AddPosition(Vector3(1.0, 0.0, 0.0) * delta);
//            entity->AddRotation(Vector3(90.0, 45.0, 30.0) * delta);
//        }
        WorldInput input = m_world->GetInput();
        
        bool input_forward = input.GetKeyDown(LINAGX_KEY_W);
        bool input_backward = input.GetKeyDown(LINAGX_KEY_S);
        bool input_left = input.GetKeyDown(LINAGX_KEY_A);
        bool input_right = input.GetKeyDown(LINAGX_KEY_D);
        
        const float movement_speed = 5.0f;
        
        for (auto entity : m_entities) {
            if (input_forward) entity->AddPosition(Vector3::Forward * movement_speed * delta);
            if (input_backward) entity->AddPosition(-Vector3::Forward * movement_speed * delta);
            if (input_left) entity->AddPosition(-Vector3::Right * movement_speed * delta);
            if (input_right) entity->AddPosition(Vector3::Right * movement_speed * delta);
        }
    }

} // namespace Lina
