// render.cpp
#include "renderer-shared.h"
#include "../../source/core/slang-math.h"

#include "d3d11/render-d3d11.h"
#include "d3d12/render-d3d12.h"
#include "open-gl/render-gl.h"
#include "vulkan/render-vk.h"
#include "cuda/render-cuda.h"
#include "cpu/render-cpu.h"
#include "debug-layer.h"

#include <cstring>

namespace gfx {
using namespace Slang;

static bool debugLayerEnabled = false;

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Global Renderer Functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#define GFX_FORMAT_SIZE(name, blockSizeInBytes, pixelsPerBlock) {blockSizeInBytes, pixelsPerBlock},

static const uint32_t s_formatSizeInfo[][2] =
{
    GFX_FORMAT(GFX_FORMAT_SIZE)
};

static bool _checkFormat()
{
    Index value = 0;
    Index count = 0;

    // Check the values are in the same order
#define GFX_FORMAT_CHECK(name, blockSizeInBytes, pixelsPerblock) count += Index(Index(Format::name) == value++);
    GFX_FORMAT(GFX_FORMAT_CHECK)

    const bool r = (count == Index(Format::CountOf));
    SLANG_ASSERT(r);
    return r;
}

// We don't make static because we will get a warning that it's unused
static const bool _checkFormatResult = _checkFormat();

struct FormatInfoMap
{
    FormatInfoMap()
    {
        // Set all to nothing initially
        for (auto& info : m_infos)
        {
            info.channelCount = 0;
            info.channelType = SLANG_SCALAR_TYPE_NONE;
        }

        set(Format::RGBA_Typeless32, SLANG_SCALAR_TYPE_UINT32, 4);
        set(Format::RGB_Typeless32, SLANG_SCALAR_TYPE_UINT32, 3);
        set(Format::RG_Typeless32, SLANG_SCALAR_TYPE_UINT32, 2);
        set(Format::R_Typeless32, SLANG_SCALAR_TYPE_UINT32, 1);

        set(Format::RGBA_Typeless16, SLANG_SCALAR_TYPE_UINT16, 4);
        set(Format::RG_Typeless16, SLANG_SCALAR_TYPE_UINT16, 2);
        set(Format::R_Typeless16, SLANG_SCALAR_TYPE_UINT16, 1);

        set(Format::RGBA_Typeless8, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::RG_Typeless8, SLANG_SCALAR_TYPE_UINT8, 2);
        set(Format::R_Typeless8, SLANG_SCALAR_TYPE_UINT8, 1);
        set(Format::BGRA_Typeless8, SLANG_SCALAR_TYPE_UINT8, 4);

        set(Format::RGBA_Float32, SLANG_SCALAR_TYPE_FLOAT32, 4);
        set(Format::RGB_Float32, SLANG_SCALAR_TYPE_FLOAT32, 3);
        set(Format::RG_Float32, SLANG_SCALAR_TYPE_FLOAT32, 2);
        set(Format::R_Float32, SLANG_SCALAR_TYPE_FLOAT32, 1);

        set(Format::RGBA_Float16, SLANG_SCALAR_TYPE_FLOAT16, 4);
        set(Format::RG_Float16, SLANG_SCALAR_TYPE_FLOAT16, 2);
        set(Format::R_Float16, SLANG_SCALAR_TYPE_FLOAT16, 1);

        set(Format::RGBA_UInt32, SLANG_SCALAR_TYPE_UINT32, 4);
        set(Format::RGB_UInt32, SLANG_SCALAR_TYPE_UINT32, 3);
        set(Format::RG_UInt32, SLANG_SCALAR_TYPE_UINT32, 2);
        set(Format::R_UInt32, SLANG_SCALAR_TYPE_UINT32, 1);

        set(Format::RGBA_UInt16, SLANG_SCALAR_TYPE_UINT16, 4);
        set(Format::RG_UInt16, SLANG_SCALAR_TYPE_UINT16, 2);
        set(Format::R_UInt16, SLANG_SCALAR_TYPE_UINT16, 1);

        set(Format::RGBA_UInt8, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::RG_UInt8, SLANG_SCALAR_TYPE_UINT8, 2);
        set(Format::R_UInt8, SLANG_SCALAR_TYPE_UINT8, 1);

        set(Format::RGBA_SInt32, SLANG_SCALAR_TYPE_INT32, 4);
        set(Format::RGB_SInt32, SLANG_SCALAR_TYPE_INT32, 3);
        set(Format::RG_SInt32, SLANG_SCALAR_TYPE_INT32, 2);
        set(Format::R_SInt32, SLANG_SCALAR_TYPE_INT32, 1);

        set(Format::RGBA_SInt16, SLANG_SCALAR_TYPE_INT16, 4);
        set(Format::RG_SInt16, SLANG_SCALAR_TYPE_INT16, 2);
        set(Format::R_SInt16, SLANG_SCALAR_TYPE_INT16, 1);

        set(Format::RGBA_SInt8, SLANG_SCALAR_TYPE_INT8, 4);
        set(Format::RG_SInt8, SLANG_SCALAR_TYPE_INT8, 2);
        set(Format::R_SInt8, SLANG_SCALAR_TYPE_INT8, 1);

        set(Format::RGBA_Unorm_UInt16, SLANG_SCALAR_TYPE_UINT16, 4);
        set(Format::RG_Unorm_UInt16, SLANG_SCALAR_TYPE_UINT16, 2);
        set(Format::R_Unorm_UInt16, SLANG_SCALAR_TYPE_UINT16, 1);

        set(Format::RGBA_Unorm_UInt8, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::RGBA_Unorm_UInt8_Srgb, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::RG_Unorm_UInt8, SLANG_SCALAR_TYPE_UINT8, 2);
        set(Format::R_Unorm_UInt8, SLANG_SCALAR_TYPE_UINT8, 1);
        set(Format::BGRA_Unorm_UInt8, SLANG_SCALAR_TYPE_UINT8, 4);

        set(Format::RGBA_Snorm_Int16, SLANG_SCALAR_TYPE_INT16, 4);
        set(Format::RG_Snorm_Int16, SLANG_SCALAR_TYPE_INT16, 2);
        set(Format::R_Snorm_Int16, SLANG_SCALAR_TYPE_INT16, 1);

        set(Format::RGBA_Snorm_Int8, SLANG_SCALAR_TYPE_INT8, 4);
        set(Format::RG_Snorm_Int8, SLANG_SCALAR_TYPE_INT8, 2);
        set(Format::R_Snorm_Int8, SLANG_SCALAR_TYPE_INT8, 1);

        set(Format::D_Float32, SLANG_SCALAR_TYPE_FLOAT32, 1);
        set(Format::D_Unorm24_S8, SLANG_SCALAR_TYPE_UINT32, 2);
        set(Format::D_Unorm16, SLANG_SCALAR_TYPE_UINT16, 1);

        set(Format::BGRA_Unorm4, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::B5G6R5_Unorm, SLANG_SCALAR_TYPE_UINT8, 3);
        set(Format::B5G5R5A1_Unorm, SLANG_SCALAR_TYPE_UINT8, 4);

        set(Format::BC1_Unorm, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC1_Unorm_Srgb, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC2_Unorm, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC2_Unorm_Srgb, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC3_Unorm, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC3_Unorm_Srgb, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC4_Unorm, SLANG_SCALAR_TYPE_UINT8, 1);
        set(Format::BC4_Snorm, SLANG_SCALAR_TYPE_INT8, 1);
        set(Format::BC5_Unorm, SLANG_SCALAR_TYPE_UINT8, 2);
        set(Format::BC5_Snorm, SLANG_SCALAR_TYPE_UINT8, 2);
        set(Format::BC6_Unsigned, SLANG_SCALAR_TYPE_FLOAT16, 3);
        set(Format::BC6_Signed, SLANG_SCALAR_TYPE_FLOAT16, 3);
        set(Format::BC7_Unorm, SLANG_SCALAR_TYPE_UINT8, 4);
        set(Format::BC7_Unorm_Srgb, SLANG_SCALAR_TYPE_UINT8, 4);
    }

