// unit-test-link-time-type-reflection.cpp

#include "../../source/core/slang-io.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"


using namespace Slang;

static String compileEntryPointToSpirvAsm(
    slang::ISession* session,
    const char* moduleName,
    const char* moduleSource,
    const char* configModuleSource)
{
    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        moduleName,
        (String(moduleName) + ".slang").getBuffer(),
        moduleSource,
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->getDefinedEntryPoint(0, entryPoint.writeRef());
    SLANG_CHECK_ABORT(entryPoint != nullptr);

    slang::IComponentType* components[3];
    SlangInt componentCount = 0;
    components[componentCount++] = module;

    ComPtr<slang::IModule> configModule;
    if (configModuleSource)
    {
        configModule = session->loadModuleFromSourceString(
            "config",
            "config.slang",
            configModuleSource,
            diagnosticBlob.writeRef());
        SLANG_CHECK_ABORT(configModule != nullptr);
        components[componentCount++] = configModule;
    }

    components[componentCount++] = entryPoint;

    ComPtr<slang::IComponentType> compositeProgram;
    session->createCompositeComponentType(
        components,
        componentCount,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(compositeProgram != nullptr);

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(linkedProgram != nullptr);

    ComPtr<slang::IBlob> codeBlob;
    linkedProgram->getTargetCode(0, codeBlob.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK_ABORT(codeBlob.get());

    return String(UnownedStringSlice(
        (const char*)codeBlob->getBufferPointer(),
        (Index)codeBlob->getBufferSize()));
}

struct EntryPointOutputLocation
{
    const char* name;
    Index location;
};

static void checkEntryPointOutputLocations(
    const UnownedStringSlice& spirvAsm,
    const char* entryPointName,
    const EntryPointOutputLocation* expected,
    Index expectedCount)
{
    for (Index ii = 0; ii < expectedCount; ++ii)
    {
        StringBuilder builder;
        builder << "OpDecorate %entryPointParam_" << entryPointName << "_" << expected[ii].name
                << " Location " << expected[ii].location;
        SLANG_CHECK(spirvAsm.indexOf(builder.getUnownedSlice()) != -1);
    }
}

static Index findEntryPointOutputLocation(
    const UnownedStringSlice& spirvAsm,
    const char* entryPointName,
    const char* fieldName)
{
    const Index notFound = Index(-1);
    const UnownedStringSlice locationPrefix = toSlice("Location ");
    const UnownedStringSlice decoratePrefix = toSlice("OpDecorate %");
    StringBuilder fieldPattern;
    fieldPattern << "entryPointParam_" << entryPointName << "_" << fieldName;

    auto remaining = spirvAsm;
    while (remaining.getLength() > 0)
    {
        auto newlineIndex = remaining.indexOf('\n');
        auto line =
            (newlineIndex == notFound) ? remaining : remaining.head(newlineIndex);

        if (line.startsWith(decoratePrefix) && line.indexOf(locationPrefix) != -1)
        {
            auto fieldPos = line.indexOf(fieldPattern.getUnownedSlice());
            if (fieldPos != -1)
            {
                auto locationPos = line.indexOf(locationPrefix);
                if (locationPos != -1)
                {
                    auto locSlice = line.tail(locationPos + locationPrefix.getLength());
                    Index digitCount = 0;
                    while (digitCount < locSlice.getLength())
                    {
                        char c = locSlice.begin()[digitCount];
                        if (c < '0' || c > '9')
                            break;
                        ++digitCount;
                    }
                    if (digitCount > 0)
                    {
                        auto digits = locSlice.head(digitCount);
                        return stringToInt(String(digits));
                    }
                }
            }
        }
        if (newlineIndex == notFound)
            break;
        remaining = remaining.tail(newlineIndex + 1);
    }

    return notFound;
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

// Ensure entry-point result layouts built from link-time associated types match the
// layouts produced when the return type is known at compile time.
SLANG_UNIT_TEST(linkTimeEntryPointResultLayoutMatchesConcreteFragmentOutputs)
{
    const char* concreteSource = R"(
        struct ColorOutput
        {
            float4 color0 : SV_Target0;
            float4 color1 : SV_Target1;
            float4 color2[2] : SV_Target2;
            float depth : SV_Depth;
        }

        [shader("fragment")]
        ColorOutput ps_main()
        {
            ColorOutput o;
            return o;
        }
        )";

    const char* linkTimeModuleSource = R"(
        interface IFragOutput { }

        struct ColorOutput : IFragOutput
        {
            float4 color0 : SV_Target0;
            float4 color1 : SV_Target1;
            float4 color2[2] : SV_Target2;
            float depth : SV_Depth;
        }

        interface IShaderMode
        {
            associatedtype FragOut : IFragOutput;
        }

        struct SolidMode : IShaderMode
        {
            typedef ColorOutput FragOut;
        }

        extern struct ShaderMode : IShaderMode;

        [shader("fragment")]
        ShaderMode::FragOut ps_main()
        {
            ShaderMode::FragOut o;
            return o;
        }
        )";

    const char* linkTimeConfigSource = R"(
        import EntryPointLayoutLinkTime;

        export struct ShaderMode : IShaderMode = SolidMode;
        )";

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

    auto concreteSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutConcrete",
        concreteSource,
        nullptr);
    auto linkTimeSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutLinkTime",
        linkTimeModuleSource,
        linkTimeConfigSource);

    Index concreteColor0 =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "color0");
    Index concreteColor1 =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "color1");
    Index concreteColor2 =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "color2");
    Index concreteDepth =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "depth");

    Index linkColor0 =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "color0");
    Index linkColor1 =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "color1");
    Index linkColor2 =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "color2");
    Index linkDepth =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "depth");

    SLANG_CHECK(concreteColor0 >= 0);
    SLANG_CHECK(concreteColor1 >= 0);
    SLANG_CHECK(concreteColor2 >= 0);
    // `SV_Depth` is a system value and does not consume a varying location.
    SLANG_CHECK(concreteDepth < 0);

    SLANG_CHECK(concreteColor0 == linkColor0);
    SLANG_CHECK(concreteColor1 == linkColor1);
    SLANG_CHECK(concreteColor2 == linkColor2);
    SLANG_CHECK(concreteDepth == linkDepth);

    SLANG_CHECK(concreteColor0 == 0);
    SLANG_CHECK(concreteColor1 == 1);
    SLANG_CHECK(concreteColor2 == 2);
}

