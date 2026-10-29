// unit-test-find-type-by-name.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

#include <stdio.h>
#include <stdlib.h>

using namespace Slang;

SLANG_UNIT_TEST(findTypeByName)
{
    const char* testSource = "struct TestStruct {"
                             "   int member0;"
                             "   Texture2D texture1;"
                             "};";
    auto session = spCreateSession();
    auto request = spCreateCompileRequest(session);
    spAddCodeGenTarget(request, SLANG_DXBC);
    int tuIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "tu1");
    spAddTranslationUnitSourceString(request, tuIndex, "internalFile", testSource);
    spCompile(request);

    auto testBody = [&]()
    {
        auto reflection = slang::ShaderReflection::get(request);
        auto testStruct = reflection->findTypeByName("TestStruct");
        SLANG_CHECK_ABORT(testStruct->getFieldCount() == 2);
        auto field0Name = testStruct->getFieldByIndex(0)->getName();
        SLANG_CHECK_ABORT(field0Name != nullptr && strcmp(field0Name, "member0") == 0);
        auto field1Name = testStruct->getFieldByIndex(1)->getName();
        SLANG_CHECK_ABORT(field1Name != nullptr && strcmp(field1Name, "texture1") == 0);

        auto intType = reflection->findTypeByName("int");
        auto intTypeName = intType->getName();
        SLANG_CHECK_ABORT(intTypeName && strcmp(intTypeName, "int") == 0);

        auto paramBlockType = reflection->findTypeByName("ParameterBlock<TestStruct>");
        SLANG_CHECK_ABORT(paramBlockType != nullptr);
        auto paramBlockTypeName = paramBlockType->getName();
        SLANG_CHECK_ABORT(paramBlockTypeName && strcmp(paramBlockTypeName, "ParameterBlock") == 0);
        auto paramBlockElementType = paramBlockType->getElementType();
        SLANG_CHECK_ABORT(paramBlockElementType != nullptr);
        auto paramBlockElementTypeName = paramBlockElementType->getName();
        SLANG_CHECK_ABORT(
            paramBlockElementTypeName && strcmp(paramBlockElementTypeName, "TestStruct") == 0);
    };

    testBody();

    spDestroyCompileRequest(request);
    spDestroySession(session);
}

SLANG_UNIT_TEST(findTypeByNameExtensionTypeAlias)
{
    const char* vectorizeSource = R"(
        module slangpy;

        public struct VectorizeGridArgTo<SlangParameterType, let Dim : int>
        {
        }

        extension<let N : int, T : __BuiltinIntegerType> VectorizeGridArgTo<vector<T, N>, N>
        {
            typealias VectorType = vector<T, N>;
        }

        extension<T : __BuiltinIntegerType> VectorizeGridArgTo<T, 1>
        {
            typealias VectorType = T;
        }
    )";

    const char* userSource = R"(
        import slangpy;
        [shader("compute")]
        [numthreads(1, 1, 1)]
        void main()
        {
        }
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_5");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnosticBlob;
    auto vectorizeModule = session->loadModuleFromSourceString(
        "slangpy",
        "slangpy.slang",
        vectorizeSource,
        diagnosticBlob.writeRef());
    if (!vectorizeModule && diagnosticBlob)
    {
        fprintf(stderr, "%s\n", (const char*)diagnosticBlob->getBufferPointer());
    }
    SLANG_CHECK_ABORT(vectorizeModule != nullptr);

    diagnosticBlob = nullptr;
    auto module = session->loadModuleFromSourceString(
        "findTypeByNameExtensionTypeAlias",
        "findTypeByNameExtensionTypeAlias.slang",
        userSource,
        diagnosticBlob.writeRef());
    if (!module && diagnosticBlob)
    {
        fprintf(stderr, "%s\n", (const char*)diagnosticBlob->getBufferPointer());
    }
    SLANG_CHECK_ABORT(module != nullptr);

    slang::IComponentType* components[] = {vectorizeModule, module};
    diagnosticBlob = nullptr;
    ComPtr<slang::IComponentType> composedProgram;
    auto composeResult = session->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        composedProgram.writeRef(),
        diagnosticBlob.writeRef());
    if (SLANG_FAILED(composeResult) && diagnosticBlob)
    {
        fprintf(stderr, "%s\n", (const char*)diagnosticBlob->getBufferPointer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(composeResult));

    auto layout = composedProgram->getLayout();
    SLANG_CHECK_ABORT(layout != nullptr);
    auto scalarAliasType = layout->findTypeByName("VectorizeGridArgTo<uint, 1>.VectorType");
    SLANG_CHECK_ABORT(scalarAliasType != nullptr);
    SLANG_CHECK_ABORT(scalarAliasType->getKind() == slang::TypeReflection::Kind::Scalar);
    SLANG_CHECK_ABORT(
        scalarAliasType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);

    auto vectorAliasType =
        layout->findTypeByName("VectorizeGridArgTo<vector<uint, 4>, 4>.VectorType");
    SLANG_CHECK_ABORT(vectorAliasType != nullptr);
    SLANG_CHECK_ABORT(vectorAliasType->getKind() == slang::TypeReflection::Kind::Vector);
    SLANG_CHECK_ABORT(vectorAliasType->getElementCount() == 4);
    SLANG_CHECK_ABORT(
        vectorAliasType->getScalarType() == slang::TypeReflection::ScalarType::UInt32);
}

