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

	class EntityWorld;
	class Entity;
	class Player;
	class EntityTemplate;
	class BubbleManager;
	class Game;
	class Material;
	class Application;

	class WeaponAnimation
	{
	public:
		void Tick(float dt, Material* weaponMaterial);

		void AddTextureID(ResourceID id);

		void SetApp(Application* app)
		{
			m_app = app;
		}

		void SetDisplayFrames(uint32 display)
		{
			m_displayFrames = display;
		}

	private:
		void UpdateMaterial(Material* mat);

	private:
		uint32			   m_displayFrames = 0;
		uint32			   m_index		   = 0;
		Vector<ResourceID> m_textureIDs;
		uint32			   m_frameCtr  = 0;
		Application*	   m_app	   = nullptr;
		ResourceID		   m_textureID = 0;
	};

	class Weapon
	{
	public:
		struct Movement
		{
			float bobPower	 = 4.0f;
			float bobSpeed	 = 8.0f;
			float swayPowerX = 0.5f;
			float swayPowerY = 0.5f;
			float swaySpeed	 = 15.0f;
		};

		struct Runtime
		{
			Vector3 startLocalPos		= Vector3::Zero;
			Vector3 startLocalEuler		= Vector3::Zero;
			Vector3 localPositionOffset = Vector3::Zero;
		};

		Weapon(EntityWorld* world, Player* player, BubbleManager* bm, Application* app);
		virtual ~Weapon();

		virtual void Tick(float dt);
		virtual void Fire() = 0;

		Movement	   m_movement	   = {};
		Runtime		   m_runtime	   = {};
		Entity*		   m_entity		   = nullptr;
		EntityWorld*   m_world		   = nullptr;
		Player*		   m_player		   = nullptr;
		BubbleManager* m_bubbleManager = nullptr;
		Application*   m_app		   = nullptr;
	};

	class WeaponMelee : public Weapon
	{
	public:
		WeaponMelee(EntityWorld* world, Player* player, BubbleManager* bm, Application* app);
		virtual ~WeaponMelee() = default;

		virtual void Tick(float dt) override;
		virtual void Fire() override;

	private:
		WeaponAnimation m_idleAnim;
	};

} // namespace Lina
