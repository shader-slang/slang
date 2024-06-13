#include "parameter-encoder.h"

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
        encodeUint32(desc.defaultMatrixLayoutMode);
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
        encodeInt32((int32_t)(entry.name));
        encodeStruct(entry.value);
    }

    void ParameterEncoder::encodeStruct(slang::CompilerOptionValue const& value)
    {
        (void)value;
    }

    void ParameterEncoder::encodeStruct(slang::TargetDesc const& targetDesc)
    {
        (void)targetDesc;
    }

    void ParameterEncoder::encodePointer(const void* value, bool omitData, size_t size)
    {
        (void)value;
        (void)omitData;
        (void)size;
    }
    // first 4-bytes is the length of the string
    void ParameterEncoder::encodeString(const char* value)
    {
        (void)value;
    }
}
