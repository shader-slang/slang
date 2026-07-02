// unit-test-translation-unit-import.cpp

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

static String getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return String((const char*)blob->getBufferPointer());
}

// Test that the reflection API provides correct info about modules with link-time types.

SLANG_UNIT_TEST(linkTimeTypeReflection)
{
    // Source for a module that contains can be specialized with a link-time type.
    const char* userSourceBody = R"(
        interface IMaterial { float4 load(); }
        extern struct Material : IMaterial;
        ConstantBuffer<Material> gMaterial;
        
        interface IFoo { float getVal(); }
        struct DefaultFoo : IFoo { float getVal() { return 0.0f; } }
        extern struct Foo<T, int x> : IFoo = DefaultFoo;
        
        RWTexture2D tex;

        extern static const int count;
        uniform uint4 buffers[count];
        uniform Foo<int4, 1> gFoo;

        [numthreads(1,1,1)]
        [shader("compute")]
        void computeMain() {
            tex[uint2(0, 0)] = gMaterial.load() + gFoo.getVal();
        }
        )";

    String moduleName = "linkTimeTypeReflection_Compute";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // Source for a module that defines the link-time type.
    String configModuleSource = "import " + moduleName + ";\n" + R"(
        export struct Material : IMaterial = MyMaterial;
        export static const int count = 11;
        struct FooImpl<T, int x> : IFoo { T vals[x]; float getVal() { return x; } }
        export struct Foo<T, int x> : IFoo = FooImpl<T, x + 1>;
        struct MyMaterial : IMaterial {
           int data;
           Texture2D diffuse;
           float4 load() { return diffuse.Load(uint3(0,0,0)); }
        }
    )";
    auto configModule = session->loadModuleFromSourceString(
        "config",
        "config.slang",
        configModuleSource.getBuffer(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(configModule != nullptr);

    slang::IComponentType* components[] = {module, configModule};

    ComPtr<slang::IComponentType> compositeProgram;
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    auto var0 = programLayout->getParameterByIndex(0);

    // `gMaterial`'s binding starts at 1, because there is an implicit global uniform buffer.
    SLANG_CHECK(var0->getOffset(slang::ParameterCategory::DescriptorTableSlot) == 1);
    SLANG_CHECK(var0->getTypeLayout()->getSize(slang::ParameterCategory::DescriptorTableSlot) == 2);

    auto elementLayout = var0->getTypeLayout()->getElementTypeLayout();
    SLANG_CHECK_ABORT(elementLayout != nullptr);
    SLANG_CHECK(elementLayout->getSize() == 16);

    auto var1 = programLayout->getParameterByIndex(1);
    SLANG_CHECK(var1->getOffset(slang::ParameterCategory::DescriptorTableSlot) == 3);

    auto var2 = programLayout->getParameterByIndex(2);
    SLANG_CHECK(var2->getTypeLayout()->getSize() == 11 * 16);

    auto var3 = programLayout->getParameterByIndex(3);
    SLANG_CHECK(var3->getTypeLayout()->getSize() == 32);

    ComPtr<slang::IBlob> codeBlob;
    SlangResult codeRes =
        linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnosticBlob.writeRef());
    if (SLANG_FAILED(codeRes) || !codeBlob)
    {
        fprintf(
            stderr,
            "[DEBUG aarch64] linkTimeTypeReflection: getTargetCode failed: result=0x%08x\n",
            (unsigned)codeRes);
        if (diagnosticBlob)
            fprintf(
                stderr,
                "[DEBUG aarch64] diagnostics:\n%.*s\n",
                (int)diagnosticBlob->getBufferSize(),
                (const char*)diagnosticBlob->getBufferPointer());
        else
            fprintf(stderr, "[DEBUG aarch64] no diagnostic blob\n");
    }

    SLANG_CHECK_ABORT(codeBlob.get());

    auto spirvStr = UnownedStringSlice((const char*)codeBlob->getBufferPointer());

    SLANG_CHECK(spirvStr.indexOf(toSlice("OpDecorate %tex Binding 3")) != -1);
}


// Test that `getDefaultValueInt` can resolve static const values under a specialized generic type.

