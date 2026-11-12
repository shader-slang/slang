// unit-test-reflection-cursor.cpp

#include "slang-com-ptr.h"
#include "slang-reflection-cursor.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

namespace refl = slang::experimental::reflection;

// Assert that `expr` throws NavigationError and leaves `cursor` unchanged (type layout and path
// length preserved) — the documented strong guarantee for an invalid navigation.
#define CHECK_NAV_THROWS(cursor, expr)                                       \
    do                                                                       \
    {                                                                        \
        auto* typeBefore = (cursor).getTypeLayout();                         \
        size_t linksBefore = (cursor).getAccessPath().getLinkCount();        \
        bool threw = false;                                                  \
        try                                                                  \
        {                                                                    \
            expr;                                                            \
        }                                                                    \
        catch (const refl::NavigationError&)                                 \
        {                                                                    \
            threw = true;                                                    \
        }                                                                    \
        SLANG_CHECK(threw);                                                  \
        SLANG_CHECK((cursor).getTypeLayout() == typeBefore);                 \
        SLANG_CHECK((cursor).getAccessPath().getLinkCount() == linksBefore); \
    } while (0)

// Return the first space-carrying resource category `variableLayout` uses, or `None`. Keeps the
// resource assertions target-agnostic (a texture is `DescriptorTableSlot` on some targets,
// `ShaderResource` on others).
static slang::ParameterCategory findResourceCategory(
    slang::VariableLayoutReflection* variableLayout)
{
    unsigned int count = variableLayout->getCategoryCount();
    for (unsigned int i = 0; i < count; ++i)
    {
        slang::ParameterCategory cat = variableLayout->getCategoryByIndex(i);
        switch (cat)
        {
        case slang::ParameterCategory::ConstantBuffer:
        case slang::ParameterCategory::ShaderResource:
        case slang::ParameterCategory::UnorderedAccess:
        case slang::ParameterCategory::SamplerState:
        case slang::ParameterCategory::DescriptorTableSlot:
            return cat;
        default:
            break;
        }
    }
    return slang::ParameterCategory::None;
}

