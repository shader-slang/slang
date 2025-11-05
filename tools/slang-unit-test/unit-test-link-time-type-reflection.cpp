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
    linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnosticBlob.writeRef());

    SLANG_CHECK_ABORT(codeBlob.get());

    auto spirvStr = UnownedStringSlice((const char*)codeBlob->getBufferPointer());

    SLANG_CHECK(spirvStr.indexOf(toSlice("OpDecorate %tex Binding 3")) != -1);
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
