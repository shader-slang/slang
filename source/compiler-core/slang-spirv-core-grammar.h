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
    class DiagnosticSink;
    class SourceView;

    struct SPIRVCoreGrammarInfo : public RefObject
    {
        static RefPtr<SPIRVCoreGrammarInfo> loadFromJSON(SourceView& source, DiagnosticSink& sink);
        static RefPtr<SPIRVCoreGrammarInfo> getEmbeddedVersion();

        template<typename K, typename T>
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

        template<typename K, typename T, T onFailure>
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
        Lookup<UnownedStringSlice, SpvOp, SpvOpMax> spvOps;

        // Returns SpvCapabilityMax (0x7fffffff) on failure
        Lookup<UnownedStringSlice, SpvCapability, SpvCapabilityMax> spvCapabilities;

        // Returns std::nullopt on failure
        // Looks up a qualified enum name, i.e. one with the type prefix.
        LookupOpt<UnownedStringSlice, SpvWord> anyEnum;

        struct OpInfo
        {
            enum Class
            {
                Other,
                TypeDeclaration,
                ConstantCreation
            };
            Class class_;
            // -1 or 0
            int8_t resultTypeIndex = -1;
            // -1 or 0 or 1
            int8_t resultIdIndex = -1;
        };
        LookupOpt<SpvOp, OpInfo> opInfo;
        LookupOpt<SpvOp, UnownedStringSlice> opNames;

    private:

        // If this is loaded from JSON, we keep the strings around instead of
        // copying them as dictionary keys
        StringSlicePool strings = StringSlicePool(StringSlicePool::Style::Empty);
    };
}
