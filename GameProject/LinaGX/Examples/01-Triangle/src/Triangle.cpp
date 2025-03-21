/*
This file is a part of: LinaGX
https://github.com/inanevin/LinaGX

Author: Inan Evin
http://www.inanevin.com

The 2-Clause BSD License

Copyright (c) [2023-] Inan Evin

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

/*

Example: Triangle

Simple example drawing a UV triangle in the screen with hardcoded vertices in the shader.

Demonstrates:
- Window creation.
- Shader compilation & pipeline creation.
- Swapchain creation & handling resizing.
- Render passes and drawing.
- Frame submission and presentation.

*/

#include "App.hpp"
#include "LinaGX/LinaGX.hpp"
#include "Triangle.hpp"
#include <iostream>
#include <cstdarg>
#include <LinaGX/Utility/PlatformUtility.hpp>

namespace LinaGX::Examples
{

#define MAIN_WINDOW_ID   0
#define FRAMES_IN_FLIGHT 2
#define BACK_BUFFER      2

    LinaGX::Instance* _lgx       = nullptr;
    uint8             _swapchain = 0;
    Window*           _window    = nullptr;
    uint32            _windowX   = 0;
    uint32            _windowY   = 0;

    // Shaders.
    uint16 _shaderProgram = 0;

    struct Vertex
    {
        float position[3];
        float color[3];
    };

    struct PerFrameData
    {
        LinaGX::CommandStream* stream = nullptr;
    };

    PerFrameData _pfd[FRAMES_IN_FLIGHT];

    void Example::Initialize()
    {
        App::Initialize();

        //******************* CONFIGURATION & INITIALIZATION
        {
            BackendAPI api = BackendAPI::DX12;

#ifdef LINAGX_PLATFORM_APPLE
            api = BackendAPI::Metal;
#endif

            LinaGX::Config.api             = api;
            LinaGX::Config.gpu             = PreferredGPUType::Discrete;
            LinaGX::Config.framesInFlight  = FRAMES_IN_FLIGHT;
            LinaGX::Config.backbufferCount = BACK_BUFFER;
            LinaGX::Config.gpuLimits       = {};
            LinaGX::Config.logLevel        = LogLevel::Verbose;
            LinaGX::Config.errorCallback   = LogError;
            LinaGX::Config.infoCallback    = LogInfo;
            _lgx                           = new LinaGX::Instance();
            _lgx->Initialize();

            std::vector<LinaGX::Format> formatSupportCheck = {LinaGX::Format::B8G8R8A8_UNORM};
            for (auto fmt : formatSupportCheck)
            {
                const LinaGX::FormatSupportInfo fsi = _lgx->GetFormatSupport(fmt);

                if (fsi.format == LinaGX::Format::UNDEFINED)
                    LOGE("Current GPU does not support the formats required by this example!");
            }
        }

        //*******************  WINDOW CREATION & CALLBACKS
        {
            _window = _lgx->GetWindowManager().CreateApplicationWindow(MAIN_WINDOW_ID, "LinaGX Triangle", 0, 0, 800, 600, WindowStyle::WindowedApplication);
            App::RegisterWindowCallbacks(_window);
            _windowX = _window->GetSize().x;
            _windowY = _window->GetSize().y;
        }

        //******************* SHADER CREATION
        {
            // Compile shaders.
            const std::string vtxShader  = LinaGX::ReadFileContentsAsString("Resources/Shaders/vert.glsl");
            const std::string fragShader = LinaGX::ReadFileContentsAsString("Resources/Shaders/frag.glsl");
            ShaderLayout      outLayout  = {};

            LINAGX_VEC<ShaderCompileData> compileData;
            compileData.push_back({
                .stage       = ShaderStage::Vertex,
                .text        = vtxShader,
                .includePath = "Resources/Shaders/Include",
            });

            compileData.push_back({
                .stage       = ShaderStage::Fragment,
                .text        = fragShader,
                .includePath = "Resources/Shaders/Include",
            });

            _lgx->CompileShader(compileData, outLayout);

            // At this stage you could serialize the blobs to disk and read it next time, instead of compiling each time.

            // Create shader program with vertex & fragment stages.
            LinaGX::ShaderColorAttachment colorAttachment = {
                .format = Format::B8G8R8A8_UNORM,
            };

            LinaGX::ShaderDepthStencilDesc depthStencil;
            depthStencil.depthStencilAttachmentFormat = LinaGX::Format::UNDEFINED;

            ShaderDesc shaderDesc = {
                .stages           = compileData,
                .colorAttachments = {colorAttachment},
                .depthStencilDesc = depthStencil,
                .layout           = outLayout,
                .polygonMode      = PolygonMode::Fill,
                .cullMode         = CullMode::None,
                .frontFace        = FrontFace::CCW,
                .topology         = Topology::TriangleList,
            };
            _shaderProgram = _lgx->CreateShader(shaderDesc);

            // Compiled binaries are not needed anymore.
            for (auto& data : compileData)
                free(data.outBlob.ptr);
        }

        //*******************  MISC
        {
            // Create a swapchain for main window.
            _swapchain = _lgx->CreateSwapchain({
                .format       = Format::B8G8R8A8_UNORM,
                .x            = 0,
                .y            = 0,
                .width        = _window->GetSize().x,
                .height       = _window->GetSize().y,
                .window       = _window->GetWindowHandle(),
                .osHandle     = _window->GetOSHandle(),
                .isFullscreen = false,
            });

            // Create command stream to record draw calls.
            for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
                _pfd[i].stream = _lgx->CreateCommandStream({CommandType::Graphics, 10, 512, 512, 64});
        }
    } // namespace LinaGX::Examples

