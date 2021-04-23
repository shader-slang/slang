#include "resource-desc-utils.h"

namespace gfx
{
IBufferResource::Desc fixupBufferDesc(const IBufferResource::Desc& desc)
{
    IBufferResource::Desc result = desc;
    result.allowedStates.add(result.defaultState);
    return result;
}

ITextureResource::Desc fixupTextureDesc(const ITextureResource::Desc& desc)
{
    ITextureResource::Desc rs = desc;
    if (desc.numMipLevels == 0)
        rs.numMipLevels = calcNumMipLevels(desc.type, desc.size);
    rs.allowedStates.add(rs.defaultState);
    return rs;
}
}