// Match vk::location output bindings between compile-time and link-time result layouts.
SLANG_UNIT_TEST(linkTimeEntryPointResultLayoutMatchesConcreteVkLocation)
{
    const char* concreteSource = R"(
        struct ColorOutput
        {
            [[vk::location(0)]] float4 color;
            [[vk::location(1)]] float2 uv;
            [[vk::location(2)]] float3 normal;
            [[vk::location(3)]] float4 extra[2];
            [[vk::location(5)]] float2 tail;
        }

        [shader("fragment")]
        ColorOutput ps_main()
        {
            ColorOutput o;
            return o;
        }
        )";

    const char* linkTimeModuleSource = R"(
        interface IFragOutput { }

        struct ColorOutput : IFragOutput
        {
            [[vk::location(0)]] float4 color;
            [[vk::location(1)]] float2 uv;
            [[vk::location(2)]] float3 normal;
            [[vk::location(3)]] float4 extra[2];
            [[vk::location(5)]] float2 tail;
        }

        interface IShaderMode
        {
            associatedtype FragOut : IFragOutput;
        }

        struct SolidMode : IShaderMode
        {
            typedef ColorOutput FragOut;
        }

        extern struct ShaderMode : IShaderMode;

        [shader("fragment")]
        ShaderMode::FragOut ps_main()
        {
            ShaderMode::FragOut o;
            return o;
        }
        )";

    const char* linkTimeConfigSource = R"(
        import EntryPointVkLocationLinkTime;

        export struct ShaderMode : IShaderMode = SolidMode;
        )";

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

    auto concreteSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointVkLocationConcrete",
        concreteSource,
        nullptr);
    auto linkTimeSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointVkLocationLinkTime",
        linkTimeModuleSource,
        linkTimeConfigSource);

    const EntryPointOutputLocation expected[] = {
        {"color", 0},
        {"uv", 1},
        {"normal", 2},
        {"extra", 3},
        {"tail", 5},
    };

    checkEntryPointOutputLocations(
        concreteSpirv.getUnownedSlice(),
        "ps_main",
        expected,
        SLANG_COUNT_OF(expected));
    checkEntryPointOutputLocations(
        linkTimeSpirv.getUnownedSlice(),
        "ps_main",
        expected,
        SLANG_COUNT_OF(expected));
}

