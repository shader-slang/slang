// unit-test-translation-unit-import.cpp

#include "slang.h"

#include <stdio.h>
#include <stdlib.h>

#include "tools/unit-test/slang-unit-test.h"
#include "slang-com-ptr.h"
#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process.h"

using namespace Slang;

static String getTypeFullName(slang::TypeReflection* type)
{
    ComPtr<ISlangBlob> blob;
    type->getFullName(blob.writeRef());
    return String((const char*)blob->getBufferPointer());
}

static void printRefl(slang::DeclReflection* refl, unsigned int level = 0)
{
    // Mapping of kind ids to names
    std::string names[] = {"Unsupported", "Struct", "Function", "Module", "Generic", "Variable"};
    for (unsigned int i = 0; i < level; i++)
    {
        std::cout << "  ";
    }
    std::cout<< "[" << names[(unsigned int)refl->getKind()] << "] (" << refl->getChildrenCount() << ")" << std::endl;

    for (auto* child : refl->getChildren())
    {
        printRefl(child, level + 1);
    }
}

// Test that the reflection API provides correct info about entry point and ordinary functions.

SLANG_UNIT_TEST(declTreeReflection)
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

        uint f(uint y) { return y; }

        struct MyType
        {
            int x;
            float f(float x) { return x; }
        }

        struct MyGenericType<T : IArithmetic & IFloat>
        {
            T z;
            T g() { return z; }
            U h<U>(U x, out T y) { y = z; return x; }

            T j<let N : int>(T x, out int o) { o = N; return x; }
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

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = { module, entryPoint.get() };
    session->createCompositeComponentType(components, 2, compositeProgram.writeRef(), diagnosticBlob.writeRef());
    SLANG_CHECK(compositeProgram != nullptr);

    auto moduleDeclReflection = module->getModuleReflection();
    SLANG_CHECK(moduleDeclReflection != nullptr);
    SLANG_CHECK(moduleDeclReflection->getKind() == slang::DeclReflection::Kind::Module);
    SLANG_CHECK(moduleDeclReflection->getChildrenCount() == 7);

    // First declaration should be a struct with 1 variable
    auto firstDecl = moduleDeclReflection->getChild(0);
    SLANG_CHECK(firstDecl->getKind() == slang::DeclReflection::Kind::Struct);
    SLANG_CHECK(firstDecl->getChildrenCount() == 1);

    {
        slang::TypeReflection* type = firstDecl->getType();
        SLANG_CHECK(getTypeFullName(type) == "MyFuncPropertyAttribute");

        // Check the field of the struct.
        SLANG_CHECK(type->getFieldCount() == 1);
        auto field = type->getFieldByIndex(0);
        SLANG_CHECK(UnownedStringSlice(field->getName()) == "v");
        SLANG_CHECK(getTypeFullName(field->getType()) == "int");
    }

    // Second declaration should be a function
    auto secondDecl = moduleDeclReflection->getChild(1);
    SLANG_CHECK(secondDecl->getKind() == slang::DeclReflection::Kind::Func);
    SLANG_CHECK(secondDecl->getChildrenCount() == 2); // Parameter declarations are children (return type is not)

    {
        auto funcReflection = secondDecl->asFunction();
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

    // Third declaration should also be a function
    auto thirdDecl = moduleDeclReflection->getChild(2);
    SLANG_CHECK(thirdDecl->getKind() == slang::DeclReflection::Kind::Func);
    SLANG_CHECK(thirdDecl->getChildrenCount() == 1); 

    {
        auto funcReflection = thirdDecl->asFunction();
        SLANG_CHECK(getTypeFullName(funcReflection->getReturnType()) == "vector<float,4>");
        SLANG_CHECK(UnownedStringSlice(funcReflection->getName()) == "fragMain");
        SLANG_CHECK(funcReflection->getParameterCount() == 1);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getParameterByIndex(0)->getName()) == "pos");
        SLANG_CHECK(getTypeFullName(funcReflection->getParameterByIndex(0)->getType()) == "vector<float,4>");
    }

    // Sixth declaration should be a generic struct
    auto sixthDecl = moduleDeclReflection->getChild(5);
    SLANG_CHECK(sixthDecl->getKind() == slang::DeclReflection::Kind::Generic);
    auto genericReflection = sixthDecl->asGeneric();
    SLANG_CHECK(genericReflection->getTypeParameterCount() == 1);
    auto typeParamT = genericReflection->getTypeParameter(0);
    SLANG_CHECK(UnownedStringSlice(typeParamT->getName()) == "T");
    auto typeParamTConstraintCount = genericReflection->getTypeParameterConstraintCount(typeParamT);
    SLANG_CHECK(typeParamTConstraintCount == 2);
    auto typeParamTConstraintType1 = genericReflection->getTypeParameterConstraintType(typeParamT, 0);
    SLANG_CHECK(getTypeFullName(typeParamTConstraintType1) == "IArithmetic");
    auto typeParamTConstraintType2 = genericReflection->getTypeParameterConstraintType(typeParamT, 1);
    SLANG_CHECK(getTypeFullName(typeParamTConstraintType2) == "IFloat");

    auto innerStruct = genericReflection->getInnerDecl();
    SLANG_CHECK(innerStruct->getKind() == slang::DeclReflection::Kind::Struct);


    // Check type-lookup-by-name
    {
        auto type = compositeProgram->getLayout()->findTypeByName("MyType");
        SLANG_CHECK(type != nullptr);
        //SLANG_CHECK(type->getKind() == slang::DeclReflection::Kind::Struct);
        SLANG_CHECK(UnownedStringSlice(type->getName()) == "MyType");
        auto funcReflection = compositeProgram->getLayout()->findFunctionByNameInType(type, "f");
        SLANG_CHECK(funcReflection != nullptr);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getName()) == "f");
        SLANG_CHECK(getTypeFullName(funcReflection->getReturnType()) == "float");
        SLANG_CHECK(funcReflection->getParameterCount() == 1);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getParameterByIndex(0)->getName()) == "x");
        SLANG_CHECK(getTypeFullName(funcReflection->getParameterByIndex(0)->getType()) == "float");
    }

    // Check type-lookup-by-name for generic type
    {
        auto type = compositeProgram->getLayout()->findTypeByName("MyGenericType<half>");
        SLANG_CHECK(type != nullptr);
        //SLANG_CHECK(type->getKind() == slang::DeclReflection::Kind::Struct);
        SLANG_CHECK(getTypeFullName(type) == "MyGenericType<half>");
        auto funcReflection = compositeProgram->getLayout()->findFunctionByNameInType(type, "g");
        SLANG_CHECK(funcReflection != nullptr);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getName()) == "g");
        SLANG_CHECK(getTypeFullName(funcReflection->getReturnType()) == "half");
        SLANG_CHECK(funcReflection->getParameterCount() == 0);

        auto varReflection = compositeProgram->getLayout()->findVarByNameInType(type, "z");
        SLANG_CHECK(varReflection != nullptr);
        SLANG_CHECK(UnownedStringSlice(varReflection->getName()) == "z");
        SLANG_CHECK(getTypeFullName(varReflection->getType()) == "half");

        funcReflection = compositeProgram->getLayout()->findFunctionByNameInType(type, "h<float>");
        SLANG_CHECK(funcReflection != nullptr);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getName()) == "h");
        SLANG_CHECK(getTypeFullName(funcReflection->getReturnType()) == "float");
        SLANG_CHECK(funcReflection->getParameterCount() == 2);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getParameterByIndex(0)->getName()) == "x");
        SLANG_CHECK(getTypeFullName(funcReflection->getParameterByIndex(0)->getType()) == "float");
        SLANG_CHECK(UnownedStringSlice(funcReflection->getParameterByIndex(1)->getName()) == "y");
        SLANG_CHECK(getTypeFullName(funcReflection->getParameterByIndex(1)->getType()) == "half");
        
        // Access parent generic container from a specialized method.
        auto specializationInfo = funcReflection->getGenericContainer();
        SLANG_CHECK(specializationInfo != nullptr);
        SLANG_CHECK(UnownedStringSlice(specializationInfo->getName()) == "h");
        SLANG_CHECK(specializationInfo->asDecl()->getKind() == slang::DeclReflection::Kind::Generic);
        // Check type parameters
        SLANG_CHECK(specializationInfo->getTypeParameterCount() == 1);
        auto typeParam = specializationInfo->getTypeParameter(0);
        SLANG_CHECK(UnownedStringSlice(typeParam->getName()) == "U"); // generic name
        SLANG_CHECK(getTypeFullName(specializationInfo->getConcreteType(typeParam)) == "float"); // specialized type name under the context in which the generic is obtained
        SLANG_CHECK(specializationInfo->getTypeParameterConstraintCount(typeParam) == 0);

        // Go up another level to the generic struct
        specializationInfo = specializationInfo->getOuterGenericContainer();
        SLANG_CHECK(specializationInfo != nullptr);
        SLANG_CHECK(UnownedStringSlice(specializationInfo->getName()) == "MyGenericType");
        SLANG_CHECK(specializationInfo->asDecl()->getKind() == slang::DeclReflection::Kind::Generic);
        // Check type parameters
        SLANG_CHECK(specializationInfo->getTypeParameterCount() == 1);
        typeParam = specializationInfo->getTypeParameter(0);
        SLANG_CHECK(UnownedStringSlice(typeParam->getName()) == "T"); // generic name
        SLANG_CHECK(getTypeFullName(specializationInfo->getConcreteType(typeParam)) == "half"); // specialized type name under the context in which the generic is obtained
        SLANG_CHECK(specializationInfo->getTypeParameterConstraintCount(typeParam) == 2);

        // Query 'j' on the type 'half'
        funcReflection = compositeProgram->getLayout()->findFunctionByNameInType(type, "j<10>");
        SLANG_CHECK(funcReflection != nullptr);
        SLANG_CHECK(UnownedStringSlice(funcReflection->getName()) == "j");
        
        // Check the generic parameters
        specializationInfo = funcReflection->getGenericContainer();
        SLANG_CHECK(specializationInfo != nullptr);
        SLANG_CHECK(UnownedStringSlice(specializationInfo->getName()) == "j");
        SLANG_CHECK(specializationInfo->asDecl()->getKind() == slang::DeclReflection::Kind::Generic);
        SLANG_CHECK(specializationInfo->getValueParameterCount() == 1);
        auto valueParam = specializationInfo->getValueParameter(0);
        SLANG_CHECK(UnownedStringSlice(valueParam->getName()) == "N"); // generic name
        SLANG_CHECK(specializationInfo->getConcreteIntVal(valueParam) == 10);
    }


    // Check iterators
    {
        unsigned int count = 0;
        for (auto* child : moduleDeclReflection->getChildren())
        {
            count++;
        }
        SLANG_CHECK(count == 7);

        count = 0;
        for (auto* child : moduleDeclReflection->getChildrenOfKind<slang::DeclReflection::Kind::Func>())
        {
            count++;
        }
        SLANG_CHECK(count == 3);

        count = 0;
        for (auto* child : moduleDeclReflection->getChildrenOfKind<slang::DeclReflection::Kind::Struct>())
        {
            count++;
        }
        SLANG_CHECK(count == 2);

        count = 0;
        for (auto* child : moduleDeclReflection->getChildrenOfKind<slang::DeclReflection::Kind::Generic>())
        {
            count++;
        }
        SLANG_CHECK(count == 1);
    }
}

