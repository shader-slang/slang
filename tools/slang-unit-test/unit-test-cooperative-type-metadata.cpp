// unit-test-cooperative-type-metadata.cpp

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

struct CooperativeMetadataTargetDesc
{
    const char* name;
    SlangCompileTarget target;
    const char* profileName;
    const char* const* capabilityNames;
    int capabilityCount;
};

static ComPtr<slang::ICooperativeTypesMetadata> _compileAndGetCooperativeMetadata(
    const char* source,
    const char* moduleNameBase,
    const CooperativeMetadataTargetDesc& target)
{
    ComPtr<slang::IGlobalSession> globalSession;
    SlangResult res = slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    res = globalSession->checkCompileTargetSupport(target.target);
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    List<slang::CompilerOptionEntry> capabilityOptions;
    for (int i = 0; i < target.capabilityCount; ++i)
    {
        auto cap = globalSession->findCapability(target.capabilityNames[i]);
        SLANG_CHECK(cap != SLANG_CAPABILITY_UNKNOWN);
        if (cap == SLANG_CAPABILITY_UNKNOWN)
            return nullptr;

        slang::CompilerOptionEntry entry = {};
        entry.name = slang::CompilerOptionName::Capability;
        entry.value.kind = slang::CompilerOptionValueKind::Int;
        entry.value.intValue0 = int32_t(cap);
        capabilityOptions.add(entry);
    }

    slang::TargetDesc targetDesc = {};
    targetDesc.format = target.target;
    if (target.profileName)
    {
        targetDesc.profile = globalSession->findProfile(target.profileName);
        SLANG_CHECK(targetDesc.profile != SLANG_PROFILE_UNKNOWN);
        if (targetDesc.profile == SLANG_PROFILE_UNKNOWN)
            return nullptr;
    }
    targetDesc.compilerOptionEntries = capabilityOptions.getBuffer();
    targetDesc.compilerOptionEntryCount = capabilityOptions.getCount();

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    res = globalSession->createSession(sessionDesc, session.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    String moduleName;
    moduleName.append(moduleNameBase);
    moduleName.append("_");
    moduleName.append(target.name);

    String fileName;
    fileName.append(moduleName);
    fileName.append(".slang");

    ComPtr<slang::IBlob> diagnostics;
    auto module = session->loadModuleFromSourceString(
        moduleName.getBuffer(),
        fileName.getBuffer(),
        source,
        diagnostics.writeRef());
    SLANG_CHECK(module != nullptr);
    if (!module)
        return nullptr;

    ComPtr<slang::IEntryPoint> entryPoint;
    res = module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint};
    res = session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnostics.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    ComPtr<slang::IComponentType> linkedProgram;
    res = compositeProgram->link(linkedProgram.writeRef(), diagnostics.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    ComPtr<slang::IMetadata> metadata;
    res = linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnostics.writeRef());
    SLANG_CHECK(res == SLANG_OK);
    if (res != SLANG_OK)
        return nullptr;

    auto ptr = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(ptr != nullptr);
    if (!ptr)
        return nullptr;

    return ComPtr<slang::ICooperativeTypesMetadata>(ptr);
}

template<typename T, typename F>
static void _checkListContainsEachExpectedExactlyOnce(
    SlangUInt actualCount,
    F getByIndex,
    const T* expected,
    int expectedCount)
{
    List<int> foundCounts;
    foundCounts.setCount(expectedCount);
    for (int i = 0; i < expectedCount; ++i)
        foundCounts[i] = 0;

    SLANG_CHECK(actualCount == SlangUInt(expectedCount));

    for (SlangUInt i = 0; i < actualCount; ++i)
    {
        T actual = getByIndex(i);
        for (int j = 0; j < expectedCount; ++j)
        {
            if (actual == expected[j])
                foundCounts[j]++;
        }
    }

    for (int j = 0; j < expectedCount; ++j)
        SLANG_CHECK(foundCounts[j] == 1);
}

