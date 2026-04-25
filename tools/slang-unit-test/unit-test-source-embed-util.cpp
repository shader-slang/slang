// unit-test-source-embed-util.cpp
//
// Contract under test
// -------------------
// `SourceEmbedUtil` exposes static helpers used by `slangc -embed-source` to
// turn a compiled artifact into a C/C++ source file containing a static
// array of bytes (or text). These tests pin the publicly-relied-on
// behaviours of the helpers so that callers (slangc, codegen tooling)
// don't quietly break:
//
//   * Only C and C++ are supported as embed-output languages. The util
//     refuses other languages by returning an empty path / E_NOT_IMPLEMENTED.
//   * `getPath` produces a header path: if the input already has a header
//     extension (h/hpp/hxx/h++/hh) it is returned verbatim; otherwise `.h`
//     is appended. Unsupported language → empty result.
//   * `getDefaultStyle` chooses a sensible default for the artifact: text
//     artifacts get `Style::Text`; SPIR-V binaries get `Style::U32` (since
//     SPIR-V is a stream of 32-bit words).
//   * `Options` has documented defaults the rest of the embedder relies on.

#include "../../source/compiler-core/slang-artifact.h"
#include "../../source/compiler-core/slang-source-embed-util.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

ArtifactDesc makeDesc(ArtifactKind k, ArtifactPayload p)
{
    ArtifactDesc d;
    d.kind = k;
    d.payload = p;
    d.style = ArtifactStyle::Unknown;
    d.flags = 0;
    return d;
}

} // namespace

// Style catalog must include all enum cases listed in the header so a
// caller iterating to render help text covers every embed style.
SLANG_UNIT_TEST(sourceEmbedUtilGetStyleInfosCoversEnum)
{
    auto styles = SourceEmbedUtil::getStyleInfos();
    SLANG_CHECK_ABORT(styles.getCount() > 0);
    // Style::CountOf is the count-of sentinel; every value below it
    // must have an entry. Otherwise help/CLI listings drop styles.
    SLANG_CHECK(styles.getCount() == Index(SourceEmbedUtil::Style::CountOf));

    // Every entry must have a non-empty name; an empty name would
    // render as a blank option in the help output.
    for (Index i = 0; i < styles.getCount(); ++i)
    {
        SLANG_CHECK(styles[i].names != nullptr);
        SLANG_CHECK(*styles[i].names != '\0');
    }
}

// Only C and C++ are valid embed-output languages. Anything else
// must be rejected — getPath returns empty, createEmbedded returns
// E_NOT_IMPLEMENTED. Callers rely on this to skip the embed step
// when the user picked a non-C target.
SLANG_UNIT_TEST(sourceEmbedUtilSupportedLanguages)
{
    SLANG_CHECK(SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_C));
    SLANG_CHECK(SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_CPP));
    SLANG_CHECK(!SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_UNKNOWN));
    SLANG_CHECK(!SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_HLSL));
    SLANG_CHECK(!SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_GLSL));
    SLANG_CHECK(!SourceEmbedUtil::isSupported(SLANG_SOURCE_LANGUAGE_SLANG));
}

// getPath: if the input path already has a recognized header
// extension, it's returned verbatim (no double-extension like
// foo.h.h).
SLANG_UNIT_TEST(sourceEmbedUtilGetPathPreservesHeaderExt)
{
    SourceEmbedUtil::Options opts;
    opts.language = SLANG_SOURCE_LANGUAGE_C;

    SLANG_CHECK(SourceEmbedUtil::getPath("foo.h", opts) == "foo.h");
    SLANG_CHECK(SourceEmbedUtil::getPath("foo.hpp", opts) == "foo.hpp");
    SLANG_CHECK(SourceEmbedUtil::getPath("path/to/bar.hxx", opts) == "path/to/bar.hxx");
    SLANG_CHECK(SourceEmbedUtil::getPath("baz.h++", opts) == "baz.h++");
    SLANG_CHECK(SourceEmbedUtil::getPath("qux.hh", opts) == "qux.hh");
}

// getPath: non-header input (.spv, .slang, .cpp) gets a `.h`
// appended. The full input path is preserved as the prefix.
SLANG_UNIT_TEST(sourceEmbedUtilGetPathAppendsDotH)
{
    SourceEmbedUtil::Options opts;
    opts.language = SLANG_SOURCE_LANGUAGE_C;

    SLANG_CHECK(SourceEmbedUtil::getPath("foo.spv", opts) == "foo.spv.h");
    SLANG_CHECK(SourceEmbedUtil::getPath("path/to/shader.slang", opts) == "path/to/shader.slang.h");
    SLANG_CHECK(SourceEmbedUtil::getPath("noext", opts) == "noext.h");
}

// getPath: empty input → empty output (callers use this to detect
// "no path supplied"). Unsupported language → empty regardless of
// input.
SLANG_UNIT_TEST(sourceEmbedUtilGetPathEdgeCases)
{
    SourceEmbedUtil::Options opts;
    opts.language = SLANG_SOURCE_LANGUAGE_C;
    SLANG_CHECK(SourceEmbedUtil::getPath(String(), opts).getLength() == 0);

    opts.language = SLANG_SOURCE_LANGUAGE_HLSL;
    // Even with a perfectly fine input path, an unsupported language
    // produces no output path.
    SLANG_CHECK(SourceEmbedUtil::getPath("foo.spv", opts).getLength() == 0);
}

// Text artifacts (HLSL / GLSL / WGSL source) embed as text by
// default — preserves human readability when the consumer opens
// the generated .h.
SLANG_UNIT_TEST(sourceEmbedUtilDefaultStyleForTextIsText)
{
    SLANG_CHECK(
        SourceEmbedUtil::getDefaultStyle(makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL))
        == SourceEmbedUtil::Style::Text);
    SLANG_CHECK(
        SourceEmbedUtil::getDefaultStyle(makeDesc(ArtifactKind::Source, ArtifactPayload::GLSL))
        == SourceEmbedUtil::Style::Text);
    SLANG_CHECK(
        SourceEmbedUtil::getDefaultStyle(makeDesc(ArtifactKind::Source, ArtifactPayload::WGSL))
        == SourceEmbedUtil::Style::Text);
}

// SPIR-V binary embed defaults to U32 because SPIR-V is a stream
// of 32-bit words; emitting bytes would force consumers to
// re-pack. This is the contract slangc relies on for `-embed-source
// -target spirv`.
SLANG_UNIT_TEST(sourceEmbedUtilDefaultStyleForSpirvIsU32)
{
    auto spirvBin = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    SLANG_CHECK(SourceEmbedUtil::getDefaultStyle(spirvBin) == SourceEmbedUtil::Style::U32);
}

// Options defaults must remain stable: `slangc -embed-source` with
// no other flags relies on these. A change here is observable to
// any caller that constructs a default-initialized Options.
SLANG_UNIT_TEST(sourceEmbedUtilOptionsDefaults)
{
    SourceEmbedUtil::Options opts;
    SLANG_CHECK(opts.style == SourceEmbedUtil::Style::Default);
    SLANG_CHECK(opts.lineLength == 120);
    SLANG_CHECK(opts.language == SLANG_SOURCE_LANGUAGE_C);
    SLANG_CHECK(opts.indent == "    ");      // four spaces, not a tab
    SLANG_CHECK(opts.variableName.getLength() == 0); // caller fills in
}