// Reflection preserves box identity through full names and container elements, while interface
// names remain declaration metadata. Payload specialization also requires a proper value type.
SLANG_UNIT_TEST(existentialBoxReflectionIdentityAndSpecialization)
{
    const char* source = R"(
        public interface IFoo {}
        public interface IBar {}
        public struct Foo : IFoo {}
        public struct SecondFoo : IFoo {}
        public struct NotFoo {}
        public struct Wrapper<T> { T value; }
        public typealias Alias = IFoo;
        public typealias Both = IFoo & IBar;
        public dyn IFoo globalBox;

        [shader("compute")]
        [numthreads(1, 1, 1)]
        public void main(uniform dyn IFoo value) {}
    )";

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    // In 202c a bare interface cannot silently become a box when a full name is looked up again.
    slang::CompilerOptionEntry languageVersion = {};
    languageVersion.name = slang::CompilerOptionName::LanguageVersion;
    languageVersion.value.kind = slang::CompilerOptionValueKind::Int;
    languageVersion.value.intValue0 = SLANG_LANGUAGE_VERSION_202C;
    slang::TargetDesc target = {};
    target.format = SLANG_HLSL;
    target.profile = globalSession->findProfile("sm_5_0");
    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &target;
    sessionDesc.compilerOptionEntryCount = 1;
    sessionDesc.compilerOptionEntries = &languageVersion;
    ComPtr<slang::ISession> session;
    SLANG_CHECK_ABORT(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        "existentialBoxReflection",
        "existential-box-reflection.slang",
        source,
        diagnostics.writeRef());
    SLANG_CHECK_ABORT(module != nullptr);
    auto layout = module->getLayout();
    SLANG_CHECK_ABORT(layout != nullptr);

    // Both direct and nested boxes must survive the `getFullName`/`findTypeByName` round trip.
    // Each spelling is paired with its reflected full name after semantic checking.
    struct TypeNameCase
    {
        const char* input;
        const char* fullName;
    };
    TypeNameCase typeNames[] = {
        {"dyn IFoo", "dyn IFoo"},
        {"dyn (IFoo)", "dyn IFoo"},
        {"(dyn IFoo)", "dyn IFoo"},
        {"dyn ::IFoo", "dyn IFoo"},
        {"::IFoo", "IFoo"},
        {"Wrapper<dyn IFoo>", "Wrapper<dyn IFoo>"},
        {"Wrapper<(dyn IFoo)>", "Wrapper<dyn IFoo>"},
        {"dyn IFoo[2]", "dyn IFoo[2]"}};
    for (auto typeName : typeNames)
    {
        auto type = layout->findTypeByName(typeName.input);
        SLANG_CHECK_ABORT(type != nullptr);
        ComPtr<ISlangBlob> fullName;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(type->getFullName(fullName.writeRef())));
        SLANG_CHECK_ABORT(fullName != nullptr);
        auto reflectedName = (const char*)fullName->getBufferPointer();
        SLANG_CHECK(strcmp(reflectedName, typeName.fullName) == 0);
        SLANG_CHECK(layout->findTypeByName(reflectedName) == type);
    }
    auto boxType = layout->findTypeByName("dyn IFoo");
    SLANG_CHECK(boxType->getKind() == slang::TypeReflection::Kind::Interface);
    SLANG_CHECK(strcmp(boxType->getName(), "IFoo") == 0);

    // Looking up a box through an interface alias must preserve its canonical semantic identity.
    auto aliasedBoxType = layout->findTypeByName("dyn Alias");
    SLANG_CHECK_ABORT(aliasedBoxType != nullptr);
    SLANG_CHECK(layout->isSubType(aliasedBoxType, boxType));
    SLANG_CHECK(layout->isSubType(boxType, aliasedBoxType));

    // Interface metadata remains available for conformance queries, but has no data layout in 202c.
    auto interfaceType = layout->findTypeByName("IFoo");
    auto conjunctionType = layout->findTypeByName("Both");
    auto concreteType = layout->findTypeByName("Foo");
    auto nonconformingType = layout->findTypeByName("NotFoo");
    SLANG_CHECK_ABORT(interfaceType != nullptr);
    SLANG_CHECK_ABORT(conjunctionType != nullptr);
    SLANG_CHECK_ABORT(concreteType != nullptr);
    SLANG_CHECK_ABORT(nonconformingType != nullptr);
    SLANG_CHECK(interfaceType != boxType);
    SLANG_CHECK(layout->isSubType(concreteType, interfaceType));
    SLANG_CHECK(!layout->isSubType(concreteType, boxType));
    SLANG_CHECK(!layout->isSubType(boxType, interfaceType));
    SLANG_CHECK(!layout->isSubType(interfaceType, boxType));
    SLANG_CHECK(layout->isSubType(boxType, boxType));

    // A real container needs a value-type element. In 202c, callers must supply the explicit box;
    // the `None` query still preserves bare interface metadata without constructing a container.
    slang::ContainerType containerKinds[] = {
        slang::ContainerType::UnsizedArray,
        slang::ContainerType::StructuredBuffer,
        slang::ContainerType::ConstantBuffer,
        slang::ContainerType::ParameterBlock};
    SLANG_CHECK(
        session->getContainerType(interfaceType, slang::ContainerType::None) == interfaceType);
    for (auto containerKind : containerKinds)
    {
        diagnostics = nullptr;
        auto rejectedContainer =
            session->getContainerType(interfaceType, containerKind, diagnostics.writeRef());
        SLANG_CHECK(rejectedContainer == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(strstr((const char*)diagnostics->getBufferPointer(), "dyn") != nullptr);

        auto boxedContainer = session->getContainerType(boxType, containerKind);
        SLANG_CHECK_ABORT(boxedContainer != nullptr);
        SLANG_CHECK(boxedContainer->getElementType() == boxType);
        SLANG_CHECK(session->getTypeLayout(boxedContainer) != nullptr);
    }

    // We query a payload's conformance metadata through both the interface and its storage box.
    // Reversing the boxed query order detects a separate ID allocator for the same conformances.
    auto secondConcreteType = layout->findTypeByName("SecondFoo");
    SLANG_CHECK_ABORT(secondConcreteType != nullptr);
    uint32_t firstInterfaceID = 0;
    uint32_t secondInterfaceID = 0;
    uint32_t firstBoxID = 0;
    uint32_t secondBoxID = 0;
    uint32_t aliasedBoxID = 0;
    SLANG_CHECK(SLANG_SUCCEEDED(session->getTypeConformanceWitnessSequentialID(
        concreteType,
        interfaceType,
        &firstInterfaceID)));
    SLANG_CHECK(SLANG_SUCCEEDED(session->getTypeConformanceWitnessSequentialID(
        secondConcreteType,
        interfaceType,
        &secondInterfaceID)));
    SLANG_CHECK(SLANG_SUCCEEDED(
        session->getTypeConformanceWitnessSequentialID(secondConcreteType, boxType, &secondBoxID)));
    SLANG_CHECK(SLANG_SUCCEEDED(
        session->getTypeConformanceWitnessSequentialID(concreteType, boxType, &firstBoxID)));
    SLANG_CHECK(SLANG_SUCCEEDED(session->getTypeConformanceWitnessSequentialID(
        concreteType,
        aliasedBoxType,
        &aliasedBoxID)));
    SLANG_CHECK(firstInterfaceID != secondInterfaceID);
    SLANG_CHECK(firstBoxID == firstInterfaceID);
    SLANG_CHECK(secondBoxID == secondInterfaceID);
    SLANG_CHECK(aliasedBoxID == firstInterfaceID);
    ComPtr<slang::IBlob> interfaceWitnessName;
    ComPtr<slang::IBlob> boxWitnessName;
    ComPtr<slang::IBlob> aliasedBoxWitnessName;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->getTypeConformanceWitnessMangledName(
        concreteType,
        interfaceType,
        interfaceWitnessName.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->getTypeConformanceWitnessMangledName(
        concreteType,
        boxType,
        boxWitnessName.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(session->getTypeConformanceWitnessMangledName(
        concreteType,
        aliasedBoxType,
        aliasedBoxWitnessName.writeRef())));
    SLANG_CHECK_ABORT(interfaceWitnessName != nullptr);
    SLANG_CHECK_ABORT(boxWitnessName != nullptr);
    SLANG_CHECK_ABORT(aliasedBoxWitnessName != nullptr);
    SLANG_CHECK(
        strcmp(
            (const char*)interfaceWitnessName->getBufferPointer(),
            (const char*)boxWitnessName->getBufferPointer()) == 0);
    SLANG_CHECK(
        strcmp(
            (const char*)interfaceWitnessName->getBufferPointer(),
            (const char*)aliasedBoxWitnessName->getBufferPointer()) == 0);

    // RTTI lookup names the same interface declaration for interface metadata and its boxes.
    ComPtr<slang::IBlob> interfaceRTTIName;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        session->getTypeRTTIMangledName(interfaceType, interfaceRTTIName.writeRef())));
    SLANG_CHECK_ABORT(interfaceRTTIName != nullptr);
    slang::TypeReflection* boxTypes[] = {boxType, aliasedBoxType};
    for (auto type : boxTypes)
    {
        ComPtr<slang::IBlob> boxRTTIName;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(session->getTypeRTTIMangledName(type, boxRTTIName.writeRef())));
        SLANG_CHECK_ABORT(boxRTTIName != nullptr);
        SLANG_CHECK(
            strcmp(
                (const char*)interfaceRTTIName->getBufferPointer(),
                (const char*)boxRTTIName->getBufferPointer()) == 0);
    }

    diagnostics = nullptr;
    SLANG_CHECK(
        session->getTypeLayout(
            interfaceType,
            0,
            slang::LayoutRules::Default,
            diagnostics.writeRef()) == nullptr);
    SLANG_CHECK(diagnostics != nullptr);
    SLANG_CHECK(session->getTypeLayout(boxType) != nullptr);
    SLANG_CHECK(layout->getTypeLayout(interfaceType) == nullptr);
    SLANG_CHECK(layout->getTypeLayout(boxType) != nullptr);

    // The same argument policy applies to module globals and uniform entry-point parameters.
    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(module->findEntryPointByName("main", entryPoint.writeRef())));
    slang::IComponentType* components[] = {module, entryPoint.get()};
    for (auto component : components)
    {
        SLANG_CHECK_ABORT(component->getSpecializationParamCount() == 1);
        slang::TypeReflection* invalidTypes[] = {interfaceType, conjunctionType};
        for (auto invalidType : invalidTypes)
        {
            auto argument = slang::SpecializationArg::fromType(invalidType);
            ComPtr<slang::IComponentType> specialized;
            diagnostics = nullptr;
            auto result =
                component->specialize(&argument, 1, specialized.writeRef(), diagnostics.writeRef());
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(specialized == nullptr);
            SLANG_CHECK_ABORT(diagnostics != nullptr);
            SLANG_CHECK(
                strstr(
                    (const char*)diagnostics->getBufferPointer(),
                    "existential box specialization requires a value type") != nullptr);
        }

        // API arguments have no expression, so a failed conformance refers to the parameter.
        auto nonconformingArgument = slang::SpecializationArg::fromType(nonconformingType);
        ComPtr<slang::IComponentType> rejected;
        diagnostics = nullptr;
        SLANG_CHECK(SLANG_FAILED(component->specialize(
            &nonconformingArgument,
            1,
            rejected.writeRef(),
            diagnostics.writeRef())));
        SLANG_CHECK(rejected == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(
            strstr(
                (const char*)diagnostics->getBufferPointer(),
                "type argument doesn't conform to interface") != nullptr);
        SLANG_CHECK(
            strstr(
                (const char*)diagnostics->getBufferPointer(),
                "existential-box-reflection.slang") != nullptr);

        auto argument = slang::SpecializationArg::fromType(concreteType);
        ComPtr<slang::IComponentType> specialized;
        diagnostics = nullptr;
        SLANG_CHECK(SLANG_SUCCEEDED(
            component->specialize(&argument, 1, specialized.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK(specialized != nullptr);
    }

    // Session type specialization fills the same slots and must apply the same payload policy.
    // The generic field has a declaration location even though these API arguments have no syntax.
    auto wrapperType = layout->findTypeByName("Wrapper<dyn IFoo>");
    SLANG_CHECK_ABORT(wrapperType != nullptr);

    // An interface alias must not change the mangled identity of a generic argument's box type.
    auto aliasedWrapperType = layout->findTypeByName("Wrapper<dyn Alias>");
    SLANG_CHECK_ABORT(aliasedWrapperType != nullptr);
    ComPtr<slang::IBlob> wrapperRTTIName;
    ComPtr<slang::IBlob> aliasedWrapperRTTIName;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(session->getTypeRTTIMangledName(wrapperType, wrapperRTTIName.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        session->getTypeRTTIMangledName(aliasedWrapperType, aliasedWrapperRTTIName.writeRef())));
    SLANG_CHECK_ABORT(wrapperRTTIName != nullptr);
    SLANG_CHECK_ABORT(aliasedWrapperRTTIName != nullptr);
    SLANG_CHECK(
        strcmp(
            (const char*)wrapperRTTIName->getBufferPointer(),
            (const char*)aliasedWrapperRTTIName->getBufferPointer()) == 0);

    slang::TypeReflection* invalidPayloadTypes[] =
        {interfaceType, conjunctionType, boxType, nonconformingType};
    for (auto invalidType : invalidPayloadTypes)
    {
        auto argument = slang::SpecializationArg::fromType(invalidType);
        diagnostics = nullptr;
        auto specialized =
            session->specializeType(wrapperType, &argument, 1, diagnostics.writeRef());
        SLANG_CHECK(specialized == nullptr);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(
            strstr(
                (const char*)diagnostics->getBufferPointer(),
                "existential-box-reflection.slang") != nullptr);
    }
    // We also accept the API's `__Dynamic` marker to preserve runtime dispatch.
    slang::TypeReflection* validPayloadTypes[] = {concreteType, session->getDynamicType()};
    for (auto validType : validPayloadTypes)
    {
        auto argument = slang::SpecializationArg::fromType(validType);
        diagnostics = nullptr;
        auto specialized =
            session->specializeType(wrapperType, &argument, 1, diagnostics.writeRef());
        SLANG_CHECK_ABORT(specialized != nullptr);
        SLANG_CHECK(session->getTypeLayout(specialized) != nullptr);
    }

    // Legacy API layout requests still form boxes implicitly; metadata lookup itself stays bare.
    languageVersion.value.intValue0 = SLANG_LANGUAGE_VERSION_LEGACY;
    ComPtr<slang::ISession> legacySession;
    SLANG_CHECK_ABORT(
        globalSession->createSession(sessionDesc, legacySession.writeRef()) == SLANG_OK);
    const char* legacySource = R"(
        public interface IFoo {}
        public void consume(IFoo value) {}
    )";
    auto legacyModule = legacySession->loadModuleFromSourceString(
        "existentialBoxReflectionLegacy",
        "existential-box-reflection-legacy.slang",
        legacySource);
    SLANG_CHECK_ABORT(legacyModule != nullptr);
    auto legacyLayout = legacyModule->getLayout();
    SLANG_CHECK_ABORT(legacyLayout != nullptr);
    auto legacyInterface = legacyLayout->findTypeByName("IFoo");
    auto legacyBox = legacyLayout->findTypeByName("dyn IFoo");
    SLANG_CHECK_ABORT(legacyInterface != nullptr);
    SLANG_CHECK_ABORT(legacyBox != nullptr);
    SLANG_CHECK(legacyInterface != legacyBox);
    // Older versions box a bare interface at the element-construction boundary. The result must
    // be the same canonical container as one explicitly constructed with `dyn IFoo`.
    SLANG_CHECK(
        legacySession->getContainerType(legacyInterface, slang::ContainerType::None) ==
        legacyInterface);
    for (auto containerKind : containerKinds)
    {
        auto implicitContainer = legacySession->getContainerType(legacyInterface, containerKind);
        auto explicitContainer = legacySession->getContainerType(legacyBox, containerKind);
        SLANG_CHECK_ABORT(implicitContainer != nullptr);
        SLANG_CHECK(implicitContainer == explicitContainer);
        SLANG_CHECK_ABORT(implicitContainer->getElementType() == legacyBox);
        SLANG_CHECK(legacySession->getTypeLayout(implicitContainer) != nullptr);
    }
    SLANG_CHECK(legacySession->getTypeLayout(legacyInterface) != nullptr);
    SLANG_CHECK(legacyLayout->getTypeLayout(legacyInterface) != nullptr);

    // A bare interface parameter is implicitly boxed in legacy code and still has an RTTI name.
    auto legacyFunction = legacyLayout->findFunctionByName("consume");
    SLANG_CHECK_ABORT(legacyFunction != nullptr);
    SLANG_CHECK_ABORT(legacyFunction->getParameterCount() == 1);
    auto legacyParameter = legacyFunction->getParameterByIndex(0);
    SLANG_CHECK_ABORT(legacyParameter != nullptr);
    auto legacyParameterType = legacyParameter->getType();
    SLANG_CHECK_ABORT(legacyParameterType != nullptr);
    SLANG_CHECK(legacyLayout->isSubType(legacyParameterType, legacyBox));
    SLANG_CHECK(legacyLayout->isSubType(legacyBox, legacyParameterType));
    ComPtr<slang::IBlob> legacyInterfaceRTTIName;
    ComPtr<slang::IBlob> legacyParameterRTTIName;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(legacySession->getTypeRTTIMangledName(
        legacyInterface,
        legacyInterfaceRTTIName.writeRef())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(legacySession->getTypeRTTIMangledName(
        legacyParameterType,
        legacyParameterRTTIName.writeRef())));
    SLANG_CHECK_ABORT(legacyInterfaceRTTIName != nullptr);
    SLANG_CHECK_ABORT(legacyParameterRTTIName != nullptr);
    SLANG_CHECK(
        strcmp(
            (const char*)legacyInterfaceRTTIName->getBufferPointer(),
            (const char*)legacyParameterRTTIName->getBufferPointer()) == 0);
}

namespace
{
// Each case uses a fresh session so language-version policy cannot leak through reflection caches.
static const SlangLanguageVersion kExistentialReflectionVersions[] = {
    SLANG_LANGUAGE_VERSION_LEGACY,
    SLANG_LANGUAGE_VERSION_2025,
    SLANG_LANGUAGE_VERSION_2026,
    SLANG_LANGUAGE_VERSION_202C};

// A session and module containing both interface contracts and proper existential types.
struct ExistentialReflectionFixture
{
    ComPtr<slang::IGlobalSession> globalSession;
    ComPtr<slang::ISession> session;
    slang::IModule* module = nullptr;
    slang::ProgramLayout* layout = nullptr;

    // Loads the same declarations under `version`; only proper-type consumers apply its policy.
    void initialize(SlangLanguageVersion version)
    {
        const char* source = R"(
            public interface IFoo {}
            public interface IRefined : IFoo {}
            public interface IGeneric<T> {}
            public struct Foo : IFoo {}
            public struct NotFoo {}
            public struct Wrapper<T> { T value; }
            public struct GenericBox<T> { dyn IGeneric<T> value; }
            public typealias Alias = IFoo;
            public void consume(dyn IFoo value) {}
            public T identity<T>(T value) { return value; }
            public int constrained<T : IFoo>(T value) { return 0; }
        )";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef())));
        slang::CompilerOptionEntry languageVersion = {};
        languageVersion.name = slang::CompilerOptionName::LanguageVersion;
        languageVersion.value.kind = slang::CompilerOptionValueKind::Int;
        languageVersion.value.intValue0 = version;
        slang::TargetDesc target = {};
        target.format = SLANG_HLSL;
        target.profile = globalSession->findProfile("sm_5_0");
        slang::SessionDesc desc = {};
        desc.targetCount = 1;
        desc.targets = &target;
        desc.compilerOptionEntryCount = 1;
        desc.compilerOptionEntries = &languageVersion;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(globalSession->createSession(desc, session.writeRef())));

        ComPtr<slang::IBlob> diagnostics;
        module = session->loadModuleFromSourceString(
            "existentialReflectionConsumers",
            "existential-reflection-consumers.slang",
            source,
            diagnostics.writeRef());
        if (!module)
        {
            if (diagnostics)
            {
                fprintf(stderr, "%s\n", (const char*)diagnostics->getBufferPointer());
            }
        }
        SLANG_CHECK_ABORT(module != nullptr);
        layout = module->getLayout();
        SLANG_CHECK_ABORT(layout != nullptr);
    }
};

// Checks classification and names together, including reconstruction from the public full name.
static void checkExistentialReflectionType(
    slang::ProgramLayout* layout,
    slang::TypeReflection* type,
    slang::TypeReflection::Kind kind,
    const char* name,
    const char* fullName)
{
    SLANG_CHECK_ABORT(type != nullptr);
    SLANG_CHECK(type->getKind() == kind);
    SLANG_CHECK_ABORT(type->getName() != nullptr);
    SLANG_CHECK(strcmp(type->getName(), name) == 0);
    ComPtr<slang::IBlob> reflectedName;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(type->getFullName(reflectedName.writeRef())));
    SLANG_CHECK_ABORT(reflectedName != nullptr);
    auto text = (const char*)reflectedName->getBufferPointer();
    SLANG_CHECK(strcmp(text, fullName) == 0);
    SLANG_CHECK(layout->findTypeByName(text) == type);
}

