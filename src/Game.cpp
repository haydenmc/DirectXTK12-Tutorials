#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game()
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND hwnd, uint32_t width, uint32_t height)
{
    m_deviceResources->SetWindow(hwnd, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    // TODO: Add your game logic here.
    elapsedTime;

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    float time{ static_cast<float>(m_timer.GetTotalSeconds()) };

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    ID3D12DescriptorHeap* heaps[]{ m_resourceDescriptors->Heap() };
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

    m_spriteBatch->Begin(commandList);
    m_spriteBatch->Draw(
        m_resourceDescriptors->GetGpuHandle(Descriptors::Cat),
        GetTextureSize(m_texture.get()),
        m_screenPos,
        nullptr,
        Colors::White,
        cosf(time) * 4.f,
        m_origin
    );
    m_spriteBatch->End();

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

    PIXEndEvent();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto viewport = m_deviceResources->GetScreenViewport();
    auto scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(uint32_t width, uint32_t height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
std::tuple<uint32_t, uint32_t> Game::GetDefaultSize() const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    return { 800, 600 };
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    // Upload resources
    m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);
    ResourceUploadBatch resourceUpload{ device };
    resourceUpload.Begin();
    DX::ThrowIfFailed(CreateDDSTextureFromFile(
        device,
        resourceUpload,
        L"cat.dds",
        m_texture.put()
    ));

    CreateShaderResourceView(
        device,
        m_texture.get(),
        m_resourceDescriptors->GetCpuHandle(Descriptors::Cat)
    );

    RenderTargetState rtState{
        m_deviceResources->GetBackBufferFormat(),
        m_deviceResources->GetDepthBufferFormat()
    };

    SpriteBatchPipelineStateDescription pd{ rtState };
    m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);

    XMUINT2 catSize = GetTextureSize(m_texture.get());

    m_origin.x = float{ catSize.x / 2.f };
    m_origin.y = float{ catSize.y / 2.f };

    auto uploadResourcesFinished{ resourceUpload.End(m_deviceResources->GetCommandQueue()) };
    uploadResourcesFinished.wait();
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto viewport{ m_deviceResources->GetScreenViewport() };
    m_spriteBatch->SetViewport(viewport);

    auto size{ m_deviceResources->GetOutputSize() };
    m_screenPos.x = static_cast<float>(size.right) / 2.f;
    m_screenPos.y = static_cast<float>(size.bottom) / 2.f;
}

void Game::OnDeviceLost()
{
    m_texture = nullptr;
    m_resourceDescriptors.reset();
    m_spriteBatch.reset();

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion