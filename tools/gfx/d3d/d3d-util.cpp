// d3d-util.cpp
#include "d3d-util.h"

#include <d3dcompiler.h>

#include <dxgi1_4.h>

// We will use the C standard library just for printing error messages.
#include <stdio.h>

namespace gfx {
using namespace Slang;

/* static */D3D_PRIMITIVE_TOPOLOGY D3DUtil::getPrimitiveTopology(PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::TriangleList:
        {
            return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        }
        default: break;
    }
    return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

/* static */DXGI_FORMAT D3DUtil::getMapFormat(Format format)
{
    switch (format)
    {
        case Format::RGBA_Float32:          return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Format::RGB_Float32:           return DXGI_FORMAT_R32G32B32_FLOAT;
        case Format::RG_Float32:            return DXGI_FORMAT_R32G32_FLOAT;
        case Format::R_Float32:             return DXGI_FORMAT_R32_FLOAT;
        case Format::RGBA_Unorm_UInt8:      return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::R_UInt16:              return DXGI_FORMAT_R16_UINT;
        case Format::R_UInt32:              return DXGI_FORMAT_R32_UINT;

        case Format::D_Float32:             return DXGI_FORMAT_D32_FLOAT;
        case Format::D_Unorm24_S8:          return DXGI_FORMAT_D24_UNORM_S8_UINT;

        default:                            return DXGI_FORMAT_UNKNOWN;
    }
}

/* static */DXGI_FORMAT D3DUtil::calcResourceFormat(UsageType usage, Int usageFlags, DXGI_FORMAT format)
{
    SLANG_UNUSED(usage);
    if (usageFlags)
    {
        switch (format)
        {
            case DXGI_FORMAT_R32_FLOAT:     /* fallthru */
            case DXGI_FORMAT_R32_UINT:
            case DXGI_FORMAT_D32_FLOAT:
            {
                return DXGI_FORMAT_R32_TYPELESS;
            }
            case DXGI_FORMAT_D24_UNORM_S8_UINT:    return DXGI_FORMAT_R24G8_TYPELESS;
            default: break;
        }
        return format;
    }
    return format;
}

/* static */DXGI_FORMAT D3DUtil::calcFormat(UsageType usage, DXGI_FORMAT format)
{
    switch (usage)
    {
        case USAGE_COUNT_OF:
        case USAGE_UNKNOWN:
        {
            return DXGI_FORMAT_UNKNOWN;
        }
        case USAGE_DEPTH_STENCIL:
        {
            switch (format)
            {
                case DXGI_FORMAT_D32_FLOAT:     /* fallthru */
                case DXGI_FORMAT_R32_TYPELESS:
                {
                    return DXGI_FORMAT_D32_FLOAT;
                }
                case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
                case DXGI_FORMAT_R24G8_TYPELESS:        return DXGI_FORMAT_D24_UNORM_S8_UINT;
                default: break;
            }
            return format;
        }
        case USAGE_TARGET:
        {
            switch (format)
            {
                case DXGI_FORMAT_D32_FLOAT:     /* fallthru */
                case DXGI_FORMAT_D24_UNORM_S8_UINT:
                {
                    return DXGI_FORMAT_UNKNOWN;
                }
                case DXGI_FORMAT_R32_TYPELESS:        return DXGI_FORMAT_R32_FLOAT;
                default: break;
            }
            return format;
        }
        case USAGE_SRV:
        {
            switch (format)
            {
                case DXGI_FORMAT_D32_FLOAT:     /* fallthru */
                case DXGI_FORMAT_R32_TYPELESS:
                {
                    return DXGI_FORMAT_R32_FLOAT;
                }
                case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:    return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
                default: break;
            }

            return format;
        }
    }

    assert(!"Not reachable");
    return DXGI_FORMAT_UNKNOWN;
}

bool D3DUtil::isTypeless(DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_BC6H_TYPELESS:
        case DXGI_FORMAT_BC7_TYPELESS:
        {
            return true;
        }
        default: break;
    }
    return false;
}

/* static */Int D3DUtil::getNumColorChannelBits(DXGI_FORMAT fmt)
{
    switch (fmt)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
        {
            return 32;
        }
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        {
            return 16;
        }
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
        {
            return 10;
        }
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_TYPELESS:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_TYPELESS:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        {
            return 8;
        }
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
        {
            return 5;
        }
        case DXGI_FORMAT_B4G4R4A4_UNORM:
            return 4;

        default:
            return 0;
    }
}