static void _validateMatrixMetadata(
    slang::ICooperativeTypesMetadata* metadata,
    const slang::CooperativeMatrixType* expectedTypes,
    int expectedTypeCount,
    const slang::CooperativeMatrixCombination* expectedCombinations,
    int expectedCombinationCount)
{
    auto matrixTypeCount = metadata->getCooperativeMatrixTypeCount();
    _checkListContainsEachExpectedExactlyOnce<slang::CooperativeMatrixType>(
        matrixTypeCount,
        [&](SlangUInt i)
        {
            slang::CooperativeMatrixType type = {};
            SLANG_CHECK(metadata->getCooperativeMatrixTypeByIndex(i, &type) == SLANG_OK);
            SLANG_CHECK(type.componentType != SLANG_SCALAR_TYPE_NONE);
            return type;
        },
        expectedTypes,
        expectedTypeCount);

    auto combinationCount = metadata->getCooperativeMatrixCombinationCount();
    _checkListContainsEachExpectedExactlyOnce<slang::CooperativeMatrixCombination>(
        combinationCount,
        [&](SlangUInt i)
        {
            slang::CooperativeMatrixCombination combination = {};
            SLANG_CHECK(
                metadata->getCooperativeMatrixCombinationByIndex(i, &combination) == SLANG_OK);
            SLANG_CHECK(combination.componentTypeA != SLANG_SCALAR_TYPE_NONE);
            SLANG_CHECK(combination.componentTypeB != SLANG_SCALAR_TYPE_NONE);
            SLANG_CHECK(combination.componentTypeC != SLANG_SCALAR_TYPE_NONE);
            SLANG_CHECK(combination.componentTypeResult != SLANG_SCALAR_TYPE_NONE);
            return combination;
        },
        expectedCombinations,
        expectedCombinationCount);

    SLANG_CHECK(metadata->getCooperativeMatrixTypeByIndex(0, nullptr) == SLANG_E_INVALID_ARG);

    slang::CooperativeMatrixType invalidType = {};
    SLANG_CHECK(
        metadata->getCooperativeMatrixTypeByIndex(matrixTypeCount, &invalidType) ==
        SLANG_E_INVALID_ARG);

    SLANG_CHECK(
        metadata->getCooperativeMatrixCombinationByIndex(0, nullptr) == SLANG_E_INVALID_ARG);

    slang::CooperativeMatrixCombination invalidCombination = {};
    SLANG_CHECK(
        metadata->getCooperativeMatrixCombinationByIndex(combinationCount, &invalidCombination) ==
        SLANG_E_INVALID_ARG);
}

static void _validateVectorTypeMetadata(
    slang::ICooperativeTypesMetadata* metadata,
    const slang::CooperativeVectorType* expectedTypes,
    int expectedTypeCount)
{
    auto typeCount = metadata->getCooperativeVectorTypeCount();

    List<int> foundCounts;
    foundCounts.setCount(expectedTypeCount);
    for (int i = 0; i < expectedTypeCount; ++i)
        foundCounts[i] = 0;

    SLANG_CHECK(typeCount == SlangUInt(expectedTypeCount));

    for (SlangUInt i = 0; i < typeCount; ++i)
    {
        slang::CooperativeVectorType type = {};
        SLANG_CHECK(metadata->getCooperativeVectorTypeByIndex(i, &type) == SLANG_OK);
        SLANG_CHECK(type.componentType != SLANG_SCALAR_TYPE_NONE);

        int matchedIndex = -1;
        for (int j = 0; j < expectedTypeCount; ++j)
        {
            if (type.componentType == expectedTypes[j].componentType)
            {
                matchedIndex = j;
                break;
            }
        }

        SLANG_CHECK(matchedIndex != -1);
        if (matchedIndex != -1)
        {
            SLANG_CHECK(type.maxSize == expectedTypes[matchedIndex].maxSize);
            SLANG_CHECK(type.usedForTrainingOp == expectedTypes[matchedIndex].usedForTrainingOp);
            foundCounts[matchedIndex]++;
        }
    }

    for (int j = 0; j < expectedTypeCount; ++j)
        SLANG_CHECK(foundCounts[j] == 1);

    SLANG_CHECK(metadata->getCooperativeVectorTypeByIndex(0, nullptr) == SLANG_E_INVALID_ARG);

    slang::CooperativeVectorType invalidType = {};
    SLANG_CHECK(
        metadata->getCooperativeVectorTypeByIndex(typeCount, &invalidType) == SLANG_E_INVALID_ARG);
}

