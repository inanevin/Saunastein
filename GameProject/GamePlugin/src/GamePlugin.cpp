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
#include "Common/Math/Vector.hpp"
#include "Common/Reflection/ReflectionSystem.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Core/System/PluginInterface.hpp"
#include "Core/World/EntityTemplate.hpp"
#include "Core/World/EntityWorld.hpp"
#include "LinaGX/Core/InputMappings.hpp"

namespace Lina {
SystemInitializationInfo Lina_GetInitInfo() { return {}; }

void GamePlugin::OnAttached() {
  Log::SetLog(m_interface->GetLog());
  LINA_TRACE("GamePlugin -> Attached.");
}

void GamePlugin::OnDetached() { LINA_TRACE("GamePlugin -> Detaching."); }

void GamePlugin::OnSetPlayMode(EntityWorld *world, PlayMode mode) {
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

Saunastein::Saunastein(EntityWorld *world) : m_world(world) {
  Entity *resman = m_world->FindEntity("ResourceManager");
  
  if (resman == NULL) {
    LINA_ERR("ResourceManager not found");
    return;
  }
  
  for (auto param : resman->GetParams().params) {
    m_resources[param.name] = param;
    LINA_INFO("Resource: {0}", param.name);
  }
  
  // Setup camera
  Camera &camera = m_world->GetWorldCamera();
  camera.SetPosition(Vector3(0.0, 1.0, 0.0));
  camera.SetRotation(Quaternion::Identity());
  
  // Span suzannes
  EntityTemplate *entity_template = GetEntityTemplate("ent_suzanne");
  if (entity_template == NULL)
    return;
  
  for (uint32_t x = 0; x < 4; ++x) {
    for (uint32_t y = 0; y < 4; ++y) {
      for (uint32_t z = 0; z < 4; ++z) {
        Entity *entity = SpawnEntity(entity_template);
        if (entity == NULL)
          break;
        entity->SetPosition(Vector3(x * 4.0f, y * 4.0f, z * 4.0f));
        m_entities.push_back(entity);
      }
    }
  }
}

Saunastein::~Saunastein() {
  for (auto entity : m_entities) {
    m_world->DestroyEntity(entity);
  }
}

EntityTemplate *Saunastein::GetEntityTemplate(String key) {
  EntityParameter param = m_resources[key];
  if (param.type != EntityParameterType::ResourceID) {
    LINA_ERR("Resource {0} is not an ResourceID", key);
    return NULL;
  }
  
  ResourceID rid = m_resources[key].valRes;
  LINA_INFO("RID: {}", rid);
  ResourceManagerV2 *rm = m_world->GetResourceManager();
  EntityTemplate *entity_template = rm->GetIfExists<EntityTemplate>(rid);
  
  if (entity_template == NULL) {
    LINA_ERR("Failed to get EntityTemplate for {0}", key);
    return NULL;
  }
  
  return entity_template;
}

Entity *Saunastein::SpawnEntity(EntityTemplate *entity_template) {
  return m_world->SpawnTemplate(entity_template);
}

Entity *Saunastein::SpawnEntity(String key) {
  EntityTemplate *entity_template = GetEntityTemplate(key);
  
  if (entity_template == NULL) {
    LINA_ERR("Failed to get EntityTemplate for {0}", key);
    return NULL;
  }
  
  return SpawnEntity(entity_template);
}

void Saunastein::OnComponentAdded(Component *c) {}

void Saunastein::OnComponentRemoved(Component *c) {}

void Saunastein::OnWorldTick(float delta, PlayMode playmode) {
  WorldInput input = m_world->GetInput();
  Camera &camera = m_world->GetWorldCamera();
  
  { // Test camera
    bool input_forward = input.GetKey(LINAGX_KEY_W);
    bool input_backward = input.GetKey(LINAGX_KEY_S);
    bool input_left = input.GetKey(LINAGX_KEY_A);
    bool input_right = input.GetKey(LINAGX_KEY_D);
    
    bool input_turn_left = input.GetKey(LINAGX_KEY_LEFT);
    bool input_turn_right = input.GetKey(LINAGX_KEY_RIGHT);
    bool input_shoot = input.GetKey(LINAGX_KEY_SPACE);
    
    const float max_movement_mag = 10.0f;
    const float accel = 1.0f;
    const float turn_speed = 120.0f * delta;
    
    Vector3 camera_pos = camera.GetPosition();
    Quaternion camera_rot = camera.GetRotation();
    
    bool has_move_input = input_forward || input_backward || input_left || input_right;
    Vector3 speed = Vector3::Zero;
    
    if (input_forward) {
      speed.z += accel;
    }
    if (input_backward) {
      speed.z -= accel;
    }
    if (input_left) {
      speed.x -= accel;
    }
    if (input_right) {
      speed.x += accel;
    }
    if (input_turn_left) {
      camera_rot = camera_rot * Quaternion(Vector3::Up, -turn_speed);
    }
    if (input_turn_right) {
      camera_rot = camera_rot * Quaternion(Vector3::Up, turn_speed);
    }
    
    if (has_move_input) {
      m_player_movement += speed;
      float mag = m_player_movement.Magnitude();
      mag = mag > max_movement_mag ? max_movement_mag : mag;
      m_player_movement.Normalize();
      m_player_movement *= mag;
    } else {
      m_player_movement *= 0.75f;
    }
    
    camera_pos += camera_rot * m_player_movement * delta;
    camera.SetPosition(camera_pos);
    camera.SetRotation(camera_rot);

    camera.Calculate(m_world->GetScreen().GetRenderSize());
  }
  
}
} // namespace Lina