// Cover the header-only slang::experimental::reflection::Cursor / AccessPath cumulative-offset
// utility (shader-slang/slang#12183). Every expected value is derived independently from the raw
// reflection API so the assertions test the accumulation machinery, not just self-consistency.
// Reflection-only; no GPU.
SLANG_UNIT_TEST(reflectionCursor)
{
    const char* userSource = R"(
        struct Inner { float x; float y; }
        struct Outer { Inner a; Inner b; Inner arr[3]; ConstantBuffer<Inner> nested; }
        struct Material { Texture2D albedo; Texture2D normal; }

        // A structured buffer nested behind a uniform prefix: `mixed` gets a nonzero uniform offset
        // (after `prefix`), which a structured-buffer element offset must exclude.
        struct Mixed { float pad; RWStructuredBuffer<Inner> sb; }
        struct Wrap { float4 prefix; Mixed mixed; }

        ConstantBuffer<Outer> gOuter;
        ParameterBlock<Material> gMat;
        GLSLShaderStorageBuffer<Inner, Std430DataLayout> gSsbo;
        RWStructuredBuffer<Inner> gSb;
        ConstantBuffer<Wrap> gWrap;
        RWStructuredBuffer<float> gOut;

        [Shader("compute")]
        [NumThreads(1, 1, 1)]
        void computeMain()
        {
            gOut[0] = gOuter.a.x + gOuter.b.x + gOuter.arr[2].y + gOuter.nested.x +
                      gMat.albedo.Load(int3(0, 0, 0)).x + gMat.normal.Load(int3(0, 0, 0)).x +
                      gSsbo.x + gSb[2].y + gWrap.prefix.x + gWrap.mixed.pad + gWrap.mixed.sb[2].y;
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "test",
        "test.slang",
        userSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findEntryPointByName("computeMain", entryPoint.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK(programLayout != nullptr);

    auto globalsVar = programLayout->getGlobalParamsVarLayout();
    auto globalsType = globalsVar->getTypeLayout();
    SLANG_CHECK(globalsType != nullptr);
    SLANG_CHECK(globalsType->getKind() == slang::TypeReflection::Kind::Struct);

    SlangInt gOuterFieldIndex = globalsType->findFieldIndexByName("gOuter");
    SLANG_CHECK(gOuterFieldIndex >= 0);
    auto gOuterVar = globalsType->getFieldByIndex((unsigned int)gOuterFieldIndex);
    auto gOuterCbType = gOuterVar->getTypeLayout();
    SLANG_CHECK(gOuterCbType->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
    auto outerContentVar = gOuterCbType->getElementVarLayout();
    auto outerType = outerContentVar->getTypeLayout();
    SLANG_CHECK(outerType->getKind() == slang::TypeReflection::Kind::Struct);

    const size_t contentBase = outerContentVar->getOffset(slang::ParameterCategory::Uniform);

    auto aVar = outerType->getFieldByIndex(0);
    auto bVar = outerType->getFieldByIndex(1);
    auto arrVar = outerType->getFieldByIndex(2);
    auto nestedVar = outerType->getFieldByIndex(3);
    auto innerType = bVar->getTypeLayout();
    auto xVar = innerType->getFieldByIndex(0);

    const size_t expectedAX = contentBase + aVar->getOffset(slang::ParameterCategory::Uniform) +
                              xVar->getOffset(slang::ParameterCategory::Uniform);
    const size_t expectedBX = contentBase + bVar->getOffset(slang::ParameterCategory::Uniform) +
                              xVar->getOffset(slang::ParameterCategory::Uniform);

    {
        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("a");
        cursor.navigateToFieldByName("x");
        SLANG_CHECK(
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset == expectedAX);
    }

    // gOuter.b.x must differ from gOuter.a.x: the reused `Inner` at two positions is exactly the
    // case an ambiguous (target, ancestor) pair cannot express and an explicit path can.
    {
        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("b");
        cursor.navigateToFieldByName("x");
        SLANG_CHECK(
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset == expectedBX);
    }
    SLANG_CHECK(expectedAX != expectedBX);

    // Array element adds `index * elementStride`.
    {
        auto arrType = arrVar->getTypeLayout();
        SLANG_CHECK(arrType->getKind() == slang::TypeReflection::Kind::Array);
        const size_t stride = arrType->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
        auto arrYVar = arrType->getElementTypeLayout()->getFieldByIndex(1);
        const size_t expectedArr =
            contentBase + arrVar->getOffset(slang::ParameterCategory::Uniform) + 2 * stride +
            arrYVar->getOffset(slang::ParameterCategory::Uniform);

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("arr");
        cursor.navigateToElement(2);
        cursor.navigateToFieldByName("y");
        SLANG_CHECK(
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset == expectedArr);
    }

    // gOuter.nested.x: entering the inner constant buffer resets the byte-offset root, so x's
    // cumulative uniform offset is relative to `nested`'s own buffer, not to `gOuter`.
    {
        SLANG_CHECK(
            nestedVar->getTypeLayout()->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
        auto nestedContentVar = nestedVar->getTypeLayout()->getElementVarLayout();
        const size_t expectedNestedX =
            nestedContentVar->getOffset(slang::ParameterCategory::Uniform) +
            nestedContentVar->getTypeLayout()->getFieldByIndex(0)->getOffset(
                slang::ParameterCategory::Uniform);

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("nested");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("x");
        SLANG_CHECK(
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset ==
            expectedNestedX);
    }

    // Resource binding + space through a ParameterBlock. Because the block owns its register space,
    // a resource inside it reports that space; if the parameter-block boundary were not marked, the
    // SubElementRegisterSpace accumulation would not run and the space would be wrong.
    {
        SlangInt gMatFieldIndex = globalsType->findFieldIndexByName("gMat");
        SLANG_CHECK(gMatFieldIndex >= 0);
        auto gMatVar = globalsType->getFieldByIndex((unsigned int)gMatFieldIndex);
        auto gMatPbType = gMatVar->getTypeLayout();
        SLANG_CHECK(gMatPbType->getKind() == slang::TypeReflection::Kind::ParameterBlock);
        auto matContentVar = gMatPbType->getElementVarLayout();
        auto normalVar = matContentVar->getTypeLayout()->getFieldByIndex(1);

        const slang::ParameterCategory resCat = findResourceCategory(normalVar);
        SLANG_CHECK(resCat != slang::ParameterCategory::None);

        const size_t blockSpace =
            gMatVar->getOffset(slang::ParameterCategory::SubElementRegisterSpace) +
            globalsVar->getOffset(slang::ParameterCategory::SubElementRegisterSpace);
        SLANG_CHECK(blockSpace != 0);

        const size_t expectedResOffset =
            matContentVar->getOffset(resCat) + normalVar->getOffset(resCat);
        const size_t expectedResSpace = matContentVar->getBindingSpace(resCat) +
                                        normalVar->getBindingSpace(resCat) + blockSpace;

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gMat");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("normal");
        auto co = cursor.calcCumulativeOffset(resCat);
        SLANG_CHECK(co.offset == expectedResOffset);
        SLANG_CHECK(co.space == expectedResSpace);
    }

    // Navigating to an entry point re-roots the cursor at that entry point's parameter scope,
    // discarding any prior path. Compare against the exact scope type layout from the raw API.
    {
        SLANG_CHECK(programLayout->getEntryPointCount() >= 1);
        auto entryScopeType =
            programLayout->getEntryPointByIndex(0)->getVarLayout()->getTypeLayout();

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter"); // move off the root first
        cursor.navigateToEntryPointByName("computeMain");
        SLANG_CHECK(cursor.getTypeLayout() == entryScopeType);
        SLANG_CHECK(cursor.getAccessPath().getLinkCount() == 1); // path was reset to just the scope

        refl::Cursor byIndex(programLayout);
        byIndex.navigateToEntryPointByIndex(0);
        SLANG_CHECK(byIndex.getTypeLayout() == entryScopeType);

        // Invalid entry-point navigations throw and leave the cursor unchanged.
        CHECK_NAV_THROWS(byIndex, byIndex.navigateToEntryPointByName("no_such_entry_point"));
        CHECK_NAV_THROWS(byIndex, byIndex.navigateToEntryPointByName(nullptr));
        CHECK_NAV_THROWS(
            byIndex,
            byIndex.navigateToEntryPointByIndex(programLayout->getEntryPointCount()));
    }

    // Default (non-special) category branch: for a unit not used anywhere on the path, the result
    // is the plain sum over every link with no boundary handling. `gOuter.b.x` uses no varying
    // input, so its cumulative VaryingInput offset is zero — and querying it exercises the default
    // branch of calcCumulativeOffset (neither Uniform nor a space-carrying resource unit).
    {
        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("b");
        cursor.navigateToFieldByName("x");
        auto co = cursor.calcCumulativeOffset(slang::ParameterCategory::VaryingInput);
        SLANG_CHECK(co.offset == 0);
        SLANG_CHECK(co.space == 0);
    }

    // Default branch with a non-zero contribution: SubElementRegisterSpace is neither Uniform nor a
    // space-carrying resource unit, so it takes the sum-all branch. On the gMat parameter-block
    // path the block contributes a non-zero space, so this would catch a default-branch that
    // dropped or mis-bounded a link. Ground truth is the plain sum of each link's own offset in
    // that unit.
    {
        SlangInt gMatFieldIndex = globalsType->findFieldIndexByName("gMat");
        auto gMatVar = globalsType->getFieldByIndex((unsigned int)gMatFieldIndex);
        auto matContentVar = gMatVar->getTypeLayout()->getElementVarLayout();
        const auto sers = slang::ParameterCategory::SubElementRegisterSpace;
        const size_t expectedSers =
            globalsVar->getOffset(sers) + gMatVar->getOffset(sers) + matContentVar->getOffset(sers);
        SLANG_CHECK(expectedSers != 0);

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gMat");
        cursor.navigateToContent();
        SLANG_CHECK(cursor.calcCumulativeOffset(sers).offset == expectedSers);
    }

    // Invalid navigations throw NavigationError and leave the cursor unchanged.
    {
        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gOuter");
        cursor.navigateToContent();
        CHECK_NAV_THROWS(cursor, cursor.navigateToFieldByName("does_not_exist"));
        CHECK_NAV_THROWS(cursor, cursor.navigateToFieldByName(nullptr));
        CHECK_NAV_THROWS(cursor, cursor.navigateToElement(0)); // current is a struct, not an array
        SLANG_CHECK(cursor.getTypeLayout() == outerType);

        cursor.navigateToFieldByName("arr");
        CHECK_NAV_THROWS(cursor, cursor.navigateToElement(3)); // arr has indices 0..2
        cursor.navigateToElement(2);
    }

    // navigateToContent on a shader storage buffer throws: an SSBO is not a uniform parameter
    // group, so navigateToContent declines it at its kind switch. (The null element-var-layout is
    // asserted separately as a fact about SSBO reflection, not as the reason for the exclusion.)
    {
        SlangInt gSsboFieldIndex = globalsType->findFieldIndexByName("gSsbo");
        SLANG_CHECK(gSsboFieldIndex >= 0);
        auto gSsboVar = globalsType->getFieldByIndex((unsigned int)gSsboFieldIndex);
        SLANG_CHECK(
            gSsboVar->getTypeLayout()->getKind() ==
            slang::TypeReflection::Kind::ShaderStorageBuffer);
        SLANG_CHECK(gSsboVar->getTypeLayout()->getElementVarLayout() == nullptr);

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gSsbo");
        CHECK_NAV_THROWS(cursor, cursor.navigateToContent());
    }

    // navigateToStructuredBufferElement steps into a structured buffer's element by index. gSb is a
    // top-level RWStructuredBuffer<Inner> with element type Inner{float x; float y;}; element [2].y
    // is at 2*stride(Inner) + offset(y), relative to the buffer's data origin. (The nested-prefix
    // case below is the one that discriminates the offset-root reset; here the buffer is top-level,
    // so its own uniform offset is zero regardless.) Direct field/element navigation on the bare
    // buffer is disallowed (like a constant buffer), so navigating the element must go through this
    // op.
    {
        auto gSbVar =
            globalsType->getFieldByIndex((unsigned int)globalsType->findFieldIndexByName("gSb"));
        auto sbType = gSbVar->getTypeLayout();
        SLANG_CHECK(sbType->getKind() == slang::TypeReflection::Kind::Resource);
        SLANG_CHECK(
            (sbType->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) ==
            SLANG_STRUCTURED_BUFFER);
        auto sbElementType = sbType->getElementTypeLayout();
        SLANG_CHECK(sbElementType != nullptr);
        const size_t stride = sbElementType->getStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
        auto sbYVar = sbElementType->getFieldByIndex(1);
        const size_t expectedSbY =
            2 * stride + sbYVar->getOffset(slang::ParameterCategory::Uniform);
        SLANG_CHECK(stride != 0);

        refl::Cursor bare(programLayout);
        bare.navigateToFieldByName("gSb");
        CHECK_NAV_THROWS(bare, bare.navigateToFieldByName("x")); // no field nav on a bare buffer
        CHECK_NAV_THROWS(bare, bare.navigateToElement(0));       // nor plain array-element nav

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gSb");
        cursor.navigateToStructuredBufferElement(2);
        cursor.navigateToFieldByName("y");
        SLANG_CHECK(
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset == expectedSbY);

        // The op requires an HLSL structured buffer. A constant buffer (gOuter) is not one, and
        // neither is a GLSL shader-storage buffer (gSsbo) — reflection exposes no element type
        // layout for the latter, so it is out of scope for v1 (shader-slang/slang#12776).
        refl::Cursor notSb(programLayout);
        notSb.navigateToFieldByName("gOuter");
        CHECK_NAV_THROWS(notSb, notSb.navigateToStructuredBufferElement(0));

        refl::Cursor ssbo(programLayout);
        ssbo.navigateToFieldByName("gSsbo");
        CHECK_NAV_THROWS(ssbo, ssbo.navigateToStructuredBufferElement(0));
    }

    // Structured-buffer element navigation resets the byte-offset root. gWrap is
    // ConstantBuffer<Wrap{ float4 prefix; Mixed{ float pad; RWStructuredBuffer<Inner> sb; } mixed;
    // }>, so `mixed` (and thus its `sb` field) sits at a nonzero uniform offset after `prefix`.
    // Entering sb's element must exclude that enclosing offset: the expected value is 2*stride +
    // offset(y) within the element only, with none of `mixed`'s uniform offset. This is the case
    // that discriminates the boundary marking — dropping it would incorrectly add mixed.sb's
    // uniform offset. As a witness that the enclosing offset is genuinely nonzero, we also check
    // the sibling scalar gWrap.mixed.pad, whose cumulative uniform offset is that same nonzero
    // value.
    {
        auto gWrapVar =
            globalsType->getFieldByIndex((unsigned int)globalsType->findFieldIndexByName("gWrap"));
        SLANG_CHECK(
            gWrapVar->getTypeLayout()->getKind() == slang::TypeReflection::Kind::ConstantBuffer);
        auto wrapContentVar = gWrapVar->getTypeLayout()->getElementVarLayout();
        auto wrapType = wrapContentVar->getTypeLayout();
        const size_t wrapBase = wrapContentVar->getOffset(slang::ParameterCategory::Uniform);
        auto mixedVar = wrapType->getFieldByIndex(1); // Wrap::mixed, after float4 prefix
        auto mixedType = mixedVar->getTypeLayout();
        auto padVar = mixedType->getFieldByIndex(0);     // Mixed::pad
        auto mixedSbVar = mixedType->getFieldByIndex(1); // Mixed::sb
        auto sbElem = mixedSbVar->getTypeLayout()->getElementTypeLayout();
        SLANG_CHECK(sbElem != nullptr);
        const size_t stride = sbElem->getStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
        auto yVar = sbElem->getFieldByIndex(1);

        // The enclosing uniform offset that a normal field would carry (proven nonzero via the
        // sibling `pad` below). A structured-buffer element must NOT carry it.
        const size_t enclosingOffset =
            wrapBase + mixedVar->getOffset(slang::ParameterCategory::Uniform);
        SLANG_CHECK(enclosingOffset != 0);

        // Ground truth for the sibling scalar: it DOES carry the enclosing offset.
        const size_t expectedPad =
            enclosingOffset + padVar->getOffset(slang::ParameterCategory::Uniform);

        // Ground truth for the buffer element: element-relative only, NO enclosing offset.
        const size_t expectedSbY = 2 * stride + yVar->getOffset(slang::ParameterCategory::Uniform);

        refl::Cursor pad(programLayout);
        pad.navigateToFieldByName("gWrap");
        pad.navigateToContent();
        pad.navigateToFieldByName("mixed");
        pad.navigateToFieldByName("pad");
        SLANG_CHECK(
            pad.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset == expectedPad);

        refl::Cursor cursor(programLayout);
        cursor.navigateToFieldByName("gWrap");
        cursor.navigateToContent();
        cursor.navigateToFieldByName("mixed");
        cursor.navigateToFieldByName("sb");
        cursor.navigateToStructuredBufferElement(2);
        cursor.navigateToFieldByName("y");
        const size_t actualSbY =
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset;
        SLANG_CHECK(actualSbY == expectedSbY);
        // Without the boundary the result would be `expectedSbY + enclosingOffset` (a resource
        // field contributes no uniform offset of its own). Since enclosingOffset != 0, this
        // inequality is exactly what the boundary buys — it fails if the reset is dropped.
        SLANG_CHECK(actualSbY != expectedSbY + enclosingOffset);
    }

    // navigateToEntryPoint requires a program-rooted cursor; a type-rooted one throws.
    {
        refl::Cursor typeRooted(outerType);
        CHECK_NAV_THROWS(typeRooted, typeRooted.navigateToEntryPointByIndex(0));
        CHECK_NAV_THROWS(typeRooted, typeRooted.navigateToEntryPointByName("computeMain"));
    }

    // A bare-type-layout root: cumulative offset is relative to that type's origin. Navigating b.y
    // over `Outer` directly yields b's offset within Outer plus y's within Inner.
    {
        auto yVar = innerType->getFieldByIndex(1);
        const size_t expectedBY = bVar->getOffset(slang::ParameterCategory::Uniform) +
                                  yVar->getOffset(slang::ParameterCategory::Uniform);

        refl::Cursor cursor(outerType);
        cursor.navigateToFieldByName("b");
        cursor.navigateToFieldByName("y");
        SLANG_CHECK(
            cursor.calcCumulativeOffset(slang::ParameterCategory::Uniform).offset == expectedBY);
    }
}
