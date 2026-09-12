#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(contentVarLayoutReflection)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_HLSL;
    targetDesc.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module =
        session->loadModule("tests/reflection/content-var-layout.slang", diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    auto reflection = module->getLayout();
    SLANG_CHECK(reflection != nullptr);

    auto bufLayout = reflection->getParameterByIndex(0)->getTypeLayout();
    auto rwBufLayout = reflection->getParameterByIndex(1)->getTypeLayout();
    auto appendBufLayout = reflection->getParameterByIndex(2)->getTypeLayout();
    auto cbLayout = reflection->getParameterByIndex(3)->getTypeLayout();
    auto pbLayout = reflection->getParameterByIndex(4)->getTypeLayout();
    auto tbLayout = reflection->getParameterByIndex(5)->getTypeLayout();
    auto texLayout = reflection->getParameterByIndex(6)->getTypeLayout();
    auto existentialBufLayout = reflection->getParameterByIndex(7)->getTypeLayout();
    SLANG_CHECK(bufLayout->getKind() == slang::TypeReflection::Kind::Resource);
    SLANG_CHECK(cbLayout->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    SLANG_CHECK(pbLayout->getKind() == slang::TypeReflection::Kind::ParameterBlock);
    SLANG_CHECK(tbLayout->getKind() == slang::TypeReflection::Kind::TextureBuffer);

    auto bufContent = bufLayout->getContentVarLayout();
    SLANG_CHECK(bufContent != nullptr);
    auto bufContentTypeLayout = bufContent->getTypeLayout();
    SLANG_CHECK(bufContentTypeLayout != nullptr);
    SLANG_CHECK(bufContentTypeLayout->getKind() == slang::TypeReflection::Kind::Array);

    // The content models an unbounded array: its uniform storage is unbounded (a fixed-size array
    // would report a finite size here).
    SLANG_CHECK(
        bufContentTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) == SLANG_UNBOUNDED_SIZE);

    // The content array's element layout is the very layout `getElementTypeLayout()` reports for
    // the buffer (shared, not a copy).
    auto contentElementTypeLayout = bufContentTypeLayout->getElementTypeLayout();
    SLANG_CHECK(contentElementTypeLayout != nullptr);
    SLANG_CHECK(contentElementTypeLayout->getKind() == slang::TypeReflection::Kind::Struct);
    SLANG_CHECK(contentElementTypeLayout == bufLayout->getElementTypeLayout());

    // The content array carries the per-element stride (S packs to 16 bytes) that the bare
    // structured-buffer layout does not report.
    SLANG_CHECK(contentElementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM) == 16);
    SLANG_CHECK(bufContentTypeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM) == 16);
    SLANG_CHECK(bufLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM) == 0);

    // Navigating to the content resets the byte-offset root, so it starts at offset 0.
    SLANG_CHECK(bufContent->getOffset(SLANG_PARAMETER_CATEGORY_UNIFORM) == 0);

    // Cached at construction: repeated calls return the same pointer.
    SLANG_CHECK(bufLayout->getContentVarLayout() == bufContent);

    // The RW and append variants go through the same producer, so they too get array content.
    SLANG_CHECK(rwBufLayout->getContentVarLayout() != nullptr);
    SLANG_CHECK(
        rwBufLayout->getContentVarLayout()->getTypeLayout()->getKind() ==
        slang::TypeReflection::Kind::Array);
    SLANG_CHECK(appendBufLayout->getContentVarLayout() != nullptr);
    SLANG_CHECK(
        appendBufLayout->getContentVarLayout()->getTypeLayout()->getKind() ==
        slang::TypeReflection::Kind::Array);

    SLANG_CHECK(cbLayout->getContentVarLayout() == cbLayout->getElementVarLayout());
    SLANG_CHECK(cbLayout->getContentVarLayout() != nullptr);
    SLANG_CHECK(pbLayout->getContentVarLayout() == pbLayout->getElementVarLayout());
    SLANG_CHECK(pbLayout->getContentVarLayout() != nullptr);
    SLANG_CHECK(tbLayout->getContentVarLayout() == tbLayout->getElementVarLayout());
    SLANG_CHECK(tbLayout->getContentVarLayout() != nullptr);

    SLANG_CHECK(texLayout->getContentVarLayout() == nullptr);

    // A structured buffer of an interface type propagates existential resource usage onto its
    // content array, matching what the buffer itself reports.
    auto existentialContent = existentialBufLayout->getContentVarLayout();
    SLANG_CHECK(existentialContent != nullptr);
    auto existentialContentTypeLayout = existentialContent->getTypeLayout();
    SLANG_CHECK(existentialContentTypeLayout->getKind() == slang::TypeReflection::Kind::Array);
    SLANG_CHECK(
        existentialContentTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM) ==
        existentialBufLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM));
    SLANG_CHECK(
        existentialContentTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM) ==
        existentialBufLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM));

    // `float3` content case on this (HLSL, scalar-packed) target: element uniform size is 12 and,
    // with 4-byte scalar alignment, the array stride is also 12 — a nonzero, alignment-consistent
    // stride from the array-layout rules.
    auto vec3BufLayout = reflection->getParameterByIndex(8)->getTypeLayout();
    auto vec3Content = vec3BufLayout->getContentVarLayout();
    SLANG_CHECK(vec3Content != nullptr);
    auto vec3ContentTypeLayout = vec3Content->getTypeLayout();
    auto vec3ElementTypeLayout = vec3ContentTypeLayout->getElementTypeLayout();
    auto vec3ElementSize = vec3ElementTypeLayout->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
    auto vec3ElementAlign = vec3ElementTypeLayout->getAlignment(SLANG_PARAMETER_CATEGORY_UNIFORM);
    auto vec3ContentStride =
        vec3ContentTypeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
    SLANG_CHECK(vec3ElementSize == 12);
    SLANG_CHECK(vec3ContentStride != 0);
    SLANG_CHECK(vec3ContentStride >= vec3ElementSize);
    SLANG_CHECK(vec3ContentStride % vec3ElementAlign == 0);

    // Discriminating case: reflect the same module for SPIR-V (std430), where a `float3` has
    // 16-byte alignment, so its array stride (16) is strictly greater than its tight uniform size
    // (12). This proves the content stride comes through the array-layout rules rather than copying
    // the element size — a distinction the HLSL scalar-packed target cannot make (there stride ==
    // size).
    slang::TargetDesc spirvTargetDesc = {};
    spirvTargetDesc.format = SLANG_SPIRV;
    spirvTargetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc spirvSessionDesc = {};
    spirvSessionDesc.targetCount = 1;
    spirvSessionDesc.targets = &spirvTargetDesc;
    ComPtr<slang::ISession> spirvSession;
    SLANG_CHECK(
        globalSession->createSession(spirvSessionDesc, spirvSession.writeRef()) == SLANG_OK);
    auto spirvModule = spirvSession->loadModule(
        "tests/reflection/content-var-layout.slang",
        diagnosticBlob.writeRef());
    SLANG_CHECK(spirvModule != nullptr);
    auto spirvReflection = spirvModule->getLayout();
    auto spirvVec3Content =
        spirvReflection->getParameterByIndex(8)->getTypeLayout()->getContentVarLayout();
    SLANG_CHECK(spirvVec3Content != nullptr);
    auto spirvVec3ContentTypeLayout = spirvVec3Content->getTypeLayout();
    auto spirvVec3ElementSize = spirvVec3ContentTypeLayout->getElementTypeLayout()->getSize(
        SLANG_PARAMETER_CATEGORY_UNIFORM);
    auto spirvVec3Stride =
        spirvVec3ContentTypeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
    SLANG_CHECK(spirvVec3ElementSize == 12);
    SLANG_CHECK(spirvVec3Stride == 16);
    SLANG_CHECK(spirvVec3Stride > spirvVec3ElementSize);
}
