#pragma once

#include <cstdio>
#include <filesystem>
#include <fstream>
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

// Write application output after the caller has waited for all GPU submissions.
// This optional readback is independent of coverage artifacts, so regressions can
// compare covered and uncovered execution without changing the timing loop.
inline void writeOutputBuffer(
    rhi::IDevice* device,
    rhi::IBuffer* buffer,
    const std::filesystem::path& path)
{
    rhi::ComPtr<slang::IBlob> data;
    auto result = device->readBuffer(buffer, 0, buffer->getDesc().size, data.writeRef());
    if (SLANG_FAILED(result))
        throw std::runtime_error(
            "read output buffer failed with SlangResult " + std::to_string(result));
    std::ofstream output(path, std::ios::binary);
    output.write(static_cast<const char*>(data->getBufferPointer()), data->getBufferSize());
    output.close();
    if (!output)
        throw std::runtime_error("failed to write output buffer to " + path.string());
}

// Coverage in these demos produces one global read/write buffer. Translate its
// compiler metadata before creating the RHI program; ordinary reflection does
// not include this buffer. debugName borrows storage from metadata, which the
// caller keeps alive through createShaderProgram().
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
inline rhi::ComPtr<rhi::IBuffer> createStorageBuffer(
    rhi::IDevice* device,
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
    if (SLANG_FAILED(device->createBuffer(desc, initialData, buffer.writeRef())))
        throw std::runtime_error("createStorageBuffer failed");
    return buffer;
}

} // namespace coverageDemo
