#pragma once

#include "../core/slang-smart-pointer.h"
#include "../core/slang-string.h"
#include "../core/slang-dictionary.h"
#include "../../external/spirv-headers/include/spirv/unified1/spirv.h"

namespace Slang
{
    struct SPIRVCoreGrammarInfo : public RefObject
    {
        template<typename T, T onFailure>
        struct Lookup
        {
            T lookup(const UnownedStringSlice& name) const
            {
                T ret = onFailure;
                if(embedded)
                    embedded(name, ret);
                else
                    dict.tryGetValue(name, ret);
                return ret;
            }

            bool (*embedded)(const UnownedStringSlice&, T&) = nullptr;
            Dictionary<String, T> dict;
        };

        // Returns SpvOpMax (0x7fffffff) on failure, which couldn't possibly be
        // a valid 16 bit opcode
        Lookup<SpvOp, SpvOpMax> spvOps;

        // Returns SpvCapabilityMax (0x7fffffff) on failure
        Lookup<SpvCapability, SpvCapabilityMax> spvCapabilities;
    };

    class DiagnosticSink;
    class SourceView;

    RefPtr<SPIRVCoreGrammarInfo> loadSPIRVCoreGrammarInfo(SourceView& source, DiagnosticSink& sink);
    RefPtr<SPIRVCoreGrammarInfo> getEmbeddedSPIRVCoreGrammarInfo();
}