static void _validateVectorCombinationMetadata(
    slang::ICooperativeTypesMetadata* metadata,
    const slang::CooperativeVectorCombination* expectedCombinations,
    int expectedCombinationCount)
{
    auto combinationCount = metadata->getCooperativeVectorCombinationCount();

    _checkListContainsEachExpectedExactlyOnce<slang::CooperativeVectorCombination>(
        combinationCount,
        [&](SlangUInt i)
        {
            slang::CooperativeVectorCombination combination = {};
            SLANG_CHECK(
                metadata->getCooperativeVectorCombinationByIndex(i, &combination) == SLANG_OK);
            SLANG_CHECK(combination.inputType != SLANG_SCALAR_TYPE_NONE);
            SLANG_CHECK(combination.inputInterpretation != SLANG_SCALAR_TYPE_NONE);
            SLANG_CHECK(combination.matrixInterpretation != SLANG_SCALAR_TYPE_NONE);
            SLANG_CHECK(combination.resultType != SLANG_SCALAR_TYPE_NONE);
            return combination;
        },
        expectedCombinations,
        expectedCombinationCount);

    SLANG_CHECK(
        metadata->getCooperativeVectorCombinationByIndex(0, nullptr) == SLANG_E_INVALID_ARG);

    slang::CooperativeVectorCombination invalidCombination = {};
    SLANG_CHECK(
        metadata->getCooperativeVectorCombinationByIndex(combinationCount, &invalidCombination) ==
        SLANG_E_INVALID_ARG);
}

static const char* const kSpirvCoopMatCaps[] = {"spvCooperativeMatrixKHR"};
static const char* const kSpirvCoopVecCaps[] = {"spvCooperativeVectorNV"};
static const char* const kSpirvCoopVecTrainingCaps[] = {
    "spvCooperativeVectorNV",
    "spvCooperativeVectorTrainingNV"};
static const char* const kCudaOptixCoopVecCaps[] = {"optix_coopvec"};

static const CooperativeMetadataTargetDesc kCooperativeMatrixSubgroupTargets[] = {
    {"spirv", SLANG_SPIRV, "spirv_1_6", kSpirvCoopMatCaps, 1},
    {"cuda", SLANG_CUDA_SOURCE, nullptr, nullptr, 0},
};

static const CooperativeMetadataTargetDesc kCooperativeMatrixWorkgroupTargets[] = {
    {"spirv", SLANG_SPIRV, "spirv_1_6", kSpirvCoopMatCaps, 1},
};

static const CooperativeMetadataTargetDesc kCooperativeVectorTargets[] = {
    {"spirv", SLANG_SPIRV, "spirv_1_6", kSpirvCoopVecCaps, 1},
    {"hlsl", SLANG_HLSL, "sm_6_9", nullptr, 0},
    {"cuda_optix", SLANG_CUDA_SOURCE, nullptr, kCudaOptixCoopVecCaps, 1},
};

static const CooperativeMetadataTargetDesc kCooperativeVectorTrainingTargets[] = {
    {"spirv", SLANG_SPIRV, "spirv_1_6", kSpirvCoopVecTrainingCaps, 2},
    {"hlsl", SLANG_HLSL, "sm_6_9", nullptr, 0},
    {"cuda_optix", SLANG_CUDA_SOURCE, nullptr, kCudaOptixCoopVecCaps, 1},
};

// Cooperative vectors are lowered before metadata collection for plain CUDA targets.
// See the lowerCooperativeVectors() dispatch in source/slang/slang-emit.cpp.
static const CooperativeMetadataTargetDesc kCooperativeVectorLoweringTargets[] = {
    {"cuda", SLANG_CUDA_SOURCE, nullptr, nullptr, 0},
};

