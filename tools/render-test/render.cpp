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

/* !!!!!!!!!!!!!!!!!!!!!!!!!!! BindingState::Desc !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void BindingState::Desc::addSampler(const SamplerDesc& desc, const RegisterDesc& registerDesc)
{
    int descIndex = int(m_samplers.Count());
    m_samplers.Add(desc);

    Binding binding;
    binding.bindingType = BindingType::Sampler;
    binding.resource = nullptr;
    binding.registerDesc = registerDesc;
    binding.descIndex = descIndex;

    m_bindings.Add(binding);
}

void BindingState::Desc::addResource(BindingType bindingType, Resource* resource, const RegisterDesc& registerDesc)
{
    assert(resource);

    Binding binding;
    binding.bindingType = bindingType;
    binding.resource = resource;
    binding.descIndex = -1;
    binding.registerDesc = registerDesc;
    m_bindings.Add(binding);
}

void BindingState::Desc::addCombinedTextureSampler(TextureResource* resource, const SamplerDesc& samplerDesc, const RegisterDesc& registerDesc)
{
    assert(resource);

    int samplerDescIndex = int(m_samplers.Count());
    m_samplers.Add(samplerDesc);

    Binding binding;
    binding.bindingType = BindingType::CombinedTextureSampler;
    binding.resource = resource;
    binding.descIndex = samplerDescIndex;
    binding.registerDesc = registerDesc;
    m_bindings.Add(binding);
}

BindingState::RegisterSet BindingState::Desc::addRegisterSet(int index)
{
    if (index < 0)
    {
        return RegisterSet();
    }
    return RegisterSet(index, 1);
}

BindingState::RegisterSet BindingState::Desc::addRegisterSet(const int* srcIndices, int numIndices)
{
    assert(numIndices >= 0);
    switch (numIndices)
    {
        case 0:     return RegisterSet(); 
        case 1:     return RegisterSet(srcIndices[0], 1);
        default:
        {
            int startIndex = int(m_indices.Count());
            m_indices.SetSize(startIndex + numIndices);
            uint16_t* dstIndices = m_indices.Buffer() + startIndex;
            for (int i = 0; i < numIndices; i++)
            {
                dstIndices[i] = uint16_t(srcIndices[i]);
            }
            return RegisterSet(startIndex, numIndices);
        }
    }
}

int BindingState::Desc::getFirst(const RegisterSet& set) const
{
    switch (set.m_numIndices)
    {
        case 0:             return -1;
        case 1:             return set.m_indexOrBase;
        default:            return m_indices[set.m_indexOrBase];
    }
}

void BindingState::Desc::clear()
{
    m_bindings.Clear();
    m_samplers.Clear();
    m_indices.Clear();
    m_numRenderTargets = 1;
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

} // renderer_test
