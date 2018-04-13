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

int TextureResource::Desc::calcMaxDimensionSize(Type type) const
{
    switch (type)
    {
        case Resource::Type::Texture1D:     return this->width;
        case Resource::Type::TextureCube:
        case Resource::Type::Texture2D:     return std::max(this->width, this->height);
        case Resource::Type::Texture3D:     return std::max(std::max(this->width, this->height), this->depth);
        default: return 0;
    }
}

int TextureResource::Desc::calcNumMipMaps(Type type) const
{
    const int maxDimensionSize = calcMaxDimensionSize(type);
    return (maxDimensionSize > 0) ? (Math::Log2Floor(maxDimensionSize) + 1) : 0;
}

int TextureResource::Desc::calcNumSubResources(Type type) const
{
    const int numMipMaps = (this->numMipLevels > 0) ? this->numMipLevels : calcNumMipMaps(type);
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
            return numMipMaps * this->depth;  
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
            this->height = 1;
            this->depth = 1;
            break;
        }
        case Resource::Type::TextureCube:
        case Resource::Type::Texture2D:
        {
            this->depth = 1;
            break;
        }
        case Resource::Type::Texture3D:
        {
            this->arraySize = 1;
            break;
        }
        default: break;
    }
}

int TextureResource::Desc::calcEffectiveArraySize(Type type) const
{
    const int arrSize = (this->arraySize > 0) ? this->arraySize : 1;

    switch (type)
    {
        case Resource::Type::Texture1D:
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
    this->width = 1;
    this->height = 1;
    this->depth = 1;

    this->format = Format::Unknown;
    this->arraySize = 0;
    this->numMipLevels = 0;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->accessFlags = 0;
}

void TextureResource::Desc::init1D(Format formatIn, int widthIn, int numMipMapsIn)
{
    this->width = widthIn;
    this->height = 1;
    this->depth = 1;
    
    this->format = format;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->accessFlags = 0; 
}

void TextureResource::Desc::init2D(Format formatIn, int widthIn, int heightIn, int numMipMapsIn)
{
    this->width = widthIn;
    this->height = heightIn;
    this->depth = 1;
    
    this->format = format;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->accessFlags = 0;
}

void TextureResource::Desc::init3D(Format formatIn, int widthIn, int heightIn, int depthIn, int numMipMapsIn)
{
    
    this->width = widthIn;
    this->height = heightIn;
    this->depth = depthIn;

    this->format = format;
    this->arraySize = 0;
    this->numMipLevels = numMipMapsIn;
    this->sampleDesc.init();

    this->bindFlags = 0;
    this->accessFlags = 0;
}

} // renderer_test
