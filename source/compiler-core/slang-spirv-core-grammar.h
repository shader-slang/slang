#pragma once

#include "../core/slang-smart-pointer.h"
#include "../core/slang-string.h"
#include "../core/slang-dictionary.h"
#include "../../external/spirv-headers/include/spirv/unified1/spirv.h"

namespace Slang
{
    struct SPIRVCoreGrammarInfo : public RefObject
    {
        // Returns SpvOpMax (0x7fffffff) on failure, which couldn't possibly be
        // a valid 16 bit opcode
        SpvOp lookupSpvOp(const UnownedStringSlice& opname) const
        {
            SpvOp ret = SpvOpMax;
            opcodes.tryGetValue(opname, ret)
                || opcodes.tryGetValue(String("Op") + opname, ret);
            return ret;
        }

        Dictionary<String, SpvOp> opcodes;
    };

    class DiagnosticSink;
    class SourceView;

    RefPtr<SPIRVCoreGrammarInfo> loadSPIRVCoreGrammarInfo(SourceView& source, DiagnosticSink& sink);
    RefPtr<SPIRVCoreGrammarInfo> getEmbeddedSPIRVCoreGrammarInfo();
}
