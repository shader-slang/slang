#include "gfx-test-util.h"
#include "tools/unit-test/slang-unit-test.h"

#include <slang-com-ptr.h>

using Slang::ComPtr;

namespace gfx_test
{
    void diagnoseIfNeeded(slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            getTestReporter()->message(TestMessageType::Info, (const char*)diagnosticsBlob->getBufferPointer());
        }
    }

    Slang::Result loadShaderProgram(
        gfx::IDevice* device,
        Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
        const char* shaderModuleName,
        slang::ProgramLayout*& slangReflection)
    {
        Slang::ComPtr<slang::ISession> slangSession;
        SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        slang::IModule* module = slangSession->loadModule(shaderModuleName, diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        if (!module)
            return SLANG_FAIL;

        char const* computeEntryPointName = "computeMain";
        ComPtr<slang::IEntryPoint> computeEntryPoint;
        SLANG_RETURN_ON_FAIL(
            module->findEntryPointByName(computeEntryPointName, computeEntryPoint.writeRef()));

        Slang::List<slang::IComponentType*> componentTypes;
        componentTypes.add(module);
        componentTypes.add(computeEntryPoint);

        Slang::ComPtr<slang::IComponentType> composedProgram;
        SlangResult result = slangSession->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            composedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);
        slangReflection = composedProgram->getLayout();

        gfx::IShaderProgram::Desc programDesc = {};
        programDesc.pipelineType = gfx::PipelineType::Compute;
        programDesc.slangProgram = composedProgram.get();

        auto shaderProgram = device->createProgram(programDesc);

        outShaderProgram = shaderProgram;
        return SLANG_OK;
    }

    void compareComputeResult(gfx::IDevice* device, gfx::IBufferResource* buffer, uint8_t* expectedResult, size_t expectedBufferSize)
    {
        // Read back the results.
        ComPtr<ISlangBlob> resultBlob;
        GFX_CHECK_CALL_ABORT(device->readBufferResource(
            buffer, 0, expectedBufferSize, resultBlob.writeRef()));
        if (resultBlob->getBufferSize() < expectedBufferSize)
        {
            getTestReporter()->addResult(TestResult::Fail);
            return;
        }

        // Compare results.
        auto result = reinterpret_cast<const uint8_t*>(resultBlob->getBufferPointer());
        for (int i = 0; i < expectedBufferSize; i++)
        {
            if (expectedResult[i] != result[i])
            {
                getTestReporter()->addResult(TestResult::Fail);
                return;
            }

        }
        getTestReporter()->addResult(TestResult::Pass);
    }
}
