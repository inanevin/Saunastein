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

#include "Core/System/Plugin.hpp"
#include "Core/World/EntityWorld.hpp"

namespace Lina
{
    class Saunastein: public EntityWorldListener {
        EntityWorld* m_world = NULL;
        std::map<String, EntityParameter> m_resources;
//        Entity* m_suzanne = NULL;
        Vector<Entity*> m_entities;
        
    public:
        Saunastein(EntityWorld* world);
        ~Saunastein();
        
        virtual void OnComponentAdded(Component* c) override;
        virtual void OnComponentRemoved(Component* c) override;
        virtual void OnWorldTick(float delta, PlayMode playmode) override;
        
        Entity* SpawnEntity(String key);
        Entity* SpawnEntity(EntityTemplate* entity_template);
        EntityTemplate* GetEntityTemplate(String key);
    };

	class GamePlugin : public Plugin
	{
        Saunastein* saunastein = NULL;
	public:
		GamePlugin(const String& path, void* platformHandle, PluginInterface* interface) : Plugin(path, platformHandle, interface){};
		virtual ~GamePlugin() = default;

		// Inherited via IPlugin
		virtual void OnAttached() override;
		virtual void OnDetached() override;
        virtual void OnSetPlayMode(EntityWorld* world, PlayMode mode) override;
	};

} // namespace Lina