SLANG_UNIT_TEST(cooperativeMatrixSubgroupTypeMetadata)
{
    const char* subgroupSource = R"(
using namespace linalg;

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<int32_t> outputBufferInt;

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
}
)";

    static const slang::CooperativeMatrixType expectedSubgroupTypes[] = {
        {.componentType = SLANG_SCALAR_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_A},
        {.componentType = SLANG_SCALAR_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_B},
        {.componentType = SLANG_SCALAR_TYPE_FLOAT32,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR},
        {.componentType = SLANG_SCALAR_TYPE_INT8,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_A},
        {.componentType = SLANG_SCALAR_TYPE_INT8,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_B},
        {.componentType = SLANG_SCALAR_TYPE_INT32,
         .scope = SLANG_SCOPE_WAVE,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR},
    };

    static const slang::CooperativeMatrixCombination expectedSubgroupCombinations[] = {
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_SCALAR_TYPE_FLOAT16,
         .componentTypeB = SLANG_SCALAR_TYPE_FLOAT16,
         .componentTypeC = SLANG_SCALAR_TYPE_FLOAT32,
         .componentTypeResult = SLANG_SCALAR_TYPE_FLOAT32,
         .saturate = false,
         .scope = SLANG_SCOPE_WAVE},
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_SCALAR_TYPE_INT8,
         .componentTypeB = SLANG_SCALAR_TYPE_INT8,
         .componentTypeC = SLANG_SCALAR_TYPE_INT32,
         .componentTypeResult = SLANG_SCALAR_TYPE_INT32,
         .saturate = false,
         .scope = SLANG_SCOPE_WAVE},
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_SCALAR_TYPE_INT8,
         .componentTypeB = SLANG_SCALAR_TYPE_INT8,
         .componentTypeC = SLANG_SCALAR_TYPE_INT32,
         .componentTypeResult = SLANG_SCALAR_TYPE_INT32,
         .saturate = true,
         .scope = SLANG_SCOPE_WAVE},
    };

    for (const auto& target : kCooperativeMatrixSubgroupTargets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(
            subgroupSource,
            "coopMatrixSubgroupTypeModule",
            target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateMatrixMetadata(
            metadata,
            expectedSubgroupTypes,
            int(SLANG_COUNT_OF(expectedSubgroupTypes)),
            expectedSubgroupCombinations,
            int(SLANG_COUNT_OF(expectedSubgroupCombinations)));

        SLANG_CHECK(metadata->getCooperativeVectorTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
    }
}

SLANG_UNIT_TEST(cooperativeMatrixWorkgroupTypeMetadata)
{
    const char* workgroupSource = R"(
using namespace linalg;

RWStructuredBuffer<float> outputBufferWorkgroup;

[shader("compute")]
[numthreads(32,1,1)]
void computeMain()
{
    let dWorkgroup = coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Workgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0),
        CoopMat<half, MemoryScope.Workgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2.0),
        CoopMat<float, MemoryScope.Workgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3.0)
    );
    dWorkgroup.Store<CoopMatMatrixLayout::RowMajor>(outputBufferWorkgroup, 0, 16);
}
)";

    static const slang::CooperativeMatrixType expectedWorkgroupTypes[] = {
        {.componentType = SLANG_SCALAR_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_THREAD_GROUP,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_A},
        {.componentType = SLANG_SCALAR_TYPE_FLOAT16,
         .scope = SLANG_SCOPE_THREAD_GROUP,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_B},
        {.componentType = SLANG_SCALAR_TYPE_FLOAT32,
         .scope = SLANG_SCOPE_THREAD_GROUP,
         .rowCount = 16,
         .columnCount = 16,
         .use = SLANG_COOPERATIVE_MATRIX_USE_ACCUMULATOR},
    };

    static const slang::CooperativeMatrixCombination expectedWorkgroupCombinations[] = {
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_SCALAR_TYPE_FLOAT16,
         .componentTypeB = SLANG_SCALAR_TYPE_FLOAT16,
         .componentTypeC = SLANG_SCALAR_TYPE_FLOAT32,
         .componentTypeResult = SLANG_SCALAR_TYPE_FLOAT32,
         .saturate = false,
         .scope = SLANG_SCOPE_THREAD_GROUP},
    };

    for (const auto& target : kCooperativeMatrixWorkgroupTargets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(
            workgroupSource,
            "coopMatrixWorkgroupTypeModule",
            target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateMatrixMetadata(
            metadata,
            expectedWorkgroupTypes,
            int(SLANG_COUNT_OF(expectedWorkgroupTypes)),
            expectedWorkgroupCombinations,
            int(SLANG_COUNT_OF(expectedWorkgroupCombinations)));

        SLANG_CHECK(metadata->getCooperativeVectorTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
    }
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

    constexpr const CoopVecComponentType signedInt8 = CoopVecComponentType::SignedInt8;
    constexpr const CoopVecComponentType signedInt32 = CoopVecComponentType::SignedInt32;
    constexpr const CoopVecMatrixLayout rowMajor = CoopVecMatrixLayout::RowMajor;
    constexpr const bool noTranspose = false;

    let resultA = coopVecMatMulAdd<int32_t, 4, 4>(
        vec4,
        signedInt8,
        matrix,
        0,
        signedInt8,
        bias,
        0,
        signedInt32,
        rowMajor,
        noTranspose,
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

    static const slang::CooperativeVectorType expectedTypes[] = {
        {.componentType = SLANG_SCALAR_TYPE_INT8, .maxSize = 8, .usedForTrainingOp = false},
        {.componentType = SLANG_SCALAR_TYPE_INT32, .maxSize = 4, .usedForTrainingOp = false},
        {.componentType = SLANG_SCALAR_TYPE_UINT32, .maxSize = 1, .usedForTrainingOp = false},
    };

    static const slang::CooperativeVectorCombination expectedCombinations[] = {
        {.inputType = SLANG_SCALAR_TYPE_INT8,
         .inputInterpretation = SLANG_SCALAR_TYPE_INT8,
         .inputPackingFactor = 1,
         .matrixInterpretation = SLANG_SCALAR_TYPE_INT8,
         .biasInterpretation = SLANG_SCALAR_TYPE_INT32,
         .resultType = SLANG_SCALAR_TYPE_INT32,
         .transpose = false},
        {.inputType = SLANG_SCALAR_TYPE_INT8,
         .inputInterpretation = SLANG_SCALAR_TYPE_INT8,
         .inputPackingFactor = 1,
         .matrixInterpretation = SLANG_SCALAR_TYPE_INT8,
         .biasInterpretation = SLANG_SCALAR_TYPE_INT32,
         .resultType = SLANG_SCALAR_TYPE_INT32,
         .transpose = true},
        {.inputType = SLANG_SCALAR_TYPE_INT8,
         .inputInterpretation = SLANG_SCALAR_TYPE_INT8,
         .inputPackingFactor = 1,
         .matrixInterpretation = SLANG_SCALAR_TYPE_INT8,
         .biasInterpretation = SLANG_SCALAR_TYPE_NONE,
         .resultType = SLANG_SCALAR_TYPE_INT32,
         .transpose = false},
        {.inputType = SLANG_SCALAR_TYPE_UINT32,
         .inputInterpretation = SLANG_SCALAR_TYPE_INT8,
         .inputPackingFactor = 4,
         .matrixInterpretation = SLANG_SCALAR_TYPE_INT8,
         .biasInterpretation = SLANG_SCALAR_TYPE_NONE,
         .resultType = SLANG_SCALAR_TYPE_INT32,
         .transpose = false},
    };

    for (const auto& target : kCooperativeVectorTargets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(source, "coopVectorTypeModule", target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateVectorTypeMetadata(metadata, expectedTypes, int(SLANG_COUNT_OF(expectedTypes)));

        _validateVectorCombinationMetadata(
            metadata,
            expectedCombinations,
            int(SLANG_COUNT_OF(expectedCombinations)));

        SLANG_CHECK(metadata->getCooperativeMatrixTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixCombinationCount() == 0);
    }
}

SLANG_UNIT_TEST(cooperativeVectorTrainingMetadata)
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
    constexpr const CoopVecComponentType float16Type = CoopVecComponentType::Float16;

    coopVecOuterProductAccumulate<float, 4, 4>(
        v,
        v,
        matrix,
        0,
        8,
        CoopVecMatrixLayout::TrainingOptimal,
        float16Type);

    coopVecReduceSumAccumulate<float, 4>(v, output, 0);
}
)";

    static const slang::CooperativeVectorType expectedTypes[] = {
        {.componentType = SLANG_SCALAR_TYPE_FLOAT16, .maxSize = 0, .usedForTrainingOp = true},
        {.componentType = SLANG_SCALAR_TYPE_FLOAT32, .maxSize = 4, .usedForTrainingOp = true},
    };

    for (const auto& target : kCooperativeVectorTrainingTargets)
    {
        auto metadata =
            _compileAndGetCooperativeMetadata(source, "coopVectorTrainingTypeModule", target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateVectorTypeMetadata(metadata, expectedTypes, int(SLANG_COUNT_OF(expectedTypes)));

        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixCombinationCount() == 0);
    }
}

