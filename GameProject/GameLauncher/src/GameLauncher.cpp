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

#include "GameLauncher.hpp"
#include "SwapchainRenderer.hpp"
#include "Core/Lina.hpp"
#include "Core/Graphics/Renderers/WorldRenderer.hpp"
#include "Common/FileSystem/FileSystem.hpp"
#include "Common/Serialization/Serialization.hpp"
#include "Core/Platform/PlatformProcess.hpp"
#include "Core/World/EntityWorld.hpp"
#include "Core/World/Components/CompAudio.hpp"
#include "Core/World/Components/CompWidget.hpp"
#include "Core/World/Components/CompLight.hpp"
#include "Core/GUI/Theme.hpp"
#include "Core/GUI/Widgets/Widget.hpp"
#include "Core/GUI/Widgets/Primitives/Text.hpp"
#include "Core/GUI/Widgets/Primitives/Button.hpp"

namespace Lina
{

#define PACKAGE_0_PATH "LinaPackage0.linapkg"
#define PACKAGE_1_PATH "LinaPackage1.linapkg"

#define FULLSCREEN 1

	SystemInitializationInfo Lina_GetInitInfo()
	{
		const SystemInitializationInfo outInfo = SystemInitializationInfo{
			.appName	  = "Lina Game",
			.windowWidth  = 800,
			.windowHeight = 600,
			.windowStyle  = WindowStyle::Borderless,
			.appDelegate  = new GameLauncher(),
		};
		return outInfo;
	}

	bool Lina::GameLauncher::PreInitialize(String& errString)
	{
		Theme::GetDef().defaultFont = ENGINE_FONT_ROBOTO_ID;
		m_pluginInterface.SetLog(m_app->GetLog());

		if (!LoadGamePlugin(errString))
			return false;

		if (!VerifyPackages(errString))
			return false;

		const String config	  = LINA_CONFIGURATION;
		const String pkg0Path = config.compare("Debug") == 0 ? "../Release/LinaPackage0.linapkg" : "LinaPackage0.linapkg";
		const String pkg1Path = config.compare("Debug") == 0 ? "../Release/LinaPackage1.linapkg" : "LinaPackage1.linapkg";

		IStream stream = Serialization::LoadFromFile(pkg0Path.c_str());
		uint32	pjSize = 0;
		stream >> pjSize;
		m_project.LoadFromStream(stream);
		stream.Destroy();

		m_app->GetResourceManager().SetUsePackages(pkg0Path, pkg1Path);
		m_app->GetResourceManager().LoadResourcesFromProject(&m_project, {ENGINE_SHADER_SWAPCHAIN_ID}, NULL, 0);

		// Check for Game Plugin
		// Check for Resources

		return true;
	}

	bool GameLauncher::Initialize(String& err)
	{
		LinaGX::Window* main = m_app->GetApplicationWindow(LINA_MAIN_SWAPCHAIN);
		main->AddListener(this);
		m_window			= main;
		m_lgx				= m_app->GetLGX();
		m_swapchainRenderer = new SwapchainRenderer(m_app, m_lgx, main, false);
		return true;
	}

	void GameLauncher::PostInitialize()
	{
		const Vector<ResourceID>& worlds = m_project.GetSettingsPackaging().worldIDsToPack;
		if (!worlds.empty())
		{
			ResourceID first = worlds.at(0);
			m_app->CreateAndLoadWorld(&m_project, first, m_window);
		}
	}

	void GameLauncher::OnWorldLoaded(WorldRenderer* wr)
	{
		m_wr	= wr;
		m_world = m_wr->GetWorld();
		m_swapchainRenderer->SetWorldRenderer(m_wr);
		m_world->GetWorldCamera().SetPosition(Vector3(0, 0, 0));
		m_world->GetWorldCamera().Calculate(m_wr->GetSize());

		if (FULLSCREEN)
		{
#ifdef LINA_PLATFORM_APPLE
			m_window->SetSize(m_window->GetMonitorWorkSize());
#else
			m_window->SetSize(m_window->GetMonitorSize());
#endif
			m_window->SetPosition({});
		}
	}

	void GameLauncher::OnWorldUnloaded(ResourceID id)
	{
		m_game.OnGameEnd();

		m_wr	= nullptr;
		m_world = nullptr;
		m_swapchainRenderer->SetWorldRenderer(nullptr);
	}

	void GameLauncher::PreTick()
	{
		if (m_wr)
			m_wr->Resize(m_window->GetSize() * m_window->GetDPIScale());

		m_swapchainRenderer->OnWindowSizeChanged(m_window, m_window->GetSize());

		m_swapchainRenderer->CheckVisibility();

		if (m_world)
		{
			m_world->GetScreen().SetRenderSize(m_wr->GetSize());
			m_world->GetScreen().SetDisplaySize(m_wr->GetSize());
			m_world->GetScreen().SetOwnerWindow(m_window);

			if (!m_gameBegun)
			{
				m_gameBegun = true;
				m_game.OnGameBegin(m_world);
			}
		}
	}

	void GameLauncher::Tick(float delta)
	{
		m_game.OnGamePreTick(delta);

		if (m_world)
			m_world->Tick(delta);

		m_game.OnGameTick(delta);

		if (m_wr)
			m_wr->Tick(delta);
		m_swapchainRenderer->Tick();
	}

