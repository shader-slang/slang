#include "gfx-test-util.h"

#include <slang-com-ptr.h>

using Slang::ComPtr;

namespace gfx_test
{
    void diagnoseIfNeeded(ISlangWriter* diagnosticWriter, slang::IBlob* diagnosticsBlob)
    {
        if (diagnosticsBlob != nullptr)
        {
            diagnosticWriter->write((const char*)diagnosticsBlob->getBufferPointer(), diagnosticsBlob->getBufferSize());
        }
    }

    Slang::Result loadShaderProgram(
        gfx::IDevice* device,
        Slang::ComPtr<gfx::IShaderProgram>& outShaderProgram,
        ISlangWriter* diagnosticWriter,
        const char* shaderModuleName,
        slang::ProgramLayout*& slangReflection)
    {
        Slang::ComPtr<slang::ISession> slangSession;
        SLANG_RETURN_ON_FAIL(device->getSlangSession(slangSession.writeRef()));
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        slang::IModule* module = slangSession->loadModule(shaderModuleName, diagnosticsBlob.writeRef());
        diagnoseIfNeeded(diagnosticWriter, diagnosticsBlob);
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
        diagnoseIfNeeded(diagnosticWriter, diagnosticsBlob);
        SLANG_RETURN_ON_FAIL(result);
        slangReflection = composedProgram->getLayout();

        gfx::IShaderProgram::Desc programDesc = {};
        programDesc.pipelineType = gfx::PipelineType::Compute;
        programDesc.slangProgram = composedProgram.get();

        auto shaderProgram = device->createProgram(programDesc);

        outShaderProgram = shaderProgram;
        return SLANG_OK;
    }

    Slang::Result compareComputeResult(gfx::IDevice* device, gfx::IBufferResource* buffer, uint8_t* expectedResult, size_t expectedBufferSize)
    {
        // Read back the results.
        ComPtr<ISlangBlob> resultBlob;
        SLANG_RETURN_ON_FAIL(device->readBufferResource(
            buffer, 0, expectedBufferSize, resultBlob.writeRef()));
        if (resultBlob->getBufferSize() < expectedBufferSize)
            return SLANG_FAIL;

        // Compare results.
        auto result = reinterpret_cast<const uint8_t*>(resultBlob->getBufferPointer());
        for (int i = 0; i < expectedBufferSize; i++)
        {
            if (expectedResult[i] != result[i])
                return SLANG_FAIL;
        }
        return SLANG_OK;
    }
}
