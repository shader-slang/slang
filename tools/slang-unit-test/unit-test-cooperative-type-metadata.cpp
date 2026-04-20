// unit-test-cooperative-type-metadata.cpp

#include "slang-com-ptr.h"
#include "slang.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Helper: compile shaderSource to SPIRV and return IMetadata (ref-counted).
// The caller can then castAs<ICooperativeTypesMetadata>() on the result.
// Returns null ComPtr if compilation fails.
static ComPtr<slang::IMetadata> _compileAndGetMetadata(
    const char* shaderSource,
    const char* entryPointName,
    SlangStage stage,
    SlangCompileTarget target = SLANG_SPIRV,
    const char* profileStr = "spirv_1_5")
{
    ComPtr<slang::IGlobalSession> globalSession;
    if (slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) != SLANG_OK)
        return {};

    slang::TargetDesc targetDesc = {};
    targetDesc.format = target;
    if (profileStr && target == SLANG_SPIRV)
        targetDesc.profile = globalSession->findProfile(profileStr);

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    ComPtr<slang::ISession> session;
    if (globalSession->createSession(sessionDesc, session.writeRef()) != SLANG_OK)
        return {};

    ComPtr<slang::IBlob> diagnosticBlob;
    auto module = session->loadModuleFromSourceString(
        "coopTestModule",
        "coopTestModule.slang",
        shaderSource,
        diagnosticBlob.writeRef());
    if (!module)
        return {};

    ComPtr<slang::IEntryPoint> entryPoint;
    module->findAndCheckEntryPoint(
        entryPointName,
        stage,
        entryPoint.writeRef(),
        diagnosticBlob.writeRef());
    if (!entryPoint)
        return {};

    ComPtr<slang::IComponentType> compositeProgram;
    slang::IComponentType* components[] = {module, entryPoint.get()};
    session->createCompositeComponentType(
        components,
        2,
        compositeProgram.writeRef(),
        diagnosticBlob.writeRef());
    if (!compositeProgram)
        return {};

    ComPtr<slang::IComponentType> linkedProgram;
    compositeProgram->link(linkedProgram.writeRef(), nullptr);
    if (!linkedProgram)
        return {};

    ComPtr<slang::IMetadata> metadata;
    linkedProgram->getTargetMetadata(0, metadata.writeRef(), diagnosticBlob.writeRef());
    return metadata;
}

// Plain compute shader with no cooperative operations.
static const char* kEmptyShaderSource = R"(
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {}
)";

// CoopMatMulAdd: half * half -> float, 16x16, Subgroup scope.
static const char* kCoopMatMulAddSource = R"(
using namespace linalg;
RWStructuredBuffer<float> outputBuffer;
[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0h),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(1.0h),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(0.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
)";

// CoopMatMulAdd called twice with identical types — deduplication test.
static const char* kCoopMatMulAddDupSource = R"(
using namespace linalg;
RWStructuredBuffer<float> outputBuffer;
[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0h),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(1.0h),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(0.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
    coopMatMulAdd<float, false>(
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(2.0h),
        CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(2.0h),
        CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
)";

// CoopVecMatMul (no bias): float16 input, float16 matrix, no bias.
// coopVecMatMul generates kIROp_CoopVecMatMulAdd without the optional bias operands.
static const char* kCoopVecMatMulAddNoBiasSource = R"(
ByteAddressBuffer input;
ByteAddressBuffer matrix;
RWStructuredBuffer<float16_t> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<float16_t, 4> vec = coopVecLoad<4, float16_t>(input);
    let result = coopVecMatMul<float16_t, 4, 4>(
        vec,
        CoopVecComponentType::Float16,
        matrix, 0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        false, 8
    );
    for (int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
)";

// CoopVecMatMulAdd with bias.
static const char* kCoopVecMatMulAddWithBiasSource = R"(
ByteAddressBuffer input;
ByteAddressBuffer matrix;
ByteAddressBuffer bias;
RWStructuredBuffer<float16_t> outputBuffer;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<float16_t, 4> vec = coopVecLoad<4, float16_t>(input);
    let result = coopVecMatMulAdd<float16_t, 4, 4>(
        vec,
        CoopVecComponentType::Float16,
        matrix, 0,
        CoopVecComponentType::Float16,
        bias, 0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        false, 8
    );
    for (int i = 0; i < result.getCount(); ++i)
        outputBuffer[i] = result[i];
}
)";

