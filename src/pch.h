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