// Checks that inference opens an existential argument and uses that payload consistently.
// Each call creates its own opening, so we compare the return and parameter within one call.
static void checkExistentialArgumentInference(
    slang::ProgramLayout* layout,
    slang::TypeReflection* argumentType)
{
    SLANG_CHECK_ABORT(argumentType != nullptr);
    auto contract = layout->findTypeByName("IFoo");
    auto box = layout->findTypeByName("dyn IFoo");
    auto intType = layout->findTypeByName("int");
    SLANG_CHECK_ABORT(contract != nullptr);
    SLANG_CHECK_ABORT(box != nullptr);
    SLANG_CHECK_ABORT(intType != nullptr);
    slang::TypeReflection* arguments[] = {argumentType};
    auto identity = layout->findFunctionByName("identity");
    auto constrained = layout->findFunctionByName("constrained");
    SLANG_CHECK_ABORT(identity != nullptr);
    SLANG_CHECK_ABORT(constrained != nullptr);
    auto resolvedIdentity = identity->specializeWithArgTypes(1, arguments);
    SLANG_CHECK_ABORT(resolvedIdentity != nullptr);
    auto openedType = resolvedIdentity->getReturnType();
    SLANG_CHECK_ABORT(openedType != nullptr);
    SLANG_CHECK_ABORT(resolvedIdentity->getParameterCount() == 1);
    auto parameter = resolvedIdentity->getParameterByIndex(0);
    SLANG_CHECK_ABORT(parameter != nullptr);
    SLANG_CHECK(openedType == parameter->getType());
    SLANG_CHECK(openedType != argumentType);
    SLANG_CHECK(openedType != contract);
    SLANG_CHECK(openedType != box);
    SLANG_CHECK(layout->isSubType(openedType, contract));

    // The opened payload supplies conformance for calls without making the box itself conform.
    auto resolvedConstrained = constrained->specializeWithArgTypes(1, arguments);
    SLANG_CHECK_ABORT(resolvedConstrained != nullptr);
    SLANG_CHECK(resolvedConstrained->getReturnType() == intType);
}
} // namespace

