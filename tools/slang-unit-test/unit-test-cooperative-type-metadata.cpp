// unit-test-cooperative-type-metadata.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

static ComPtr<slang::ICooperativeTypesMetadata> _compileAndGetCooperativeMetadata(
    const char* source,
    const char* moduleName,
    const char* const* capabilityNames,
    int capabilityCount)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK(slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);

    if (globalSession->checkCompileTargetSupport(SLANG_SPIRV) != SLANG_OK)
    {
        SLANG_IGNORE_TEST;
    }

    List<slang::CompilerOptionEntry> capabilityOptions;
    for (int i = 0; i < capabilityCount; ++i)
    {
        auto cap = globalSession->findCapability(capabilityNames[i]);
        if (cap == SLANG_CAPABILITY_UNKNOWN)
        {
            SLANG_IGNORE_TEST;
        }

        slang::CompilerOptionEntry entry = {};
        entry.name = slang::CompilerOptionName::Capability;
        entry.value.kind = slang::CompilerOptionValueKind::Int;
        entry.value.intValue0 = int32_t(cap);
        capabilityOptions.add(entry);
    }

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = globalSession->findProfile("spirv_1_6");
    targetDesc.compilerOptionEntries = capabilityOptions.getBuffer();
    targetDesc.compilerOptionEntryCount = capabilityOptions.getCount();

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    SLANG_CHECK(globalSession->createSession(sessionDesc, session.writeRef()) == SLANG_OK);

    String fileNameBuf;
    fileNameBuf.append(moduleName);
    fileNameBuf.append(".slang");

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        moduleName,
        fileNameBuf.getBuffer(),
        source,
        diagnostics.writeRef());
    SLANG_CHECK(module != nullptr);

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_CHECK(
        module->findAndCheckEntryPoint(
            "computeMain",
            SLANG_STAGE_COMPUTE,
            entryPoint.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint};
    SLANG_CHECK(
        session->createCompositeComponentType(
            components,
            2,
            compositeProgram.writeRef(),
            diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_CHECK(
        compositeProgram->link(linkedProgram.writeRef(), diagnostics.writeRef()) == SLANG_OK);

    ComPtr<slang::IMetadata> metadata;
    SLANG_CHECK(
        linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnostics.writeRef()) ==
        SLANG_OK);

    ComPtr<slang::ICooperativeTypesMetadata> cooperativeMetadata;
    SLANG_CHECK(
        metadata->queryInterface(SLANG_IID_PPV_ARGS(cooperativeMetadata.writeRef())) == SLANG_OK);

    return cooperativeMetadata;
}

SLANG_UNIT_TEST(cooperativeMatrixTypeMetadata)
{
    const char* source = R"(
using namespace linalg;

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<int32_t> outputBufferInt;
RWStructuredBuffer<float> outputBufferWorkgroup;

[shader("compute")]
[numthreads(32,1,1)]
void computeMain()
{
    let d = coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2.0),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3.0)
    );
    d.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);

    let dInt = coopMatMulAdd<int32_t, false>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1),
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2),
        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3)
    );
    dInt.Store<CoopMatMatrixLayout::RowMajor>(outputBufferInt, 0, 16);

    let dIntSat = coopMatMulAdd<int32_t, true>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1),
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2),
        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3)
    );
    dIntSat.Store<CoopMatMatrixLayout::RowMajor>(outputBufferInt, 16, 16);

    let dWorkgroup = coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Workgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0),
        CoopMat<half, MemoryScope.Workgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2.0),
        CoopMat<float, MemoryScope.Workgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3.0)
    );
    dWorkgroup.Store<CoopMatMatrixLayout::RowMajor>(outputBufferWorkgroup, 0, 16);
}
)";

    const char* caps[] = {"spvCooperativeMatrixKHR"};
    auto cooperativeMatrixMetadata =
        _compileAndGetCooperativeMetadata(source, "coopTypeModule", caps, 1);

    constexpr int kExpectedTypeCount = 9;
    slang::CooperativeMatrixType expectedTypes[kExpectedTypeCount] = {
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_A},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_B},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_A},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_B},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_THREAD_GROUP,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_A},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_THREAD_GROUP,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_B},
        {.componentType = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
         .scope = SLANG_SCOPE_THREAD_GROUP,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR},
    };

    int foundExpectedTypeCounts[kExpectedTypeCount] = {};

    auto matrixTypeCount = cooperativeMatrixMetadata->getCooperativeMatrixTypeCount();
    SLANG_CHECK(matrixTypeCount >= SlangUInt(kExpectedTypeCount));

    for (SlangUInt i = 0; i < matrixTypeCount; ++i)
    {
        slang::CooperativeMatrixType type = {};
        SLANG_CHECK(
            cooperativeMatrixMetadata->getCooperativeMatrixTypeByIndex(i, &type) == SLANG_OK);
        SLANG_CHECK(type.componentType != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);

        for (int j = 0; j < kExpectedTypeCount; ++j)
        {
            if (type == expectedTypes[j])
                foundExpectedTypeCounts[j]++;
        }
    }

    for (int j = 0; j < kExpectedTypeCount; ++j)
        SLANG_CHECK(foundExpectedTypeCounts[j] == 1);

    constexpr int kExpectedCombinationCount = 4;
    slang::CooperativeMatrixCombination expectedCombinations[kExpectedCombinationCount] = {
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .componentTypeB = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .componentTypeC = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
         .componentTypeResult = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
         .saturate = false,
         .scope = SLANG_SCOPE_WAVE},
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .componentTypeB = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .componentTypeC = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .componentTypeResult = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .saturate = false,
         .scope = SLANG_SCOPE_WAVE},
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .componentTypeB = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .componentTypeC = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .componentTypeResult = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .saturate = true,
         .scope = SLANG_SCOPE_WAVE},
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .componentTypeB = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
         .componentTypeC = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
         .componentTypeResult = SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
         .saturate = false,
         .scope = SLANG_SCOPE_THREAD_GROUP},
    };

    int foundExpectedCombinationCounts[kExpectedCombinationCount] = {};

    auto combinationCount = cooperativeMatrixMetadata->getCooperativeMatrixCombinationCount();
    SLANG_CHECK(combinationCount >= SlangUInt(kExpectedCombinationCount));

    for (SlangUInt i = 0; i < combinationCount; ++i)
    {
        slang::CooperativeMatrixCombination combination = {};
        SLANG_CHECK(
            cooperativeMatrixMetadata->getCooperativeMatrixCombinationByIndex(i, &combination) ==
            SLANG_OK);
        SLANG_CHECK(combination.componentTypeA != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);
        SLANG_CHECK(combination.componentTypeB != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);
        SLANG_CHECK(combination.componentTypeC != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);
        SLANG_CHECK(combination.componentTypeResult != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);

        for (int j = 0; j < kExpectedCombinationCount; ++j)
        {
            if (combination == expectedCombinations[j])
                foundExpectedCombinationCounts[j]++;
        }
    }

    for (int j = 0; j < kExpectedCombinationCount; ++j)
        SLANG_CHECK(foundExpectedCombinationCounts[j] == 1);

    SLANG_CHECK(
        cooperativeMatrixMetadata->getCooperativeMatrixTypeByIndex(0, nullptr) ==
        SLANG_E_INVALID_ARG);

    slang::CooperativeMatrixType invalidType = {};
    SLANG_CHECK(
        cooperativeMatrixMetadata->getCooperativeMatrixTypeByIndex(matrixTypeCount, &invalidType) ==
        SLANG_E_INVALID_ARG);

    SLANG_CHECK(
        cooperativeMatrixMetadata->getCooperativeMatrixCombinationByIndex(0, nullptr) ==
        SLANG_E_INVALID_ARG);

    slang::CooperativeMatrixCombination invalidCombination = {};
    SLANG_CHECK(
        cooperativeMatrixMetadata->getCooperativeMatrixCombinationByIndex(
            combinationCount,
            &invalidCombination) == SLANG_E_INVALID_ARG);
}

