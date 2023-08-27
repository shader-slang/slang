#pragma once

#include "../core/slang-smart-pointer.h"
#include "../core/slang-string.h"
#include "../core/slang-string-slice-pool.h"
#include "../core/slang-dictionary.h"
#include "../../external/spirv-headers/include/spirv/unified1/spirv.h"
#include <optional>

namespace Slang
{
    using SpvWord = uint32_t;

    struct SPIRVCoreGrammarInfo : public RefObject
    {
        template<typename T, typename K = UnownedStringSlice>
        struct LookupOpt
        {
            std::optional<T> lookup(const K& name) const
            {
                T ret;
                const auto found = embedded ? embedded(name, ret) : dict.tryGetValue(name, ret);
                return found ? ret : std::nullopt;
            }

            bool (*embedded)(const K&, T&) = nullptr;
            Dictionary<K, T> dict;
        };

        template<typename T, T onFailure, typename K = UnownedStringSlice>
        struct Lookup
        {
            T lookup(const K& name) const
            {
                T ret;
                const auto found = embedded ? embedded(name, ret) : dict.tryGetValue(name, ret);
                return found ? ret : onFailure;
            }

            bool (*embedded)(const K&, T&) = nullptr;
            Dictionary<K, T> dict;
        };

        // Returns SpvOpMax (0x7fffffff) on failure, which couldn't possibly be
        // a valid 16 bit opcode
        Lookup<SpvOp, SpvOpMax> spvOps;

        // Returns SpvCapabilityMax (0x7fffffff) on failure
        Lookup<SpvCapability, SpvCapabilityMax> spvCapabilities;

        // Returns std::nullopt on failure
        LookupOpt<SpvWord> anyEnum;

        // If this is loaded from JSON, we keep the strings around instead of
        // copying them as dictionary keys
        StringSlicePool strings = StringSlicePool(StringSlicePool::Style::Empty);
    };

    class DiagnosticSink;
    class SourceView;

    RefPtr<SPIRVCoreGrammarInfo> loadSPIRVCoreGrammarInfo(SourceView& source, DiagnosticSink& sink);
    RefPtr<SPIRVCoreGrammarInfo> getEmbeddedSPIRVCoreGrammarInfo();
}
