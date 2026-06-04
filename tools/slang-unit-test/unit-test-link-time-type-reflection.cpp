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

    auto getStaticIntBlob = [&](const char* typeName, const char* varName) -> int32_t
    {
        auto type = programLayout->findTypeByName(typeName);
        SLANG_CHECK_ABORT(type != nullptr);

        auto valueVar = programLayout->findVarByNameInType(type, varName);
        SLANG_CHECK_ABORT(valueVar != nullptr);

        ComPtr<slang::IBlob> blob;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(valueVar->getDefaultValueBlob(blob.writeRef())));
        SLANG_CHECK_ABORT(blob != nullptr);
        SLANG_CHECK_ABORT(blob->getBufferSize() == sizeof(int32_t));
        return ((const int32_t*)blob->getBufferPointer())[0];
    };

    SLANG_CHECK(getStaticInt("StaticConstCarrier<5>", "Value") == 6);
    SLANG_CHECK(getStaticInt("NestedCarrier<5>", "Value") == 9);
    SLANG_CHECK(getStaticInt("TypeCarrier<float,5>", "Value") == 10);
    SLANG_CHECK(getStaticInt("LiteralCarrier", "Value") == 23);

    SLANG_CHECK(getStaticIntBlob("StaticConstCarrier<5>", "Value") == 6);
    SLANG_CHECK(getStaticIntBlob("NestedCarrier<5>", "Value") == 9);
    SLANG_CHECK(getStaticIntBlob("TypeCarrier<float,5>", "Value") == 10);
    SLANG_CHECK(getStaticIntBlob("LiteralCarrier", "Value") == 23);
}