SLANG_UNIT_TEST(linkTimeStaticConstIntReflection)
{
    const char* userSourceBody = R"(
        module LinkTimeStaticConstInt;

        public struct StaticConstCarrier<int N>
        {
            public static const int Value = N + 1;
        }

        public struct NestedCarrier<int N>
        {
            public static const int Value = StaticConstCarrier<N>.Value + 3;
        }

        public struct TypeCarrier<T, int N>
        {
            public static const int Value = StaticConstCarrier<N>.Value + sizeof(T);
        }

        public struct LiteralCarrier
        {
            public static const int Value = 23;
        }
        )";

    String moduleName = "LinkTimeStaticConstInt";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    module->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK_ABORT(programLayout != nullptr);

    auto getStaticInt = [&](const char* typeName, const char* varName) -> int64_t
    {
        auto type = programLayout->findTypeByName(typeName);
        SLANG_CHECK_ABORT(type != nullptr);

        auto valueVar = programLayout->findVarByNameInType(type, varName);
        SLANG_CHECK_ABORT(valueVar != nullptr);

        int64_t value = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(valueVar->getDefaultValueInt(&value)));
        return value;
    };

    SLANG_CHECK(getStaticInt("StaticConstCarrier<5>", "Value") == 6);
    SLANG_CHECK(getStaticInt("NestedCarrier<5>", "Value") == 9);
    SLANG_CHECK(getStaticInt("TypeCarrier<float,5>", "Value") == 10);
    SLANG_CHECK(getStaticInt("LiteralCarrier", "Value") == 23);
}


// Test that the reflection API provides correct info about modules using link-time constants in a
// `Conditional` field.

