/*
This file is a part of: LinaVG
https://github.com/inanevin/LinaVG

Author: Inan Evin
http://www.inanevin.com

The 2-Clause BSD License

Copyright (c) [2022-] Inan Evin

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this
	  list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "Main.hpp"
#include "Backends/GLFWWindow.hpp"
#include "Backends/GL/GLBackend.hpp"
#include <iostream>
#include <chrono>
#include <chrono>

int main(int argc, char* argv[])
{
	LinaVG::Examples::ExampleApp app;
	app.Run();
	return 0;
}

namespace LinaVG
{
	namespace Examples
	{
		ExampleApp* ExampleApp::s_exampleApp = nullptr;

		void ExampleApp::Run()
		{
			s_exampleApp = this;
			GLFWWindow window;

			const unsigned int sizeX = 1440;
			const unsigned int sizeY = 960;

			// Initialize example exampleBackend.
			window.InitWindow(static_cast<int>(sizeX), static_cast<int>(sizeY));

			// Setup Lina VG config.
			GLBackend::s_displayPosX   = 0;
			GLBackend::s_displayPosY   = 0;
			GLBackend::s_displayWidth  = sizeX;
			GLBackend::s_displayHeight = sizeY;

			LinaVG::Config.errorCallback = [](const std::string& err) {
				std::cerr << err.c_str() << std::endl;
			};

			LinaVG::Config.logCallback = [](const std::string& log) {
				std::cout << log.c_str() << std::endl;
			};

			LinaVG::Config.defaultBufferReserve = 100000;
			LinaVG::Config.gcCollectInterval	= 20000;
			LinaVG::Config.maxFontAtlasSize		= 1024;

			LinaVG::InitializeText();

			m_checkeredTexture = GLBackend::LoadTexture("Resources/Textures/Checkered.png");
			m_linaTexture	   = GLBackend::LoadTexture("Resources/Textures/Lina.png");

			// Init LinaVG
			m_renderingBackend = new GLBackend();

			m_lvgDrawer.GetCallbacks().draw			  = std::bind(&GLBackend::DrawDefault, m_renderingBackend, std::placeholders::_1);
			m_lvgText.GetCallbacks().atlasNeedsUpdate = std::bind(&GLBackend::OnAtlasUpdate, m_renderingBackend, std::placeholders::_1);
			m_demoScreens.Initialize();

			float prevTime	  = window.GetTime();
			float lastFPSTime = window.GetTime();
			int	  frameCount  = 0;

			// Application loop.
			while (!m_shouldClose)
			{
				float now	= window.GetTime();
				m_deltaTime = now - prevTime;
				prevTime	= now;
				m_elapsedTime += m_deltaTime;

				if (now > lastFPSTime + 1.0f)
				{
					m_fps			= frameCount;
					frameCount		= 0;
					lastFPSTime		= now;
					m_deltaTimeRead = m_deltaTime;
				}

				// Example exampleBackend input & rendering.
				window.Poll();
				window.Clear();

				// Lina VG start frame.
				m_renderingBackend->StartFrame();

				// Setup style.
				StyleOptions style;
				style.outlineOptions.thickness	   = 2.0f;
				style.outlineOptions.drawDirection = OutlineDrawDirection::Inwards;

				// Same options as style.m_color
				style.outlineOptions.color.start = Vec4(1, 0, 0, 1);
				style.outlineOptions.color.end	 = Vec4(0, 0, 1, 1);

				// Same options as style.m_textureXXX
				style.outlineOptions.textureHandle			= ExampleApp::Get()->GetCheckeredTexture();
				style.outlineOptions.textureTilingAndOffset = Vec4(1.0f, 1.0f, 0.0f, 0.0f);

				m_demoScreens.ShowBackground();

				auto demoNow = std::chrono::high_resolution_clock::now();

				if (m_currentDemoScreen == 1)
					m_demoScreens.ShowDemoScreen1_Shapes();
				else if (m_currentDemoScreen == 2)
					m_demoScreens.ShowDemoScreen2_Colors();
				else if (m_currentDemoScreen == 3)
					m_demoScreens.ShowDemoScreen3_Outlines();
				else if (m_currentDemoScreen == 4)
					m_demoScreens.ShowDemoScreen4_Lines();
				else if (m_currentDemoScreen == 5)
					m_demoScreens.ShowDemoScreen5_Texts();
				else if (m_currentDemoScreen == 6)
					m_demoScreens.ShowDemoScreen6_DrawOrder();
				else if (m_currentDemoScreen == 7)
					m_demoScreens.ShowDemoScreen7_Clipping();
				else if (m_currentDemoScreen == 8)
					m_demoScreens.ShowDemoScreen8_Animated();
				else if (m_currentDemoScreen == 9)
					m_demoScreens.ShowDemoScreen9_Final();

				auto demoNow2			 = std::chrono::high_resolution_clock::now();
				auto duration			 = std::chrono::duration_cast<std::chrono::nanoseconds>(demoNow2 - demoNow);
				m_demoScreens.m_screenMS = static_cast<float>(duration.count()) * 1e-6f;

				// Flush everything we've drawn so far.
				m_lvgDrawer.FlushBuffers();
				m_lvgDrawer.ResetFrame();

				m_demoScreens.PreEndFrame();
				m_renderingBackend->EndFrame();

				// Backend window swap buffers.
				window.SwapBuffers();
				frameCount++;
			}

			delete m_linaTexture;
			delete m_checkeredTexture;
			// Terminate Lina VG & example exampleBackend.
			m_demoScreens.Terminate();
			delete m_renderingBackend;
			LinaVG::TerminateText();
			window.Terminate();
		}

		void ExampleApp::OnHorizontalKeyCallback(float input)
		{
			GLBackend::s_debugOffset.x += input * m_deltaTime * 1000;
		}

		void ExampleApp::OnVerticalKeyCallback(float input)
		{
			GLBackend::s_debugOffset.y -= input * m_deltaTime * 1000;
		}

		void ExampleApp::OnNumKeyCallback(int key)
		{
			if (key > 0 && key < 10)
				m_currentDemoScreen = key;

			if (key == 7)
				m_demoScreens.m_clippingEnabled = true;
		}

		void ExampleApp::OnPCallback()
		{
			m_demoScreens.m_statsWindowOn = !m_demoScreens.m_statsWindowOn;
		}

		void ExampleApp::OnRCallback()
		{
			m_demoScreens.m_rotate = !m_demoScreens.m_rotate;
		}

		void ExampleApp::OnFCallback()
		{
			GLBackend::s_debugWireframe = !GLBackend::s_debugWireframe;
		}

		void ExampleApp::OnCCallback()
		{
			if (m_currentDemoScreen == 7)
				m_demoScreens.m_clippingEnabled = !m_demoScreens.m_clippingEnabled;
		}

		void ExampleApp::OnECallback()
		{
			m_demoScreens.m_rotateAngle = 0.0f;
		}

		void ExampleApp::OnMouseScrollCallback(float val)
		{
			GLBackend::s_debugZoom -= val * m_deltaTime * 10;
		}

		void ExampleApp::OnWindowResizeCallback(int width, int height)
		{
			GLBackend::s_displayWidth  = static_cast<BackendHandle>(width);
			GLBackend::s_displayHeight = static_cast<BackendHandle>(height);
		}
		void ExampleApp::OnWindowCloseCallback()
		{
			m_shouldClose = true;
		}
	} // namespace Examples
} // namespace LinaVG