SLANG_UNIT_TEST(defaultValueBlobReflection)
{
    const char* userSourceBody = R"(
        module DefaultValueBlobReflection;

        public struct MaterialConstants
        {
            public float3 baseOrDiffuseColor = 1.0f;
            public int materialID = -1;
        }

        public enum TopMode : int
        {
            C = 4,
            D = 6,
        }

        public struct Defaults
        {
            public struct Pair
            {
                public float X = 2.0f;
                public int Y;
            }

            public struct Base
            {
                public float2 BaseValue = float2(7.0f, 8.0f);
            }

            public struct Derived : Base
            {
                public int Z = 9;
            }

            public enum Mode : int
            {
                A = 0,
                B = 3,
            }

            public enum ByteMode : uint8_t
            {
                Low = 2,
                High = 250,
            }

            public static const float ScalarFloat = 1.5f;
            public static const int ScalarInt = -7;
            public static const bool ScalarBool = true;
            public static const bool ScalarBoolFromInt = 1;
            public static const bool ScalarBoolFalse = false;
            public static const bool ScalarBoolFromIntZero = 0;
            public static const int ScalarIntFromBool = true;
            public static const float ScalarFloatFromBool = false;
            public static const half ScalarHalf = half(0.5f);
            public static const double ScalarDouble = 3.25;
            public static const int8_t ScalarInt8 = -5;
            public static const uint8_t ScalarUInt8 = 250;
            public static const int16_t ScalarInt16 = -300;
            public static const uint16_t ScalarUInt16 = 65000;
            public static const uint ScalarUInt = 0xDEADBEEFu;
            public static const int64_t ScalarInt64 = 0x100000001LL;
            public static const uint64_t ScalarUInt64 = 0xFFFFFFFF00000001ULL;
            public static const intptr_t ScalarIntPtr = -11;
            public static const uintptr_t ScalarUIntPtr = 13;
            typedef float3 AliasFloat3;
            public static const AliasFloat3 AliasVector = float3(9.0f, 10.0f, 11.0f);
            public static const float3 VectorFloat = float3(1.0f, 2.0f, 1.5f);
            public static const float3 VectorSplat = float3(4.0f);
            public static const int3 VectorInt = int3(1, -2, 3);
            public static const float3 VectorPartial = { 5.0f, 6.0f };
            public static const float3 VectorDefault = {};
            public static const float2x3 MatrixValue = { float3(1.0f, 2.0f, 3.0f), float3(4.0f, 5.0f, 6.0f) };
            public static const float2x3 MatrixDefault = {};
            public static const int ArrayValue[3] = { 1, 2 };
            public static const int ArrayDefault[3] = {};
            public static const int LargeArray[16777217] = { 1 };
            public static const Pair StructPartial = { 5.0f };
            public static const Pair StructDefault = {};
            public static const Derived DerivedDefault = {};
            public static const Derived DerivedExplicit = { 3 };
            public static const Mode EnumValue = Mode.B;
            public static const Mode EnumAlias = EnumValue;
            public static const Mode EnumCast = Mode(5);
            public static const Mode EnumDefault = {};
            public static const ByteMode ByteEnumValue = ByteMode.High;
            public Derived ValueWithoutInitializer;
        }
        )";

    String moduleName = "DefaultValueBlobReflection";

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

    auto defaultsType = programLayout->findTypeByName("Defaults");
    SLANG_CHECK_ABORT(defaultsType != nullptr);
    auto materialConstantsType = programLayout->findTypeByName("MaterialConstants");
    SLANG_CHECK_ABORT(materialConstantsType != nullptr);
    auto topModeType = programLayout->findTypeByName("TopMode");
    SLANG_CHECK_ABORT(topModeType != nullptr);

    auto getDefaultBlob = [&](const char* varName) -> ComPtr<slang::IBlob>
    {
        auto valueVar = programLayout->findVarByNameInType(defaultsType, varName);
        SLANG_CHECK_ABORT(valueVar != nullptr);

        ComPtr<slang::IBlob> blob;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(valueVar->getDefaultValueBlob(blob.writeRef())));
        SLANG_CHECK_ABORT(blob != nullptr);
        return blob;
    };

    auto getDefaultBlobResult = [&](const char* varName, ComPtr<slang::IBlob>& blob) -> SlangResult
    {
        auto valueVar = programLayout->findVarByNameInType(defaultsType, varName);
        SLANG_CHECK_ABORT(valueVar != nullptr);
        return valueVar->getDefaultValueBlob(blob.writeRef());
    };

    auto getFieldDefaultBlob = [&](slang::TypeReflection* type, const char* fieldName)
        -> ComPtr<slang::IBlob>
    {
        auto fieldVar = programLayout->findVarByNameInType(type, fieldName);
        SLANG_CHECK_ABORT(fieldVar != nullptr);

        ComPtr<slang::IBlob> blob;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(fieldVar->getDefaultValueBlob(blob.writeRef())));
        SLANG_CHECK_ABORT(blob != nullptr);
        return blob;
    };

    auto baseOrDiffuseColor = getFieldDefaultBlob(materialConstantsType, "baseOrDiffuseColor");
    SLANG_CHECK(baseOrDiffuseColor->getBufferSize() == sizeof(float) * 3);
    auto baseOrDiffuseColorData = (const float*)baseOrDiffuseColor->getBufferPointer();
    SLANG_CHECK(baseOrDiffuseColorData[0] == 1.0f);
    SLANG_CHECK(baseOrDiffuseColorData[1] == 1.0f);
    SLANG_CHECK(baseOrDiffuseColorData[2] == 1.0f);

    auto materialID = getFieldDefaultBlob(materialConstantsType, "materialID");
    SLANG_CHECK(materialID->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)materialID->getBufferPointer())[0] == -1);

    auto scalarFloat = getDefaultBlob("ScalarFloat");
    SLANG_CHECK(scalarFloat->getBufferSize() == sizeof(float));
    SLANG_CHECK(((const float*)scalarFloat->getBufferPointer())[0] == 1.5f);

    auto scalarInt = getDefaultBlob("ScalarInt");
    SLANG_CHECK(scalarInt->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)scalarInt->getBufferPointer())[0] == -7);

    auto scalarBool = getDefaultBlob("ScalarBool");
    SLANG_CHECK(scalarBool->getBufferSize() == sizeof(uint32_t));
    SLANG_CHECK(((const uint32_t*)scalarBool->getBufferPointer())[0] == 1);

    auto scalarBoolFromInt = getDefaultBlob("ScalarBoolFromInt");
    SLANG_CHECK(scalarBoolFromInt->getBufferSize() == sizeof(uint32_t));
    SLANG_CHECK(((const uint32_t*)scalarBoolFromInt->getBufferPointer())[0] == 1);

    auto scalarBoolFalse = getDefaultBlob("ScalarBoolFalse");
    SLANG_CHECK(scalarBoolFalse->getBufferSize() == sizeof(uint32_t));
    SLANG_CHECK(((const uint32_t*)scalarBoolFalse->getBufferPointer())[0] == 0);

    auto scalarBoolFromIntZero = getDefaultBlob("ScalarBoolFromIntZero");
    SLANG_CHECK(scalarBoolFromIntZero->getBufferSize() == sizeof(uint32_t));
    SLANG_CHECK(((const uint32_t*)scalarBoolFromIntZero->getBufferPointer())[0] == 0);

    auto scalarIntFromBool = getDefaultBlob("ScalarIntFromBool");
    SLANG_CHECK(scalarIntFromBool->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)scalarIntFromBool->getBufferPointer())[0] == 1);

    auto scalarFloatFromBool = getDefaultBlob("ScalarFloatFromBool");
    SLANG_CHECK(scalarFloatFromBool->getBufferSize() == sizeof(float));
    SLANG_CHECK(((const float*)scalarFloatFromBool->getBufferPointer())[0] == 0.0f);

    auto scalarHalf = getDefaultBlob("ScalarHalf");
    SLANG_CHECK(scalarHalf->getBufferSize() == sizeof(uint16_t));
    SLANG_CHECK(((const uint16_t*)scalarHalf->getBufferPointer())[0] == 0x3800);

    auto scalarDouble = getDefaultBlob("ScalarDouble");
    SLANG_CHECK(scalarDouble->getBufferSize() == sizeof(double));
    SLANG_CHECK(((const double*)scalarDouble->getBufferPointer())[0] == 3.25);

    auto scalarInt8 = getDefaultBlob("ScalarInt8");
    SLANG_CHECK(scalarInt8->getBufferSize() == sizeof(int8_t));
    SLANG_CHECK(((const int8_t*)scalarInt8->getBufferPointer())[0] == -5);

    auto scalarUInt8 = getDefaultBlob("ScalarUInt8");
    SLANG_CHECK(scalarUInt8->getBufferSize() == sizeof(uint8_t));
    SLANG_CHECK(((const uint8_t*)scalarUInt8->getBufferPointer())[0] == 250);

    auto scalarInt16 = getDefaultBlob("ScalarInt16");
    SLANG_CHECK(scalarInt16->getBufferSize() == sizeof(int16_t));
    SLANG_CHECK(((const int16_t*)scalarInt16->getBufferPointer())[0] == -300);

    auto scalarUInt16 = getDefaultBlob("ScalarUInt16");
    SLANG_CHECK(scalarUInt16->getBufferSize() == sizeof(uint16_t));
    SLANG_CHECK(((const uint16_t*)scalarUInt16->getBufferPointer())[0] == 65000);

    auto scalarUInt = getDefaultBlob("ScalarUInt");
    SLANG_CHECK(scalarUInt->getBufferSize() == sizeof(uint32_t));
    SLANG_CHECK(((const uint32_t*)scalarUInt->getBufferPointer())[0] == 0xDEADBEEF);

    auto scalarInt64 = getDefaultBlob("ScalarInt64");
    SLANG_CHECK(scalarInt64->getBufferSize() == sizeof(int64_t));
    SLANG_CHECK(((const int64_t*)scalarInt64->getBufferPointer())[0] == 0x100000001LL);

    auto scalarUInt64 = getDefaultBlob("ScalarUInt64");
    SLANG_CHECK(scalarUInt64->getBufferSize() == sizeof(uint64_t));
    SLANG_CHECK(((const uint64_t*)scalarUInt64->getBufferPointer())[0] == 0xFFFFFFFF00000001ULL);

    auto scalarIntPtr = getDefaultBlob("ScalarIntPtr");
    SLANG_CHECK(scalarIntPtr->getBufferSize() == sizeof(intptr_t));
    SLANG_CHECK(((const intptr_t*)scalarIntPtr->getBufferPointer())[0] == -11);

    auto scalarUIntPtr = getDefaultBlob("ScalarUIntPtr");
    SLANG_CHECK(scalarUIntPtr->getBufferSize() == sizeof(uintptr_t));
    SLANG_CHECK(((const uintptr_t*)scalarUIntPtr->getBufferPointer())[0] == 13);

    auto aliasVector = getDefaultBlob("AliasVector");
    SLANG_CHECK(aliasVector->getBufferSize() == sizeof(float) * 3);
    auto aliasVectorData = (const float*)aliasVector->getBufferPointer();
    SLANG_CHECK(aliasVectorData[0] == 9.0f);
    SLANG_CHECK(aliasVectorData[1] == 10.0f);
    SLANG_CHECK(aliasVectorData[2] == 11.0f);

    auto vectorFloat = getDefaultBlob("VectorFloat");
    SLANG_CHECK(vectorFloat->getBufferSize() == sizeof(float) * 3);
    auto vectorFloatData = (const float*)vectorFloat->getBufferPointer();
    SLANG_CHECK(vectorFloatData[0] == 1.0f);
    SLANG_CHECK(vectorFloatData[1] == 2.0f);
    SLANG_CHECK(vectorFloatData[2] == 1.5f);

    auto vectorSplat = getDefaultBlob("VectorSplat");
    SLANG_CHECK(vectorSplat->getBufferSize() == sizeof(float) * 3);
    auto vectorSplatData = (const float*)vectorSplat->getBufferPointer();
    SLANG_CHECK(vectorSplatData[0] == 4.0f);
    SLANG_CHECK(vectorSplatData[1] == 4.0f);
    SLANG_CHECK(vectorSplatData[2] == 4.0f);

    auto vectorInt = getDefaultBlob("VectorInt");
    SLANG_CHECK(vectorInt->getBufferSize() == sizeof(int32_t) * 3);
    auto vectorIntData = (const int32_t*)vectorInt->getBufferPointer();
    SLANG_CHECK(vectorIntData[0] == 1);
    SLANG_CHECK(vectorIntData[1] == -2);
    SLANG_CHECK(vectorIntData[2] == 3);

    auto vectorPartial = getDefaultBlob("VectorPartial");
    SLANG_CHECK(vectorPartial->getBufferSize() == sizeof(float) * 3);
    auto vectorPartialData = (const float*)vectorPartial->getBufferPointer();
    SLANG_CHECK(vectorPartialData[0] == 5.0f);
    SLANG_CHECK(vectorPartialData[1] == 6.0f);
    SLANG_CHECK(vectorPartialData[2] == 0.0f);

    auto vectorDefault = getDefaultBlob("VectorDefault");
    SLANG_CHECK(vectorDefault->getBufferSize() == sizeof(float) * 3);
    auto vectorDefaultData = (const float*)vectorDefault->getBufferPointer();
    SLANG_CHECK(vectorDefaultData[0] == 0.0f);
    SLANG_CHECK(vectorDefaultData[1] == 0.0f);
    SLANG_CHECK(vectorDefaultData[2] == 0.0f);

    auto matrixValue = getDefaultBlob("MatrixValue");
    SLANG_CHECK(matrixValue->getBufferSize() == sizeof(float) * 6);
    auto matrixData = (const float*)matrixValue->getBufferPointer();
    for (int i = 0; i < 6; ++i)
        SLANG_CHECK(matrixData[i] == float(i + 1));

    auto matrixDefault = getDefaultBlob("MatrixDefault");
    SLANG_CHECK(matrixDefault->getBufferSize() == sizeof(float) * 6);
    auto matrixDefaultData = (const float*)matrixDefault->getBufferPointer();
    for (int i = 0; i < 6; ++i)
        SLANG_CHECK(matrixDefaultData[i] == 0.0f);

    auto arrayValue = getDefaultBlob("ArrayValue");
    SLANG_CHECK(arrayValue->getBufferSize() == sizeof(int32_t) * 3);
    auto arrayData = (const int32_t*)arrayValue->getBufferPointer();
    SLANG_CHECK(arrayData[0] == 1);
    SLANG_CHECK(arrayData[1] == 2);
    SLANG_CHECK(arrayData[2] == 0);

    auto arrayDefault = getDefaultBlob("ArrayDefault");
    SLANG_CHECK(arrayDefault->getBufferSize() == sizeof(int32_t) * 3);
    auto arrayDefaultData = (const int32_t*)arrayDefault->getBufferPointer();
    SLANG_CHECK(arrayDefaultData[0] == 0);
    SLANG_CHECK(arrayDefaultData[1] == 0);
    SLANG_CHECK(arrayDefaultData[2] == 0);

    ComPtr<slang::IBlob> largeArray;
    SLANG_CHECK(getDefaultBlobResult("LargeArray", largeArray) == SLANG_E_NOT_AVAILABLE);
    SLANG_CHECK(largeArray == nullptr);

    auto structPartial = getDefaultBlob("StructPartial");
    SLANG_CHECK(structPartial->getBufferSize() == sizeof(float) + sizeof(int32_t));
    auto structPartialBytes = (const uint8_t*)structPartial->getBufferPointer();
    SLANG_CHECK(*(const float*)(structPartialBytes) == 5.0f);
    SLANG_CHECK(*(const int32_t*)(structPartialBytes + sizeof(float)) == 0);

    auto structDefault = getDefaultBlob("StructDefault");
    SLANG_CHECK(structDefault->getBufferSize() == sizeof(float) + sizeof(int32_t));
    auto structDefaultBytes = (const uint8_t*)structDefault->getBufferPointer();
    SLANG_CHECK(*(const float*)(structDefaultBytes) == 2.0f);
    SLANG_CHECK(*(const int32_t*)(structDefaultBytes + sizeof(float)) == 0);

    auto derivedDefault = getDefaultBlob("DerivedDefault");
    SLANG_CHECK(derivedDefault->getBufferSize() == sizeof(float) * 2 + sizeof(int32_t));
    auto derivedDefaultBytes = (const uint8_t*)derivedDefault->getBufferPointer();
    auto derivedBaseData = (const float*)derivedDefaultBytes;
    SLANG_CHECK(derivedBaseData[0] == 7.0f);
    SLANG_CHECK(derivedBaseData[1] == 8.0f);
    SLANG_CHECK(*(const int32_t*)(derivedDefaultBytes + sizeof(float) * 2) == 9);

    auto derivedExplicit = getDefaultBlob("DerivedExplicit");
    SLANG_CHECK(derivedExplicit->getBufferSize() == sizeof(float) * 2 + sizeof(int32_t));
    auto derivedExplicitBytes = (const uint8_t*)derivedExplicit->getBufferPointer();
    auto derivedExplicitBaseData = (const float*)derivedExplicitBytes;
    SLANG_CHECK(derivedExplicitBaseData[0] == 7.0f);
    SLANG_CHECK(derivedExplicitBaseData[1] == 8.0f);
    SLANG_CHECK(*(const int32_t*)(derivedExplicitBytes + sizeof(float) * 2) == 3);

    auto enumValue = getDefaultBlob("EnumValue");
    SLANG_CHECK(enumValue->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)enumValue->getBufferPointer())[0] == 3);

    auto enumAlias = getDefaultBlob("EnumAlias");
    SLANG_CHECK(enumAlias->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)enumAlias->getBufferPointer())[0] == 3);

    auto enumCast = getDefaultBlob("EnumCast");
    SLANG_CHECK(enumCast->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)enumCast->getBufferPointer())[0] == 5);

    auto enumDefault = getDefaultBlob("EnumDefault");
    SLANG_CHECK(enumDefault->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)enumDefault->getBufferPointer())[0] == 0);

    auto byteEnumValue = getDefaultBlob("ByteEnumValue");
    SLANG_CHECK(byteEnumValue->getBufferSize() == sizeof(uint8_t));
    SLANG_CHECK(((const uint8_t*)byteEnumValue->getBufferPointer())[0] == 250);

    SLANG_CHECK_ABORT(topModeType->getFieldCount() == 2);
    auto enumCase = topModeType->getFieldByIndex(1);
    SLANG_CHECK_ABORT(enumCase != nullptr);
    ComPtr<slang::IBlob> enumCaseBlob;
    SLANG_CHECK(SLANG_SUCCEEDED(enumCase->getDefaultValueBlob(enumCaseBlob.writeRef())));
    SLANG_CHECK(enumCaseBlob->getBufferSize() == sizeof(int32_t));
    SLANG_CHECK(((const int32_t*)enumCaseBlob->getBufferPointer())[0] == 6);

    ComPtr<slang::IBlob> valueWithoutInitializer;
    SLANG_CHECK(SLANG_SUCCEEDED(getDefaultBlobResult("ValueWithoutInitializer", valueWithoutInitializer)));
    SLANG_CHECK(valueWithoutInitializer == nullptr);

    ComPtr<slang::IBlob> invalidArgBlob;
    SLANG_CHECK(
        spReflectionVariable_GetDefaultValueBlob(nullptr, invalidArgBlob.writeRef()) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        spReflectionVariable_GetDefaultValueBlob(
            (SlangReflectionVariable*)programLayout->findVarByNameInType(defaultsType, "ScalarFloat"),
            nullptr) == SLANG_E_INVALID_ARG);
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
