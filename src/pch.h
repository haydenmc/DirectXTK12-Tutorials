#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// C++/WinRT
#include <Unknwn.h>
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>

// DirectX
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dx12.h>
#ifdef _DEBUG
    #include <dxgidebug.h>
#endif

// DirectXTK12
#include <DirectXTK12/BufferHelpers.h>
#include <DirectXTK12/CommonStates.h>
#include <DirectXTK12/DDSTextureLoader.h>
#include <DirectXTK12/DescriptorHeap.h>
#include <DirectXTK12/DirectXHelpers.h>
#include <DirectXTK12/EffectPipelineStateDescription.h>
#include <DirectXTK12/Effects.h>
#include <DirectXTK12/GamePad.h>
#include <DirectXTK12/GeometricPrimitive.h>
#include <DirectXTK12/GraphicsMemory.h>
#include <DirectXTK12/Keyboard.h>
#include <DirectXTK12/Model.h>
#include <DirectXTK12/Mouse.h>
#include <DirectXTK12/PostProcess.h>
#include <DirectXTK12/PrimitiveBatch.h>
#include <DirectXTK12/RenderTargetState.h>
#include <DirectXTK12/ResourceUploadBatch.h>
#include <DirectXTK12/ScreenGrab.h>
#include <DirectXTK12/SimpleMath.h>
#include <DirectXTK12/SpriteBatch.h>
#include <DirectXTK12/SpriteFont.h>
#include <DirectXTK12/VertexTypes.h>
#include <DirectXTK12/WICTextureLoader.h>

// Instrumenting library
#include <pix.h>

// Utilities
namespace DX
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw com_exception(hr);
        }
    }
}