// Interface declaration queries agree for raw and boxed types without erasing box identity.
// Nested `Optional` types additionally distinguish semantic types from physical layout types.
SLANG_UNIT_TEST(existentialBoxReflectionMetadataAndNestedLayout)
{
    for (auto version : kExistentialReflectionVersions)
    {
        ExistentialReflectionFixture fixture;
        fixture.initialize(version);
        auto layout = fixture.layout;
        auto raw = layout->findTypeByName("IFoo");
        auto box = layout->findTypeByName("dyn IFoo");
        checkExistentialReflectionType(
            layout,
            raw,
            slang::TypeReflection::Kind::Interface,
            "IFoo",
            "IFoo");
        checkExistentialReflectionType(
            layout,
            box,
            slang::TypeReflection::Kind::Interface,
            "IFoo",
            "dyn IFoo");
        SLANG_CHECK(raw != box);

        // Generic arguments need proper types even when the outer lookup is metadata.
        auto implicitWrapper = layout->findTypeByName("Wrapper<IFoo>");
        auto explicitWrapper = layout->findTypeByName("Wrapper<dyn IFoo>");
        SLANG_CHECK_ABORT(explicitWrapper != nullptr);
        if (version == SLANG_LANGUAGE_VERSION_202C)
        {
            SLANG_CHECK(implicitWrapper == nullptr);
            SLANG_CHECK(layout->findTypeByName("Wrapper<IFoo>") == nullptr);
        }
        else
        {
            SLANG_CHECK(implicitWrapper == explicitWrapper);
        }

        // Alias lookup preserves the contract/box distinction and the corresponding semantic type.
        auto rawAlias = layout->findTypeByName("Alias");
        auto boxedAlias = layout->findTypeByName("dyn Alias");
        SLANG_CHECK_ABORT(rawAlias != nullptr);
        SLANG_CHECK_ABORT(boxedAlias != nullptr);
        SLANG_CHECK(rawAlias->getKind() == slang::TypeReflection::Kind::Interface);
        SLANG_CHECK(boxedAlias->getKind() == slang::TypeReflection::Kind::Interface);
        SLANG_CHECK(layout->isSubType(rawAlias, raw));
        SLANG_CHECK(layout->isSubType(raw, rawAlias));
        SLANG_CHECK(layout->isSubType(boxedAlias, box));
        SLANG_CHECK(layout->isSubType(box, boxedAlias));
        SLANG_CHECK(!layout->isSubType(boxedAlias, rawAlias));

        // Generic-container queries project to the interface declaration but keep its arguments.
        auto intType = layout->findTypeByName("int");
        SLANG_CHECK_ABORT(intType != nullptr);
        auto genericRaw = layout->findTypeByName("IGeneric<int>");
        auto genericBox = layout->findTypeByName("dyn IGeneric<int>");
        checkExistentialReflectionType(
            layout,
            genericRaw,
            slang::TypeReflection::Kind::Interface,
            "IGeneric",
            "IGeneric<int>");
        checkExistentialReflectionType(
            layout,
            genericBox,
            slang::TypeReflection::Kind::Interface,
            "IGeneric",
            "dyn IGeneric<int>");
        SLANG_CHECK(genericRaw != genericBox);
        slang::TypeReflection* genericTypes[] = {genericRaw, genericBox};
        for (auto type : genericTypes)
        {
            auto generic = type->getGenericContainer();
            SLANG_CHECK_ABORT(generic != nullptr);
            SLANG_CHECK_ABORT(generic->getName() != nullptr);
            SLANG_CHECK(strcmp(generic->getName(), "IGeneric") == 0);
            SLANG_CHECK_ABORT(generic->getTypeParameterCount() == 1);
            SLANG_CHECK(generic->getConcreteType(generic->getTypeParameter(0)) == intType);
        }

        // Substituting `T` inside `dyn IGeneric<T>` preserves the box around the interface.
        auto genericWrapper = layout->findTypeByName("GenericBox");
        SLANG_CHECK_ABORT(genericWrapper != nullptr);
        SLANG_CHECK_ABORT(genericWrapper->getFieldCount() == 1);
        auto genericField = genericWrapper->getFieldByIndex(0);
        SLANG_CHECK_ABORT(genericField != nullptr);
        auto unspecializedBox = genericField->getType();
        SLANG_CHECK_ABORT(unspecializedBox != nullptr);
        slang::GenericArgType argKind = slang::GenericArgType::SLANG_GENERIC_ARG_TYPE;
        slang::GenericArgReflection intArgument = {intType};
        auto specializedGeneric = layout->specializeGeneric(
            genericWrapper->getGenericContainer(),
            1,
            &argKind,
            &intArgument,
            nullptr);
        SLANG_CHECK_ABORT(specializedGeneric != nullptr);
        SLANG_CHECK(unspecializedBox->applySpecializations(specializedGeneric) == genericBox);

        // `Optional<dyn IFoo>` remains a struct type even though layout uses the nullable box.
        auto optional = layout->findTypeByName("Optional<dyn IFoo>");
        auto nestedOptional = layout->findTypeByName("Optional<Optional<dyn IFoo>>");
        auto wrapper = layout->findTypeByName("Wrapper<Optional<dyn IFoo>>");
        checkExistentialReflectionType(
            layout,
            optional,
            slang::TypeReflection::Kind::Struct,
            "Optional",
            "Optional<dyn IFoo>");
        checkExistentialReflectionType(
            layout,
            nestedOptional,
            slang::TypeReflection::Kind::Struct,
            "Optional",
            "Optional<Optional<dyn IFoo>>");
        checkExistentialReflectionType(
            layout,
            wrapper,
            slang::TypeReflection::Kind::Struct,
            "Wrapper",
            "Wrapper<Optional<dyn IFoo>>");
        SLANG_CHECK(optional->getElementType() == nullptr);
        auto optionalGeneric = optional->getGenericContainer();
        SLANG_CHECK_ABORT(optionalGeneric != nullptr);
        SLANG_CHECK_ABORT(optionalGeneric->getTypeParameterCount() == 1);
        SLANG_CHECK(optionalGeneric->getConcreteType(optionalGeneric->getTypeParameter(0)) == box);
        SLANG_CHECK_ABORT(wrapper->getFieldCount() == 1);
        auto wrapperField = wrapper->getFieldByIndex(0);
        SLANG_CHECK_ABORT(wrapperField != nullptr);
        SLANG_CHECK(wrapperField->getType() == optional);

        auto boxLayout = layout->getTypeLayout(box);
        auto optionalLayout = layout->getTypeLayout(optional);
        SLANG_CHECK_ABORT(boxLayout != nullptr);
        SLANG_CHECK_ABORT(optionalLayout != nullptr);
        SLANG_CHECK(boxLayout->getType() == box);
        SLANG_CHECK(boxLayout->getKind() == slang::TypeReflection::Kind::Interface);
        SLANG_CHECK_ABORT(boxLayout->getName() != nullptr);
        SLANG_CHECK(strcmp(boxLayout->getName(), "IFoo") == 0);
        SLANG_CHECK(optionalLayout->getType() == box);
        SLANG_CHECK(optionalLayout->getKind() == slang::TypeReflection::Kind::Interface);
        SLANG_CHECK_ABORT(optionalLayout->getName() != nullptr);
        SLANG_CHECK(strcmp(optionalLayout->getName(), "IFoo") == 0);
        SLANG_CHECK(optionalLayout->getSize() == boxLayout->getSize());
        SLANG_CHECK(optionalLayout->getAlignment() == boxLayout->getAlignment());
        SLANG_CHECK(
            optionalLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM) ==
            boxLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_TYPE_PARAM));
        SLANG_CHECK(
            optionalLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM) ==
            boxLayout->getSize(SLANG_PARAMETER_CATEGORY_EXISTENTIAL_OBJECT_PARAM));
        auto wrapperLayout = layout->getTypeLayout(wrapper);
        SLANG_CHECK_ABORT(wrapperLayout != nullptr);
        SLANG_CHECK_ABORT(wrapperLayout->getFieldCount() == 1);
        auto wrapperFieldLayout = wrapperLayout->getFieldByIndex(0);
        SLANG_CHECK_ABORT(wrapperFieldLayout != nullptr);
        SLANG_CHECK(wrapperFieldLayout->getType() == optional);
        auto optionalFieldLayout = wrapperFieldLayout->getTypeLayout();
        SLANG_CHECK_ABORT(optionalFieldLayout != nullptr);
        SLANG_CHECK(optionalFieldLayout->getType() == box);
    }
}

