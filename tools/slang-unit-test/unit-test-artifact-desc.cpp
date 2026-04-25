// unit-test-artifact-desc.cpp
//
// Contract under test
// -------------------
// `ArtifactDesc` is a (kind, payload, style, flags) 4-tuple
// classifying any compiler artifact (source files, intermediate
// representations, compiled binaries, debug info, etc.).
// `ArtifactDescUtil` and the kind/payload/style hierarchy
// predicates are the public query surface. Used everywhere the
// compiler decides "what kind of thing is this artifact?", and
// the answers feed into:
//
//   * Backend selection (isCpuLikeTarget / isCpuBinary / isGpuUsable)
//   * Linker behaviour (isLinkable / isKindBinaryLinkable)
//   * Source-vs-binary handling (isText)
//   * Disassembly relationships (isDisassembly)
//   * File-extension inference for I/O (getDescFromExtension /
//     getDescFromPath / appendDefaultExtension)
//   * Round-tripping with the public C-API target enum
//     (makeDescForCompileTarget / getCompileTargetFromDesc)
//
// The hierarchy queries (`isDerivedFrom(child, parent)`) are the
// foundation: a kind / payload / style derives from itself
// (reflexive), and from each ancestor in its taxonomy. Used by
// caller code to write "is this an HLSL-like source?" without
// enumerating every concrete flavour.

#include "../../source/compiler-core/slang-artifact-desc-util.h"
#include "../../source/compiler-core/slang-artifact.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

namespace
{

ArtifactDesc makeDesc(ArtifactKind k, ArtifactPayload p, ArtifactStyle s = ArtifactStyle::Unknown)
{
    ArtifactDesc d;
    d.kind = k;
    d.payload = p;
    d.style = s;
    d.flags = 0;
    return d;
}

} // namespace

SLANG_UNIT_TEST(artifactKindHierarchy)
{
    // Kind hierarchy: every child should be derived from its parent.
    SLANG_CHECK(isDerivedFrom(ArtifactKind::Source, ArtifactKind::Base));
    SLANG_CHECK(isDerivedFrom(ArtifactKind::SharedLibrary, ArtifactKind::Base));
    // Identity: a kind is derived from itself.
    SLANG_CHECK(isDerivedFrom(ArtifactKind::Source, ArtifactKind::Source));
    // Negative: unrelated kinds.
    SLANG_CHECK(!isDerivedFrom(ArtifactKind::Source, ArtifactKind::SharedLibrary));
}

SLANG_UNIT_TEST(artifactPayloadHierarchy)
{
    SLANG_CHECK(isDerivedFrom(ArtifactPayload::HLSL, ArtifactPayload::Source));
    SLANG_CHECK(isDerivedFrom(ArtifactPayload::SPIRV, ArtifactPayload::Base));
    SLANG_CHECK(isDerivedFrom(ArtifactPayload::HLSL, ArtifactPayload::HLSL));
}

SLANG_UNIT_TEST(artifactStyleHierarchy)
{
    SLANG_CHECK(isDerivedFrom(ArtifactStyle::Kernel, ArtifactStyle::Base));
    SLANG_CHECK(isDerivedFrom(ArtifactStyle::Host, ArtifactStyle::Base));
    SLANG_CHECK(isDerivedFrom(ArtifactStyle::Host, ArtifactStyle::Host));
}

// Distinct kinds must produce distinct human-readable names so
// callers (diagnostic / log code) can tell artifacts apart in
// output. A naming collision would confuse error messages.
SLANG_UNIT_TEST(artifactKindNamesAreDistinct)
{
    auto src = getName(ArtifactKind::Source);
    auto so = getName(ArtifactKind::SharedLibrary);
    auto obj = getName(ArtifactKind::ObjectCode);

    SLANG_CHECK(src.getLength() > 0);
    SLANG_CHECK(so.getLength() > 0);
    SLANG_CHECK(obj.getLength() > 0);
    SLANG_CHECK(src != so);
    SLANG_CHECK(src != obj);
    SLANG_CHECK(so != obj);
}