// Note: this subroutine is now only used by D3D11 for generating bytecode to go into input layouts.
//
// TODO: we can probably remove that code completely by switching to a PSO-like model across all APIs.
//
/* static */Result D3DUtil::compileHLSLShader(char const* sourcePath, char const* source, char const* entryPointName, char const* dxProfileName, ComPtr<ID3DBlob>& shaderBlobOut)
{
    // Rather than statically link against the `d3dcompile` library, we
    // dynamically load it.
    //
    // Note: A more realistic application would compile from HLSL text to D3D
    // shader bytecode as part of an offline process, rather than doing it
    // on-the-fly like this
    //
    static pD3DCompile compileFunc = nullptr;
    if (!compileFunc)
    {
        // TODO(tfoley): maybe want to search for one of a few versions of the DLL
        HMODULE compilerModule = LoadLibraryA("d3dcompiler_47.dll");
        if (!compilerModule)
        {
            fprintf(stderr, "error: failed load 'd3dcompiler_47.dll'\n");
            return SLANG_FAIL;
        }

        compileFunc = (pD3DCompile)GetProcAddress(compilerModule, "D3DCompile");
        if (!compileFunc)
        {
            fprintf(stderr, "error: failed load symbol 'D3DCompile'\n");
            return SLANG_FAIL;
        }
    }

    // For this example, we turn on debug output, and turn off all
    // optimization. A real application would only use these flags
    // when shader debugging is needed.
    UINT flags = 0;
    flags |= D3DCOMPILE_DEBUG;
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_SKIP_OPTIMIZATION;

    // We will always define `__HLSL__` when compiling here, so that
    // input code can react differently to being compiled as pure HLSL.
    D3D_SHADER_MACRO defines[] = {
        { "__HLSL__", "1" },
        { nullptr, nullptr },
    };

    // The `D3DCompile` entry point takes a bunch of parameters, but we
    // don't really need most of them for Slang-generated code.
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = compileFunc(source, strlen(source), sourcePath, &defines[0], nullptr, entryPointName, dxProfileName, flags, 0,
        shaderBlob.writeRef(), errorBlob.writeRef());

    // If the HLSL-to-bytecode compilation produced any diagnostic messages
    // then we will print them out (whether or not the compilation failed).
    if (errorBlob)
    {
        ::fputs((const char*)errorBlob->GetBufferPointer(), stderr);
        ::fflush(stderr);
        ::OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
    }

    SLANG_RETURN_ON_FAIL(hr);
    shaderBlobOut.swap(shaderBlob);
    return SLANG_OK;
}

/* static */void D3DUtil::appendWideChars(const char* in, List<wchar_t>& out)
{
    size_t len = ::strlen(in);

    const DWORD dwFlags = 0;
    int outSize = ::MultiByteToWideChar(CP_UTF8, dwFlags, in, int(len), nullptr, 0);

    if (outSize > 0)
    {
        const Index prevSize = out.getCount();
        out.setCount(prevSize + len + 1);

        WCHAR* dst = out.getBuffer() + prevSize;
        ::MultiByteToWideChar(CP_UTF8, dwFlags, in, int(len), dst, outSize);
        // Make null terminated
        dst[outSize] = 0;
        // Remove terminating 0 from array
        out.unsafeShrinkToCount(prevSize + outSize);
    }
}

/* static */HMODULE D3DUtil::getDxgiModule()
{
    static HMODULE s_dxgiModule = LoadLibraryA("dxgi.dll");
    if (!s_dxgiModule)
    {
        fprintf(stderr, "error: failed load 'dxgi.dll'\n");
        return nullptr;
    }

    return s_dxgiModule;
}