// Proper-type API arguments form canonical boxes through 2026 and require explicit `dyn` in 202c.
SLANG_UNIT_TEST(existentialBoxReflectionProperTypeConsumers)
{
    for (auto version : kExistentialReflectionVersions)
    {
        ExistentialReflectionFixture fixture;
        fixture.initialize(version);
        auto layout = fixture.layout;
        auto raw = layout->findTypeByName("IFoo");
        auto box = layout->findTypeByName("dyn IFoo");
        SLANG_CHECK_ABORT(raw != nullptr);
        SLANG_CHECK_ABORT(box != nullptr);

        // Function calls open boxed arguments; raw metadata must enter through the same rule.
        auto refined = layout->findTypeByName("IRefined");
        auto refinedBox = layout->findTypeByName("dyn IRefined");
        SLANG_CHECK_ABORT(refined != nullptr);
        SLANG_CHECK_ABORT(refinedBox != nullptr);
        checkExistentialArgumentInference(layout, box);
        checkExistentialArgumentInference(layout, refinedBox);
        slang::TypeReflection* contracts[] = {raw, refined};
        for (auto contract : contracts)
        {
            if (version == SLANG_LANGUAGE_VERSION_202C)
            {
                slang::TypeReflection* arguments[] = {contract};
                auto identity = layout->findFunctionByName("identity");
                SLANG_CHECK_ABORT(identity != nullptr);
                SLANG_CHECK(identity->specializeWithArgTypes(1, arguments) == nullptr);
            }
            else
            {
                checkExistentialArgumentInference(layout, contract);
            }
        }

        // Layout and all four container kinds use the same proper-type policy.
        auto rawLayout = layout->getTypeLayout(raw);
        auto sessionRawLayout = fixture.session->getTypeLayout(raw);
        if (version == SLANG_LANGUAGE_VERSION_202C)
        {
            SLANG_CHECK(rawLayout == nullptr);
            SLANG_CHECK(sessionRawLayout == nullptr);
        }
        else
        {
            SLANG_CHECK_ABORT(rawLayout != nullptr);
            SLANG_CHECK_ABORT(sessionRawLayout != nullptr);
            SLANG_CHECK(rawLayout->getType() == box);
            SLANG_CHECK(sessionRawLayout->getType() == box);
        }
        SLANG_CHECK(fixture.session->getContainerType(raw, slang::ContainerType::None) == raw);
        slang::ContainerType containers[] = {
            slang::ContainerType::UnsizedArray,
            slang::ContainerType::StructuredBuffer,
            slang::ContainerType::ConstantBuffer,
            slang::ContainerType::ParameterBlock};
        for (auto container : containers)
        {
            auto explicitContainer = fixture.session->getContainerType(box, container);
            auto implicitContainer = fixture.session->getContainerType(raw, container);
            SLANG_CHECK_ABORT(explicitContainer != nullptr);
            SLANG_CHECK(explicitContainer->getElementType() == box);
            auto containerLayout = fixture.session->getTypeLayout(explicitContainer);
            SLANG_CHECK_ABORT(containerLayout != nullptr);
            auto elementLayout = containerLayout->getElementTypeLayout();
            SLANG_CHECK_ABORT(elementLayout != nullptr);
            SLANG_CHECK(elementLayout->getType() == box);
            if (version == SLANG_LANGUAGE_VERSION_202C)
            {
                SLANG_CHECK(implicitContainer == nullptr);
            }
            else
            {
                SLANG_CHECK(implicitContainer == explicitContainer);
            }
        }

        // Explicit generic arguments pass through the same rule before their substitution is kept.
        auto wrapper = layout->findTypeByName("Wrapper");
        SLANG_CHECK_ABORT(wrapper != nullptr);
        slang::GenericArgType argKind = slang::GenericArgType::SLANG_GENERIC_ARG_TYPE;
        slang::GenericArgReflection rawArg = {raw};
        auto specializedGeneric =
            layout
                ->specializeGeneric(wrapper->getGenericContainer(), 1, &argKind, &rawArg, nullptr);
        if (version == SLANG_LANGUAGE_VERSION_202C)
        {
            SLANG_CHECK(specializedGeneric == nullptr);
        }
        else
        {
            SLANG_CHECK_ABORT(specializedGeneric != nullptr);
            auto specializedWrapper = wrapper->applySpecializations(specializedGeneric);
            SLANG_CHECK_ABORT(specializedWrapper != nullptr);
            SLANG_CHECK_ABORT(specializedWrapper->getFieldCount() == 1);
            auto field = specializedWrapper->getFieldByIndex(0);
            SLANG_CHECK_ABORT(field != nullptr);
            SLANG_CHECK(field->getType() == box);
        }

        // Explicit generic arguments do not use call-site opening to satisfy a constraint.
        auto constrained = layout->findFunctionByName("constrained");
        SLANG_CHECK_ABORT(constrained != nullptr);
        slang::TypeReflection* invalidGenericArguments[] = {raw, refined, box, refinedBox};
        for (auto type : invalidGenericArguments)
        {
            slang::GenericArgReflection argument = {type};
            SLANG_CHECK(
                layout->specializeGeneric(
                    constrained->getGenericContainer(),
                    1,
                    &argKind,
                    &argument,
                    nullptr) == nullptr);
        }
    }
}

