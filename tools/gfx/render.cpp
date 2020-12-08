// render.cpp
#include "render.h"

#include "../../source/core/slang-math.h"

#include "d3d11/render-d3d11.h"
#include "d3d12/render-d3d12.h"
#include "open-gl/render-gl.h"
#include "vulkan/render-vk.h"
#include "cuda/render-cuda.h"

namespace gfx {
using namespace Slang;

/* static */const Resource::BindFlag::Enum Resource::s_requiredBinding[] =
{
    BindFlag::VertexBuffer,                 // VertexBuffer
    BindFlag::IndexBuffer,                  // IndexBuffer
    BindFlag::ConstantBuffer,               // ConstantBuffer
    BindFlag::StreamOutput,                 // StreamOut
    BindFlag::RenderTarget,                 // RenderTager
    BindFlag::DepthStencil,                 // DepthRead
    BindFlag::DepthStencil,                 // DepthWrite
    BindFlag::UnorderedAccess,              // UnorderedAccess
    BindFlag::PixelShaderResource,          // PixelShaderResource
    BindFlag::NonPixelShaderResource,       // NonPixelShaderResource
    BindFlag::Enum(BindFlag::PixelShaderResource | BindFlag::NonPixelShaderResource), // GenericRead
};


/* static */void Resource::compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_requiredBinding) == int(Usage::CountOf));
}

static const Resource::DescBase s_emptyDescBase = {};

const Resource::DescBase& Resource::getDescBase() const
{
    if (isBuffer())
    {
        return static_cast<const BufferResource *>(this)->getDesc();
    }
    else if (isTexture())
    {
        return static_cast<const TextureResource *>(this)->getDesc();
    }
    return s_emptyDescBase;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! RendererUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */const uint8_t RendererUtil::s_formatSize[] =
{
    0,                               // Unknown,

    uint8_t(sizeof(float) * 4),      // RGBA_Float32,
    uint8_t(sizeof(float) * 3),      // RGB_Float32,
    uint8_t(sizeof(float) * 2),      // RG_Float32,
    uint8_t(sizeof(float) * 1),      // R_Float32,

    uint8_t(sizeof(uint32_t)),       // RGBA_Unorm_UInt8,

    uint8_t(sizeof(uint16_t)),       // R_UInt16,
    uint8_t(sizeof(uint32_t)),       // R_UInt32,

    uint8_t(sizeof(float)),          // D_Float32,
    uint8_t(sizeof(uint32_t)),       // D_Unorm24_S8,
};

/* static */const BindingStyle RendererUtil::s_rendererTypeToBindingStyle[] =
{
    BindingStyle::Unknown,      // Unknown,
    BindingStyle::DirectX,      // DirectX11,
    BindingStyle::DirectX,      // DirectX12,
    BindingStyle::OpenGl,       // OpenGl,
    BindingStyle::Vulkan,       // Vulkan
    BindingStyle::CPU,          // CPU
    BindingStyle::CUDA,         // CUDA
};

/* static */void RendererUtil::compileTimeAsserts()
{
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_formatSize) == int(Format::CountOf));
    SLANG_COMPILE_TIME_ASSERT(SLANG_COUNT_OF(s_rendererTypeToBindingStyle) == int(RendererType::CountOf));
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! BindingState::Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
#if 0
void BindingState::Desc::addSampler(const SamplerDesc& desc, const RegisterRange& registerRange)
{
    int descIndex = int(m_samplerDescs.Count());
    m_samplerDescs.Add(desc);

    Binding binding;
    binding.bindingType = BindingType::Sampler;
    binding.resource = nullptr;
    binding.registerRange = registerRange;
    binding.descIndex = descIndex;

    m_bindings.Add(binding);
}

void BindingState::Desc::addResource(BindingType bindingType, Resource* resource, const RegisterRange& registerRange)
{
    assert(resource);

    Binding binding;
    binding.bindingType = bindingType;
    binding.resource = resource;
    binding.descIndex = -1;
    binding.registerRange = registerRange;
    m_bindings.Add(binding);
}