SLANG_UNIT_TEST(cooperativeVectorTypeMetadata)
{
    const char* source = R"(
using namespace linalg;

RWStructuredBuffer<int32_t> outputBuffer;
ByteAddressBuffer input;
RWByteAddressBuffer matrix;
RWByteAddressBuffer bias;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    let vec4 = coopVecLoad<4, int8_t>(input);
    let vec8 = coopVecLoad<8, int8_t>(input);
    let packedVec = coopVecLoad<1, uint>(input);

    let resultA = coopVecMatMulAdd<int32_t, 4, 4>(
        vec4,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        bias,
        0,
        CoopVecComponentType::SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    let resultB = coopVecMatMulAdd<int32_t, 4, 8>(
        vec8,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        bias,
        0,
        CoopVecComponentType::SignedInt32,
        CoopVecMatrixLayout::RowMajor,
        true,
        8);

    let resultC = coopVecMatMul<int32_t, 4, 4>(
        vec4,
        CoopVecComponentType::SignedInt8,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    let resultPacked = coopVecMatMulPacked<int32_t, 4, 1>(
        packedVec,
        CoopVecComponentType::SignedInt8Packed,
        4,
        matrix,
        0,
        CoopVecComponentType::SignedInt8,
        CoopVecMatrixLayout::RowMajor,
        false,
        4);

    for (int i = 0; i < resultA.getCount(); ++i)
    {
        outputBuffer[i] = resultA[i] + resultB[i] + resultC[i] + resultPacked[i];
    }
}
)";

    const char* caps[] = {"spvCooperativeVectorNV"};
    auto cooperativeTypeMetadata =
        _compileAndGetCooperativeMetadata(source, "coopVectorTypeModule", caps, 1);

    auto vectorTypeCount = cooperativeTypeMetadata->getCooperativeVectorTypeCount();
    SLANG_CHECK(vectorTypeCount >= 3);

    int int8VectorTypeCount = 0;
    int int32VectorTypeCount = 0;
    int uint32VectorTypeCount = 0;

    for (SlangUInt i = 0; i < vectorTypeCount; ++i)
    {
        SlangCooperativeComponentType type = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
        SLANG_CHECK(cooperativeTypeMetadata->getCooperativeVectorTypeByIndex(i, &type) == SLANG_OK);
        SLANG_CHECK(type != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);

        if (type == SLANG_COOPERATIVE_COMPONENT_TYPE_INT8)
            int8VectorTypeCount++;
        if (type == SLANG_COOPERATIVE_COMPONENT_TYPE_INT32)
            int32VectorTypeCount++;
        if (type == SLANG_COOPERATIVE_COMPONENT_TYPE_UINT32)
            uint32VectorTypeCount++;
    }

    SLANG_CHECK(int8VectorTypeCount == 1);
    SLANG_CHECK(int32VectorTypeCount == 1);
    SLANG_CHECK(uint32VectorTypeCount == 1);

    constexpr int kExpectedCombinationCount = 4;
    slang::CooperativeVectorCombination expectedCombinations[kExpectedCombinationCount] = {
        {.inputType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .inputInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .matrixInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .biasInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .resultType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .transpose = false},
        {.inputType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .inputInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .matrixInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .biasInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .resultType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .transpose = true},
        {.inputType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .inputInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .matrixInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .biasInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE,
         .resultType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .transpose = false},
        {.inputType = SLANG_COOPERATIVE_COMPONENT_TYPE_UINT32,
         .inputInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8_PACKED,
         .matrixInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_INT8,
         .biasInterpretation = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE,
         .resultType = SLANG_COOPERATIVE_COMPONENT_TYPE_INT32,
         .transpose = false},
    };

    int foundExpectedCombinationCounts[kExpectedCombinationCount] = {};

    auto combinationCount = cooperativeTypeMetadata->getCooperativeVectorCombinationCount();
    SLANG_CHECK(combinationCount >= SlangUInt(kExpectedCombinationCount));

    for (SlangUInt i = 0; i < combinationCount; ++i)
    {
        slang::CooperativeVectorCombination combination = {};
        SLANG_CHECK(
            cooperativeTypeMetadata->getCooperativeVectorCombinationByIndex(i, &combination) ==
            SLANG_OK);
        SLANG_CHECK(combination.inputType != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);
        SLANG_CHECK(combination.inputInterpretation != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);
        SLANG_CHECK(combination.matrixInterpretation != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);
        SLANG_CHECK(combination.resultType != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);

        for (int j = 0; j < kExpectedCombinationCount; ++j)
        {
            if (combination == expectedCombinations[j])
                foundExpectedCombinationCounts[j]++;
        }
    }

    for (int j = 0; j < kExpectedCombinationCount; ++j)
    {
        SLANG_CHECK(foundExpectedCombinationCounts[j] == 1);
    }

    auto trainingTypeCount = cooperativeTypeMetadata->getCooperativeVectorTrainingTypeCount();
    SLANG_CHECK(trainingTypeCount == 0);

    SLANG_CHECK(
        cooperativeTypeMetadata->getCooperativeVectorTypeByIndex(0, nullptr) ==
        SLANG_E_INVALID_ARG);

    SlangCooperativeComponentType invalidVectorType = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
    SLANG_CHECK(
        cooperativeTypeMetadata->getCooperativeVectorTypeByIndex(
            vectorTypeCount,
            &invalidVectorType) == SLANG_E_INVALID_ARG);

    SLANG_CHECK(
        cooperativeTypeMetadata->getCooperativeVectorCombinationByIndex(0, nullptr) ==
        SLANG_E_INVALID_ARG);

    slang::CooperativeVectorCombination invalidCombination = {};
    SLANG_CHECK(
        cooperativeTypeMetadata->getCooperativeVectorCombinationByIndex(
            combinationCount,
            &invalidCombination) == SLANG_E_INVALID_ARG);

    SLANG_CHECK(
        cooperativeTypeMetadata->getCooperativeVectorTrainingTypeByIndex(0, nullptr) ==
        SLANG_E_INVALID_ARG);

    SlangCooperativeComponentType invalidTrainingType = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
    SLANG_CHECK(
        cooperativeTypeMetadata->getCooperativeVectorTrainingTypeByIndex(
            trainingTypeCount,
            &invalidTrainingType) == SLANG_E_INVALID_ARG);
}

