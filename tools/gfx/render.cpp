// render.cpp
#include "renderer-shared.h"
#include "../../source/core/slang-math.h"

#include "d3d11/render-d3d11.h"
#include "d3d12/render-d3d12.h"
#include "open-gl/render-gl.h"
#include "vulkan/render-vk.h"
#include "cuda/render-cuda.h"
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

static const BindingStyle s_rendererTypeToBindingStyle[] = {
    BindingStyle::Unknown, // Unknown,
    BindingStyle::DirectX, // DirectX11,
    BindingStyle::DirectX, // DirectX12,
    BindingStyle::OpenGl, // OpenGl,
    BindingStyle::Vulkan, // Vulkan
    BindingStyle::CPU, // CPU
    BindingStyle::CUDA, // CUDA
};

static void _compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_formatSize) == int(Format::CountOf));
    SLANG_COMPILE_TIME_ASSERT(
        SLANG_COUNT_OF(s_rendererTypeToBindingStyle) == int(RendererType::CountOf));
}

extern "C"
{
    size_t SLANG_MCALL gfxGetFormatSize(Format format)
    {
        return s_formatSize[int(format)];
    }

    BindingStyle SLANG_MCALL gfxGetBindingStyle(RendererType type)
    {
        return s_rendererTypeToBindingStyle[int(type)];
    }

    const char* SLANG_MCALL gfxGetRendererName(RendererType type)
    {
        switch (type)
        {
        case RendererType::DirectX11:
            return "DirectX11";
        case RendererType::DirectX12:
            return "DirectX12";
        case RendererType::OpenGl:
            return "OpenGL";
        case RendererType::Vulkan:
            return "Vulkan";
        case RendererType::Unknown:
            return "Unknown";
        case RendererType::CPU:
            return "CPU";
        case RendererType::CUDA:
            return "CUDA";
        default:
            return "?!?";
        }
    }

    SLANG_GFX_API SlangResult SLANG_MCALL gfxCreateRenderer(const IRenderer::Desc* desc, IRenderer** outRenderer)
    {
        switch (desc->rendererType)
        {
#if SLANG_WINDOWS_FAMILY
        case RendererType::DirectX11:
            {
                return createD3D11Renderer(desc, outRenderer);
            }
        case RendererType::DirectX12:
            {
                return createD3D12Renderer(desc, outRenderer);
            }
        case RendererType::OpenGl:
            {
                return createGLRenderer(desc, outRenderer);
            }
        case RendererType::Vulkan:
            {
                return createVKRenderer(desc, outRenderer);
            }
        case RendererType::CUDA:
            {
                return createCUDARenderer(desc, outRenderer);
            }
#endif

        default:
            return SLANG_FAIL;
        }
    }

    ProjectionStyle SLANG_MCALL gfxGetProjectionStyle(RendererType type)
    {
        switch (type)
        {
        case RendererType::DirectX11:
        case RendererType::DirectX12:
            {
                return ProjectionStyle::DirectX;
            }
        case RendererType::OpenGl:
            return ProjectionStyle::OpenGl;
        case RendererType::Vulkan:
            return ProjectionStyle::Vulkan;
        case RendererType::Unknown:
            return ProjectionStyle::Unknown;
        default:
            {
                assert(!"Unhandled type");
                return ProjectionStyle::Unknown;
            }
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