void BindingState::Desc::addCombinedTextureSampler(TextureResource* resource, const SamplerDesc& samplerDesc, const RegisterRange& registerRange)
{
    assert(resource);

    int samplerDescIndex = int(m_samplerDescs.Count());
    m_samplerDescs.Add(samplerDesc);

    Binding binding;
    binding.bindingType = BindingType::CombinedTextureSampler;
    binding.resource = resource;
    binding.descIndex = samplerDescIndex;
    binding.registerRange = registerRange;
    m_bindings.Add(binding);
}

void BindingState::Desc::clear()
{
    m_bindings.Clear();
    m_samplerDescs.Clear();
    m_numRenderTargets = 1;
}

int BindingState::Desc::findBindingIndex(Resource::BindFlag::Enum bindFlag, int registerIndex) const
{
    const int numBindings = int(m_bindings.Count());
    for (int i = 0; i < numBindings; ++i)
    {
        const Binding& binding = m_bindings[i];
        if (binding.resource && (binding.resource->getDescBase().bindFlags & bindFlag) != 0)
        {
            if (binding.registerRange.hasRegister(registerIndex))
            {
                return i;
            }
        }
    }

    return -1;
}
#endif

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! TextureResource::Size !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

int TextureResource::Size::calcMaxDimension(Type type) const
{
    switch (type)
    {
        case Resource::Type::Texture1D:     return this->width;
        case Resource::Type::Texture3D:     return std::max(std::max(this->width, this->height), this->depth);
        case Resource::Type::TextureCube:   // fallthru
        case Resource::Type::Texture2D:
        {
            return std::max(this->width, this->height);
        }
        default: return 0;
    }
}

