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

	// DirectX Resources
	std::unique_ptr<DX::DeviceResources> m_deviceResources;
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;
	std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
	winrt::com_ptr<ID3D12Resource> m_texture;

	enum Descriptors
	{
		Cat,
		Count
	};

	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	DirectX::SimpleMath::Vector2 m_screenPos;
	DirectX::SimpleMath::Vector2 m_origin;
	DirectX::SimpleMath::Vector2 m_velocity;

	// Rendering loop timer.
	DX::StepTimer m_timer;

	// Input
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;
	DirectX::Keyboard::KeyboardStateTracker m_keys;
	DirectX::Mouse::ButtonStateTracker m_mouseButtons;
};