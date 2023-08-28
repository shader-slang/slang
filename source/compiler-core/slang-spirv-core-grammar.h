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
            constexpr static int8_t kNoResultTypeId = -1;
            constexpr static int8_t kNoResultId = -1;

            Class class_;
            // -1 or 0
            int8_t resultTypeIndex = kNoResultTypeId;
            // -1 or 0 or 1
            int8_t resultIdIndex = kNoResultId;
            // The range of valid WordCount for this instruction
            uint16_t minWordCount;
            uint16_t maxWordCount;
        };
        LookupOpt<SpvOp, OpInfo> opInfo;
        LookupOpt<SpvOp, UnownedStringSlice> opNames;

        struct EnumCategory
        {
            int32_t index; 
        };
        LookupOpt<UnownedStringSlice, EnumCategory> enumCategories;

    private:

        // If this is loaded from JSON, we keep the strings around instead of
        // copying them as dictionary keys
        StringSlicePool strings = StringSlicePool(StringSlicePool::Style::Empty);
    };
}