SLANG_UNIT_TEST(cooperativeVectorTrainingTypeMetadata)
{
    const char* source = R"(
using namespace linalg;

ByteAddressBuffer input;
RWByteAddressBuffer matrix;
RWByteAddressBuffer output;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    let v = coopVecLoad<4, float>(input);

    coopVecOuterProductAccumulate<float, 4, 4>(
        v,
        v,
        matrix,
        0,
        8,
        CoopVecMatrixLayout::TrainingOptimal,
        CoopVecComponentType::Float16);

    coopVecReduceSumAccumulate<float, 4>(v, output, 0);
}
)";

    const char* caps[] = {"spvCooperativeVectorNV", "spvCooperativeVectorTrainingNV"};
    auto cooperativeTypeMetadata =
        _compileAndGetCooperativeMetadata(source, "coopVectorTrainingTypeModule", caps, 2);

    constexpr int kExpectedTrainingTypeCount = 2;
    SlangCooperativeComponentType expectedTrainingTypes[kExpectedTrainingTypeCount] = {
        SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT16,
        SLANG_COOPERATIVE_COMPONENT_TYPE_FLOAT32,
    };

    int foundExpectedTrainingTypeCounts[kExpectedTrainingTypeCount] = {};

    auto trainingTypeCount = cooperativeTypeMetadata->getCooperativeVectorTrainingTypeCount();
    SLANG_CHECK(trainingTypeCount >= SlangUInt(kExpectedTrainingTypeCount));

    for (SlangUInt i = 0; i < trainingTypeCount; ++i)
    {
        SlangCooperativeComponentType trainingType = SLANG_COOPERATIVE_COMPONENT_TYPE_NONE;
        SLANG_CHECK(
            cooperativeTypeMetadata->getCooperativeVectorTrainingTypeByIndex(i, &trainingType) ==
            SLANG_OK);
        SLANG_CHECK(trainingType != SLANG_COOPERATIVE_COMPONENT_TYPE_NONE);

        for (int j = 0; j < kExpectedTrainingTypeCount; ++j)
        {
            if (trainingType == expectedTrainingTypes[j])
                foundExpectedTrainingTypeCounts[j]++;
        }
    }

    for (int j = 0; j < kExpectedTrainingTypeCount; ++j)
    {
        SLANG_CHECK(foundExpectedTrainingTypeCounts[j] == 1);
    }
}

SLANG_UNIT_TEST(cooperativeTypeMetadataNoUsage)
{
    const char* source = R"(
[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
}
)";

    auto cooperativeTypeMetadata =
        _compileAndGetCooperativeMetadata(source, "coopTypeNoUsageModule", nullptr, 0);

    SLANG_CHECK(cooperativeTypeMetadata->getCooperativeMatrixTypeCount() == 0);
    SLANG_CHECK(cooperativeTypeMetadata->getCooperativeMatrixCombinationCount() == 0);
    SLANG_CHECK(cooperativeTypeMetadata->getCooperativeVectorTypeCount() == 0);
    SLANG_CHECK(cooperativeTypeMetadata->getCooperativeVectorCombinationCount() == 0);
    SLANG_CHECK(cooperativeTypeMetadata->getCooperativeVectorTrainingTypeCount() == 0);
}
