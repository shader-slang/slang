#pragma once

#include "../core/slang-smart-pointer.h"
#include "../core/slang-string.h"
#include "../core/slang-dictionary.h"
#include "../../external/spirv-headers/include/spirv/unified1/spirv.h"
#include <optional>

namespace Slang
{
    using SpvWord = uint32_t;

    struct SPIRVCoreGrammarInfo : public RefObject
    {
        template<typename T>
        struct LookupOpt
        {
            std::optional<T> lookup(const UnownedStringSlice& name) const
            {
                T ret;
                const auto found = embedded ? embedded(name, ret) : dict.tryGetValue(name, ret);
                return found ? ret : std::nullopt;
            }

            bool (*embedded)(const UnownedStringSlice&, T&) = nullptr;
            Dictionary<String, T> dict;
        };

        template<typename T, T onFailure>
        struct Lookup
        {
            T lookup(const UnownedStringSlice& name) const
            {
                T ret;
                const auto found = embedded ? embedded(name, ret) : dict.tryGetValue(name, ret);
                return found ? ret : onFailure;
            }

            bool (*embedded)(const UnownedStringSlice&, T&) = nullptr;
            Dictionary<String, T> dict;
        };


        // Returns SpvOpMax (0x7fffffff) on failure, which couldn't possibly be
        // a valid 16 bit opcode
        Lookup<SpvOp, SpvOpMax> spvOps;

        // Returns SpvCapabilityMax (0x7fffffff) on failure
        Lookup<SpvCapability, SpvCapabilityMax> spvCapabilities;

        // Returns std::nullopt on failure
        LookupOpt<SpvWord> anyEnum;
    };

    class DiagnosticSink;
    class SourceView;

    RefPtr<SPIRVCoreGrammarInfo> loadSPIRVCoreGrammarInfo(SourceView& source, DiagnosticSink& sink);
    RefPtr<SPIRVCoreGrammarInfo> getEmbeddedSPIRVCoreGrammarInfo();
}
