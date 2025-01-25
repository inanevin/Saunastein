#include "HudManager.hpp"
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
    m_hudEntity = world->FindEntity("GUIHud");
  
    if (m_hudEntity == nullptr) {
      LINA_ERR("GUIHud not found!");
      return;
    }
    
    SetMainText("Hello");
	}

	HudManager::~HudManager()
	{
    
	}
  
  void HudManager::SetMainText(String text) {
    m_text = text;
    m_timer = 3.0f;
  }
	
  void HudManager::Tick(float dt)
	{
    m_timer -= dt;
    if (m_timer <= 0.0f) m_text = "";
    
    EntityWorld* world = m_game->m_world;
    
    CompWidget* cw = world->GetComponent<CompWidget>(m_hudEntity);
    
    if (cw)
    {
      WidgetManager& wm = cw->GetWidgetManager();
      Widget* mainText = wm.GetRoot()->FindChildWithDebugName("MainText");
      if (mainText)
      {
        static_cast<Text*>(mainText)->UpdateTextAndCalcSize(m_text);
      }
    }
	}
} // namespace Lina
