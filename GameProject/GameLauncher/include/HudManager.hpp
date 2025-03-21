#pragma once

#include "Common/Math/Vector.hpp"
#include "Common/Math/Quaternion.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Game.hpp"

namespace LinaGX
{

} // namespace LinaGX

namespace Lina
{
	class EntityWorld;
	class Entity;
	class Player;

	class HudManager
	{
	public:
		HudManager(Game* g);

		void Tick(float dt);
		~HudManager();

		void SetMainText(String text);
		void SetTopText(String text);
		void SetTopRight(String text);

		//	private:
		float		   m_timer		  = 0;
		String		   m_text		  = "";
		String		   m_topText	  = "";
		String		   m_topRightText = "";
		Game*		   m_game		  = nullptr;
		Entity*		   m_hudEntity	  = nullptr;
		Entity*		   m_playerHud	  = nullptr;
		WidgetManager* m_plHudManager = nullptr;
	};
} // namespace Lina
