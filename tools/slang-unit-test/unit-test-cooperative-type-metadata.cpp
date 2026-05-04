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

template<typename T>
static bool _equals(T a, T b)
{
    return a == b;
}

static bool _equals(const slang::CooperativeMatrixType& a, const slang::CooperativeMatrixType& b)
{
    return a.componentType == b.componentType && a.scope == b.scope && a.rowCount == b.rowCount &&
           a.columnCount == b.columnCount && a.use == b.use;
}

static bool _equals(
    const slang::CooperativeMatrixCombination& a,
    const slang::CooperativeMatrixCombination& b)
{
    return a.m == b.m && a.n == b.n && a.k == b.k && a.componentTypeA == b.componentTypeA &&
           a.componentTypeB == b.componentTypeB && a.componentTypeC == b.componentTypeC &&
           a.componentTypeResult == b.componentTypeResult && a.saturate == b.saturate &&
           a.scope == b.scope;
}

static bool _equals(
    const slang::CooperativeVectorTypeUsageInfo& a,
    const slang::CooperativeVectorTypeUsageInfo& b)
{
    return a.componentType == b.componentType && a.maxSize == b.maxSize &&
           a.usedForTrainingOp == b.usedForTrainingOp;
}

static bool _equals(
    const slang::CooperativeVectorCombination& a,
    const slang::CooperativeVectorCombination& b)
{
    return a.inputType == b.inputType && a.inputInterpretation == b.inputInterpretation &&
           a.inputPackingFactor == b.inputPackingFactor &&
           a.matrixInterpretation == b.matrixInterpretation &&
           a.biasInterpretation == b.biasInterpretation && a.resultType == b.resultType &&
           a.transpose == b.transpose;
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
            if (_equals(actual, expected[j]))
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
    const slang::CooperativeVectorTypeUsageInfo* expectedTypes,
    int expectedTypeCount)
{
    auto typeCount = metadata->getCooperativeVectorTypeCount();
    _checkListContainsEachExpectedExactlyOnce<slang::CooperativeVectorTypeUsageInfo>(
        typeCount,
        [&](SlangUInt i)
        {
            slang::CooperativeVectorTypeUsageInfo type = {};
            SLANG_CHECK(metadata->getCooperativeVectorTypeByIndex(i, &type) == SLANG_OK);
            SLANG_CHECK(type.componentType != SLANG_SCALAR_TYPE_NONE);
            return type;
        },
        expectedTypes,
        expectedTypeCount);

    SLANG_CHECK(metadata->getCooperativeVectorTypeByIndex(0, nullptr) == SLANG_E_INVALID_ARG);

    slang::CooperativeVectorTypeUsageInfo invalidType = {};
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
};

// HLSL cooperative matrix requires shader model 6.10 and does not currently support the
// saturating-accumulation variant of `coopMatMulAdd`.
static const CooperativeMetadataTargetDesc kCooperativeMatrixHlslTargets[] = {
    {"hlsl", SLANG_HLSL, "sm_6_10", nullptr, 0},
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

#define COOPERATIVE_MATRIX_SUBGROUP_NON_SATURATING_OPS                                       \
    "    let d = coopMatMulAdd<float, false>(\n"                                             \
    "        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0),\n" \
    "        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2.0),\n" \
    "        CoopMat<float, MemoryScope.Subgroup, 16, 16, "                                  \
    "CoopMatMatrixUse::MatrixAccumulator>(3.0)\n"                                            \
    "    );\n"                                                                               \
    "    d.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);\n"                     \
    "\n"                                                                                     \
    "    let dInt = coopMatMulAdd<int32_t, false>(\n"                                        \
    "        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1),\n" \
    "        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2),\n" \
    "        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, "                                \
    "CoopMatMatrixUse::MatrixAccumulator>(3)\n"                                              \
    "    );\n"                                                                               \
    "    dInt.Store<CoopMatMatrixLayout::RowMajor>(outputBufferInt, 0, 16);\n"

static const char kCooperativeMatrixSubgroupSource[] = R"(
using namespace linalg;

RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<int32_t> outputBufferInt;

[shader("compute")]
[numthreads(32,1,1)]
void computeMain()
{
)" COOPERATIVE_MATRIX_SUBGROUP_NON_SATURATING_OPS R"(
    let dIntSat = coopMatMulAdd<int32_t, true>(
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1),
        CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2),
        CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3)
    );
    dIntSat.Store<CoopMatMatrixLayout::RowMajor>(outputBufferInt, 16, 16);
}
)";

static const char kCooperativeMatrixHlslSource[] = R"(
using namespace linalg;

RWByteAddressBuffer outputBuffer;
RWByteAddressBuffer outputBufferInt;

[shader("compute")]
[numthreads(32,1,1)]
void computeMain()
{
)" COOPERATIVE_MATRIX_SUBGROUP_NON_SATURATING_OPS R"(
}
)";

#undef COOPERATIVE_MATRIX_SUBGROUP_NON_SATURATING_OPS