SLANG_UNIT_TEST(cooperativeVectorMixedTrainingAndNonTrainingMetadata)
{
    const char* source = R"(
using namespace linalg;

ByteAddressBuffer input;
RWByteAddressBuffer matrix;
RWByteAddressBuffer bias;
RWByteAddressBuffer reduceOutput;
RWStructuredBuffer<float> resultBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    let vec8 = coopVecLoad<8, float>(input);
    let vec4 = coopVecLoad<4, float>(input);

    let result = coopVecMatMulAdd<float, 4, 8>(
        vec8,
        CoopVecComponentType::Float32,
        matrix,
        0,
        CoopVecComponentType::Float32,
        bias,
        0,
        CoopVecComponentType::Float32,
        CoopVecMatrixLayout::RowMajor,
        false,
        8);

    coopVecReduceSumAccumulate<float, 4>(vec4, reduceOutput, 0);

    for (int i = 0; i < result.getCount(); ++i)
    {
        resultBuffer[i] = result[i];
    }
}
)";

    static const slang::CooperativeVectorType expectedTypes[] = {
        {.componentType = SLANG_SCALAR_TYPE_FLOAT32, .maxSize = 8, .usedForTrainingOp = true},
    };

    static const slang::CooperativeVectorCombination expectedCombinations[] = {
        {.inputType = SLANG_SCALAR_TYPE_FLOAT32,
         .inputInterpretation = SLANG_SCALAR_TYPE_FLOAT32,
         .inputPackingFactor = 1,
         .matrixInterpretation = SLANG_SCALAR_TYPE_FLOAT32,
         .biasInterpretation = SLANG_SCALAR_TYPE_FLOAT32,
         .resultType = SLANG_SCALAR_TYPE_FLOAT32,
         .transpose = false},
    };

    for (const auto& target : kCooperativeVectorTrainingTargets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(
            source,
            "coopVectorMixedTrainingNonTrainingTypeModule",
            target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateVectorTypeMetadata(metadata, expectedTypes, int(SLANG_COUNT_OF(expectedTypes)));
        _validateVectorCombinationMetadata(
            metadata,
            expectedCombinations,
            int(SLANG_COUNT_OF(expectedCombinations)));

        SLANG_CHECK(metadata->getCooperativeMatrixTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixCombinationCount() == 0);
    }
}

