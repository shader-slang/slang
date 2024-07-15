// unit-test-translation-unit-import.cpp

#include <slang.h>

#include <stdio.h>
#include <stdlib.h>

#include "tools/unit-test/slang-unit-test.h"
#include <slang-com-ptr.h>
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"

using namespace Slang;

static String getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return String((const char*)blob->getBufferPointer());
}

// Test that the reflection API provides correct info about entry point and ordinary functions.

SLANG_UNIT_TEST(functionReflection)
{
    // Source for a module that contains an undecorated entrypoint.
    const char* userSourceBody = R"(
        [__AttributeUsage(_AttributeTargets.Function)]
        struct MyFuncPropertyAttribute {int v;}

        [MyFuncProperty(1024)]
        [Differentiable]
        float ordinaryFunc(no_diff float x, int y) { return x + y; }

        float4 fragMain(float4 pos:SV_Position) : SV_Position
        {
            return pos;
        }
        )";

    auto moduleName = "moduleG" + String(Process::getId());
    String userSource = "import " + moduleName + ";\n" + userSourceBody;
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
    auto module = session->loadModuleFromSourceString("m", "m.slang", userSourceBody, diagnosticBlob.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint("fragMain", SLANG_STAGE_FRAGMENT, entryPoint.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(entryPoint != nullptr);

    auto entryPointFuncReflection = entryPoint->getFunctionReflection();
    SLANG_CHECK(entryPointFuncReflection != nullptr);
    SLANG_CHECK(UnownedStringSlice(entryPointFuncReflection->getName()) == "fragMain");
    SLANG_CHECK(entryPointFuncReflection->getParameterCount() == 1);
    SLANG_CHECK(UnownedStringSlice(entryPointFuncReflection->getParameterByIndex(0)->getName()) == "pos");
    SLANG_CHECK(getTypeFullName(entryPointFuncReflection->getParameterByIndex(0)->getType()) == "vector<float,4>");

    auto funcReflection = module->getLayout()->findFunctionByName("ordinaryFunc");
    SLANG_CHECK(funcReflection != nullptr);

    SLANG_CHECK(funcReflection->findModifier(slang::Modifier::Differentiable) != nullptr);
    SLANG_CHECK(getTypeFullName(funcReflection->getReturnType()) == "float");
    SLANG_CHECK(UnownedStringSlice(funcReflection->getName()) == "ordinaryFunc");
    SLANG_CHECK(funcReflection->getParameterCount() == 2);
    SLANG_CHECK(UnownedStringSlice(funcReflection->getParameterByIndex(0)->getName()) == "x");
    SLANG_CHECK(getTypeFullName(funcReflection->getParameterByIndex(0)->getType()) == "float");
    SLANG_CHECK(funcReflection->getParameterByIndex(0)->findModifier(slang::Modifier::NoDiff) != nullptr);

    SLANG_CHECK(UnownedStringSlice(funcReflection->getParameterByIndex(1)->getName()) == "y");
    SLANG_CHECK(getTypeFullName(funcReflection->getParameterByIndex(1)->getType()) == "int");

    SLANG_CHECK(funcReflection->getUserAttributeCount() == 1);
    auto userAttribute = funcReflection->getUserAttributeByIndex(0);
    SLANG_CHECK(UnownedStringSlice(userAttribute->getName()) == "MyFuncProperty");
    SLANG_CHECK(userAttribute->getArgumentCount() == 1);
    SLANG_CHECK(getTypeFullName(userAttribute->getArgumentType(0)) == "int");
    int val = 0;
    auto result = userAttribute->getArgumentValueInt(0, &val);
    SLANG_CHECK(result == SLANG_OK);
    SLANG_CHECK(val == 1024);
    SLANG_CHECK(funcReflection->findUserAttributeByName(globalSession.get(), "MyFuncProperty") == userAttribute);
}

