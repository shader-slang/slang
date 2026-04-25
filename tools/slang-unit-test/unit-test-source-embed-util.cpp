// unit-test-source-embed-util.cpp
//
// Tests for `SourceEmbedUtil` static helpers — style inference,
// language support predicate, output-path derivation, style infos.
// `createEmbedded` (actual blob → C/C++ array conversion) needs a
// live IArtifact instance and is exercised indirectly via the
// Slang slangc embed feature; left as future work.

#include "../../source/compiler-core/slang-source-embed-util.h"
#include "../../source/compiler-core/slang-artifact.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(sourceEmbedUtilGetStyleInfos)
{
    auto styles = SourceEmbedUtil::getStyleInfos();
    // Definitions in the header: None, Default, Text, BinaryText,
    // U8, U16, U32, U64. Style::CountOf marks the end of the enum.
    SLANG_CHECK(styles.getCount() >= 4);
    // Every entry should have a non-empty name.
    for (Index i = 0; i < styles.getCount(); ++i)
    {
        SLANG_CHECK(styles[i].names != nullptr);
        SLANG_CHECK(*styles[i].names != '\0');
    }
}

SLANG_UNIT_TEST(sourceEmbedUtilIsSupportedLanguage)
{
    // C / C++ are the canonical supported languages for embedded
    // source output.
    SLANG_CHECK(SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_C));
    SLANG_CHECK(SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_CPP));

    // SPIR-V isn't a source language for the embedder.
    SLANG_CHECK(!SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_UNKNOWN));
}

SLANG_UNIT_TEST(sourceEmbedUtilGetPath)
{
    SourceEmbedUtil::Options opts;
    opts.language = SLANG_SOURCE_LANGUAGE_C;

    String out = SourceEmbedUtil::getPath("foo.slang", opts);
    // Output path should differ from the input — embedded form
    // typically appends a .h / .c-style extension.
    SLANG_CHECK(out.getLength() > 0);
    SLANG_CHECK(out != "foo.slang");
}

SLANG_UNIT_TEST(sourceEmbedUtilGetPathEmpty)
{
    SourceEmbedUtil::Options opts;
    String out = SourceEmbedUtil::getPath(String(), opts);
    // No input path → no output path.
    SLANG_CHECK(out.getLength() == 0);
}

SLANG_UNIT_TEST(sourceEmbedUtilGetDefaultStyleForTextSource)
{
    // Text-typed artifact — default style should be a text style
    // (Text or BinaryText).
    ArtifactDesc desc;
    desc.kind = ArtifactKind::Source;
    desc.payload = ArtifactPayload::HLSL;
    desc.style = ArtifactStyle::Unknown;
    desc.flags = 0;
    auto style = SourceEmbedUtil::getDefaultStyle(desc);
    SLANG_CHECK(style != SourceEmbedUtil::Style::None);
}

SLANG_UNIT_TEST(sourceEmbedUtilGetDefaultStyleForBinary)
{
    // Binary artifact (SPIRV executable) — default style should
    // be one of the byte-array styles.
    ArtifactDesc desc;
    desc.kind = ArtifactKind::Executable;
    desc.payload = ArtifactPayload::SPIRV;
    desc.style = ArtifactStyle::Unknown;
    desc.flags = 0;
    auto style = SourceEmbedUtil::getDefaultStyle(desc);
    SLANG_CHECK(style != SourceEmbedUtil::Style::None);
}

SLANG_UNIT_TEST(sourceEmbedUtilDefaultOptions)
{
    // Default Options should be self-consistent.
    SourceEmbedUtil::Options opts;
    SLANG_CHECK(opts.style == SourceEmbedUtil::Style::Default);
    SLANG_CHECK(opts.lineLength == 120);
    SLANG_CHECK(opts.language == SLANG_SOURCE_LANGUAGE_C);
    SLANG_CHECK(opts.indent.getLength() > 0);
}
