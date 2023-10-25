#pragma once

#include "slang-gfx.h"
#include "core/slang-basic.h"

namespace gfx
{
    class SlangContext
    {
    public:
        Slang::ComPtr<slang::IGlobalSession> globalSession;
        Slang::ComPtr<slang::ISession> session;
        Result initialize(const gfx::IDevice::SlangDesc& desc, SlangCompileTarget compileTarget, const char* defaultProfileName,
            Slang::ConstArrayView<slang::PreprocessorMacroDesc> additionalMacros)
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
            slangSessionDesc.preprocessorMacroCount = desc.preprocessorMacroCount + additionalMacros.getCount();
            Slang::List<slang::PreprocessorMacroDesc> macros;
            macros.addRange(desc.preprocessorMacros, desc.preprocessorMacroCount);
            macros.addRange(additionalMacros.getBuffer(), additionalMacros.getCount());
            slangSessionDesc.preprocessorMacros = macros.getBuffer();
            slang::TargetDesc targetDesc = {};
            targetDesc.format = compileTarget;
            auto targetProfile = desc.targetProfile;
            if (targetProfile == nullptr)
                targetProfile = defaultProfileName;
            targetDesc.profile = globalSession->findProfile(targetProfile);
            targetDesc.floatingPointMode = desc.floatingPointMode;
            targetDesc.lineDirectiveMode = desc.lineDirectiveMode;
            targetDesc.flags = desc.targetFlags;
            targetDesc.forceGLSLScalarBufferLayout = true;

            slangSessionDesc.targets = &targetDesc;
            slangSessionDesc.targetCount = 1;

            SLANG_RETURN_ON_FAIL(globalSession->createSession(slangSessionDesc, session.writeRef()));
            return SLANG_OK;
        }
    };
}
