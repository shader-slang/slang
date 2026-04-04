// slang-options-value-category.h
//
// Shared internal header for ValueCategory enum and related macros.
// Used by both slang-options.cpp and slang-options-init.cpp to ensure
// consistent mapping between types and CommandOptions::UserValue integers.

#ifndef SLANG_OPTIONS_VALUE_CATEGORY_H
#define SLANG_OPTIONS_VALUE_CATEGORY_H

#include "../compiler-core/slang-source-embed-util.h"
#include "../core/slang-command-options-writer.h"
#include "../core/slang-type-text-util.h"
#include "slang-compiler-options.h"
#include "slang-hlsl-to-vulkan-layout-options.h"
#include "slang-profile.h"

namespace Slang
{

// ValueCategory enum maps types to CommandOptions::UserValue integers.
// CRITICAL: This enum must remain in sync across all uses to ensure
// correct option parsing.
enum class ValueCategory
{
    Compiler,
    Target,
    Language,
    FloatingPointMode,
    FloatingPointDenormalMode,
    ArchiveType,
    Stage,
    LineDirectiveMode,
    DebugInfoFormat,
    HelpStyle,
    OptimizationLevel,
    DebugLevel,
    FileSystemType,
    VulkanShift,
    SourceEmbedStyle,
    LanguageVersion,

    CountOf,
};

template<typename T>
struct GetValueCategory;

#define SLANG_GET_VALUE_CATEGORY(cat, type)   \
    template<>                                \
    struct GetValueCategory<type>             \
    {                                         \
        enum                                  \
        {                                     \
            Value = Index(ValueCategory::cat) \
        };                                    \
    };

SLANG_GET_VALUE_CATEGORY(Compiler, SlangPassThrough)
SLANG_GET_VALUE_CATEGORY(ArchiveType, SlangArchiveType)
SLANG_GET_VALUE_CATEGORY(LineDirectiveMode, SlangLineDirectiveMode)
SLANG_GET_VALUE_CATEGORY(FloatingPointMode, FloatingPointMode)
SLANG_GET_VALUE_CATEGORY(FloatingPointDenormalMode, FloatingPointDenormalMode)
SLANG_GET_VALUE_CATEGORY(FileSystemType, TypeTextUtil::FileSystemType)
SLANG_GET_VALUE_CATEGORY(HelpStyle, CommandOptionsWriter::Style)
SLANG_GET_VALUE_CATEGORY(OptimizationLevel, SlangOptimizationLevel)
SLANG_GET_VALUE_CATEGORY(VulkanShift, HLSLToVulkanLayoutOptions::Kind)
SLANG_GET_VALUE_CATEGORY(SourceEmbedStyle, SourceEmbedUtil::Style)
SLANG_GET_VALUE_CATEGORY(Language, SourceLanguage)

} // namespace Slang

#endif // SLANG_OPTIONS_VALUE_CATEGORY_H