// CoopVecOuterProductAccumulate.
static const char* kCoopVecOuterProductSource = R"(
RWByteAddressBuffer output;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<half, 4> vecA;
    CoopVec<half, 8> vecB;
    for (int i = 0; i < vecA.getCount(); ++i)
        vecA[i] = half(i + 1);
    for (int i = 0; i < vecB.getCount(); ++i)
        vecB[i] = half(i + 1);
    coopVecOuterProductAccumulate(
        vecA, vecB, output, 0, 32,
        CoopVecMatrixLayout::TrainingOptimal,
        CoopVecComponentType::Float16
    );
}
)";

// CoopVecReduceSumAccumulate.
static const char* kCoopVecReduceSumSource = R"(
RWByteAddressBuffer output;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    CoopVec<half, 4> vec;
    for (int i = 0; i < vec.getCount(); ++i)
        vec[i] = half(i + 1);
    coopVecReduceSumAccumulate(vec, output, 0);
}
)";

SLANG_UNIT_TEST(cooperativeMetadata_emptyShader)
{
    auto metadata = _compileAndGetMetadata(kEmptyShaderSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    SLANG_CHECK(coopMeta->getCooperativeMatrixTypeCount() == 0);
    SLANG_CHECK(coopMeta->getCooperativeMatrixCombinationCount() == 0);
    SLANG_CHECK(coopMeta->getCooperativeVectorTypeCount() == 0);
    SLANG_CHECK(coopMeta->getCooperativeVectorCombinationCount() == 0);
}

SLANG_UNIT_TEST(cooperativeMetadata_castAs)
{
    auto metadata = _compileAndGetMetadata(kEmptyShaderSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    // castAs<ICooperativeTypesMetadata> must succeed
    void* coopMetaPtr = metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid());
    SLANG_CHECK(coopMetaPtr != nullptr);

    // castAs<IMetadata> on IMetadata* itself must also succeed
    void* metaPtr = metadata->castAs(slang::IMetadata::getTypeGuid());
    SLANG_CHECK(metaPtr != nullptr);
}

SLANG_UNIT_TEST(cooperativeMetadata_coopMatMulAdd_spirv)
{
    auto metadata =
        _compileAndGetMetadata(kCoopMatMulAddSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    // Should have at least 1 combination (half*half->float, 16x16x16, Subgroup)
    SLANG_CHECK(coopMeta->getCooperativeMatrixCombinationCount() >= 1);

    slang::CooperativeMatrixCombination combo;
    SLANG_CHECK(coopMeta->getCooperativeMatrixCombinationByIndex(0, &combo) == SLANG_OK);
    SLANG_CHECK(combo.m == 16);
    SLANG_CHECK(combo.n == 16);
    SLANG_CHECK(combo.k == 16);
    SLANG_CHECK(combo.componentTypeA == SLANG_SCALAR_TYPE_FLOAT16);
    SLANG_CHECK(combo.componentTypeB == SLANG_SCALAR_TYPE_FLOAT16);
    SLANG_CHECK(combo.componentTypeResult == SLANG_SCALAR_TYPE_FLOAT32);
    SLANG_CHECK(combo.scope == slang::MemoryScope::Subgroup);
    SLANG_CHECK(combo.saturate == false);

    // Matrix types should be non-empty
    SLANG_CHECK(coopMeta->getCooperativeMatrixTypeCount() > 0);
}

SLANG_UNIT_TEST(cooperativeMetadata_coopMat_deduplication)
{
    auto metadata =
        _compileAndGetMetadata(kCoopMatMulAddDupSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    // Two calls with identical types => still 1 combination (no duplicates)
    SLANG_CHECK(coopMeta->getCooperativeMatrixCombinationCount() == 1);
}

SLANG_UNIT_TEST(cooperativeMetadata_coopVecMatMulAdd_noBias)
{
    auto metadata =
        _compileAndGetMetadata(kCoopVecMatMulAddNoBiasSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    SLANG_CHECK(coopMeta->getCooperativeVectorCombinationCount() >= 1);

    slang::CooperativeVectorCombination combo;
    SLANG_CHECK(coopMeta->getCooperativeVectorCombinationByIndex(0, &combo) == SLANG_OK);
    SLANG_CHECK(combo.inputType == SLANG_SCALAR_TYPE_FLOAT16);
    SLANG_CHECK(combo.resultType == SLANG_SCALAR_TYPE_FLOAT16);
    SLANG_CHECK(combo.biasInterpretation == SLANG_SCALAR_TYPE_NONE); // no bias
    SLANG_CHECK(combo.inputPackingFactor == 1);
}

SLANG_UNIT_TEST(cooperativeMetadata_coopVecMatMulAdd_withBias)
{
    auto metadata =
        _compileAndGetMetadata(kCoopVecMatMulAddWithBiasSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    SLANG_CHECK(coopMeta->getCooperativeVectorCombinationCount() >= 1);

    slang::CooperativeVectorCombination combo;
    SLANG_CHECK(coopMeta->getCooperativeVectorCombinationByIndex(0, &combo) == SLANG_OK);
    SLANG_CHECK(combo.biasInterpretation != SLANG_SCALAR_TYPE_NONE); // bias present
}

SLANG_UNIT_TEST(cooperativeMetadata_coopVecTraining_outerProduct)
{
    auto metadata =
        _compileAndGetMetadata(kCoopVecOuterProductSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    // OuterProductAccumulate should set usedForTrainingOp=true on the vector type
    bool foundTraining = false;
    for (SlangUInt i = 0; i < coopMeta->getCooperativeVectorTypeCount(); ++i)
    {
        slang::CooperativeVectorType vt;
        SLANG_CHECK(coopMeta->getCooperativeVectorTypeByIndex(i, &vt) == SLANG_OK);
        if (vt.usedForTrainingOp)
        {
            foundTraining = true;
            // maxSize should be 0 for outer-product (no fixed vector width for driver queries)
            SLANG_CHECK(vt.maxSize == 0);
        }
    }
    SLANG_CHECK(foundTraining);
}

SLANG_UNIT_TEST(cooperativeMetadata_coopVecTraining_reduceSum)
{
    auto metadata =
        _compileAndGetMetadata(kCoopVecReduceSumSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    // ReduceSumAccumulate should set usedForTrainingOp=true
    bool foundTraining = false;
    for (SlangUInt i = 0; i < coopMeta->getCooperativeVectorTypeCount(); ++i)
    {
        slang::CooperativeVectorType vt;
        SLANG_CHECK(coopMeta->getCooperativeVectorTypeByIndex(i, &vt) == SLANG_OK);
        if (vt.usedForTrainingOp)
        {
            foundTraining = true;
            SLANG_CHECK(vt.maxSize > 0); // ReduceSum has a fixed vector width
        }
    }
    SLANG_CHECK(foundTraining);
}

SLANG_UNIT_TEST(cooperativeMetadata_outOfBoundsReturnsError)
{
    auto metadata = _compileAndGetMetadata(kEmptyShaderSource, "computeMain", SLANG_STAGE_COMPUTE);
    SLANG_CHECK(metadata != nullptr);
    if (!metadata)
        return;

    auto* coopMeta = static_cast<slang::ICooperativeTypesMetadata*>(
        metadata->castAs(slang::ICooperativeTypesMetadata::getTypeGuid()));
    SLANG_CHECK(coopMeta != nullptr);
    if (!coopMeta)
        return;

    // All lists are empty; index 0 should return SLANG_E_INVALID_ARG
    slang::CooperativeMatrixType matType;
    SLANG_CHECK(coopMeta->getCooperativeMatrixTypeByIndex(0, &matType) == SLANG_E_INVALID_ARG);

    slang::CooperativeMatrixCombination matCombo;
    SLANG_CHECK(
        coopMeta->getCooperativeMatrixCombinationByIndex(0, &matCombo) == SLANG_E_INVALID_ARG);

    slang::CooperativeVectorType vecType;
    SLANG_CHECK(coopMeta->getCooperativeVectorTypeByIndex(0, &vecType) == SLANG_E_INVALID_ARG);

    slang::CooperativeVectorCombination vecCombo;
    SLANG_CHECK(
        coopMeta->getCooperativeVectorCombinationByIndex(0, &vecCombo) == SLANG_E_INVALID_ARG);

    // Null outType pointer should also return SLANG_E_INVALID_ARG
    SLANG_CHECK(coopMeta->getCooperativeMatrixTypeByIndex(0, nullptr) == SLANG_E_INVALID_ARG);
}