// Both public type-specialization APIs convert the base before finding its concrete payload slot.
SLANG_UNIT_TEST(existentialBoxReflectionBaseSpecialization)
{
    for (auto version : kExistentialReflectionVersions)
    {
        ExistentialReflectionFixture fixture;
        fixture.initialize(version);
        auto layout = fixture.layout;
        auto raw = layout->findTypeByName("IFoo");
        auto box = layout->findTypeByName("dyn IFoo");
        auto concrete = layout->findTypeByName("Foo");
        SLANG_CHECK_ABORT(raw != nullptr);
        SLANG_CHECK_ABORT(box != nullptr);
        SLANG_CHECK_ABORT(concrete != nullptr);
        auto argument = slang::SpecializationArg::fromType(concrete);
        slang::TypeReflection* arguments[] = {concrete};
        ComPtr<slang::IBlob> diagnostics;
        auto sessionExplicit = fixture.session->specializeType(box, &argument, 1);
        auto programExplicit = layout->specializeType(box, 1, arguments, nullptr);
        SLANG_CHECK_ABORT(sessionExplicit != nullptr);
        SLANG_CHECK_ABORT(programExplicit != nullptr);
        SLANG_CHECK(sessionExplicit == programExplicit);
        SLANG_CHECK(sessionExplicit->getKind() == slang::TypeReflection::Kind::Specialized);
        auto reflected = (SlangReflectionType*)sessionExplicit;
        SLANG_CHECK(spReflectionType_getSpecializedTypeArgCount(reflected) == 1);
        SLANG_CHECK(
            spReflectionType_getSpecializedTypeArgType(reflected, 0) ==
            (SlangReflectionType*)concrete);
        SLANG_CHECK(fixture.session->getTypeLayout(sessionExplicit) != nullptr);
        SLANG_CHECK(layout->getTypeLayout(programExplicit) != nullptr);

        auto sessionImplicit =
            fixture.session->specializeType(raw, &argument, 1, diagnostics.writeRef());
        auto programImplicit = layout->specializeType(raw, 1, arguments, nullptr);
        if (version == SLANG_LANGUAGE_VERSION_202C)
        {
            SLANG_CHECK(sessionImplicit == nullptr);
            SLANG_CHECK(programImplicit == nullptr);
            SLANG_CHECK(diagnostics != nullptr);
        }
        else
        {
            SLANG_CHECK(sessionImplicit == sessionExplicit);
            SLANG_CHECK(programImplicit == programExplicit);
        }

        // A converted base does not relax payload conformance: boxes and unrelated structs fail.
        slang::TypeReflection* invalidPayloads[] = {raw, box, layout->findTypeByName("NotFoo")};
        for (auto payload : invalidPayloads)
        {
            SLANG_CHECK_ABORT(payload != nullptr);
            auto invalidArgument = slang::SpecializationArg::fromType(payload);
            slang::TypeReflection* invalidArguments[] = {payload};
            SLANG_CHECK(fixture.session->specializeType(box, &invalidArgument, 1) == nullptr);
            SLANG_CHECK(layout->specializeType(box, 1, invalidArguments, nullptr) == nullptr);
        }
    }
}