// Match TEXCOORD outputs between compile-time and link-time layouts for vertex shaders.
SLANG_UNIT_TEST(linkTimeEntryPointResultLayoutMatchesConcreteVertex)
{
    const char* concreteSource = R"(
        struct VertexOutput
        {
            float4 position : SV_Position;
            float2 uv : TEXCOORD1;
            float3 normal : TEXCOORD2;
        }

        [shader("vertex")]
        VertexOutput vs_main()
        {
            VertexOutput o;
            return o;
        }
        )";

    const char* linkTimeModuleSource = R"(
        interface IVertexOutput { }

        struct VertexOutput : IVertexOutput
        {
            float4 position : SV_Position;
            float2 uv : TEXCOORD1;
            float3 normal : TEXCOORD2;
        }

        interface IShaderMode
        {
            associatedtype VertOut : IVertexOutput;
        }

        struct SolidMode : IShaderMode
        {
            typedef VertexOutput VertOut;
        }

        extern struct ShaderMode : IShaderMode;

        [shader("vertex")]
        ShaderMode::VertOut vs_main()
        {
            ShaderMode::VertOut o;
            return o;
        }
        )";

    const char* linkTimeConfigSource = R"(
        import EntryPointLayoutVertexLinkTime;

        export struct ShaderMode : IShaderMode = SolidMode;
        )";

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

    auto concreteSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutVertexConcrete",
        concreteSource,
        nullptr);
    auto linkTimeSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutVertexLinkTime",
        linkTimeModuleSource,
        linkTimeConfigSource);

    Index concreteUv =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "vs_main", "uv");
    Index concreteNormal =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "vs_main", "normal");
    Index concretePosition =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "vs_main", "position");

    Index linkUv =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "vs_main", "uv");
    Index linkNormal =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "vs_main", "normal");
    Index linkPosition =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "vs_main", "position");

    SLANG_CHECK(concreteUv >= 0);
    SLANG_CHECK(concreteNormal >= 0);
    // `SV_Position` is a system value and does not consume a varying location.
    SLANG_CHECK(concretePosition < 0);

    SLANG_CHECK(concreteUv == linkUv);
    SLANG_CHECK(concreteNormal == linkNormal);
    SLANG_CHECK(concretePosition == linkPosition);

    SLANG_CHECK(concreteUv >= 0);
    SLANG_CHECK(concreteNormal >= 0);
}

