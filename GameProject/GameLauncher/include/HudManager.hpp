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
		//	private:
    float m_timer = 0;
    String m_text = "";
		Game* m_game = nullptr;
    Entity* m_hudEntity = nullptr;
	};
} // namespace Lina