// Conformance creation interprets its target as an interface role, but needs a real implementation.
SLANG_UNIT_TEST(existentialBoxReflectionConformanceComponents)
{
    for (auto version : kExistentialReflectionVersions)
    {
        ExistentialReflectionFixture fixture;
        fixture.initialize(version);
        auto layout = fixture.layout;
        auto raw = layout->findTypeByName("IFoo");
        auto box = layout->findTypeByName("dyn IFoo");
        auto concrete = layout->findTypeByName("Foo");
        auto unrelated = layout->findTypeByName("NotFoo");
        auto consume = layout->findFunctionByName("consume");
        SLANG_CHECK_ABORT(raw != nullptr);
        SLANG_CHECK_ABORT(box != nullptr);
        SLANG_CHECK_ABORT(concrete != nullptr);
        SLANG_CHECK_ABORT(unrelated != nullptr);
        SLANG_CHECK_ABORT(consume != nullptr);
        SLANG_CHECK_ABORT(consume->getParameterCount() == 1);
        auto parameter = consume->getParameterByIndex(0);
        SLANG_CHECK_ABORT(parameter != nullptr);
        auto parameterType = parameter->getType();
        SLANG_CHECK_ABORT(parameterType != nullptr);
        SLANG_CHECK(parameterType == box);

        // Hosts can recover the target from lookup, an alias, or a shader function parameter.
        slang::TypeReflection* targets[] =
            {raw, box, layout->findTypeByName("dyn Alias"), parameterType};
        for (auto target : targets)
        {
            SLANG_CHECK_ABORT(target != nullptr);
            ComPtr<slang::ITypeConformance> conformance;
            SLANG_CHECK(SLANG_SUCCEEDED(fixture.session->createTypeConformanceComponentType(
                concrete,
                target,
                conformance.writeRef(),
                -1,
                nullptr)));
            SLANG_CHECK(conformance != nullptr);
            conformance = nullptr;
            SLANG_CHECK(SLANG_FAILED(fixture.session->createTypeConformanceComponentType(
                unrelated,
                target,
                conformance.writeRef(),
                -1,
                nullptr)));
            SLANG_CHECK(conformance == nullptr);
        }

        // Contract equality and refinement do not supply runtime implementation witnesses.
        slang::TypeReflection* invalidImplementations[] =
            {raw, layout->findTypeByName("IRefined"), box, layout->findTypeByName("dyn IRefined")};
        for (auto implementation : invalidImplementations)
        {
            SLANG_CHECK_ABORT(implementation != nullptr);
            ComPtr<slang::ITypeConformance> conformance;
            SLANG_CHECK(SLANG_FAILED(fixture.session->createTypeConformanceComponentType(
                implementation,
                raw,
                conformance.writeRef(),
                -1,
                nullptr)));
            SLANG_CHECK(conformance == nullptr);
        }
        SLANG_CHECK(layout->isSubType(concrete, raw));
        SLANG_CHECK(!layout->isSubType(concrete, box));
        SLANG_CHECK(!layout->isSubType(box, raw));
    }
}