    void Example::OnWindowResized(uint32 w, uint32 h)
    {
        LGXVector2ui          monitor    = _window->GetMonitorSize();
        SwapchainRecreateDesc resizeDesc = {
            .swapchain    = _swapchain,
            .width        = w,
            .height       = h,
            .isFullscreen = w == monitor.x && h == monitor.y,
        };
        _lgx->RecreateSwapchain(resizeDesc);
        _windowX = w;
        _windowY = h;
    }

    void Example::Shutdown()
    {
        // First get rid of the window.
        _lgx->GetWindowManager().DestroyApplicationWindow(MAIN_WINDOW_ID);

        // Wait for queues to finish
        _lgx->Join();

        // Get rid of resources
        _lgx->DestroySwapchain(_swapchain);
        _lgx->DestroyShader(_shaderProgram);

        for (uint32 i = 0; i < FRAMES_IN_FLIGHT; i++)
            _lgx->DestroyCommandStream(_pfd[i].stream);

        // Terminate renderer & shutdown app.
        delete _lgx;
    }

    void Example::OnTick()
    {
        // Check for window inputs.
        _lgx->TickWindowSystem();
    }

    void Example::OnRender()
    {
        // Let LinaGX know we are starting a new frame.
        _lgx->StartFrame();

        auto& currentFrame = _pfd[_lgx->GetCurrentFrameIndex()];

        // Barrier to Color Attachment
        {
            LinaGX::CMDBarrier* barrier              = currentFrame.stream->AddCommand<LinaGX::CMDBarrier>();
            barrier->srcStageFlags                   = LinaGX::PSF_TopOfPipe;
            barrier->dstStageFlags                   = LinaGX::PSF_ColorAttachment;
            barrier->textureBarrierCount             = 1;
            barrier->textureBarriers                 = currentFrame.stream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier));
            barrier->textureBarriers->srcAccessFlags = LinaGX::AF_MemoryRead | LinaGX::AF_MemoryWrite;
            barrier->textureBarriers->dstAccessFlags = LinaGX::AF_ColorAttachmentRead;
            barrier->textureBarriers->isSwapchain    = true;
            barrier->textureBarriers->texture        = static_cast<uint32>(_swapchain);
            barrier->textureBarriers->toState        = LinaGX::TextureState::ColorAttachment;
        }

        // Render pass begin
        {
            Viewport                          viewport = {.x = 0, .y = 0, .width = _windowX, .height = _windowY, .minDepth = 0.0f, .maxDepth = 1.0f};
            ScissorsRect                      sc       = {.x = 0, .y = 0, .width = _windowX, .height = _windowY};
            LinaGX::RenderPassColorAttachment att      = {};
            att.isSwapchain                            = true;
            att.texture                                = static_cast<uint32>(_swapchain);
            att.clearColor                             = {32.0f / 255.0f, 17.0f / 255.0f, 39.0f / 255.0f, 1.0f};
            CMDBeginRenderPass* beginRenderPass        = currentFrame.stream->AddCommand<CMDBeginRenderPass>();
            beginRenderPass->colorAttachmentCount      = 1;
            beginRenderPass->colorAttachments          = currentFrame.stream->EmplaceAuxMemory<LinaGX::RenderPassColorAttachment>(att);
            beginRenderPass->viewport                  = viewport;
            beginRenderPass->scissors                  = sc;
        }

        // Set shader
        {
            CMDBindPipeline* bindPipeline = currentFrame.stream->AddCommand<CMDBindPipeline>();
            bindPipeline->shader          = _shaderProgram;
        }

        // Draw the triangle
        {
            CMDDrawInstanced* drawInstanced       = currentFrame.stream->AddCommand<CMDDrawInstanced>();
            drawInstanced->vertexCountPerInstance = 3;
            drawInstanced->instanceCount          = 1;
            drawInstanced->startInstanceLocation  = 0;
            drawInstanced->startVertexLocation    = 0;
        }

        // End render pass
        {
            CMDEndRenderPass* end = currentFrame.stream->AddCommand<CMDEndRenderPass>();
        }

        // Barrier to Present
        {
            LinaGX::CMDBarrier* barrier              = currentFrame.stream->AddCommand<LinaGX::CMDBarrier>();
            barrier->srcStageFlags                   = LinaGX::PSF_ColorAttachment;
            barrier->dstStageFlags                   = LinaGX::PSF_BottomOfPipe;
            barrier->textureBarrierCount             = 1;
            barrier->textureBarriers                 = currentFrame.stream->EmplaceAuxMemorySizeOnly<LinaGX::TextureBarrier>(sizeof(LinaGX::TextureBarrier));
            barrier->textureBarriers->srcAccessFlags = LinaGX::AF_ColorAttachmentWrite;
            barrier->textureBarriers->dstAccessFlags = 0;
            barrier->textureBarriers->isSwapchain    = true;
            barrier->textureBarriers->texture        = static_cast<uint32>(_swapchain);
            barrier->textureBarriers->toState        = LinaGX::TextureState::Present;
        }

        // This does the actual *recording* of every single command stream alive.
        _lgx->CloseCommandStreams(&currentFrame.stream, 1);

        // Submit work on gpu.
        _lgx->SubmitCommandStreams({.targetQueue = _lgx->GetPrimaryQueue(CommandType::Graphics), .streams = &currentFrame.stream, .streamCount = 1});

        // Present main swapchain.
        _lgx->Present({.swapchains = &_swapchain, .swapchainCount = 1});

        // Let LinaGX know we are finalizing this frame.
        _lgx->EndFrame();
    }

} // namespace LinaGX::Examples
