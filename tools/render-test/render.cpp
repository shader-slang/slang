// render.cpp
#include "render.h"

#include "../../source/core/slang-math.h"

namespace renderer_test {
using namespace Slang;

/* static */const Resource::BindFlag::Enum Resource::s_requiredBinding[int(Usage::CountOf)] = 
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

/* static */const uint8_t RendererUtil::s_formatSize[int(Format::CountOf)] = 
{
    0,                               // Unknown,

    uint8_t(sizeof(float) * 4),      // RGBA_Float32,
    uint8_t(sizeof(float) * 3),      // RGB_Float32,
    uint8_t(sizeof(float) * 2),      // RG_Float32,
    uint8_t(sizeof(float) * 1),      // R_Float32,

    uint8_t(sizeof(uint32_t)),       // RGBA_Unorm_UInt8,

    uint8_t(sizeof(float)),          // D_Float32,
    uint8_t(sizeof(uint32_t)),       // D_Unorm24_S8,
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! BindingState::Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void BindingState::Desc::addSampler(const SamplerDesc& desc, const ShaderBindSet& shaderBindSet)
{
    int descIndex = int(m_samplerDescs.Count());
    m_samplerDescs.Add(desc);

    Binding binding;
    binding.bindingType = BindingType::Sampler;
    binding.resource = nullptr;
    binding.shaderBindSet = shaderBindSet;
    binding.descIndex = descIndex;

    m_bindings.Add(binding);
}

void BindingState::Desc::addResource(BindingType bindingType, Resource* resource, const ShaderBindSet& shaderBindSet)
{
    assert(resource);

    Binding binding;
    binding.bindingType = bindingType;
    binding.resource = resource;
    binding.descIndex = -1;
    binding.shaderBindSet = shaderBindSet;
    m_bindings.Add(binding);
}

void BindingState::Desc::addCombinedTextureSampler(TextureResource* resource, const SamplerDesc& samplerDesc, const ShaderBindSet& shaderBindSet)
{
    assert(resource);

    int samplerDescIndex = int(m_samplerDescs.Count());
    m_samplerDescs.Add(samplerDesc);

    Binding binding;
    binding.bindingType = BindingType::CombinedTextureSampler;
    binding.resource = resource;
    binding.descIndex = samplerDescIndex;
    binding.shaderBindSet = shaderBindSet;
    m_bindings.Add(binding);
}

BindingState::CompactBindIndexSlice BindingState::Desc::makeCompactSlice(int index)
{
    if (index < 0)
    {
        return CompactBindIndexSlice();
    }
    return CompactBindIndexSlice(index, 1);
}

BindingState::CompactBindIndexSlice BindingState::Desc::makeCompactSlice(const int* srcIndices, int numIndices)
{
    assert(numIndices >= 0);
    switch (numIndices)
    {
        case 0:     return CompactBindIndexSlice(); 
        case 1:     return CompactBindIndexSlice(srcIndices[0], 1);
        default:
        {
            int startIndex = int(m_sharedBindIndices.Count());
            m_sharedBindIndices.SetSize(startIndex + numIndices);
            uint16_t* dstIndices = m_sharedBindIndices.Buffer() + startIndex;
            for (int i = 0; i < numIndices; i++)
            {
                assert(srcIndices[i] >= 0);
                dstIndices[i] = uint16_t(srcIndices[i]);
            }
            return CompactBindIndexSlice(startIndex, numIndices);
        }
    }
}

int BindingState::Desc::getFirst(const CompactBindIndexSlice& set) const
{
    switch (set.m_size)
    {
        case 0:             return -1;
        case 1:             return set.m_indexOrBase;
        default:            return m_sharedBindIndices[set.m_indexOrBase];
    }
}

int BindingState::Desc::getFirst(ShaderStyle style, const ShaderBindSet& shaderBindSet) const
{
    return getFirst(shaderBindSet.shaderSlices[int(style)]); 
}

void BindingState::Desc::clear()
{
    m_bindings.Clear();
    m_samplerDescs.Clear();
    m_sharedBindIndices.Clear();
    m_numRenderTargets = 1;
}

BindingState::BindIndexSlice BindingState::Desc::asSlice(const CompactBindIndexSlice& compactSlice) const
{
    switch (compactSlice.m_size)
    {
        case 0:     return BindIndexSlice{ nullptr, 0 };
        case 1:     return BindIndexSlice{ &compactSlice.m_indexOrBase, 1 };
        default:    return BindIndexSlice{ m_sharedBindIndices.Buffer() + compactSlice.m_indexOrBase, compactSlice.m_size };
    }
}

BindingState::BindIndexSlice BindingState::Desc::asSlice(ShaderStyle style, const ShaderBindSet& shaderBindSet) const
{
    return asSlice(shaderBindSet.shaderSlices[int(style)]);
}


int BindingState::Desc::findBindingIndex(Resource::BindFlag::Enum bindFlag, ShaderStyleFlags shaderStyleFlags, BindIndex index) const
{
    const int numBindings = int(m_bindings.Count());
    for (int i = 0; i < numBindings; ++i)
    {
        const Binding& binding = m_bindings[i];
        if (binding.resource && (binding.resource->getDescBase().bindFlags & bindFlag) != 0)
        {
            for (int j = 0; j < int(ShaderStyle::CountOf); ++j)
            {
                if (indexOf(binding.shaderBindSet.shaderSlices[j], index) >= 0)
                {
                    return i;
                }
            }
        }
    }

    return -1;
}
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

int TextureResource::Desc::calcNumMipLevels(Type type) const
{
    const int maxDimensionSize = this->size.calcMaxDimension(type);
    return (maxDimensionSize > 0) ? (Math::Log2Floor(maxDimensionSize) + 1) : 0;
}

int TextureResource::Desc::calcNumSubResources(Type type) const
{
    const int numMipMaps = (this->numMipLevels > 0) ? this->numMipLevels : calcNumMipLevels(type);
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

void TextureResource::Desc::fixSize(Type type)
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

void TextureResource::Desc::setDefaults(Type type, Usage initialUsage)
{
    fixSize(type);
    if (this->bindFlags == 0)
    {
        this->bindFlags = Resource::s_requiredBinding[int(initialUsage)];
    }
    if (this->numMipLevels <= 0)
    {
        this->numMipLevels = calcNumMipLevels(type);
    }
}

int TextureResource::Desc::calcEffectiveArraySize(Type type) const
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

void TextureResource::Desc::init()
{
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
    this->size.init(widthIn);
    
    this->format = format;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->cpuAccessFlags = 0; 
}

void TextureResource::Desc::init2D(Format formatIn, int widthIn, int heightIn, int numMipMapsIn)
{
    this->size.init(widthIn, heightIn);
    
    this->format = format;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->cpuAccessFlags = 0;
}

void TextureResource::Desc::init3D(Format formatIn, int widthIn, int heightIn, int depthIn, int numMipMapsIn)
{
    this->size.init(widthIn, heightIn, depthIn);

    this->format = format;
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

} // renderer_test
