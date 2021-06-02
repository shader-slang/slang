#pragma once

#include "slang-gfx.h"

namespace gfx
{
    class SlangContext
    {
    public:
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        Slang::ComPtr<slang::ISession> session;
        Result initialize(const gfx::IDevice::SlangDesc& desc, SlangCompileTarget compileTarget, const char* defaultProfileName)
        {
            if (desc.slangGlobalSession)
            {
                globalSession = desc.slangGlobalSession;
            }
            else
            {
                SLANG_RETURN_ON_FAIL(slang::createGlobalSession(globalSession.writeRef()));
            }

            slang::SessionDesc slangSessionDesc = {};
            slangSessionDesc.defaultMatrixLayoutMode = desc.defaultMatrixLayoutMode;
            slangSessionDesc.searchPathCount = desc.searchPathCount;
            slangSessionDesc.searchPaths = desc.searchPaths;
            slangSessionDesc.preprocessorMacroCount = desc.preprocessorMacroCount;
            slangSessionDesc.preprocessorMacros = desc.preprocessorMacros;
            slang::TargetDesc targetDesc = {};
            targetDesc.format = compileTarget;
            auto targetProfile = desc.targetProfile;
            if (targetProfile == nullptr)
                targetProfile = defaultProfileName;
            targetDesc.profile = globalSession->findProfile(targetProfile);
            targetDesc.optimizationLevel = desc.optimizationLevel;
            targetDesc.floatingPointMode = desc.floatingPointMode;
            targetDesc.lineDirectiveMode = desc.lineDirectiveMode;
            targetDesc.flags = desc.targetFlags;
            slangSessionDesc.targetCount = 1;
            slangSessionDesc.targets = &targetDesc;
            SLANG_RETURN_ON_FAIL(globalSession->createSession(slangSessionDesc, session.writeRef()));
            return SLANG_OK;
        }
    };
}