    void set(Format format, SlangScalarType type, Index channelCount)
    {
        FormatInfo& info = m_infos[Index(format)];
        info.channelCount = uint8_t(channelCount);
        info.channelType = uint8_t(type);

        auto sizeInfo = s_formatSizeInfo[Index(format)];
        info.blockSizeInBytes = sizeInfo[0];
        info.pixelsPerBlock = sizeInfo[1];
    }

    const FormatInfo& get(Format format) const { return m_infos[Index(format)]; }

    FormatInfo m_infos[Index(Format::CountOf)];
};

static const FormatInfoMap s_formatInfoMap;

static void _compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_formatSizeInfo) == int(Format::CountOf));
}

extern "C"
{
    SLANG_GFX_API SlangResult gfxGetFormatInfo(Format format, FormatInfo* outInfo)
    {
        *outInfo = s_formatInfoMap.get(format);
        return SLANG_OK;
    }

    SlangResult _createDevice(const IDevice::Desc* desc, IDevice** outDevice)
    {
        switch (desc->deviceType)
        {
#if SLANG_WINDOWS_FAMILY
        case DeviceType::DirectX11:
            {
                return createD3D11Device(desc, outDevice);
            }
        case DeviceType::DirectX12:
            {
                return createD3D12Device(desc, outDevice);
            }
        case DeviceType::OpenGl:
            {
                return createGLDevice(desc, outDevice);
            }
        case DeviceType::Vulkan:
            {
                return createVKDevice(desc, outDevice);
            }
        case DeviceType::CUDA:
            {
                return createCUDADevice(desc, outDevice);
            }
        case DeviceType::Default:
            {
                IDevice::Desc newDesc = *desc;
                newDesc.deviceType = DeviceType::DirectX12;
                if (_createDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                newDesc.deviceType = DeviceType::Vulkan;
                if (_createDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                newDesc.deviceType = DeviceType::DirectX11;
                if (_createDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                newDesc.deviceType = DeviceType::OpenGl;
                if (_createDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                return SLANG_FAIL;
            }
            break;
#elif SLANG_LINUX_FAMILY && !defined(__CYGWIN__)
        case DeviceType::Default:
        case DeviceType::Vulkan:
        {
            return createVKDevice(desc, outDevice);
        }
        case DeviceType::CUDA:
        {
            return createCUDADevice(desc, outDevice);
        }
#endif
        case DeviceType::CPU:
            {
                return createCPUDevice(desc, outDevice);
            }
            break;

        default:
            return SLANG_FAIL;
        }
    }

    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxCreateDevice(const IDevice::Desc* desc, IDevice** outDevice)
    {
        ComPtr<IDevice> innerDevice;
        auto resultCode = _createDevice(desc, innerDevice.writeRef());
        if (SLANG_FAILED(resultCode))
            return resultCode;
        if (!debugLayerEnabled)
        {
            returnComPtr(outDevice, innerDevice);
            return resultCode;
        }
        RefPtr<DebugDevice> debugDevice = new DebugDevice();
        debugDevice->baseObject = innerDevice;
        returnComPtr(outDevice, debugDevice);
        return resultCode;
    }

    SLANG_GFX_API SlangResult SLANG_MCALL gfxSetDebugCallback(IDebugCallback* callback)
    {
        _getDebugCallback() = callback;
        return SLANG_OK;
    }

    SLANG_GFX_API void SLANG_MCALL gfxEnableDebugLayer()
    {
        debugLayerEnabled = true;
    }

    const char* SLANG_MCALL gfxGetDeviceTypeName(DeviceType type)
    {
        switch (type)
        {
        case gfx::DeviceType::Unknown:
            return "Unknown";
        case gfx::DeviceType::Default:
            return "Default";
        case gfx::DeviceType::DirectX11:
            return "DirectX11";
        case gfx::DeviceType::DirectX12:
            return "DirectX12";
        case gfx::DeviceType::OpenGl:
            return "OpenGL";
        case gfx::DeviceType::Vulkan:
            return "Vulkan";
        case gfx::DeviceType::CPU:
            return "CPU";
        case gfx::DeviceType::CUDA:
            return "CUDA";
        default:
            return "?";
        }
    }


    void SLANG_MCALL gfxGetIdentityProjection(ProjectionStyle style, float projMatrix[16])
    {
        switch (style)
        {
        case ProjectionStyle::DirectX:
        case ProjectionStyle::OpenGl:
            {
                static const float kIdentity[] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
                ::memcpy(projMatrix, kIdentity, sizeof(kIdentity));
                break;
            }
        case ProjectionStyle::Vulkan:
            {
                static const float kIdentity[] = {1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
                ::memcpy(projMatrix, kIdentity, sizeof(kIdentity));
                break;
            }
        default:
            {
                assert(!"Not handled");
            }
        }
    }
}

} // renderer_test
