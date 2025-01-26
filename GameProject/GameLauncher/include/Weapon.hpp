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
#include "Common/SizeDefinitions.hpp"

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
	class AudioManager;

	class WeaponAnimation
	{
	public:
		WeaponAnimation(EntityWorld* world, Application* app);

		inline void Idle()
		{
			m_used = m_idle;
		}

		inline void Fire()
		{
			m_ctr  = 0;
			m_used = m_fire;
		}

		void Tick(float dt);

		void UpdateMaterial();

		uint32		 m_ctr		   = 0;
		uint32		 m_fireDisplay = 0;
		ResourceID	 m_idle		   = 0;
		ResourceID	 m_fire		   = 0;
		ResourceID	 m_run		   = 0;
		ResourceID	 m_used		   = 0;
		Material*	 m_material	   = nullptr;
		Application* m_app		   = nullptr;
		bool		 m_isRunning;
	};

	class Weapon
	{
	public:
		struct Movement
		{
			float	bobPower	  = 4.0f;
			float	bobSpeed	  = 8.0f;
			float	swayPowerX	  = 0.5f;
			float	swayPowerY	  = 0.5f;
			float	swaySpeed	  = 15.0f;
			Vector3 localOffset	  = Vector3::Zero;
			Vector3 holsterOffset = Vector3::Zero;
			Vector3 runningOffset = Vector3::Zero;
			float	holsterAlpha  = 0.0f;
			bool	spawnLight	  = false;
		};

		struct Runtime
		{
			Vector3 startLocalEuler		= Vector3::Zero;
			Vector3 localPositionOffset = Vector3::Zero;
			bool	isRunning			= false;
		};

		Weapon(EntityWorld* world, Player* player, BubbleManager* bm, Application* app, AudioManager* am);
		virtual ~Weapon();

		virtual void Tick(float dt);

		void	Fire();
		void	SetAnim(const String& animIdle, const String& animFire, const String& run);
		Vector3 CalculateTargetPosition(const Vector2& md);

		inline void MoveAway()
		{
			m_isMovingAway = true;
		}

		Movement		 m_movement		 = {};
		Runtime			 m_runtime		 = {};
		Entity*			 m_entity		 = nullptr;
		Entity*			 m_light		 = nullptr;
		EntityWorld*	 m_world		 = nullptr;
		Player*			 m_player		 = nullptr;
		BubbleManager*	 m_bubbleManager = nullptr;
		Application*	 m_app			 = nullptr;
		WeaponAnimation* m_animation	 = nullptr;
		bool			 m_isMovingAway	 = false;
		bool			 m_movedAway	 = false;
		float			 m_swayAlpha	 = 0.0f;
		uint32			 m_lightCtr		 = 0;
		bool			 m_isPistolHack	 = false;
		AudioManager*	 m_audioManager	 = nullptr;
	};

} // namespace Lina
