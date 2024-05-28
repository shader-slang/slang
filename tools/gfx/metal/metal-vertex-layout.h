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
    List<MTL::VertexDescriptor*> m_vertexDescs;
    List<MTL::VertexBufferLayoutDescriptor*> m_bufferLayoutDescs;
};

} // namespace metal
} // namespace gfx
