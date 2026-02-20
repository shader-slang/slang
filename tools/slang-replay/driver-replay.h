#pragma once

#include <slang-com-ptr.h>
#include <slang-rhi.h>
#include <slang.h>

#include <unordered_set>

/// Manages an RHI device and attempts to create compute pipelines
/// from linked IComponentType objects produced during replay.
class DriverReplay
{
public:
    SlangResult init(rhi::DeviceType deviceType);

    /// Attempt to create a compute pipeline from a linked component type.
    /// Skips non-compute programs and programs already attempted.
    void tryCreateComputePipeline(slang::IComponentType* linkedProgram);

    void printSummary() const;

    int getSuccessCount() const { return m_successCount; }
    int getFailCount() const { return m_failCount; }
    int getSkipCount() const { return m_skipCount; }

private:
    Slang::ComPtr<rhi::IDevice> m_device;
    std::unordered_set<slang::IComponentType*> m_attempted;
    int m_successCount = 0;
    int m_failCount = 0;
    int m_skipCount = 0;
};