	void GameLauncher::PreShutdown()
	{
		if (m_gamePlugin)
		{
			PlatformProcess::UnloadPlugin(m_gamePlugin);
		}

		if (m_wr)
			m_app->UnloadWorld(m_wr);

		m_app->GetApplicationWindow(LINA_MAIN_SWAPCHAIN)->RemoveListener(this);
		delete m_swapchainRenderer;
	}

	void GameLauncher::SyncRender()
	{
		if (m_wr)
			m_wr->SyncRender();
		m_swapchainRenderer->SyncRender();
	}

	void GameLauncher::Render(uint32 frameIndex)
	{
		if (m_wr)
		{
			m_wr->UpdateBuffers(frameIndex);
			m_wr->FlushTransfers(frameIndex);
			m_wr->Render(frameIndex);
			m_wr->CloseAndSend(frameIndex);
		}

		Vector<uint16> waitSemaphores;
		Vector<uint64> waitValues;

		if (m_wr)
		{
			const SemaphoreData& sem = m_wr->GetSubmitSemaphore(frameIndex);
			waitSemaphores.push_back(sem.GetSemaphore());
			waitValues.push_back(sem.GetValue());
		}

		if (!m_swapchainRenderer->GetIsVisible())
			return;

		LinaGX::CommandStream* cmd		 = m_swapchainRenderer->Render(frameIndex);
		uint8				   swapchain = m_swapchainRenderer->GetSwapchain();

		m_lgx->SubmitCommandStreams({
			.targetQueue	= m_lgx->GetPrimaryQueue(LinaGX::CommandType::Graphics),
			.streams		= &cmd,
			.streamCount	= static_cast<uint32>(1),
			.useWait		= !waitValues.empty(),
			.waitCount		= static_cast<uint32>(waitValues.size()),
			.waitSemaphores = waitSemaphores.data(),
			.waitValues		= waitValues.data(),
			.useSignal		= false,
		});

		m_lgx->Present({
			.swapchains		= &swapchain,
			.swapchainCount = static_cast<uint32>(1),
		});
	}

	void GameLauncher::OnWindowClose(LinaGX::Window* window)
	{
		m_app->Quit();
	}

	void GameLauncher::OnWindowSizeChanged(LinaGX::Window* window, const LinaGX::LGXVector2ui& sz)
	{
		m_app->JoinRender();

		if (m_wr)
			m_wr->Resize(sz * m_window->GetDPIScale());

		m_swapchainRenderer->OnWindowSizeChanged(window, sz * m_window->GetDPIScale());
	}

	void GameLauncher::OnWindowKey(LinaGX::Window* window, uint32 keycode, int32 scancode, LinaGX::InputAction inputAction)
	{
		if (window != m_window)
			return;

		m_game.OnKey(keycode, scancode, inputAction);

		if (m_world)
			m_world->GetInput().OnKey(keycode, scancode, inputAction);
	}

	void GameLauncher::OnWindowMouse(LinaGX::Window* window, uint32 button, LinaGX::InputAction inputAction)
	{
		if (window != m_window)
			return;

		m_game.OnMouse(button, inputAction);

		if (m_world)
			m_world->GetInput().OnMouse(button, inputAction);
	}

	void GameLauncher::OnWindowMouseWheel(LinaGX::Window* window, float amt)
	{
		if (window != m_window)
			return;

		if (m_world)
			m_world->GetInput().OnMouseWheel(amt);
	}

	void GameLauncher::OnWindowMouseMove(LinaGX::Window* window, const LinaGX::LGXVector2& move)
	{
		if (window != m_window)
			return;

		if (m_world)
			m_world->GetInput().OnMouseMove(move);
	}

	void GameLauncher::OnWindowFocus(LinaGX::Window* window, bool gainedFocus)
	{
		m_game.OnWindowFocus(gainedFocus);
	}

	void GameLauncher::OnWindowHoverBegin(LinaGX::Window* window)
	{
	}

	void GameLauncher::OnWindowHoverEnd(LinaGX::Window* window)
	{
	}

	bool GameLauncher::LoadGamePlugin(String& errString)
	{
		return true;
#ifdef LINA_PLATFORM_WINDOWS
		const String path = "GamePlugin.dll";
#endif

#ifdef LINA_PLATFORM_APPLE
		const String path = "libGamePlugin.dylib";
#endif

		if (!FileSystem::FileOrPathExists(path))
		{
			errString = "GamePlugin could not be found!";
			return false;
		}

		m_gamePlugin = PlatformProcess::LoadPlugin(path, &m_pluginInterface);
		if (!m_gamePlugin)
		{
			errString = "GamePlugin could not be loaded!";
			return false;
		}

		return true;
	}

	bool GameLauncher::VerifyPackages(String& errString)
	{
		const String config = LINA_CONFIGURATION;

		const String pkg0Path = config.compare("Debug") == 0 ? "../Release/LinaPackage0.linapkg" : "LinaPackage0.linapkg";
		const String pkg1Path = config.compare("Debug") == 0 ? "../Release/LinaPackage1.linapkg" : "LinaPackage1.linapkg";

		if (!FileSystem::FileOrPathExists(pkg0Path))
		{
			errString = "Could not locate package LinaPackage0.linapkg";
			return false;
		}

		if (!FileSystem::FileOrPathExists(pkg1Path))
		{
			errString = "Could not locate package LinaPackage1.linapkg";
			return false;
		}

		return true;
	}
} // namespace Lina
