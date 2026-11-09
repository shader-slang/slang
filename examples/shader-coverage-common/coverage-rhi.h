#pragma once

#include <cstdio>
#include <slang-rhi.h>
#include <slang-rhi/synthetic-bindings.h>
#include <stdexcept>
#include <string>

namespace coverageDemo
{

// Forward RHI diagnostics to stderr, including the backend's explanation of failures.
// The callback must outlive the device. A single fprintf call keeps each message together
// when RHI reports from multiple threads.
class DiagnosticCallback : public rhi::IDebugCallback
{
public:
    SLANG_NO_THROW void SLANG_MCALL handleMessage(
        rhi::DebugMessageType type,
        rhi::DebugMessageSource source,
        const char* message) override
    {
        std::fprintf(stderr, "RHI: %s\n", message);
    }
};

// Coverage in these demos produces one global read/write buffer. Translate its
// compiler metadata before creating the RHI program; ordinary reflection does
// not include this buffer. debugName borrows storage from metadata, which the
// caller keeps alive through createShaderProgram().
//
// Returns SLANG_FAIL if the metadata does not describe exactly one resource,
// SLANG_E_NOT_IMPLEMENTED if that resource is not a single global read/write
// buffer (the only shape these demos support), and SLANG_OK otherwise.
inline SlangResult getCoverageResourceDesc(
    slang::IMetadata* metadata,
    rhi::SyntheticResourceBindingDesc& desc)
{
    auto* resources = static_cast<slang::ISyntheticResourceMetadata*>(
        metadata->castAs(slang::ISyntheticResourceMetadata::getTypeGuid()));
    if (!resources || resources->getResourceCount() != 1)
        return SLANG_FAIL;

    slang::SyntheticResourceInfo info = {};
    SLANG_RETURN_ON_FAIL(resources->getResourceInfo(0, &info));
    if (info.scope != slang::SyntheticResourceScope::Global ||
        info.access != slang::SyntheticResourceAccess::ReadWrite || info.arraySize != 1)
        return SLANG_E_NOT_IMPLEMENTED;

    desc.id = info.id;
    desc.bindingType = info.bindingType;
    desc.arraySize = info.arraySize;
    // Fixed by the shape check above; update both together if it is relaxed.
    desc.scope = rhi::SyntheticResourceScope::Global;
    desc.access = rhi::SyntheticResourceAccess::ReadWrite;
    desc.entryPointIndex = info.entryPointIndex;
    desc.space = info.space;
    desc.binding = info.binding;
    desc.uniformOffset = info.uniformOffset;
    desc.uniformStride = info.uniformStride;
    desc.debugName = info.debugName;
    return SLANG_OK;
}

// All demo resources are storage buffers. Initial data is uploaded by RHI;
// CopyDestination also permits updating the dispatch parameters between batches.
// `name` identifies the buffer in the error if allocation fails. `elementSize`
// is the shader-side element stride in bytes (for example sizeof(Ray) for a
// StructuredBuffer<Ray>, or the counter width for the coverage buffer), not the
// total buffer size; RHI reports it back through reflection.
inline rhi::ComPtr<rhi::IBuffer> createStorageBuffer(
    rhi::IDevice* device,
    const char* name,
    size_t byteSize,
    uint32_t elementSize,
    const void* initialData = nullptr)
{
    rhi::BufferDesc desc = {};
    desc.size = byteSize;
    desc.elementSize = elementSize;
    desc.usage = rhi::BufferUsage::ShaderResource | rhi::BufferUsage::UnorderedAccess |
                 rhi::BufferUsage::CopySource | rhi::BufferUsage::CopyDestination;
    desc.defaultState = rhi::ResourceState::UnorderedAccess;
    desc.memoryType = rhi::MemoryType::DeviceLocal;
    rhi::ComPtr<rhi::IBuffer> buffer;
    const SlangResult result = device->createBuffer(desc, initialData, buffer.writeRef());
    if (SLANG_FAILED(result))
        throw std::runtime_error(
            std::string("createBuffer(") + name + ") failed with SlangResult " +
            std::to_string(result));
    return buffer;
}

} // namespace coverageDemo
