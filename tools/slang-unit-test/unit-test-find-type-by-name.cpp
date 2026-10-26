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

// Full type names preserve existential box identity, while interface names remain declaration
// metadata. Specializing an existential box's payload requires a data type, not interface metadata.
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