SLANG_UNIT_TEST(cooperativeMetadataLoweredVectorTarget)
{
    const char* source = R"(
using namespace linalg;

ByteAddressBuffer input;
RWByteAddressBuffer matrix;
RWByteAddressBuffer bias;
RWStructuredBuffer<float> resultBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    let vec8 = coopVecLoad<8, float>(input);

    let result = coopVecMatMulAdd<float, 4, 8>(
        vec8,
        CoopVecComponentType::Float32,
        matrix,
        0,
        CoopVecComponentType::Float32,
        bias,
        0,
        CoopVecComponentType::Float32,
        CoopVecMatrixLayout::RowMajor,
        false,
        8);

    for (int i = 0; i < result.getCount(); ++i)
    {
        resultBuffer[i] = result[i];
    }
}
)";

    for (const auto& target : kCooperativeVectorLoweringTargets)
    {
        auto metadata =
            _compileAndGetCooperativeMetadata(source, "coopLoweredVectorTargetModule", target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        SLANG_CHECK(metadata->getCooperativeMatrixTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixCombinationCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
    }
}

SLANG_UNIT_TEST(cooperativeMetadataEmptyShader)
{
    const char* source = R"(
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    outputBuffer[0] = 1.0f;
}
)";

    static const CooperativeMetadataTargetDesc targets[] = {
        {"spirv", SLANG_SPIRV, "spirv_1_6", kSpirvCoopMatCaps, 1},
        {"hlsl", SLANG_HLSL, "sm_6_9", nullptr, 0},
    };

    for (const auto& target : targets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(source, "coopEmptyModule", target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        SLANG_CHECK(metadata->getCooperativeMatrixTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixCombinationCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
    }
}
