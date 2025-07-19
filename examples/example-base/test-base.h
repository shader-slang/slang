#pragma once

#include "core/slang-string-util.h"
#include "slang-com-ptr.h"
#include "slang-rhi.h"
#include "slang.h"

using Slang::ComPtr;

class TestBase
{

public:
    // Parses command line options including help, API selection, and test mode.
    int parseOption(int argc, char** argv);

    void printEntrypointHashes(
        int entryPointCount,
        int targetCount,
        ComPtr<slang::IComponentType>& composedProgram);

    bool isTestMode() const { return m_isTestMode; }
    bool shouldShowHelp() const { return m_showHelp; }
    rhi::DeviceType getDeviceType() const { return m_deviceType; }

    void printUsage(const char* programName) const;

private:
    bool m_isTestMode = false;
    bool m_showHelp = false;
    rhi::DeviceType m_deviceType = rhi::DeviceType::Default;
    uint64_t m_globalCounter = 0;
};