static const slang::CooperativeMatrixType kExpectedCooperativeMatrixSubgroupTypes[] = {
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

static const slang::CooperativeMatrixCombination kExpectedCooperativeMatrixSubgroupCombinations[] =
    {
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

SLANG_UNIT_TEST(cooperativeMatrixSubgroupTypeMetadata)
{
    for (const auto& target : kCooperativeMatrixSubgroupTargets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(
            kCooperativeMatrixSubgroupSource,
            "coopMatrixSubgroupTypeModule",
            target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateMatrixMetadata(
            metadata,
            kExpectedCooperativeMatrixSubgroupTypes,
            int(SLANG_COUNT_OF(kExpectedCooperativeMatrixSubgroupTypes)),
            kExpectedCooperativeMatrixSubgroupCombinations,
            int(SLANG_COUNT_OF(kExpectedCooperativeMatrixSubgroupCombinations)));

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

SLANG_UNIT_TEST(cooperativeMatrixHlslTypeMetadata)
{
    for (const auto& target : kCooperativeMatrixHlslTargets)
    {
        auto metadata = _compileAndGetCooperativeMetadata(
            kCooperativeMatrixHlslSource,
            "coopMatrixHlslTypeModule",
            target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateMatrixMetadata(
            metadata,
            kExpectedCooperativeMatrixSubgroupTypes,
            int(SLANG_COUNT_OF(kExpectedCooperativeMatrixSubgroupTypes)),
            kExpectedCooperativeMatrixSubgroupCombinations,
            2);

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

    static const slang::CooperativeVectorTypeUsageInfo expectedTypes[] = {
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

    static const slang::CooperativeVectorTypeUsageInfo expectedTypes[] = {
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

    static const slang::CooperativeVectorTypeUsageInfo expectedTypes[] = {
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

// Two coopMatMulAdd operations with identical matrix type parameters must collapse to
// one matrix combination and three matrix types, exercising the sorted-unique insertion
// path for both the matrix-type and matrix-combination lists.
SLANG_UNIT_TEST(cooperativeMatrixMetadataDeduplication)
{
    const char* source = R"(
using namespace linalg;

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(32,1,1)]
void computeMain()
{
    let d1 = coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2.0),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(3.0)
    );
    d1.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);

    let d2 = coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(4.0),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(5.0),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(6.0)
    );
    d2.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 16, 16);
}
)";

    static const slang::CooperativeMatrixType expectedTypes[] = {
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
    };

    static const slang::CooperativeMatrixCombination expectedCombinations[] = {
        {.m = 16,
         .n = 16,
         .k = 16,
         .componentTypeA = SLANG_SCALAR_TYPE_FLOAT16,
         .componentTypeB = SLANG_SCALAR_TYPE_FLOAT16,
         .componentTypeC = SLANG_SCALAR_TYPE_FLOAT32,
         .componentTypeResult = SLANG_SCALAR_TYPE_FLOAT32,
         .saturate = false,
         .scope = SLANG_SCOPE_WAVE},
    };

    for (const auto& target : kCooperativeMatrixSubgroupTargets)
    {
        auto metadata =
            _compileAndGetCooperativeMetadata(source, "coopMatrixDeduplicationModule", target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateMatrixMetadata(
            metadata,
            expectedTypes,
            int(SLANG_COUNT_OF(expectedTypes)),
            expectedCombinations,
            int(SLANG_COUNT_OF(expectedCombinations)));

        SLANG_CHECK(metadata->getCooperativeVectorTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
    }
}

// Loading cooperative vectors of the same component type with several distinct widths
// must collapse to a single vector type-usage entry whose `maxSize` equals the largest width,
// exercising the size-merging path of `_insertOrUpdateCooperativeVectorTypeUsageInfo`.
SLANG_UNIT_TEST(cooperativeVectorTypeMaxSizeMerging)
{
    const char* source = R"(
using namespace linalg;

ByteAddressBuffer input;
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    let vec4 = coopVecLoad<4, float>(input);
    let vec8 = coopVecLoad<8, float>(input);
    let vec16 = coopVecLoad<16, float>(input);

    outputBuffer[0] = vec4[0];
    outputBuffer[1] = vec8[0];
    outputBuffer[2] = vec16[0];
}
)";

    static const slang::CooperativeVectorTypeUsageInfo expectedTypes[] = {
        {.componentType = SLANG_SCALAR_TYPE_FLOAT32, .maxSize = 16, .usedForTrainingOp = false},
    };

    for (const auto& target : kCooperativeVectorTargets)
    {
        auto metadata =
            _compileAndGetCooperativeMetadata(source, "coopVectorMaxSizeMergingModule", target);
        SLANG_CHECK(metadata != nullptr);
        if (!metadata)
            return;

        _validateVectorTypeMetadata(metadata, expectedTypes, int(SLANG_COUNT_OF(expectedTypes)));

        SLANG_CHECK(metadata->getCooperativeVectorCombinationCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixTypeCount() == 0);
        SLANG_CHECK(metadata->getCooperativeMatrixCombinationCount() == 0);
    }
}
