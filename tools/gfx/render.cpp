// render.cpp
#include "renderer-shared.h"
#include "../../source/core/slang-math.h"

#include "d3d11/render-d3d11.h"
#include "d3d12/render-d3d12.h"
#include "open-gl/render-gl.h"
#include "vulkan/render-vk.h"
#include "cuda/render-cuda.h"
#include "cpu/render-cpu.h"
#include <cstring>

namespace gfx {
using namespace Slang;

static const IResource::DescBase s_emptyDescBase = {};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Global Renderer Functions !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

static const uint8_t s_formatSize[] = {
    0, // Unknown,

    uint8_t(sizeof(float) * 4), // RGBA_Float32,
    uint8_t(sizeof(float) * 3), // RGB_Float32,
    uint8_t(sizeof(float) * 2), // RG_Float32,
    uint8_t(sizeof(float) * 1), // R_Float32,

    uint8_t(sizeof(uint32_t)), // RGBA_Unorm_UInt8,
    uint8_t(sizeof(uint32_t)), // BGRA_Unorm_UInt8,

    uint8_t(sizeof(uint16_t)), // R_UInt16,
    uint8_t(sizeof(uint32_t)), // R_UInt32,

    uint8_t(sizeof(float)), // D_Float32,
    uint8_t(sizeof(uint32_t)), // D_Unorm24_S8,
};

static void _compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_formatSize) == int(Format::CountOf));
}

extern "C"
{
    size_t SLANG_MCALL gfxGetFormatSize(Format format)
    {
        return s_formatSize[int(format)];
    }

    SLANG_GFX_API SlangResult SLANG_MCALL
        gfxCreateDevice(const IDevice::Desc* desc, IDevice** outDevice)
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
                if (gfxCreateDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                newDesc.deviceType = DeviceType::Vulkan;
                if (gfxCreateDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                newDesc.deviceType = DeviceType::DirectX11;
                if (gfxCreateDevice(&newDesc, outDevice) == SLANG_OK)
                    return SLANG_OK;
                newDesc.deviceType = DeviceType::OpenGl;
                if (gfxCreateDevice(&newDesc, outDevice) == SLANG_OK)
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
