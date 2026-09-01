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
