#include "driver-replay.h"

#include <stdio.h>

SlangResult DriverReplay::init(rhi::DeviceType deviceType)
{
    rhi::IRHI* rhiInstance = rhi::getRHI();
    if (!rhiInstance)
    {
        fprintf(stderr, "[DriverReplay] Failed to get RHI instance\n");
        return SLANG_FAIL;
    }

    if (!rhiInstance->isDeviceTypeSupported(deviceType))
    {
        fprintf(
            stderr,
            "[DriverReplay] Device type '%s' is not supported on this system\n",
            rhiInstance->getDeviceTypeName(deviceType));
        return SLANG_FAIL;
    }

    rhi::DeviceDesc desc = {};
    desc.deviceType = deviceType;

    SlangResult result = rhiInstance->createDevice(desc, m_device.writeRef());
    if (SLANG_FAILED(result))
    {
        fprintf(stderr, "[DriverReplay] Failed to create device (0x%08x)\n", (unsigned)result);
        return result;
    }

    fprintf(
        stdout,
        "[DriverReplay] Created %s device\n",
        rhiInstance->getDeviceTypeName(deviceType));
    return SLANG_OK;
}

void DriverReplay::tryCreateComputePipeline(slang::IComponentType* linkedProgram)
{
    if (!linkedProgram || !m_device)
        return;

    if (m_attempted.count(linkedProgram))
        return;
    m_attempted.insert(linkedProgram);

    auto* layout = linkedProgram->getLayout();
    if (!layout)
        return;

    SlangUInt entryPointCount = layout->getEntryPointCount();
    if (entryPointCount == 0)
        return;

    for (SlangUInt i = 0; i < entryPointCount; i++)
    {
        auto* ep = layout->getEntryPointByIndex(i);
        if (!ep || ep->getStage() != SLANG_STAGE_COMPUTE)
        {
            m_skipCount++;
            return;
        }
    }

    const char* entryPointName = layout->getEntryPointByIndex(0)->getName();

    rhi::ShaderProgramDesc programDesc = {};
    programDesc.slangGlobalScope = linkedProgram;

    Slang::ComPtr<rhi::IShaderProgram> shaderProgram;
    SlangResult result =
        m_device->createShaderProgram(programDesc, shaderProgram.writeRef(), nullptr);
    if (SLANG_FAILED(result))
    {
        fprintf(
            stderr,
            "[DriverReplay] FAIL createShaderProgram for '%s' (0x%08x)\n",
            entryPointName ? entryPointName : "?",
            (unsigned)result);
        m_failCount++;
        return;
    }

    rhi::ComputePipelineDesc pipelineDesc = {};
    pipelineDesc.program = shaderProgram;

    Slang::ComPtr<rhi::IComputePipeline> pipeline;
    result = m_device->createComputePipeline(pipelineDesc, pipeline.writeRef());
    if (SLANG_FAILED(result))
    {
        fprintf(
            stderr,
            "[DriverReplay] FAIL createComputePipeline for '%s' (0x%08x)\n",
            entryPointName ? entryPointName : "?",
            (unsigned)result);
        m_failCount++;
        return;
    }

    fprintf(
        stdout,
        "[DriverReplay] OK compute pipeline for '%s'\n",
        entryPointName ? entryPointName : "?");
    m_successCount++;
}

void DriverReplay::printSummary() const
{
    fprintf(
        stdout,
        "\n[DriverReplay] Summary: %d succeeded, %d failed, %d skipped (non-compute)\n",
        m_successCount,
        m_failCount,
        m_skipCount);
}