SLANG_UNIT_TEST(artifactPayloadNamesAreDistinct)
{
    auto hlsl = getName(ArtifactPayload::HLSL);
    auto glsl = getName(ArtifactPayload::GLSL);
    auto spirv = getName(ArtifactPayload::SPIRV);
    auto dxil = getName(ArtifactPayload::DXIL);

    SLANG_CHECK(hlsl.getLength() > 0);
    SLANG_CHECK(glsl.getLength() > 0);
    SLANG_CHECK(spirv.getLength() > 0);
    SLANG_CHECK(dxil.getLength() > 0);
    SLANG_CHECK(hlsl != glsl);
    SLANG_CHECK(spirv != dxil);
    SLANG_CHECK(hlsl != spirv);
}

SLANG_UNIT_TEST(artifactDescIsCpuTarget)
{
    auto cpuObj = makeDesc(ArtifactKind::ObjectCode, ArtifactPayload::HostCPU);
    SLANG_CHECK(ArtifactDescUtil::isCpuLikeTarget(cpuObj));
    SLANG_CHECK(ArtifactDescUtil::isCpuBinary(cpuObj));

    auto spirv = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    SLANG_CHECK(!ArtifactDescUtil::isCpuLikeTarget(spirv));
    SLANG_CHECK(!ArtifactDescUtil::isCpuBinary(spirv));
}

SLANG_UNIT_TEST(artifactDescIsGpuUsable)
{
    auto spirv = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    SLANG_CHECK(ArtifactDescUtil::isGpuUsable(spirv));

    auto hlslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    // Source is not "GPU usable" — it needs compilation first.
    SLANG_CHECK(!ArtifactDescUtil::isGpuUsable(hlslSource));
}

SLANG_UNIT_TEST(artifactDescIsText)
{
    auto hlslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    SLANG_CHECK(ArtifactDescUtil::isText(hlslSource));

    auto glslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::GLSL);
    SLANG_CHECK(ArtifactDescUtil::isText(glslSource));

    auto spirvBin = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    SLANG_CHECK(!ArtifactDescUtil::isText(spirvBin));
}

SLANG_UNIT_TEST(artifactDescDerivedFrom)
{
    auto hlslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    auto sourceBase = makeDesc(ArtifactKind::Source, ArtifactPayload::Source);

    SLANG_CHECK(ArtifactDescUtil::isDescDerivedFrom(hlslSource, sourceBase));
    // Self-derivation always holds.
    SLANG_CHECK(ArtifactDescUtil::isDescDerivedFrom(hlslSource, hlslSource));
    // GLSL is not derived from HLSL.
    auto glslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::GLSL);
    SLANG_CHECK(!ArtifactDescUtil::isDescDerivedFrom(glslSource, hlslSource));
}

SLANG_UNIT_TEST(artifactDescGetDescFromExtension)
{
    auto hlsl = ArtifactDescUtil::getDescFromExtension(toSlice("hlsl"));
    SLANG_CHECK(isDerivedFrom(hlsl.payload, ArtifactPayload::HLSL));
    SLANG_CHECK(isDerivedFrom(hlsl.kind, ArtifactKind::Source));

    auto glsl = ArtifactDescUtil::getDescFromExtension(toSlice("glsl"));
    SLANG_CHECK(isDerivedFrom(glsl.payload, ArtifactPayload::GLSL));

    auto spv = ArtifactDescUtil::getDescFromExtension(toSlice("spv"));
    SLANG_CHECK(isDerivedFrom(spv.payload, ArtifactPayload::SPIRV));
}

SLANG_UNIT_TEST(artifactDescGetDescFromPath)
{
    auto hlsl = ArtifactDescUtil::getDescFromPath(toSlice("path/to/foo.hlsl"));
    SLANG_CHECK(isDerivedFrom(hlsl.payload, ArtifactPayload::HLSL));

    auto glsl = ArtifactDescUtil::getDescFromPath(toSlice("foo.glsl"));
    SLANG_CHECK(isDerivedFrom(glsl.payload, ArtifactPayload::GLSL));
}