/* static */SlangResult D3DUtil::createFactory(DeviceCheckFlags flags, ComPtr<IDXGIFactory>& outFactory)
{
    auto dxgiModule =  getDxgiModule();
    if (!dxgiModule)
    {
        return SLANG_FAIL;
    }

    typedef HRESULT(WINAPI *PFN_DXGI_CREATE_FACTORY)(REFIID riid, void   **ppFactory);
    typedef HRESULT(WINAPI *PFN_DXGI_CREATE_FACTORY_2)(UINT Flags, REFIID riid, _COM_Outptr_ void **ppFactory);

    {
        auto createFactory2 = (PFN_DXGI_CREATE_FACTORY_2)::GetProcAddress(dxgiModule, "CreateDXGIFactory2");
        if (createFactory2)
        {
            UINT dxgiFlags = 0;

            if (flags & DeviceCheckFlag::UseDebug)
            {
                dxgiFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }

            ComPtr<IDXGIFactory4> factory;
            SLANG_RETURN_ON_FAIL(createFactory2(dxgiFlags, IID_PPV_ARGS(factory.writeRef())));

            outFactory = factory;
            return SLANG_OK;
        }
    }

    {
        auto createFactory = (PFN_DXGI_CREATE_FACTORY)::GetProcAddress(dxgiModule, "CreateDXGIFactory");
        if (!createFactory)
        {
            fprintf(stderr, "error: failed load symbol '%s'\n", "CreateDXGIFactory");
            return SLANG_FAIL;
        }
        return createFactory(IID_PPV_ARGS(outFactory.writeRef()));
    }
}

/* static */SlangResult D3DUtil::findAdapters(DeviceCheckFlags flags, const Slang::UnownedStringSlice& adapaterName, List<ComPtr<IDXGIAdapter>>& outDxgiAdapters)
{
    ComPtr<IDXGIFactory> factory;
    SLANG_RETURN_ON_FAIL(createFactory(flags, factory));
    return findAdapters(flags, adapaterName, factory, outDxgiAdapters);
}

static bool _isMatch(IDXGIAdapter* adapter, const Slang::UnownedStringSlice& lowerAdapaterName)
{
    if (lowerAdapaterName.getLength() == 0)
    {
        return true;
    }

    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);

    String descName = String::fromWString(desc.Description).toLower();

    return descName.indexOf(lowerAdapaterName) != Index(-1);
}

/* static */bool D3DUtil::isWarp(IDXGIFactory* dxgiFactory, IDXGIAdapter* adapterIn)
{
    ComPtr<IDXGIFactory4> dxgiFactory4;
    if (SLANG_SUCCEEDED(dxgiFactory->QueryInterface(IID_PPV_ARGS(dxgiFactory4.writeRef()))))
    {
        ComPtr<IDXGIAdapter> warpAdapter;
        dxgiFactory4->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.writeRef()));

        return adapterIn == warpAdapter;
    }

    return false;
}

/* static */SlangResult D3DUtil::findAdapters(DeviceCheckFlags flags, const UnownedStringSlice& adapterName, IDXGIFactory* dxgiFactory, List<ComPtr<IDXGIAdapter>>& outDxgiAdapters)
{
    String lowerAdapterName = String(adapterName).toLower();

    outDxgiAdapters.clear();

    ComPtr<IDXGIAdapter> warpAdapter;
    if ((flags & DeviceCheckFlag::UseHardwareDevice) == 0)
    {
        ComPtr<IDXGIFactory4> dxgiFactory4;
        if (SLANG_SUCCEEDED(dxgiFactory->QueryInterface(IID_PPV_ARGS(dxgiFactory4.writeRef()))))
        {
            dxgiFactory4->EnumWarpAdapter(IID_PPV_ARGS(warpAdapter.writeRef()));
            if (_isMatch(warpAdapter, lowerAdapterName.getUnownedSlice()))
            {
                outDxgiAdapters.add(warpAdapter);
            }
        }
    }

    for (UINT adapterIndex = 0; true; adapterIndex++)
    {
        ComPtr<IDXGIAdapter> dxgiAdapter;
        if (dxgiFactory->EnumAdapters(adapterIndex, dxgiAdapter.writeRef()) == DXGI_ERROR_NOT_FOUND)
            break;

        // Skip if warp (as we will have already added it)
        if (dxgiAdapter == warpAdapter)
        {
            continue;
        }
        if (!_isMatch(dxgiAdapter, lowerAdapterName.getUnownedSlice()))
        {
            continue;
        }

        // Get if it's software
        UINT deviceFlags = 0;
        ComPtr<IDXGIAdapter1> dxgiAdapter1;
        if (SLANG_SUCCEEDED(dxgiAdapter->QueryInterface(IID_PPV_ARGS(dxgiAdapter1.writeRef()))))
        {
            DXGI_ADAPTER_DESC1 desc;
            dxgiAdapter1->GetDesc1(&desc);
            deviceFlags = desc.Flags;
        }

        // If the right type then add it
        if ((deviceFlags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && (flags & DeviceCheckFlag::UseHardwareDevice) != 0)
        {
            outDxgiAdapters.add(dxgiAdapter);
        }
    }

    return SLANG_OK;
}

} // renderer_test
