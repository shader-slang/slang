// unit-test-artifact-desc.cpp
//
// Tests for `ArtifactDescUtil` — predicates / queries on
// `ArtifactDesc` (kind/payload/style triples). Pure-value tests, no
// IArtifact instances needed.

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

SLANG_UNIT_TEST(artifactKindNames)
{
    // Kind names are non-empty for canonical kinds.
    SLANG_CHECK(getName(ArtifactKind::Source).getLength() > 0);
    SLANG_CHECK(getName(ArtifactKind::SharedLibrary).getLength() > 0);
    SLANG_CHECK(getName(ArtifactKind::ObjectCode).getLength() > 0);
}

SLANG_UNIT_TEST(artifactPayloadNames)
{
    SLANG_CHECK(getName(ArtifactPayload::HLSL).getLength() > 0);
    SLANG_CHECK(getName(ArtifactPayload::GLSL).getLength() > 0);
    SLANG_CHECK(getName(ArtifactPayload::SPIRV).getLength() > 0);
    SLANG_CHECK(getName(ArtifactPayload::DXIL).getLength() > 0);
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

SLANG_UNIT_TEST(artifactDescGetText)
{
    auto hlslSource = makeDesc(ArtifactKind::Source, ArtifactPayload::HLSL);
    String text = ArtifactDescUtil::getText(hlslSource);
    // The text representation should be non-empty and mention the
    // payload (or kind) name in some form.
    SLANG_CHECK(text.getLength() > 0);
}

SLANG_UNIT_TEST(artifactDescAppendDefaultExtension)
{
    StringBuilder buf;
    auto spirv = makeDesc(ArtifactKind::Executable, ArtifactPayload::SPIRV);
    SlangResult r = ArtifactDescUtil::appendDefaultExtension(spirv, buf);
    SLANG_CHECK(SLANG_SUCCEEDED(r));
    SLANG_CHECK(buf.getLength() > 0);
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
