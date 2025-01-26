#include "HudManager.hpp"
#include "Player.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Physics/PhysicsWorld.hpp"
#include "Core/World/Components/CompWidget.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Game.hpp"

namespace Lina
{
	HudManager::HudManager(Game* g) : m_game(g)
	{
		EntityWorld* world = g->m_world;
		m_hudEntity		   = world->FindEntity("GUIHud");

		m_playerHud = world->FindEntity("PlayerHud");
		if (m_playerHud)
		{
			CompWidget* ph = world->GetComponent<CompWidget>(m_playerHud);
			m_plHudManager = &ph->GetWidgetManager();
		}

		if (m_hudEntity == nullptr)
		{
			LINA_ERR("GUIHud not found!");
			return;
		}

		SetMainText("Hello");
	}

	HudManager::~HudManager()
	{
	}

	void HudManager::SetTopText(String text)
	{
		m_topText = text;
	}

	void HudManager::SetTopRight(String text)
	{
		m_topRightText = text;
	}

	void HudManager::SetMainText(String text)
	{
		m_text	= text;
		m_timer = 3.0f;
	}

	void HudManager::Tick(float dt)
	{

		if (m_plHudManager)
		{
			Widget* health = m_plHudManager->GetRoot()->FindChildWithDebugName("HealthDisplay");
			Widget* heat   = m_plHudManager->GetRoot()->FindChildWithDebugName("HeatDisplay");

			if (health)
			{
				health->SetAlignedSizeY(Math::Remap(m_game->m_player->m_health, 0.0f, 100.0f, 0.01f, 0.6f));
			}

			if (heat)
			{
				heat->SetAlignedSizeY(Math::Remap(m_game->m_heatLevel, 0.0f, 100.0f, 0.01f, 0.7f));
			}
		}

		m_timer -= dt;
		if (m_timer <= 0.0f)
			m_text = "";

		EntityWorld* world = m_game->m_world;

		CompWidget* cw = world->GetComponent<CompWidget>(m_hudEntity);
		if (cw)
		{
			WidgetManager& wm = cw->GetWidgetManager();

			Widget* mainText = wm.GetRoot()->FindChildWithDebugName("MainText");
			if (mainText)
				static_cast<Text*>(mainText)->UpdateTextAndCalcSize(m_text);

			Widget* topText = wm.GetRoot()->FindChildWithDebugName("TopText");
			if (topText)
				static_cast<Text*>(topText)->UpdateTextAndCalcSize(m_topText);

			Widget* topRightText = wm.GetRoot()->FindChildWithDebugName("TopRightText");
			if (topRightText)
				static_cast<Text*>(topRightText)->UpdateTextAndCalcSize(m_topRightText);
		}
	}
} // namespace Lina
