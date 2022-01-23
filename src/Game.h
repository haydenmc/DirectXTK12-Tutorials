#pragma once

#include <tuple>

#include <DirectXTK12/GraphicsMemory.h>

#include "DeviceResources.h"
#include "StepTimer.h"


class Game : public DX::IDeviceNotify
{
public:
	// Constructor/Destructor
	Game();
	~Game();
	Game(Game&&) = default;
	Game& operator= (Game&&) = default;
	Game(Game const&) = delete;
	Game& operator= (Game const&) = delete;

	// Initialization and management
	void Initialize(HWND hwnd, uint32_t width, uint32_t height);
	void Tick();
	
	// IDeviceNotify
	void OnDeviceLost() override;
	void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowMoved();
	void OnWindowSizeChanged(uint32_t width, uint32_t height);

	// Properties
	std::tuple<uint32_t, uint32_t> GetDefaultSize() const noexcept;

private:
	void Update(DX::StepTimer const& timer);
	void Render();

	void Clear();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	// Device resources.
	std::unique_ptr<DX::DeviceResources>        m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer                               m_timer;

	// If using the DirectX Tool Kit for DX12, uncomment this line:
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
};