SLANG_UNIT_TEST(linkTimeConditionalReflection)
{
    // Source for a module that contains can be specialized with a link-time constant.
    const char* userSourceBody = R"(
        module LinkTimeConditional;

        extern static const bool hasNormal;
        extern static const bool hasColor;

        struct VertexOut
        {
            float4 pos : SV_Position;
            float someData;
            Conditional<float3, hasNormal> normal;
            Conditional<float3, hasColor> color;
        }

        [shader("vertex")]
        VertexOut vertexMain()
        {
            VertexOut v;
            v.pos = float4(0,0,0,1);
            v.someData = 2.0f;
            v.normal.set(float3(1,0,0));
            v.color.set(float3(1,1,1));
            return v;
        }
        )";

    String moduleName = "LinkTimeConditional";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // Source for a module that defines the link-time constants.
    String configModuleSource = R"(
        export static const bool hasNormal = false;
        export static const bool hasColor = true;
    )";
    auto configModule = session->loadModuleFromSourceString(
        "config",
        "config.slang",
        configModuleSource.getBuffer(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(configModule != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->getDefinedEntryPoint(0, entryPoint.writeRef());

    slang::IComponentType* components[] = {module, configModule, entryPoint};

    ComPtr<slang::IComponentType> compositeProgram;
    session->createCompositeComponentType(
        components,
        3,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();

    auto entryPointLayout = programLayout->getEntryPointByIndex(0);

    auto resultLayout = entryPointLayout->getResultVarLayout();
    SLANG_CHECK_ABORT(resultLayout != nullptr);

    // Number of varying output is 2, because `pos` is a system value that doesn't count towards
    // varying output.
    SLANG_CHECK(
        resultLayout->getTypeLayout()->getSize(slang::ParameterCategory::VaryingOutput) == 2);

    ComPtr<slang::IBlob> codeBlob;
    linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnosticBlob.writeRef());

    SLANG_CHECK_ABORT(codeBlob.get());

    auto spirvStr = UnownedStringSlice((const char*)codeBlob->getBufferPointer());

    // Test that the resulting spirv defines output at location 1, but not at location 2.
    SLANG_CHECK(spirvStr.indexOf(toSlice("Location 1")) != -1);
    SLANG_CHECK(spirvStr.indexOf(toSlice("Location 2")) == -1);
}

// Test for issue #10749: Link-time type specialization of a struct member results in segfault.
// When an extern struct is used as a direct member of another struct, computing the type layout
// should not crash, and reflecting the type via findTypeByName + getTypeLayout should resolve the
// extern member to its concrete link-time definition.

SLANG_UNIT_TEST(linkTimeTypeReflectionStructMember)
{
    const char* userSourceBody = R"(
        interface IAccelerationStructure { int getType(); }
        extern struct AccelerationStructure : IAccelerationStructure;

        struct Scene {
            AccelerationStructure accelStruct;
        }

        ParameterBlock<Scene> gScene;

        [numthreads(1,1,1)]
        [shader("compute")]
        void computeMain() {
            int x = gScene.accelStruct.getType();
        }
    )";

    String moduleName = "linkTimeStructMember_Compute";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    String configModuleSource = "import " + moduleName + ";\n" + R"(
        struct HWAccelerationStructure : IAccelerationStructure {
            uint bufferHandle;
            int getType() { return 1; }
        }
        export struct AccelerationStructure : IAccelerationStructure = HWAccelerationStructure;
    )";
    auto configModule = session->loadModuleFromSourceString(
        "config",
        "config.slang",
        configModuleSource.getBuffer(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(configModule != nullptr);

    slang::IComponentType* components[] = {module, configModule};

    ComPtr<slang::IComponentType> compositeProgram;
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    // Computing the layout for `Scene` (which has an extern struct member) used to
    // segfault in lookupExternDeclRefType; now it succeeds.
    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK(programLayout != nullptr);

    auto var0 = programLayout->getParameterByIndex(0);
    SLANG_CHECK(var0 != nullptr);

    // The Scene struct should have a valid layout with the resolved AccelerationStructure type.
    auto typeLayout = var0->getTypeLayout();
    SLANG_CHECK(typeLayout != nullptr);

    // Now exercise the findTypeByName + getTypeLayout reflection path. Because
    // spReflection_GetTypeLayout threads the ProgramLayout through to
    // TargetRequest::getTypeLayout, the extern `AccelerationStructure` member must
    // resolve to its concrete link-time definition (HWAccelerationStructure), not
    // the bare unresolved extern declaration. We assert the resolved shape so a
    // regression that silently produces an empty (zero-field) layout is caught,
    // rather than only checking for non-null.
    auto sceneType = programLayout->findTypeByName("Scene");
    SLANG_CHECK(sceneType != nullptr);
    if (sceneType)
    {
        auto sceneLayout = programLayout->getTypeLayout(sceneType);
        SLANG_CHECK(sceneLayout != nullptr);
        SLANG_CHECK(sceneLayout->getFieldCount() == 1);
        if (sceneLayout->getFieldCount() == 1)
        {
            // Scene.accelStruct resolves to HWAccelerationStructure, which has one
            // field (uint bufferHandle).
            auto accelStructFieldLayout = sceneLayout->getFieldByIndex(0);
            SLANG_CHECK(accelStructFieldLayout != nullptr);
            auto accelStructTypeLayout = accelStructFieldLayout->getTypeLayout();
            SLANG_CHECK(accelStructTypeLayout != nullptr);
            SLANG_CHECK(accelStructTypeLayout->getFieldCount() == 1);
        }
    }
}

// Test for issue #10749 (variant with associated types): More closely matches the user's actual
// code pattern where the extern struct implements an interface with an associated type.

SLANG_UNIT_TEST(linkTimeTypeReflectionStructMemberAssocType)
{
    const char* userSourceBody = R"(
        interface IRayQuery { int status(); }
        interface IAccelerationStructure {
            associatedtype RayQueryImpl : IRayQuery;
            RayQueryImpl trace();
        }
        extern struct SceneAS : IAccelerationStructure;

        struct Scene {
            SceneAS as;
        }

        ParameterBlock<Scene> gScene;

        [numthreads(1,1,1)]
        [shader("compute")]
        void computeMain() {
            let rq = gScene.as.trace();
        }
    )";

    String moduleName = "linkTimeStructMemberAssoc_Compute";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    String configModuleSource = "import " + moduleName + ";\n" + R"(
        struct HWRayQuery : IRayQuery { int status() { return 1; } }
        struct HWAccelerationStructure : IAccelerationStructure {
            RaytracingAccelerationStructure rtAS;
            typealias RayQueryImpl = HWRayQuery;
            RayQueryImpl trace() { HWRayQuery rq; return rq; }
        }
        export struct SceneAS : IAccelerationStructure = HWAccelerationStructure;
    )";
    auto configModule = session->loadModuleFromSourceString(
        "config",
        "config.slang",
        configModuleSource.getBuffer(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(configModule != nullptr);

    slang::IComponentType* components[] = {module, configModule};

    ComPtr<slang::IComponentType> compositeProgram;
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    SLANG_CHECK(programLayout != nullptr);

    // Exercise getTypeLayout() on the Scene type directly via findTypeByName +
    // getTypeLayout. This is the user's actual scenario: the extern struct `SceneAS`
    // implements an interface with an associated type and resolves to
    // HWAccelerationStructure. spReflection_GetTypeLayout threads the ProgramLayout
    // through, so the extern member resolves to its concrete linked type; we assert
    // the resolved field count so a silently-empty layout regression is caught.
    if (programLayout)
    {
        auto sceneType = programLayout->findTypeByName("Scene");
        SLANG_CHECK(sceneType != nullptr);
        if (sceneType)
        {
            auto sceneLayout = programLayout->getTypeLayout(sceneType);
            SLANG_CHECK(sceneLayout != nullptr);
            SLANG_CHECK(sceneLayout->getFieldCount() == 1);
            if (sceneLayout->getFieldCount() == 1)
            {
                // Scene.as resolves to HWAccelerationStructure, which has one field
                // (RaytracingAccelerationStructure rtAS).
                auto asFieldLayout = sceneLayout->getFieldByIndex(0);
                SLANG_CHECK(asFieldLayout != nullptr);
                auto asTypeLayout = asFieldLayout->getTypeLayout();
                SLANG_CHECK(asTypeLayout != nullptr);
                SLANG_CHECK(asTypeLayout->getFieldCount() == 1);
            }
        }
    }
}

// Test for issue #10749 (program-less reflection path): `ISession::getTypeLayout`
// (i.e. `Linkage::getTypeLayout`) computes a layout without a linked `ProgramLayout`,
// so it reaches the `programLayout == nullptr` branch of `buildExternTypeMap`. This is
// the path whose unguarded dereference originally segfaulted; this test exercises that
// guard directly (the other two tests go through the program-ful
// `spReflection_GetTypeLayout` path, which now threads a non-null `ProgramLayout`).
//
// A module is loaded but never linked with a definition for its `extern` member, so
// there is no link-time type to resolve to. The intended, in-contract behavior for this
// path is: do not crash, and leave the `extern` member unresolved rather than
// fabricate a definition. We assert exactly that so a regression in either direction
// (a re-introduced crash, or a change in the unresolved-layout contract) is caught.
SLANG_UNIT_TEST(linkTimeTypeReflectionStructMemberSessionGetTypeLayout)
{
    const char* userSourceBody = R"(
        interface IAccelerationStructure { int getType(); }
        extern struct AccelerationStructure : IAccelerationStructure;

        struct Scene {
            AccelerationStructure accelStruct;
        }
    )";

    String moduleName = "linkTimeStructMember_SessionLayout";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // Module-level reflection: no link step, so there is no resolved program.
    auto moduleReflection = module->getLayout();
    SLANG_CHECK_ABORT(moduleReflection != nullptr);

    auto sceneType = moduleReflection->findTypeByName("Scene");
    SLANG_CHECK_ABORT(sceneType != nullptr);

    // Go through ISession::getTypeLayout (Linkage::getTypeLayout), which passes
    // programLayout=nullptr to TargetRequest::getTypeLayout. This used to segfault
    // in buildExternTypeMap for a type with an extern member.
    ComPtr<slang::IBlob> layoutDiagnostics;
    auto sceneLayout = session->getTypeLayout(
        sceneType,
        0,
        slang::LayoutRules::Default,
        layoutDiagnostics.writeRef());

    // Must not crash and must return a layout. The single field (the extern member)
    // is present, but its element type stays unresolved because there is no linked
    // definition available on the program-less path.
    SLANG_CHECK(sceneLayout != nullptr);
    if (sceneLayout)
    {
        SLANG_CHECK(sceneLayout->getFieldCount() == 1);
        if (sceneLayout->getFieldCount() == 1)
        {
            // The extern member is present in the layout, but its element type
            // stays unresolved on the program-less path: there is no linked
            // definition, so the extern struct has no fields of its own. This
            // pins the intended contract so a regression in either direction is
            // caught -- a re-introduced crash, or an accidental change that
            // resolves (or drops) the extern member here.
            auto accelStructFieldLayout = sceneLayout->getFieldByIndex(0);
            SLANG_CHECK(accelStructFieldLayout != nullptr);
            auto accelStructTypeLayout =
                accelStructFieldLayout ? accelStructFieldLayout->getTypeLayout() : nullptr;
            SLANG_CHECK(accelStructTypeLayout != nullptr);
            if (accelStructTypeLayout)
                SLANG_CHECK(accelStructTypeLayout->getFieldCount() == 0);
        }
    }
}

// Test that loading a module that defines an `export` type, but not linking with the module should
// not affect the type layout.

SLANG_UNIT_TEST(linkTimeTypeReflectionWithLoadedButNotLinkedModule)
{
    // Source for a module that contains can be specialized with a link-time type.
    const char* userSourceBody = R"(
        interface IFoo { float getVal(); }
        struct DefaultFoo : IFoo { float getVal() { return 0.0f; } }
        extern struct Foo<T, int x> : IFoo = DefaultFoo;
        
        uniform Foo<int4, 1> gFoo;
        RWTexture2D tex;

        [numthreads(1,1,1)]
        [shader("compute")]
        void computeMain() {
            tex[uint2(0, 0)] = gFoo.getVal();
        }
        )";

    String moduleName = "linkTimeTypeReflection_Compute";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // Source for a module that defines the link-time type, but we won't link with it.
    String configModuleSource = "import " + moduleName + ";\n" + R"(
        struct FooImpl<T, int x> : IFoo { T vals[x]; float getVal() { return x; } }
        export struct Foo<T, int x> : IFoo = FooImpl<T, x + 1>;
    )";
    auto configModule = session->loadModuleFromSourceString(
        "config",
        "config.slang",
        configModuleSource.getBuffer(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(configModule != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    module->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    auto programLayout = linkedProgram->getLayout();
    auto var0 = programLayout->getParameterByIndex(0);

    // Size of `gFoo` is 0, because the module that defines `Foo = FooImpl` is not linked.
    // Therefore `gFoo`'s type is defaulted to `DefaultFoo`, which has no fields.
    SLANG_CHECK(var0->getTypeLayout()->getSize() == 0);

    ComPtr<slang::IBlob> codeBlob;
    linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnosticBlob.writeRef());

    SLANG_CHECK_ABORT(codeBlob.get());

    auto spirvStr = UnownedStringSlice((const char*)codeBlob->getBufferPointer());

    SLANG_CHECK(spirvStr.indexOf(toSlice("OpDecorate %tex Binding 0")) != -1);
}

// Test for issue #10749 (global-generic-param path): computing a layout for a type
// that references a module-scope `type_param` (a `GlobalGenericParamDecl`) via the
// program-less `ISession::getTypeLayout` reaches the other two null-guards added in
// this PR -- `findGlobalGenericSpecializationArg` (no program => no specialization
// argument) and `_createTypeLayoutForGlobalGenericTypeParam` (no program => no global
// param index, `paramIndex = -1`). The extern-member tests above do not reach these,
// so this pins them: a regression that turned either guard back into an unconditional
// `programLayout->...` dereference would crash here.
SLANG_UNIT_TEST(linkTimeTypeReflectionGlobalTypeParamSessionGetTypeLayout)
{
    const char* userSourceBody = R"(
        interface IBase {}
        type_param TParam : IBase;

        struct Wrap {
            TParam field;
        }
    )";

    String moduleName = "linkTimeGlobalTypeParam_SessionLayout";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        (moduleName + ".slang").getBuffer(),
        userSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    // Module-level reflection: no link step, so there is no resolved program.
    auto moduleReflection = module->getLayout();
    SLANG_CHECK_ABORT(moduleReflection != nullptr);

    auto wrapType = moduleReflection->findTypeByName("Wrap");
    SLANG_CHECK_ABORT(wrapType != nullptr);

    // Go through ISession::getTypeLayout (Linkage::getTypeLayout), which passes
    // programLayout=nullptr. Laying out `Wrap` reaches the field of type `TParam`
    // (a GlobalGenericParamDecl), exercising the two global-generic-param null guards.
    ComPtr<slang::IBlob> layoutDiagnostics;
    auto wrapLayout = session->getTypeLayout(
        wrapType,
        0,
        slang::LayoutRules::Default,
        layoutDiagnostics.writeRef());

    // Must not crash and must return a layout with the single `TParam` field. The
    // field's global-generic param index is unavailable on the program-less path
    // (paramIndex = -1), but reflecting that index is not part of this test's
    // contract; the point is that laying out the `type_param`-typed field does not
    // dereference a null program layout.
    SLANG_CHECK(wrapLayout != nullptr);
    if (wrapLayout)
    {
        SLANG_CHECK(wrapLayout->getFieldCount() == 1);
    }
}

