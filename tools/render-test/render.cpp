// render.cpp

#include "render.h"


namespace renderer_test {

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
}; 

} // renderer_test