// getText must mention the payload by name so a human reading
// the diagnostic can identify the artifact. Different descs must
// produce different strings (otherwise the rendering is useless
// for distinguishing artifacts).
SLANG_UNIT_TEST(artifactDescGetTextMentionsPayload)
{
    String hlslText = ArtifactDescUtil::getText(
        makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL));
    String spirvText = ArtifactDescUtil::getText(
        makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV));

    SLANG_CHECK(hlslText.getLength() > 0);
    SLANG_CHECK(spirvText.getLength() > 0);
    SLANG_CHECK(hlslText != spirvText);
}

// `appendDefaultExtension` must produce the canonical extension
// for the given desc — the one that round-trips through
// `getDescFromExtension`. So if we extend an arbitrary base path
// with the default extension, then ask "what desc does that file
// have?", we should recover the (kind, payload) we started with.
SLANG_UNIT_TEST(artifactDescAppendDefaultExtensionRoundTrips)
{
    auto hlslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    StringBuilder hlslBuf;
    SLANG_CHECK(SLANG_SUCCEEDED(
        ArtifactDescUtil::appendDefaultExtension(hlslSource, hlslBuf)));
    SLANG_CHECK(hlslBuf.getLength() > 0);
    // Round-trip: ext → desc.
    auto recovered = ArtifactDescUtil::getDescFromExtension(hlslBuf.getUnownedSlice());
    SLANG_CHECK(isDerivedFrom(recovered.payload, ArtifactPayload::HLSL));

    auto glslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::GLSL);
    StringBuilder glslBuf;
    SLANG_CHECK(SLANG_SUCCEEDED(
        ArtifactDescUtil::appendDefaultExtension(glslSource, glslBuf)));
    auto recoveredGlsl = ArtifactDescUtil::getDescFromExtension(glslBuf.getUnownedSlice());
    SLANG_CHECK(isDerivedFrom(recoveredGlsl.payload, ArtifactPayload::GLSL));
}

SLANG_UNIT_TEST(artifactDescMakeDescForCompileTarget)
{
    auto spv = ArtifactDescUtil::makeDescForCompileTarget(SLANG_SPIRV);
    SLANG_CHECK(isDerivedFrom(spv.payload, ArtifactPayload::SPIRV));

    auto hlsl = ArtifactDescUtil::makeDescForCompileTarget(SLANG_HLSL);
    SLANG_CHECK(isDerivedFrom(hlsl.payload, ArtifactPayload::HLSL));
}

SLANG_UNIT_TEST(artifactDescGetCompileTargetFromDesc)
{
    auto spv = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    SLANG_CHECK(ArtifactDescUtil::getCompileTargetFromDesc(spv) == SLANG_SPIRV);

    auto hlsl = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    SLANG_CHECK(ArtifactDescUtil::getCompileTargetFromDesc(hlsl) == SLANG_HLSL);
}

SLANG_UNIT_TEST(artifactDescIsKindBinaryLinkable)
{
    SLANG_CHECK(ArtifactDescUtil::isKindBinaryLinkable(ArtifactKind::ObjectCode));
    SLANG_CHECK(ArtifactDescUtil::isKindBinaryLinkable(ArtifactKind::Library));
    SLANG_CHECK(!ArtifactDescUtil::isKindBinaryLinkable(ArtifactKind::Source));
}

SLANG_UNIT_TEST(artifactDescIsLinkable)
{
    auto obj = makeDesc(ArtifactKind::ObjectCode, ArtifactPayload::HostCPU);
    SLANG_CHECK(ArtifactDescUtil::isLinkable(obj));

    auto src = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    SLANG_CHECK(!ArtifactDescUtil::isLinkable(src));
}

SLANG_UNIT_TEST(artifactDescIsDisassembly)
{
    // Disassembly is encoded as `kind=Assembly` + same payload as
    // the binary form.
    auto spirvBin = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    auto spirvAsm = makeDesc(ArtifactKind::Assembly, ArtifactPayload::SPIRV);
    SLANG_CHECK(ArtifactDescUtil::isDisassembly(spirvBin, spirvAsm));
    // Reverse direction is not disassembly.
    SLANG_CHECK(!ArtifactDescUtil::isDisassembly(spirvAsm, spirvBin));
}
