#include "parameter-encoder.h"
#include "api_callId.h"

namespace SlangCapture
{
    void ParameterEncoder::encodeStruct(slang::SessionDesc const& desc)
    {
        encodeUint64(desc.structureSize);
        encodeInt64(desc.targetCount);

        for (SlangInt i = 0; i < desc.targetCount; i++)
        {
            encodeStruct(desc.targets[i]);
        }

        encodeUint32(desc.flags);
        encodeEnumValue(desc.defaultMatrixLayoutMode);
        encodeInt64(desc.searchPathCount);
        for (SlangInt i = 0; i < desc.searchPathCount; i++)
        {
            encodeString(desc.searchPaths[i]);
        }

        encodeInt64(desc.preprocessorMacroCount);
        for (SlangInt i = 0; i < desc.preprocessorMacroCount; i++)
        {
            encodeStruct(desc.preprocessorMacros[i]);
        }

        encodeBool(desc.enableEffectAnnotations);
        encodeBool(desc.allowGLSLSyntax);

        encodeUint32(desc.compilerOptionEntryCount);
        for (uint32_t i = 0; i < desc.compilerOptionEntryCount; i++)
        {
            encodeStruct(desc.compilerOptionEntries[i]);
        }
    }

    void ParameterEncoder::encodeStruct(slang::PreprocessorMacroDesc const& desc)
    {
        encodeString(desc.name);
        encodeString(desc.value);
    }

    void ParameterEncoder::encodeStruct(slang::CompilerOptionEntry const& entry)
    {
        encodeEnumValue(entry.name);
        encodeStruct(entry.value);
    }

    void ParameterEncoder::encodeStruct(slang::CompilerOptionValue const& value)
    {
        encodeEnumValue(value.kind);
        encodeInt32(value.intValue0);
        encodeString(value.stringValue0);
        encodeString(value.stringValue1);
    }

    void ParameterEncoder::encodeStruct(slang::TargetDesc const& targetDesc)
    {
        encodeUint64(targetDesc.structureSize);
        encodeEnumValue(targetDesc.format);
        encodeEnumValue(targetDesc.profile);
        encodeEnumValue(targetDesc.flags);
        encodeEnumValue(targetDesc.floatingPointMode);
        encodeEnumValue(targetDesc.lineDirectiveMode);
        encodeBool(targetDesc.forceGLSLScalarBufferLayout);
        encodeUint32(targetDesc.compilerOptionEntryCount);
        for (uint32_t i = 0; i < targetDesc.compilerOptionEntryCount; i++)
        {
            encodeStruct(targetDesc.compilerOptionEntries[i]);
        }
    }

    void ParameterEncoder::encodeStruct(slang::SpecializationArg const& specializationArg)
    {
        encodeEnumValue(specializationArg.kind);
        encodeAddress(specializationArg.type);
    }

    void ParameterEncoder::encodePointer(const void* value, bool omitData, size_t size)
    {
        encodeAddress(value);
        if (omitData)
        {
            return;
        }

        encodeUint64(size);
        if (size)
        {
            m_stream->write(value, size);
        }
    }

    void ParameterEncoder::encodePointer(ISlangBlob* blob)
    {
        encodeAddress(static_cast<const void*>(blob));

        if (blob)
        {
            size_t size = blob->getBufferSize();
            const void* buffer = blob->getBufferPointer();
            encodePointer(buffer, false, size);
        }
    }

    // first 4-bytes is the length of the string
    void ParameterEncoder::encodeString(const char* value)
    {
        if (value == nullptr)
        {
            encodeUint32(0);
        }
        else
        {
            uint32_t size = (uint32_t)strlen(value);
            encodeUint32(size);
            m_stream->write(value, size);
        }
    }
}
