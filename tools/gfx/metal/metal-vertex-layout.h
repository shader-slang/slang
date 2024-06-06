// metal-vertex-layout.h
#pragma once

#include "metal-base.h"

namespace gfx
{

using namespace Slang;

namespace metal
{

class InputLayoutImpl : public InputLayoutBase
{
public:
    NS::SharedPtr<MTL::VertexDescriptor> m_vertexDescriptor;
};

} // namespace metal
} // namespace gfx