// Match nested struct outputs between compile-time and link-time layouts for fragment shaders.
SLANG_UNIT_TEST(linkTimeEntryPointResultLayoutMatchesConcreteFragmentNested)
{
    const char* concreteSource = R"(
        struct Inner
        {
            float4 color0 : SV_Target0;
            float4 color2 : SV_Target2;
        }

        struct Outer
        {
            Inner inner;
            float4 color1 : SV_Target1;
        }

        [shader("fragment")]
        Outer ps_main()
        {
            Outer o;
            return o;
        }
        )";

    const char* linkTimeModuleSource = R"(
        interface IFragOutput { }

        struct Inner
        {
            float4 color0 : SV_Target0;
            float4 color2 : SV_Target2;
        }

        struct Outer : IFragOutput
        {
            Inner inner;
            float4 color1 : SV_Target1;
        }

        interface IShaderMode
        {
            associatedtype FragOut : IFragOutput;
        }

        struct SolidMode : IShaderMode
        {
            typedef Outer FragOut;
        }

        extern struct ShaderMode : IShaderMode;

        [shader("fragment")]
        ShaderMode::FragOut ps_main()
        {
            ShaderMode::FragOut o;
            return o;
        }
        )";

    const char* linkTimeConfigSource = R"(
        import EntryPointLayoutNestedLinkTime;

        export struct ShaderMode : IShaderMode = SolidMode;
        )";

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

    auto concreteSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutNestedConcrete",
        concreteSource,
        nullptr);
    auto linkTimeSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutNestedLinkTime",
        linkTimeModuleSource,
        linkTimeConfigSource);

    Index concreteInnerColor0 =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "inner_color0");
    Index concreteInnerColor2 =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "inner_color2");
    Index concreteColor1 =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "ps_main", "color1");

    Index linkInnerColor0 =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "inner_color0");
    Index linkInnerColor2 =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "inner_color2");
    Index linkColor1 =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "ps_main", "color1");

    SLANG_CHECK(concreteInnerColor0 >= 0);
    SLANG_CHECK(concreteInnerColor2 >= 0);
    SLANG_CHECK(concreteColor1 >= 0);

    SLANG_CHECK(concreteInnerColor0 == linkInnerColor0);
    SLANG_CHECK(concreteInnerColor2 == linkInnerColor2);
    SLANG_CHECK(concreteColor1 == linkColor1);

    SLANG_CHECK(concreteInnerColor0 == 0);
    SLANG_CHECK(concreteInnerColor2 == 1);
    SLANG_CHECK(concreteColor1 == 2);
}

// Match nested struct outputs between compile-time and link-time layouts for vertex shaders.
SLANG_UNIT_TEST(linkTimeEntryPointResultLayoutMatchesConcreteVertexNested)
{
    const char* concreteSource = R"(
        struct Inner
        {
            float2 uv : TEXCOORD1;
            float3 normal : TEXCOORD2;
        }

        struct Outer
        {
            float4 position : SV_Position;
            Inner inner;
        }

        [shader("vertex")]
        Outer vs_main()
        {
            Outer o;
            return o;
        }
        )";

    const char* linkTimeModuleSource = R"(
        interface IVertexOutput { }

        struct Inner
        {
            float2 uv : TEXCOORD1;
            float3 normal : TEXCOORD2;
        }

        struct Outer : IVertexOutput
        {
            float4 position : SV_Position;
            Inner inner;
        }

        interface IShaderMode
        {
            associatedtype VertOut : IVertexOutput;
        }

        struct SolidMode : IShaderMode
        {
            typedef Outer VertOut;
        }

        extern struct ShaderMode : IShaderMode;

        [shader("vertex")]
        ShaderMode::VertOut vs_main()
        {
            ShaderMode::VertOut o;
            return o;
        }
        )";

    const char* linkTimeConfigSource = R"(
        import EntryPointLayoutVertexNestedLinkTime;

        export struct ShaderMode : IShaderMode = SolidMode;
        )";

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

    auto concreteSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutVertexNestedConcrete",
        concreteSource,
        nullptr);
    auto linkTimeSpirv = compileEntryPointToSpirvAsm(
        session,
        "EntryPointLayoutVertexNestedLinkTime",
        linkTimeModuleSource,
        linkTimeConfigSource);

    Index concreteInnerUv =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "vs_main", "inner_uv");
    Index concreteInnerNormal =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "vs_main", "inner_normal");
    Index concretePosition =
        findEntryPointOutputLocation(concreteSpirv.getUnownedSlice(), "vs_main", "position");

    Index linkInnerUv =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "vs_main", "inner_uv");
    Index linkInnerNormal =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "vs_main", "inner_normal");
    Index linkPosition =
        findEntryPointOutputLocation(linkTimeSpirv.getUnownedSlice(), "vs_main", "position");

    SLANG_CHECK(concreteInnerUv >= 0);
    SLANG_CHECK(concreteInnerNormal >= 0);
    // `SV_Position` is a system value and does not consume a varying location.
    SLANG_CHECK(concretePosition < 0);

    SLANG_CHECK(concreteInnerUv == linkInnerUv);
    SLANG_CHECK(concreteInnerNormal == linkInnerNormal);
    SLANG_CHECK(concretePosition == linkPosition);
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