// Test for issue #10749 (per-program cache correctness): the same base module with an
// `extern` struct member is linked against two different config modules that resolve
// the extern to concrete types of different shape, and each linked program is reflected
// via `programLayout->getTypeLayout`. Because the reflection type-layout cache is scoped
// to the owning `TargetProgram` (not the session-long `TargetRequest`), each program
// must report its own resolved shape.
//
// This pins the lifetime/scoping invariant the cache fix relies on: a refactor that
// folded the two caches back onto `TargetRequest` keyed only by `{type, rules}` would
// return program A's layout for program B's identical `Scene`/`Type*` query, and this
// test would catch it.
SLANG_UNIT_TEST(linkTimeTypeReflectionStructMemberPerProgramCache)
{
    const char* baseSourceBody = R"(
        interface IAccelerationStructure { int getType(); }
        extern struct AccelerationStructure : IAccelerationStructure;

        struct Scene {
            AccelerationStructure accelStruct;
        }

        ParameterBlock<Scene> gScene;

        [numthreads(1,1,1)]
        [shader("compute")]
        void computeMain() {
            int x = gScene.accelStruct.getType();
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV_ASM;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    // A single session: the base module and its `Scene`/`AccelerationStructure` decls
    // (and thus the `Type*` that `findTypeByName("Scene")` yields) are shared across
    // both linked programs, and both programs' `TargetProgram`s hang off the same
    // session-long `TargetRequest`. This is exactly the setup where a `TargetRequest`
    // cache keyed only by `{type, rules}` would alias between programs.
    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto baseModule = session->loadModuleFromSourceString(
        "perProgramCacheBase",
        "perProgramCacheBase.slang",
        baseSourceBody,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(baseModule != nullptr);

    // Link the shared base module against the given config module and return the
    // resolved field count of `Scene.accelStruct` as seen through
    // `programLayout->getTypeLayout`.
    auto resolvedAccelStructFieldCount = [&](const char* configName, const char* configBody) -> int
    {
        String configSource = "import perProgramCacheBase;\n" + String(configBody);
        auto configModule = session->loadModuleFromSourceString(
            configName,
            (String(configName) + ".slang").getBuffer(),
            configSource.getBuffer(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(configModule != nullptr);

        slang::IComponentType* components[] = {baseModule, configModule};
        ComPtr<slang::IComponentType> compositeProgram;
        session->createCompositeComponentType(
            components,
            2,
            compositeProgram.writeRef(),
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(compositeProgram != nullptr);

        ComPtr<slang::IComponentType> linkedProgram;
        compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(linkedProgram != nullptr);

        auto programLayout = linkedProgram->getLayout();
        SLANG_CHECK_ABORT(programLayout != nullptr);

        auto sceneType = programLayout->findTypeByName("Scene");
        SLANG_CHECK_ABORT(sceneType != nullptr);

        auto sceneLayout = programLayout->getTypeLayout(sceneType);
        SLANG_CHECK_ABORT(sceneLayout != nullptr);

        // Two back-to-back calls on the same program must be memoized to the same pointer.
        SLANG_CHECK(programLayout->getTypeLayout(sceneType) == sceneLayout);

        SLANG_CHECK_ABORT(sceneLayout->getFieldCount() == 1);
        auto accelStructTypeLayout = sceneLayout->getFieldByIndex(0)->getTypeLayout();
        SLANG_CHECK_ABORT(accelStructTypeLayout != nullptr);
        return (int)accelStructTypeLayout->getFieldCount();
    };

    // Config A resolves AccelerationStructure to a type with one field.
    const char* configA = R"(
        struct HWAccelerationStructureA : IAccelerationStructure {
            uint handle;
            int getType() { return 1; }
        }
        export struct AccelerationStructure : IAccelerationStructure = HWAccelerationStructureA;
    )";

    // Config B resolves the same extern to a type with two fields.
    const char* configB = R"(
        struct HWAccelerationStructureB : IAccelerationStructure {
            float x;
            float y;
            int getType() { return 2; }
        }
        export struct AccelerationStructure : IAccelerationStructure = HWAccelerationStructureB;
    )";

    int fieldsA = resolvedAccelStructFieldCount("perProgramCacheConfigA", configA);
    int fieldsB = resolvedAccelStructFieldCount("perProgramCacheConfigB", configB);

    // Each program reports its own resolved shape. If the cache were shared across
    // programs keyed only by {type, rules}, the second query would alias the first
    // and both would report the same field count.
    SLANG_CHECK(fieldsA == 1);
    SLANG_CHECK(fieldsB == 2);
}