TextureResource::Size TextureResource::Size::calcMipSize(int mipLevel) const
{
    Size size;
    size.width = TextureResource::calcMipSize(this->width, mipLevel);
    size.height = TextureResource::calcMipSize(this->height, mipLevel);
    size.depth = TextureResource::calcMipSize(this->depth, mipLevel);
    return size;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! BufferResource::Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void BufferResource::Desc::setDefaults(Usage initialUsage)
{
    if (this->bindFlags == 0)
    {
        this->bindFlags = Resource::s_requiredBinding[int(initialUsage)];
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! TextureResource::Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

int TextureResource::Desc::calcNumMipLevels() const
{
    const int maxDimensionSize = this->size.calcMaxDimension(type);
    return (maxDimensionSize > 0) ? (Math::Log2Floor(maxDimensionSize) + 1) : 0;
}

int TextureResource::Desc::calcNumSubResources() const
{
    const int numMipMaps = (this->numMipLevels > 0) ? this->numMipLevels : calcNumMipLevels();
    const int arrSize = (this->arraySize > 0) ? this->arraySize : 1;

    switch (type)
    {
        case Resource::Type::Texture1D:
        case Resource::Type::Texture2D:
        {
            return numMipMaps * arrSize;
        }
        case Resource::Type::Texture3D:
        {
            // can't have arrays of 3d textures
            assert(this->arraySize <= 1);
            return numMipMaps * this->size.depth;
        }
        case Resource::Type::TextureCube:
        {
            // There are 6 faces to a cubemap
            return numMipMaps * arrSize * 6;
        }
        default:                            return 0;
    }
}

void TextureResource::Desc::fixSize()
{
    switch (type)
    {
        case Resource::Type::Texture1D:
        {
            this->size.height = 1;
            this->size.depth = 1;
            break;
        }
        case Resource::Type::TextureCube:
        case Resource::Type::Texture2D:
        {
            this->size.depth = 1;
            break;
        }
        case Resource::Type::Texture3D:
        {
            // Can't have an array
            this->arraySize = 0;
            break;
        }
        default: break;
    }
}

void TextureResource::Desc::setDefaults(Usage initialUsage)
{
    fixSize();
    if (this->bindFlags == 0)
    {
        this->bindFlags = Resource::s_requiredBinding[int(initialUsage)];
    }
    if (this->numMipLevels <= 0)
    {
        this->numMipLevels = calcNumMipLevels();
    }
}

int TextureResource::Desc::calcEffectiveArraySize() const
{
    const int arrSize = (this->arraySize > 0) ? this->arraySize : 1;

    switch (type)
    {
        case Resource::Type::Texture1D:         // fallthru
        case Resource::Type::Texture2D:
        {
            return arrSize;
        }
        case Resource::Type::TextureCube:       return arrSize * 6;
        case Resource::Type::Texture3D:         return 1;
        default:                                return 0;
    }
}

void TextureResource::Desc::init(Type typeIn)
{
    this->type = typeIn;
    this->size.init();

    this->format = Format::Unknown;
    this->arraySize = 0;
    this->numMipLevels = 0;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->cpuAccessFlags = 0;
}

void TextureResource::Desc::init1D(Format formatIn, int widthIn, int numMipMapsIn)
{
    this->type = Type::Texture1D;
    this->size.init(widthIn);

    this->format = formatIn;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->cpuAccessFlags = 0;
}

void TextureResource::Desc::init2D(Type typeIn, Format formatIn, int widthIn, int heightIn, int numMipMapsIn)
{
    assert(typeIn == Type::Texture2D || typeIn == Type::TextureCube);

    this->type = typeIn;
    this->size.init(widthIn, heightIn);

    this->format = formatIn;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->cpuAccessFlags = 0;
}

void TextureResource::Desc::init3D(Format formatIn, int widthIn, int heightIn, int depthIn, int numMipMapsIn)
{
    this->type = Type::Texture3D;
    this->size.init(widthIn, heightIn, depthIn);

    this->format = formatIn;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->cpuAccessFlags = 0;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! RennderUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

ProjectionStyle RendererUtil::getProjectionStyle(RendererType type)
{
    switch (type)
    {
        case RendererType::DirectX11:
        case RendererType::DirectX12:
        {
            return ProjectionStyle::DirectX;
        }
        case RendererType::OpenGl:              return ProjectionStyle::OpenGl;
        case RendererType::Vulkan:              return ProjectionStyle::Vulkan;
        case RendererType::Unknown:             return ProjectionStyle::Unknown;
        default:
        {
            assert(!"Unhandled type");
            return ProjectionStyle::Unknown;
        }
    }
}

/* static */void RendererUtil::getIdentityProjection(ProjectionStyle style, float projMatrix[16])
{
    switch (style)
    {
        case ProjectionStyle::DirectX:
        case ProjectionStyle::OpenGl:
        {
            static const float kIdentity[] =
            {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            ::memcpy(projMatrix, kIdentity, sizeof(kIdentity));
            break;
        }
        case ProjectionStyle::Vulkan:
        {
            static const float kIdentity[] =
            {
                1, 0, 0, 0,
                0, -1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
            ::memcpy(projMatrix, kIdentity, sizeof(kIdentity));
            break;
        }
        default:
        {
            assert(!"Not handled");
        }
    }
}

/* static */UnownedStringSlice RendererUtil::toText(RendererType type)
{
    switch (type)
    {
        case RendererType::DirectX11:       return UnownedStringSlice::fromLiteral("DirectX11");
        case RendererType::DirectX12:       return UnownedStringSlice::fromLiteral("DirectX11");
        case RendererType::OpenGl:          return UnownedStringSlice::fromLiteral("OpenGL");
        case RendererType::Vulkan:          return UnownedStringSlice::fromLiteral("Vulkan");
        case RendererType::Unknown:         return UnownedStringSlice::fromLiteral("Unknown");
        case RendererType::CPU:             return UnownedStringSlice::fromLiteral("CPU");
        case RendererType::CUDA:            return UnownedStringSlice::fromLiteral("CUDA");
        default:                            return UnownedStringSlice::fromLiteral("?!?");
    }
}

/* static */ RendererUtil::CreateFunc RendererUtil::getCreateFunc(RendererType type)
{
    switch (type)
    {
#if SLANG_WINDOWS_FAMILY
        case RendererType::DirectX11:
        {
            return &createD3D11Renderer;
        }
        case RendererType::DirectX12:
        {
            return &createD3D12Renderer;
        }
        case RendererType::OpenGl:
        {
            return &createGLRenderer;
        }
        case RendererType::Vulkan:
        {
            return &createVKRenderer;
        }
        case RendererType::CUDA:
        {
            return &createCUDARenderer;
        }
#endif

        default: return nullptr;
    }
}


} // renderer_test
