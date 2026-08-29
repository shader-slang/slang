#pragma once

// Shared implementation support for the decomposed NVVM unit-test translation units.

#include "compiler-core/slang-artifact-representation.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "compiler-core/slang-nvvm-compiler.h"
#include "compiler-core/slang-nvvm-ir-builder.h"
#include "compiler-core/slang-nvvm-semantic-catalog.h"
#include "core/slang-blob.h"
#include "core/slang-io.h"
#include "core/slang-process-util.h"
#include "core/slang-shared-library.h"
#include "cuda-driver-test-util.h"
#include "scoped-env-var.h"
#include "slang-com-helper.h"
#include "slang-com-ptr.h"
#include "slang.h"
#include "slang/slang-compiler.h"
#include "unit-test-nvvm-bitcode-fixture.h"
#include "unit-test/slang-unit-test.h"

#include <math.h>
#include <string.h>

using namespace Slang;

namespace
{

using namespace Slang::TestCUDA;

// These declarations mirror the small public libNVVM C ABI used by the production implementation.
// Keeping the fake independent of CUDA headers verifies that libNVVM remains an optional build
// dependency.
enum class TestNVVMResult
{
    Success = 0,
    OutOfMemory = 1,
    ProgramCreationFailure = 2,
    IRVersionMismatch = 3,
    InvalidInput = 4,
    InvalidProgram = 5,
    InvalidIR = 6,
    InvalidOption = 7,
    NoModuleInProgram = 8,
    Compilation = 9,
    Cancelled = 10,
};

struct TestNVVMProgramStorage
{
    int unused;
};
typedef TestNVVMProgramStorage* TestNVVMProgram;

enum class FakeFailure
{
    None,
    CreateProgram,
    AddModule,
    LazyAddModule,
    EagerAddModule,
    VerifyProgram,
    CompileProgram,
    GetResultSize,
    GetResult,
    GetLogSize,
    GetLog,
};

enum class FakeModuleAddKind
{
    Normal,
    Lazy,
};

enum class FakeLogPhase
{
    General,
    Verifier,
    Compiler,
};

enum class FakeResultMode
{
    NullTerminated,
    TerminatorOnly,
    Unterminated,
};

static const char* const kRequiredSymbols[] = {
    "nvvmGetErrorString",
    "nvvmVersion",
    "nvvmIRVersion",
    "nvvmCreateProgram",
    "nvvmDestroyProgram",
    "nvvmAddModuleToProgram",
    "nvvmVerifyProgram",
    "nvvmCompileProgram",
    "nvvmGetCompiledResultSize",
    "nvvmGetCompiledResult",
    "nvvmGetProgramLogSize",
    "nvvmGetProgramLog",
};

static const char kMinimalNVVMIR[] =
    "target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
    "i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-"
    "v128:128:128-n16:32:64\"\n"
    "target triple = \"nvptx64-nvidia-cuda\"\n"
    "\n"
    "define void @testEmpty() {\n"
    "entry:\n"
    "  ret void\n"
    "}\n"
    "\n"
    "!nvvmir.version = !{!0}\n"
    "!nvvm.annotations = !{!1}\n"
    "!0 = !{i32 2, i32 0}\n"
    "!1 = !{void ()* @testEmpty, !\"kernel\", i32 1}\n";

static const char kLibdeviceSineKernelName[] = "libdeviceSine";
static const char kLibdeviceSineNVVMIR[] =
    "target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
    "i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-"
    "v128:128:128-n16:32:64\"\n"
    "target triple = \"nvptx64-nvidia-cuda\"\n"
    "\n"
    "declare float @__nv_sinf(float)\n"
    "\n"
    "define void @libdeviceSine(float addrspace(1)* %destination, float %x) {\n"
    "entry:\n"
    "  %result = call float @__nv_sinf(float %x)\n"
    "  store float %result, float addrspace(1)* %destination, align 4\n"
    "  ret void\n"
    "}\n"
    "\n"
    "!nvvmir.version = !{!0}\n"
    "!nvvm.annotations = !{!1}\n"
    "!0 = !{i32 2, i32 0}\n"
    "!1 = !{void (float addrspace(1)*, float)* @libdeviceSine, !\"kernel\", i32 1}\n";

static const char kFakePTX[] = ".version 7.5\n"
                               ".target sm_75\n"
                               ".address_size 64\n"
                               ".visible .entry testEmpty()\n"
                               "{\n"
                               "    ret;\n"
                               "}\n";

static const char kFakeDirectPTX[] = ".version 7.5\n"
                                     ".target sm_70\n"
                                     ".address_size 64\n"
                                     ".visible .entry computeMain()\n"
                                     "{\n"
                                     "    ret;\n"
                                     "}\n";

struct FakeNVVMBuilderModuleStorage
{
};
struct FakeNVVMBuilderVoidTypeStorage
{
};
struct FakeNVVMBuilderFunctionTypeStorage
{
};
struct FakeNVVMBuilderFunctionStorage
{
};
struct FakeNVVMBuilderBlockStorage
{
};
struct FakeNVVMBuilderIntegerTypeStorage
{
};
struct FakeNVVMBuilderBooleanTypeStorage
{
};
struct FakeNVVMBuilderFloatTypeStorage
{
};
struct FakeNVVMBuilderHalfTypeStorage
{
};
struct FakeNVVMBuilderPointerTypeStorage
{
};
struct FakeNVVMBuilderFloatPointerTypeStorage
{
};
struct FakeNVVMBuilderArrayTypeStorage
{
};
struct FakeNVVMBuilderArrayPointerTypeStorage
{
};
struct FakeNVVMBuilderVectorTypeStorage
{
};
struct FakeNVVMBuilderStructTypeStorage
{
};
struct FakeNVVMBuilderScalarStructTypeStorage
{
};
struct FakeNVVMBuilderScalarStructPointerTypeStorage
{
};
struct FakeNVVMBuilderParameterStorage
{
};
struct FakeNVVMBuilderLoadStorage
{
};
struct FakeNVVMBuilderIntegerConstantStorage
{
};
struct FakeNVVMBuilderFloatingPointConstantStorage
{
};
struct FakeNVVMBuilderScalarPhiStorage
{
};
struct FakeNVVMBuilderCallStorage
{
};
struct FakeNVVMBuilderPointerOffsetStorage
{
};
struct FakeNVVMBuilderByteOffsetPointerStorage
{
};
struct FakeNVVMBuilderSequentialElementPointerStorage
{
};
struct FakeNVVMBuilderStructFieldPointerStorage
{
};
struct FakeNVVMBuilderAggregateElementStorage
{
};
struct FakeNVVMBuilderAggregateConstructStorage
{
};
struct FakeNVVMBuilderScalarOperationStorage
{
};
struct FakeNVVMBuilderIntrinsicStorage
{
};
struct FakeNVVMBuilderSurfaceOperationStorage
{
};
struct FakeNVVMBuilderTextureOperationStorage
{
};
struct FakeNVVMBuilderRelaxedGlobalI32AtomicAddStorage
{
};
struct FakeNVVMBuilderResourceViewTypeStorage
{
};
struct FakeNVVMBuilderExecutionRegisterStorage
{
};
struct FakeNVVMBuilderVectorElementStorage
{
};
struct FakeNVVMBuilderVectorConstructStorage
{
};
struct FakeNVVMBuilderGlobalStorage
{
};
struct FakeNVVMBuilderLocalStorage
{
};

enum class FakeNVVMBuilderValueKind
{
    Parameter,
    Load,
    ScalarOperation,
    Intrinsic,
    SurfaceOperation,
    TextureOperation,
    IntegerConstant,
    FloatingPointConstant,
    ScalarPhi,
    Call,
    PointerOffset,
    ByteOffsetPointer,
    SequentialElementPointer,
    StructFieldPointer,
    AggregateElement,
    AggregateConstruct,
    RelaxedGlobalI32AtomicAdd,
    ExecutionRegister,
    VectorConstruct,
    VectorElement,
    GlobalStorage,
    LocalStorage,
};

enum class FakeNVVMBuilderScalarFamily : uint32_t
{
    Unary,
    Binary,
    Compare,
    FloatingUnary,
    FloatingBinary,
    FloatingCompare,
    Select,
    Count,
};

struct FakeNVVMBuilderScalarOperationKey
{
    FakeNVVMBuilderScalarFamily family = FakeNVVMBuilderScalarFamily::Count;
    uint32_t operation = 0;
};

struct FakeNVVMBuilderValueRef
{
    FakeNVVMBuilderValueKind kind = FakeNVVMBuilderValueKind::Parameter;
    Index index = -1;
    Index functionIndex = -1;
};

struct FakeNVVMBuilderScalarOperation
{
    FakeNVVMBuilderScalarOperationKey key;
    SlangNVVMValueTypeDesc resultType = {};
    SlangNVVMValueTypeDesc operandTypes[3] = {};
    Index callerBlockIndex = -1;
    FakeNVVMBuilderValueRef operands[3];
    uint32_t operandCount = 0;
};

enum class FakeNVVMBuilderResultTypeKind
{
    Void,
    Integer,
    Boolean,
    Half,
    Float,
    ValueVector,
    ScalarStruct,
};

enum class FakeNVVMBuilderParameterTypeKind
{
    Integer,
    Boolean,
    Pointer,
    Half,
    Float,
    FloatPointer,
    ArrayPointer,
    ScalarStructPointer,
    ScalarStruct,
    ResourceView,
    ValueVector,
    NumericArray,
};

enum class FakeNVVMBuilderScalarTypeKind
{
    Integer,
    Boolean,
    Float,
    UInt2,
    UInt3,
    UInt4,
    Float2,
    Float3,
    Float4,
    NumericArray,
    NumericArrayPointer,
    ResourceView,
    ScalarStructPointer,
    ScalarStruct,
    Half,
    Half2,
    Half3,
    Half4,
    Count,
};

static const char kFakeNVVMBuilderAssembly[] = "fake LLVM assembly";
static const uint8_t kFakeNVVMBuilderBitcode[] = {0x42, 0x43, 0xc0, 0xde, 0x00, 0x11};

struct FakeNVVMBuilderState
{
    void resetCalls()
    {
        createModuleCallCount = 0;
        destroyModuleCallCount = 0;
        getVoidTypeCallCount = 0;
        getFunctionTypeCallCount = 0;
        declareFunctionCallCount = 0;
        declareGlobalStorageCallCount = 0;
        createBlockCallCount = 0;
        setInsertBlockCallCount = 0;
        emitReturnVoidCallCount = 0;
        markFunctionAsKernelCallCount = 0;
        serializeQueryCallCount = 0;
        serializeWriteCallCount = 0;
        serializeWithDiagnosticsQueryCallCount = 0;
        serializeWithDiagnosticsWriteCallCount = 0;
        getIntegerTypeCallCount = 0;
        getFloatingPointTypeCallCount = 0;
        getArrayTypeCallCount = 0;
        getVectorTypeCallCount = 0;
        getStructTypeCallCount = 0;
        getPointerTypeCallCount = 0;
        getFunctionParameterCallCount = 0;
        setFunctionParameterAttributesCallCount = 0;
        emitLoadCallCount = 0;
        emitStoreCallCount = 0;
        emitLocalStorageCallCount = 0;
        emitBranchCallCount = 0;
        emitConditionalBranchCallCount = 0;
        emitSwitchCallCount = 0;
        lastSwitchCaseCount = 0;
        getIntegerConstantCallCount = 0;
        getFloatingPointConstantCallCount = 0;
        emitPhiCallCount = 0;
        addPhiIncomingCallCount = 0;
        emitIntegerCallCallCount = 0;
        emitIntegerReturnCallCount = 0;
        emitCallCallCount = 0;
        emitValueReturnCallCount = 0;
        emitIntrinsicCallCount = 0;
        isOperationSupportedCallCount = 0;
        emitPointerOffsetCallCount = 0;
        emitByteOffsetPointerCallCount = 0;
        emitSequentialElementPointerCallCount = 0;
        emitStructFieldPointerCallCount = 0;
        emitAggregateElementExtractCallCount = 0;
        emitAggregateConstructCallCount = 0;
        emitRelaxedGlobalI32AtomicAddCallCount = 0;
        emitVectorConstructCallCount = 0;
        emitSequentialElementExtractCallCount = 0;
        workgroupBarrierCallCount = 0;
        deviceMemoryBarrierCallCount = 0;
        for (Index family = 0; family < Index(FakeNVVMBuilderScalarFamily::Count); ++family)
        {
            scalarFamilyCallCounts[family] = 0;
            valueOperationFamilyCallCounts[family] = 0;
            for (Index operation = 0; operation < SLANG_COUNT_OF(scalarOperationCallCounts[0]);
                 ++operation)
            {
                scalarOperationCallCounts[family][operation] = 0;
            }
        }
        integerBitWidth = 0;
        floatingPointBitWidth = 0;
        arrayElementCount = 0;
        arrayElementType = nullptr;
        vectorElementType = nullptr;
        vectorElementCount = 0;
        structFieldTypes.clear();
        scalarStructFieldTypes.clear();
        globalStorageValueType = nullptr;
        globalStorageLinkage = SLANG_NVVM_LINKAGE_INTERNAL;
        globalStorageAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
        globalStorageAlignment = 0;
        globalStorageNames.clear();
        localStorageValueTypes.clear();
        localStorageAlignments.clear();
        localStorageNames.clear();
        pointerAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
        pointerPointeeTypes.clear();
        pointerAddressSpaces.clear();
        functionParameterIndex = 0;
        functionParameterCount = 0;
        functionParameterTypeKinds.clear();
        functionParameterTypes.clear();
        functionTypeResultKinds.clear();
        functionTypeResultTypes.clear();
        functionTypeParameterCounts.clear();
        functionTypeParameterKindOffsets.clear();
        functionNames.clear();
        functionLinkages.clear();
        functionFlags.clear();
        parameterAttributeFunctionIndices.clear();
        parameterAttributeIndices.clear();
        parameterAttributeFlags.clear();
        parameterAttributePointeeTypes.clear();
        parameterAttributeAlignments.clear();
        functionTypeIndices.clear();
        blockFunctionIndices.clear();
        loadAlignment = 0;
        storeAlignment = 0;
        loadAlignments.clear();
        storeAlignments.clear();
        scalarOperations.clear();
        emittedValueOperations.clear();
        surfaceOperations.clear();
        textureOperations.clear();
        intrinsicOperations.clear();
        intrinsicResultTypes.clear();
        intrinsicCallerBlockIndices.clear();
        intrinsicArgumentOffsets.clear();
        intrinsicArgumentCounts.clear();
        intrinsicArgumentValueRefs.clear();
        integerConstantValues.clear();
        integerConstantBitWidths.clear();
        floatingPointConstantBitWidths.clear();
        floatingPointConstantBitPatterns.clear();
        scalarPhiTargetBlockIndices.clear();
        scalarPhiTypes.clear();
        scalarPhiIncomingPhiIndices.clear();
        scalarPhiIncomingValueRefs.clear();
        scalarPhiIncomingPredecessorBlockIndices.clear();
        functionParameterIndices.clear();
        loadPointerParameterIndices.clear();
        loadFlags.clear();
        storePointerFunctionIndices.clear();
        storePointerParameterIndices.clear();
        storeValueKinds.clear();
        storeValueParameterIndices.clear();
        storeValueBinaryIndices.clear();
        storeValueRefs.clear();
        storeBlockIndices.clear();
        branchSourceBlockIndices.clear();
        branchTargetBlockIndices.clear();
        callCalleeFunctionIndices.clear();
        callCallerBlockIndices.clear();
        callArgumentOffsets.clear();
        callArgumentCounts.clear();
        callArgumentValueRefs.clear();
        callResultKinds.clear();
        callResultTypes.clear();
        integerReturnBlockIndices.clear();
        integerReturnValueRefs.clear();
        scalarReturnBlockIndices.clear();
        scalarReturnValueRefs.clear();
        pointerOffsetCallerBlockIndices.clear();
        pointerOffsetBaseValueRefs.clear();
        pointerOffsetElementValueRefs.clear();
        byteOffsetPointerCallerBlockIndices.clear();
        byteOffsetPointerBaseValueRefs.clear();
        byteOffsetPointerOffsetValueRefs.clear();
        byteOffsetPointerPointeeTypes.clear();
        byteOffsetPointerTypeKinds.clear();
        sequentialElementPointerCallerBlockIndices.clear();
        sequentialElementPointerBaseValueRefs.clear();
        sequentialElementPointerIndexValueRefs.clear();
        sequentialElementPointerTypeKinds.clear();
        structFieldPointerBaseValueRefs.clear();
        structFieldPointerIndices.clear();
        structFieldPointerTypeKinds.clear();
        aggregateElementBaseValueRefs.clear();
        aggregateElementIndices.clear();
        aggregateElementTypeKinds.clear();
        aggregateElementIsFirstClassValue.clear();
        aggregateConstructResultTypes.clear();
        aggregateConstructElementOffsets.clear();
        aggregateConstructElementCounts.clear();
        aggregateConstructElementValueRefs.clear();
        relaxedGlobalI32AtomicAddCallerBlockIndices.clear();
        relaxedGlobalI32AtomicAddPointerValueRefs.clear();
        relaxedGlobalI32AtomicAddValueRefs.clear();
        loadPointerValueRefs.clear();
        loadResultTypeKinds.clear();
        storePointerValueRefs.clear();
        kernelFunctionIndices.clear();
        executionRegisterOperations.clear();
        executionRegisterCallerBlockIndices.clear();
        vectorConstructResultTypes.clear();
        vectorConstructElementOffsets.clear();
        vectorConstructElementCounts.clear();
        vectorConstructElementValueRefs.clear();
        vectorElementBaseValueRefs.clear();
        vectorElementIndexValueRefs.clear();
        vectorElementIndices.clear();
        vectorElementTypeKinds.clear();
        currentInsertBlockIndex = -1;
        conditionalSourceBlockIndex = -1;
        conditionalTrueBlockIndex = -1;
        conditionalFalseBlockIndex = -1;
        moduleName = String();
        functionName = String();
        blockName = String();
    }

    void reset()
    {
        SLANG_ASSERT(liveLibraryCount == 0);
        api = {};
        foundation = {};
        construction = {};
        valueOperations = {};
        surfaceOperationsAPI = {};
        textureOperationsAPI = {};
        acceptedABIRevision = SLANG_NVVM_BUILDER_ABI_REVISION;
        omittedInterface = SlangNVVMBuilderInterfaceID(~uint32_t(0));
        omitAPISymbol = false;
        libraryUnavailable = false;
        returnNullModule = false;
        returnNullIntegerType = false;
        returnNullFloatingPointType = false;
        returnNullArrayType = false;
        returnNullSequentialElementPointer = false;
        returnNullGlobalStorage = false;
        returnNullScalarOperation = {};
        returnNullRelaxedGlobalI32AtomicAdd = false;
        failIntegerTypeAfterWrite = false;
        failFloatingPointTypeAfterWrite = false;
        failArrayTypeAfterWrite = false;
        failGlobalStorageAfterWrite = false;
        failIntegerConstantAfterWrite = false;
        failFloatingPointConstantAfterWrite = false;
        failScalarPhiAfterWrite = false;
        failIntegerCallAfterWrite = false;
        failIntegerReturn = false;
        failCallAfterWrite = false;
        failValueReturn = false;
        returnNullIntrinsic = false;
        failIntrinsicAfterWrite = false;
        rejectValueOperation = false;
        rejectedValueOperation = 0;
        rejectedValueOperationResultType = {};
        rejectedValueOperationOperandCount = 0;
        for (auto& operandType : rejectedValueOperationOperandTypes)
            operandType = {};
        failPointerOffsetAfterWrite = false;
        failByteOffsetPointerAfterWrite = false;
        failSequentialElementPointerAfterWrite = false;
        failScalarOperationAfterWrite = {};
        failRelaxedGlobalI32AtomicAddAfterWrite = false;
        reportMismatchedWriteSize = false;
        verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
        serializationWithDiagnosticsResult = SLANG_OK;
        verificationDiagnostic = String();
        omitValidSerializedOutput = false;
        reportMismatchedSerializedDiagnosticWriteSize = false;
        reportMismatchedVerificationDiagnosticWriteSize = false;
        reportMismatchedVerificationStatus = false;
        loadedPath = String();
        loadRequestCount = 0;
        successfulLoadCount = 0;
        liveLibraryCount = 0;
        destroyedLibraryCount = 0;
        resetCalls();
    }

    SlangNVVMBuilderAPI api = {};
    SlangNVVMBuilderFoundationAPI foundation = {};
    SlangNVVMBuilderConstructionAPI construction = {};
    SlangNVVMBuilderValueOperationsAPI valueOperations = {};
    SlangNVVMBuilderSurfaceOperationsAPI surfaceOperationsAPI = {};
    SlangNVVMBuilderTextureOperationsAPI textureOperationsAPI = {};
    uint32_t acceptedABIRevision = SLANG_NVVM_BUILDER_ABI_REVISION;
    SlangNVVMBuilderInterfaceID omittedInterface = SlangNVVMBuilderInterfaceID(~uint32_t(0));
    bool omitAPISymbol = false;
    bool libraryUnavailable = false;
    bool returnNullModule = false;
    bool returnNullIntegerType = false;
    bool returnNullFloatingPointType = false;
    bool returnNullArrayType = false;
    bool returnNullSequentialElementPointer = false;
    bool returnNullGlobalStorage = false;
    FakeNVVMBuilderScalarOperationKey returnNullScalarOperation;
    bool returnNullRelaxedGlobalI32AtomicAdd = false;
    bool failIntegerTypeAfterWrite = false;
    bool failFloatingPointTypeAfterWrite = false;
    bool failArrayTypeAfterWrite = false;
    bool failGlobalStorageAfterWrite = false;
    bool reportMismatchedWriteSize = false;
    SlangNVVMVerificationStatus verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
    SlangNVVMResult serializationWithDiagnosticsResult = SLANG_OK;
    String verificationDiagnostic;
    bool omitValidSerializedOutput = false;
    bool reportMismatchedSerializedDiagnosticWriteSize = false;
    bool reportMismatchedVerificationDiagnosticWriteSize = false;
    bool reportMismatchedVerificationStatus = false;
    String loadedPath;
    int loadRequestCount = 0;
    int successfulLoadCount = 0;
    int liveLibraryCount = 0;
    int destroyedLibraryCount = 0;

    FakeNVVMBuilderModuleStorage moduleStorage;
    FakeNVVMBuilderVoidTypeStorage voidTypeStorage;
    FakeNVVMBuilderFunctionTypeStorage functionTypeStorage[8];
    FakeNVVMBuilderFunctionStorage functionStorage[8];
    FakeNVVMBuilderBlockStorage blockStorage[32];
    FakeNVVMBuilderIntegerTypeStorage integerTypeStorage;
    FakeNVVMBuilderBooleanTypeStorage booleanTypeStorage;
    FakeNVVMBuilderHalfTypeStorage halfTypeStorage;
    FakeNVVMBuilderFloatTypeStorage floatTypeStorage;
    FakeNVVMBuilderPointerTypeStorage pointerTypeStorage;
    FakeNVVMBuilderFloatPointerTypeStorage floatPointerTypeStorage;
    FakeNVVMBuilderArrayTypeStorage arrayTypeStorage;
    FakeNVVMBuilderArrayPointerTypeStorage arrayPointerTypeStorage;
    FakeNVVMBuilderVectorTypeStorage vectorTypeStorage[4][3];
    FakeNVVMBuilderStructTypeStorage structTypeStorage;
    FakeNVVMBuilderScalarStructTypeStorage scalarStructTypeStorage;
    FakeNVVMBuilderScalarStructPointerTypeStorage scalarStructPointerTypeStorage;
    FakeNVVMBuilderResourceViewTypeStorage
        resourceViewTypeStorage[static_cast<uint32_t>(FakeNVVMBuilderScalarTypeKind::Count)];
    FakeNVVMBuilderPointerTypeStorage vectorPointerTypeStorage[2][3];
    FakeNVVMBuilderParameterStorage parameterStorage[64];
    FakeNVVMBuilderLoadStorage loadStorage[16];
    FakeNVVMBuilderScalarOperationStorage scalarOperationStorage[64];
    FakeNVVMBuilderIntrinsicStorage intrinsicStorage[8];
    FakeNVVMBuilderSurfaceOperationStorage surfaceOperationStorage[32];
    FakeNVVMBuilderTextureOperationStorage textureOperationStorage[16];
    FakeNVVMBuilderIntegerConstantStorage integerConstantStorage[64];
    FakeNVVMBuilderFloatingPointConstantStorage floatingPointConstantStorage[64];
    FakeNVVMBuilderScalarPhiStorage scalarPhiStorage[8];
    FakeNVVMBuilderCallStorage callStorage[16];
    FakeNVVMBuilderPointerOffsetStorage pointerOffsetStorage[16];
    FakeNVVMBuilderByteOffsetPointerStorage byteOffsetPointerStorage[16];
    FakeNVVMBuilderSequentialElementPointerStorage sequentialElementPointerStorage[16];
    FakeNVVMBuilderStructFieldPointerStorage structFieldPointerStorage[16];
    FakeNVVMBuilderAggregateElementStorage aggregateElementStorage[16];
    FakeNVVMBuilderAggregateConstructStorage aggregateConstructStorage[16];
    FakeNVVMBuilderRelaxedGlobalI32AtomicAddStorage relaxedGlobalI32AtomicAddStorage[16];
    FakeNVVMBuilderExecutionRegisterStorage executionRegisterStorage[8];
    FakeNVVMBuilderVectorConstructStorage vectorConstructStorage[16];
    FakeNVVMBuilderVectorElementStorage vectorElementStorage[64];
    FakeNVVMBuilderGlobalStorage globalStorage[4];
    FakeNVVMBuilderLocalStorage localStorage[8];

    int createModuleCallCount = 0;
    int destroyModuleCallCount = 0;
    int getVoidTypeCallCount = 0;
    int getFunctionTypeCallCount = 0;
    int declareFunctionCallCount = 0;
    int declareGlobalStorageCallCount = 0;
    int createBlockCallCount = 0;
    int setInsertBlockCallCount = 0;
    int emitReturnVoidCallCount = 0;
    int markFunctionAsKernelCallCount = 0;
    int serializeQueryCallCount = 0;
    int serializeWriteCallCount = 0;
    int serializeWithDiagnosticsQueryCallCount = 0;
    int serializeWithDiagnosticsWriteCallCount = 0;
    int getIntegerTypeCallCount = 0;
    int getFloatingPointTypeCallCount = 0;
    int getArrayTypeCallCount = 0;
    int getVectorTypeCallCount = 0;
    int getStructTypeCallCount = 0;
    int getPointerTypeCallCount = 0;
    int getFunctionParameterCallCount = 0;
    int setFunctionParameterAttributesCallCount = 0;
    int emitLoadCallCount = 0;
    int emitStoreCallCount = 0;
    int emitLocalStorageCallCount = 0;
    int emitBranchCallCount = 0;
    int emitConditionalBranchCallCount = 0;
    int emitSwitchCallCount = 0;
    size_t lastSwitchCaseCount = 0;
    int getIntegerConstantCallCount = 0;
    int getFloatingPointConstantCallCount = 0;
    int emitPhiCallCount = 0;
    int addPhiIncomingCallCount = 0;
    int emitIntegerCallCallCount = 0;
    int emitIntegerReturnCallCount = 0;
    int emitCallCallCount = 0;
    int emitValueReturnCallCount = 0;
    int emitIntrinsicCallCount = 0;
    int isOperationSupportedCallCount = 0;
    int emitPointerOffsetCallCount = 0;
    int emitByteOffsetPointerCallCount = 0;
    int emitSequentialElementPointerCallCount = 0;
    int emitStructFieldPointerCallCount = 0;
    int emitAggregateElementExtractCallCount = 0;
    int emitAggregateConstructCallCount = 0;
    int emitRelaxedGlobalI32AtomicAddCallCount = 0;
    int emitVectorConstructCallCount = 0;
    int emitSequentialElementExtractCallCount = 0;
    int workgroupBarrierCallCount = 0;
    int deviceMemoryBarrierCallCount = 0;
    int scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Count)] = {};
    int valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Count)] = {};
    int scalarOperationCallCounts[Index(FakeNVVMBuilderScalarFamily::Count)]
                                 [SLANG_NVVM_VALUE_OPERATION_COUNT] = {};
    uint32_t integerBitWidth = 0;
    uint32_t floatingPointBitWidth = 0;
    uint32_t arrayElementCount = 0;
    SlangNVVMTypeHandle arrayElementType = nullptr;
    SlangNVVMTypeHandle vectorElementType = nullptr;
    uint32_t vectorElementCount = 0;
    List<SlangNVVMTypeHandle> structFieldTypes;
    List<SlangNVVMTypeHandle> scalarStructFieldTypes;
    SlangNVVMTypeHandle globalStorageValueType = nullptr;
    SlangNVVMLinkage globalStorageLinkage = SLANG_NVVM_LINKAGE_INTERNAL;
    SlangNVVMAddressSpace globalStorageAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
    uint32_t globalStorageAlignment = 0;
    List<String> globalStorageNames;
    List<SlangNVVMTypeHandle> localStorageValueTypes;
    List<uint32_t> localStorageAlignments;
    List<String> localStorageNames;
    SlangNVVMAddressSpace pointerAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
    List<SlangNVVMTypeHandle> pointerPointeeTypes;
    List<SlangNVVMAddressSpace> pointerAddressSpaces;
    size_t functionParameterIndex = 0;
    size_t functionParameterCount = 0;
    List<FakeNVVMBuilderParameterTypeKind> functionParameterTypeKinds;
    List<SlangNVVMTypeHandle> functionParameterTypes;
    List<FakeNVVMBuilderResultTypeKind> functionTypeResultKinds;
    List<SlangNVVMTypeHandle> functionTypeResultTypes;
    List<size_t> functionTypeParameterCounts;
    List<Index> functionTypeParameterKindOffsets;
    List<String> functionNames;
    List<SlangNVVMLinkage> functionLinkages;
    List<SlangNVVMFunctionFlags> functionFlags;
    List<Index> parameterAttributeFunctionIndices;
    List<size_t> parameterAttributeIndices;
    List<SlangNVVMParameterFlags> parameterAttributeFlags;
    List<SlangNVVMTypeHandle> parameterAttributePointeeTypes;
    List<uint32_t> parameterAttributeAlignments;
    List<Index> functionTypeIndices;
    List<Index> blockFunctionIndices;
    uint32_t loadAlignment = 0;
    uint32_t storeAlignment = 0;
    List<uint32_t> loadAlignments;
    List<uint32_t> storeAlignments;
    List<FakeNVVMBuilderScalarOperation> scalarOperations;
    List<FakeNVVMBuilderScalarOperationKey> emittedValueOperations;
    List<SlangNVVMSurfaceOperationDesc> surfaceOperations;
    List<SlangNVVMTextureOperationDesc> textureOperations;
    List<SlangNVVMValueOperation> intrinsicOperations;
    List<SlangNVVMValueTypeDesc> intrinsicResultTypes;
    bool rejectValueOperation = false;
    SlangNVVMValueOperation rejectedValueOperation = 0;
    SlangNVVMValueTypeDesc rejectedValueOperationResultType = {};
    SlangNVVMValueTypeDesc rejectedValueOperationOperandTypes[3] = {};
    uint32_t rejectedValueOperationOperandCount = 0;
    List<Index> intrinsicCallerBlockIndices;
    List<Index> intrinsicArgumentOffsets;
    List<size_t> intrinsicArgumentCounts;
    List<FakeNVVMBuilderValueRef> intrinsicArgumentValueRefs;
    List<int64_t> integerConstantValues;
    List<uint32_t> integerConstantBitWidths;
    List<uint32_t> floatingPointConstantBitWidths;
    List<uint64_t> floatingPointConstantBitPatterns;
    List<Index> scalarPhiTargetBlockIndices;
    List<SlangNVVMTypeHandle> scalarPhiTypes;
    List<Index> scalarPhiIncomingPhiIndices;
    List<FakeNVVMBuilderValueRef> scalarPhiIncomingValueRefs;
    List<Index> scalarPhiIncomingPredecessorBlockIndices;
    List<size_t> functionParameterIndices;
    List<size_t> loadPointerParameterIndices;
    List<SlangNVVMLoadFlags> loadFlags;
    List<Index> storePointerFunctionIndices;
    List<size_t> storePointerParameterIndices;
    List<FakeNVVMBuilderValueKind> storeValueKinds;
    List<size_t> storeValueParameterIndices;
    List<Index> storeValueBinaryIndices;
    List<FakeNVVMBuilderValueRef> storeValueRefs;
    List<Index> storeBlockIndices;
    List<Index> branchSourceBlockIndices;
    List<Index> branchTargetBlockIndices;
    List<Index> callCalleeFunctionIndices;
    List<Index> callCallerBlockIndices;
    List<Index> callArgumentOffsets;
    List<size_t> callArgumentCounts;
    List<FakeNVVMBuilderValueRef> callArgumentValueRefs;
    List<FakeNVVMBuilderResultTypeKind> callResultKinds;
    List<SlangNVVMTypeHandle> callResultTypes;
    List<Index> integerReturnBlockIndices;
    List<FakeNVVMBuilderValueRef> integerReturnValueRefs;
    List<Index> scalarReturnBlockIndices;
    List<FakeNVVMBuilderValueRef> scalarReturnValueRefs;
    List<Index> pointerOffsetCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> pointerOffsetBaseValueRefs;
    List<FakeNVVMBuilderValueRef> pointerOffsetElementValueRefs;
    List<Index> byteOffsetPointerCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> byteOffsetPointerBaseValueRefs;
    List<FakeNVVMBuilderValueRef> byteOffsetPointerOffsetValueRefs;
    List<SlangNVVMTypeHandle> byteOffsetPointerPointeeTypes;
    List<FakeNVVMBuilderScalarTypeKind> byteOffsetPointerTypeKinds;
    List<Index> sequentialElementPointerCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> sequentialElementPointerBaseValueRefs;
    List<FakeNVVMBuilderValueRef> sequentialElementPointerIndexValueRefs;
    List<FakeNVVMBuilderScalarTypeKind> sequentialElementPointerTypeKinds;
    List<FakeNVVMBuilderValueRef> structFieldPointerBaseValueRefs;
    List<uint32_t> structFieldPointerIndices;
    List<FakeNVVMBuilderScalarTypeKind> structFieldPointerTypeKinds;
    List<FakeNVVMBuilderValueRef> aggregateElementBaseValueRefs;
    List<uint32_t> aggregateElementIndices;
    List<FakeNVVMBuilderScalarTypeKind> aggregateElementTypeKinds;
    List<bool> aggregateElementIsFirstClassValue;
    List<SlangNVVMTypeHandle> aggregateConstructResultTypes;
    List<Index> aggregateConstructElementOffsets;
    List<size_t> aggregateConstructElementCounts;
    List<FakeNVVMBuilderValueRef> aggregateConstructElementValueRefs;
    List<Index> relaxedGlobalI32AtomicAddCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> relaxedGlobalI32AtomicAddPointerValueRefs;
    List<FakeNVVMBuilderValueRef> relaxedGlobalI32AtomicAddValueRefs;
    List<FakeNVVMBuilderValueRef> loadPointerValueRefs;
    List<FakeNVVMBuilderScalarTypeKind> loadResultTypeKinds;
    List<FakeNVVMBuilderValueRef> storePointerValueRefs;
    List<Index> kernelFunctionIndices;
    List<SlangNVVMValueOperation> executionRegisterOperations;
    List<Index> executionRegisterCallerBlockIndices;
    List<SlangNVVMTypeHandle> vectorConstructResultTypes;
    List<Index> vectorConstructElementOffsets;
    List<size_t> vectorConstructElementCounts;
    List<FakeNVVMBuilderValueRef> vectorConstructElementValueRefs;
    List<FakeNVVMBuilderValueRef> vectorElementBaseValueRefs;
    List<FakeNVVMBuilderValueRef> vectorElementIndexValueRefs;
    List<uint32_t> vectorElementIndices;
    List<FakeNVVMBuilderScalarTypeKind> vectorElementTypeKinds;
    Index currentInsertBlockIndex = -1;
    Index conditionalSourceBlockIndex = -1;
    Index conditionalTrueBlockIndex = -1;
    Index conditionalFalseBlockIndex = -1;
    String moduleName;
    String functionName;
    String blockName;
    bool failIntegerConstantAfterWrite = false;
    bool failFloatingPointConstantAfterWrite = false;
    bool failScalarPhiAfterWrite = false;
    bool failIntegerCallAfterWrite = false;
    bool failIntegerReturn = false;
    bool failCallAfterWrite = false;
    bool failValueReturn = false;
    bool returnNullIntrinsic = false;
    bool failIntrinsicAfterWrite = false;
    bool failPointerOffsetAfterWrite = false;
    bool failByteOffsetPointerAfterWrite = false;
    bool failSequentialElementPointerAfterWrite = false;
    FakeNVVMBuilderScalarOperationKey failScalarOperationAfterWrite;
    bool failRelaxedGlobalI32AtomicAddAfterWrite = false;
};

FakeNVVMBuilderState gFakeNVVMBuilder;

static SlangNVVMModuleHandle _getFakeNVVMBuilderModule()
{
    return reinterpret_cast<SlangNVVMModuleHandle>(&gFakeNVVMBuilder.moduleStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderVoidType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.voidTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderFunctionType(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.functionTypeStorage));
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.functionTypeStorage[index]);
}

static bool _getFakeNVVMBuilderFunctionTypeIndex(SlangNVVMTypeHandle type, Index& outIndex)
{
    for (Index i = 0; i < SLANG_COUNT_OF(gFakeNVVMBuilder.functionTypeStorage); ++i)
    {
        if (type == _getFakeNVVMBuilderFunctionType(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderFunction(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.functionStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.functionStorage[index]);
}

static bool _getFakeNVVMBuilderFunctionIndex(SlangNVVMValueHandle function, Index& outIndex)
{
    for (Index i = 0; i < SLANG_COUNT_OF(gFakeNVVMBuilder.functionStorage); ++i)
    {
        if (function == _getFakeNVVMBuilderFunction(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMBlockHandle _getFakeNVVMBuilderBlock(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.blockStorage));
    return reinterpret_cast<SlangNVVMBlockHandle>(&gFakeNVVMBuilder.blockStorage[index]);
}

static bool _getFakeNVVMBuilderBlockIndex(SlangNVVMBlockHandle block, Index& outIndex)
{
    for (Index i = 0; i < SLANG_COUNT_OF(gFakeNVVMBuilder.blockStorage); ++i)
    {
        if (block == _getFakeNVVMBuilderBlock(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static bool _getFakeNVVMBuilderBlockFunctionIndex(
    SlangNVVMBlockHandle block,
    Index& outFunctionIndex)
{
    Index blockIndex = -1;
    if (!_getFakeNVVMBuilderBlockIndex(block, blockIndex) || blockIndex < 0 ||
        blockIndex >= gFakeNVVMBuilder.blockFunctionIndices.getCount())
    {
        return false;
    }
    outFunctionIndex = gFakeNVVMBuilder.blockFunctionIndices[blockIndex];
    return true;
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderIntegerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.integerTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderBooleanType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.booleanTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderFloatType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.floatTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderHalfType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.halfTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.pointerTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderFloatPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.floatPointerTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderArrayType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.arrayTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderArrayPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.arrayPointerTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderVectorType(
    uint32_t elementCount = 3,
    FakeNVVMBuilderScalarTypeKind elementTypeKind = FakeNVVMBuilderScalarTypeKind::Integer)
{
    SLANG_ASSERT(elementCount >= 2 && elementCount <= 4);
    SLANG_ASSERT(
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Boolean ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Half ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Float);
    const Index elementTypeIndex = elementTypeKind == FakeNVVMBuilderScalarTypeKind::Boolean ? 1
                                   : elementTypeKind == FakeNVVMBuilderScalarTypeKind::Half  ? 2
                                   : elementTypeKind == FakeNVVMBuilderScalarTypeKind::Float ? 3
                                                                                             : 0;
    return reinterpret_cast<SlangNVVMTypeHandle>(
        &gFakeNVVMBuilder.vectorTypeStorage[elementTypeIndex][elementCount - 2]);
}

static bool _getFakeNVVMBuilderVectorTypeInfo(
    SlangNVVMTypeHandle type,
    uint32_t& outElementCount,
    FakeNVVMBuilderScalarTypeKind& outElementTypeKind)
{
    const FakeNVVMBuilderScalarTypeKind elementTypeKinds[] = {
        FakeNVVMBuilderScalarTypeKind::Integer,
        FakeNVVMBuilderScalarTypeKind::Boolean,
        FakeNVVMBuilderScalarTypeKind::Half,
        FakeNVVMBuilderScalarTypeKind::Float,
    };
    for (auto elementTypeKind : elementTypeKinds)
    {
        for (uint32_t elementCount = 2; elementCount <= 4; ++elementCount)
        {
            if (type == _getFakeNVVMBuilderVectorType(elementCount, elementTypeKind))
            {
                outElementCount = elementCount;
                outElementTypeKind = elementTypeKind;
                return true;
            }
        }
    }
    return false;
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderVectorPointerType(
    uint32_t elementCount,
    FakeNVVMBuilderScalarTypeKind elementTypeKind)
{
    SLANG_ASSERT(elementCount >= 2 && elementCount <= 4);
    SLANG_ASSERT(
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Float);
    const Index elementTypeIndex = elementTypeKind == FakeNVVMBuilderScalarTypeKind::Float ? 1 : 0;
    return reinterpret_cast<SlangNVVMTypeHandle>(
        &gFakeNVVMBuilder.vectorPointerTypeStorage[elementTypeIndex][elementCount - 2]);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderScalarStructPointerType();

static bool _getFakeNVVMBuilderPointerElementTypeKind(
    SlangNVVMTypeHandle type,
    FakeNVVMBuilderScalarTypeKind& outElementTypeKind)
{
    if (type == _getFakeNVVMBuilderPointerType())
    {
        outElementTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
        return true;
    }
    if (type == _getFakeNVVMBuilderFloatPointerType())
    {
        outElementTypeKind = FakeNVVMBuilderScalarTypeKind::Float;
        return true;
    }
    if (type == _getFakeNVVMBuilderScalarStructPointerType())
    {
        outElementTypeKind = FakeNVVMBuilderScalarTypeKind::ScalarStruct;
        return true;
    }
    if (type == _getFakeNVVMBuilderArrayPointerType())
    {
        outElementTypeKind = FakeNVVMBuilderScalarTypeKind::NumericArray;
        return true;
    }
    const FakeNVVMBuilderScalarTypeKind elementTypeKinds[] = {
        FakeNVVMBuilderScalarTypeKind::Integer,
        FakeNVVMBuilderScalarTypeKind::Float,
    };
    for (auto elementTypeKind : elementTypeKinds)
    {
        for (uint32_t elementCount = 2; elementCount <= 4; ++elementCount)
        {
            if (type == _getFakeNVVMBuilderVectorPointerType(elementCount, elementTypeKind))
            {
                outElementTypeKind =
                    elementTypeKind == FakeNVVMBuilderScalarTypeKind::Float
                        ? FakeNVVMBuilderScalarTypeKind(
                              Index(FakeNVVMBuilderScalarTypeKind::Float2) + elementCount - 2)
                        : FakeNVVMBuilderScalarTypeKind(
                              Index(FakeNVVMBuilderScalarTypeKind::UInt2) + elementCount - 2);
                return true;
            }
        }
    }
    return false;
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderStructType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.structTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderScalarStructType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.scalarStructTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderScalarStructPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle>(&gFakeNVVMBuilder.scalarStructPointerTypeStorage);
}

static SlangNVVMTypeHandle _getFakeNVVMBuilderResourceViewType(
    FakeNVVMBuilderScalarTypeKind elementTypeKind = FakeNVVMBuilderScalarTypeKind::Integer)
{
    SLANG_ASSERT(
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::Float ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::ScalarStruct ||
        (elementTypeKind >= FakeNVVMBuilderScalarTypeKind::UInt2 &&
         elementTypeKind <= FakeNVVMBuilderScalarTypeKind::Float4));
    return reinterpret_cast<SlangNVVMTypeHandle>(
        &gFakeNVVMBuilder.resourceViewTypeStorage[Index(elementTypeKind)]);
}

static bool _getFakeNVVMBuilderResourceViewElementTypeKind(
    SlangNVVMTypeHandle type,
    FakeNVVMBuilderScalarTypeKind& outElementTypeKind)
{
    const FakeNVVMBuilderScalarTypeKind elementTypeKinds[] = {
        FakeNVVMBuilderScalarTypeKind::Integer,
        FakeNVVMBuilderScalarTypeKind::Float,
        FakeNVVMBuilderScalarTypeKind::UInt2,
        FakeNVVMBuilderScalarTypeKind::UInt3,
        FakeNVVMBuilderScalarTypeKind::UInt4,
        FakeNVVMBuilderScalarTypeKind::Float2,
        FakeNVVMBuilderScalarTypeKind::Float3,
        FakeNVVMBuilderScalarTypeKind::Float4,
        FakeNVVMBuilderScalarTypeKind::ScalarStruct,
    };
    for (auto elementTypeKind : elementTypeKinds)
    {
        if (type == _getFakeNVVMBuilderResourceViewType(elementTypeKind))
        {
            outElementTypeKind = elementTypeKind;
            return true;
        }
    }
    return false;
}

static bool _getFakeNVVMBuilderTypeKind(
    SlangNVVMTypeHandle type,
    FakeNVVMBuilderScalarTypeKind& outTypeKind)
{
    if (type == _getFakeNVVMBuilderIntegerType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
    else if (type == _getFakeNVVMBuilderBooleanType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Boolean;
    else if (type == _getFakeNVVMBuilderHalfType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Half;
    else if (type == _getFakeNVVMBuilderFloatType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Float;
    else if (type == _getFakeNVVMBuilderArrayType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::NumericArray;
    else if (type == _getFakeNVVMBuilderArrayPointerType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::NumericArrayPointer;
    else if (type == _getFakeNVVMBuilderScalarStructPointerType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::ScalarStructPointer;
    else if (type == _getFakeNVVMBuilderScalarStructType())
        outTypeKind = FakeNVVMBuilderScalarTypeKind::ScalarStruct;
    else
    {
        uint32_t vectorElementCount = 0;
        FakeNVVMBuilderScalarTypeKind vectorElementTypeKind;
        if (_getFakeNVVMBuilderVectorTypeInfo(type, vectorElementCount, vectorElementTypeKind))
        {
            outTypeKind =
                vectorElementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer
                    ? FakeNVVMBuilderScalarTypeKind(
                          Index(FakeNVVMBuilderScalarTypeKind::UInt2) + vectorElementCount - 2)
                : vectorElementTypeKind == FakeNVVMBuilderScalarTypeKind::Float
                    ? FakeNVVMBuilderScalarTypeKind(
                          Index(FakeNVVMBuilderScalarTypeKind::Float2) + vectorElementCount - 2)
                : vectorElementTypeKind == FakeNVVMBuilderScalarTypeKind::Half
                    ? FakeNVVMBuilderScalarTypeKind(
                          Index(FakeNVVMBuilderScalarTypeKind::Half2) + vectorElementCount - 2)
                    : FakeNVVMBuilderScalarTypeKind::Boolean;
            return true;
        }
        FakeNVVMBuilderScalarTypeKind resourceElementTypeKind;
        if (!_getFakeNVVMBuilderResourceViewElementTypeKind(type, resourceElementTypeKind))
            return false;
        outTypeKind = FakeNVVMBuilderScalarTypeKind::ResourceView;
    }
    return true;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderFunctionParameter(
    Index functionIndex,
    Index parameterIndex)
{
    const Index storageIndex = functionIndex * 8 + parameterIndex;
    SLANG_ASSERT(functionIndex >= 0 && functionIndex < 8);
    SLANG_ASSERT(parameterIndex >= 0 && parameterIndex < 8);
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.parameterStorage[storageIndex]);
}

// Provides the original single-function test view through the canonical function/parameter map.
static SlangNVVMValueHandle _getFakeNVVMBuilderParameter(Index index = 0)
{
    return _getFakeNVVMBuilderFunctionParameter(0, index);
}

static bool _getFakeNVVMBuilderParameterRef(
    SlangNVVMValueHandle value,
    Index& outFunctionIndex,
    size_t& outParameterIndex)
{
    for (Index functionIndex = 0; functionIndex < 8; ++functionIndex)
    {
        for (Index parameterIndex = 0; parameterIndex < 8; ++parameterIndex)
        {
            if (value == _getFakeNVVMBuilderFunctionParameter(functionIndex, parameterIndex))
            {
                outFunctionIndex = functionIndex;
                outParameterIndex = size_t(parameterIndex);
                return true;
            }
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderLoad(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.loadStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.loadStorage[index]);
}

static bool _getFakeNVVMBuilderLoadIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.loadResultTypeKinds.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderLoad(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static bool _isFakeNVVMBuilderScalarOperation(
    const FakeNVVMBuilderScalarOperationKey& key,
    FakeNVVMBuilderScalarFamily family,
    uint32_t operation)
{
    return key.family == family && key.operation == operation;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderScalarOperation(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.scalarOperationStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.scalarOperationStorage[index]);
}

static bool _getFakeNVVMBuilderScalarOperationIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderScalarOperation(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderIntrinsic(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.intrinsicStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.intrinsicStorage[index]);
}

static bool _getFakeNVVMBuilderIntrinsicIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.intrinsicOperations.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderIntrinsic(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderSurfaceOperation(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.surfaceOperationStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.surfaceOperationStorage[index]);
}

static bool _getFakeNVVMBuilderSurfaceOperationIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.surfaceOperations.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderSurfaceOperation(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderTextureOperation(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.textureOperationStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.textureOperationStorage[index]);
}

static bool _getFakeNVVMBuilderTextureOperationIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.textureOperations.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderTextureOperation(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static Index _findFakeNVVMBuilderScalarOperation(
    FakeNVVMBuilderScalarFamily family,
    uint32_t operation,
    Index occurrence = 0)
{
    for (Index i = 0; i < gFakeNVVMBuilder.scalarOperations.getCount(); ++i)
    {
        if (_isFakeNVVMBuilderScalarOperation(
                gFakeNVVMBuilder.scalarOperations[i].key,
                family,
                operation) &&
            occurrence-- == 0)
        {
            return i;
        }
    }
    return -1;
}

static int _getFakeNVVMBuilderScalarOperationCallCount(
    FakeNVVMBuilderScalarFamily family,
    uint32_t operation)
{
    SLANG_ASSERT(operation < SLANG_COUNT_OF(gFakeNVVMBuilder.scalarOperationCallCounts[0]));
    return gFakeNVVMBuilder.scalarOperationCallCounts[Index(family)][operation];
}

static void _setFakeNVVMBuilderScalarOperationFailure(
    FakeNVVMBuilderScalarOperationKey& failure,
    FakeNVVMBuilderScalarFamily family,
    uint32_t operation,
    bool enabled)
{
    failure = enabled ? FakeNVVMBuilderScalarOperationKey{family, operation}
                      : FakeNVVMBuilderScalarOperationKey{};
}

static SlangNVVMValueHandle _getFakeNVVMBuilderIntegerConstant(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.integerConstantStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.integerConstantStorage[index]);
}

static bool _getFakeNVVMBuilderIntegerConstantIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.integerConstantValues.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderIntegerConstant(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderFloatingPointConstant(Index index = 0)
{
    SLANG_ASSERT(
        index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.floatingPointConstantStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.floatingPointConstantStorage[index]);
}

static bool _getFakeNVVMBuilderFloatingPointConstantIndex(
    SlangNVVMValueHandle value,
    Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderFloatingPointConstant(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderScalarPhi(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.scalarPhiStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.scalarPhiStorage[index]);
}

static bool _getFakeNVVMBuilderScalarPhiIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderScalarPhi(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderCall(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.callStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.callStorage[index]);
}

static bool _getFakeNVVMBuilderCallIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderCall(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderPointerOffset(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.pointerOffsetStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.pointerOffsetStorage[index]);
}

static bool _getFakeNVVMBuilderPointerOffsetIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderPointerOffset(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderByteOffsetPointer(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.byteOffsetPointerStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.byteOffsetPointerStorage[index]);
}

static bool _getFakeNVVMBuilderByteOffsetPointerIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.byteOffsetPointerBaseValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderByteOffsetPointer(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderSequentialElementPointer(Index index = 0)
{
    SLANG_ASSERT(
        index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.sequentialElementPointerStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.sequentialElementPointerStorage[index]);
}

static bool _getFakeNVVMBuilderSequentialElementPointerIndex(
    SlangNVVMValueHandle value,
    Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderSequentialElementPointer(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderStructFieldPointer(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.structFieldPointerStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.structFieldPointerStorage[index]);
}

static bool _getFakeNVVMBuilderStructFieldPointerIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.structFieldPointerBaseValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderStructFieldPointer(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderAggregateElement(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.aggregateElementStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.aggregateElementStorage[index]);
}

static bool _getFakeNVVMBuilderAggregateElementIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.aggregateElementBaseValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderAggregateElement(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderAggregateConstruct(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.aggregateConstructStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.aggregateConstructStorage[index]);
}

static bool _getFakeNVVMBuilderAggregateConstructIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.aggregateConstructResultTypes.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderAggregateConstruct(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderRelaxedGlobalI32AtomicAdd(Index index = 0)
{
    SLANG_ASSERT(
        index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.relaxedGlobalI32AtomicAddStorage[index]);
}

static bool _getFakeNVVMBuilderRelaxedGlobalI32AtomicAddIndex(
    SlangNVVMValueHandle value,
    Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderRelaxedGlobalI32AtomicAdd(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderExecutionRegister(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.executionRegisterStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(
        &gFakeNVVMBuilder.executionRegisterStorage[index]);
}

static bool _getFakeNVVMBuilderExecutionRegisterIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.executionRegisterOperations.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderExecutionRegister(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderVectorConstruct(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.vectorConstructStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.vectorConstructStorage[index]);
}

static bool _getFakeNVVMBuilderVectorConstructIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.vectorConstructResultTypes.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderVectorConstruct(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderVectorElement(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.vectorElementStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.vectorElementStorage[index]);
}

static bool _getFakeNVVMBuilderVectorElementIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.vectorElementIndices.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderVectorElement(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderGlobalStorage(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.globalStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.globalStorage[index]);
}

static bool _getFakeNVVMBuilderGlobalStorageIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.globalStorageNames.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderGlobalStorage(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle _getFakeNVVMBuilderLocalStorage(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.localStorage));
    return reinterpret_cast<SlangNVVMValueHandle>(&gFakeNVVMBuilder.localStorage[index]);
}

static bool _getFakeNVVMBuilderLocalStorageIndex(SlangNVVMValueHandle value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.localStorageValueTypes.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderLocalStorage(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static bool _getFakeNVVMBuilderValueRef(SlangNVVMValueHandle value, FakeNVVMBuilderValueRef& outRef)
{
    Index parameterFunctionIndex = -1;
    size_t parameterIndex = 0;
    Index valueIndex = -1;
    if (_getFakeNVVMBuilderParameterRef(value, parameterFunctionIndex, parameterIndex))
    {
        outRef = {
            FakeNVVMBuilderValueKind::Parameter,
            Index(parameterIndex),
            parameterFunctionIndex};
        return true;
    }
    if (_getFakeNVVMBuilderLoadIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::Load, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderScalarOperationIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::ScalarOperation, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderIntrinsicIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::Intrinsic, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderSurfaceOperationIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::SurfaceOperation, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderTextureOperationIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::TextureOperation, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderIntegerConstantIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::IntegerConstant, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderFloatingPointConstantIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::FloatingPointConstant, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderScalarPhiIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::ScalarPhi, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderCallIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::Call, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderPointerOffsetIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::PointerOffset, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderByteOffsetPointerIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::ByteOffsetPointer, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderSequentialElementPointerIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::SequentialElementPointer, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderStructFieldPointerIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::StructFieldPointer, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderAggregateElementIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::AggregateElement, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderAggregateConstructIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::AggregateConstruct, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderRelaxedGlobalI32AtomicAddIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::RelaxedGlobalI32AtomicAdd, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderExecutionRegisterIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::ExecutionRegister, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderVectorConstructIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::VectorConstruct, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderVectorElementIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::VectorElement, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderGlobalStorageIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::GlobalStorage, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderLocalStorageIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::LocalStorage, valueIndex};
        return true;
    }
    return false;
}

static bool _hasFakeNVVMBuilderPhiIncoming(
    Index phiIndex,
    FakeNVVMBuilderValueKind valueKind,
    Index valueIndex,
    Index predecessorBlockIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.getCount(); ++i)
    {
        const FakeNVVMBuilderValueRef valueRef = gFakeNVVMBuilder.scalarPhiIncomingValueRefs[i];
        if (gFakeNVVMBuilder.scalarPhiIncomingPhiIndices[i] == phiIndex &&
            valueRef.kind == valueKind && valueRef.index == valueIndex &&
            gFakeNVVMBuilder.scalarPhiIncomingPredecessorBlockIndices[i] == predecessorBlockIndex)
        {
            return true;
        }
    }
    return false;
}

static bool _hasFakeNVVMBuilderBranch(Index sourceBlockIndex, Index targetBlockIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.branchSourceBlockIndices.getCount(); ++i)
    {
        if (gFakeNVVMBuilder.branchSourceBlockIndices[i] == sourceBlockIndex &&
            gFakeNVVMBuilder.branchTargetBlockIndices[i] == targetBlockIndex)
        {
            return true;
        }
    }
    return false;
}

static bool _getFakeNVVMBuilderParameterTypeKind(
    const FakeNVVMBuilderValueRef& valueRef,
    FakeNVVMBuilderParameterTypeKind& outTypeKind)
{
    if (valueRef.kind != FakeNVVMBuilderValueKind::Parameter || valueRef.functionIndex < 0 ||
        valueRef.functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount())
    {
        return false;
    }
    const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[valueRef.functionIndex];
    if (functionTypeIndex < 0 ||
        functionTypeIndex >= gFakeNVVMBuilder.functionTypeParameterCounts.getCount() ||
        functionTypeIndex >= gFakeNVVMBuilder.functionTypeParameterKindOffsets.getCount() ||
        valueRef.index < 0 ||
        size_t(valueRef.index) >= gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex])
    {
        return false;
    }
    const Index typeKindIndex =
        gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex] + valueRef.index;
    if (typeKindIndex < 0 ||
        typeKindIndex >= gFakeNVVMBuilder.functionParameterTypeKinds.getCount())
    {
        return false;
    }
    outTypeKind = gFakeNVVMBuilder.functionParameterTypeKinds[typeKindIndex];
    return true;
}

static bool _getFakeNVVMBuilderParameterType(
    const FakeNVVMBuilderValueRef& valueRef,
    SlangNVVMTypeHandle& outType)
{
    if (valueRef.kind != FakeNVVMBuilderValueKind::Parameter || valueRef.functionIndex < 0 ||
        valueRef.functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount())
    {
        return false;
    }
    const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[valueRef.functionIndex];
    if (functionTypeIndex < 0 ||
        functionTypeIndex >= gFakeNVVMBuilder.functionTypeParameterCounts.getCount() ||
        functionTypeIndex >= gFakeNVVMBuilder.functionTypeParameterKindOffsets.getCount() ||
        valueRef.index < 0 ||
        size_t(valueRef.index) >= gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex])
    {
        return false;
    }
    const Index typeIndex =
        gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex] + valueRef.index;
    if (typeIndex < 0 || typeIndex >= gFakeNVVMBuilder.functionParameterTypes.getCount())
        return false;
    outType = gFakeNVVMBuilder.functionParameterTypes[typeIndex];
    return true;
}

// Checks a first-class value produced by aggregate extraction against its recorded element type.
static bool _isFakeNVVMBuilderAggregateElementValueOfTypeKind(
    const FakeNVVMBuilderValueRef& valueRef,
    FakeNVVMBuilderScalarTypeKind expectedTypeKind)
{
    return valueRef.kind == FakeNVVMBuilderValueKind::AggregateElement && valueRef.index >= 0 &&
           valueRef.index < gFakeNVVMBuilder.aggregateElementTypeKinds.getCount() &&
           valueRef.index < gFakeNVVMBuilder.aggregateElementIsFirstClassValue.getCount() &&
           gFakeNVVMBuilder.aggregateElementIsFirstClassValue[valueRef.index] &&
           gFakeNVVMBuilder.aggregateElementTypeKinds[valueRef.index] == expectedTypeKind;
}

static bool _isFakeNVVMBuilderIntegerValue(SlangNVVMValueHandle value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    switch (valueRef.kind)
    {
    case FakeNVVMBuilderValueKind::Parameter:
        {
            FakeNVVMBuilderParameterTypeKind parameterTypeKind;
            return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
                   parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Integer;
        }
    case FakeNVVMBuilderValueKind::Load:
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount() &&
               gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] ==
                   FakeNVVMBuilderScalarTypeKind::Integer;
    case FakeNVVMBuilderValueKind::IntegerConstant:
        if (valueRef.index < 0 ||
            valueRef.index >= gFakeNVVMBuilder.integerConstantBitWidths.getCount())
        {
            return false;
        }
        switch (gFakeNVVMBuilder.integerConstantBitWidths[valueRef.index])
        {
        case 8:
        case 16:
        case 32:
        case 64:
            return true;
        default:
            return false;
        }
    case FakeNVVMBuilderValueKind::RelaxedGlobalI32AtomicAdd:
        return true;
    case FakeNVVMBuilderValueKind::VectorElement:
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.vectorElementTypeKinds.getCount() &&
               gFakeNVVMBuilder.vectorElementTypeKinds[valueRef.index] ==
                   FakeNVVMBuilderScalarTypeKind::Integer;
    case FakeNVVMBuilderValueKind::Call:
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.callResultTypes.getCount() &&
               gFakeNVVMBuilder.callResultTypes[valueRef.index] == _getFakeNVVMBuilderIntegerType();
    case FakeNVVMBuilderValueKind::FloatingPointConstant:
        return false;
    case FakeNVVMBuilderValueKind::ScalarPhi:
        return valueRef.index >= 0 && valueRef.index < gFakeNVVMBuilder.scalarPhiTypes.getCount() &&
               gFakeNVVMBuilder.scalarPhiTypes[valueRef.index] == _getFakeNVVMBuilderIntegerType();
    case FakeNVVMBuilderValueKind::ScalarOperation:
        {
            const FakeNVVMBuilderScalarOperation& operation =
                gFakeNVVMBuilder.scalarOperations[valueRef.index];
            return operation.key.family == FakeNVVMBuilderScalarFamily::Unary ||
                   operation.key.family == FakeNVVMBuilderScalarFamily::Binary ||
                   (operation.key.family == FakeNVVMBuilderScalarFamily::Select &&
                    (operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                     operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
                    operation.resultType.laneCount == 1);
        }
    case FakeNVVMBuilderValueKind::Intrinsic:
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.intrinsicResultTypes.getCount() &&
               (gFakeNVVMBuilder.intrinsicResultTypes[valueRef.index].kind ==
                    SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                gFakeNVVMBuilder.intrinsicResultTypes[valueRef.index].kind ==
                    SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER);
    case FakeNVVMBuilderValueKind::TextureOperation:
        {
            if (valueRef.index < 0 ||
                valueRef.index >= gFakeNVVMBuilder.textureOperations.getCount())
                return false;
            const SlangNVVMTextureOperationDesc& textureOperation =
                gFakeNVVMBuilder.textureOperations[valueRef.index];
            switch (textureOperation.operation)
            {
            case SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH:
            case SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT:
            case SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH:
                return true;
            case SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL:
                return (textureOperation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                        textureOperation.elementType.kind ==
                            SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
                       textureOperation.elementType.bitWidth == 32 &&
                       textureOperation.elementType.laneCount == 1;
            default:
                return false;
            }
        }
    case FakeNVVMBuilderValueKind::SurfaceOperation:
        {
            if (valueRef.index < 0 ||
                valueRef.index >= gFakeNVVMBuilder.surfaceOperations.getCount())
                return false;
            const SlangNVVMSurfaceOperationDesc& surfaceOperation =
                gFakeNVVMBuilder.surfaceOperations[valueRef.index];
            return surfaceOperation.operation == SLANG_NVVM_SURFACE_OP_LOAD &&
                   (surfaceOperation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                    surfaceOperation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
                   surfaceOperation.elementType.bitWidth == 32 &&
                   surfaceOperation.elementType.laneCount == 1;
        }
    case FakeNVVMBuilderValueKind::PointerOffset:
    case FakeNVVMBuilderValueKind::ByteOffsetPointer:
    case FakeNVVMBuilderValueKind::SequentialElementPointer:
    case FakeNVVMBuilderValueKind::AggregateConstruct:
    case FakeNVVMBuilderValueKind::ExecutionRegister:
    case FakeNVVMBuilderValueKind::VectorConstruct:
        return false;
    case FakeNVVMBuilderValueKind::AggregateElement:
        return _isFakeNVVMBuilderAggregateElementValueOfTypeKind(
            valueRef,
            FakeNVVMBuilderScalarTypeKind::Integer);
    }
    return false;
}

static bool _isFakeNVVMBuilderVectorValue(
    SlangNVVMValueHandle value,
    FakeNVVMBuilderScalarTypeKind expectedElementTypeKind,
    uint32_t expectedElementCount)
{
    if ((expectedElementTypeKind != FakeNVVMBuilderScalarTypeKind::Integer &&
         expectedElementTypeKind != FakeNVVMBuilderScalarTypeKind::Boolean &&
         expectedElementTypeKind != FakeNVVMBuilderScalarTypeKind::Half &&
         expectedElementTypeKind != FakeNVVMBuilderScalarTypeKind::Float) ||
        expectedElementCount < 2 || expectedElementCount > 4)
    {
        return false;
    }

    FakeNVVMBuilderScalarTypeKind expectedVectorTypeKind = FakeNVVMBuilderScalarTypeKind::Boolean;
    if (expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer)
    {
        expectedVectorTypeKind = expectedElementCount == 2   ? FakeNVVMBuilderScalarTypeKind::UInt2
                                 : expectedElementCount == 3 ? FakeNVVMBuilderScalarTypeKind::UInt3
                                                             : FakeNVVMBuilderScalarTypeKind::UInt4;
    }
    else if (expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Float)
    {
        expectedVectorTypeKind = expectedElementCount == 2 ? FakeNVVMBuilderScalarTypeKind::Float2
                                 : expectedElementCount == 3
                                     ? FakeNVVMBuilderScalarTypeKind::Float3
                                     : FakeNVVMBuilderScalarTypeKind::Float4;
    }
    else if (expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Half)
    {
        expectedVectorTypeKind = expectedElementCount == 2   ? FakeNVVMBuilderScalarTypeKind::Half2
                                 : expectedElementCount == 3 ? FakeNVVMBuilderScalarTypeKind::Half3
                                                             : FakeNVVMBuilderScalarTypeKind::Half4;
    }

    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;
    if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        SlangNVVMTypeHandle parameterType = nullptr;
        return _getFakeNVVMBuilderParameterType(valueRef, parameterType) &&
               parameterType ==
                   _getFakeNVVMBuilderVectorType(expectedElementCount, expectedElementTypeKind);
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::ExecutionRegister)
        return expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer &&
               expectedElementCount == 3;
    if (valueRef.kind == FakeNVVMBuilderValueKind::Load && valueRef.index >= 0 &&
        valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount())
    {
        return expectedElementTypeKind != FakeNVVMBuilderScalarTypeKind::Boolean &&
               gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] == expectedVectorTypeKind;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::VectorConstruct && valueRef.index >= 0 &&
        valueRef.index < gFakeNVVMBuilder.vectorConstructResultTypes.getCount())
    {
        return gFakeNVVMBuilder.vectorConstructResultTypes[valueRef.index] ==
               _getFakeNVVMBuilderVectorType(expectedElementCount, expectedElementTypeKind);
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::AggregateElement)
        return _isFakeNVVMBuilderAggregateElementValueOfTypeKind(valueRef, expectedVectorTypeKind);
    if (valueRef.kind == FakeNVVMBuilderValueKind::VectorElement)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.vectorElementTypeKinds.getCount() &&
               gFakeNVVMBuilder.vectorElementTypeKinds[valueRef.index] == expectedVectorTypeKind;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::SurfaceOperation && valueRef.index >= 0 &&
        valueRef.index < gFakeNVVMBuilder.surfaceOperations.getCount())
    {
        const SlangNVVMSurfaceOperationDesc& operation =
            gFakeNVVMBuilder.surfaceOperations[valueRef.index];
        const uint32_t expectedBitWidth =
            expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Half ? 16 : 32;
        const bool isExpectedKind =
            expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer
                ? operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                      operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER
                : operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
        return operation.operation == SLANG_NVVM_SURFACE_OP_LOAD && isExpectedKind &&
               operation.elementType.bitWidth == expectedBitWidth &&
               operation.elementType.laneCount == expectedElementCount;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::TextureOperation && valueRef.index >= 0 &&
        valueRef.index < gFakeNVVMBuilder.textureOperations.getCount())
    {
        const SlangNVVMTextureOperationDesc& operation =
            gFakeNVVMBuilder.textureOperations[valueRef.index];
        const bool isExpectedKind =
            expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer
                ? operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                      operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER
                : expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Float &&
                      operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
        return operation.operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL && isExpectedKind &&
               operation.elementType.bitWidth == 32 &&
               operation.elementType.laneCount == expectedElementCount;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::ScalarOperation && valueRef.index >= 0 &&
        valueRef.index < gFakeNVVMBuilder.scalarOperations.getCount())
    {
        const SlangNVVMValueTypeDesc& resultType =
            gFakeNVVMBuilder.scalarOperations[valueRef.index].resultType;
        const bool isExpectedKind =
            expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Integer
                ? resultType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                      resultType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER
            : expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Boolean
                ? resultType.kind == SLANG_NVVM_VALUE_TYPE_BOOL
                : resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT;
        const bool isExpectedWidth =
            expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Half
                ? resultType.bitWidth == 16
            : expectedElementTypeKind == FakeNVVMBuilderScalarTypeKind::Float
                ? resultType.bitWidth == 32
                : true;
        return isExpectedKind && isExpectedWidth && resultType.laneCount == expectedElementCount;
    }
    const SlangNVVMTypeHandle expectedType =
        _getFakeNVVMBuilderVectorType(expectedElementCount, expectedElementTypeKind);
    if (valueRef.kind == FakeNVVMBuilderValueKind::ScalarPhi)
    {
        return valueRef.index >= 0 && valueRef.index < gFakeNVVMBuilder.scalarPhiTypes.getCount() &&
               gFakeNVVMBuilder.scalarPhiTypes[valueRef.index] == expectedType;
    }
    return valueRef.kind == FakeNVVMBuilderValueKind::Call && valueRef.index >= 0 &&
           valueRef.index < gFakeNVVMBuilder.callResultTypes.getCount() &&
           gFakeNVVMBuilder.callResultTypes[valueRef.index] == expectedType;
}

static bool _isFakeNVVMBuilderIntegerVectorValue(
    SlangNVVMValueHandle value,
    uint32_t expectedElementCount)
{
    return _isFakeNVVMBuilderVectorValue(
        value,
        FakeNVVMBuilderScalarTypeKind::Integer,
        expectedElementCount);
}

static bool _isFakeNVVMBuilderFloatingPointValue(
    SlangNVVMValueHandle value,
    uint32_t expectedBitWidth);
static bool _isFakeNVVMBuilderBooleanValue(SlangNVVMValueHandle value);
static bool _getFakeNVVMBuilderPointerScalarTypeKind(
    const FakeNVVMBuilderValueRef& pointerRef,
    FakeNVVMBuilderScalarTypeKind& outTypeKind);

static bool _isFakeNVVMBuilderValueOfTypeKind(
    SlangNVVMValueHandle value,
    FakeNVVMBuilderScalarTypeKind typeKind)
{
    switch (typeKind)
    {
    case FakeNVVMBuilderScalarTypeKind::Integer:
        return _isFakeNVVMBuilderIntegerValue(value);
    case FakeNVVMBuilderScalarTypeKind::Boolean:
        return _isFakeNVVMBuilderBooleanValue(value);
    case FakeNVVMBuilderScalarTypeKind::Half:
        return _isFakeNVVMBuilderFloatingPointValue(value, 16);
    case FakeNVVMBuilderScalarTypeKind::Float:
        return _isFakeNVVMBuilderFloatingPointValue(value, 32);
    case FakeNVVMBuilderScalarTypeKind::UInt2:
        return _isFakeNVVMBuilderIntegerVectorValue(value, 2);
    case FakeNVVMBuilderScalarTypeKind::UInt3:
        return _isFakeNVVMBuilderIntegerVectorValue(value, 3);
    case FakeNVVMBuilderScalarTypeKind::UInt4:
        return _isFakeNVVMBuilderIntegerVectorValue(value, 4);
    case FakeNVVMBuilderScalarTypeKind::Float2:
        return _isFakeNVVMBuilderVectorValue(value, FakeNVVMBuilderScalarTypeKind::Float, 2);
    case FakeNVVMBuilderScalarTypeKind::Float3:
        return _isFakeNVVMBuilderVectorValue(value, FakeNVVMBuilderScalarTypeKind::Float, 3);
    case FakeNVVMBuilderScalarTypeKind::Float4:
        return _isFakeNVVMBuilderVectorValue(value, FakeNVVMBuilderScalarTypeKind::Float, 4);
    case FakeNVVMBuilderScalarTypeKind::Half2:
        return _isFakeNVVMBuilderVectorValue(value, FakeNVVMBuilderScalarTypeKind::Half, 2);
    case FakeNVVMBuilderScalarTypeKind::Half3:
        return _isFakeNVVMBuilderVectorValue(value, FakeNVVMBuilderScalarTypeKind::Half, 3);
    case FakeNVVMBuilderScalarTypeKind::Half4:
        return _isFakeNVVMBuilderVectorValue(value, FakeNVVMBuilderScalarTypeKind::Half, 4);
    case FakeNVVMBuilderScalarTypeKind::NumericArray:
        {
            FakeNVVMBuilderValueRef valueRef;
            if (!_getFakeNVVMBuilderValueRef(value, valueRef) || valueRef.index < 0)
                return false;
            if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
            {
                FakeNVVMBuilderParameterTypeKind parameterTypeKind;
                return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
                       parameterTypeKind == FakeNVVMBuilderParameterTypeKind::NumericArray;
            }
            if (valueRef.kind == FakeNVVMBuilderValueKind::AggregateConstruct)
            {
                return valueRef.index < gFakeNVVMBuilder.aggregateConstructResultTypes.getCount() &&
                       gFakeNVVMBuilder.aggregateConstructResultTypes[valueRef.index] ==
                           _getFakeNVVMBuilderArrayType();
            }
            if (valueRef.kind == FakeNVVMBuilderValueKind::ScalarPhi)
            {
                return valueRef.index < gFakeNVVMBuilder.scalarPhiTypes.getCount() &&
                       gFakeNVVMBuilder.scalarPhiTypes[valueRef.index] ==
                           _getFakeNVVMBuilderArrayType();
            }
            if (valueRef.kind == FakeNVVMBuilderValueKind::Call)
            {
                return valueRef.index < gFakeNVVMBuilder.callResultTypes.getCount() &&
                       gFakeNVVMBuilder.callResultTypes[valueRef.index] ==
                           _getFakeNVVMBuilderArrayType();
            }
            return valueRef.kind == FakeNVVMBuilderValueKind::Load &&
                   valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount() &&
                   gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] ==
                       FakeNVVMBuilderScalarTypeKind::NumericArray;
        }
    case FakeNVVMBuilderScalarTypeKind::NumericArrayPointer:
        return false;
    case FakeNVVMBuilderScalarTypeKind::ScalarStruct:
        {
            FakeNVVMBuilderValueRef valueRef;
            if (!_getFakeNVVMBuilderValueRef(value, valueRef) || valueRef.index < 0)
                return false;
            if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
            {
                SlangNVVMTypeHandle parameterType = nullptr;
                return _getFakeNVVMBuilderParameterType(valueRef, parameterType) &&
                       parameterType == _getFakeNVVMBuilderScalarStructType();
            }
            if (valueRef.kind == FakeNVVMBuilderValueKind::AggregateConstruct)
            {
                return valueRef.index < gFakeNVVMBuilder.aggregateConstructResultTypes.getCount() &&
                       gFakeNVVMBuilder.aggregateConstructResultTypes[valueRef.index] ==
                           _getFakeNVVMBuilderScalarStructType();
            }
            if (valueRef.kind == FakeNVVMBuilderValueKind::Load)
            {
                return valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount() &&
                       gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] ==
                           FakeNVVMBuilderScalarTypeKind::ScalarStruct;
            }
            return valueRef.kind == FakeNVVMBuilderValueKind::Call &&
                   valueRef.index < gFakeNVVMBuilder.callResultTypes.getCount() &&
                   gFakeNVVMBuilder.callResultTypes[valueRef.index] ==
                       _getFakeNVVMBuilderScalarStructType();
        }
    default:
        return false;
    }
}

// Checks a generic function/control-flow value against the exact type handle supplied by the API.
static bool _isFakeNVVMBuilderValueOfType(SlangNVVMValueHandle value, SlangNVVMTypeHandle type)
{
    FakeNVVMBuilderScalarTypeKind expectedPointerElementTypeKind;
    if (_getFakeNVVMBuilderPointerElementTypeKind(type, expectedPointerElementTypeKind))
    {
        FakeNVVMBuilderValueRef valueRef;
        FakeNVVMBuilderScalarTypeKind actualPointerElementTypeKind;
        return _getFakeNVVMBuilderValueRef(value, valueRef) &&
               _getFakeNVVMBuilderPointerScalarTypeKind(valueRef, actualPointerElementTypeKind) &&
               actualPointerElementTypeKind == expectedPointerElementTypeKind;
    }
    uint32_t vectorElementCount = 0;
    FakeNVVMBuilderScalarTypeKind vectorElementTypeKind;
    if (_getFakeNVVMBuilderVectorTypeInfo(type, vectorElementCount, vectorElementTypeKind))
    {
        return _isFakeNVVMBuilderVectorValue(value, vectorElementTypeKind, vectorElementCount);
    }
    if (type == _getFakeNVVMBuilderBooleanType())
        return _isFakeNVVMBuilderBooleanValue(value);
    FakeNVVMBuilderScalarTypeKind typeKind;
    return _getFakeNVVMBuilderTypeKind(type, typeKind) &&
           _isFakeNVVMBuilderValueOfTypeKind(value, typeKind);
}

static bool _isFakeNVVMBuilderFloatingPointValue(
    SlangNVVMValueHandle value,
    uint32_t expectedBitWidth)
{
    if (expectedBitWidth != 16 && expectedBitWidth != 32)
        return false;
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    const SlangNVVMTypeHandle expectedType =
        expectedBitWidth == 16 ? _getFakeNVVMBuilderHalfType() : _getFakeNVVMBuilderFloatType();
    const FakeNVVMBuilderScalarTypeKind expectedTypeKind =
        expectedBitWidth == 16 ? FakeNVVMBuilderScalarTypeKind::Half
                               : FakeNVVMBuilderScalarTypeKind::Float;

    if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        SlangNVVMTypeHandle parameterType = nullptr;
        return _getFakeNVVMBuilderParameterType(valueRef, parameterType) &&
               parameterType == expectedType;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Load)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount() &&
               gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] == expectedTypeKind;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::ScalarPhi)
    {
        return valueRef.index >= 0 && valueRef.index < gFakeNVVMBuilder.scalarPhiTypes.getCount() &&
               gFakeNVVMBuilder.scalarPhiTypes[valueRef.index] == expectedType;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Call)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.callResultTypes.getCount() &&
               gFakeNVVMBuilder.callResultTypes[valueRef.index] == expectedType;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Intrinsic)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.intrinsicResultTypes.getCount() &&
               gFakeNVVMBuilder.intrinsicResultTypes[valueRef.index].kind ==
                   SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
               gFakeNVVMBuilder.intrinsicResultTypes[valueRef.index].bitWidth == expectedBitWidth &&
               gFakeNVVMBuilder.intrinsicResultTypes[valueRef.index].laneCount == 1;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::SurfaceOperation)
    {
        if (valueRef.index < 0 || valueRef.index >= gFakeNVVMBuilder.surfaceOperations.getCount())
            return false;
        const SlangNVVMSurfaceOperationDesc& operation =
            gFakeNVVMBuilder.surfaceOperations[valueRef.index];
        return operation.operation == SLANG_NVVM_SURFACE_OP_LOAD &&
               operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
               operation.elementType.bitWidth == expectedBitWidth &&
               operation.elementType.laneCount == 1;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::TextureOperation)
    {
        if (valueRef.index < 0 || valueRef.index >= gFakeNVVMBuilder.textureOperations.getCount())
            return false;
        const SlangNVVMTextureOperationDesc& operation =
            gFakeNVVMBuilder.textureOperations[valueRef.index];
        return (operation.operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL ||
                operation.operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL) &&
               operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
               operation.elementType.bitWidth == expectedBitWidth &&
               operation.elementType.laneCount == 1;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::VectorElement)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.vectorElementTypeKinds.getCount() &&
               gFakeNVVMBuilder.vectorElementTypeKinds[valueRef.index] == expectedTypeKind;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::FloatingPointConstant)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.floatingPointConstantBitWidths.getCount() &&
               gFakeNVVMBuilder.floatingPointConstantBitWidths[valueRef.index] == expectedBitWidth;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::AggregateElement)
        return _isFakeNVVMBuilderAggregateElementValueOfTypeKind(valueRef, expectedTypeKind);
    if (valueRef.kind != FakeNVVMBuilderValueKind::ScalarOperation || valueRef.index < 0 ||
        valueRef.index >= gFakeNVVMBuilder.scalarOperations.getCount())
    {
        return false;
    }
    const SlangNVVMValueTypeDesc& resultType =
        gFakeNVVMBuilder.scalarOperations[valueRef.index].resultType;
    if (resultType.kind == SLANG_NVVM_VALUE_TYPE_VOID)
    {
        const FakeNVVMBuilderScalarFamily family =
            gFakeNVVMBuilder.scalarOperations[valueRef.index].key.family;
        return expectedBitWidth == 32 && (family == FakeNVVMBuilderScalarFamily::FloatingUnary ||
                                          family == FakeNVVMBuilderScalarFamily::FloatingBinary);
    }
    return resultType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
           resultType.bitWidth == expectedBitWidth && resultType.laneCount == 1;
}

static bool _isFakeNVVMBuilderBooleanValue(SlangNVVMValueHandle value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;
    if (valueRef.kind == FakeNVVMBuilderValueKind::IntegerConstant)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.integerConstantBitWidths.getCount() &&
               gFakeNVVMBuilder.integerConstantBitWidths[valueRef.index] == 1;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        FakeNVVMBuilderParameterTypeKind parameterTypeKind;
        return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
               parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Boolean;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Call)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.callResultTypes.getCount() &&
               gFakeNVVMBuilder.callResultTypes[valueRef.index] == _getFakeNVVMBuilderBooleanType();
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::ScalarPhi)
    {
        return valueRef.index >= 0 && valueRef.index < gFakeNVVMBuilder.scalarPhiTypes.getCount() &&
               gFakeNVVMBuilder.scalarPhiTypes[valueRef.index] == _getFakeNVVMBuilderBooleanType();
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Intrinsic)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.intrinsicResultTypes.getCount() &&
               gFakeNVVMBuilder.intrinsicResultTypes[valueRef.index].kind ==
                   SLANG_NVVM_VALUE_TYPE_BOOL;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::VectorElement)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.vectorElementTypeKinds.getCount() &&
               gFakeNVVMBuilder.vectorElementTypeKinds[valueRef.index] ==
                   FakeNVVMBuilderScalarTypeKind::Boolean;
    }
    Index operationIndex = -1;
    if (!_getFakeNVVMBuilderScalarOperationIndex(value, operationIndex))
        return false;
    const FakeNVVMBuilderScalarOperation& operation =
        gFakeNVVMBuilder.scalarOperations[operationIndex];
    return operation.key.family == FakeNVVMBuilderScalarFamily::Compare ||
           operation.key.family == FakeNVVMBuilderScalarFamily::FloatingCompare ||
           (operation.key.family == FakeNVVMBuilderScalarFamily::Select &&
            operation.resultType.kind == SLANG_NVVM_VALUE_TYPE_BOOL &&
            operation.resultType.laneCount == 1);
}

static bool _getFakeNVVMBuilderResourceViewElementTypeKind(
    const FakeNVVMBuilderValueRef& valueRef,
    FakeNVVMBuilderScalarTypeKind& outElementTypeKind);

static bool _isFakeNVVMBuilderPointerValue(SlangNVVMValueHandle value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    if (valueRef.kind == FakeNVVMBuilderValueKind::PointerOffset ||
        valueRef.kind == FakeNVVMBuilderValueKind::ByteOffsetPointer ||
        valueRef.kind == FakeNVVMBuilderValueKind::SequentialElementPointer ||
        valueRef.kind == FakeNVVMBuilderValueKind::StructFieldPointer)
        return true;
    if (valueRef.kind == FakeNVVMBuilderValueKind::LocalStorage)
        return true;
    if (valueRef.kind == FakeNVVMBuilderValueKind::AggregateElement)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.aggregateElementIsFirstClassValue.getCount() &&
               !gFakeNVVMBuilder.aggregateElementIsFirstClassValue[valueRef.index];
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Load)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount() &&
               (gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] ==
                    FakeNVVMBuilderScalarTypeKind::ScalarStructPointer ||
                gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] ==
                    FakeNVVMBuilderScalarTypeKind::NumericArrayPointer);
    }
    FakeNVVMBuilderParameterTypeKind parameterTypeKind;
    return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
           (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Pointer ||
            parameterTypeKind == FakeNVVMBuilderParameterTypeKind::FloatPointer ||
            parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ArrayPointer ||
            parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ScalarStructPointer);
}

static bool _getFakeNVVMBuilderResourceViewElementTypeKind(
    const FakeNVVMBuilderValueRef& valueRef,
    FakeNVVMBuilderScalarTypeKind& outElementTypeKind)
{
    if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        SlangNVVMTypeHandle parameterType = nullptr;
        return _getFakeNVVMBuilderParameterType(valueRef, parameterType) &&
               _getFakeNVVMBuilderResourceViewElementTypeKind(parameterType, outElementTypeKind);
    }
    if (valueRef.kind != FakeNVVMBuilderValueKind::Load || valueRef.index < 0 ||
        valueRef.index >= gFakeNVVMBuilder.loadResultTypeKinds.getCount() ||
        gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] !=
            FakeNVVMBuilderScalarTypeKind::ResourceView ||
        valueRef.index >= gFakeNVVMBuilder.loadPointerValueRefs.getCount())
    {
        return false;
    }

    const FakeNVVMBuilderValueRef& pointerRef =
        gFakeNVVMBuilder.loadPointerValueRefs[valueRef.index];
    if (pointerRef.kind != FakeNVVMBuilderValueKind::StructFieldPointer || pointerRef.index < 0 ||
        pointerRef.index >= gFakeNVVMBuilder.structFieldPointerIndices.getCount())
    {
        return false;
    }
    const uint32_t fieldIndex = gFakeNVVMBuilder.structFieldPointerIndices[pointerRef.index];
    return fieldIndex < uint32_t(gFakeNVVMBuilder.structFieldTypes.getCount()) &&
           _getFakeNVVMBuilderResourceViewElementTypeKind(
               gFakeNVVMBuilder.structFieldTypes[fieldIndex],
               outElementTypeKind);
}

static bool _getFakeNVVMBuilderPointerScalarTypeKind(
    const FakeNVVMBuilderValueRef& pointerRef,
    FakeNVVMBuilderScalarTypeKind& outTypeKind)
{
    switch (pointerRef.kind)
    {
    case FakeNVVMBuilderValueKind::Parameter:
        {
            FakeNVVMBuilderParameterTypeKind parameterTypeKind;
            if (!_getFakeNVVMBuilderParameterTypeKind(pointerRef, parameterTypeKind))
                return false;
            if (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Pointer)
            {
                outTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
                return true;
            }
            if (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::FloatPointer)
            {
                outTypeKind = FakeNVVMBuilderScalarTypeKind::Float;
                return true;
            }
            if (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ScalarStructPointer)
            {
                outTypeKind = FakeNVVMBuilderScalarTypeKind::ScalarStruct;
                return true;
            }
            if (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ArrayPointer)
            {
                outTypeKind = FakeNVVMBuilderScalarTypeKind::NumericArray;
                return true;
            }
            return false;
        }
    case FakeNVVMBuilderValueKind::LocalStorage:
        if (pointerRef.index < 0 ||
            pointerRef.index >= gFakeNVVMBuilder.localStorageValueTypes.getCount())
        {
            return false;
        }
        return _getFakeNVVMBuilderTypeKind(
            gFakeNVVMBuilder.localStorageValueTypes[pointerRef.index],
            outTypeKind);
    case FakeNVVMBuilderValueKind::GlobalStorage:
        return gFakeNVVMBuilder.globalStorageValueType &&
               _getFakeNVVMBuilderTypeKind(gFakeNVVMBuilder.globalStorageValueType, outTypeKind);
    case FakeNVVMBuilderValueKind::PointerOffset:
        return pointerRef.index >= 0 &&
               pointerRef.index < gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() &&
               _getFakeNVVMBuilderPointerScalarTypeKind(
                   gFakeNVVMBuilder.pointerOffsetBaseValueRefs[pointerRef.index],
                   outTypeKind);
    case FakeNVVMBuilderValueKind::ByteOffsetPointer:
        if (pointerRef.index < 0 ||
            pointerRef.index >= gFakeNVVMBuilder.byteOffsetPointerTypeKinds.getCount())
        {
            return false;
        }
        outTypeKind = gFakeNVVMBuilder.byteOffsetPointerTypeKinds[pointerRef.index];
        return true;
    case FakeNVVMBuilderValueKind::Load:
        if (pointerRef.index < 0 ||
            pointerRef.index >= gFakeNVVMBuilder.loadResultTypeKinds.getCount())
        {
            return false;
        }
        if (gFakeNVVMBuilder.loadResultTypeKinds[pointerRef.index] ==
            FakeNVVMBuilderScalarTypeKind::ScalarStructPointer)
        {
            outTypeKind = FakeNVVMBuilderScalarTypeKind::ScalarStruct;
            return true;
        }
        if (gFakeNVVMBuilder.loadResultTypeKinds[pointerRef.index] ==
            FakeNVVMBuilderScalarTypeKind::NumericArrayPointer)
        {
            outTypeKind = FakeNVVMBuilderScalarTypeKind::NumericArray;
            return true;
        }
        return false;
    case FakeNVVMBuilderValueKind::SequentialElementPointer:
        if (pointerRef.index < 0 ||
            pointerRef.index >= gFakeNVVMBuilder.sequentialElementPointerTypeKinds.getCount())
        {
            return false;
        }
        outTypeKind = gFakeNVVMBuilder.sequentialElementPointerTypeKinds[pointerRef.index];
        return true;
    case FakeNVVMBuilderValueKind::AggregateElement:
        if (pointerRef.index < 0 ||
            pointerRef.index >= gFakeNVVMBuilder.aggregateElementTypeKinds.getCount())
        {
            return false;
        }
        outTypeKind = gFakeNVVMBuilder.aggregateElementTypeKinds[pointerRef.index];
        return true;
    case FakeNVVMBuilderValueKind::StructFieldPointer:
        if (pointerRef.index < 0 ||
            pointerRef.index >= gFakeNVVMBuilder.structFieldPointerTypeKinds.getCount())
        {
            return false;
        }
        outTypeKind = gFakeNVVMBuilder.structFieldPointerTypeKinds[pointerRef.index];
        return true;
    default:
        return false;
    }
}

static bool _getFakeNVVMBuilderSequentialElementTypeKind(
    SlangNVVMValueHandle value,
    FakeNVVMBuilderScalarTypeKind& outTypeKind)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    FakeNVVMBuilderScalarTypeKind pointeeTypeKind;
    if (!_getFakeNVVMBuilderPointerScalarTypeKind(valueRef, pointeeTypeKind))
        return false;
    if (pointeeTypeKind == FakeNVVMBuilderScalarTypeKind::NumericArray)
        return _getFakeNVVMBuilderTypeKind(gFakeNVVMBuilder.arrayElementType, outTypeKind);
    if (pointeeTypeKind >= FakeNVVMBuilderScalarTypeKind::UInt2 &&
        pointeeTypeKind <= FakeNVVMBuilderScalarTypeKind::UInt4)
    {
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
        return true;
    }
    if (pointeeTypeKind >= FakeNVVMBuilderScalarTypeKind::Float2 &&
        pointeeTypeKind <= FakeNVVMBuilderScalarTypeKind::Float4)
    {
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Float;
        return true;
    }
    if (pointeeTypeKind >= FakeNVVMBuilderScalarTypeKind::Half2 &&
        pointeeTypeKind <= FakeNVVMBuilderScalarTypeKind::Half4)
    {
        outTypeKind = FakeNVVMBuilderScalarTypeKind::Half;
        return true;
    }
    return false;
}

static bool _isFakeNVVMBuilderResourceViewValue(SlangNVVMValueHandle value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;
    FakeNVVMBuilderScalarTypeKind elementTypeKind;
    return _getFakeNVVMBuilderResourceViewElementTypeKind(valueRef, elementTypeKind);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderCreateModule(
    const char* moduleName,
    size_t moduleNameSize,
    SlangNVVMModuleHandle* outModule)
{
    ++gFakeNVVMBuilder.createModuleCallCount;
    if ((!moduleName && moduleNameSize) || !outModule)
        return SLANG_E_INVALID_ARG;
    gFakeNVVMBuilder.moduleName = String(UnownedStringSlice(moduleName, moduleNameSize));
    *outModule = gFakeNVVMBuilder.returnNullModule ? nullptr : _getFakeNVVMBuilderModule();
    return SLANG_OK;
}

static void SLANG_NVVM_CALL _fakeNVVMBuilderDestroyModule(SlangNVVMModuleHandle module)
{
    SLANG_ASSERT(module == _getFakeNVVMBuilderModule());
    ++gFakeNVVMBuilder.destroyModuleCallCount;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderGetVoidType(SlangNVVMModuleHandle module, SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getVoidTypeCallCount;
    if (module != _getFakeNVVMBuilderModule() || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = _getFakeNVVMBuilderVoidType();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetVectorType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getVectorTypeCallCount;
    const bool isSupportedElementType = elementType == _getFakeNVVMBuilderIntegerType() ||
                                        elementType == _getFakeNVVMBuilderBooleanType() ||
                                        elementType == _getFakeNVVMBuilderHalfType() ||
                                        elementType == _getFakeNVVMBuilderFloatType();
    const FakeNVVMBuilderScalarTypeKind elementTypeKind =
        elementType == _getFakeNVVMBuilderIntegerType() ? FakeNVVMBuilderScalarTypeKind::Integer
        : elementType == _getFakeNVVMBuilderHalfType()  ? FakeNVVMBuilderScalarTypeKind::Half
        : elementType == _getFakeNVVMBuilderFloatType() ? FakeNVVMBuilderScalarTypeKind::Float
                                                        : FakeNVVMBuilderScalarTypeKind::Boolean;
    if (module != _getFakeNVVMBuilderModule() || !isSupportedElementType ||
        (elementTypeKind != FakeNVVMBuilderScalarTypeKind::Integer &&
         elementTypeKind != FakeNVVMBuilderScalarTypeKind::Boolean &&
         elementTypeKind != FakeNVVMBuilderScalarTypeKind::Half &&
         elementTypeKind != FakeNVVMBuilderScalarTypeKind::Float) ||
        elementCount < 2 || elementCount > 4 || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.vectorElementType = elementType;
    gFakeNVVMBuilder.vectorElementCount = elementCount;
    *outType = _getFakeNVVMBuilderVectorType(elementCount, elementTypeKind);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetStructType(
    SlangNVVMModuleHandle module,
    const SlangNVVMTypeHandle* fieldTypes,
    size_t fieldCount,
    SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getStructTypeCallCount;
    if (module != _getFakeNVVMBuilderModule() || (!fieldTypes && fieldCount) || !outType)
        return SLANG_E_INVALID_ARG;
    FakeNVVMBuilderScalarTypeKind resourceElementTypeKind;
    const bool isResourceView =
        fieldCount == 2 &&
        _getFakeNVVMBuilderPointerElementTypeKind(fieldTypes[0], resourceElementTypeKind) &&
        fieldTypes[1] == _getFakeNVVMBuilderIntegerType();
    bool isCopyableStruct = fieldCount != 0 && !isResourceView;
    for (size_t i = 0; isCopyableStruct && i < fieldCount; ++i)
    {
        uint32_t vectorElementCount = 0;
        FakeNVVMBuilderScalarTypeKind vectorElementTypeKind;
        const bool isNumericVector =
            _getFakeNVVMBuilderVectorTypeInfo(
                fieldTypes[i],
                vectorElementCount,
                vectorElementTypeKind) &&
            vectorElementTypeKind != FakeNVVMBuilderScalarTypeKind::Boolean;
        isCopyableStruct = fieldTypes[i] == _getFakeNVVMBuilderIntegerType() ||
                           fieldTypes[i] == _getFakeNVVMBuilderHalfType() ||
                           fieldTypes[i] == _getFakeNVVMBuilderFloatType() ||
                           fieldTypes[i] == _getFakeNVVMBuilderArrayType() || isNumericVector;
    }
    bool isGlobalParams = fieldCount != 0 && !isResourceView && !isCopyableStruct;
    bool hasGlobalResource = false;
    for (size_t i = 0; isGlobalParams && i < fieldCount; ++i)
    {
        FakeNVVMBuilderScalarTypeKind globalResourceElementTypeKind;
        if (_getFakeNVVMBuilderResourceViewElementTypeKind(
                fieldTypes[i],
                globalResourceElementTypeKind))
        {
            hasGlobalResource = true;
        }
        else if (
            fieldTypes[i] != _getFakeNVVMBuilderIntegerType() &&
            fieldTypes[i] != _getFakeNVVMBuilderFloatType() &&
            fieldTypes[i] != _getFakeNVVMBuilderArrayPointerType() &&
            fieldTypes[i] != _getFakeNVVMBuilderScalarStructPointerType())
        {
            isGlobalParams = false;
        }
    }
    isGlobalParams = isGlobalParams && hasGlobalResource;
    if (!isResourceView && !isCopyableStruct && !isGlobalParams)
        return SLANG_E_INVALID_ARG;
    if (isGlobalParams)
    {
        gFakeNVVMBuilder.structFieldTypes.clear();
        gFakeNVVMBuilder.structFieldTypes.addRange(fieldTypes, Index(fieldCount));
        *outType = _getFakeNVVMBuilderStructType();
    }
    else if (isCopyableStruct)
    {
        gFakeNVVMBuilder.scalarStructFieldTypes.clear();
        gFakeNVVMBuilder.scalarStructFieldTypes.addRange(fieldTypes, Index(fieldCount));
        *outType = _getFakeNVVMBuilderScalarStructType();
    }
    else
    {
        *outType = _getFakeNVVMBuilderResourceViewType(resourceElementTypeKind);
    }
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFunctionType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle resultType,
    const SlangNVVMTypeHandle* parameterTypes,
    size_t parameterCount,
    SlangNVVMTypeHandle* outType)
{
    const Index functionTypeIndex = gFakeNVVMBuilder.getFunctionTypeCallCount++;
    uint32_t resultVectorElementCount = 0;
    FakeNVVMBuilderScalarTypeKind resultVectorElementTypeKind;
    const bool isVectorResult = _getFakeNVVMBuilderVectorTypeInfo(
        resultType,
        resultVectorElementCount,
        resultVectorElementTypeKind);
    const bool hasSupportedResult = resultType == _getFakeNVVMBuilderVoidType() ||
                                    resultType == _getFakeNVVMBuilderIntegerType() ||
                                    resultType == _getFakeNVVMBuilderBooleanType() ||
                                    resultType == _getFakeNVVMBuilderHalfType() ||
                                    resultType == _getFakeNVVMBuilderFloatType() ||
                                    resultType == _getFakeNVVMBuilderScalarStructType() ||
                                    isVectorResult;
    if (module != _getFakeNVVMBuilderModule() || !hasSupportedResult ||
        (!parameterTypes && parameterCount) || !outType ||
        functionTypeIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.functionTypeStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.functionTypeResultKinds.add(
        resultType == _getFakeNVVMBuilderVoidType()      ? FakeNVVMBuilderResultTypeKind::Void
        : resultType == _getFakeNVVMBuilderIntegerType() ? FakeNVVMBuilderResultTypeKind::Integer
        : resultType == _getFakeNVVMBuilderBooleanType() ? FakeNVVMBuilderResultTypeKind::Boolean
        : resultType == _getFakeNVVMBuilderHalfType()    ? FakeNVVMBuilderResultTypeKind::Half
        : resultType == _getFakeNVVMBuilderFloatType()   ? FakeNVVMBuilderResultTypeKind::Float
        : resultType == _getFakeNVVMBuilderScalarStructType()
            ? FakeNVVMBuilderResultTypeKind::ScalarStruct
            : FakeNVVMBuilderResultTypeKind::ValueVector);
    gFakeNVVMBuilder.functionTypeResultTypes.add(resultType);
    gFakeNVVMBuilder.functionTypeParameterCounts.add(parameterCount);
    gFakeNVVMBuilder.functionTypeParameterKindOffsets.add(
        gFakeNVVMBuilder.functionParameterTypeKinds.getCount());
    for (size_t i = 0; i < parameterCount; ++i)
    {
        FakeNVVMBuilderScalarTypeKind resourceElementTypeKind;
        uint32_t vectorElementCount = 0;
        FakeNVVMBuilderScalarTypeKind vectorElementTypeKind;
        if (parameterTypes[i] != _getFakeNVVMBuilderIntegerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderBooleanType() &&
            parameterTypes[i] != _getFakeNVVMBuilderHalfType() &&
            parameterTypes[i] != _getFakeNVVMBuilderFloatType() &&
            parameterTypes[i] != _getFakeNVVMBuilderPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderFloatPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderArrayPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderScalarStructPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderScalarStructType() &&
            parameterTypes[i] != _getFakeNVVMBuilderArrayType() &&
            !_getFakeNVVMBuilderVectorTypeInfo(
                parameterTypes[i],
                vectorElementCount,
                vectorElementTypeKind) &&
            !_getFakeNVVMBuilderResourceViewElementTypeKind(
                parameterTypes[i],
                resourceElementTypeKind))
        {
            return SLANG_E_INVALID_ARG;
        }
    }
    for (size_t i = 0; i < parameterCount; ++i)
    {
        FakeNVVMBuilderScalarTypeKind resourceElementTypeKind;
        uint32_t vectorElementCount = 0;
        FakeNVVMBuilderScalarTypeKind vectorElementTypeKind;
        const bool isResourceView = _getFakeNVVMBuilderResourceViewElementTypeKind(
            parameterTypes[i],
            resourceElementTypeKind);
        const bool isValueVector = _getFakeNVVMBuilderVectorTypeInfo(
            parameterTypes[i],
            vectorElementCount,
            vectorElementTypeKind);
        const FakeNVVMBuilderParameterTypeKind parameterTypeKind =
            parameterTypes[i] == _getFakeNVVMBuilderIntegerType()
                ? FakeNVVMBuilderParameterTypeKind::Integer
            : parameterTypes[i] == _getFakeNVVMBuilderBooleanType()
                ? FakeNVVMBuilderParameterTypeKind::Boolean
            : parameterTypes[i] == _getFakeNVVMBuilderHalfType()
                ? FakeNVVMBuilderParameterTypeKind::Half
            : parameterTypes[i] == _getFakeNVVMBuilderFloatType()
                ? FakeNVVMBuilderParameterTypeKind::Float
            : parameterTypes[i] == _getFakeNVVMBuilderPointerType()
                ? FakeNVVMBuilderParameterTypeKind::Pointer
            : parameterTypes[i] == _getFakeNVVMBuilderFloatPointerType()
                ? FakeNVVMBuilderParameterTypeKind::FloatPointer
            : parameterTypes[i] == _getFakeNVVMBuilderArrayPointerType()
                ? FakeNVVMBuilderParameterTypeKind::ArrayPointer
            : parameterTypes[i] == _getFakeNVVMBuilderScalarStructPointerType()
                ? FakeNVVMBuilderParameterTypeKind::ScalarStructPointer
            : parameterTypes[i] == _getFakeNVVMBuilderScalarStructType()
                ? FakeNVVMBuilderParameterTypeKind::ScalarStruct
            : parameterTypes[i] == _getFakeNVVMBuilderArrayType()
                ? FakeNVVMBuilderParameterTypeKind::NumericArray
            : isValueVector ? FakeNVVMBuilderParameterTypeKind::ValueVector
                            : FakeNVVMBuilderParameterTypeKind::ResourceView;
        SLANG_ASSERT(
            parameterTypeKind != FakeNVVMBuilderParameterTypeKind::ResourceView || isResourceView);
        gFakeNVVMBuilder.functionParameterTypeKinds.add(parameterTypeKind);
        gFakeNVVMBuilder.functionParameterTypes.add(parameterTypes[i]);
    }
    gFakeNVVMBuilder.functionParameterCount = parameterCount;
    *outType = _getFakeNVVMBuilderFunctionType(functionTypeIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderDeclareFunction(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle functionType,
    SlangNVVMLinkage linkage,
    SlangNVVMFunctionFlags flags,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle* outFunction)
{
    const Index functionIndex = gFakeNVVMBuilder.declareFunctionCallCount++;
    Index functionTypeIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionTypeIndex(functionType, functionTypeIndex) ||
        functionTypeIndex >= gFakeNVVMBuilder.functionTypeResultKinds.getCount() ||
        (linkage != SLANG_NVVM_LINKAGE_INTERNAL && linkage != SLANG_NVVM_LINKAGE_EXTERNAL) ||
        (flags & ~SLANG_NVVM_FUNCTION_FLAG_NO_INLINE) || (!name && nameSize) || !outFunction ||
        functionIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.functionStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.functionName = String(UnownedStringSlice(name, nameSize));
    gFakeNVVMBuilder.functionNames.add(gFakeNVVMBuilder.functionName);
    gFakeNVVMBuilder.functionLinkages.add(linkage);
    gFakeNVVMBuilder.functionFlags.add(flags);
    gFakeNVVMBuilder.functionTypeIndices.add(functionTypeIndex);
    *outFunction = _getFakeNVVMBuilderFunction(functionIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderCreateBlock(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    const char* name,
    size_t nameSize,
    SlangNVVMBlockHandle* outBlock)
{
    const Index blockIndex = gFakeNVVMBuilder.createBlockCallCount++;
    Index functionIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionIndex(function, functionIndex) ||
        functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount() || (!name && nameSize) ||
        !outBlock || blockIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.blockStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.blockName = String(UnownedStringSlice(name, nameSize));
    gFakeNVVMBuilder.blockFunctionIndices.add(functionIndex);
    *outBlock = _getFakeNVVMBuilderBlock(blockIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderSetInsertBlock(SlangNVVMModuleHandle module, SlangNVVMBlockHandle block)
{
    ++gFakeNVVMBuilder.setInsertBlockCallCount;
    Index blockIndex = -1;
    if (module != _getFakeNVVMBuilderModule() || !_getFakeNVVMBuilderBlockIndex(block, blockIndex))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.currentInsertBlockIndex = blockIndex;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitReturnVoid(SlangNVVMModuleHandle module)
{
    ++gFakeNVVMBuilder.emitReturnVoidCallCount;
    Index functionIndex = -1;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        gFakeNVVMBuilder.currentInsertBlockIndex >=
            gFakeNVVMBuilder.blockFunctionIndices.getCount())
    {
        return SLANG_E_INVALID_ARG;
    }
    functionIndex = gFakeNVVMBuilder.blockFunctionIndices[gFakeNVVMBuilder.currentInsertBlockIndex];
    if (functionIndex < 0 || functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount())
        return SLANG_E_INVALID_ARG;
    const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[functionIndex];
    return gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
                   FakeNVVMBuilderResultTypeKind::Void
               ? SLANG_OK
               : SLANG_E_INVALID_ARG;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderMarkFunctionAsKernel(SlangNVVMModuleHandle module, SlangNVVMValueHandle function)
{
    ++gFakeNVVMBuilder.markFunctionAsKernelCallCount;
    Index functionIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionIndex(function, functionIndex) ||
        functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount())
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.kernelFunctionIndices.add(functionIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderSerializeModule(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* destination,
    size_t destinationSize,
    size_t* outSerializedSize)
{
    if (module != _getFakeNVVMBuilderModule() || !outSerializedSize ||
        (!destination && destinationSize))
    {
        return SLANG_E_INVALID_ARG;
    }

    const void* source = nullptr;
    size_t sourceSize = 0;
    switch (format)
    {
    case SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY:
        source = kFakeNVVMBuilderAssembly;
        sourceSize = sizeof(kFakeNVVMBuilderAssembly) - 1;
        break;
    case SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE:
        source = kFakeNVVMBuilderBitcode;
        sourceSize = sizeof(kFakeNVVMBuilderBitcode);
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }

    *outSerializedSize = sourceSize;
    if (!destination)
    {
        ++gFakeNVVMBuilder.serializeQueryCallCount;
        return SLANG_OK;
    }

    ++gFakeNVVMBuilder.serializeWriteCallCount;
    if (destinationSize < sourceSize)
        return SLANG_E_BUFFER_TOO_SMALL;
    ::memcpy(destination, source, sourceSize);
    if (gFakeNVVMBuilder.reportMismatchedWriteSize)
        *outSerializedSize = sourceSize - 1;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderSerializeModuleWithDiagnostics(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus* outVerificationStatus)
{
    if (module != _getFakeNVVMBuilderModule() || !outSerializedSize || !outDiagnosticSize ||
        !outVerificationStatus || (!serializedDestination && serializedDestinationSize) ||
        (!diagnosticDestination && diagnosticDestinationSize))
    {
        return SLANG_E_INVALID_ARG;
    }

    const void* serializedSource = nullptr;
    size_t serializedSize = 0;
    if (format != SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY &&
        format != SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE)
    {
        return SLANG_E_INVALID_ARG;
    }
    if (gFakeNVVMBuilder.verificationStatus == SLANG_NVVM_VERIFICATION_VALID &&
        !gFakeNVVMBuilder.omitValidSerializedOutput)
    {
        if (format == SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY)
        {
            serializedSource = kFakeNVVMBuilderAssembly;
            serializedSize = sizeof(kFakeNVVMBuilderAssembly) - 1;
        }
        else
        {
            serializedSource = kFakeNVVMBuilderBitcode;
            serializedSize = sizeof(kFakeNVVMBuilderBitcode);
        }
    }

    const size_t diagnosticSize = size_t(gFakeNVVMBuilder.verificationDiagnostic.getLength());
    *outSerializedSize = serializedSize;
    *outDiagnosticSize = diagnosticSize;
    *outVerificationStatus = gFakeNVVMBuilder.verificationStatus;

    const bool isQuery = !serializedDestination && !diagnosticDestination;
    if (isQuery)
    {
        ++gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount;
        return gFakeNVVMBuilder.serializationWithDiagnosticsResult;
    }

    ++gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount;
    if (serializedDestinationSize < serializedSize || diagnosticDestinationSize < diagnosticSize ||
        (serializedSize && !serializedDestination) || (diagnosticSize && !diagnosticDestination))
    {
        return SLANG_E_BUFFER_TOO_SMALL;
    }
    if (gFakeNVVMBuilder.serializationWithDiagnosticsResult < 0)
        return gFakeNVVMBuilder.serializationWithDiagnosticsResult;

    if (serializedSize)
        ::memcpy(serializedDestination, serializedSource, serializedSize);
    if (diagnosticSize)
    {
        ::memcpy(
            diagnosticDestination,
            gFakeNVVMBuilder.verificationDiagnostic.getBuffer(),
            diagnosticSize);
    }
    if (gFakeNVVMBuilder.reportMismatchedSerializedDiagnosticWriteSize)
        *outSerializedSize = serializedSize - 1;
    if (gFakeNVVMBuilder.reportMismatchedVerificationDiagnosticWriteSize)
        *outDiagnosticSize = diagnosticSize ? diagnosticSize - 1 : 1;
    if (gFakeNVVMBuilder.reportMismatchedVerificationStatus)
        *outVerificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderSerializeNVVMIR20AssemblyWithDiagnostics(
    SlangNVVMModuleHandle module,
    SlangNVVMSerializationFormat format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus* outVerificationStatus)
{
    if (format != SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY)
        return SLANG_E_INVALID_ARG;

    return _fakeNVVMBuilderSerializeModuleWithDiagnostics(
        module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        serializedDestination,
        serializedDestinationSize,
        outSerializedSize,
        diagnosticDestination,
        diagnosticDestinationSize,
        outDiagnosticSize,
        outVerificationStatus);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetIntegerType(
    SlangNVVMModuleHandle module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getIntegerTypeCallCount;
    gFakeNVVMBuilder.integerBitWidth = bitWidth;
    if (module != _getFakeNVVMBuilderModule() || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = gFakeNVVMBuilder.returnNullIntegerType ? nullptr
               : bitWidth == 1                        ? _getFakeNVVMBuilderBooleanType()
                                                      : _getFakeNVVMBuilderIntegerType();
    return gFakeNVVMBuilder.failIntegerTypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFloatingPointType(
    SlangNVVMModuleHandle module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getFloatingPointTypeCallCount;
    gFakeNVVMBuilder.floatingPointBitWidth = bitWidth;
    if (outType)
        *outType = nullptr;
    if (module != _getFakeNVVMBuilderModule() || (bitWidth != 16 && bitWidth != 32) || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = gFakeNVVMBuilder.returnNullFloatingPointType ? nullptr
               : bitWidth == 16                             ? _getFakeNVVMBuilderHalfType()
                                                            : _getFakeNVVMBuilderFloatType();
    return gFakeNVVMBuilder.failFloatingPointTypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetArrayType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getArrayTypeCallCount;
    gFakeNVVMBuilder.arrayElementType = elementType;
    gFakeNVVMBuilder.arrayElementCount = elementCount;
    if (outType)
        *outType = nullptr;
    FakeNVVMBuilderScalarTypeKind elementTypeKind;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderTypeKind(elementType, elementTypeKind) ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::NumericArray ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::ResourceView ||
        elementTypeKind == FakeNVVMBuilderScalarTypeKind::ScalarStructPointer ||
        elementCount == 0 || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    *outType = gFakeNVVMBuilder.returnNullArrayType ? nullptr : _getFakeNVVMBuilderArrayType();
    return gFakeNVVMBuilder.failArrayTypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetPointerType(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle pointeeType,
    SlangNVVMAddressSpace addressSpace,
    SlangNVVMTypeHandle* outType)
{
    ++gFakeNVVMBuilder.getPointerTypeCallCount;
    gFakeNVVMBuilder.pointerAddressSpace = addressSpace;
    uint32_t vectorElementCount = 0;
    FakeNVVMBuilderScalarTypeKind vectorElementTypeKind;
    const bool isVector =
        _getFakeNVVMBuilderVectorTypeInfo(pointeeType, vectorElementCount, vectorElementTypeKind);
    if (module != _getFakeNVVMBuilderModule() ||
        (pointeeType != _getFakeNVVMBuilderIntegerType() &&
         pointeeType != _getFakeNVVMBuilderFloatType() &&
         pointeeType != _getFakeNVVMBuilderArrayType() &&
         pointeeType != _getFakeNVVMBuilderScalarStructType() &&
         (!isVector || vectorElementTypeKind == FakeNVVMBuilderScalarTypeKind::Boolean ||
          vectorElementTypeKind == FakeNVVMBuilderScalarTypeKind::Half)) ||
        !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.pointerPointeeTypes.add(pointeeType);
    gFakeNVVMBuilder.pointerAddressSpaces.add(addressSpace);
    *outType =
        pointeeType == _getFakeNVVMBuilderIntegerType() ? _getFakeNVVMBuilderPointerType()
        : pointeeType == _getFakeNVVMBuilderFloatType() ? _getFakeNVVMBuilderFloatPointerType()
        : pointeeType == _getFakeNVVMBuilderScalarStructType()
            ? _getFakeNVVMBuilderScalarStructPointerType()
        : pointeeType == _getFakeNVVMBuilderArrayType()
            ? _getFakeNVVMBuilderArrayPointerType()
            : _getFakeNVVMBuilderVectorPointerType(vectorElementCount, vectorElementTypeKind);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFunctionParameter(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    size_t parameterIndex,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.getFunctionParameterCallCount;
    gFakeNVVMBuilder.functionParameterIndex = parameterIndex;
    Index functionIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionIndex(function, functionIndex) || parameterIndex >= 8 ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }
    if (functionIndex < gFakeNVVMBuilder.functionTypeIndices.getCount())
    {
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[functionIndex];
        if (parameterIndex >= gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex])
            return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.functionParameterIndices.add(parameterIndex);
    *outValue = _getFakeNVVMBuilderFunctionParameter(functionIndex, Index(parameterIndex));
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderSetFunctionParameterAttributes(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    size_t parameterIndex,
    SlangNVVMParameterFlags flags,
    SlangNVVMTypeHandle pointeeType,
    uint32_t alignment)
{
    ++gFakeNVVMBuilder.setFunctionParameterAttributesCallCount;
    Index functionIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionIndex(function, functionIndex) ||
        functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount() ||
        (flags & ~SLANG_NVVM_PARAMETER_FLAG_BY_VALUE))
    {
        return SLANG_E_INVALID_ARG;
    }

    if (flags == SLANG_NVVM_PARAMETER_FLAG_NONE)
        return !pointeeType && !alignment ? SLANG_OK : SLANG_E_INVALID_ARG;

    const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[functionIndex];
    if (parameterIndex >= gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex])
        return SLANG_E_INVALID_ARG;
    const Index parameterTypeIndex =
        gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex] + parameterIndex;
    if (flags != SLANG_NVVM_PARAMETER_FLAG_BY_VALUE ||
        gFakeNVVMBuilder.functionParameterTypeKinds[parameterTypeIndex] !=
            FakeNVVMBuilderParameterTypeKind::ScalarStructPointer ||
        pointeeType != _getFakeNVVMBuilderScalarStructType() || !alignment ||
        (alignment & (alignment - 1)))
    {
        return SLANG_E_INVALID_ARG;
    }

    for (Index attributeIndex = 0;
         attributeIndex < gFakeNVVMBuilder.parameterAttributeFunctionIndices.getCount();
         ++attributeIndex)
    {
        if (gFakeNVVMBuilder.parameterAttributeFunctionIndices[attributeIndex] == functionIndex &&
            gFakeNVVMBuilder.parameterAttributeIndices[attributeIndex] == parameterIndex)
        {
            return SLANG_E_INVALID_ARG;
        }
    }

    gFakeNVVMBuilder.parameterAttributeFunctionIndices.add(functionIndex);
    gFakeNVVMBuilder.parameterAttributeIndices.add(parameterIndex);
    gFakeNVVMBuilder.parameterAttributeFlags.add(flags);
    gFakeNVVMBuilder.parameterAttributePointeeTypes.add(pointeeType);
    gFakeNVVMBuilder.parameterAttributeAlignments.add(alignment);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitLoad(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle pointer,
    uint32_t alignment,
    SlangNVVMLoadFlags flags,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitLoadCallCount;
    gFakeNVVMBuilder.loadAlignment = alignment;
    gFakeNVVMBuilder.loadAlignments.add(alignment);
    Index pointerFunctionIndex = -1;
    size_t pointerIndex = size_t(-1);
    FakeNVVMBuilderValueRef pointerRef;
    if (module != _getFakeNVVMBuilderModule() || !_isFakeNVVMBuilderPointerValue(pointer) ||
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef) ||
        (flags & ~SLANG_NVVM_LOAD_FLAG_INVARIANT) != SLANG_NVVM_LOAD_FLAG_NONE || !outValue ||
        gFakeNVVMBuilder.loadResultTypeKinds.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.loadStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    FakeNVVMBuilderScalarTypeKind resultTypeKind;
    if (!_getFakeNVVMBuilderPointerScalarTypeKind(pointerRef, resultTypeKind))
        return SLANG_E_INVALID_ARG;

    _getFakeNVVMBuilderParameterRef(pointer, pointerFunctionIndex, pointerIndex);
    const Index resultIndex = gFakeNVVMBuilder.loadResultTypeKinds.getCount();
    gFakeNVVMBuilder.loadPointerParameterIndices.add(pointerIndex);
    gFakeNVVMBuilder.loadPointerValueRefs.add(pointerRef);
    gFakeNVVMBuilder.loadFlags.add(flags);
    gFakeNVVMBuilder.loadResultTypeKinds.add(resultTypeKind);
    *outValue = _getFakeNVVMBuilderLoad(resultIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitStore(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle pointer,
    uint32_t alignment)
{
    ++gFakeNVVMBuilder.emitStoreCallCount;
    gFakeNVVMBuilder.storeAlignment = alignment;
    gFakeNVVMBuilder.storeAlignments.add(alignment);
    Index pointerFunctionIndex = -1;
    size_t pointerIndex = size_t(-1);
    FakeNVVMBuilderValueRef pointerRef;
    FakeNVVMBuilderValueRef valueRef;
    FakeNVVMBuilderScalarTypeKind pointerTypeKind;
    if (module != _getFakeNVVMBuilderModule() || !_getFakeNVVMBuilderValueRef(value, valueRef) ||
        !_isFakeNVVMBuilderPointerValue(pointer) ||
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef) ||
        !_getFakeNVVMBuilderPointerScalarTypeKind(pointerRef, pointerTypeKind) ||
        !_isFakeNVVMBuilderValueOfTypeKind(value, pointerTypeKind))
    {
        return SLANG_E_INVALID_ARG;
    }
    _getFakeNVVMBuilderParameterRef(pointer, pointerFunctionIndex, pointerIndex);
    gFakeNVVMBuilder.storePointerFunctionIndices.add(pointerFunctionIndex);
    gFakeNVVMBuilder.storePointerParameterIndices.add(pointerIndex);
    gFakeNVVMBuilder.storePointerValueRefs.add(pointerRef);
    gFakeNVVMBuilder.storeValueRefs.add(valueRef);
    gFakeNVVMBuilder.storeBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    size_t valueParameterIndex = size_t(-1);
    Index valueParameterFunctionIndex = -1;
    Index valueBinaryIndex = -1;
    if (_getFakeNVVMBuilderParameterRef(value, valueParameterFunctionIndex, valueParameterIndex))
        gFakeNVVMBuilder.storeValueKinds.add(FakeNVVMBuilderValueKind::Parameter);
    else if (valueRef.kind == FakeNVVMBuilderValueKind::Load)
        gFakeNVVMBuilder.storeValueKinds.add(FakeNVVMBuilderValueKind::Load);
    else if (_getFakeNVVMBuilderScalarOperationIndex(value, valueBinaryIndex))
    {
        gFakeNVVMBuilder.storeValueKinds.add(FakeNVVMBuilderValueKind::ScalarOperation);
    }
    else
    {
        gFakeNVVMBuilder.storeValueKinds.add(valueRef.kind);
    }
    gFakeNVVMBuilder.storeValueParameterIndices.add(valueParameterIndex);
    gFakeNVVMBuilder.storeValueBinaryIndices.add(valueBinaryIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitLocalStorage(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle valueType,
    uint32_t alignment,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle* outStorage)
{
    ++gFakeNVVMBuilder.emitLocalStorageCallCount;
    if (outStorage)
        *outStorage = nullptr;
    const Index storageIndex = gFakeNVVMBuilder.localStorageValueTypes.getCount();
    FakeNVVMBuilderScalarTypeKind valueTypeKind;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderTypeKind(valueType, valueTypeKind) ||
        valueTypeKind == FakeNVVMBuilderScalarTypeKind::ResourceView ||
        valueTypeKind == FakeNVVMBuilderScalarTypeKind::ScalarStructPointer || !alignment ||
        (alignment & (alignment - 1)) || (!name && nameSize) || !outStorage ||
        gFakeNVVMBuilder.currentInsertBlockIndex < 0 || storageIndex < 0 ||
        storageIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.localStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.localStorageValueTypes.add(valueType);
    gFakeNVVMBuilder.localStorageAlignments.add(alignment);
    gFakeNVVMBuilder.localStorageNames.add(String(UnownedStringSlice(name, nameSize)));
    *outStorage = _getFakeNVVMBuilderLocalStorage(storageIndex);
    return SLANG_OK;
}

static SlangResult _recordFakeNVVMBuilderScalarOperation(
    SlangNVVMModuleHandle module,
    FakeNVVMBuilderScalarOperationKey key,
    const SlangNVVMValueHandle* operands,
    uint32_t operandCount,
    SlangNVVMValueHandle* outValue,
    const SlangNVVMValueTypeDesc* resultType = nullptr,
    const SlangNVVMValueTypeDesc* operandTypes = nullptr)
{
    SLANG_ASSERT(key.family < FakeNVVMBuilderScalarFamily::Count);
    SLANG_ASSERT(key.operation < SLANG_COUNT_OF(gFakeNVVMBuilder.scalarOperationCallCounts[0]));
    ++gFakeNVVMBuilder.scalarFamilyCallCounts[Index(key.family)];
    ++gFakeNVVMBuilder.scalarOperationCallCounts[Index(key.family)][key.operation];
    if (outValue)
        *outValue = nullptr;

    FakeNVVMBuilderScalarOperation recorded = {};
    recorded.key = key;
    if (resultType)
        recorded.resultType = *resultType;
    recorded.callerBlockIndex = gFakeNVVMBuilder.currentInsertBlockIndex;
    recorded.operandCount = operandCount;
    if (module != _getFakeNVVMBuilderModule() || recorded.callerBlockIndex < 0 || !outValue ||
        !operands || operandCount == 0 || operandCount > SLANG_COUNT_OF(recorded.operands) ||
        gFakeNVVMBuilder.scalarOperations.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.scalarOperationStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    for (uint32_t i = 0; i < operandCount; ++i)
    {
        if (operandTypes)
        {
            const SlangNVVMValueTypeDesc& operandType = operandTypes[i];
            recorded.operandTypes[i] = operandType;
            bool isExpectedType = false;
            if (operandType.laneCount >= 2 && operandType.laneCount <= 4)
            {
                const FakeNVVMBuilderScalarTypeKind elementTypeKind =
                    operandType.kind == SLANG_NVVM_VALUE_TYPE_BOOL
                        ? FakeNVVMBuilderScalarTypeKind::Boolean
                    : operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT
                        ? operandType.bitWidth == 16 ? FakeNVVMBuilderScalarTypeKind::Half
                                                     : FakeNVVMBuilderScalarTypeKind::Float
                        : FakeNVVMBuilderScalarTypeKind::Integer;
                isExpectedType = _isFakeNVVMBuilderVectorValue(
                    operands[i],
                    elementTypeKind,
                    operandType.laneCount);
            }
            else if (operandType.kind == SLANG_NVVM_VALUE_TYPE_BOOL)
                isExpectedType = _isFakeNVVMBuilderBooleanValue(operands[i]);
            else if (operandType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
                isExpectedType =
                    _isFakeNVVMBuilderFloatingPointValue(operands[i], operandType.bitWidth);
            else
                isExpectedType = _isFakeNVVMBuilderIntegerValue(operands[i]);

            if (!isExpectedType || !_getFakeNVVMBuilderValueRef(operands[i], recorded.operands[i]))
            {
                return SLANG_E_INVALID_ARG;
            }
            continue;
        }

        const bool isIntegerToFloat = key.operation == SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT;
        const bool isFloatToInteger = key.operation == SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER;
        const bool isFloating =
            isFloatToInteger ||
            (!isIntegerToFloat && (key.family == FakeNVVMBuilderScalarFamily::FloatingUnary ||
                                   key.family == FakeNVVMBuilderScalarFamily::FloatingBinary ||
                                   key.family == FakeNVVMBuilderScalarFamily::FloatingCompare));
        const bool isIntegerVector = resultType &&
                                     (resultType->kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
                                      resultType->kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
                                     resultType->bitWidth == 32 && resultType->laneCount >= 2 &&
                                     resultType->laneCount <= 4;
        if ((isFloating ? !_isFakeNVVMBuilderFloatingPointValue(operands[i], 32)
             : isIntegerVector
                 ? !_isFakeNVVMBuilderIntegerVectorValue(operands[i], resultType->laneCount)
                 : !_isFakeNVVMBuilderIntegerValue(operands[i])) ||
            !_getFakeNVVMBuilderValueRef(operands[i], recorded.operands[i]))
        {
            return SLANG_E_INVALID_ARG;
        }
    }

    const Index resultIndex = gFakeNVVMBuilder.scalarOperations.getCount();
    gFakeNVVMBuilder.scalarOperations.add(recorded);
    if (!_isFakeNVVMBuilderScalarOperation(
            gFakeNVVMBuilder.returnNullScalarOperation,
            key.family,
            key.operation))
    {
        *outValue = _getFakeNVVMBuilderScalarOperation(resultIndex);
    }
    return _isFakeNVVMBuilderScalarOperation(
               gFakeNVVMBuilder.failScalarOperationAfterWrite,
               key.family,
               key.operation)
               ? SLANG_FAIL
               : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedLessThan(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueHandle operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_VALUE_OP_LESS_THAN},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderEmitBranch(SlangNVVMModuleHandle module, SlangNVVMBlockHandle targetBlock)
{
    ++gFakeNVVMBuilder.emitBranchCallCount;
    Index targetIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderBlockIndex(targetBlock, targetIndex))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.branchSourceBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.branchTargetBlockIndices.add(targetIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitConditionalBranch(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle condition,
    SlangNVVMBlockHandle trueBlock,
    SlangNVVMBlockHandle falseBlock)
{
    ++gFakeNVVMBuilder.emitConditionalBranchCallCount;
    Index trueIndex = -1;
    Index falseIndex = -1;
    if (module != _getFakeNVVMBuilderModule() || !_isFakeNVVMBuilderBooleanValue(condition) ||
        !_getFakeNVVMBuilderBlockIndex(trueBlock, trueIndex) ||
        !_getFakeNVVMBuilderBlockIndex(falseBlock, falseIndex))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.conditionalSourceBlockIndex = gFakeNVVMBuilder.currentInsertBlockIndex;
    gFakeNVVMBuilder.conditionalTrueBlockIndex = trueIndex;
    gFakeNVVMBuilder.conditionalFalseBlockIndex = falseIndex;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitSwitch(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle condition,
    const SlangNVVMValueHandle* caseValues,
    const SlangNVVMBlockHandle* caseBlocks,
    size_t caseCount,
    SlangNVVMBlockHandle defaultBlock)
{
    ++gFakeNVVMBuilder.emitSwitchCallCount;
    Index defaultIndex = -1;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderIntegerValue(condition) ||
        !_getFakeNVVMBuilderBlockIndex(defaultBlock, defaultIndex) ||
        (caseCount && (!caseValues || !caseBlocks)))
    {
        return SLANG_E_INVALID_ARG;
    }
    for (size_t i = 0; i < caseCount; ++i)
    {
        Index caseBlockIndex = -1;
        if (!_isFakeNVVMBuilderIntegerValue(caseValues[i]) ||
            !_getFakeNVVMBuilderBlockIndex(caseBlocks[i], caseBlockIndex))
        {
            return SLANG_E_INVALID_ARG;
        }
    }
    gFakeNVVMBuilder.lastSwitchCaseCount = caseCount;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetIntegerConstant(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle type,
    int64_t value,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.getIntegerConstantCallCount;
    const uint32_t bitWidth = type == _getFakeNVVMBuilderBooleanType() ? 1u : 32u;
    if (module != _getFakeNVVMBuilderModule() ||
        (type != _getFakeNVVMBuilderIntegerType() && type != _getFakeNVVMBuilderBooleanType()) ||
        (bitWidth == 1 && value != 0 && value != 1) || !outValue ||
        gFakeNVVMBuilder.integerConstantValues.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.integerConstantStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.integerConstantValues.getCount();
    gFakeNVVMBuilder.integerConstantValues.add(value);
    gFakeNVVMBuilder.integerConstantBitWidths.add(bitWidth);
    *outValue = _getFakeNVVMBuilderIntegerConstant(resultIndex);
    return gFakeNVVMBuilder.failIntegerConstantAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitPhi(
    SlangNVVMModuleHandle module,
    SlangNVVMBlockHandle targetBlock,
    SlangNVVMTypeHandle type,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;

    ++gFakeNVVMBuilder.emitPhiCallCount;

    Index targetIndex = -1;
    FakeNVVMBuilderScalarTypeKind typeKind;
    if (!_getFakeNVVMBuilderTypeKind(type, typeKind) ||
        typeKind == FakeNVVMBuilderScalarTypeKind::ResourceView ||
        typeKind == FakeNVVMBuilderScalarTypeKind::ScalarStructPointer)
    {
        return SLANG_E_INVALID_ARG;
    }
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderBlockIndex(targetBlock, targetIndex) || !outValue ||
        gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.scalarPhiStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.scalarPhiTargetBlockIndices.getCount();
    gFakeNVVMBuilder.scalarPhiTargetBlockIndices.add(targetIndex);
    gFakeNVVMBuilder.scalarPhiTypes.add(type);
    *outValue = _getFakeNVVMBuilderScalarPhi(resultIndex);
    return gFakeNVVMBuilder.failScalarPhiAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderAddPhiIncoming(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle phi,
    SlangNVVMValueHandle value,
    SlangNVVMBlockHandle predecessorBlock)
{
    ++gFakeNVVMBuilder.addPhiIncomingCallCount;
    Index phiIndex = -1;
    Index predecessorIndex = -1;
    FakeNVVMBuilderValueRef valueRef;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderScalarPhiIndex(phi, phiIndex) ||
        !_getFakeNVVMBuilderValueRef(value, valueRef) ||
        !_getFakeNVVMBuilderBlockIndex(predecessorBlock, predecessorIndex) ||
        phiIndex >= gFakeNVVMBuilder.scalarPhiTypes.getCount() ||
        !_isFakeNVVMBuilderValueOfType(value, gFakeNVVMBuilder.scalarPhiTypes[phiIndex]))
    {
        return SLANG_E_INVALID_ARG;
    }

    gFakeNVVMBuilder.scalarPhiIncomingPhiIndices.add(phiIndex);
    gFakeNVVMBuilder.scalarPhiIncomingValueRefs.add(valueRef);
    gFakeNVVMBuilder.scalarPhiIncomingPredecessorBlockIndices.add(predecessorIndex);
    return SLANG_OK;
}

static bool _isFakeNVVMBuilderFunctionArgument(
    SlangNVVMValueHandle value,
    SlangNVVMTypeHandle parameterType,
    bool requireInteger)
{
    if (requireInteger)
    {
        return parameterType == _getFakeNVVMBuilderIntegerType() &&
               _isFakeNVVMBuilderIntegerValue(value);
    }
    return _isFakeNVVMBuilderValueOfType(value, parameterType);
}

static SlangResult _fakeNVVMBuilderEmitCallImpl(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle callee,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle* outValue,
    bool requireInteger,
    bool failAfterWrite)
{
    if (outValue)
        *outValue = nullptr;

    Index calleeFunctionIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionIndex(callee, calleeFunctionIndex) ||
        calleeFunctionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount() ||
        gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        gFakeNVVMBuilder.currentInsertBlockIndex >=
            gFakeNVVMBuilder.blockFunctionIndices.getCount() ||
        (!arguments && argumentCount) || !outValue ||
        gFakeNVVMBuilder.callCalleeFunctionIndices.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.callStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[calleeFunctionIndex];
    const FakeNVVMBuilderResultTypeKind resultKind =
        gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex];
    const bool isGenericResult = resultKind == FakeNVVMBuilderResultTypeKind::Void ||
                                 resultKind == FakeNVVMBuilderResultTypeKind::Integer ||
                                 resultKind == FakeNVVMBuilderResultTypeKind::Boolean ||
                                 resultKind == FakeNVVMBuilderResultTypeKind::Half ||
                                 resultKind == FakeNVVMBuilderResultTypeKind::Float ||
                                 resultKind == FakeNVVMBuilderResultTypeKind::ValueVector ||
                                 resultKind == FakeNVVMBuilderResultTypeKind::ScalarStruct;
    if ((requireInteger ? resultKind != FakeNVVMBuilderResultTypeKind::Integer
                        : !isGenericResult) ||
        gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] != argumentCount)
    {
        return SLANG_E_INVALID_ARG;
    }

    List<FakeNVVMBuilderValueRef> argumentRefs;
    for (size_t i = 0; i < argumentCount; ++i)
    {
        FakeNVVMBuilderValueRef argumentRef;
        const Index parameterTypeKindIndex =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex] + Index(i);
        if (parameterTypeKindIndex < 0 ||
            parameterTypeKindIndex >= gFakeNVVMBuilder.functionParameterTypes.getCount() ||
            !_isFakeNVVMBuilderFunctionArgument(
                arguments[i],
                gFakeNVVMBuilder.functionParameterTypes[parameterTypeKindIndex],
                requireInteger) ||
            !_getFakeNVVMBuilderValueRef(arguments[i], argumentRef))
        {
            return SLANG_E_INVALID_ARG;
        }
        argumentRefs.add(argumentRef);
    }

    const Index resultIndex = gFakeNVVMBuilder.callCalleeFunctionIndices.getCount();
    gFakeNVVMBuilder.callCalleeFunctionIndices.add(calleeFunctionIndex);
    gFakeNVVMBuilder.callCallerBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.callArgumentOffsets.add(gFakeNVVMBuilder.callArgumentValueRefs.getCount());
    gFakeNVVMBuilder.callArgumentCounts.add(argumentCount);
    gFakeNVVMBuilder.callArgumentValueRefs.addRange(
        argumentRefs.getBuffer(),
        argumentRefs.getCount());
    SlangNVVMTypeHandle resultType = gFakeNVVMBuilder.functionTypeResultTypes[functionTypeIndex];
    gFakeNVVMBuilder.callResultKinds.add(resultKind);
    gFakeNVVMBuilder.callResultTypes.add(resultType);
    *outValue = _getFakeNVVMBuilderCall(resultIndex);
    return failAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerCall(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle callee,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerCallCallCount;
    return _fakeNVVMBuilderEmitCallImpl(
        module,
        callee,
        arguments,
        argumentCount,
        outValue,
        true,
        gFakeNVVMBuilder.failIntegerCallAfterWrite);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitCall(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle callee,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitCallCallCount;
    return _fakeNVVMBuilderEmitCallImpl(
        module,
        callee,
        arguments,
        argumentCount,
        outValue,
        false,
        gFakeNVVMBuilder.failCallAfterWrite);
}

static SlangResult _fakeNVVMBuilderEmitValueReturnImpl(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    bool requireInteger,
    bool recordIntegerReturn,
    bool fail)
{
    FakeNVVMBuilderValueRef valueRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        gFakeNVVMBuilder.currentInsertBlockIndex >=
            gFakeNVVMBuilder.blockFunctionIndices.getCount() ||
        !_getFakeNVVMBuilderValueRef(value, valueRef))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index functionIndex =
        gFakeNVVMBuilder.blockFunctionIndices[gFakeNVVMBuilder.currentInsertBlockIndex];
    if (functionIndex < 0 || functionIndex >= gFakeNVVMBuilder.functionTypeIndices.getCount())
        return SLANG_E_INVALID_ARG;
    const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[functionIndex];
    const FakeNVVMBuilderResultTypeKind resultKind =
        gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex];
    const SlangNVVMTypeHandle resultType =
        gFakeNVVMBuilder.functionTypeResultTypes[functionTypeIndex];
    const bool isExactValue = requireInteger
                                  ? resultKind == FakeNVVMBuilderResultTypeKind::Integer &&
                                        _isFakeNVVMBuilderIntegerValue(value)
                                  : _isFakeNVVMBuilderValueOfType(value, resultType);
    if (!isExactValue || (requireInteger && resultKind != FakeNVVMBuilderResultTypeKind::Integer))
    {
        return SLANG_E_INVALID_ARG;
    }

    if (fail)
        return SLANG_FAIL;
    if (recordIntegerReturn)
    {
        gFakeNVVMBuilder.integerReturnBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
        gFakeNVVMBuilder.integerReturnValueRefs.add(valueRef);
    }
    else
    {
        gFakeNVVMBuilder.scalarReturnBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
        gFakeNVVMBuilder.scalarReturnValueRefs.add(valueRef);
    }
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderEmitIntegerReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value)
{
    ++gFakeNVVMBuilder.emitIntegerReturnCallCount;
    return _fakeNVVMBuilderEmitValueReturnImpl(
        module,
        value,
        true,
        true,
        gFakeNVVMBuilder.failIntegerReturn);
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderEmitValueReturn(SlangNVVMModuleHandle module, SlangNVVMValueHandle value)
{
    ++gFakeNVVMBuilder.emitValueReturnCallCount;
    return _fakeNVVMBuilderEmitValueReturnImpl(
        module,
        value,
        false,
        false,
        gFakeNVVMBuilder.failValueReturn);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntrinsic(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc& operation,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitIntrinsicCallCount;
    if (outValue)
        *outValue = nullptr;
    const size_t expectedArgumentCount = operation.operandCount;
    if (module != _getFakeNVVMBuilderModule() || argumentCount != expectedArgumentCount ||
        (!arguments && argumentCount) || !outValue ||
        gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        gFakeNVVMBuilder.intrinsicOperations.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.intrinsicStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    FakeNVVMBuilderValueRef argumentRefs[3];
    for (Index i = 0; i < Index(argumentCount); ++i)
    {
        if (!_getFakeNVVMBuilderValueRef(arguments[i], argumentRefs[i]))
            return SLANG_E_INVALID_ARG;
        const SlangNVVMValueTypeKind kind = operation.operandTypes[i].kind;
        const bool typeMatches = kind == SLANG_NVVM_VALUE_TYPE_BOOL
                                     ? _isFakeNVVMBuilderBooleanValue(arguments[i])
                                 : kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT
                                     ? _isFakeNVVMBuilderFloatingPointValue(arguments[i], 32)
                                     : _isFakeNVVMBuilderIntegerValue(arguments[i]);
        if (!typeMatches)
            return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.intrinsicOperations.getCount();
    gFakeNVVMBuilder.intrinsicOperations.add(operation.operation);
    gFakeNVVMBuilder.intrinsicResultTypes.add(operation.resultType);
    gFakeNVVMBuilder.intrinsicCallerBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.intrinsicArgumentOffsets.add(
        gFakeNVVMBuilder.intrinsicArgumentValueRefs.getCount());
    gFakeNVVMBuilder.intrinsicArgumentCounts.add(argumentCount);
    gFakeNVVMBuilder.intrinsicArgumentValueRefs.addRange(argumentRefs, Index(argumentCount));
    if (!gFakeNVVMBuilder.returnNullIntrinsic)
        *outValue = _getFakeNVVMBuilderIntrinsic(resultIndex);
    return gFakeNVVMBuilder.failIntrinsicAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitPointerOffset(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle basePointer,
    SlangNVVMValueHandle elementOffset,
    SlangNVVMValueHandle* outPointer)
{
    ++gFakeNVVMBuilder.emitPointerOffsetCallCount;
    if (outPointer)
        *outPointer = nullptr;

    FakeNVVMBuilderValueRef baseRef;
    FakeNVVMBuilderValueRef elementRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderPointerValue(basePointer) ||
        !_getFakeNVVMBuilderValueRef(basePointer, baseRef) ||
        !_isFakeNVVMBuilderIntegerValue(elementOffset) ||
        !_getFakeNVVMBuilderValueRef(elementOffset, elementRef) || !outPointer ||
        gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.pointerOffsetStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount();
    gFakeNVVMBuilder.pointerOffsetCallerBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.pointerOffsetBaseValueRefs.add(baseRef);
    gFakeNVVMBuilder.pointerOffsetElementValueRefs.add(elementRef);
    *outPointer = _getFakeNVVMBuilderPointerOffset(resultIndex);
    return gFakeNVVMBuilder.failPointerOffsetAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitByteOffsetPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle basePointer,
    SlangNVVMValueHandle byteOffset,
    SlangNVVMTypeHandle resultPointeeType,
    SlangNVVMValueHandle* outPointer)
{
    ++gFakeNVVMBuilder.emitByteOffsetPointerCallCount;
    if (outPointer)
        *outPointer = nullptr;

    FakeNVVMBuilderValueRef baseRef;
    FakeNVVMBuilderValueRef offsetRef;
    FakeNVVMBuilderScalarTypeKind resultTypeKind;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderPointerValue(basePointer) ||
        !_getFakeNVVMBuilderValueRef(basePointer, baseRef) ||
        !_isFakeNVVMBuilderIntegerValue(byteOffset) ||
        !_getFakeNVVMBuilderValueRef(byteOffset, offsetRef) ||
        !_getFakeNVVMBuilderTypeKind(resultPointeeType, resultTypeKind) || !outPointer ||
        gFakeNVVMBuilder.byteOffsetPointerBaseValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.byteOffsetPointerStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.byteOffsetPointerBaseValueRefs.getCount();
    gFakeNVVMBuilder.byteOffsetPointerCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.byteOffsetPointerBaseValueRefs.add(baseRef);
    gFakeNVVMBuilder.byteOffsetPointerOffsetValueRefs.add(offsetRef);
    gFakeNVVMBuilder.byteOffsetPointerPointeeTypes.add(resultPointeeType);
    gFakeNVVMBuilder.byteOffsetPointerTypeKinds.add(resultTypeKind);
    *outPointer = _getFakeNVVMBuilderByteOffsetPointer(resultIndex);
    return gFakeNVVMBuilder.failByteOffsetPointerAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitSequentialElementPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseSequentialPointer,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle* outPointer)
{
    ++gFakeNVVMBuilder.emitSequentialElementPointerCallCount;
    if (outPointer)
        *outPointer = nullptr;

    FakeNVVMBuilderValueRef baseRef;
    FakeNVVMBuilderValueRef indexRef;
    FakeNVVMBuilderScalarTypeKind resultTypeKind;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_getFakeNVVMBuilderSequentialElementTypeKind(baseSequentialPointer, resultTypeKind) ||
        !_getFakeNVVMBuilderValueRef(baseSequentialPointer, baseRef) ||
        !_isFakeNVVMBuilderIntegerValue(elementIndex) ||
        !_getFakeNVVMBuilderValueRef(elementIndex, indexRef) || !outPointer ||
        gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.sequentialElementPointerStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs.getCount();
    gFakeNVVMBuilder.sequentialElementPointerCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.sequentialElementPointerBaseValueRefs.add(baseRef);
    gFakeNVVMBuilder.sequentialElementPointerIndexValueRefs.add(indexRef);
    gFakeNVVMBuilder.sequentialElementPointerTypeKinds.add(resultTypeKind);
    *outPointer = gFakeNVVMBuilder.returnNullSequentialElementPointer
                      ? nullptr
                      : _getFakeNVVMBuilderSequentialElementPointer(resultIndex);
    return gFakeNVVMBuilder.failSequentialElementPointerAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitStructFieldPointer(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle baseStructPointer,
    uint32_t fieldIndex,
    SlangNVVMValueHandle* outPointer)
{
    ++gFakeNVVMBuilder.emitStructFieldPointerCallCount;
    if (outPointer)
        *outPointer = nullptr;

    FakeNVVMBuilderValueRef baseRef;
    SlangNVVMTypeHandle fieldType = nullptr;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_getFakeNVVMBuilderValueRef(baseStructPointer, baseRef) || !outPointer ||
        gFakeNVVMBuilder.structFieldPointerBaseValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.structFieldPointerStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    if (baseRef.kind == FakeNVVMBuilderValueKind::GlobalStorage &&
        gFakeNVVMBuilder.globalStorageValueType == _getFakeNVVMBuilderStructType() &&
        fieldIndex < uint32_t(gFakeNVVMBuilder.structFieldTypes.getCount()))
    {
        fieldType = gFakeNVVMBuilder.structFieldTypes[fieldIndex];
    }
    else
    {
        FakeNVVMBuilderScalarTypeKind basePointeeTypeKind;
        if (_getFakeNVVMBuilderPointerScalarTypeKind(baseRef, basePointeeTypeKind) &&
            basePointeeTypeKind == FakeNVVMBuilderScalarTypeKind::ScalarStruct &&
            fieldIndex < uint32_t(gFakeNVVMBuilder.scalarStructFieldTypes.getCount()))
        {
            fieldType = gFakeNVVMBuilder.scalarStructFieldTypes[fieldIndex];
        }
    }
    FakeNVVMBuilderScalarTypeKind fieldTypeKind;
    if (!fieldType || !_getFakeNVVMBuilderTypeKind(fieldType, fieldTypeKind))
        return SLANG_E_INVALID_ARG;

    const Index resultIndex = gFakeNVVMBuilder.structFieldPointerBaseValueRefs.getCount();
    gFakeNVVMBuilder.structFieldPointerBaseValueRefs.add(baseRef);
    gFakeNVVMBuilder.structFieldPointerIndices.add(fieldIndex);
    gFakeNVVMBuilder.structFieldPointerTypeKinds.add(fieldTypeKind);
    *outPointer = _getFakeNVVMBuilderStructFieldPointer(resultIndex);
    return SLANG_OK;
}

static bool _getFakeNVVMBuilderAggregateElementType(
    SlangNVVMTypeHandle aggregateType,
    uint32_t elementIndex,
    SlangNVVMTypeHandle& outElementType)
{
    outElementType = nullptr;
    if (aggregateType == _getFakeNVVMBuilderArrayType())
    {
        if (elementIndex >= gFakeNVVMBuilder.arrayElementCount)
            return false;
        outElementType = gFakeNVVMBuilder.arrayElementType;
        return outElementType != nullptr;
    }
    if (aggregateType == _getFakeNVVMBuilderStructType() &&
        elementIndex < uint32_t(gFakeNVVMBuilder.structFieldTypes.getCount()))
    {
        outElementType = gFakeNVVMBuilder.structFieldTypes[elementIndex];
        return true;
    }
    if (aggregateType == _getFakeNVVMBuilderScalarStructType() &&
        elementIndex < uint32_t(gFakeNVVMBuilder.scalarStructFieldTypes.getCount()))
    {
        outElementType = gFakeNVVMBuilder.scalarStructFieldTypes[elementIndex];
        return true;
    }
    return false;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitAggregateConstruct(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle aggregateType,
    const SlangNVVMValueHandle* elements,
    size_t elementCount,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitAggregateConstructCallCount;
    if (outValue)
        *outValue = nullptr;

    const size_t expectedElementCount =
        aggregateType == _getFakeNVVMBuilderArrayType() ? size_t(gFakeNVVMBuilder.arrayElementCount)
        : aggregateType == _getFakeNVVMBuilderStructType()
            ? size_t(gFakeNVVMBuilder.structFieldTypes.getCount())
        : aggregateType == _getFakeNVVMBuilderScalarStructType()
            ? size_t(gFakeNVVMBuilder.scalarStructFieldTypes.getCount())
            : 0;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !expectedElementCount || expectedElementCount != elementCount || !elements || !outValue ||
        gFakeNVVMBuilder.aggregateConstructResultTypes.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.aggregateConstructStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    List<FakeNVVMBuilderValueRef> elementRefs;
    for (size_t i = 0; i < elementCount; ++i)
    {
        SlangNVVMTypeHandle elementType = nullptr;
        FakeNVVMBuilderValueRef elementRef;
        if (!_getFakeNVVMBuilderAggregateElementType(aggregateType, uint32_t(i), elementType) ||
            !_isFakeNVVMBuilderValueOfType(elements[i], elementType) ||
            !_getFakeNVVMBuilderValueRef(elements[i], elementRef))
        {
            return SLANG_E_INVALID_ARG;
        }
        elementRefs.add(elementRef);
    }

    const Index resultIndex = gFakeNVVMBuilder.aggregateConstructResultTypes.getCount();
    const Index elementOffset = gFakeNVVMBuilder.aggregateConstructElementValueRefs.getCount();
    gFakeNVVMBuilder.aggregateConstructResultTypes.add(aggregateType);
    gFakeNVVMBuilder.aggregateConstructElementOffsets.add(elementOffset);
    gFakeNVVMBuilder.aggregateConstructElementCounts.add(elementCount);
    gFakeNVVMBuilder.aggregateConstructElementValueRefs.addRange(elementRefs);
    *outValue = _getFakeNVVMBuilderAggregateConstruct(resultIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitAggregateElementExtract(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle aggregateValue,
    uint32_t elementIndex,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitAggregateElementExtractCallCount;
    if (outValue)
        *outValue = nullptr;

    FakeNVVMBuilderValueRef baseRef;
    FakeNVVMBuilderScalarTypeKind elementTypeKind;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_getFakeNVVMBuilderValueRef(aggregateValue, baseRef) || !outValue ||
        gFakeNVVMBuilder.aggregateElementBaseValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.aggregateElementStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    SlangNVVMTypeHandle aggregateType = nullptr;
    if (baseRef.kind == FakeNVVMBuilderValueKind::AggregateConstruct && baseRef.index >= 0 &&
        baseRef.index < gFakeNVVMBuilder.aggregateConstructResultTypes.getCount())
    {
        aggregateType = gFakeNVVMBuilder.aggregateConstructResultTypes[baseRef.index];
    }
    else if (
        baseRef.kind == FakeNVVMBuilderValueKind::ScalarPhi && baseRef.index >= 0 &&
        baseRef.index < gFakeNVVMBuilder.scalarPhiTypes.getCount())
    {
        aggregateType = gFakeNVVMBuilder.scalarPhiTypes[baseRef.index];
    }
    else if (
        baseRef.kind == FakeNVVMBuilderValueKind::Call && baseRef.index >= 0 &&
        baseRef.index < gFakeNVVMBuilder.callResultTypes.getCount())
    {
        aggregateType = gFakeNVVMBuilder.callResultTypes[baseRef.index];
    }
    else if (baseRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        _getFakeNVVMBuilderParameterType(baseRef, aggregateType);
    }
    else if (
        baseRef.kind == FakeNVVMBuilderValueKind::Load && baseRef.index >= 0 &&
        baseRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount())
    {
        const FakeNVVMBuilderScalarTypeKind loadTypeKind =
            gFakeNVVMBuilder.loadResultTypeKinds[baseRef.index];
        aggregateType = loadTypeKind == FakeNVVMBuilderScalarTypeKind::NumericArray
                            ? _getFakeNVVMBuilderArrayType()
                        : loadTypeKind == FakeNVVMBuilderScalarTypeKind::ScalarStruct
                            ? _getFakeNVVMBuilderScalarStructType()
                            : nullptr;
    }

    SlangNVVMTypeHandle elementType = nullptr;
    bool isAggregateElement = false;
    if (aggregateType &&
        _getFakeNVVMBuilderAggregateElementType(aggregateType, elementIndex, elementType) &&
        _getFakeNVVMBuilderTypeKind(elementType, elementTypeKind))
    {
        isAggregateElement = true;
    }
    else if (
        elementIndex != 0 || !_isFakeNVVMBuilderResourceViewValue(aggregateValue) ||
        !_getFakeNVVMBuilderResourceViewElementTypeKind(baseRef, elementTypeKind))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.aggregateElementBaseValueRefs.getCount();
    gFakeNVVMBuilder.aggregateElementBaseValueRefs.add(baseRef);
    gFakeNVVMBuilder.aggregateElementIndices.add(elementIndex);
    gFakeNVVMBuilder.aggregateElementTypeKinds.add(elementTypeKind);
    gFakeNVVMBuilder.aggregateElementIsFirstClassValue.add(isAggregateElement);
    *outValue = _getFakeNVVMBuilderAggregateElement(resultIndex);
    return SLANG_OK;
}

static SlangResult _recordFakeNVVMBuilderUnaryOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueHandle operands[] = {value};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Unary, operation},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult _recordFakeNVVMBuilderBinaryOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueHandle operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Binary, operation},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerMultiply(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_VALUE_OP_MULTIPLY,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitAnd(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_VALUE_OP_BIT_AND,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitOr(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_VALUE_OP_BIT_OR,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitXor(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_VALUE_OP_BIT_XOR,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitNot(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderUnaryOperation(
        module,
        SLANG_NVVM_VALUE_OP_BIT_NOT,
        value,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerNegate(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderUnaryOperation(
        module,
        SLANG_NVVM_VALUE_OP_NEGATE,
        value,
        outValue);
}
static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitRelaxedGlobalI32AtomicAdd(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle pointer,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outOldValue)
{
    ++gFakeNVVMBuilder.emitRelaxedGlobalI32AtomicAddCallCount;
    if (outOldValue)
        *outOldValue = nullptr;

    FakeNVVMBuilderValueRef pointerRef;
    FakeNVVMBuilderValueRef valueRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderPointerValue(pointer) ||
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef) ||
        !_isFakeNVVMBuilderIntegerValue(value) || !_getFakeNVVMBuilderValueRef(value, valueRef) ||
        !outOldValue ||
        gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs.getCount();
    gFakeNVVMBuilder.relaxedGlobalI32AtomicAddCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.relaxedGlobalI32AtomicAddPointerValueRefs.add(pointerRef);
    gFakeNVVMBuilder.relaxedGlobalI32AtomicAddValueRefs.add(valueRef);
    *outOldValue = gFakeNVVMBuilder.returnNullRelaxedGlobalI32AtomicAdd
                       ? nullptr
                       : _getFakeNVVMBuilderRelaxedGlobalI32AtomicAdd(resultIndex);
    return gFakeNVVMBuilder.failRelaxedGlobalI32AtomicAddAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult _recordFakeNVVMBuilderCompareOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueHandle operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Compare, operation},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_VALUE_OP_EQUAL,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerNotEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedGreaterThan(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedLessEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_VALUE_OP_LESS_EQUAL,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedGreaterEqual(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
        left,
        right,
        outValue);
}
static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Unary)];
    gFakeNVVMBuilder.emittedValueOperations.add(
        {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation)});
    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_BIT_NOT:
        return _recordFakeNVVMBuilderUnaryOperation(module, operation, value, outValue);
    case SLANG_NVVM_VALUE_OP_NEGATE:
        return _recordFakeNVVMBuilderUnaryOperation(module, operation, value, outValue);
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBinaryOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)];
    gFakeNVVMBuilder.emittedValueOperations.add(
        {FakeNVVMBuilderScalarFamily::Binary, uint32_t(operation)});
    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_ADD:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_SUBTRACT:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_MULTIPLY:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_BIT_AND:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_BIT_OR:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_BIT_XOR:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerCompare(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)];
    gFakeNVVMBuilder.emittedValueOperations.add(
        {FakeNVVMBuilderScalarFamily::Compare, uint32_t(operation)});
    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_LESS_THAN:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_NOT_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_GREATER_THAN:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_LESS_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_VALUE_OP_GREATER_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitFloatingBinary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder
          .valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingBinary)];
    gFakeNVVMBuilder.emittedValueOperations.add(
        {FakeNVVMBuilderScalarFamily::FloatingBinary, uint32_t(operation)});
    if (operation != SLANG_NVVM_VALUE_OP_ADD && operation != SLANG_NVVM_VALUE_OP_SUBTRACT &&
        operation != SLANG_NVVM_VALUE_OP_MULTIPLY && operation != SLANG_NVVM_VALUE_OP_DIVIDE)
    {
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
    const SlangNVVMValueHandle operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::FloatingBinary, uint32_t(operation)},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitFloatingUnary(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle value,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder
          .valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingUnary)];
    gFakeNVVMBuilder.emittedValueOperations.add(
        {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation)});
    if (operation != SLANG_NVVM_VALUE_OP_NEGATE)
    {
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
    const SlangNVVMValueHandle operands[] = {value};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation)},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitFloatingCompare(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder
          .valueOperationFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingCompare)];
    gFakeNVVMBuilder.emittedValueOperations.add(
        {FakeNVVMBuilderScalarFamily::FloatingCompare, uint32_t(operation)});
    if (operation != SLANG_NVVM_VALUE_OP_EQUAL && operation != SLANG_NVVM_VALUE_OP_NOT_EQUAL &&
        operation != SLANG_NVVM_VALUE_OP_GREATER_THAN &&
        operation != SLANG_NVVM_VALUE_OP_LESS_EQUAL &&
        operation != SLANG_NVVM_VALUE_OP_GREATER_EQUAL &&
        operation != SLANG_NVVM_VALUE_OP_LESS_THAN)
    {
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
    const SlangNVVMValueHandle operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::FloatingCompare, uint32_t(operation)},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFloatingPointConstant(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle floatingPointType,
    uint32_t bitWidth,
    uint64_t bitPattern,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.getFloatingPointConstantCallCount;
    const bool isSupportedTypeAndWidth =
        (floatingPointType == _getFakeNVVMBuilderHalfType() && bitWidth == 16) ||
        (floatingPointType == _getFakeNVVMBuilderFloatType() && bitWidth == 32);
    if (module != _getFakeNVVMBuilderModule() || !isSupportedTypeAndWidth ||
        (bitPattern >> bitWidth) != 0 || !outValue ||
        gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.floatingPointConstantStorage))
    {
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.floatingPointConstantBitPatterns.getCount();
    gFakeNVVMBuilder.floatingPointConstantBitWidths.add(bitWidth);
    gFakeNVVMBuilder.floatingPointConstantBitPatterns.add(bitPattern);
    *outValue = _getFakeNVVMBuilderFloatingPointConstant(resultIndex);
    return gFakeNVVMBuilder.failFloatingPointConstantAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static void _rejectFakeNVVMBuilderValueOperation(const SlangNVVMValueOperationDesc& operation)
{
    SLANG_ASSERT(operation.operandCount <= 3);
    gFakeNVVMBuilder.rejectValueOperation = true;
    gFakeNVVMBuilder.rejectedValueOperation = operation.operation;
    gFakeNVVMBuilder.rejectedValueOperationResultType = operation.resultType;
    gFakeNVVMBuilder.rejectedValueOperationOperandCount = uint32_t(operation.operandCount);
    for (uint32_t i = 0; i < operation.operandCount; ++i)
        gFakeNVVMBuilder.rejectedValueOperationOperandTypes[i] = operation.operandTypes[i];
}

static bool _isRejectedFakeNVVMBuilderValueOperation(const SlangNVVMValueOperationDesc& operation)
{
    if (!gFakeNVVMBuilder.rejectValueOperation ||
        operation.operation != gFakeNVVMBuilder.rejectedValueOperation ||
        operation.operandCount != gFakeNVVMBuilder.rejectedValueOperationOperandCount ||
        !NVVMSemantics::areSameType(
            operation.resultType,
            gFakeNVVMBuilder.rejectedValueOperationResultType))
    {
        return false;
    }
    for (uint32_t i = 0; i < operation.operandCount; ++i)
    {
        if (!NVVMSemantics::areSameType(
                operation.operandTypes[i],
                gFakeNVVMBuilder.rejectedValueOperationOperandTypes[i]))
        {
            return false;
        }
    }
    return true;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderIsOperationSupported(
    const SlangNVVMValueOperationDesc* operation,
    uint32_t* outSupported)
{
    ++gFakeNVVMBuilder.isOperationSupportedCallCount;
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported || (!operation->operandTypes && operation->operandCount))
    {
        return SLANG_E_INVALID_ARG;
    }
    *outSupported = NVVMSemantics::isSupported(*operation) &&
                            !_isRejectedFakeNVVMBuilderValueOperation(*operation)
                        ? 1u
                        : 0u;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderDeclareGlobalStorage(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle valueType,
    SlangNVVMLinkage linkage,
    SlangNVVMAddressSpace addressSpace,
    uint32_t alignment,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle* outStorage)
{
    const Index storageIndex = gFakeNVVMBuilder.declareGlobalStorageCallCount++;
    if (outStorage)
        *outStorage = nullptr;
    const bool isSharedArray = valueType == _getFakeNVVMBuilderArrayType() &&
                               linkage == SLANG_NVVM_LINKAGE_INTERNAL &&
                               addressSpace == SLANG_NVVM_ADDRESS_SPACE_SHARED;
    const bool isConstantStruct = valueType == _getFakeNVVMBuilderStructType() &&
                                  linkage == SLANG_NVVM_LINKAGE_EXTERNAL &&
                                  addressSpace == SLANG_NVVM_ADDRESS_SPACE_CONSTANT;
    if (module != _getFakeNVVMBuilderModule() || (!isSharedArray && !isConstantStruct) ||
        !alignment || (alignment & (alignment - 1)) || !name || !nameSize || !outStorage ||
        storageIndex < 0 || storageIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.globalStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const String storageName = String(UnownedStringSlice(name, nameSize));
    for (const auto& existingName : gFakeNVVMBuilder.globalStorageNames)
    {
        if (existingName == storageName)
            return SLANG_E_INVALID_ARG;
    }

    gFakeNVVMBuilder.globalStorageValueType = valueType;
    gFakeNVVMBuilder.globalStorageLinkage = linkage;
    gFakeNVVMBuilder.globalStorageAddressSpace = addressSpace;
    gFakeNVVMBuilder.globalStorageAlignment = alignment;
    gFakeNVVMBuilder.globalStorageNames.add(storageName);
    *outStorage = gFakeNVVMBuilder.returnNullGlobalStorage
                      ? nullptr
                      : _getFakeNVVMBuilderGlobalStorage(storageIndex);
    return gFakeNVVMBuilder.failGlobalStorageAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitSequentialElementExtract(
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle sequentialValue,
    SlangNVVMValueHandle elementIndex,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitSequentialElementExtractCallCount;
    if (outValue)
        *outValue = nullptr;
    FakeNVVMBuilderValueRef vectorRef;
    FakeNVVMBuilderValueRef elementIndexRef;
    const bool hasSequentialValueRef = _getFakeNVVMBuilderValueRef(sequentialValue, vectorRef);
    Index elementIndexConstantIndex = -1;
    uint32_t recordedElementIndex = UINT32_MAX;
    if (_getFakeNVVMBuilderIntegerConstantIndex(elementIndex, elementIndexConstantIndex))
    {
        const int64_t constantValue =
            gFakeNVVMBuilder.integerConstantValues[elementIndexConstantIndex];
        if (constantValue >= 0 && constantValue <= UINT32_MAX)
            recordedElementIndex = uint32_t(constantValue);
    }
    const bool hasElementIndexRef = _getFakeNVVMBuilderValueRef(elementIndex, elementIndexRef);
    uint32_t vectorElementCount = 0;
    FakeNVVMBuilderScalarTypeKind vectorElementTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
    if (_isFakeNVVMBuilderValueOfTypeKind(
            sequentialValue,
            FakeNVVMBuilderScalarTypeKind::NumericArray))
    {
        vectorElementCount = gFakeNVVMBuilder.arrayElementCount;
        if (!_getFakeNVVMBuilderTypeKind(gFakeNVVMBuilder.arrayElementType, vectorElementTypeKind))
        {
            return SLANG_E_INVALID_ARG;
        }
    }
    for (uint32_t candidateCount = 2; candidateCount <= 4; ++candidateCount)
    {
        if (vectorElementCount)
            break;
        if (_isFakeNVVMBuilderIntegerVectorValue(sequentialValue, candidateCount))
        {
            vectorElementCount = candidateCount;
            vectorElementTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
            break;
        }
        if (_isFakeNVVMBuilderVectorValue(
                sequentialValue,
                FakeNVVMBuilderScalarTypeKind::Boolean,
                candidateCount))
        {
            vectorElementCount = candidateCount;
            vectorElementTypeKind = FakeNVVMBuilderScalarTypeKind::Boolean;
            break;
        }
        if (_isFakeNVVMBuilderVectorValue(
                sequentialValue,
                FakeNVVMBuilderScalarTypeKind::Float,
                candidateCount))
        {
            vectorElementCount = candidateCount;
            vectorElementTypeKind = FakeNVVMBuilderScalarTypeKind::Float;
            break;
        }
        if (_isFakeNVVMBuilderVectorValue(
                sequentialValue,
                FakeNVVMBuilderScalarTypeKind::Half,
                candidateCount))
        {
            vectorElementCount = candidateCount;
            vectorElementTypeKind = FakeNVVMBuilderScalarTypeKind::Half;
            break;
        }
    }
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !vectorElementCount || !hasSequentialValueRef ||
        !_isFakeNVVMBuilderIntegerValue(elementIndex) || !hasElementIndexRef ||
        (recordedElementIndex != UINT32_MAX && recordedElementIndex >= vectorElementCount) ||
        !outValue ||
        gFakeNVVMBuilder.vectorElementIndices.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.vectorElementStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    const Index resultIndex = gFakeNVVMBuilder.vectorElementIndices.getCount();
    gFakeNVVMBuilder.vectorElementBaseValueRefs.add(vectorRef);
    gFakeNVVMBuilder.vectorElementIndexValueRefs.add(elementIndexRef);
    gFakeNVVMBuilder.vectorElementIndices.add(recordedElementIndex);
    gFakeNVVMBuilder.vectorElementTypeKinds.add(vectorElementTypeKind);
    *outValue = _getFakeNVVMBuilderVectorElement(resultIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitVectorConstruct(
    SlangNVVMModuleHandle module,
    SlangNVVMTypeHandle vectorType,
    const SlangNVVMValueHandle* elements,
    size_t elementCount,
    SlangNVVMValueHandle* outValue)
{
    ++gFakeNVVMBuilder.emitVectorConstructCallCount;
    if (outValue)
        *outValue = nullptr;

    FakeNVVMBuilderScalarTypeKind vectorElementTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
    uint32_t vectorElementCount = 0;
    const bool isVectorType =
        _getFakeNVVMBuilderVectorTypeInfo(vectorType, vectorElementCount, vectorElementTypeKind) &&
        vectorElementCount == elementCount;
    FakeNVVMBuilderValueRef elementRefs[4] = {};
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !isVectorType || !elements || !outValue ||
        gFakeNVVMBuilder.vectorConstructResultTypes.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.vectorConstructStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    for (size_t i = 0; i < elementCount; ++i)
    {
        if (!_isFakeNVVMBuilderValueOfTypeKind(elements[i], vectorElementTypeKind) ||
            !_getFakeNVVMBuilderValueRef(elements[i], elementRefs[i]))
        {
            return SLANG_E_INVALID_ARG;
        }
    }

    const Index resultIndex = gFakeNVVMBuilder.vectorConstructResultTypes.getCount();
    gFakeNVVMBuilder.vectorConstructResultTypes.add(vectorType);
    gFakeNVVMBuilder.vectorConstructElementOffsets.add(
        gFakeNVVMBuilder.vectorConstructElementValueRefs.getCount());
    gFakeNVVMBuilder.vectorConstructElementCounts.add(elementCount);
    for (size_t i = 0; i < elementCount; ++i)
        gFakeNVVMBuilder.vectorConstructElementValueRefs.add(elementRefs[i]);
    *outValue = _getFakeNVVMBuilderVectorConstruct(resultIndex);
    return SLANG_OK;
}

static SlangResult _fakeNVVMBuilderEmitExecutionOperation(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle* outValue)
{
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !outValue ||
        gFakeNVVMBuilder.executionRegisterOperations.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.executionRegisterStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    const Index resultIndex = gFakeNVVMBuilder.executionRegisterOperations.getCount();
    gFakeNVVMBuilder.executionRegisterOperations.add(operation);
    gFakeNVVMBuilder.executionRegisterCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    *outValue = _getFakeNVVMBuilderExecutionRegister(resultIndex);
    return SLANG_OK;
}

static SlangResult _fakeNVVMBuilderEmitBarrier(
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    SlangNVVMValueHandle* outValue)
{
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }
    if (operation == SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER)
        ++gFakeNVVMBuilder.workgroupBarrierCallCount;
    else if (operation == SLANG_NVVM_VALUE_OP_DEVICE_MEMORY_BARRIER)
        ++gFakeNVVMBuilder.deviceMemoryBarrierCallCount;
    else
        return SLANG_E_INVALID_ARG;
    *outValue = nullptr;
    return SLANG_OK;
}

static SlangResult _fakeNVVMBuilderEmitCatalogOperation(
    SlangNVVMModuleHandle module,
    const NVVMSemantics::CatalogEntry& entry,
    const SlangNVVMValueHandle* operands,
    SlangNVVMValueHandle* outValue)
{
    const SlangNVVMValueOperationDesc operation = NVVMSemantics::getOperationDesc(entry);
    if (entry.operation == SLANG_NVVM_VALUE_OP_FLOAT_CONVERT)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(entry.operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(entry.operation)},
            operands,
            entry.operandCount,
            outValue,
            &operation.resultType,
            operation.operandTypes);
    }
    if (entry.operandCount && entry.operandTypes[0].kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT)
    {
        if (entry.operandCount == 1)
            return _fakeNVVMBuilderEmitFloatingUnary(
                module,
                entry.operation,
                operands[0],
                outValue);
        if (entry.resultType.kind == SLANG_NVVM_VALUE_TYPE_BOOL)
        {
            return _fakeNVVMBuilderEmitFloatingCompare(
                module,
                entry.operation,
                operands[0],
                operands[1],
                outValue);
        }
        return _fakeNVVMBuilderEmitFloatingBinary(
            module,
            entry.operation,
            operands[0],
            operands[1],
            outValue);
    }

    if (entry.operation >= SLANG_NVVM_VALUE_OP_EQUAL &&
        entry.operation <= SLANG_NVVM_VALUE_OP_GREATER_EQUAL)
    {
        return _fakeNVVMBuilderEmitIntegerCompare(
            module,
            entry.operation,
            operands[0],
            operands[1],
            outValue);
    }
    if (entry.operation <= SLANG_NVVM_VALUE_OP_NEGATE)
    {
        return entry.operandCount == 1 ? _fakeNVVMBuilderEmitIntegerUnary(
                                             module,
                                             entry.operation,
                                             operands[0],
                                             outValue)
                                       : _fakeNVVMBuilderEmitIntegerBinaryOperation(
                                             module,
                                             entry.operation,
                                             operands[0],
                                             operands[1],
                                             outValue);
    }

    switch (entry.operation)
    {
    case SLANG_NVVM_VALUE_OP_THREAD_INDEX:
    case SLANG_NVVM_VALUE_OP_BLOCK_INDEX:
    case SLANG_NVVM_VALUE_OP_BLOCK_DIMENSIONS:
    case SLANG_NVVM_VALUE_OP_GRID_DIMENSIONS:
        return _fakeNVVMBuilderEmitExecutionOperation(module, entry.operation, outValue);
    case SLANG_NVVM_VALUE_OP_WORKGROUP_BARRIER:
    case SLANG_NVVM_VALUE_OP_DEVICE_MEMORY_BARRIER:
        return _fakeNVVMBuilderEmitBarrier(module, entry.operation, outValue);
    default:
        return _fakeNVVMBuilderEmitIntrinsic(
            module,
            operation,
            operands,
            entry.operandCount,
            outValue);
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMValueOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    if (!operation || !outValue || operation->operandCount != operandCount ||
        (!operands && operandCount))
    {
        return SLANG_E_INVALID_ARG;
    }

    if (operation->operation == SLANG_NVVM_VALUE_OP_BIT_REINTERPRET)
    {
        NVVMSemantics::ValueOperationFamilyResolution resolution;
        if (!NVVMSemantics::resolveValueOperationFamily(*operation, resolution) ||
            resolution.family != NVVMSemantics::ValueOperationFamily::BitReinterpret)
        {
            return SLANG_E_INVALID_ARG;
        }
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }

    const NVVMSemantics::CatalogEntry* entry = NVVMSemantics::find(*operation);
    if (entry)
        return _fakeNVVMBuilderEmitCatalogOperation(module, *entry, operands, outValue);

    NVVMSemantics::ValueOperationFamilyResolution resolution;
    if (!NVVMSemantics::resolveValueOperationFamily(*operation, resolution))
        return SLANG_E_INVALID_ARG;
    if (resolution.family == NVVMSemantics::ValueOperationFamily::IntegerBinary ||
        resolution.family == NVVMSemantics::ValueOperationFamily::BooleanBinary)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Binary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Binary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::BooleanUnary)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::FloatUnary)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::IntegerCompare)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Compare, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Compare, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::FloatCompare)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::FloatingCompare, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::FloatingCompare, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::BooleanCompare)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Compare, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Compare, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::FloatBinary)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::FloatingBinary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::FloatingBinary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::IntegerConvert)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::IntegerToFloat)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::FloatToInteger)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::FloatConvert)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::FloatingUnary, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    if (resolution.family == NVVMSemantics::ValueOperationFamily::Select)
    {
        gFakeNVVMBuilder.emittedValueOperations.add(
            {FakeNVVMBuilderScalarFamily::Select, uint32_t(operation->operation)});
        return _recordFakeNVVMBuilderScalarOperation(
            module,
            {FakeNVVMBuilderScalarFamily::Select, uint32_t(operation->operation)},
            operands,
            uint32_t(operandCount),
            outValue,
            &operation->resultType,
            operation->operandTypes);
    }
    return SLANG_E_INVALID_ARG;
}

static SlangNVVMBuilderFoundationAPI _makeFakeNVVMBuilderFoundationAPI()
{
    SlangNVVMBuilderFoundationAPI api = {};
    api.createModule = _fakeNVVMBuilderCreateModule;
    api.destroyModule = _fakeNVVMBuilderDestroyModule;
    api.serializeModuleWithDiagnostics = _fakeNVVMBuilderSerializeModuleWithDiagnostics;
    api.serializeNVVMIR20AssemblyWithDiagnostics =
        _fakeNVVMBuilderSerializeNVVMIR20AssemblyWithDiagnostics;
    return api;
}

static SlangNVVMBuilderConstructionAPI _makeFakeNVVMBuilderConstructionAPI()
{
    SlangNVVMBuilderConstructionAPI api = {};
    api.getVoidType = _fakeNVVMBuilderGetVoidType;
    api.getIntegerType = _fakeNVVMBuilderGetIntegerType;
    api.getFloatingPointType = _fakeNVVMBuilderGetFloatingPointType;
    api.getPointerType = _fakeNVVMBuilderGetPointerType;
    api.getFunctionType = _fakeNVVMBuilderGetFunctionType;
    api.getArrayType = _fakeNVVMBuilderGetArrayType;
    api.getVectorType = _fakeNVVMBuilderGetVectorType;
    api.getStructType = _fakeNVVMBuilderGetStructType;
    api.declareFunction = _fakeNVVMBuilderDeclareFunction;
    api.getFunctionParameter = _fakeNVVMBuilderGetFunctionParameter;
    api.setFunctionParameterAttributes = _fakeNVVMBuilderSetFunctionParameterAttributes;
    api.createBlock = _fakeNVVMBuilderCreateBlock;
    api.setInsertBlock = _fakeNVVMBuilderSetInsertBlock;
    api.emitLoad = _fakeNVVMBuilderEmitLoad;
    api.emitStore = _fakeNVVMBuilderEmitStore;
    api.emitLocalStorage = _fakeNVVMBuilderEmitLocalStorage;
    api.emitBranch = _fakeNVVMBuilderEmitBranch;
    api.emitConditionalBranch = _fakeNVVMBuilderEmitConditionalBranch;
    api.emitSwitch = _fakeNVVMBuilderEmitSwitch;
    api.getIntegerConstant = _fakeNVVMBuilderGetIntegerConstant;
    api.getFloatingPointConstant = _fakeNVVMBuilderGetFloatingPointConstant;
    api.emitPhi = _fakeNVVMBuilderEmitPhi;
    api.addPhiIncoming = _fakeNVVMBuilderAddPhiIncoming;
    api.emitCall = _fakeNVVMBuilderEmitCall;
    api.emitValueReturn = _fakeNVVMBuilderEmitValueReturn;
    api.emitReturnVoid = _fakeNVVMBuilderEmitReturnVoid;
    api.emitPointerOffset = _fakeNVVMBuilderEmitPointerOffset;
    api.emitByteOffsetPointer = _fakeNVVMBuilderEmitByteOffsetPointer;
    api.emitSequentialElementPointer = _fakeNVVMBuilderEmitSequentialElementPointer;
    api.emitStructFieldPointer = _fakeNVVMBuilderEmitStructFieldPointer;
    api.emitAggregateConstruct = _fakeNVVMBuilderEmitAggregateConstruct;
    api.emitAggregateElementExtract = _fakeNVVMBuilderEmitAggregateElementExtract;
    api.emitVectorConstruct = _fakeNVVMBuilderEmitVectorConstruct;
    api.emitSequentialElementExtract = _fakeNVVMBuilderEmitSequentialElementExtract;
    api.emitRelaxedGlobalI32AtomicAdd = _fakeNVVMBuilderEmitRelaxedGlobalI32AtomicAdd;
    api.declareGlobalStorage = _fakeNVVMBuilderDeclareGlobalStorage;
    api.markFunctionAsKernel = _fakeNVVMBuilderMarkFunctionAsKernel;
    return api;
}

static SlangNVVMBuilderValueOperationsAPI _makeFakeNVVMBuilderValueOperationsAPI()
{
    SlangNVVMBuilderValueOperationsAPI api = {};
    api.isOperationSupported = _fakeNVVMBuilderIsOperationSupported;
    api.emitOperation = _fakeNVVMBuilderEmitOperation;
    return api;
}

static bool _isFakeNVVMSurfaceOperationSupported(const SlangNVVMSurfaceOperationDesc& operation)
{
    const bool isSupportedShape = operation.shape == SLANG_NVVM_TEXTURE_SHAPE_1D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D ||
                                  operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D;
    const bool is32BitNumeric =
        operation.elementType.bitWidth == 32 &&
        (operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER);
    if ((operation.operation != SLANG_NVVM_SURFACE_OP_LOAD &&
         operation.operation != SLANG_NVVM_SURFACE_OP_STORE) ||
        !isSupportedShape || operation.isArray > 1 ||
        (operation.isArray && operation.shape != SLANG_NVVM_TEXTURE_SHAPE_2D) ||
        (operation.elementType.laneCount != 1 && operation.elementType.laneCount != 2 &&
         operation.elementType.laneCount != 4) ||
        operation.boundaryMode != SLANG_NVVM_SURFACE_BOUNDARY_ZERO)
    {
        return false;
    }
    if (operation.storageFormat == SLANG_NVVM_SURFACE_STORAGE_NATIVE)
    {
        return is32BitNumeric ||
               (operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                operation.elementType.bitWidth == 16 && !operation.isArray &&
                operation.shape != SLANG_NVVM_TEXTURE_SHAPE_3D);
    }
    return operation.storageFormat == SLANG_NVVM_SURFACE_STORAGE_FLOAT16 &&
           operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
           operation.elementType.bitWidth == 32 && !operation.isArray &&
           operation.shape != SLANG_NVVM_TEXTURE_SHAPE_3D;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderIsSurfaceOperationSupported(
    const SlangNVVMSurfaceOperationDesc* operation,
    uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported)
        return SLANG_E_INVALID_ARG;
    *outSupported = _isFakeNVVMSurfaceOperationSupported(*operation) ? 1u : 0u;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitSurfaceOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMSurfaceOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    const size_t expectedOperandCount =
        operation && operation->operation == SLANG_NVVM_SURFACE_OP_LOAD    ? 2
        : operation && operation->operation == SLANG_NVVM_SURFACE_OP_STORE ? 3
                                                                           : 0;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !operation || !outValue || !operands || operandCount != expectedOperandCount ||
        !_isFakeNVVMSurfaceOperationSupported(*operation) ||
        gFakeNVVMBuilder.surfaceOperations.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.surfaceOperationStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    for (size_t i = 0; i < operandCount; ++i)
    {
        FakeNVVMBuilderValueRef ref;
        if (!_getFakeNVVMBuilderValueRef(operands[i], ref))
            return SLANG_E_INVALID_ARG;
    }

    const uint32_t coordinateLaneCount = uint32_t(operation->shape) + operation->isArray;
    if (!_isFakeNVVMBuilderIntegerValue(operands[0]) ||
        (coordinateLaneCount == 1
             ? !_isFakeNVVMBuilderIntegerValue(operands[1])
             : !_isFakeNVVMBuilderIntegerVectorValue(operands[1], coordinateLaneCount)))
    {
        return SLANG_E_INVALID_ARG;
    }
    if (operation->operation == SLANG_NVVM_SURFACE_OP_STORE)
    {
        const uint32_t semanticBitWidth = operation->elementType.bitWidth;
        const FakeNVVMBuilderScalarTypeKind semanticScalarKind =
            operation->elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT
                ? (semanticBitWidth == 16 ? FakeNVVMBuilderScalarTypeKind::Half
                                          : FakeNVVMBuilderScalarTypeKind::Float)
                : FakeNVVMBuilderScalarTypeKind::Integer;
        const bool hasExpectedValue =
            operation->elementType.laneCount == 1
                ? (semanticScalarKind == FakeNVVMBuilderScalarTypeKind::Integer
                       ? _isFakeNVVMBuilderIntegerValue(operands[2])
                       : _isFakeNVVMBuilderFloatingPointValue(operands[2], semanticBitWidth))
                : _isFakeNVVMBuilderVectorValue(
                      operands[2],
                      semanticScalarKind,
                      operation->elementType.laneCount);
        if (!hasExpectedValue)
            return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.surfaceOperations.getCount();
    gFakeNVVMBuilder.surfaceOperations.add(*operation);
    if (operation->operation == SLANG_NVVM_SURFACE_OP_STORE)
        return SLANG_OK;
    *outValue = _getFakeNVVMBuilderSurfaceOperation(resultIndex);
    return SLANG_OK;
}

static SlangNVVMBuilderSurfaceOperationsAPI _makeFakeNVVMBuilderSurfaceOperationsAPI()
{
    SlangNVVMBuilderSurfaceOperationsAPI api = {};
    api.isOperationSupported = _fakeNVVMBuilderIsSurfaceOperationSupported;
    api.emitOperation = _fakeNVVMBuilderEmitSurfaceOperation;
    return api;
}

static bool _isFakeNVVMTextureOperationSupported(const SlangNVVMTextureOperationDesc& operation)
{
    if (operation.isArray > 1)
    {
        return false;
    }
    bool isValidShape = false;
    switch (operation.shape)
    {
    case SLANG_NVVM_TEXTURE_SHAPE_1D:
    case SLANG_NVVM_TEXTURE_SHAPE_2D:
    case SLANG_NVVM_TEXTURE_SHAPE_CUBE:
        isValidShape = true;
        break;
    case SLANG_NVVM_TEXTURE_SHAPE_3D:
        isValidShape = operation.isArray == 0;
        break;
    default:
        return false;
    }
    if (!isValidShape)
        return false;

    const bool isScalarFloat = operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT &&
                               operation.elementType.bitWidth == 32 &&
                               operation.elementType.laneCount == 1;
    const bool isFetchElement =
        (operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_FLOATING_POINT ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER ||
         operation.elementType.kind == SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER) &&
        operation.elementType.bitWidth == 32 &&
        (operation.elementType.laneCount == 1 || operation.elementType.laneCount == 2 ||
         operation.elementType.laneCount == 4);
    switch (operation.operation)
    {
    case SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL:
        return isScalarFloat;
    case SLANG_NVVM_TEXTURE_OP_QUERY_WIDTH:
        return isScalarFloat;
    case SLANG_NVVM_TEXTURE_OP_QUERY_HEIGHT:
        return isScalarFloat && operation.shape != SLANG_NVVM_TEXTURE_SHAPE_1D;
    case SLANG_NVVM_TEXTURE_OP_QUERY_DEPTH:
        return isScalarFloat && operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D;
    case SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL:
        return isFetchElement &&
               (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_2D ||
                (operation.shape == SLANG_NVVM_TEXTURE_SHAPE_3D && !operation.isArray));
    default:
        return false;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderIsTextureOperationSupported(
    const SlangNVVMTextureOperationDesc* operation,
    uint32_t* outSupported)
{
    if (outSupported)
        *outSupported = 0;
    if (!operation || !outSupported)
        return SLANG_E_INVALID_ARG;
    *outSupported = _isFakeNVVMTextureOperationSupported(*operation) ? 1u : 0u;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitTextureOperation(
    SlangNVVMModuleHandle module,
    const SlangNVVMTextureOperationDesc* operation,
    const SlangNVVMValueHandle* operands,
    size_t operandCount,
    SlangNVVMValueHandle* outValue)
{
    if (outValue)
        *outValue = nullptr;
    const size_t expectedOperandCount =
        operation && (operation->operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL ||
                      operation->operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL)
            ? 3
            : 1;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !operation || !operands || operandCount != expectedOperandCount || !outValue ||
        !_isFakeNVVMTextureOperationSupported(*operation) ||
        gFakeNVVMBuilder.textureOperations.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.textureOperationStorage) ||
        !_isFakeNVVMBuilderIntegerValue(operands[0]))
    {
        return SLANG_E_INVALID_ARG;
    }

    if (operation->operation == SLANG_NVVM_TEXTURE_OP_SAMPLE_LEVEL ||
        operation->operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL)
    {
        const bool isFetchLevel = operation->operation == SLANG_NVVM_TEXTURE_OP_FETCH_LEVEL;
        if (isFetchLevel ? !_isFakeNVVMBuilderIntegerValue(operands[2])
                         : !_isFakeNVVMBuilderFloatingPointValue(operands[2], 32))
            return SLANG_E_INVALID_ARG;

        uint32_t coordinateLaneCount = 0;
        switch (operation->shape)
        {
        case SLANG_NVVM_TEXTURE_SHAPE_1D:
            coordinateLaneCount = 1;
            break;
        case SLANG_NVVM_TEXTURE_SHAPE_2D:
            coordinateLaneCount = 2;
            break;
        case SLANG_NVVM_TEXTURE_SHAPE_3D:
        case SLANG_NVVM_TEXTURE_SHAPE_CUBE:
            coordinateLaneCount = 3;
            break;
        default:
            return SLANG_E_INVALID_ARG;
        }
        coordinateLaneCount += operation->isArray;
        const FakeNVVMBuilderScalarTypeKind coordinateElementType =
            isFetchLevel ? FakeNVVMBuilderScalarTypeKind::Integer
                         : FakeNVVMBuilderScalarTypeKind::Float;
        const bool hasCoordinate =
            coordinateLaneCount == 1
                ? (isFetchLevel ? _isFakeNVVMBuilderIntegerValue(operands[1])
                                : _isFakeNVVMBuilderFloatingPointValue(operands[1], 32))
                : _isFakeNVVMBuilderVectorValue(
                      operands[1],
                      coordinateElementType,
                      coordinateLaneCount);
        if (!hasCoordinate)
            return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.textureOperations.getCount();
    gFakeNVVMBuilder.textureOperations.add(*operation);
    *outValue = _getFakeNVVMBuilderTextureOperation(resultIndex);
    return SLANG_OK;
}

static SlangNVVMBuilderTextureOperationsAPI _makeFakeNVVMBuilderTextureOperationsAPI()
{
    SlangNVVMBuilderTextureOperationsAPI api = {};
    api.isOperationSupported = _fakeNVVMBuilderIsTextureOperationSupported;
    api.emitOperation = _fakeNVVMBuilderEmitTextureOperation;
    return api;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderQueryInterface(SlangNVVMBuilderInterfaceID interfaceID, const void** outInterface)
{
    if (outInterface)
        *outInterface = nullptr;
    if (!outInterface)
        return SLANG_E_INVALID_ARG;
    if (interfaceID == gFakeNVVMBuilder.omittedInterface)
        return SLANG_E_NO_INTERFACE;
    switch (interfaceID)
    {
    case SLANG_NVVM_BUILDER_INTERFACE_FOUNDATION:
        *outInterface = &gFakeNVVMBuilder.foundation;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_CONSTRUCTION:
        *outInterface = &gFakeNVVMBuilder.construction;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_VALUE_OPERATIONS:
        *outInterface = &gFakeNVVMBuilder.valueOperations;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_SURFACE_OPERATIONS:
        *outInterface = &gFakeNVVMBuilder.surfaceOperationsAPI;
        return SLANG_OK;
    case SLANG_NVVM_BUILDER_INTERFACE_TEXTURE_OPERATIONS:
        *outInterface = &gFakeNVVMBuilder.textureOperationsAPI;
        return SLANG_OK;
    default:
        return SLANG_E_NO_INTERFACE;
    }
}

static SlangNVVMBuilderAPI _makeFakeNVVMBuilderAPI()
{
    SlangNVVMBuilderAPI api = {};
    api.llvmVersionMajor = 14;
    api.llvmVersionMinor = 0;
    api.llvmVersionPatch = 6;
    api.nvvmIRVersionMajor = 2;
    api.nvvmIRVersionMinor = 0;
    api.pointerModel = SLANG_NVVM_POINTER_MODEL_TYPED;
    api.queryInterface = _fakeNVVMBuilderQueryInterface;
    return api;
}

static SlangResult SLANG_NVVM_CALL
_fakeGetNVVMBuilderAPI(uint32_t abiRevision, SlangNVVMBuilderAPI* outAPI)
{
    if (!outAPI || abiRevision != gFakeNVVMBuilder.acceptedABIRevision)
        return SLANG_E_NO_INTERFACE;
    *outAPI = gFakeNVVMBuilder.api;
    return SLANG_OK;
}

class FakeNVVMBuilderLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    FakeNVVMBuilderLibrary() { ++gFakeNVVMBuilder.liveLibraryCount; }
    ~FakeNVVMBuilderLibrary()
    {
        --gFakeNVVMBuilder.liveLibraryCount;
        ++gFakeNVVMBuilder.destroyedLibraryCount;
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(const char* name)
        SLANG_OVERRIDE
    {
        return name && !gFakeNVVMBuilder.omitAPISymbol &&
                       UnownedStringSlice(name) == SLANG_NVVM_BUILDER_GET_API_NAME
                   ? reinterpret_cast<void*>(_fakeGetNVVMBuilderAPI)
                   : nullptr;
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                guid == ISlangSharedLibrary::getTypeGuid())
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }
};

class FakeNVVMBuilderLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (!path || !outLibrary)
            return SLANG_E_INVALID_ARG;
        *outLibrary = nullptr;
        gFakeNVVMBuilder.loadedPath = path;
        ++gFakeNVVMBuilder.loadRequestCount;
        if (gFakeNVVMBuilder.loadedPath != "slang-llvm-nvvm")
            return SLANG_E_NOT_FOUND;
        if (gFakeNVVMBuilder.libraryUnavailable)
            return SLANG_E_NOT_FOUND;
        ++gFakeNVVMBuilder.successfulLoadCount;
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        *outLibrary = library.detach();
        return SLANG_OK;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

struct FakeNVVMState
{
    void resetCalls()
    {
        createProgramCallCount = 0;
        destroyProgramCallCount = 0;
        addModuleCallCount = 0;
        lazyAddModuleCallCount = 0;
        verifyProgramCallCount = 0;
        compileProgramCallCount = 0;
        getResultSizeCallCount = 0;
        getResultCallCount = 0;
        getLogSizeCallCount = 0;
        getLogCallCount = 0;
        addedModule = String();
        addedModuleName = String();
        addedLibraryModule = String();
        addedLibraryModuleName = String();
        moduleAddKinds.clear();
        moduleAddNames.clear();
        verifyOptions.clear();
        compileOptions.clear();
        currentLogPhase = FakeLogPhase::General;
    }

    void reset()
    {
        SLANG_ASSERT(liveLibraryCount == 0);
        failure = FakeFailure::None;
        failureResult = TestNVVMResult::Compilation;
        missingSymbol = String();
        omitOptionalSymbols = false;
        loadedPath = String();
        successfulLoadCount = 0;
        liveLibraryCount = 0;
        destroyedLibraryCount = 0;
        programLog = String();
        verifierLog = String();
        compilerLog = String();
        usePhaseLogs = false;
        resultMode = FakeResultMode::NullTerminated;
        compiledPTX = kFakePTX;
        resetCalls();
    }

    FakeFailure failure = FakeFailure::None;
    TestNVVMResult failureResult = TestNVVMResult::Compilation;
    String missingSymbol;
    bool omitOptionalSymbols = false;

    String loadedPath;
    int successfulLoadCount = 0;
    int liveLibraryCount = 0;
    int destroyedLibraryCount = 0;

    TestNVVMProgramStorage programStorage = {};
    int createProgramCallCount = 0;
    int destroyProgramCallCount = 0;
    int addModuleCallCount = 0;
    int lazyAddModuleCallCount = 0;
    int verifyProgramCallCount = 0;
    int compileProgramCallCount = 0;
    int getResultSizeCallCount = 0;
    int getResultCallCount = 0;
    int getLogSizeCallCount = 0;
    int getLogCallCount = 0;

    String addedModule;
    String addedModuleName;
    String addedLibraryModule;
    String addedLibraryModuleName;
    List<FakeModuleAddKind> moduleAddKinds;
    List<String> moduleAddNames;
    List<String> verifyOptions;
    List<String> compileOptions;
    String programLog;
    String verifierLog;
    String compilerLog;
    FakeLogPhase currentLogPhase = FakeLogPhase::General;
    bool usePhaseLogs = false;
    FakeResultMode resultMode = FakeResultMode::NullTerminated;
    String compiledPTX;
};

FakeNVVMState gFakeNVVM;

static TestNVVMResult _fakeFailureResult(FakeFailure operation)
{
    return gFakeNVVM.failure == operation ? gFakeNVVM.failureResult : TestNVVMResult::Success;
}

static bool _isFakeProgram(TestNVVMProgram program)
{
    return program == &gFakeNVVM.programStorage;
}

static const String& _getCurrentFakeProgramLog()
{
    if (gFakeNVVM.usePhaseLogs)
    {
        switch (gFakeNVVM.currentLogPhase)
        {
        case FakeLogPhase::Verifier:
            return gFakeNVVM.verifierLog;
        case FakeLogPhase::Compiler:
            return gFakeNVVM.compilerLog;
        default:
            break;
        }
    }
    return gFakeNVVM.programLog;
}

static void _captureOptions(int optionCount, const char** options, List<String>& outOptions)
{
    outOptions.clear();
    for (int i = 0; i < optionCount; ++i)
        outOptions.add(options[i]);
}

static const char* _fakeGetErrorString(TestNVVMResult result)
{
    return result == TestNVVMResult::Success ? "success" : "fake NVVM compilation failure";
}

static TestNVVMResult _fakeVersion(int* major, int* minor)
{
    if (!major || !minor)
        return TestNVVMResult::InvalidInput;
    *major = 2;
    *minor = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeIRVersion(int* majorIR, int* minorIR, int* majorDebug, int* minorDebug)
{
    if (!majorIR || !minorIR || !majorDebug || !minorDebug)
        return TestNVVMResult::InvalidInput;
    *majorIR = 2;
    *minorIR = 0;
    *majorDebug = 3;
    *minorDebug = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeCreateProgram(TestNVVMProgram* outProgram)
{
    ++gFakeNVVM.createProgramCallCount;
    if (!outProgram)
        return TestNVVMResult::InvalidInput;
    if (gFakeNVVM.failure == FakeFailure::CreateProgram)
        return TestNVVMResult::ProgramCreationFailure;
    *outProgram = &gFakeNVVM.programStorage;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeDestroyProgram(TestNVVMProgram* program)
{
    if (!program || !_isFakeProgram(*program))
        return TestNVVMResult::InvalidProgram;
    ++gFakeNVVM.destroyProgramCallCount;
    *program = nullptr;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeAddModuleToProgram(
    TestNVVMProgram program,
    const char* buffer,
    size_t size,
    const char* name)
{
    ++gFakeNVVM.addModuleCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::General;
    if (!_isFakeProgram(program) || (!buffer && size) || !name)
        return TestNVVMResult::InvalidInput;
    gFakeNVVM.moduleAddKinds.add(FakeModuleAddKind::Normal);
    gFakeNVVM.moduleAddNames.add(name);
    if (gFakeNVVM.addModuleCallCount == 1)
    {
        gFakeNVVM.addedModule = String(UnownedStringSlice(buffer, size));
        gFakeNVVM.addedModuleName = name;
        return _fakeFailureResult(FakeFailure::AddModule);
    }

    gFakeNVVM.addedLibraryModule = String(UnownedStringSlice(buffer, size));
    gFakeNVVM.addedLibraryModuleName = name;
    return _fakeFailureResult(FakeFailure::EagerAddModule);
}

static TestNVVMResult _fakeVerifyProgram(
    TestNVVMProgram program,
    int optionCount,
    const char** options)
{
    ++gFakeNVVM.verifyProgramCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::Verifier;
    if (!_isFakeProgram(program) || optionCount < 0 || (optionCount && !options))
        return TestNVVMResult::InvalidInput;
    _captureOptions(optionCount, options, gFakeNVVM.verifyOptions);
    return _fakeFailureResult(FakeFailure::VerifyProgram);
}

static TestNVVMResult _fakeCompileProgram(
    TestNVVMProgram program,
    int optionCount,
    const char** options)
{
    ++gFakeNVVM.compileProgramCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::Compiler;
    if (!_isFakeProgram(program) || optionCount < 0 || (optionCount && !options))
        return TestNVVMResult::InvalidInput;
    _captureOptions(optionCount, options, gFakeNVVM.compileOptions);
    return _fakeFailureResult(FakeFailure::CompileProgram);
}

static TestNVVMResult _fakeGetCompiledResultSize(TestNVVMProgram program, size_t* outSize)
{
    ++gFakeNVVM.getResultSizeCallCount;
    if (!_isFakeProgram(program) || !outSize)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetResultSize);
    if (result != TestNVVMResult::Success)
        return result;
    // A conforming libNVVM result includes the C-string terminator in its reported size. The other
    // modes deliberately violate that contract so the compiler's boundary checks can be tested.
    switch (gFakeNVVM.resultMode)
    {
    case FakeResultMode::TerminatorOnly:
        *outSize = 1;
        break;
    case FakeResultMode::Unterminated:
        *outSize = size_t(gFakeNVVM.compiledPTX.getLength());
        break;
    default:
        *outSize = size_t(gFakeNVVM.compiledPTX.getLength()) + 1;
        break;
    }
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeGetCompiledResult(TestNVVMProgram program, char* outResult)
{
    ++gFakeNVVM.getResultCallCount;
    if (!_isFakeProgram(program) || !outResult)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetResult);
    if (result != TestNVVMResult::Success)
        return result;
    if (gFakeNVVM.resultMode == FakeResultMode::TerminatorOnly)
    {
        outResult[0] = 0;
        return TestNVVMResult::Success;
    }
    const Index size = gFakeNVVM.compiledPTX.getLength();
    if (size)
        ::memcpy(outResult, gFakeNVVM.compiledPTX.getBuffer(), size_t(size));
    if (gFakeNVVM.resultMode == FakeResultMode::NullTerminated)
        outResult[size] = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeGetProgramLogSize(TestNVVMProgram program, size_t* outSize)
{
    ++gFakeNVVM.getLogSizeCallCount;
    if (!_isFakeProgram(program) || !outSize)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetLogSize);
    if (result != TestNVVMResult::Success)
        return result;
    *outSize = size_t(_getCurrentFakeProgramLog().getLength()) + 1;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeGetProgramLog(TestNVVMProgram program, char* outLog)
{
    ++gFakeNVVM.getLogCallCount;
    if (!_isFakeProgram(program) || !outLog)
        return TestNVVMResult::InvalidInput;
    TestNVVMResult result = _fakeFailureResult(FakeFailure::GetLog);
    if (result != TestNVVMResult::Success)
        return result;
    const String& programLog = _getCurrentFakeProgramLog();
    const Index size = programLog.getLength();
    if (size)
        ::memcpy(outLog, programLog.getBuffer(), size_t(size));
    outLog[size] = 0;
    return TestNVVMResult::Success;
}

static TestNVVMResult _fakeLazyAddModuleToProgram(
    TestNVVMProgram program,
    const char* buffer,
    size_t size,
    const char* name)
{
    ++gFakeNVVM.lazyAddModuleCallCount;
    gFakeNVVM.currentLogPhase = FakeLogPhase::General;
    if (!_isFakeProgram(program) || (!buffer && size) || !name)
        return TestNVVMResult::InvalidInput;
    gFakeNVVM.addedLibraryModule = String(UnownedStringSlice(buffer, size));
    gFakeNVVM.addedLibraryModuleName = name;
    gFakeNVVM.moduleAddKinds.add(FakeModuleAddKind::Lazy);
    gFakeNVVM.moduleAddNames.add(name);
    return _fakeFailureResult(FakeFailure::LazyAddModule);
}

static TestNVVMResult _fakeLLVMVersion(const char* architecture, int* major)
{
    SLANG_UNUSED(architecture);
    if (major)
        *major = 7;
    return TestNVVMResult::Success;
}

class FakeNVVMLibrary : public RefObject, public ISlangSharedLibrary
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    FakeNVVMLibrary() { ++gFakeNVVM.liveLibraryCount; }

    ~FakeNVVMLibrary()
    {
        --gFakeNVVM.liveLibraryCount;
        ++gFakeNVVM.destroyedLibraryCount;
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL castAs(const SlangUUID& guid) SLANG_OVERRIDE
    {
        return getInterface(guid);
    }

    virtual SLANG_NO_THROW void* SLANG_MCALL findSymbolAddressByName(const char* name)
        SLANG_OVERRIDE
    {
        if (!name)
            return nullptr;
        const UnownedStringSlice symbol(name);
        if (gFakeNVVM.missingSymbol.getLength() &&
            symbol == gFakeNVVM.missingSymbol.getUnownedSlice())
        {
            return nullptr;
        }

        if (symbol == "nvvmGetErrorString")
            return (void*)_fakeGetErrorString;
        if (symbol == "nvvmVersion")
            return (void*)_fakeVersion;
        if (symbol == "nvvmIRVersion")
            return (void*)_fakeIRVersion;
        if (symbol == "nvvmCreateProgram")
            return (void*)_fakeCreateProgram;
        if (symbol == "nvvmDestroyProgram")
            return (void*)_fakeDestroyProgram;
        if (symbol == "nvvmAddModuleToProgram")
            return (void*)_fakeAddModuleToProgram;
        if (symbol == "nvvmVerifyProgram")
            return (void*)_fakeVerifyProgram;
        if (symbol == "nvvmCompileProgram")
            return (void*)_fakeCompileProgram;
        if (symbol == "nvvmGetCompiledResultSize")
            return (void*)_fakeGetCompiledResultSize;
        if (symbol == "nvvmGetCompiledResult")
            return (void*)_fakeGetCompiledResult;
        if (symbol == "nvvmGetProgramLogSize")
            return (void*)_fakeGetProgramLogSize;
        if (symbol == "nvvmGetProgramLog")
            return (void*)_fakeGetProgramLog;
        if (!gFakeNVVM.omitOptionalSymbols && symbol == "nvvmLazyAddModuleToProgram")
            return (void*)_fakeLazyAddModuleToProgram;
        if (!gFakeNVVM.omitOptionalSymbols && symbol == "nvvmLLVMVersion")
            return (void*)_fakeLLVMVersion;
        return nullptr;
    }

protected:
    void* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() || guid == ICastable::getTypeGuid() ||
                guid == ISlangSharedLibrary::getTypeGuid())
                   ? static_cast<ISlangSharedLibrary*>(this)
                   : nullptr;
    }
};

class FakeNVVMLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (!outLibrary)
            return SLANG_E_INVALID_ARG;
        *outLibrary = nullptr;
        gFakeNVVM.loadedPath = path ? path : "";
        if (!path || UnownedStringSlice(path) != "nvvm")
            return SLANG_E_NOT_FOUND;

        ++gFakeNVVM.successfulLoadCount;
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMLibrary);
        *outLibrary = library.detach();
        return SLANG_OK;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

// Serves both independently loaded modules used by the public direct route. Accepting decorated
// paths keeps the test deterministic even when the parent environment names explicit CUDA and
// SLANG_NVVM_BUILDER_PATH directories.
class FakeDirectNVVMLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (!path || !outLibrary)
            return SLANG_E_INVALID_ARG;
        *outLibrary = nullptr;

        const String requestedPath(path);
        if (requestedPath.getUnownedSlice().indexOf(toSlice("slang-llvm-nvvm")) >= 0)
        {
            gFakeNVVMBuilder.loadedPath = requestedPath;
            ++gFakeNVVMBuilder.loadRequestCount;
            if (gFakeNVVMBuilder.libraryUnavailable)
                return SLANG_E_NOT_FOUND;
            ++gFakeNVVMBuilder.successfulLoadCount;
            ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
            *outLibrary = library.detach();
            return SLANG_OK;
        }
        if (requestedPath.getUnownedSlice().indexOf(toSlice("nvvm")) >= 0)
        {
            gFakeNVVM.loadedPath = requestedPath;
            ++gFakeNVVM.successfulLoadCount;
            ComPtr<ISlangSharedLibrary> library(new FakeNVVMLibrary);
            *outLibrary = library.detach();
            return SLANG_OK;
        }
        return SLANG_E_NOT_FOUND;
    }

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

// Records filesystem load spellings while returning the in-process fake library. The candidate
// files used by discovery tests are inert; this loader ensures none reaches the platform loader.
class RecordingFakeNVVMLoader : public RefObject, public ISlangSharedLibraryLoader
{
public:
    SLANG_REF_OBJECT_IUNKNOWN_ALL

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL
    loadSharedLibrary(const char* path, ISlangSharedLibrary** outLibrary) SLANG_OVERRIDE
    {
        if (!path || !outLibrary)
            return SLANG_E_INVALID_ARG;
        *outLibrary = nullptr;
        loadRequests.add(path);
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMLibrary);
        *outLibrary = library.detach();
        return SLANG_OK;
    }

    List<String> loadRequests;

protected:
    ISlangUnknown* getInterface(const Guid& guid)
    {
        return (guid == ISlangUnknown::getTypeGuid() ||
                guid == ISlangSharedLibraryLoader::getTypeGuid())
                   ? static_cast<ISlangSharedLibraryLoader*>(this)
                   : nullptr;
    }
};

static IDownstreamCompiler* _findNVVMCompiler(DownstreamCompilerSet* set)
{
    DownstreamCompilerDesc desc(SLANG_PASS_THROUGH_NVVM);
    return DownstreamCompilerUtil::findCompiler(
        set,
        DownstreamCompilerUtil::MatchType::Newest,
        desc);
}

static IDownstreamCompiler* _findNVRTCCompiler(DownstreamCompilerSet* set)
{
    DownstreamCompilerDesc desc(SLANG_PASS_THROUGH_NVRTC);
    return DownstreamCompilerUtil::findCompiler(
        set,
        DownstreamCompilerUtil::MatchType::Newest,
        desc);
}

static SlangResult _locateFakeNVVM(
    RefPtr<DownstreamCompilerSet>& outSet,
    IDownstreamCompiler*& outCompiler)
{
    outSet = new DownstreamCompilerSet;
    outCompiler = nullptr;
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
    SLANG_RETURN_ON_FAIL(NVVMDownstreamCompilerUtil::locateCompilers(String(), loader, outSet));
    outCompiler = _findNVVMCompiler(outSet);
    return outCompiler ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _locateRealNVVM(
    const String& path,
    RefPtr<DownstreamCompilerSet>& outSet,
    IDownstreamCompiler*& outCompiler)
{
    outSet = new DownstreamCompilerSet;
    outCompiler = nullptr;
    SLANG_RETURN_ON_FAIL(NVVMDownstreamCompilerUtil::locateCompilers(
        path,
        DefaultSharedLibraryLoader::getSingleton(),
        outSet));
    outCompiler = _findNVVMCompiler(outSet);
    return outCompiler ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _locateRealNVRTC(
    RefPtr<DownstreamCompilerSet>& outSet,
    IDownstreamCompiler*& outCompiler)
{
    outSet = new DownstreamCompilerSet;
    outCompiler = nullptr;
    SLANG_RETURN_ON_FAIL(NVRTCDownstreamCompilerUtil::locateCompilers(
        String(),
        DefaultSharedLibraryLoader::getSingleton(),
        outSet));
    outCompiler = _findNVRTCCompiler(outSet);
    return outCompiler ? SLANG_OK : SLANG_FAIL;
}

static ComPtr<IArtifact> _createNVVMIRArtifact(const char* ir = kMinimalNVVMIR)
{
    ComPtr<IArtifact> artifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_SHADER_LLVM_IR);
    artifact->addRepresentationUnknown(StringBlob::create(UnownedStringSlice(ir)));
    return artifact;
}

static ComPtr<IArtifact> _createNVVMBitcodeArtifact(const void* data, size_t size)
{
    ComPtr<IArtifact> artifact = ArtifactUtil::createArtifact(ArtifactDesc::make(
        ArtifactKind::ObjectCode,
        ArtifactPayload::LLVMIR,
        ArtifactStyle::Kernel));
    artifact->addRepresentationUnknown(RawBlob::create(data, size));
    return artifact;
}

static ComPtr<IArtifact> _createNVVMBitcodeArtifact()
{
    return _createNVVMBitcodeArtifact(kMinimalNVVMBitcode, SLANG_COUNT_OF(kMinimalNVVMBitcode));
}

static ComPtr<IArtifact> _createCUDASourceArtifact(const UnownedStringSlice& source)
{
    ComPtr<IArtifact> artifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_CUDA_SOURCE);
    artifact->setName("slang-nvvm-scalar-reference.cu");
    artifact->addRepresentationUnknown(StringBlob::create(source));
    return artifact;
}

struct CompileSettings
{
    DownstreamCompileOptions::OptimizationLevel optimizationLevel =
        DownstreamCompileOptions::OptimizationLevel::Default;
    DownstreamCompileOptions::DebugInfoType debugInfoType =
        DownstreamCompileOptions::DebugInfoType::None;
    DownstreamCompileOptions::FloatingPointMode floatingPointMode =
        DownstreamCompileOptions::FloatingPointMode::Default;
    DownstreamCompileOptions::FloatingPointDenormalMode denormalModeFp32 =
        DownstreamCompileOptions::FloatingPointDenormalMode::Any;
    DownstreamCompileOptions::FloatingPointDenormalMode denormalModeFp16 =
        DownstreamCompileOptions::FloatingPointDenormalMode::Any;
    DownstreamCompileOptions::FloatingPointDenormalMode denormalModeFp64 =
        DownstreamCompileOptions::FloatingPointDenormalMode::Any;
    bool requiresCUDADeviceLibrary = false;
    bool addFakeCompilerArgument = false;
    const char* compilerSpecificArgument = nullptr;
};

static SlangResult _compileNVVM(
    IDownstreamCompiler* compiler,
    IArtifact* sourceArtifact,
    const CompileSettings& settings,
    IArtifact** outArtifact)
{
    IArtifact* sourceArtifacts[] = {sourceArtifact};
    DownstreamCompileOptions::CapabilityVersion capability;
    capability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    capability.version.set(7, 5);

    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    options.targetType = SLANG_PTX;
    options.optimizationLevel = settings.optimizationLevel;
    options.debugInfoType = settings.debugInfoType;
    options.floatingPointMode = settings.floatingPointMode;
    options.denormalModeFp16 = settings.denormalModeFp16;
    options.denormalModeFp32 = settings.denormalModeFp32;
    options.denormalModeFp64 = settings.denormalModeFp64;
    options.requiresCUDADeviceLibrary = settings.requiresCUDADeviceLibrary;
    options.sourceArtifacts = makeSlice(sourceArtifacts, SLANG_COUNT_OF(sourceArtifacts));
    options.requiredCapabilityVersions = makeSlice(&capability, 1);
    TerminatedCharSlice selectedArgument(
        settings.compilerSpecificArgument ? settings.compilerSpecificArgument
                                          : "-fake-nvvm-option");
    if (settings.addFakeCompilerArgument || settings.compilerSpecificArgument)
        options.compilerSpecificArguments = makeSlice(&selectedArgument, 1);
    return compiler->compile(options, outArtifact);
}

static SlangResult _compileNVRTC(
    IDownstreamCompiler* compiler,
    IArtifact* sourceArtifact,
    IArtifact** outArtifact)
{
    IArtifact* sourceArtifacts[] = {sourceArtifact};
    DownstreamCompileOptions::CapabilityVersion capability;
    capability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    capability.version.set(7, 5);

    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_CUDA;
    options.targetType = SLANG_PTX;
    options.sourceArtifacts = makeSlice(sourceArtifacts, SLANG_COUNT_OF(sourceArtifacts));
    options.requiredCapabilityVersions = makeSlice(&capability, 1);
    return compiler->compile(options, outArtifact);
}

static bool _hasOption(const List<String>& options, const char* expected)
{
    for (const auto& option : options)
    {
        if (option == expected)
            return true;
    }
    return false;
}

static bool _diagnosticsContain(IArtifactDiagnostics* diagnostics, const char* expected)
{
    if (!diagnostics)
        return false;
    const TerminatedCharSlice raw = diagnostics->getRaw();
    return raw.data && ::strstr(raw.data, expected);
}

static IArtifactDiagnostics* _findDiagnostics(IArtifact* artifact)
{
    return artifact ? findAssociatedRepresentation<IArtifactDiagnostics>(artifact) : nullptr;
}

static void _reportArtifactDiagnostics(IArtifact* artifact)
{
    IArtifactDiagnostics* diagnostics = _findDiagnostics(artifact);
    if (!diagnostics)
        return;
    const TerminatedCharSlice raw = diagnostics->getRaw();
    if (raw.count)
        getTestReporter()->message(TestMessageType::Info, raw.data);
}

struct ScopedNVVMBuilderModule
{
    const NVVMIRBuilder* builder = nullptr;
    SlangNVVMModuleHandle module = nullptr;

    ~ScopedNVVMBuilderModule()
    {
        if (builder && module)
            builder->destroyModule(module);
    }
};

struct RealNVVMBuilderLocation
{
    String directory;
    bool isExplicit = false;
    bool moduleExists = false;
};

static RealNVVMBuilderLocation _getRealNVVMBuilderLocation(UnitTestContext* context)
{
    RealNVVMBuilderLocation location;
    StringBuilder pathBuilder;
    location.isExplicit = SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
                              toSlice("SLANG_NVVM_BUILDER_PATH"),
                              pathBuilder)) &&
                          pathBuilder.getLength();
    location.directory =
        location.isExplicit ? pathBuilder.produceString() : String(context->executableDirectory);
    const String modulePath = Path::combine(
        location.directory,
        SharedLibrary::calcPlatformPath(toSlice("slang-llvm-nvvm")));
    location.moduleExists = File::exists(modulePath);
    return location;
}

// Loads the real provider without converting absence into an ignored test. Child-process tests use
// this form because an ignored child exits successfully and would make the parent pass vacuously.
static SlangResult _loadRealNVVMBuilder(
    UnitTestContext* context,
    NVVMIRBuilder& outBuilder,
    RealNVVMBuilderLocation* outLocation = nullptr)
{
    const RealNVVMBuilderLocation location = _getRealNVVMBuilderLocation(context);
    if (outLocation)
        *outLocation = location;
    if (!location.isExplicit && !location.moduleExists)
        return SLANG_E_NOT_FOUND;
    return NVVMIRBuilder::load(
        location.directory,
        DefaultSharedLibraryLoader::getSingleton(),
        outBuilder);
}

static void _requireRealNVVMBuilder(UnitTestContext* context, NVVMIRBuilder& outBuilder)
{
    RealNVVMBuilderLocation location;
    const SlangResult loadResult = _loadRealNVVMBuilder(context, outBuilder, &location);
    if (loadResult == SLANG_E_NOT_FOUND && !location.isExplicit && !location.moduleExists)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real LLVM 14 NVVM builder test because slang-llvm-nvvm was not found.");
        SLANG_IGNORE_TEST;
    }

    if (SLANG_FAILED(loadResult))
    {
        StringBuilder message;
        message << "Unable to load a compatible LLVM 14 NVVM builder";
        if (location.isExplicit)
            message << " from the explicit SLANG_NVVM_BUILDER_PATH directory: "
                    << location.directory;
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(loadResult));
    }
    if (!outBuilder.isInitialized())
        SLANG_CHECK_ABORT(outBuilder.isInitialized());
    SLANG_CHECK_ABORT(outBuilder.isInitialized());
}

static SlangResult _populateEmptyNVVMKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(module, voidType, nullptr, 0, functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        function));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildEmptyNVVMKernel(
    const NVVMIRBuilder& builder,
    const UnownedStringSlice& kernelName,
    ComPtr<ISlangBlob>& outAssembly,
    ComPtr<ISlangBlob>& outBitcode)
{
    outAssembly.setNull();
    outBitcode.setNull();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-unit-test"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateEmptyNVVMKernel(builder, scope.module, kernelName));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly));
    SLANG_RETURN_ON_FAIL(
        builder.serializeModule(scope.module, SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE, outBitcode));
    return SLANG_OK;
}

static const char kWriteScalarKernelName[] = "writeScalar";
static const char kCopyScalarKernelName[] = "copyScalar";
static const char kChooseScalarKernelName[] = "chooseScalar";
static const char kSumToLimitKernelName[] = "sumToLimit";
static const char kCallScalarKernelName[] = "callScalar";
static const char kIncrementScalarHelperName[] = "incrementScalar";
static const char kCopyIndexedKernelName[] = "copyIndexed";
static const char kCopyByteOffsetKernelName[] = "copyByteOffset";
static const char kCopyArrayElementKernelName[] = "copyArrayElement";
static const char kMultiplyScalarKernelName[] = "multiplyScalar";
static const char kBitAndScalarKernelName[] = "bitAndScalar";
static const char kBitOrScalarKernelName[] = "bitOrScalar";
static const char kBitXorScalarKernelName[] = "bitXorScalar";
static const char kBitNotScalarKernelName[] = "bitNotScalar";
static const char kNegateScalarKernelName[] = "negateScalar";
static const char kRelaxedGlobalI32AtomicAddKernelName[] = "relaxedGlobalI32AtomicAdd";
static const char kEqualScalarKernelName[] = "equalScalar";
static const char kNotEqualScalarKernelName[] = "notEqualScalar";
static const char kGreaterThanScalarKernelName[] = "greaterThanScalar";
static const char kLessEqualScalarKernelName[] = "lessEqualScalar";
static const char kGreaterEqualScalarKernelName[] = "greaterEqualScalar";
static const char kFloat32AddKernelName[] = "float32Add";
static const char kFloat32SubtractKernelName[] = "float32Subtract";
static const char kFloat32MultiplyKernelName[] = "float32Multiply";
static const char kFloat32DivideKernelName[] = "float32Divide";
static const char kFloat32NegateKernelName[] = "float32Negate";
static const char kFloat32EqualKernelName[] = "float32Equal";
static const char kFloat32NotEqualKernelName[] = "float32NotEqual";
static const char kFloat32GreaterThanKernelName[] = "float32GreaterThan";
static const char kFloat32LessEqualKernelName[] = "float32LessEqual";
static const char kFloat32GreaterEqualKernelName[] = "float32GreaterEqual";
static const char kFloat32LessThanKernelName[] = "float32LessThan";
static const char kScalarReferenceCUDASource[] = R"(
extern "C" __global__ void writeScalar(int* destination, int value)
{
    *destination = value;
}

extern "C" __global__ void copyScalar(int* destination, const int* source)
{
    *destination = *source;
}
)";

static SlangResult _populateScalarReferenceKernels(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    {
        const SlangNVVMTypeHandle parameterTypes[] = {
            globalIntegerPointerType,
            integerType,
        };
        SlangNVVMTypeHandle functionType = nullptr;
        SlangNVVMValueHandle function = nullptr;
        SlangNVVMValueHandle destination = nullptr;
        SlangNVVMValueHandle value = nullptr;
        SlangNVVMBlockHandle entryBlock = nullptr;
        SLANG_RETURN_ON_FAIL(builder.getFunctionType(
            module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType));
        SLANG_RETURN_ON_FAIL(builder.declareFunction(
            module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice(kWriteScalarKernelName),
            function));
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, value));
        SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
        SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
        SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destination, 4));
        SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
        SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    }

    {
        const SlangNVVMTypeHandle parameterTypes[] = {
            globalIntegerPointerType,
            globalIntegerPointerType,
        };
        SlangNVVMTypeHandle functionType = nullptr;
        SlangNVVMValueHandle function = nullptr;
        SlangNVVMValueHandle destination = nullptr;
        SlangNVVMValueHandle source = nullptr;
        SlangNVVMValueHandle value = nullptr;
        SlangNVVMBlockHandle entryBlock = nullptr;
        SLANG_RETURN_ON_FAIL(builder.getFunctionType(
            module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType));
        SLANG_RETURN_ON_FAIL(builder.declareFunction(
            module,
            functionType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            toSlice(kCopyScalarKernelName),
            function));
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
        SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
        SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
        SLANG_RETURN_ON_FAIL(builder.emitLoad(module, source, 4, SLANG_NVVM_LOAD_FLAG_NONE, value));
        SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destination, 4));
        SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
        SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    }
    return SLANG_OK;
}

static SlangResult _buildScalarReferenceModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(
        builder.createModule(toSlice("slang-nvvm-scalar-reference"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateScalarReferenceKernels(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _compileRealNVVMBitcode(
    const String& nvvmPath,
    const void* bitcode,
    size_t bitcodeSize,
    ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_RETURN_ON_FAIL(_locateRealNVVM(nvvmPath, set, compiler));
    if (!compiler)
        return SLANG_FAIL;

    ComPtr<IArtifact> sourceArtifact = _createNVVMBitcodeArtifact(bitcode, bitcodeSize);
    CompileSettings settings;
    const SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outArtifact);
        if (SLANG_FAILED(compileResult))
            return compileResult;
        return diagnostics ? diagnostics->getResult() : SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _compileRealNVVMIRWithLibdevice(
    const String& cudaRoot,
    ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_RETURN_ON_FAIL(_locateRealNVVM(cudaRoot, set, compiler));
    if (!compiler)
        return SLANG_FAIL;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact(kLibdeviceSineNVVMIR);
    CompileSettings settings;
    settings.requiresCUDADeviceLibrary = true;
    const SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outArtifact);
        if (SLANG_FAILED(compileResult))
            return compileResult;
        return diagnostics ? diagnostics->getResult() : SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _compileRealNVRTCSource(
    const UnownedStringSlice& source,
    ComPtr<IArtifact>& outArtifact)
{
    outArtifact.setNull();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_RETURN_ON_FAIL(_locateRealNVRTC(set, compiler));
    if (!compiler)
        return SLANG_FAIL;

    ComPtr<IArtifact> sourceArtifact = _createCUDASourceArtifact(source);
    const SlangResult compileResult =
        _compileNVRTC(compiler, sourceArtifact, outArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outArtifact);
        if (SLANG_FAILED(compileResult))
            return compileResult;
        return diagnostics ? diagnostics->getResult() : SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _loadPTXText(IArtifact* artifact, String& outText)
{
    outText = String();
    if (!artifact)
        return SLANG_E_INVALID_ARG;

    ComPtr<ISlangBlob> blob;
    SLANG_RETURN_ON_FAIL(artifact->loadBlob(ArtifactKeep::Yes, blob.writeRef()));
    if (!blob || !blob->getBufferSize() || !blob->getBufferPointer())
        return SLANG_FAIL;

    UnownedStringSlice text(
        static_cast<const char*>(blob->getBufferPointer()),
        blob->getBufferSize());
    const Index terminatorIndex = text.indexOf('\0');
    if (terminatorIndex >= 0)
    {
        if (terminatorIndex != text.getLength() - 1)
            return SLANG_FAIL;
        text = text.head(terminatorIndex);
    }
    if (!text.getLength())
        return SLANG_FAIL;

    outText = String(text);
    return SLANG_OK;
}

static String _getBlobText(ISlangBlob* blob)
{
    if (!blob || !blob->getBufferPointer() || !blob->getBufferSize())
        return String();
    return String(UnownedStringSlice(
        static_cast<const char*>(blob->getBufferPointer()),
        blob->getBufferSize()));
}

static const char kDirectNVVMEmptyComputeSource[] =
    "[shader(\"compute\")] [numthreads(1, 1, 1)] void computeMain() {}";
static const char kDirectNVVMWriteScalarSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value;
}
)";

static SlangResult _populateFloat32ArithmeticKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    uint32_t operandCount,
    uint32_t operation)
{
    if (operandCount != 1 && operandCount != 2)
        return SLANG_E_INVALID_ARG;
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalFloatPointerType,
        floatType,
        floatType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle left = nullptr;
    SlangNVVMValueHandle right = nullptr;
    SlangNVVMValueHandle sum = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, voidType, parameterTypes, operandCount + 1, functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, left));
    if (operandCount == 2)
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, right));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    if (operandCount == 1)
    {
        SLANG_RETURN_ON_FAIL(
            builder.emitFloatingUnary(module, SlangNVVMValueOperation(operation), left, sum));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(
            builder
                .emitFloatingBinary(module, SlangNVVMValueOperation(operation), left, right, sum));
    }
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, sum, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _populateFloat32CopyKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalFloatPointerType,
        globalFloatPointerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.emitLoad(module, source, 4, SLANG_NVVM_LOAD_FLAG_NONE, value));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _populateFloat32ConstantKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    uint32_t bitPattern)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, voidType, &globalFloatPointerType, 1, functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(
        builder.getFloatingPointConstant(module, floatType, 32, bitPattern, value));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _populateFloat32PhiKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalFloatPointerType,
        integerType,
        floatType,
        floatType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        function));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle conditionValue = nullptr;
    SlangNVVMValueHandle left = nullptr;
    SlangNVVMValueHandle right = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, conditionValue));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, left));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 3, right));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle trueBlock = nullptr;
    SlangNVVMBlockHandle falseBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("true"), trueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("false"), falseBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("merge"), mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SlangNVVMValueHandle zero = nullptr;
    SlangNVVMValueHandle condition = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 0, zero));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerCompare(
        module,
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        conditionValue,
        zero,
        condition));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, condition, trueBlock, falseBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, trueBlock));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, falseBlock));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));

    SlangNVVMValueHandle phi = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitPhi(module, mergeBlock, floatType, phi));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, mergeBlock));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, phi, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.addPhiIncoming(module, phi, left, trueBlock));
    SLANG_RETURN_ON_FAIL(builder.addPhiIncoming(module, phi, right, falseBlock));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _populateFloat32FunctionKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    const SlangNVVMTypeHandle helperParameterTypes[] = {floatType, floatType};
    SlangNVVMTypeHandle helperType = nullptr;
    SlangNVVMValueHandle helper = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        floatType,
        helperParameterTypes,
        SLANG_COUNT_OF(helperParameterTypes),
        helperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        helperName,
        helper));
    SlangNVVMValueHandle helperLeft = nullptr;
    SlangNVVMValueHandle helperRight = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 0, helperLeft));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 1, helperRight));

    const SlangNVVMTypeHandle kernelParameterTypes[] = {
        globalFloatPointerType,
        floatType,
        floatType,
    };
    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle kernelLeft = nullptr;
    SlangNVVMValueHandle kernelRight = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, kernelLeft));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 2, kernelRight));

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, helper, toSlice("entry"), helperBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, helperBlock));
    SlangNVVMValueHandle sum = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitFloatingBinary(module, SLANG_NVVM_VALUE_OP_ADD, helperLeft, helperRight, sum));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, sum));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    const SlangNVVMValueHandle arguments[] = {kernelLeft, kernelRight};
    SlangNVVMValueHandle result = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitCall(module, helper, arguments, SLANG_COUNT_OF(arguments), result));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, result, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _emitNVVMTestIntrinsic(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    const SlangNVVMValueTypeDesc& valueType,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle& outValue)
{
    SlangNVVMValueTypeDesc operandTypes[3] = {};
    SlangNVVMValueTypeDesc resultType = valueType;
    switch (operation)
    {
    case SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX:
    case SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT:
        resultType = NVVMSemantics::kUnsignedI32;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT:
        operandTypes[0] = NVVMSemantics::kUnsignedI32;
        operandTypes[1] = valueType;
        operandTypes[2] = NVVMSemantics::kSignedI32;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_FIRST:
        operandTypes[0] = NVVMSemantics::kUnsignedI32;
        operandTypes[1] = valueType;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ANY_TRUE:
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_TRUE:
        resultType = operation == SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT ? NVVMSemantics::kUnsignedI32
                                                                       : NVVMSemantics::kBool;
        operandTypes[0] = NVVMSemantics::kUnsignedI32;
        operandTypes[1] = NVVMSemantics::kBool;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE:
        resultType = NVVMSemantics::kBool;
        operandTypes[0] = NVVMSemantics::kUnsignedI32;
        break;
    case SLANG_NVVM_VALUE_OP_WAVE_MASK_ALL_EQUAL:
        resultType = NVVMSemantics::kBool;
        operandTypes[0] = NVVMSemantics::kUnsignedI32;
        operandTypes[1] = valueType;
        break;
    default:
        return SLANG_E_INVALID_ARG;
    }
    const SlangNVVMValueOperationDesc desc = {
        operation,
        resultType,
        argumentCount ? operandTypes : nullptr,
        argumentCount,
    };
    return builder.emitValueOperation(module, desc, arguments, argumentCount, outValue);
}

static SlangResult _emitNVVMTestIntrinsic(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    SlangNVVMValueOperation operation,
    const SlangNVVMValueHandle* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle& outValue)
{
    return _emitNVVMTestIntrinsic(
        builder,
        module,
        operation,
        NVVMSemantics::kSignedI32,
        arguments,
        argumentCount,
        outValue);
}

static SlangResult _populateWaveIntrinsicKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& laneIndexHelperName,
    const UnownedStringSlice* laneCountHelperName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle helperType = nullptr;
    SlangNVVMValueHandle laneIndexHelper = nullptr;
    SlangNVVMValueHandle laneCountHelper = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(module, integerType, nullptr, 0, helperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        laneIndexHelperName,
        laneIndexHelper));
    if (laneCountHelperName)
    {
        SLANG_RETURN_ON_FAIL(builder.declareFunction(
            module,
            helperType,
            SLANG_NVVM_LINKAGE_EXTERNAL,
            SLANG_NVVM_FUNCTION_FLAG_NONE,
            *laneCountHelperName,
            laneCountHelper));
    }

    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, voidType, &globalIntegerPointerType, 1, kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));
    SlangNVVMValueHandle destination = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));

    SlangNVVMBlockHandle laneIndexHelperBlock = nullptr;
    SlangNVVMBlockHandle laneCountHelperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, laneIndexHelper, toSlice("entry"), laneIndexHelperBlock));
    if (laneCountHelper)
    {
        SLANG_RETURN_ON_FAIL(
            builder.createBlock(module, laneCountHelper, toSlice("entry"), laneCountHelperBlock));
    }
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, laneIndexHelperBlock));
    SlangNVVMValueHandle laneIndex = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
        nullptr,
        0,
        laneIndex));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, laneIndex));

    if (laneCountHelper)
    {
        SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, laneCountHelperBlock));
        SlangNVVMValueHandle laneCount = nullptr;
        SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
            builder,
            module,
            SLANG_NVVM_VALUE_OP_WAVE_LANE_COUNT,
            nullptr,
            0,
            laneCount));
        SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, laneCount));
    }

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle laneIndexResult = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(module, laneIndexHelper, nullptr, 0, laneIndexResult));
    SlangNVVMValueHandle storedValue = laneIndexResult;
    SlangNVVMValueHandle storePointer = destination;
    if (laneCountHelper)
    {
        SLANG_RETURN_ON_FAIL(builder.emitCall(module, laneCountHelper, nullptr, 0, storedValue));
        SLANG_RETURN_ON_FAIL(
            builder.emitPointerOffset(module, destination, laneIndexResult, storePointer));
    }
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, storedValue, storePointer, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _populateWaveLaneIndexKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName)
{
    return _populateWaveIntrinsicKernel(builder, module, kernelName, helperName, nullptr);
}

static SlangResult _populateWaveLaneCountKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& laneIndexHelperName,
    const UnownedStringSlice& laneCountHelperName)
{
    return _populateWaveIntrinsicKernel(
        builder,
        module,
        kernelName,
        laneIndexHelperName,
        &laneCountHelperName);
}

static SlangResult _populateWaveReadLaneAtUIntKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& laneIndexHelperName,
    const UnownedStringSlice& readLaneHelperName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle laneIndexHelperType = nullptr;
    SlangNVVMValueHandle laneIndexHelper = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, integerType, nullptr, 0, laneIndexHelperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        laneIndexHelperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        laneIndexHelperName,
        laneIndexHelper));

    SlangNVVMTypeHandle readLaneHelperType = nullptr;
    SlangNVVMValueHandle readLaneHelper = nullptr;
    SlangNVVMTypeHandle readLaneParameterTypes[] = {integerType, integerType, integerType};
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        integerType,
        readLaneParameterTypes,
        SLANG_COUNT_OF(readLaneParameterTypes),
        readLaneHelperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        readLaneHelperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        readLaneHelperName,
        readLaneHelper));

    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SlangNVVMTypeHandle kernelParameterTypes[] = {
        globalIntegerPointerType,
        integerType,
        integerType};
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle mask = nullptr;
    SlangNVVMValueHandle sourceLane = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, mask));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 2, sourceLane));

    SlangNVVMBlockHandle laneIndexBlock = nullptr;
    SlangNVVMBlockHandle readLaneBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, laneIndexHelper, toSlice("entry"), laneIndexBlock));
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, readLaneHelper, toSlice("entry"), readLaneBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, laneIndexBlock));
    SlangNVVMValueHandle laneIndex = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
        nullptr,
        0,
        laneIndex));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, laneIndex));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, readLaneBlock));
    SlangNVVMValueHandle readLaneArguments[3] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(readLaneArguments); ++i)
    {
        SLANG_RETURN_ON_FAIL(
            builder.getFunctionParameter(module, readLaneHelper, size_t(i), readLaneArguments[i]));
    }
    SlangNVVMValueHandle readLaneValue = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT,
        NVVMSemantics::kUnsignedI32,
        readLaneArguments,
        SLANG_COUNT_OF(readLaneArguments),
        readLaneValue));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, readLaneValue));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle laneIndexResult = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(module, laneIndexHelper, nullptr, 0, laneIndexResult));
    SlangNVVMValueHandle kernelReadLaneArguments[] = {mask, laneIndexResult, sourceLane};
    SlangNVVMValueHandle storedValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(
        module,
        readLaneHelper,
        kernelReadLaneArguments,
        SLANG_COUNT_OF(kernelReadLaneArguments),
        storedValue));
    SlangNVVMValueHandle storePointer = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitPointerOffset(module, destination, laneIndexResult, storePointer));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, storedValue, storePointer, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _populateWaveReadLaneAtLoadedScalarKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& laneIndexHelperName,
    const UnownedStringSlice& readLaneHelperName,
    SlangNVVMTypeHandle integerType,
    SlangNVVMTypeHandle payloadType,
    const SlangNVVMValueTypeDesc& payloadSemanticType,
    SlangNVVMValueOperation operation)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle globalPayloadPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        payloadType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalPayloadPointerType));

    SlangNVVMTypeHandle laneIndexHelperType = nullptr;
    SlangNVVMValueHandle laneIndexHelper = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, integerType, nullptr, 0, laneIndexHelperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        laneIndexHelperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        laneIndexHelperName,
        laneIndexHelper));

    SlangNVVMTypeHandle readLaneHelperType = nullptr;
    SlangNVVMValueHandle readLaneHelper = nullptr;
    SlangNVVMTypeHandle readLaneParameterTypes[] = {integerType, payloadType, integerType};
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        payloadType,
        readLaneParameterTypes,
        SLANG_COUNT_OF(readLaneParameterTypes),
        readLaneHelperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        readLaneHelperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        readLaneHelperName,
        readLaneHelper));

    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SlangNVVMTypeHandle kernelParameterTypes[] =
        {globalPayloadPointerType, globalPayloadPointerType, integerType, integerType};
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle mask = nullptr;
    SlangNVVMValueHandle sourceLane = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, source));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 2, mask));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 3, sourceLane));

    SlangNVVMBlockHandle laneIndexBlock = nullptr;
    SlangNVVMBlockHandle readLaneBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, laneIndexHelper, toSlice("entry"), laneIndexBlock));
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, readLaneHelper, toSlice("entry"), readLaneBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, laneIndexBlock));
    SlangNVVMValueHandle laneIndex = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
        nullptr,
        0,
        laneIndex));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, laneIndex));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, readLaneBlock));
    SlangNVVMValueHandle readLaneArguments[3] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(readLaneArguments); ++i)
    {
        SLANG_RETURN_ON_FAIL(
            builder.getFunctionParameter(module, readLaneHelper, size_t(i), readLaneArguments[i]));
    }
    SlangNVVMValueHandle readLaneValue = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        operation,
        payloadSemanticType,
        readLaneArguments,
        SLANG_COUNT_OF(readLaneArguments),
        readLaneValue));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, readLaneValue));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle laneIndexResult = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(module, laneIndexHelper, nullptr, 0, laneIndexResult));
    SlangNVVMValueHandle sourcePointer = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitPointerOffset(module, source, laneIndexResult, sourcePointer));
    SlangNVVMValueHandle sourceValue = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitLoad(module, sourcePointer, 4, SLANG_NVVM_LOAD_FLAG_NONE, sourceValue));
    SlangNVVMValueHandle kernelReadLaneArguments[] = {mask, sourceValue, sourceLane};
    SlangNVVMValueHandle storedValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(
        module,
        readLaneHelper,
        kernelReadLaneArguments,
        SLANG_COUNT_OF(kernelReadLaneArguments),
        storedValue));
    SlangNVVMValueHandle storePointer = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitPointerOffset(module, destination, laneIndexResult, storePointer));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, storedValue, storePointer, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _populateWaveReadLaneAtIntKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& laneIndexHelperName,
    const UnownedStringSlice& readLaneHelperName)
{
    SlangNVVMTypeHandle integerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    return _populateWaveReadLaneAtLoadedScalarKernel(
        builder,
        module,
        kernelName,
        laneIndexHelperName,
        readLaneHelperName,
        integerType,
        integerType,
        NVVMSemantics::kSignedI32,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT);
}

static SlangResult _populateWaveReadLaneAtFloatKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& laneIndexHelperName,
    const UnownedStringSlice& readLaneHelperName)
{
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    return _populateWaveReadLaneAtLoadedScalarKernel(
        builder,
        module,
        kernelName,
        laneIndexHelperName,
        readLaneHelperName,
        integerType,
        floatType,
        NVVMSemantics::kFloat32,
        SLANG_NVVM_VALUE_OP_WAVE_READ_LANE_AT);
}

static SlangResult _populateWaveActiveMaskKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle boolType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 1, boolType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, voidType, &globalIntegerPointerType, 1, kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));
    SlangNVVMValueHandle destination = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));

    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));

    SlangNVVMValueHandle laneIndex = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_LANE_INDEX,
        nullptr,
        0,
        laneIndex));
    SlangNVVMValueHandle storePointer = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitPointerOffset(module, destination, laneIndex, storePointer));

    SlangNVVMValueHandle fullMask = nullptr;
    SlangNVVMValueHandle trueValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, -1, fullMask));
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, boolType, 1, trueValue));
    const SlangNVVMValueHandle arguments[] = {fullMask, trueValue};
    SlangNVVMValueHandle activeMask = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT,
        arguments,
        SLANG_COUNT_OF(arguments),
        activeMask));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, activeMask, storePointer, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _populateWaveIsFirstLaneKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle boolType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 1, boolType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle helperType = nullptr;
    SlangNVVMValueHandle helper = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(module, boolType, &integerType, 1, helperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        helperName,
        helper));

    SlangNVVMTypeHandle kernelParameterTypes[] = {globalIntegerPointerType, integerType};
    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle mask = nullptr;
    SlangNVVMValueHandle helperMask = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, mask));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 0, helperMask));

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, helper, toSlice("entry"), helperBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, helperBlock));
    SlangNVVMValueHandle isFirst = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_IS_FIRST_LANE,
        &helperMask,
        1,
        isFirst));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, isFirst));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle predicate = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(module, helper, &mask, 1, predicate));
    const SlangNVVMValueHandle ballotArguments[] = {mask, predicate};
    SlangNVVMValueHandle ballot = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT,
        ballotArguments,
        SLANG_COUNT_OF(ballotArguments),
        ballot));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, ballot, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

enum class WavePredicateValueKind
{
    Boolean,
    Integer,
    Float,
};

static SlangResult _populateWavePredicateIntrinsicKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& helperName,
    SlangNVVMValueOperation operation,
    WavePredicateValueKind valueKind)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle boolType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 1, boolType));
    if (valueKind == WavePredicateValueKind::Float)
        SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle helperValueType = valueKind == WavePredicateValueKind::Boolean ? boolType
                                          : valueKind == WavePredicateValueKind::Float
                                              ? floatType
                                              : integerType;
    SlangNVVMTypeHandle helperParameterTypes[] = {integerType, helperValueType};
    SlangNVVMTypeHandle helperType = nullptr;
    SlangNVVMValueHandle helper = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        boolType,
        helperParameterTypes,
        SLANG_COUNT_OF(helperParameterTypes),
        helperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        helperName,
        helper));

    SlangNVVMTypeHandle kernelParameterTypes[] = {globalIntegerPointerType, integerType};
    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle mask = nullptr;
    SlangNVVMValueHandle helperMask = nullptr;
    SlangNVVMValueHandle helperValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, mask));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 0, helperMask));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 1, helperValue));

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, helper, toSlice("entry"), helperBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, helperBlock));
    const SlangNVVMValueHandle intrinsicArguments[] = {helperMask, helperValue};
    SlangNVVMValueHandle predicate = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        operation,
        valueKind == WavePredicateValueKind::Float     ? NVVMSemantics::kFloat32
        : valueKind == WavePredicateValueKind::Boolean ? NVVMSemantics::kBool
                                                       : NVVMSemantics::kSignedI32,
        intrinsicArguments,
        SLANG_COUNT_OF(intrinsicArguments),
        predicate));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, predicate));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle value = nullptr;
    if (valueKind == WavePredicateValueKind::Float)
    {
        SLANG_RETURN_ON_FAIL(builder.getFloatingPointConstant(
            module,
            helperValueType,
            32,
            UINT64_C(0x3f800000),
            value));
    }
    else
    {
        SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, helperValueType, 1, value));
    }
    const SlangNVVMValueHandle callArguments[] = {mask, value};
    predicate = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitCall(module, helper, callArguments, SLANG_COUNT_OF(callArguments), predicate));
    const SlangNVVMValueHandle ballotArguments[] = {mask, predicate};
    SlangNVVMValueHandle ballot = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        SLANG_NVVM_VALUE_OP_WAVE_MASK_BALLOT,
        ballotArguments,
        SLANG_COUNT_OF(ballotArguments),
        ballot));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, ballot, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _populateWaveReadLaneFirstKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const UnownedStringSlice& kernelName,
    const UnownedStringSlice& readFirstHelperName,
    SlangNVVMValueOperation operation,
    bool usesFloatValue)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle valueType = nullptr;
    SlangNVVMTypeHandle globalValuePointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    if (usesFloatValue)
    {
        SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, valueType));
    }
    else
    {
        valueType = integerType;
    }
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        valueType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalValuePointerType));

    SlangNVVMTypeHandle helperParameterTypes[] = {integerType, valueType};
    SlangNVVMTypeHandle helperType = nullptr;
    SlangNVVMValueHandle helper = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        valueType,
        helperParameterTypes,
        SLANG_COUNT_OF(helperParameterTypes),
        helperType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        readFirstHelperName,
        helper));

    SlangNVVMTypeHandle kernelParameterTypes[] = {globalValuePointerType, integerType, valueType};
    SlangNVVMTypeHandle kernelType = nullptr;
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        kernelName,
        kernel));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle mask = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, mask));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 2, value));

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, helper, toSlice("entry"), helperBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, helperBlock));
    SlangNVVMValueHandle helperArguments[2] = {};
    for (Index i = 0; i < SLANG_COUNT_OF(helperArguments); ++i)
    {
        SLANG_RETURN_ON_FAIL(
            builder.getFunctionParameter(module, helper, size_t(i), helperArguments[i]));
    }
    SlangNVVMValueHandle firstValue = nullptr;
    SLANG_RETURN_ON_FAIL(_emitNVVMTestIntrinsic(
        builder,
        module,
        operation,
        usesFloatValue ? NVVMSemantics::kFloat32 : NVVMSemantics::kSignedI32,
        helperArguments,
        SLANG_COUNT_OF(helperArguments),
        firstValue));
    SLANG_RETURN_ON_FAIL(builder.emitValueReturn(module, firstValue));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle kernelArguments[] = {mask, value};
    SlangNVVMValueHandle storedValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitCall(
        module,
        helper,
        kernelArguments,
        SLANG_COUNT_OF(kernelArguments),
        storedValue));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, storedValue, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static const char kDirectNVVMFloat32AddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left + right;
}
)";

static const char kDirectNVVMFloat16ValueSource[] = R"(
half2 chooseHalf2(half2 left, half2 right, bool chooseLeft)
{
    half2 selected;
    if (chooseLeft)
        selected = left;
    else
        selected = right;
    return selected;
}

half2 adjustHalf2(half2 value)
{
    return -(value + half2(1.0h, 2.0h));
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right,
    uniform int integerValue)
{
    half first = half(left);
    half second = half(integerValue);
    half2 pair = half2(first, second);
    half2 converted = half2(float2(right, float(integerValue + 1)));
    half2 result = adjustHalf2(chooseHalf2(pair, converted, left > right));
    bool2 compared = result < pair;
    float2 widened = float2(result);
    int2 integers = int2(result);
    half selectedLane = result[integerValue & 1];
    *destination =
        widened.x + float(integers.y) + float(selectedLane) + (compared.x ? 1.0 : 0.0);
}
)";

static const char kDirectNVVMOpaqueHalfConversionSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half narrowed = f32tof16_(input);
    *destination = f16tof32(narrowed);
}
)";

static const char kDirectNVVMUnsupportedOpaqueHalfConversionSignatureSource[] = R"(
half malformedFloatToHalf(float input, int extra)
{
    __target_switch
    {
    case cuda: __intrinsic_asm "__float2half";
    default: return half(input + float(extra));
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half narrowed = malformedFloatToHalf(input, 0);
    *destination = f16tof32(narrowed);
}
)";

static const char kDirectNVVMUnsupportedSurfaceSignatureSource[] = R"SLANG(
RWTexture2D<half> surface;

half malformedSurfaceLoad(RWTexture2D<half> resource, int2 coordinate, int extra)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm
            "surf2Dread$C<$T0>($0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
    default:
        return half(extra);
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = float(malformedSurfaceLoad(surface, int2(x, y), 0));
}
)SLANG";

static const char kDirectNVVMLocalVectorSwizzleSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half4 value = half4(
        half(input),
        half(input + 1.0),
        half(input + 2.0),
        half(input + 3.0));
    value.xyz = -value.zwx;
    *destination = float(value.x + value.y + value.z + value.w);
}
)";

static const char kDirectNVVMStatefulAggregateHelperSource[] = R"(
struct Counter
{
    __init(int initialValue)
    {
        value = initialValue;
    }

    [mutating] int next()
    {
        int result = value;
        value++;
        return result;
    }

    int value;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int initialValue)
{
    Counter counter = Counter(initialValue);
    *destination = counter.next() + counter.next();
}
)";

static const char kDirectNVVMThreadLocalGlobalContextSource[] = R"(
static int accumulator = 7;

int accumulate(int value)
{
    accumulator += value;
    return accumulator;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = accumulate(value);
}
)";

static const char kDirectNVVMCopyableValueHelperSource[] = R"(
struct Payload
{
    int bias;
    float4 lanes;
};

Payload addOffset(Payload value, inout int offset)
{
    value.bias += offset;
    offset += 1;
    return value;
}

float readValue(Payload value)
{
    return float(value.bias) + value.lanes.x;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int input)
{
    Payload value;
    value.bias = input;
    value.lanes = float4(1.0, 2.0, 3.0, 4.0);
    int offset = 2;
    value = addOffset(value, offset);
    *destination = readValue(value) + float(offset);
}
)";

static const char kDirectNVVMLocalArrayHelperSource[] = R"(
void initializeArray(out float3 values[4])
{
    values[0] = float3(1.0, 1.0, 1.0);
    values[1] = float3(2.0, 2.0, 2.0);
    values[2] = float3(3.0, 3.0, 3.0);
    values[3] = float3(4.0, 4.0, 4.0);
}

void updateArray(inout float3 values[4])
{
    values[0] = float3(5.0, 5.0, 5.0);
}

[CUDAKernel]
void computeMain(uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    float3 values[4];
    initializeArray(values);
    updateArray(values);
    *destination = values[0].x;
}
)";

static const char kDirectNVVMCopyableStructuredBufferAggregateSource[] = R"(
struct Thing
{
    uint pos;
    float radius;
    half4 color;
};

[CUDAKernel]
void computeMain(RWStructuredBuffer<Thing> destination, uniform uint index)
{
    Thing value;
    value.pos = index;
    value.radius = float(index);
    value.color = half4(1.0h, 2.0h, 3.0h, 4.0h);
    destination[index] = value;
}
)";

static const char kDirectNVVMIncompatibleStructuredBufferAggregateLayoutSource[] = R"(
struct MisalignedThing
{
    half leading;
    half4 payload;
};

[CUDAKernel]
void computeMain(RWStructuredBuffer<MisalignedThing> destination)
{
    MisalignedThing value;
    value.leading = 1.0h;
    value.payload = half4(2.0h, 3.0h, 4.0h, 5.0h);
    destination[0] = value;
}
)";

static const char kDirectNVVMDynamicLocalVectorStoreSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input,
    uniform int index)
{
    half4 value = half4(1.0h, 2.0h, 3.0h, 4.0h);
    value[index & 3] = half(input);
    *destination = float(value.x + value.y + value.z + value.w);
}
)";

static const char kDirectNVVMFloatMatrixValueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float a,
    uniform float b)
{
    float2x2 ma = float2x2(a, b, b, a);
    float2x2 mb = ma + 1.0;
    float2x2 ms = ma + mb;
    float2x2 selected;
    if (a > b)
        selected = ms;
    else
        selected = mb;
    *destination = selected[1][1];
}
)";

static const char kDirectNVVMMatrixMemorySource[] = R"(
ConstantBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<float> outputBuffer;

[CudaDeviceExport]
float __slang_nvvm_internal_0(float value)
{
    return value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float4x4 input = matrixBuffer;
    float4x4 squared = mul(input, input);
    float4 transformed = mul(float4(1.0, 2.0, 3.0, 1.0), input);
    outputBuffer[0] = __slang_nvvm_internal_0(squared[0][0] + transformed.x);
}
)";

static const char kDirectNVVMStructuredMatrixMemorySource[] = R"(
RWStructuredBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int value = int(tid.x);
    outputBuffer[tid.x] = asint(matrixBuffer[0][(value + 1) & 3][(value + 3) & 3]);
}
)";

static const char kDirectNVVMUnsupportedStructuredMatrixWriteSource[] = R"(
RWStructuredBuffer<float4x4> matrixBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    matrixBuffer[0][tid.x & 3][tid.y & 3] = float(tid.x);
}
)";

static SlangResult _populateNumericFamilyFunction(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle int8Type = nullptr;
    SlangNVVMTypeHandle int16Type = nullptr;
    SlangNVVMTypeHandle int32Type = nullptr;
    SlangNVVMTypeHandle int64Type = nullptr;
    SlangNVVMTypeHandle boolTypeHandle = nullptr;
    SlangNVVMTypeHandle halfType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle int2Type = nullptr;
    SlangNVVMTypeHandle int8x2Type = nullptr;
    SlangNVVMTypeHandle bool2Type = nullptr;
    SlangNVVMTypeHandle half2Type = nullptr;
    SlangNVVMTypeHandle float3Type = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 8, int8Type));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 16, int16Type));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, int32Type));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 64, int64Type));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 1, boolTypeHandle));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 16, halfType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getVectorType(module, int32Type, 2, int2Type));
    SLANG_RETURN_ON_FAIL(builder.getVectorType(module, int8Type, 2, int8x2Type));
    SLANG_RETURN_ON_FAIL(builder.getVectorType(module, boolTypeHandle, 2, bool2Type));
    SLANG_RETURN_ON_FAIL(builder.getVectorType(module, halfType, 2, half2Type));
    SLANG_RETURN_ON_FAIL(builder.getVectorType(module, floatType, 3, float3Type));

    const SlangNVVMTypeHandle parameterTypes[] = {
        int8Type,
        int8Type,
        floatType,
        int2Type,
        int2Type,
        int8x2Type,
        int8x2Type,
        float3Type,
        float3Type,
        int32Type,
        boolTypeHandle,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        int2Type,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice("numericFamilies"),
        function));

    SlangNVVMValueHandle parameters[SLANG_COUNT_OF(parameterTypes)] = {};
    for (size_t i = 0; i < SLANG_COUNT_OF(parameters); ++i)
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, i, parameters[i]));
    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    const SlangNVVMValueTypeDesc signedI8 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        8,
        1,
    };
    const SlangNVVMValueTypeDesc unsignedI8 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        8,
        1,
    };
    const SlangNVVMValueTypeDesc unsignedI16 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        16,
        1,
    };
    const SlangNVVMValueTypeDesc signedI64 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        64,
        1,
    };
    const SlangNVVMValueTypeDesc float32 = NVVMSemantics::kFloat32;
    const SlangNVVMValueTypeDesc float16 = NVVMSemantics::kFloat16;
    const SlangNVVMValueTypeDesc boolType = NVVMSemantics::kBool;
    const SlangNVVMValueTypeDesc signedI32x2 = NVVMSemantics::kSignedI32x2;
    const SlangNVVMValueTypeDesc signedI32 = NVVMSemantics::kSignedI32;
    const SlangNVVMValueTypeDesc unsignedI32x2 = {
        SLANG_NVVM_VALUE_TYPE_UNSIGNED_INTEGER,
        32,
        2,
    };
    const SlangNVVMValueTypeDesc signedI8x2 = {
        SLANG_NVVM_VALUE_TYPE_SIGNED_INTEGER,
        8,
        2,
    };
    const SlangNVVMValueTypeDesc bool2 = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 2};
    const SlangNVVMValueTypeDesc float32x3 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        32,
        3,
    };
    const SlangNVVMValueTypeDesc float16x2 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        16,
        2,
    };
    const SlangNVVMValueTypeDesc float32x2 = {
        SLANG_NVVM_VALUE_TYPE_FLOATING_POINT,
        32,
        2,
    };

    auto emitOperation = [&](SlangNVVMValueOperation operation,
                             const SlangNVVMValueTypeDesc& resultType,
                             const SlangNVVMValueTypeDesc* operandTypes,
                             const SlangNVVMValueHandle* operands,
                             size_t operandCount,
                             SlangNVVMValueHandle& outValue)
    {
        const SlangNVVMValueOperationDesc desc = {
            operation,
            resultType,
            operandTypes,
            operandCount,
        };
        return builder.emitValueOperation(module, desc, operands, operandCount, outValue);
    };

    SlangNVVMValueHandle ignored = nullptr;
    SlangNVVMValueTypeDesc operandTypes[2] = {signedI8, signedI8};
    SlangNVVMValueHandle operands[2] = {parameters[0], parameters[1]};
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_ADD, signedI8, operandTypes, operands, 2, ignored));
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_LESS_THAN, boolType, operandTypes, operands, 2, ignored));
    operandTypes[0] = unsignedI8;
    operandTypes[1] = unsignedI8;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        boolType,
        operandTypes,
        operands,
        2,
        ignored));

    operandTypes[0] = signedI8;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
        signedI64,
        operandTypes,
        operands,
        1,
        ignored));
    operandTypes[0] = unsignedI8;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_INTEGER_CONVERT,
        signedI64,
        operandTypes,
        operands + 1,
        1,
        ignored));
    operandTypes[0] = signedI8;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT,
        float32,
        operandTypes,
        operands,
        1,
        ignored));
    operandTypes[0] = float32;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER,
        unsignedI16,
        operandTypes,
        parameters + 2,
        1,
        ignored));
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        signedI32,
        operandTypes,
        parameters + 2,
        1,
        ignored));
    operandTypes[0] = signedI32;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_BIT_REINTERPRET,
        float32,
        operandTypes,
        parameters + 9,
        1,
        ignored));

    SlangNVVMValueHandle halfValues[2] = {};
    operandTypes[0] = signedI8;
    for (Index i = 0; i < 2; ++i)
    {
        SLANG_RETURN_ON_FAIL(emitOperation(
            SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT,
            float16,
            operandTypes,
            parameters + i,
            1,
            halfValues[i]));
    }
    SlangNVVMValueHandle halfOne = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointConstant(module, halfType, 16, 0x3c00, halfOne));
    operandTypes[0] = float16;
    operandTypes[1] = float16;
    operands[0] = halfValues[0];
    operands[1] = halfOne;
    SlangNVVMValueHandle halfSum = nullptr;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_ADD, float16, operandTypes, operands, 2, halfSum));
    operands[0] = halfSum;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_NEGATE, float16, operandTypes, operands, 1, ignored));
    operands[1] = halfValues[1];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_LESS_THAN, boolType, operandTypes, operands, 2, ignored));
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        float32,
        operandTypes,
        operands,
        1,
        ignored));
    operandTypes[0] = float32;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        float16,
        operandTypes,
        parameters + 2,
        1,
        ignored));
    operandTypes[0] = float16;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER,
        signedI8,
        operandTypes,
        &halfSum,
        1,
        ignored));

    const SlangNVVMValueHandle halfElements[] = {halfValues[0], halfValues[1]};
    SlangNVVMValueHandle constructedHalf2 = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitVectorConstruct(
        module,
        half2Type,
        halfElements,
        SLANG_COUNT_OF(halfElements),
        constructedHalf2));
    SlangNVVMValueHandle zeroIndex = nullptr;
    SlangNVVMValueHandle extractedHalf = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, int32Type, 0, zeroIndex));
    SLANG_RETURN_ON_FAIL(
        builder.emitSequentialElementExtract(module, constructedHalf2, zeroIndex, extractedHalf));

    operandTypes[0] = signedI32x2;
    operandTypes[1] = signedI32x2;
    operands[0] = parameters[3];
    operands[1] = parameters[4];
    SlangNVVMValueHandle vectorSum = nullptr;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_ADD, signedI32x2, operandTypes, operands, 2, vectorSum));

    SlangNVVMValueHandle halfVectors[2] = {};
    operandTypes[0] = signedI32x2;
    for (Index i = 0; i < 2; ++i)
    {
        SLANG_RETURN_ON_FAIL(emitOperation(
            SLANG_NVVM_VALUE_OP_INTEGER_TO_FLOAT,
            float16x2,
            operandTypes,
            parameters + 3 + i,
            1,
            halfVectors[i]));
    }
    operandTypes[0] = float16x2;
    operandTypes[1] = float16x2;
    operands[0] = halfVectors[0];
    operands[1] = halfVectors[1];
    SlangNVVMValueHandle halfVectorSum = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_ADD,
        float16x2,
        operandTypes,
        operands,
        2,
        halfVectorSum));
    operands[0] = halfVectorSum;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_NEGATE, float16x2, operandTypes, operands, 1, ignored));
    operands[1] = halfVectors[1];
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
        bool2,
        operandTypes,
        operands,
        2,
        ignored));
    SlangNVVMValueHandle floatVector = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        float32x2,
        operandTypes,
        operands,
        1,
        floatVector));
    operandTypes[0] = float32x2;
    SlangNVVMValueHandle narrowedHalfVector = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_CONVERT,
        float16x2,
        operandTypes,
        &floatVector,
        1,
        narrowedHalfVector));
    operandTypes[0] = float16x2;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_FLOAT_TO_INTEGER,
        signedI32x2,
        operandTypes,
        &narrowedHalfVector,
        1,
        ignored));

    operandTypes[0] = signedI32x2;
    operandTypes[1] = signedI32;
    operands[0] = parameters[3];
    operands[1] = parameters[9];
    SlangNVVMValueHandle broadcastSum = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_ADD,
        signedI32x2,
        operandTypes,
        operands,
        2,
        broadcastSum));
    operandTypes[0] = signedI32;
    operandTypes[1] = signedI32x2;
    operands[0] = parameters[9];
    operands[1] = parameters[4];
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SUBTRACT,
        signedI32x2,
        operandTypes,
        operands,
        2,
        ignored));
    operandTypes[0] = unsignedI32x2;
    operandTypes[1] = signedI32;
    operands[0] = parameters[3];
    operands[1] = parameters[9];
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SHIFT_RIGHT,
        unsignedI32x2,
        operandTypes,
        operands,
        2,
        ignored));
    operandTypes[0] = signedI32x2;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_EQUAL, bool2, operandTypes, operands, 2, ignored));

    operandTypes[0] = signedI32x2;
    operandTypes[1] = signedI32x2;
    operands[0] = parameters[3];
    operands[1] = parameters[4];
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SHIFT_LEFT,
        signedI32x2,
        operandTypes,
        operands,
        2,
        ignored));
    operandTypes[0] = unsignedI32x2;
    operandTypes[1] = unsignedI32x2;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SHIFT_RIGHT,
        unsignedI32x2,
        operandTypes,
        operands,
        2,
        ignored));
    SlangNVVMValueHandle vectorComparison = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_EQUAL,
        bool2,
        operandTypes,
        operands,
        2,
        vectorComparison));
    SlangNVVMValueHandle dynamicBooleanLane = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitSequentialElementExtract(
        module,
        vectorComparison,
        parameters[9],
        dynamicBooleanLane));
    const SlangNVVMValueHandle booleanElements[] = {parameters[10], dynamicBooleanLane};
    SlangNVVMValueHandle constructedBooleanVector = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitVectorConstruct(
        module,
        bool2Type,
        booleanElements,
        SLANG_COUNT_OF(booleanElements),
        constructedBooleanVector));

    operandTypes[0] = signedI8x2;
    operandTypes[1] = signedI8x2;
    operands[0] = parameters[5];
    operands[1] = parameters[6];
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SHIFT_RIGHT,
        signedI8x2,
        operandTypes,
        operands,
        2,
        ignored));
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_DIVIDE, signedI8x2, operandTypes, operands, 2, ignored));
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_REMAINDER,
        signedI8x2,
        operandTypes,
        operands,
        2,
        ignored));
    operandTypes[1] = signedI8;
    operands[1] = parameters[0];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_LESS_THAN, bool2, operandTypes, operands, 2, ignored));

    operandTypes[0] = float32x3;
    operandTypes[1] = float32x3;
    operands[0] = parameters[7];
    operands[1] = parameters[8];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_ADD, float32x3, operandTypes, operands, 2, ignored));
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_REMAINDER,
        float32x3,
        operandTypes,
        operands,
        2,
        ignored));
    operandTypes[0] = float32x3;
    operandTypes[1] = float32;
    operands[0] = parameters[7];
    operands[1] = parameters[2];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_MULTIPLY, float32x3, operandTypes, operands, 2, ignored));

    const SlangNVVMValueOperation floatComparisons[] = {
        SLANG_NVVM_VALUE_OP_EQUAL,
        SLANG_NVVM_VALUE_OP_NOT_EQUAL,
        SLANG_NVVM_VALUE_OP_LESS_THAN,
        SLANG_NVVM_VALUE_OP_GREATER_THAN,
        SLANG_NVVM_VALUE_OP_LESS_EQUAL,
        SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
    };
    const SlangNVVMValueTypeDesc bool3 = {SLANG_NVVM_VALUE_TYPE_BOOL, 1, 3};
    operandTypes[0] = float32x3;
    operandTypes[1] = float32x3;
    operands[0] = parameters[7];
    operands[1] = parameters[8];
    for (SlangNVVMValueOperation comparison : floatComparisons)
    {
        SLANG_RETURN_ON_FAIL(emitOperation(comparison, bool3, operandTypes, operands, 2, ignored));
    }
    operandTypes[1] = float32;
    operands[1] = parameters[2];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_LESS_THAN, bool3, operandTypes, operands, 2, ignored));
    operandTypes[0] = float32;
    operandTypes[1] = float32x3;
    operands[0] = parameters[2];
    operands[1] = parameters[8];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_GREATER_THAN, bool3, operandTypes, operands, 2, ignored));

    operandTypes[0] = bool2;
    operands[0] = vectorComparison;
    SlangNVVMValueHandle invertedComparison = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_BIT_NOT,
        bool2,
        operandTypes,
        operands,
        1,
        invertedComparison));
    operandTypes[1] = boolType;
    operands[1] = parameters[10];
    SlangNVVMValueHandle combinedComparison = nullptr;
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_BIT_AND,
        bool2,
        operandTypes,
        operands,
        2,
        combinedComparison));
    operandTypes[0] = boolType;
    operandTypes[1] = bool2;
    operands[0] = parameters[10];
    operands[1] = combinedComparison;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_BIT_OR, bool2, operandTypes, operands, 2, ignored));
    operandTypes[0] = bool2;
    operandTypes[1] = bool2;
    operands[0] = constructedBooleanVector;
    operands[1] = combinedComparison;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_EQUAL, bool2, operandTypes, operands, 2, ignored));
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_NOT_EQUAL, bool2, operandTypes, operands, 2, ignored));
    const SlangNVVMValueTypeDesc boolSelectOperandTypes[] = {bool2, bool2, bool2};
    const SlangNVVMValueHandle boolSelectOperands[] = {
        vectorComparison,
        constructedBooleanVector,
        combinedComparison,
    };
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SELECT,
        bool2,
        boolSelectOperandTypes,
        boolSelectOperands,
        SLANG_COUNT_OF(boolSelectOperands),
        ignored));

    const SlangNVVMValueTypeDesc integerSelectOperandTypes[] = {
        bool2,
        signedI32x2,
        signedI32x2,
    };
    const SlangNVVMValueHandle integerSelectOperands[] = {
        vectorComparison,
        parameters[3],
        parameters[4],
    };
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SELECT,
        signedI32x2,
        integerSelectOperandTypes,
        integerSelectOperands,
        SLANG_COUNT_OF(integerSelectOperands),
        ignored));

    const SlangNVVMValueTypeDesc halfSelectOperandTypes[] = {boolType, float16, float16};
    const SlangNVVMValueHandle halfSelectOperands[] = {
        parameters[10],
        halfValues[0],
        halfValues[1],
    };
    SLANG_RETURN_ON_FAIL(emitOperation(
        SLANG_NVVM_VALUE_OP_SELECT,
        float16,
        halfSelectOperandTypes,
        halfSelectOperands,
        SLANG_COUNT_OF(halfSelectOperands),
        ignored));
    operandTypes[1] = boolType;
    operands[1] = parameters[10];
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_EQUAL, bool2, operandTypes, operands, 2, ignored));
    operandTypes[0] = boolType;
    operandTypes[1] = bool2;
    operands[0] = parameters[10];
    operands[1] = constructedBooleanVector;
    SLANG_RETURN_ON_FAIL(
        emitOperation(SLANG_NVVM_VALUE_OP_NOT_EQUAL, bool2, operandTypes, operands, 2, ignored));
    return builder.emitValueReturn(module, broadcastSum);
}

static const char kDirectNVVMVectorOperationFamilySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    int2 shift = int2(7, -3) >> 1;
    int2 broadcastSum = int2(7, -3) + 2;
    int2 reverseDifference = 20 - int2(1, 3);
    int8_t2 narrow = int8_t2(-6, 7);
    int8_t2 quotient = narrow / int8_t2(2, 2);
    int8_t2 remainder = narrow % int8_t2(4, 4);
    bool2 negative = narrow < int8_t(0);
    bool predicate = destination[8] != 0;
    bool2 logic = (!negative && predicate) || negative;
    float3 sum = float3(1.5, 2.5, 3.5) + 0.5;
    float3 floatRemainder = float3(7.5, -7.5, 8.5) % float3(2.0, 2.0, 3.0);
    bool3 floatLess = sum < floatRemainder;
    bool2 explicitBoolean = bool2(predicate, negative.x);
    bool2 equalBoolean = explicitBoolean == logic;
    destination[0] = shift.y;
    destination[1] = int(quotient.x);
    destination[2] = int(remainder.y);
    destination[3] = negative.x ? 1 : 0;
    destination[4] = int(sum.z * 2.0);
    destination[5] = int(floatRemainder.y * 2.0);
    destination[6] = broadcastSum.x + reverseDifference.y;
    destination[7] = logic.y ? 1 : 0;
    destination[8] = floatLess.x ? 1 : 0;
    destination[9] = equalBoolean.y ? 1 : 0;
}
)";

static const char kDirectNVVMTypedSelectSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    int2 values = int2(value, value + 1);
    bool2 condition = values > 0;
    bool2 whenTrue = values < 4;
    bool2 whenFalse = values == 0;
    bool2 selected = condition ? whenTrue : whenFalse;
    *destination = all(selected) ? 1 : 0;
}
)";

static const char kDirectNVVMFlattenedVectorConstructionSource[] = R"(
[noinline]
half2 makePair(float left, float right)
{
    return half2(half(left), half(right));
}

[noinline]
half selectLane(half4 value, int index)
{
    return value[index & 3];
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float first,
    uniform float second,
    uniform float third,
    uniform float fourth,
    uniform int index)
{
    half2 pair = makePair(first, second);
    half4 combined = half4(pair, half(third), half(fourth));
    *destination = float(selectLane(combined, index));
}
)";

static const char kDirectNVVMWaveLaneIndexSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = laneIndex;
}
)";
static const char kDirectNVVMCUDAExecutionSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint3 threadIndex = cudaThreadIdx();
    uint3 blockIndex = cudaBlockIdx();
    uint3 blockDimensions = cudaBlockDim();
    uint3 gridDimensions = cudaGridDim();
    destination[0] = threadIndex.x;
    destination[1] = threadIndex.y;
    destination[2] = threadIndex.z;
    destination[3] = blockIndex.x;
    destination[4] = blockIndex.y;
    destination[5] = blockIndex.z;
    destination[6] = blockDimensions.x;
    destination[7] = blockDimensions.y;
    destination[8] = blockDimensions.z;
    destination[9] = gridDimensions.x;
    destination[10] = gridDimensions.y;
    destination[11] = gridDimensions.z;
    GroupMemoryBarrierWithGroupSync();
}
)";
static const char kDirectNVVMIntegerVectorSwizzleSource[] = R"(
[shader("compute")]
[numthreads(2, 2, 1)]
void computeMain(
    int2 dispatchThreadID : SV_DispatchThreadID,
    StructuredBuffer<uint> source,
    RWStructuredBuffer<uint> destination)
{
    destination[uint(dispatchThreadID.x)] = source[uint(dispatchThreadID.y)];
}
)";
static const char kDirectNVVMDynamicVectorIndexSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint index)
{
    uint3 threadIndex = cudaThreadIdx();
    destination[0] = threadIndex[index];
}
)";
static const char kDirectNVVMCUDATypeLayoutSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    destination[0] = __alignOf<uint8_t>();
    destination[1] = __alignOf<vector<half, 3> >();
    destination[2] = __alignOf<vector<half, 4> >();
    destination[3] = __alignOf<vector<double, 2> >();
    destination[4] = __sizeOf<vector<half, 3> >();
    destination[5] = __sizeOf<vector<double, 4> >();
}
)";
static const char kDirectNVVMCUDAAggregateLayoutSource[] = R"(
struct PadLadenStruct
{
    double a;
    uint8_t b;
};

struct StructWithArray : IDefaultInitializable
{
    PadLadenStruct a[1];
    uint8_t b;
    matrix<half, 3, 3> c;
    uint8_t d;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    StructWithArray value;
    destination[0] = __sizeOf(value);
    destination[1] = __offsetOf(value, value.a);
    destination[2] = __offsetOf(value, value.b);
    destination[3] = __offsetOf(value, value.c);
    destination[4] = __offsetOf(value, value.d);
    destination[5] = __sizeOf<int>();
    destination[6] = __alignOf<StructWithArray>();
    destination[7] = __alignOf(value);
    destination[8] = __sizeOf<StructWithArray>();
}
)";
static const char kDirectNVVMNonCanonicalCUDAOffsetSource[] = R"(
struct OffsetStruct : IDefaultInitializable
{
    int value;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    OffsetStruct left;
    OffsetStruct right;
    left.value = 1;
    right.value = 2;
    destination[0] = __offsetOf(left, right.value);
}
)";
static const char kDirectNVVMCUDAExecutionRuntimeSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> counter,
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint3 threadIndex = cudaThreadIdx();
    uint3 blockIndex = cudaBlockIdx();
    uint3 blockDimensions = cudaBlockDim();
    uint3 gridDimensions = cudaGridDim();
    int slot;
    InterlockedAdd(*counter, 1, slot);
    int outputBase = slot * 12;
    GroupMemoryBarrierWithGroupSync();
    destination[outputBase + 0] = threadIndex.x;
    destination[outputBase + 1] = threadIndex.y;
    destination[outputBase + 2] = threadIndex.z;
    destination[outputBase + 3] = blockIndex.x;
    destination[outputBase + 4] = blockIndex.y;
    destination[outputBase + 5] = blockIndex.z;
    destination[outputBase + 6] = blockDimensions.x;
    destination[outputBase + 7] = blockDimensions.y;
    destination[outputBase + 8] = blockDimensions.z;
    destination[outputBase + 9] = gridDimensions.x;
    destination[outputBase + 10] = gridDimensions.y;
    destination[outputBase + 11] = gridDimensions.z;
}
)";
static const char kDirectNVVMConventionalComputeSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    outputBuffer[index] = 42;
}
)";
static const char kDirectNVVMConventionalScalarParameterBlockSource[] = R"(
uniform uint frame;

struct Block
{
    uint dummy;
};

ParameterBlock<Block> block;
RWStructuredBuffer<uint> outputBuffer;

struct TestGlobalParams
{
    uint frame;
    Block* block;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    TestGlobalParams gp = {};
    outputBuffer[0] = __offsetOf(gp, gp.frame);
    outputBuffer[1] = __offsetOf(gp, gp.block);
    outputBuffer[2] = __sizeOf<TestGlobalParams>();
    outputBuffer[3] = __alignOf<Block*>();
    outputBuffer[4] = frame;
    outputBuffer[5] = block.dummy;
}
)";
static const char kDirectNVVMConventionalScalarConstantBufferSource[] = R"(
struct Params
{
    uint value;
    float scale;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = params.value;
}
)";
static const char kDirectNVVMUnsupportedNestedParameterBlockSource[] = R"(
struct Inner
{
    uint value;
};

struct Block
{
    Inner inner;
};

ParameterBlock<Block> block;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = block.inner.value;
}
)";
static const char kDirectNVVMUnsupportedNestedConstantBufferSource[] = R"(
struct Inner
{
    uint value;
};

struct Params
{
    Inner inner;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = params.inner.value;
}
)";
static const char kDirectNVVMConventionalSamplerStorageSource[] = R"(
SamplerComparisonState comparisonSampler;
SamplerComparisonState comparisonSamplers[];
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1.0f;
}
)";
static const char kDirectNVVMUnsupportedFixedSamplerArrayStorageSource[] = R"(
SamplerComparisonState comparisonSamplers[2];
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1.0f;
}
)";
static const char kDirectNVVMMultidimensionalWaveSource[] = R"(
uniform RWStructuredBuffer<float> outputBuffer;

[numthreads(8, 8, 1)]
void computeMain(uint lane : SV_GroupIndex)
{
    uint i = lane * 2;
    outputBuffer[i] = WaveIsFirstLane() ? 1.0 : 0.0;
    outputBuffer[i + 1] = float(WaveGetLaneIndex());
}
)";
static const char kDirectNVVMSharedMemorySource[] = R"(
groupshared int sharedValues[64];

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> counter,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    int ticket;
    InterlockedAdd(*counter, 1, ticket);
    sharedValues[ticket] = ticket * 3 + 1;
    GroupMemoryBarrierWithGroupSync();
    destination[ticket] = sharedValues[63 - ticket];
}
)";
static const char kDirectNVVMUnsignedSharedArrayIndexSource[] = R"(
groupshared int sharedValues[4];

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint writeIndex,
    uniform uint readIndex)
{
    sharedValues[writeIndex] = int(writeIndex) + 1;
    GroupMemoryBarrierWithGroupSync();
    destination[writeIndex] = sharedValues[readIndex];
}
)";
static const char kDirectNVVMUnsupportedSharedFloatArraySource[] = R"(
groupshared float sharedValues[64];

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int index)
{
    sharedValues[index] = 1.0;
    GroupMemoryBarrierWithGroupSync();
    destination[index] = sharedValues[index];
}
)";
static const char kDirectNVVMWaveLaneCountSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    uint laneCount = WaveGetLaneCount();
    destination[laneIndex] = laneCount;
}
)";
static const char kDirectNVVMWaveReadLaneAtUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint mask,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveMaskReadLaneAt(mask, laneIndex, sourceLane);
}
)";
static const char kDirectNVVMWaveReadLaneAtIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform uint mask,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveMaskReadLaneAt(mask, source[laneIndex], sourceLane);
}
)";
static const char kDirectNVVMWaveReadLaneAtFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source,
    uniform uint mask,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveMaskReadLaneAt(mask, source[laneIndex], sourceLane);
}
)";
static const char kDirectNVVMWaveActiveMaskSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveGetActiveMask();
}
)";
static const char kDirectNVVMWaveReadLaneFirstUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(laneIndex);
}
)";
static const char kDirectNVVMWaveReadLaneFirstIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(source[laneIndex]);
}
)";
static const char kDirectNVVMWaveReadLaneFirstFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(source[laneIndex]);
}
)";
static const char kDirectNVVMWaveIsFirstLaneSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveIsFirstLane() ? 1 : 0;
}
)";
static const char kDirectNVVMWaveActiveAnyTrueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAnyTrue(source[laneIndex] != 0) ? 1 : 0;
}
)";
static const char kDirectNVVMWaveActiveAllTrueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllTrue(source[laneIndex] != 0) ? 1 : 0;
}
)";
static const char kDirectNVVMWaveActiveAllEqualIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAllEqualUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<uint, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAllEqualFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
)";
static const char kDirectNVVMUnmaskedWaveReadLaneAtUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(laneIndex, sourceLane);
}
)";
static const char kDirectNVVMUnmaskedWaveReadLaneAtIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(source[laneIndex], sourceLane);
}
)";
static const char kDirectNVVMUnmaskedWaveReadLaneAtFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(source[laneIndex], sourceLane);
}
)";
static const char kDirectNVVMFloat32SubtractSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left - right;
}
)";
static const char kDirectNVVMFloat32MultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left * right;
}
)";
static const char kDirectNVVMFloat32DivideSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left / right;
}
)";
static const char kDirectNVVMFloat32NegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float value)
{
    *destination = -value;
}
)";
static const char kDirectNVVMFloat32CopySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
)";
static const char kDirectNVVMFloat32ConstantSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    *destination = 1.5f;
}
)";
static const char kDirectNVVMFloat32PhiSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int condition,
    uniform float left,
    uniform float right)
{
    float selected;
    if (condition != 0)
        selected = left;
    else
        selected = right;
    *destination = selected;
}
)";
static const char kDirectNVVMFloat32FunctionSource[] = R"(
float addFloat32(float left, float right)
{
    return left + right;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = addFloat32(left, right);
}
)";

enum class NVVMFloat32ArithmeticTestOperation
{
    Add,
    Subtract,
    Multiply,
    Divide,
    Negate,
};

struct NVVMFloat32ArithmeticRuntimeCase
{
    float left;
    float right;
    float expected;
};

struct NVVMFloat32ArithmeticTestCase
{
    NVVMFloat32ArithmeticTestOperation testOperation;
    uint32_t operandCount;
    uint32_t operation;
    const char* source;
    const char* kernelName;
    const char* llvmOpcode;
    const char* diagnosticName;
    const NVVMFloat32ArithmeticRuntimeCase* runtimeCases;
    Index runtimeCaseCount;
};

static const NVVMFloat32ArithmeticRuntimeCase kNVVMFloat32AddRuntimeCases[] = {
    {1.5f, 2.25f, 3.75f},
    {-8.0f, 0.5f, -7.5f},
    {1024.0f, -256.0f, 768.0f},
};

static const NVVMFloat32ArithmeticRuntimeCase kNVVMFloat32SubtractRuntimeCases[] = {
    {8.0f, 0.5f, 7.5f},
    {-8.0f, 0.5f, -8.5f},
    {1024.0f, -256.0f, 1280.0f},
};

static const NVVMFloat32ArithmeticRuntimeCase kNVVMFloat32MultiplyRuntimeCases[] = {
    {1.5f, 2.0f, 3.0f},
    {-8.0f, 0.5f, -4.0f},
    {1024.0f, -0.25f, -256.0f},
};

static const NVVMFloat32ArithmeticRuntimeCase kNVVMFloat32DivideRuntimeCases[] = {
    {8.0f, 2.0f, 4.0f},
    {-8.0f, 0.5f, -16.0f},
    {1024.0f, -256.0f, -4.0f},
};

static const NVVMFloat32ArithmeticRuntimeCase kNVVMFloat32NegateRuntimeCases[] = {
    {1.5f, 0.0f, -1.5f},
    {-8.0f, 0.0f, 8.0f},
    {1024.0f, 0.0f, -1024.0f},
};

static const NVVMFloat32ArithmeticTestCase kNVVMFloat32ArithmeticTestCases[] = {
    {NVVMFloat32ArithmeticTestOperation::Add,
     2,
     SLANG_NVVM_VALUE_OP_ADD,
     kDirectNVVMFloat32AddSource,
     kFloat32AddKernelName,
     "fadd",
     "float32-add",
     kNVVMFloat32AddRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32AddRuntimeCases)},
    {NVVMFloat32ArithmeticTestOperation::Subtract,
     2,
     SLANG_NVVM_VALUE_OP_SUBTRACT,
     kDirectNVVMFloat32SubtractSource,
     kFloat32SubtractKernelName,
     "fsub",
     "float32-subtract",
     kNVVMFloat32SubtractRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32SubtractRuntimeCases)},
    {NVVMFloat32ArithmeticTestOperation::Multiply,
     2,
     SLANG_NVVM_VALUE_OP_MULTIPLY,
     kDirectNVVMFloat32MultiplySource,
     kFloat32MultiplyKernelName,
     "fmul",
     "float32-multiply",
     kNVVMFloat32MultiplyRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32MultiplyRuntimeCases)},
    {NVVMFloat32ArithmeticTestOperation::Divide,
     2,
     SLANG_NVVM_VALUE_OP_DIVIDE,
     kDirectNVVMFloat32DivideSource,
     kFloat32DivideKernelName,
     "fdiv",
     "float32-divide",
     kNVVMFloat32DivideRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32DivideRuntimeCases)},
    {NVVMFloat32ArithmeticTestOperation::Negate,
     1,
     SLANG_NVVM_VALUE_OP_NEGATE,
     kDirectNVVMFloat32NegateSource,
     kFloat32NegateKernelName,
     "fneg",
     "float32-negate",
     kNVVMFloat32NegateRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32NegateRuntimeCases)},
};

static const NVVMFloat32ArithmeticTestCase& _getNVVMFloat32ArithmeticTestCase(
    NVVMFloat32ArithmeticTestOperation operation)
{
    const Index index = Index(operation);
    SLANG_RELEASE_ASSERT(index >= 0 && index < SLANG_COUNT_OF(kNVVMFloat32ArithmeticTestCases));
    const NVVMFloat32ArithmeticTestCase& testCase = kNVVMFloat32ArithmeticTestCases[index];
    SLANG_RELEASE_ASSERT(testCase.testOperation == operation);
    return testCase;
}

static const char kDirectNVVMFloatingEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left == right ? 1 : 0;
}
)";

static const char kDirectNVVMFloatingNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left != right ? 1 : 0;
}
)";

static const char kDirectNVVMFloatingGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left > right ? 1 : 0;
}
)";

static const char kDirectNVVMFloatingLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left <= right ? 1 : 0;
}
)";

static const char kDirectNVVMFloatingGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left >= right ? 1 : 0;
}
)";

static const char kDirectNVVMFloatingLessThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left < right ? 1 : 0;
}
)";

enum class NVVMFloat32ComparisonTestOperation
{
    OrderedEqual,
    UnorderedNotEqual,
    OrderedGreaterThan,
    OrderedLessEqual,
    OrderedGreaterEqual,
    OrderedLessThan,
};

struct NVVMFloat32ComparisonRuntimeCase
{
    float left;
    float right;
    int expected;
};

struct NVVMFloat32ComparisonTestCase
{
    NVVMFloat32ComparisonTestOperation testOperation;
    SlangNVVMValueOperation operation;
    const char* source;
    const char* kernelName;
    const char* llvmOpcode;
    const char* diagnosticName;
    const NVVMFloat32ComparisonRuntimeCase* runtimeCases;
    Index runtimeCaseCount;
};

static const NVVMFloat32ComparisonRuntimeCase kNVVMFloat32OrderedEqualRuntimeCases[] = {
    {3.75f, 3.75f, 1},
    {-8.0f, 0.5f, 0},
    {0.0f, -0.0f, 1},
    {NAN, NAN, 0},
};

static const NVVMFloat32ComparisonRuntimeCase kNVVMFloat32UnorderedNotEqualRuntimeCases[] = {
    {3.75f, 3.75f, 0},
    {-8.0f, 0.5f, 1},
    {0.0f, -0.0f, 0},
    {NAN, NAN, 1},
};

static const NVVMFloat32ComparisonRuntimeCase kNVVMFloat32OrderedGreaterThanRuntimeCases[] = {
    {3.75f, 1.5f, 1},
    {-8.0f, 0.5f, 0},
    {0.0f, -0.0f, 0},
    {NAN, -1.0f, 0},
};

static const NVVMFloat32ComparisonRuntimeCase kNVVMFloat32OrderedLessEqualRuntimeCases[] = {
    {1.5f, 3.75f, 1},
    {0.5f, -8.0f, 0},
    {0.0f, -0.0f, 1},
    {NAN, 1.0f, 0},
};

static const NVVMFloat32ComparisonRuntimeCase kNVVMFloat32OrderedGreaterEqualRuntimeCases[] = {
    {3.75f, 1.5f, 1},
    {-8.0f, 0.5f, 0},
    {0.0f, -0.0f, 1},
    {NAN, -1.0f, 0},
};

static const NVVMFloat32ComparisonRuntimeCase kNVVMFloat32OrderedLessThanRuntimeCases[] = {
    {1.5f, 3.75f, 1},
    {0.5f, -8.0f, 0},
    {0.0f, -0.0f, 0},
    {NAN, 1.0f, 0},
};

static const NVVMFloat32ComparisonTestCase kNVVMFloat32ComparisonTestCases[] = {
    {NVVMFloat32ComparisonTestOperation::OrderedEqual,
     SLANG_NVVM_VALUE_OP_EQUAL,
     kDirectNVVMFloatingEqualSource,
     kFloat32EqualKernelName,
     "fcmp oeq",
     "float32-ordered-equal",
     kNVVMFloat32OrderedEqualRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32OrderedEqualRuntimeCases)},
    {NVVMFloat32ComparisonTestOperation::UnorderedNotEqual,
     SLANG_NVVM_VALUE_OP_NOT_EQUAL,
     kDirectNVVMFloatingNotEqualSource,
     kFloat32NotEqualKernelName,
     "fcmp une",
     "float32-unordered-not-equal",
     kNVVMFloat32UnorderedNotEqualRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32UnorderedNotEqualRuntimeCases)},
    {NVVMFloat32ComparisonTestOperation::OrderedGreaterThan,
     SLANG_NVVM_VALUE_OP_GREATER_THAN,
     kDirectNVVMFloatingGreaterThanSource,
     kFloat32GreaterThanKernelName,
     "fcmp ogt",
     "float32-ordered-greater-than",
     kNVVMFloat32OrderedGreaterThanRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32OrderedGreaterThanRuntimeCases)},
    {NVVMFloat32ComparisonTestOperation::OrderedLessEqual,
     SLANG_NVVM_VALUE_OP_LESS_EQUAL,
     kDirectNVVMFloatingLessEqualSource,
     kFloat32LessEqualKernelName,
     "fcmp ole",
     "float32-ordered-less-equal",
     kNVVMFloat32OrderedLessEqualRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32OrderedLessEqualRuntimeCases)},
    {NVVMFloat32ComparisonTestOperation::OrderedGreaterEqual,
     SLANG_NVVM_VALUE_OP_GREATER_EQUAL,
     kDirectNVVMFloatingGreaterEqualSource,
     kFloat32GreaterEqualKernelName,
     "fcmp oge",
     "float32-ordered-greater-equal",
     kNVVMFloat32OrderedGreaterEqualRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32OrderedGreaterEqualRuntimeCases)},
    {NVVMFloat32ComparisonTestOperation::OrderedLessThan,
     SLANG_NVVM_VALUE_OP_LESS_THAN,
     kDirectNVVMFloatingLessThanSource,
     kFloat32LessThanKernelName,
     "fcmp olt",
     "float32-ordered-less-than",
     kNVVMFloat32OrderedLessThanRuntimeCases,
     SLANG_COUNT_OF(kNVVMFloat32OrderedLessThanRuntimeCases)},
};

static const NVVMFloat32ComparisonTestCase& _getNVVMFloat32ComparisonTestCase(
    NVVMFloat32ComparisonTestOperation operation)
{
    const Index index = Index(operation);
    SLANG_RELEASE_ASSERT(index >= 0 && index < SLANG_COUNT_OF(kNVVMFloat32ComparisonTestCases));
    const NVVMFloat32ComparisonTestCase& testCase = kNVVMFloat32ComparisonTestCases[index];
    SLANG_RELEASE_ASSERT(testCase.testOperation == operation);
    return testCase;
}

static const char kDirectNVVMUnsupportedHalfAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<half, Access::ReadWrite, AddressSpace::Device> destination,
    uniform half left,
    uniform half right)
{
    *destination = left + right;
}
)";
static const char kDirectNVVMUnsupportedDoubleAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<double, Access::ReadWrite, AddressSpace::Device> destination,
    uniform double left,
    uniform double right)
{
    *destination = left + right;
}
)";
static const char kDirectNVVMCopyScalarSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
)";
static const char kDirectNVVMChooseScalarSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    if (x < y)
        *destination = x + y;
    else
        *destination = x - y;
}

)";
static const char kDirectNVVMIntegerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left == right ? 1 : 0;
}
)";
static const char kDirectNVVMUnsignedIntegerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left == right ? 1 : 0;
}
)";
static const char kDirectNVVMWideIntegerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left == right ? 1 : 0;
}
)";
static const char kDirectNVVMPointerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left == right ? 1 : 0;
}
)";
static const char kDirectNVVMIntegerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left != right ? 1 : 0;
}
)";
static const char kDirectNVVMUnsignedIntegerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left != right ? 1 : 0;
}
)";
static const char kDirectNVVMWideIntegerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left != right ? 1 : 0;
}
)";
static const char kDirectNVVMPointerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left != right ? 1 : 0;
}
)";
static const char kDirectNVVMIntegerSignedGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left > right ? 1 : 0;
}
)";
static const char kDirectNVVMUnsignedIntegerGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left > right ? 1 : 0;
}
)";
static const char kDirectNVVMWideIntegerGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left > right ? 1 : 0;
}
)";
static const char kDirectNVVMPointerGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left > right ? 1 : 0;
}
)";
static const char kDirectNVVMIntegerSignedLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left <= right ? 1 : 0;
}
)";
static const char kDirectNVVMUnsignedIntegerLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left <= right ? 1 : 0;
}
)";
static const char kDirectNVVMWideIntegerLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left <= right ? 1 : 0;
}
)";
static const char kDirectNVVMPointerLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left <= right ? 1 : 0;
}
)";
static const char kDirectNVVMIntegerSignedGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int left,
    uniform int right)
{
    *destination = left >= right ? 1 : 0;
}
)";
static const char kDirectNVVMUnsignedIntegerGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left >= right ? 1 : 0;
}
)";
static const char kDirectNVVMWideIntegerGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left >= right ? 1 : 0;
}
)";
static const char kDirectNVVMPointerGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left >= right ? 1 : 0;
}
)";
static const char kDirectNVVMSelectedKernelSource[] = R"(
[CUDAKernel]
void unselectedKernel()
{
    GroupMemoryBarrierWithGroupSync();
}

[CUDAKernel]
void computeMain()
{}
)";
static const char kDirectNVVMConventionalParameterizedComputeSource[] = R"(
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform int value)
{}
)";
static const char kDirectNVVMUnsupportedCallSource[] =
    "[shader(\"compute\")] [numthreads(1, 1, 1)] void computeMain() { "
    "GroupMemoryBarrierWithGroupSync(); }";
static const char kDirectNVVMIntegerConstantSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value + 1;
}
)";
static const char kDirectNVVMMixedNumericSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int8_t, Access::ReadWrite, AddressSpace::Device> output8,
    uniform Ptr<uint16_t, Access::ReadWrite, AddressSpace::Device> output16,
    uniform Ptr<int64_t, Access::ReadWrite, AddressSpace::Device> output64,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> output32,
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> outputFloat,
    uniform Ptr<int2, Access::ReadWrite, AddressSpace::Device> outputVector,
    uniform Ptr<int2, Access::Read, AddressSpace::Device> leftVector,
    uniform Ptr<int2, Access::Read, AddressSpace::Device> rightVector,
    uniform int8_t a,
    uniform uint8_t b,
    uniform int16_t c,
    uniform uint16_t d,
    uniform int64_t e,
    uniform uint64_t f,
    uniform float g)
{
    int index = int(cudaThreadIdx().x);
    output8[index] = ~(a + int8_t(b));
    output16[index] = (uint16_t(c) + d) ^ uint16_t(0x55aa);
    output64[index] = (int64_t(f) + e) * int64_t(3);
    int converted = int(g) + int(b);
    if (a < int8_t(b))
        converted += 1000;
    if (d > uint16_t(c))
        converted += 2000;
    output32[index] = converted;
    outputFloat[index] = float(c) + g;
    outputVector[index] = leftVector[index] + rightVector[index];
}
)";
static const char kDirectNVVMMergePhiSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    int selected;
    if (x < y)
        selected = x;
    else
        selected = y;
    *destination = selected;
}
)";
static const char kDirectNVVMFiniteLoopSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int limit)
{
    int sum = 0;
    for (int i = 0; i < limit; ++i)
        sum += i;
    *destination = sum;
}
)";
static const char kDirectNVVMIntegerMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x * y;
}
)";
static const char kDirectNVVMUnsignedMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x * y);
}
)";
static const char kDirectNVVMWideIntegerMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x * y);
}
)";
static const char kDirectNVVMFloatingMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x,
    uniform float y)
{
    *destination = int(x * y);
}
)";
static const char kDirectNVVMFloatingSineSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x)
{
    *destination = int(sin(x));
}
)";
static const char kDirectNVVMIntegerBitAndSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x & y;
}
)";
static const char kDirectNVVMIntegerBitOrSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x | y;
}
)";
static const char kDirectNVVMIntegerBitXorSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x ^ y;
}
)";
static const char kDirectNVVMIntegerBitNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    *destination = ~x;
}
)";
static const char kDirectNVVMLogicalNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform bool x)
{
    *destination = !x ? 1 : 0;
}
)";
static const char kDirectNVVMUnsignedIntegerBitNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x)
{
    *destination = int(~x);
}
)";
static const char kDirectNVVMWideIntegerBitNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x)
{
    *destination = int(~x);
}
)";
static const char kDirectNVVMIntegerNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    *destination = -x;
}
)";
static const char kDirectNVVMUnsignedIntegerNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x)
{
    *destination = int(-x);
}
)";
static const char kDirectNVVMWideIntegerNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x)
{
    *destination = int(-x);
}
)";
static const char kDirectNVVMFloatingNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x)
{
    *destination = int(-x);
}
)";
static const char kDirectNVVMRelaxedGlobalI32AtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1);
}
)";
static const char kDirectNVVMRelaxedGlobalI32AtomicAddOldValueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> oldValueDestination)
{
    int oldValue;
    InterlockedAdd(*destination, 1, oldValue);
    *oldValueDestination = oldValue;
}
)";
static const char kDirectNVVMUnsignedAtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1u);
}
)";
static const char kDirectNVVMWideAtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int64_t, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, int64_t(1));
}
)";
static const char kDirectNVVMFloatingAtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1.0f);
}
)";
static const char kDirectNVVMAtomicSubSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    __atomic_sub(*destination, 1);
}
)";
static const char kDirectNVVMAtomicExchangeSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedExchange(*destination, 1);
}
)";
static const char kDirectNVVMAcquireGlobalI32AtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    __atomic_add(*destination, 1, MemoryOrder::Acquire);
}
)";
static const char kDirectNVVMGroupSharedI32AtomicAddSource[] = R"(
groupshared int atomicCounter;

[CUDAKernel]
void computeMain()
{
    InterlockedAdd(atomicCounter, 1);
}
)";
static const char kDirectNVVMIntegerLeftShiftSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int amount)
{
    *destination = x << amount;
}
)";
static const char kDirectNVVMIntegerRightShiftSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int amount)
{
    *destination = x >> amount;
}
)";
static const char kDirectNVVMIntegerDivideSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x / y;
}
)";
static const char kDirectNVVMIntegerRemainderSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x % y;
}
)";
static const char kDirectNVVMUnsignedIntegerBitAndSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x & y);
}
)";
static const char kDirectNVVMWideIntegerBitAndSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x & y);
}
)";
static const char kDirectNVVMUnsignedIntegerBitOrSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x | y);
}
)";
static const char kDirectNVVMWideIntegerBitOrSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x | y);
}
)";
static const char kDirectNVVMUnsignedIntegerBitXorSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x ^ y);
}
)";
static const char kDirectNVVMWideIntegerBitXorSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x ^ y);
}
)";
static const char kDirectNVVMPointerOffsetSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int index)
{
    *(destination + index) = *(source + index);
}
)";
static const char kDirectNVVMUnsignedPointerOffsetSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *(destination + uint(1)) = *(source + uint(1));
}
)";
static const char kDirectNVVMFixedDeviceArraySource[] = R"(
typealias RWIntArray4 = Ptr<int[4], Access::ReadWrite, AddressSpace::Device>;
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWIntArray4 destination,
    uniform RIntArray4 source,
    uniform int index)
{
    (*destination)[index] = (*source)[index];
}
)";
static const char kDirectNVVMRawRWStructuredBufferI32StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<int> destination, uniform int index)
{
    destination[index] = 42;
}
)";
static const char kDirectNVVMRawRWStructuredBufferU32StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<uint> destination, uniform int index)
{
    destination[index] = 42;
}
)";
static const char kDirectNVVMRawRWStructuredBufferF32StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<float> destination, uniform int index)
{
    destination[index] = 42.0;
}
)";
static const char kDirectNVVMRawRWStructuredBufferF64StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<double> destination, uniform int index)
{
    destination[index] = 42.0;
}
)";
static const char kDirectNVVMRawBufferDataPointerSource[] = R"(
[CUDAKernel]
void computeMain(
    RWStructuredBuffer<int> structuredSource,
    RWByteAddressBuffer byteSource,
    RWStructuredBuffer<int> destination,
    uniform uint index)
{
    let structuredPointer = __getStructuredBufferPtr(structuredSource);
    let bytePointer = __getByteAddressBufferPtr(byteSource);
    destination[index] = (*structuredPointer)[index] + int((*bytePointer)[index]);
}
)";
static const char kDirectNVVMReadOnlyByteAddressDataPointerSource[] = R"(
[CUDAKernel]
void computeMain(
    ByteAddressBuffer source,
    RWStructuredBuffer<uint> destination,
    uniform uint index)
{
    let sourcePointer = __getByteAddressBufferPtr(source);
    destination[index] = (*sourcePointer)[index];
}
)";
static const char kDirectNVVMReadOnlyByteAddressStoreSource[] = R"(
[CUDAKernel]
void computeMain(ByteAddressBuffer source, uniform uint index)
{
    let sourcePointer = __getByteAddressBufferPtr(source);
    (*sourcePointer)[index] = 42;
}
)";
static const char kDirectNVVMCoreByteAddressAccessSource[] = R"(
[CUDAKernel]
void computeMain(
    ByteAddressBuffer source,
    RWByteAddressBuffer destination,
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output,
    uniform uint offset)
{
    uint4 sourceValues = source.Load4Aligned(offset, 16);
    uint destinationValue = destination.Load(offset + 4);
    destination.Store(offset, sourceValues.x + destinationValue);
    output[0] = sourceValues.y;
}
)";
static const char kDirectNVVMFloatVectorByteAddressAccessSource[] = R"(
[CUDAKernel]
void computeMain(
    RWByteAddressBuffer source,
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    float4 wide = source.LoadAligned<float4>(0, 16);
    float4 scalarized = source.LoadAligned<float4>(16, 4);
    source.StoreAligned(32, scalarized);
    source.Store<float4>(48, wide, 4);
    int signedValue = source.Load<int>(64);
    source.Store<int>(68, signedValue);
    destination[0] = wide.x;
}
)";
static const char kDirectNVVMWideIntegerByteAddressAccessSource[] = R"(
[CUDAKernel]
void computeMain(
    ByteAddressBuffer readOnlySource,
    RWByteAddressBuffer readWriteSource,
    uniform Ptr<uint64_t, Access::ReadWrite, AddressSpace::Device> destination)
{
    int64_t signedValue = readOnlySource.LoadAligned<int64_t>(0, 8);
    uint64_t unsignedValue = readWriteSource.Load<uint64_t>(8);
    readWriteSource.Store<int64_t>(16, signedValue, 8);
    readWriteSource.Store<uint64_t>(24, unsignedValue);
    destination[0] = unsignedValue;
}
)";
static const char kDirectNVVMNumericArrayByteAddressAccessSource[] = R"(
struct Block
{
    float4 values[2];
};

[CUDAKernel]
void computeMain(ByteAddressBuffer source, RWByteAddressBuffer destination)
{
    destination.Store(0, source.LoadAligned<Block>(0));
}
)";
static const char kDirectNVVMUnsupportedNestedArrayByteAddressAccessSource[] = R"(
struct NestedBlock
{
    float4 values[2][2];
};

[CUDAKernel]
void computeMain(RWByteAddressBuffer source)
{
    source.Store(0, source.LoadAligned<NestedBlock>(0));
}
)";
static const char kDirectNVVMAggregateAndReadOnlyResourceSource[] = R"(
struct Padding
{
    uint64_t big;
    uint16_t little;
};

[CUDAKernel]
void computeMain(
    uniform Padding padding,
    RWStructuredBuffer<float> destination,
    StructuredBuffer<float> source,
    uniform uint index)
{
    destination[index] = source[index] + float(padding.big) + float(padding.little);
}
)";
static const char kDirectNVVMVectorStructuredBufferSource[] = R"(
[CUDAKernel]
void computeMain(
    StructuredBuffer<int4> source,
    RWStructuredBuffer<float4> destination,
    RWStructuredBuffer<int> output)
{
    int4 loaded = source[0];
    destination[0].wzyx = float4(1.0, 2.0, 3.0, 4.0);
    float4 stored = destination[0];
    output[0] = loaded.x + int(stored.w);
}
)";
static const char kDirectNVVMUnsupportedDoubleVectorStructuredBufferSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<double2> destination)
{
    destination[0] = double2(1.0, 2.0);
}
)";
static const char kDirectNVVMUnsupportedNestedAggregateParameterSource[] = R"(
struct Inner
{
    uint value;
};

struct Outer
{
    Inner inner;
};

[CUDAKernel]
void computeMain(uniform Outer value)
{
}
)";
static const char kDirectNVVMRawRWStructuredBufferI32AtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<int> destination, uniform int index)
{
    InterlockedAdd(destination[index], 1);
}
)";
static const char kDirectNVVMUnsignedFixedArrayIndexSource[] = R"(
typealias RWIntArray4 = Ptr<int[4], Access::ReadWrite, AddressSpace::Device>;
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(uniform RWIntArray4 destination, uniform RIntArray4 source)
{
    (*destination)[uint(1)] = (*source)[uint(1)];
}
)";
static const char kDirectNVVMUnsupportedFloatArraySource[] = R"(
typealias RWFloatArray4 = Ptr<float[4], Access::ReadWrite, AddressSpace::Device>;
typealias RFloatArray4 = Ptr<float[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWFloatArray4 destination,
    uniform RFloatArray4 source,
    uniform int index)
{
    (*destination)[index] = (*source)[index];
}
)";
static const char kDirectNVVMUnsupportedNestedArraySource[] = R"(
typealias RWNestedArray = Ptr<int[2][2], Access::ReadWrite, AddressSpace::Device>;
typealias RNestedArray = Ptr<int[2][2], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWNestedArray destination,
    uniform RNestedArray source,
    uniform int index)
{
    (*destination)[index][0] = (*source)[index][0];
}
)";
static const char kDirectNVVMUnsupportedNestedLocalArraySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    int values[2][2];
    values[0][0] = x;
    *destination = values[0][0];
}
)";
static const char kDirectNVVMUnsupportedStructPointerSource[] = R"(
struct Pair
{
    int x;
    int y;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<Pair, Access::Read, AddressSpace::Device> source)
{
    *destination = (*source).x;
}
)";
static const char kDirectNVVMUnsupportedArrayPointerHelperSource[] = R"(
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

int readArrayElement(RIntArray4 source, int index)
{
    return (*source)[index];
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform RIntArray4 source,
    uniform int index)
{
    *destination = readArrayElement(source, index);
}
)";
static const char kDirectNVVMUnsupportedNestedStructHelperSource[] = R"(
struct Inner
{
    int value;
};

struct Outer
{
    Inner inner;
};

int readValue(Outer value)
{
    return value.inner.value;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int input)
{
    Outer value;
    value.inner.value = input;
    *destination = readValue(value);
}
)";
static const char kDirectNVVMUnsupportedPointerHelperParameterSource[] = R"(
int readValue(Ptr<int, Access::Read, AddressSpace::Device> source)
{
    return *source;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = readValue(source);
}
)";
static const char kDirectNVVMUnsupportedPointerHelperResultSource[] = R"(
Ptr<int, Access::Read, AddressSpace::Device> identity(
    Ptr<int, Access::Read, AddressSpace::Device> source)
{
    return source;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = *identity(source);
}
)";
static const char kDirectNVVMScalarFunctionSource[] = R"(
int increment(int value)
{
    return value + 1;
}

int incrementTwice(int value)
{
    return increment(increment(value));
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = increment(value) + incrementTwice(value);
}
)";
static const char kDirectNVVMVectorFunctionSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[noinline]
int4 chooseInt4(bool condition, int4 left, int4 right)
{
    return condition ? left : right;
}

[noinline]
float3 identityFloat3(float3 value)
{
    return value;
}

[noinline]
bool2 identityBool2(bool2 value)
{
    return value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int4 selected = chooseInt4(true, int4(1, 2, 3, 4), int4(5, 6, 7, 8));
    float3 floats = identityFloat3(float3(9.0, 10.0, 11.0));
    bool2 booleans = identityBool2(int2(1, 2) == int2(1, 3));
    outputBuffer[0] = selected.x + int(floats.y) + (booleans.x ? 1 : 0);
}
)";
static const char kDirectNVVMUnsupportedVectorFunctionSources[][512] = {
    R"(
RWStructuredBuffer<int> outputBuffer;
double2 identity(double2 value) { return value; }
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() { outputBuffer[0] = int(identity(double2(1.0, 2.0)).x); }
)",
    R"(
RWStructuredBuffer<int> outputBuffer;
vector<int, 5> identity(vector<int, 5> value) { return value; }
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() { outputBuffer[0] = identity(vector<int, 5>(1)).x; }
)",
};
static const char kDirectNVVMFunctionContractSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[noinline]
int helperFunc(int value)
{
    return value + 1;
}

int plainHelper(int value)
{
    return value * 2;
}

[CudaDeviceExport]
[noinline]
int exportedFunc(int value)
{
    return value + 3;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = helperFunc(42) + plainHelper(7) + exportedFunc(1);
}
)";
static const char kDirectNVVMPrunesUnreachableHelperSource[] = R"(
int unusedMultiply(int x, int y)
{
    return x * y;
}

int increment(int value)
{
    return value + 1;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = increment(value);
}
)";

enum class ScalarRuntimeOperation
{
    Write,
    Copy,
    Choose,
    Multiply,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    Negate,
    Equal,
    NotEqual,
    GreaterThan,
    LessEqual,
    GreaterEqual,
};

enum class NVVMScalarTestOperation
{
    Multiply,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    Negate,
    Equal,
    NotEqual,
    SignedGreaterThan,
    SignedLessEqual,
    SignedGreaterEqual,
};

enum class NVVMScalarPTXEvidence
{
    Multiply,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    Negate,
    EqualityComparison,
    SignedComparison,
};

struct NVVMScalarTestCase
{
    NVVMScalarTestOperation testOperation;
    FakeNVVMBuilderScalarOperationKey key;
    const char* source;
    const char* kernelName;
    const char* llvmOpcode;
    ScalarRuntimeOperation runtimeOperation;
    NVVMScalarPTXEvidence ptxEvidence;
    const char* diagnosticName;
};

static const NVVMScalarTestCase kNVVMUnaryScalarTestCases[] = {
    {NVVMScalarTestOperation::BitNot,
     {FakeNVVMBuilderScalarFamily::Unary, SLANG_NVVM_VALUE_OP_BIT_NOT},
     kDirectNVVMIntegerBitNotSource,
     kBitNotScalarKernelName,
     "xor",
     ScalarRuntimeOperation::BitNot,
     NVVMScalarPTXEvidence::BitNot,
     "integer-bit-NOT"},
    {NVVMScalarTestOperation::Negate,
     {FakeNVVMBuilderScalarFamily::Unary, SLANG_NVVM_VALUE_OP_NEGATE},
     kDirectNVVMIntegerNegateSource,
     kNegateScalarKernelName,
     "sub",
     ScalarRuntimeOperation::Negate,
     NVVMScalarPTXEvidence::Negate,
     "integer-negate"},
};

static const NVVMScalarTestCase kNVVMBinaryScalarTestCases[] = {
    {NVVMScalarTestOperation::Multiply,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_VALUE_OP_MULTIPLY},
     kDirectNVVMIntegerMultiplySource,
     kMultiplyScalarKernelName,
     "mul",
     ScalarRuntimeOperation::Multiply,
     NVVMScalarPTXEvidence::Multiply,
     "integer-multiply"},
    {NVVMScalarTestOperation::BitAnd,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_VALUE_OP_BIT_AND},
     kDirectNVVMIntegerBitAndSource,
     kBitAndScalarKernelName,
     "and",
     ScalarRuntimeOperation::BitAnd,
     NVVMScalarPTXEvidence::BitAnd,
     "integer-bit-AND"},
    {NVVMScalarTestOperation::BitOr,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_VALUE_OP_BIT_OR},
     kDirectNVVMIntegerBitOrSource,
     kBitOrScalarKernelName,
     "or",
     ScalarRuntimeOperation::BitOr,
     NVVMScalarPTXEvidence::BitOr,
     "integer-bit-OR"},
    {NVVMScalarTestOperation::BitXor,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_VALUE_OP_BIT_XOR},
     kDirectNVVMIntegerBitXorSource,
     kBitXorScalarKernelName,
     "xor",
     ScalarRuntimeOperation::BitXor,
     NVVMScalarPTXEvidence::BitXor,
     "integer-bit-XOR"},
};

static const NVVMScalarTestCase kNVVMCompareScalarTestCases[] = {
    {NVVMScalarTestOperation::Equal,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_VALUE_OP_EQUAL},
     kDirectNVVMIntegerEqualSource,
     kEqualScalarKernelName,
     "icmp eq",
     ScalarRuntimeOperation::Equal,
     NVVMScalarPTXEvidence::EqualityComparison,
     "integer-equality"},
    {NVVMScalarTestOperation::NotEqual,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_VALUE_OP_NOT_EQUAL},
     kDirectNVVMIntegerNotEqualSource,
     kNotEqualScalarKernelName,
     "icmp ne",
     ScalarRuntimeOperation::NotEqual,
     NVVMScalarPTXEvidence::EqualityComparison,
     "integer-inequality"},
    {NVVMScalarTestOperation::SignedGreaterThan,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_VALUE_OP_GREATER_THAN},
     kDirectNVVMIntegerSignedGreaterThanSource,
     kGreaterThanScalarKernelName,
     "icmp sgt",
     ScalarRuntimeOperation::GreaterThan,
     NVVMScalarPTXEvidence::SignedComparison,
     "integer-signed-greater-than"},
    {NVVMScalarTestOperation::SignedLessEqual,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_VALUE_OP_LESS_EQUAL},
     kDirectNVVMIntegerSignedLessEqualSource,
     kLessEqualScalarKernelName,
     "icmp sle",
     ScalarRuntimeOperation::LessEqual,
     NVVMScalarPTXEvidence::SignedComparison,
     "integer-signed-less-equal"},
    {NVVMScalarTestOperation::SignedGreaterEqual,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_VALUE_OP_GREATER_EQUAL},
     kDirectNVVMIntegerSignedGreaterEqualSource,
     kGreaterEqualScalarKernelName,
     "icmp sge",
     ScalarRuntimeOperation::GreaterEqual,
     NVVMScalarPTXEvidence::SignedComparison,
     "integer-signed-greater-equal"},
};

static const NVVMScalarTestCase& _getNVVMScalarTestCase(NVVMScalarTestOperation operation)
{
    const NVVMScalarTestCase* const families[] = {
        kNVVMUnaryScalarTestCases,
        kNVVMBinaryScalarTestCases,
        kNVVMCompareScalarTestCases,
    };
    const Index familyCounts[] = {
        SLANG_COUNT_OF(kNVVMUnaryScalarTestCases),
        SLANG_COUNT_OF(kNVVMBinaryScalarTestCases),
        SLANG_COUNT_OF(kNVVMCompareScalarTestCases),
    };
    for (Index family = 0; family < SLANG_COUNT_OF(families); ++family)
    {
        for (Index i = 0; i < familyCounts[family]; ++i)
        {
            if (families[family][i].testOperation == operation)
                return families[family][i];
        }
    }
    SLANG_UNEXPECTED("unknown NVVM scalar test operation");
}

static SlangResult _emitNVVMScalarTestOperation(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMScalarTestCase& testCase,
    SlangNVVMValueHandle left,
    SlangNVVMValueHandle right,
    SlangNVVMValueHandle& outValue)
{
    switch (testCase.key.family)
    {
    case FakeNVVMBuilderScalarFamily::Unary:
        return builder.emitIntegerUnary(
            module,
            SlangNVVMValueOperation(testCase.key.operation),
            left,
            outValue);
    case FakeNVVMBuilderScalarFamily::Binary:
        return builder.emitIntegerBinaryOperation(
            module,
            SlangNVVMValueOperation(testCase.key.operation),
            left,
            right,
            outValue);
    case FakeNVVMBuilderScalarFamily::Compare:
        return builder.emitIntegerCompare(
            module,
            SlangNVVMValueOperation(testCase.key.operation),
            left,
            right,
            outValue);
    }
    return SLANG_E_INVALID_ARG;
}

// Materializes the shared comparison consumer: branch on i1, store one or zero, then merge.
static SlangResult _emitNVVMBooleanResultAsI32(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    SlangNVVMValueHandle function,
    SlangNVVMValueHandle destination,
    SlangNVVMTypeHandle integerType,
    SlangNVVMValueHandle condition)
{
    SlangNVVMBlockHandle trueBlock = nullptr;
    SlangNVVMBlockHandle falseBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("true"), trueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("false"), falseBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("merge"), mergeBlock));

    SlangNVVMValueHandle zero = nullptr;
    SlangNVVMValueHandle one = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 0, zero));
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 1, one));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, condition, trueBlock, falseBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, trueBlock));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, one, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, falseBlock));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, zero, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, mergeBlock));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _populateNVVMScalarTestKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMScalarTestCase& testCase)
{
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(
        builder.getPointerType(module, integerType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, pointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, integerType, integerType};
    const size_t parameterCount = isUnary ? 2 : 3;
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, voidType, parameterTypes, parameterCount, functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        UnownedStringSlice(testCase.kernelName),
        function));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle left = nullptr;
    SlangNVVMValueHandle right = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, left));
    if (!isUnary)
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, right));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle result = nullptr;
    SLANG_RETURN_ON_FAIL(
        _emitNVVMScalarTestOperation(builder, module, testCase, left, right, result));
    if (!isCompare)
    {
        SLANG_RETURN_ON_FAIL(builder.emitStore(module, result, destination, 4));
        SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
        SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
        return SLANG_OK;
    }

    return _emitNVVMBooleanResultAsI32(builder, module, function, destination, integerType, result);
}

static SlangResult _populateFloat32ComparisonKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module,
    const NVVMFloat32ComparisonTestCase& testCase)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle floatType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(
        builder.getPointerType(module, integerType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, pointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, floatType, floatType};
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        UnownedStringSlice(testCase.kernelName),
        function));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle left = nullptr;
    SlangNVVMValueHandle right = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, left));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, right));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle result = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitFloatingCompare(module, testCase.operation, left, right, result));
    return _emitNVVMBooleanResultAsI32(builder, module, function, destination, integerType, result);
}

static SlangResult _buildNVVMScalarTestModule(
    const NVVMIRBuilder& builder,
    NVVMScalarTestOperation operation,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-scalar-test"), scope.module));
    SLANG_RETURN_ON_FAIL(
        _populateNVVMScalarTestKernel(builder, scope.module, _getNVVMScalarTestCase(operation)));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static void _resetDirectNVVMFakes()
{
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.foundation = _makeFakeNVVMBuilderFoundationAPI();
    gFakeNVVMBuilder.construction = _makeFakeNVVMBuilderConstructionAPI();
    gFakeNVVMBuilder.valueOperations = _makeFakeNVVMBuilderValueOperationsAPI();
    gFakeNVVMBuilder.surfaceOperationsAPI = _makeFakeNVVMBuilderSurfaceOperationsAPI();
    gFakeNVVMBuilder.textureOperationsAPI = _makeFakeNVVMBuilderTextureOperationsAPI();
    gFakeNVVM.reset();
    gFakeNVVM.compiledPTX = kFakeDirectPTX;
}

static SlangResult _createSlangPTXLinkedProgram(
    slang::IGlobalSession* globalSession,
    const char* source,
    SlangEmitCUDAMethod emissionMethod,
    ComPtr<slang::ISession>& outSession,
    ComPtr<slang::IComponentType>& outProgram,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    outSession.setNull();
    outProgram.setNull();
    outDiagnostics.setNull();
    if (!globalSession || !source)
        return SLANG_E_INVALID_ARG;

    const SlangCapabilityID cudaSM70 = globalSession->findCapability("cuda_sm_7_0");
    if (cudaSM70 == SLANG_CAPABILITY_UNKNOWN)
        return SLANG_E_NOT_FOUND;

    slang::CompilerOptionEntry targetOptions[2] = {};
    targetOptions[0].name = slang::CompilerOptionName::EmitCUDAMethod;
    targetOptions[0].value.kind = slang::CompilerOptionValueKind::Int;
    targetOptions[0].value.intValue0 = emissionMethod;
    targetOptions[1].name = slang::CompilerOptionName::Capability;
    targetOptions[1].value.kind = slang::CompilerOptionValueKind::Int;
    targetOptions[1].value.intValue0 = int32_t(cudaSM70);

    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_PTX;
    targetDesc.compilerOptionEntryCount = SLANG_COUNT_OF(targetOptions);
    targetDesc.compilerOptionEntries = targetOptions;

    slang::SessionDesc sessionDesc = {};
    sessionDesc.targetCount = 1;
    sessionDesc.targets = &targetDesc;

    SLANG_RETURN_ON_FAIL(globalSession->createSession(sessionDesc, outSession.writeRef()));

    ComPtr<slang::IModule> module(outSession->loadModuleFromSourceString(
        "directNVVM",
        "direct-nvvm.slang",
        source,
        outDiagnostics.writeRef()));
    if (!module)
        return SLANG_FAIL;

    ComPtr<slang::IEntryPoint> entryPoint;
    SLANG_RETURN_ON_FAIL(module->findAndCheckEntryPoint(
        "computeMain",
        SLANG_STAGE_COMPUTE,
        entryPoint.writeRef(),
        outDiagnostics.writeRef()));

    slang::IComponentType* components[] = {module.get(), entryPoint.get()};
    ComPtr<slang::IComponentType> program;
    SLANG_RETURN_ON_FAIL(outSession->createCompositeComponentType(
        components,
        SLANG_COUNT_OF(components),
        program.writeRef(),
        outDiagnostics.writeRef()));

    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(program->link(linkedProgram.writeRef(), outDiagnostics.writeRef()));
    outProgram = linkedProgram;
    return SLANG_OK;
}

static SlangResult _populateScalarConditionalKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalIntegerPointerType,
        integerType,
        integerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle x = nullptr;
    SlangNVVMValueHandle y = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kChooseScalarKernelName),
        function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, x));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, y));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle trueBlock = nullptr;
    SlangNVVMBlockHandle falseBlock = nullptr;
    SlangNVVMBlockHandle mergeBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("if.true"), trueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("if.false"), falseBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("if.merge"), mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerSignedLessThan(module, x, y, condition));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, condition, trueBlock, falseBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, trueBlock));
    SlangNVVMValueHandle sum = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerBinary(module, SLANG_NVVM_VALUE_OP_ADD, x, y, sum));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, sum, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, falseBlock));
    SlangNVVMValueHandle difference = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_VALUE_OP_SUBTRACT, x, y, difference));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, difference, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, mergeBlock));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildScalarConditionalModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(
        builder.createModule(toSlice("slang-nvvm-scalar-conditional"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateScalarConditionalKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _populateScalarSSALoopKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle limit = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kSumToLimitKernelName),
        function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, limit));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SlangNVVMBlockHandle headerBlock = nullptr;
    SlangNVVMBlockHandle bodyBlock = nullptr;
    SlangNVVMBlockHandle continueBlock = nullptr;
    SlangNVVMBlockHandle exitBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, function, toSlice("loop.header"), headerBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("loop.body"), bodyBlock));
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, function, toSlice("loop.continue"), continueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("loop.exit"), exitBlock));

    SlangNVVMValueHandle zero = nullptr;
    SlangNVVMValueHandle one = nullptr;
    SlangNVVMValueHandle i = nullptr;
    SlangNVVMValueHandle sum = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 0, zero));
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 1, one));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerPhi(module, headerBlock, integerType, i));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerPhi(module, headerBlock, integerType, sum));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, headerBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, headerBlock));
    SlangNVVMValueHandle condition = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerSignedLessThan(module, i, limit, condition));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, condition, bodyBlock, exitBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, bodyBlock));
    SlangNVVMValueHandle nextSum = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_VALUE_OP_ADD, sum, i, nextSum));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, continueBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, continueBlock));
    SlangNVVMValueHandle nextI = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerBinary(module, SLANG_NVVM_VALUE_OP_ADD, i, one, nextI));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, headerBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, exitBlock));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, sum, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));

    // Add incoming edges only after the complete CFG exists, so the provider can validate both
    // predecessor membership and value availability at each predecessor terminator.
    SLANG_RETURN_ON_FAIL(builder.addIntegerPhiIncoming(module, i, zero, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.addIntegerPhiIncoming(module, i, nextI, continueBlock));
    SLANG_RETURN_ON_FAIL(builder.addIntegerPhiIncoming(module, sum, zero, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.addIntegerPhiIncoming(module, sum, nextSum, continueBlock));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildScalarSSALoopModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-scalar-ssa"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateScalarSSALoopKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _populateScalarFunctionKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle helperType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(module, integerType, &integerType, 1, helperType));
    SlangNVVMValueHandle helper = nullptr;
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        helperType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kIncrementScalarHelperName),
        helper));
    SlangNVVMValueHandle helperValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 0, helperValue));

    const SlangNVVMTypeHandle kernelParameterTypes[] = {
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle kernelType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SlangNVVMValueHandle kernel = nullptr;
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        kernelType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kCallScalarKernelName),
        kernel));
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle kernelValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, kernelValue));

    SlangNVVMBlockHandle helperBlock = nullptr;
    SlangNVVMBlockHandle kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, helper, toSlice("helper.entry"), helperBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("kernel.entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, helperBlock));
    SlangNVVMValueHandle one = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 1, one));
    SlangNVVMValueHandle incremented = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_VALUE_OP_ADD, helperValue, one, incremented));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerReturn(module, incremented));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle callResult = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerCall(module, helper, &kernelValue, 1, callResult));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, callResult, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, kernel));
    return SLANG_OK;
}

static SlangResult _buildScalarFunctionModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-scalar-function"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateScalarFunctionKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _populatePointerOffsetKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalIntegerPointerType,
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kCopyIndexedKernelName),
        function));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle index = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, index));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle destinationElement = nullptr;
    SlangNVVMValueHandle sourceElement = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitPointerOffset(module, destination, index, destinationElement));
    SLANG_RETURN_ON_FAIL(builder.emitPointerOffset(module, source, index, sourceElement));
    SLANG_RETURN_ON_FAIL(
        builder.emitLoad(module, sourceElement, 4, SLANG_NVVM_LOAD_FLAG_NONE, value));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destinationElement, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildPointerOffsetModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-pointer-offset"), scope.module));
    SLANG_RETURN_ON_FAIL(_populatePointerOffsetKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _populateByteOffsetPointerKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle uint4Type = nullptr;
    SlangNVVMTypeHandle globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getVectorType(module, integerType, 4, uint4Type));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalIntegerPointerType,
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kCopyByteOffsetKernelName),
        function));

    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle byteOffset = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, source));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, byteOffset));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle vectorPointer = nullptr;
    SlangNVVMValueHandle vectorValue = nullptr;
    SlangNVVMValueHandle firstValue = nullptr;
    SlangNVVMValueHandle firstIndex = nullptr;
    SlangNVVMValueHandle scalarPointer = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitByteOffsetPointer(module, source, byteOffset, uint4Type, vectorPointer));
    SLANG_RETURN_ON_FAIL(
        builder.emitLoad(module, vectorPointer, 16, SLANG_NVVM_LOAD_FLAG_INVARIANT, vectorValue));
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 0, firstIndex));
    SLANG_RETURN_ON_FAIL(
        builder.emitSequentialElementExtract(module, vectorValue, firstIndex, firstValue));
    SLANG_RETURN_ON_FAIL(
        builder.emitByteOffsetPointer(module, destination, byteOffset, integerType, scalarPointer));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, firstValue, scalarPointer, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildByteOffsetPointerModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outNVVMAssembly,
    String& outNVVMAssemblyDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outNVVMAssembly.setNull();
    outNVVMAssemblyDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-byte-offset"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateByteOffsetPointerKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        outNVVMAssembly,
        outNVVMAssemblyDiagnostics));
    return SLANG_OK;
}

static SlangResult _populateArrayElementKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle arrayType = nullptr;
    SlangNVVMTypeHandle globalArrayPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getArrayType(module, integerType, 4, arrayType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        arrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalArrayPointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {
        globalArrayPointerType,
        globalArrayPointerType,
        integerType,
    };
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kCopyArrayElementKernelName),
        function));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle source = nullptr;
    SlangNVVMValueHandle index = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, index));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle destinationElement = nullptr;
    SlangNVVMValueHandle sourceElement = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitSequentialElementPointer(module, destination, index, destinationElement));
    SLANG_RETURN_ON_FAIL(
        builder.emitSequentialElementPointer(module, source, index, sourceElement));
    SLANG_RETURN_ON_FAIL(
        builder.emitLoad(module, sourceElement, 4, SLANG_NVVM_LOAD_FLAG_NONE, value));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destinationElement, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildArrayElementModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-array-element"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateArrayElementKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _populateRelaxedGlobalI32AtomicAddKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle module)
{
    SlangNVVMTypeHandle voidType = nullptr;
    SlangNVVMTypeHandle integerType = nullptr;
    SlangNVVMTypeHandle pointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(
        builder.getPointerType(module, integerType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, pointerType));

    const SlangNVVMTypeHandle parameterTypes[] = {pointerType, pointerType, integerType};
    SlangNVVMTypeHandle functionType = nullptr;
    SlangNVVMValueHandle function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        SLANG_NVVM_LINKAGE_EXTERNAL,
        SLANG_NVVM_FUNCTION_FLAG_NONE,
        toSlice(kRelaxedGlobalI32AtomicAddKernelName),
        function));

    SlangNVVMValueHandle destination = nullptr;
    SlangNVVMValueHandle oldValueDestination = nullptr;
    SlangNVVMValueHandle value = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, oldValueDestination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, value));

    SlangNVVMBlockHandle entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle oldValue = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitRelaxedGlobalI32AtomicAdd(module, destination, value, oldValue));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, oldValue, oldValueDestination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildRelaxedGlobalI32AtomicAddModule(
    const NVVMIRBuilder& builder,
    ComPtr<ISlangBlob>& outAssembly,
    String& outAssemblyDiagnostics,
    ComPtr<ISlangBlob>& outBitcode,
    String& outBitcodeDiagnostics)
{
    outAssembly.setNull();
    outAssemblyDiagnostics = String();
    outBitcode.setNull();
    outBitcodeDiagnostics = String();

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_RETURN_ON_FAIL(
        builder.createModule(toSlice("slang-nvvm-relaxed-global-i32-atomic-add"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateRelaxedGlobalI32AtomicAddKernel(builder, scope.module));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_NVVM_IR_2_0_ASSEMBLY,
        outAssembly,
        outAssemblyDiagnostics));
    SLANG_RETURN_ON_FAIL(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        outBitcode,
        outBitcodeDiagnostics));
    return SLANG_OK;
}

static SlangResult _createDirectNVVMLinkedProgram(
    slang::IGlobalSession* globalSession,
    const char* source,
    ComPtr<slang::ISession>& outSession,
    ComPtr<slang::IComponentType>& outProgram,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    return _createSlangPTXLinkedProgram(
        globalSession,
        source,
        SLANG_EMIT_CUDA_VIA_NVVM,
        outSession,
        outProgram,
        outDiagnostics);
}

// Compiles ordinary Slang source through the public PTX target so this fixture crosses option
// resolution, linked IR, the optional builder, registered libNVVM, and result extraction.
static SlangResult _compileSlangWithDirectNVVM(
    slang::IGlobalSession* globalSession,
    const char* source,
    ComPtr<slang::IBlob>& outCode,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    outCode.setNull();
    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(_createDirectNVVMLinkedProgram(
        globalSession,
        source,
        session,
        linkedProgram,
        outDiagnostics));
    return linkedProgram->getEntryPointCode(0, 0, outCode.writeRef(), outDiagnostics.writeRef());
}

static SlangResult _compileSlangWithPTXMethod(
    slang::IGlobalSession* globalSession,
    const char* source,
    SlangEmitCUDAMethod emissionMethod,
    ComPtr<slang::IBlob>& outCode,
    ComPtr<slang::IBlob>& outDiagnostics)
{
    outCode.setNull();
    ComPtr<slang::ISession> session;
    ComPtr<slang::IComponentType> linkedProgram;
    SLANG_RETURN_ON_FAIL(_createSlangPTXLinkedProgram(
        globalSession,
        source,
        emissionMethod,
        session,
        linkedProgram,
        outDiagnostics));
    return linkedProgram->getEntryPointCode(0, 0, outCode.writeRef(), outDiagnostics.writeRef());
}

static SlangResult _runScalarKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    ScalarRuntimeOperation operation,
    int x,
    int y,
    int expected)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(int)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(int)) != 0)
        return SLANG_FAIL;

    CudaDevicePtr source = 0;
    if (cuda.cuMemAlloc(&source, sizeof(int)) != 0 || !source)
        return SLANG_FAIL;
    CudaBufferGuard sourceGuard{cuda, source};
    if (cuda.cuMemcpyHtoD(source, &x, sizeof(x)) != 0)
        return SLANG_FAIL;

    void* writeParameters[] = {&destination, &x};
    void* copyParameters[] = {&destination, &source};
    void* chooseParameters[] = {&destination, &x, &y};
    void** parameters = nullptr;
    switch (operation)
    {
    case ScalarRuntimeOperation::Write:
    case ScalarRuntimeOperation::BitNot:
    case ScalarRuntimeOperation::Negate:
        parameters = writeParameters;
        break;
    case ScalarRuntimeOperation::Copy:
        parameters = copyParameters;
        break;
    case ScalarRuntimeOperation::Choose:
    case ScalarRuntimeOperation::Multiply:
    case ScalarRuntimeOperation::BitAnd:
    case ScalarRuntimeOperation::BitOr:
    case ScalarRuntimeOperation::BitXor:
    case ScalarRuntimeOperation::Equal:
    case ScalarRuntimeOperation::NotEqual:
    case ScalarRuntimeOperation::GreaterThan:
    case ScalarRuntimeOperation::LessEqual:
    case ScalarRuntimeOperation::GreaterEqual:
        parameters = chooseParameters;
        break;
    }

    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int actual = 0;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return actual == expected ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runFloat32ArithmeticKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    uint32_t operandCount,
    float left,
    float right,
    float expected)
{
    if (operandCount != 1 && operandCount != 2)
        return SLANG_E_INVALID_ARG;
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(float)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(float)) != 0)
        return SLANG_FAIL;

    void* unaryParameters[] = {&destination, &left};
    void* binaryParameters[] = {&destination, &left, &right};
    void** parameters = operandCount == 1 ? unaryParameters : binaryParameters;
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    float actual = 0.0f;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return FloatAsInt(actual) == FloatAsInt(expected) ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runFloat32ComparisonKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    float left,
    float right,
    int expected)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(int)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(int)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&destination, &left, &right};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int actual = 0;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return actual == expected ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runFloat32CopyKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    float sourceValue)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(float)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(float)) != 0)
        return SLANG_FAIL;

    CudaDevicePtr source = 0;
    if (cuda.cuMemAlloc(&source, sizeof(float)) != 0 || !source)
        return SLANG_FAIL;
    CudaBufferGuard sourceGuard{cuda, source};
    if (cuda.cuMemcpyHtoD(source, &sourceValue, sizeof(sourceValue)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&destination, &source};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    float actual = 0.0f;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return actual == sourceValue ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runFloat32ConstantKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    float expected)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(float)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(float)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&destination};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    float actual = 0.0f;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return FloatAsInt(actual) == FloatAsInt(expected) ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runFloat32PhiKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int condition,
    float left,
    float right,
    float expected)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(float)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(float)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&destination, &condition, &left, &right};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    float actual = 0.0f;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return FloatAsInt(actual) == FloatAsInt(expected) ? SLANG_OK : SLANG_FAIL;
}

enum class WaveScalar32Expected
{
    LaneIndex,
    LaneCount,
    ActiveMask,
    IsFirstLane,
    ActiveAnyTrue,
    ActiveAllTrue,
    ActiveAllEqualIntMixed,
    ActiveAllEqualIntUniform,
    ActiveAllEqualUIntMixed,
    ActiveAllEqualUIntUniform,
    ActiveAllEqualFloatMixed,
    ActiveAllEqualFloatUniform,
    UIntFirstLane,
    IntFirstLane,
    FloatFirstLane,
    UIntSourceLane,
    UnmaskedUIntSourceLane,
    IntSourceLane,
    UnmaskedIntSourceLane,
    FloatSourceLane,
    UnmaskedFloatSourceLane,
};

static SlangResult _runWaveScalar32Kernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    WaveScalar32Expected expectedKind,
    int sourceLane = 0)
{
    static const uint32_t kLaneCount = 32;
    const String ptx = _getBlobText(ptxBlob);
    const bool readsSourceLane = expectedKind == WaveScalar32Expected::UIntSourceLane ||
                                 expectedKind == WaveScalar32Expected::UnmaskedUIntSourceLane ||
                                 expectedKind == WaveScalar32Expected::IntSourceLane ||
                                 expectedKind == WaveScalar32Expected::UnmaskedIntSourceLane ||
                                 expectedKind == WaveScalar32Expected::FloatSourceLane ||
                                 expectedKind == WaveScalar32Expected::UnmaskedFloatSourceLane;
    if (!ptx.getLength() || (readsSourceLane && (sourceLane < 0 || sourceLane >= int(kLaneCount))))
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(uint32_t) * kLaneCount) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0xff, sizeof(uint32_t) * kLaneCount) != 0)
        return SLANG_FAIL;

    int32_t intSourceValues[kLaneCount] = {};
    float floatSourceValues[kLaneCount] = {};
    for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
    {
        intSourceValues[laneIndex] = int32_t(laneIndex * 3) - 40;
        floatSourceValues[laneIndex] = float(laneIndex) * 0.75f - 11.5f;
    }
    if (expectedKind == WaveScalar32Expected::ActiveAnyTrue)
    {
        for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
            intSourceValues[laneIndex] = 0;
        intSourceValues[7] = 1;
    }
    else if (expectedKind == WaveScalar32Expected::ActiveAllTrue)
    {
        for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
            intSourceValues[laneIndex] = 1;
        intSourceValues[7] = 0;
    }
    else if (expectedKind == WaveScalar32Expected::ActiveAllEqualIntUniform)
    {
        for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
            intSourceValues[laneIndex] = -17;
    }
    else if (expectedKind == WaveScalar32Expected::ActiveAllEqualUIntUniform)
    {
        for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
            intSourceValues[laneIndex] = 23;
    }
    else if (expectedKind == WaveScalar32Expected::ActiveAllEqualFloatUniform)
    {
        for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
            floatSourceValues[laneIndex] = 3.25f;
    }
    CudaDevicePtr source = 0;
    const bool hasLoadedSource = expectedKind == WaveScalar32Expected::IntSourceLane ||
                                 expectedKind == WaveScalar32Expected::ActiveAnyTrue ||
                                 expectedKind == WaveScalar32Expected::ActiveAllTrue ||
                                 expectedKind == WaveScalar32Expected::ActiveAllEqualIntMixed ||
                                 expectedKind == WaveScalar32Expected::ActiveAllEqualIntUniform ||
                                 expectedKind == WaveScalar32Expected::ActiveAllEqualUIntMixed ||
                                 expectedKind == WaveScalar32Expected::ActiveAllEqualUIntUniform ||
                                 expectedKind == WaveScalar32Expected::ActiveAllEqualFloatMixed ||
                                 expectedKind == WaveScalar32Expected::ActiveAllEqualFloatUniform ||
                                 expectedKind == WaveScalar32Expected::IntFirstLane ||
                                 expectedKind == WaveScalar32Expected::UnmaskedIntSourceLane ||
                                 expectedKind == WaveScalar32Expected::FloatFirstLane ||
                                 expectedKind == WaveScalar32Expected::FloatSourceLane ||
                                 expectedKind == WaveScalar32Expected::UnmaskedFloatSourceLane;
    if (hasLoadedSource && (cuda.cuMemAlloc(&source, sizeof(intSourceValues)) != 0 || !source))
    {
        return SLANG_FAIL;
    }
    CudaBufferGuard sourceGuard{cuda, source};
    if (source)
    {
        const bool hasFloatSource =
            expectedKind == WaveScalar32Expected::FloatFirstLane ||
            expectedKind == WaveScalar32Expected::ActiveAllEqualFloatMixed ||
            expectedKind == WaveScalar32Expected::ActiveAllEqualFloatUniform ||
            expectedKind == WaveScalar32Expected::FloatSourceLane ||
            expectedKind == WaveScalar32Expected::UnmaskedFloatSourceLane;
        const void* sourceValues = hasFloatSource ? static_cast<const void*>(floatSourceValues)
                                                  : static_cast<const void*>(intSourceValues);
        if (cuda.cuMemcpyHtoD(source, sourceValues, sizeof(intSourceValues)) != 0)
            return SLANG_FAIL;
    }

    uint32_t mask = ~uint32_t(0);
    void* laneParameters[] = {&destination};
    void* uintShuffleParameters[] = {&destination, &mask, &sourceLane};
    void* unmaskedUIntShuffleParameters[] = {&destination, &sourceLane};
    void* loadedShuffleParameters[] = {&destination, &source, &mask, &sourceLane};
    void* unmaskedLoadedShuffleParameters[] = {&destination, &source, &sourceLane};
    void* loadedFirstParameters[] = {&destination, &source};
    void** parameters = laneParameters;
    if (expectedKind == WaveScalar32Expected::UIntSourceLane)
        parameters = uintShuffleParameters;
    else if (expectedKind == WaveScalar32Expected::IntFirstLane)
        parameters = loadedFirstParameters;
    else if (expectedKind == WaveScalar32Expected::FloatFirstLane)
        parameters = loadedFirstParameters;
    else if (
        expectedKind == WaveScalar32Expected::ActiveAnyTrue ||
        expectedKind == WaveScalar32Expected::ActiveAllTrue ||
        expectedKind == WaveScalar32Expected::ActiveAllEqualIntMixed ||
        expectedKind == WaveScalar32Expected::ActiveAllEqualIntUniform ||
        expectedKind == WaveScalar32Expected::ActiveAllEqualUIntMixed ||
        expectedKind == WaveScalar32Expected::ActiveAllEqualUIntUniform ||
        expectedKind == WaveScalar32Expected::ActiveAllEqualFloatMixed ||
        expectedKind == WaveScalar32Expected::ActiveAllEqualFloatUniform)
        parameters = loadedFirstParameters;
    else if (expectedKind == WaveScalar32Expected::UnmaskedUIntSourceLane)
        parameters = unmaskedUIntShuffleParameters;
    else if (
        expectedKind == WaveScalar32Expected::UnmaskedIntSourceLane ||
        expectedKind == WaveScalar32Expected::UnmaskedFloatSourceLane)
        parameters = unmaskedLoadedShuffleParameters;
    else if (hasLoadedSource)
        parameters = loadedShuffleParameters;
    if (cuda.cuLaunchKernel(function, 1, 1, 1, kLaneCount, 1, 1, 0, nullptr, parameters, nullptr) !=
            0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    uint32_t actual[kLaneCount] = {};
    if (cuda.cuMemcpyDtoH(actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    for (uint32_t laneIndex = 0; laneIndex < kLaneCount; ++laneIndex)
    {
        uint32_t expected = laneIndex;
        if (expectedKind == WaveScalar32Expected::LaneCount)
            expected = kLaneCount;
        else if (expectedKind == WaveScalar32Expected::ActiveMask)
            expected = ~uint32_t(0);
        else if (expectedKind == WaveScalar32Expected::IsFirstLane)
            expected = laneIndex == 0 ? 1 : 0;
        else if (expectedKind == WaveScalar32Expected::ActiveAnyTrue)
            expected = 1;
        else if (expectedKind == WaveScalar32Expected::ActiveAllTrue)
            expected = 0;
        else if (expectedKind == WaveScalar32Expected::ActiveAllEqualIntMixed)
            expected = 0;
        else if (expectedKind == WaveScalar32Expected::ActiveAllEqualIntUniform)
            expected = 1;
        else if (expectedKind == WaveScalar32Expected::ActiveAllEqualUIntMixed)
            expected = 0;
        else if (expectedKind == WaveScalar32Expected::ActiveAllEqualUIntUniform)
            expected = 1;
        else if (expectedKind == WaveScalar32Expected::ActiveAllEqualFloatMixed)
            expected = 0;
        else if (expectedKind == WaveScalar32Expected::ActiveAllEqualFloatUniform)
            expected = 1;
        else if (expectedKind == WaveScalar32Expected::UIntFirstLane)
            expected = 0;
        else if (expectedKind == WaveScalar32Expected::IntFirstLane)
            expected = uint32_t(intSourceValues[0]);
        else if (expectedKind == WaveScalar32Expected::FloatFirstLane)
            expected = uint32_t(FloatAsInt(floatSourceValues[0]));
        else if (
            expectedKind == WaveScalar32Expected::UIntSourceLane ||
            expectedKind == WaveScalar32Expected::UnmaskedUIntSourceLane)
            expected = uint32_t(sourceLane);
        else if (
            expectedKind == WaveScalar32Expected::IntSourceLane ||
            expectedKind == WaveScalar32Expected::UnmaskedIntSourceLane)
            expected = uint32_t(intSourceValues[sourceLane]);
        else if (
            expectedKind == WaveScalar32Expected::FloatSourceLane ||
            expectedKind == WaveScalar32Expected::UnmaskedFloatSourceLane)
            expected = uint32_t(FloatAsInt(floatSourceValues[sourceLane]));
        if (actual[laneIndex] != expected)
            return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _runWaveLaneIndexKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::LaneIndex);
}

static SlangResult _runWaveLaneCountKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::LaneCount);
}

static SlangResult _runWaveActiveMaskKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveMask);
}

static SlangResult _runWaveIsFirstLaneKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::IsFirstLane);
}

static SlangResult _runWaveActiveAnyTrueKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAnyTrue);
}

static SlangResult _runWaveActiveAllTrueKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllTrue);
}

static SlangResult _runWaveActiveAllEqualIntKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    SLANG_RETURN_ON_FAIL(
        _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllEqualIntMixed));
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllEqualIntUniform);
}

static SlangResult _runWaveActiveAllEqualUIntKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    SLANG_RETURN_ON_FAIL(
        _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllEqualUIntMixed));
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllEqualUIntUniform);
}

static SlangResult _runWaveActiveAllEqualFloatKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    SLANG_RETURN_ON_FAIL(
        _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllEqualFloatMixed));
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::ActiveAllEqualFloatUniform);
}

static SlangResult _runWaveReadLaneFirstUIntKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::UIntFirstLane);
}

static SlangResult _runWaveReadLaneFirstIntKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::IntFirstLane);
}

static SlangResult _runWaveReadLaneFirstFloatKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::FloatFirstLane);
}

static SlangResult _runWaveReadLaneAtUIntKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int sourceLane)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::UIntSourceLane, sourceLane);
}

static SlangResult _runUnmaskedWaveReadLaneAtUIntKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int sourceLane)
{
    return _runWaveScalar32Kernel(
        cuda,
        ptxBlob,
        WaveScalar32Expected::UnmaskedUIntSourceLane,
        sourceLane);
}

static SlangResult _runWaveReadLaneAtIntKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int sourceLane)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::IntSourceLane, sourceLane);
}

static SlangResult _runUnmaskedWaveReadLaneAtIntKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int sourceLane)
{
    return _runWaveScalar32Kernel(
        cuda,
        ptxBlob,
        WaveScalar32Expected::UnmaskedIntSourceLane,
        sourceLane);
}

static SlangResult _runWaveReadLaneAtFloatKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int sourceLane)
{
    return _runWaveScalar32Kernel(cuda, ptxBlob, WaveScalar32Expected::FloatSourceLane, sourceLane);
}

static SlangResult _runUnmaskedWaveReadLaneAtFloatKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int sourceLane)
{
    return _runWaveScalar32Kernel(
        cuda,
        ptxBlob,
        WaveScalar32Expected::UnmaskedFloatSourceLane,
        sourceLane);
}

static SlangResult _runRelaxedGlobalI32AtomicAddKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    uint32_t gridWidth,
    uint32_t blockWidth,
    int initialValue)
{
    const String ptx = _getBlobText(ptxBlob);
    const uint64_t invocationCount = uint64_t(gridWidth) * uint64_t(blockWidth);
    const int64_t maxIncrement = int64_t(INT_MAX) - int64_t(initialValue);
    if (!ptx.getLength() || !gridWidth || !blockWidth || invocationCount > uint64_t(maxIncrement))
    {
        return SLANG_E_INVALID_ARG;
    }

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(int)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemcpyHtoD(destination, &initialValue, sizeof(initialValue)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&destination};
    if (cuda.cuLaunchKernel(
            function,
            gridWidth,
            1,
            1,
            blockWidth,
            1,
            1,
            0,
            nullptr,
            parameters,
            nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int actualValue = 0;
    if (cuda.cuMemcpyDtoH(&actualValue, destination, sizeof(actualValue)) != 0)
        return SLANG_FAIL;
    const int expectedValue = int(int64_t(initialValue) + int64_t(invocationCount));
    return actualValue == expectedValue ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runCUDAExecutionKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    static const uint32_t kGrid[] = {3, 2, 2};
    static const uint32_t kBlock[] = {4, 3, 2};
    static const uint32_t kValuesPerThread = 12;
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    const size_t invocationCount =
        size_t(kGrid[0]) * kGrid[1] * kGrid[2] * kBlock[0] * kBlock[1] * kBlock[2];
    const size_t outputCount = invocationCount * kValuesPerThread;
    CudaDevicePtr counter = 0;
    if (cuda.cuMemAlloc(&counter, sizeof(int)) != 0 || !counter)
        return SLANG_FAIL;
    CudaBufferGuard counterGuard{cuda, counter};
    if (cuda.cuMemsetD8(counter, 0, sizeof(int)) != 0)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, outputCount * sizeof(uint32_t)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0xff, outputCount * sizeof(uint32_t)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&counter, &destination};
    if (cuda.cuLaunchKernel(
            function,
            kGrid[0],
            kGrid[1],
            kGrid[2],
            kBlock[0],
            kBlock[1],
            kBlock[2],
            0,
            nullptr,
            parameters,
            nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    List<uint32_t> actual;
    actual.setCount(Index(outputCount));
    if (cuda.cuMemcpyDtoH(actual.getBuffer(), destination, outputCount * sizeof(uint32_t)) != 0)
    {
        return SLANG_FAIL;
    }

    int actualCount = 0;
    if (cuda.cuMemcpyDtoH(&actualCount, counter, sizeof(actualCount)) != 0 ||
        actualCount != int(invocationCount))
    {
        return SLANG_FAIL;
    }

    List<uint8_t> seen;
    seen.setCount(Index(invocationCount));
    memset(seen.getBuffer(), 0, invocationCount);
    for (size_t invocation = 0; invocation < invocationCount; ++invocation)
    {
        const size_t outputBase = invocation * kValuesPerThread;
        const uint32_t threadX = actual[Index(outputBase) + 0];
        const uint32_t threadY = actual[Index(outputBase) + 1];
        const uint32_t threadZ = actual[Index(outputBase) + 2];
        const uint32_t blockX = actual[Index(outputBase) + 3];
        const uint32_t blockY = actual[Index(outputBase) + 4];
        const uint32_t blockZ = actual[Index(outputBase) + 5];
        if (threadX >= kBlock[0] || threadY >= kBlock[1] || threadZ >= kBlock[2] ||
            blockX >= kGrid[0] || blockY >= kGrid[1] || blockZ >= kGrid[2] ||
            actual[Index(outputBase) + 6] != kBlock[0] ||
            actual[Index(outputBase) + 7] != kBlock[1] ||
            actual[Index(outputBase) + 8] != kBlock[2] ||
            actual[Index(outputBase) + 9] != kGrid[0] ||
            actual[Index(outputBase) + 10] != kGrid[1] ||
            actual[Index(outputBase) + 11] != kGrid[2])
        {
            return SLANG_FAIL;
        }

        const size_t threadLinear = threadX + threadY * kBlock[0] + threadZ * kBlock[0] * kBlock[1];
        const size_t blockLinear = blockX + blockY * kGrid[0] + blockZ * kGrid[0] * kGrid[1];
        const size_t expectedInvocation =
            blockLinear * (kBlock[0] * kBlock[1] * kBlock[2]) + threadLinear;
        if (seen[Index(expectedInvocation)])
            return SLANG_FAIL;
        seen[Index(expectedInvocation)] = 1;
    }
    return SLANG_OK;
}

static SlangResult _runMixedNumericKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};
    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr output8 = 0;
    CudaDevicePtr output16 = 0;
    CudaDevicePtr output64 = 0;
    CudaDevicePtr output32 = 0;
    CudaDevicePtr outputFloat = 0;
    CudaDevicePtr outputVector = 0;
    CudaDevicePtr leftVector = 0;
    CudaDevicePtr rightVector = 0;
    if (cuda.cuMemAlloc(&output8, sizeof(int8_t)) != 0 || !output8)
        return SLANG_FAIL;
    CudaBufferGuard output8Guard{cuda, output8};
    if (cuda.cuMemAlloc(&output16, sizeof(uint16_t)) != 0 || !output16)
        return SLANG_FAIL;
    CudaBufferGuard output16Guard{cuda, output16};
    if (cuda.cuMemAlloc(&output64, sizeof(int64_t)) != 0 || !output64)
        return SLANG_FAIL;
    CudaBufferGuard output64Guard{cuda, output64};
    if (cuda.cuMemAlloc(&output32, sizeof(int32_t)) != 0 || !output32)
        return SLANG_FAIL;
    CudaBufferGuard output32Guard{cuda, output32};
    if (cuda.cuMemAlloc(&outputFloat, sizeof(float)) != 0 || !outputFloat)
        return SLANG_FAIL;
    CudaBufferGuard outputFloatGuard{cuda, outputFloat};
    if (cuda.cuMemAlloc(&outputVector, sizeof(int32_t) * 2) != 0 || !outputVector)
        return SLANG_FAIL;
    CudaBufferGuard outputVectorGuard{cuda, outputVector};
    if (cuda.cuMemAlloc(&leftVector, sizeof(int32_t) * 2) != 0 || !leftVector)
        return SLANG_FAIL;
    CudaBufferGuard leftVectorGuard{cuda, leftVector};
    if (cuda.cuMemAlloc(&rightVector, sizeof(int32_t) * 2) != 0 || !rightVector)
        return SLANG_FAIL;
    CudaBufferGuard rightVectorGuard{cuda, rightVector};

    const int32_t leftValues[] = {1, -2};
    const int32_t rightValues[] = {3, 7};
    if (cuda.cuMemcpyHtoD(leftVector, leftValues, sizeof(leftValues)) != 0 ||
        cuda.cuMemcpyHtoD(rightVector, rightValues, sizeof(rightValues)) != 0)
    {
        return SLANG_FAIL;
    }

    int8_t a = -100;
    uint8_t b = 250;
    int16_t c = -1234;
    uint16_t d = 60000;
    int64_t e = -7;
    uint64_t f = 9;
    float g = -3.75f;
    void* parameters[] = {
        &output8,
        &output16,
        &output64,
        &output32,
        &outputFloat,
        &outputVector,
        &leftVector,
        &rightVector,
        &a,
        &b,
        &c,
        &d,
        &e,
        &f,
        &g,
    };
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int8_t actual8 = 0;
    uint16_t actual16 = 0;
    int64_t actual64 = 0;
    int32_t actual32 = 0;
    float actualFloat = 0.0f;
    int32_t actualVector[2] = {};
    if (cuda.cuMemcpyDtoH(&actual8, output8, sizeof(actual8)) != 0 ||
        cuda.cuMemcpyDtoH(&actual16, output16, sizeof(actual16)) != 0 ||
        cuda.cuMemcpyDtoH(&actual64, output64, sizeof(actual64)) != 0 ||
        cuda.cuMemcpyDtoH(&actual32, output32, sizeof(actual32)) != 0 ||
        cuda.cuMemcpyDtoH(&actualFloat, outputFloat, sizeof(actualFloat)) != 0 ||
        cuda.cuMemcpyDtoH(actualVector, outputVector, sizeof(actualVector)) != 0)
    {
        return SLANG_FAIL;
    }

    const uint16_t expected16 = uint16_t(uint16_t(uint16_t(c) + d) ^ uint16_t(0x55aa));
    return actual8 == int8_t(105) && actual16 == expected16 && actual64 == 6 && actual32 == 1247 &&
                   actualFloat == -1237.75f && actualVector[0] == 4 && actualVector[1] == 5
               ? SLANG_OK
               : SLANG_FAIL;
}

static SlangResult _runSharedMemoryKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    static const uint32_t kThreadCount = 64;
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr counter = 0;
    if (cuda.cuMemAlloc(&counter, sizeof(int)) != 0 || !counter)
        return SLANG_FAIL;
    CudaBufferGuard counterGuard{cuda, counter};
    if (cuda.cuMemsetD8(counter, 0, sizeof(int)) != 0)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, kThreadCount * sizeof(int)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0xff, kThreadCount * sizeof(int)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&counter, &destination};
    if (cuda.cuLaunchKernel(
            function,
            1,
            1,
            1,
            kThreadCount,
            1,
            1,
            0,
            nullptr,
            parameters,
            nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int actualCount = 0;
    int actual[kThreadCount] = {};
    if (cuda.cuMemcpyDtoH(&actualCount, counter, sizeof(actualCount)) != 0 ||
        cuda.cuMemcpyDtoH(actual, destination, sizeof(actual)) != 0 ||
        actualCount != int(kThreadCount))
    {
        return SLANG_FAIL;
    }
    for (uint32_t ticket = 0; ticket < kThreadCount; ++ticket)
    {
        const int expected = int((kThreadCount - 1 - ticket) * 3 + 1);
        if (actual[ticket] != expected)
            return SLANG_FAIL;
    }
    return SLANG_OK;
}

static SlangResult _runLibdeviceSineKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    float input,
    float tolerance)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, kLibdeviceSineKernelName) != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(float)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemsetD8(destination, 0, sizeof(float)) != 0)
        return SLANG_FAIL;

    void* parameters[] = {&destination, &input};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    float actual = 0.0f;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    const float expected = ::sinf(input);
    return ::fabsf(actual - expected) <= tolerance ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runPointerOffsetKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
    int index,
    bool useInteriorBases)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    static const int kSourceValues[] = {19, -7, 42, 83};
    static const int kDestinationSentinels[] = {101, 102, 103, 104};
    int expected[SLANG_COUNT_OF(kDestinationSentinels)];
    ::memcpy(expected, kDestinationSentinels, sizeof(expected));
    const Index allocationIndex = Index(index) + (useInteriorBases ? 1 : 0);
    if (allocationIndex < 0 || allocationIndex >= SLANG_COUNT_OF(kSourceValues))
        return SLANG_E_INVALID_ARG;
    expected[allocationIndex] = kSourceValues[allocationIndex];

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(kDestinationSentinels)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    if (cuda.cuMemcpyHtoD(destination, kDestinationSentinels, sizeof(kDestinationSentinels)) != 0)
    {
        return SLANG_FAIL;
    }

    CudaDevicePtr source = 0;
    if (cuda.cuMemAlloc(&source, sizeof(kSourceValues)) != 0 || !source)
        return SLANG_FAIL;
    CudaBufferGuard sourceGuard{cuda, source};
    if (cuda.cuMemcpyHtoD(source, kSourceValues, sizeof(kSourceValues)) != 0)
        return SLANG_FAIL;

    const size_t interiorByteOffset = useInteriorBases ? sizeof(int) : 0;
    CudaDevicePtr destinationArgument = destination + interiorByteOffset;
    CudaDevicePtr sourceArgument = source + interiorByteOffset;
    void* parameters[] = {&destinationArgument, &sourceArgument, &index};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int actual[SLANG_COUNT_OF(kDestinationSentinels)] = {};
    if (cuda.cuMemcpyDtoH(actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return ::memcmp(actual, expected, sizeof(actual)) == 0 ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _runRawRWStructuredBufferI32StoreKernel(CudaDriverApi& cuda, ISlangBlob* ptxBlob)
{
    const String ptx = _getBlobText(ptxBlob);
    if (!ptx.getLength())
        return SLANG_E_INVALID_ARG;

    CudaModule module = nullptr;
    if (cuda.cuModuleLoadData(&module, ptx.getBuffer()) != 0 || !module)
        return SLANG_FAIL;
    CudaModuleGuard moduleGuard{cuda, module};

    CudaFunction function = nullptr;
    if (cuda.cuModuleGetFunction(&function, module, "computeMain") != 0 || !function)
        return SLANG_FAIL;

    CudaDevicePtr destination = 0;
    if (cuda.cuMemAlloc(&destination, sizeof(int)) != 0 || !destination)
        return SLANG_FAIL;
    CudaBufferGuard destinationGuard{cuda, destination};
    const int sentinel = -1;
    if (cuda.cuMemcpyHtoD(destination, &sentinel, sizeof(sentinel)) != 0)
        return SLANG_FAIL;

    struct RawRWStructuredBufferI32Argument
    {
        CudaDevicePtr data;
        uint64_t count;
    };
    static_assert(sizeof(RawRWStructuredBufferI32Argument) == 16);
    RawRWStructuredBufferI32Argument buffer = {destination, 1};
    int index = 0;
    void* parameters[] = {&buffer, &index};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    int actual = 0;
    if (cuda.cuMemcpyDtoH(&actual, destination, sizeof(actual)) != 0)
        return SLANG_FAIL;
    return actual == 42 ? SLANG_OK : SLANG_FAIL;
}

static ComPtr<IArtifact> _createPTXArtifact(ISlangBlob* ptx)
{
    if (!ptx)
        return nullptr;
    auto artifact = ArtifactUtil::createArtifactForCompileTarget(SLANG_PTX);
    artifact->addRepresentationUnknown(
        RawBlob::create(ptx->getBufferPointer(), ptx->getBufferSize()));
    return artifact;
}

static Index _countOccurrences(const UnownedStringSlice& text, const UnownedStringSlice& needle)
{
    if (!needle.getLength())
        return 0;

    Index count = 0;
    Index cursor = 0;
    while (cursor < text.getLength())
    {
        const Index index = text.tail(cursor).indexOf(needle);
        if (index < 0)
            break;
        ++count;
        cursor += index + needle.getLength();
    }
    return count;
}

static SlangResult _extractPTXEntry(
    const UnownedStringSlice& ptx,
    const UnownedStringSlice& entryPointName,
    String& outSignature,
    String& outBody)
{
    outSignature = String();
    outBody = String();
    if (!ptx.getLength() || !entryPointName.getLength())
        return SLANG_E_INVALID_ARG;

    StringBuilder markerBuilder;
    markerBuilder << ".visible .entry " << entryPointName;
    const UnownedStringSlice marker = markerBuilder.getUnownedSlice();
    const Index entryIndex = ptx.indexOf(marker);
    if (entryIndex < 0)
        return SLANG_E_NOT_FOUND;

    const UnownedStringSlice entryTail = ptx.tail(entryIndex);
    if (entryTail.getLength() <= marker.getLength())
        return SLANG_FAIL;
    const char delimiter = entryTail[marker.getLength()];
    if (delimiter != '(' && delimiter != ' ' && delimiter != '\t' && delimiter != '\r' &&
        delimiter != '\n')
    {
        return SLANG_FAIL;
    }

    const Index bodyStart = entryTail.indexOf('{');
    if (bodyStart < 0)
        return SLANG_FAIL;

    Index braceDepth = 0;
    Index bodyEnd = -1;
    for (Index i = bodyStart; i < entryTail.getLength(); ++i)
    {
        if (entryTail[i] == '{')
        {
            ++braceDepth;
        }
        else if (entryTail[i] == '}')
        {
            if (--braceDepth == 0)
            {
                bodyEnd = i + 1;
                break;
            }
        }
    }
    if (bodyEnd < 0 || braceDepth != 0)
        return SLANG_FAIL;

    outSignature = String(entryTail.head(bodyStart));
    outBody = String(entryTail.subString(bodyStart, bodyEnd - bodyStart));
    return SLANG_OK;
}

static SlangResult _getPTXScalarBitWidth(const UnownedStringSlice& text, uint32_t& outBitWidth)
{
    outBitWidth = 0;
    static const char* k8BitSpellings[] = {".b8", ".s8", ".u8"};
    static const char* k16BitSpellings[] = {".b16", ".s16", ".u16", ".f16"};
    static const char* k32BitSpellings[] = {".b32", ".s32", ".u32", ".f32"};
    static const char* k64BitSpellings[] = {".b64", ".s64", ".u64"};

    int matchCount = 0;
    for (const char* spelling : k8BitSpellings)
    {
        if (text.indexOf(UnownedStringSlice(spelling)) >= 0)
        {
            outBitWidth = 8;
            ++matchCount;
        }
    }
    for (const char* spelling : k32BitSpellings)
    {
        if (text.indexOf(UnownedStringSlice(spelling)) >= 0)
        {
            outBitWidth = 32;
            ++matchCount;
        }
    }
    for (const char* spelling : k16BitSpellings)
    {
        if (text.indexOf(UnownedStringSlice(spelling)) >= 0)
        {
            outBitWidth = 16;
            ++matchCount;
        }
    }
    for (const char* spelling : k64BitSpellings)
    {
        if (text.indexOf(UnownedStringSlice(spelling)) >= 0)
        {
            outBitWidth = 64;
            ++matchCount;
        }
    }
    return matchCount == 1 ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _collectPTXParameterWidths(
    const UnownedStringSlice& signature,
    List<uint32_t>& outBitWidths)
{
    outBitWidths.clear();
    const Index parameterListStart = signature.indexOf('(');
    const Index parameterListEnd = signature.lastIndexOf(')');
    if (parameterListStart < 0 || parameterListEnd < parameterListStart)
        return SLANG_FAIL;

    const UnownedStringSlice parameterList =
        signature.subString(parameterListStart + 1, parameterListEnd - parameterListStart - 1);
    Index cursor = 0;
    while (cursor < parameterList.getLength())
    {
        const UnownedStringSlice remaining = parameterList.tail(cursor);
        const Index markerIndex = remaining.indexOf(toSlice(".param"));
        if (markerIndex < 0)
            break;

        const Index declarationStart = cursor + markerIndex;
        const UnownedStringSlice declarationTail = parameterList.tail(declarationStart);
        const Index commaIndex = declarationTail.indexOf(',');
        const UnownedStringSlice declaration =
            commaIndex >= 0 ? declarationTail.head(commaIndex) : declarationTail;

        uint32_t bitWidth = 0;
        SLANG_RETURN_ON_FAIL(_getPTXScalarBitWidth(declaration, bitWidth));
        outBitWidths.add(bitWidth);

        if (commaIndex < 0)
            break;
        cursor = declarationStart + commaIndex + 1;
    }
    return SLANG_OK;
}

static bool _ptxEntryHasInstruction(
    const UnownedStringSlice& body,
    const UnownedStringSlice& instructionFamily,
    uint32_t bitWidth)
{
    Index cursor = 0;
    while (cursor < body.getLength())
    {
        const UnownedStringSlice remaining = body.tail(cursor);
        const Index instructionIndex = remaining.indexOf(instructionFamily);
        if (instructionIndex < 0)
            return false;

        const Index absoluteInstructionIndex = cursor + instructionIndex;
        Index lineStart = absoluteInstructionIndex;
        while (lineStart > 0 && body[lineStart - 1] != '\n')
            --lineStart;
        const UnownedStringSlice linePrefix =
            body.subString(lineStart, absoluteInstructionIndex - lineStart);
        const bool isLineComment = linePrefix.indexOf(toSlice("//")) >= 0;
        const char previousCharacter =
            absoluteInstructionIndex ? body[absoluteInstructionIndex - 1] : '\0';
        const bool hasTokenBoundary = absoluteInstructionIndex == 0 || previousCharacter == ' ' ||
                                      previousCharacter == '\t' || previousCharacter == '\r' ||
                                      previousCharacter == '\n' || previousCharacter == '{' ||
                                      previousCharacter == ';';
        const UnownedStringSlice instructionTail = body.tail(absoluteInstructionIndex);
        const Index terminatorIndex = instructionTail.indexOf(';');
        const UnownedStringSlice instruction =
            terminatorIndex >= 0 ? instructionTail.head(terminatorIndex + 1) : instructionTail;
        uint32_t instructionBitWidth = 0;
        if (!isLineComment && hasTokenBoundary &&
            SLANG_SUCCEEDED(_getPTXScalarBitWidth(instruction, instructionBitWidth)) &&
            instructionBitWidth == bitWidth)
        {
            return true;
        }
        cursor = absoluteInstructionIndex + instructionFamily.getLength();
    }
    return false;
}

struct PTXEntrySummary
{
    List<uint32_t> parameterBitWidths;
    bool hasGlobalLoad32 = false;
    bool hasGlobalStore32 = false;
    bool hasAdd32 = false;
    bool hasFloatAdd32 = false;
    bool hasFloatSubtract32 = false;
    bool hasFloatMultiply32 = false;
    bool hasFloatDivide32 = false;
    bool hasFloatNegate32 = false;
    bool hasFloatComparison32 = false;
    bool hasMultiply32 = false;
    bool hasBitAnd32 = false;
    bool hasBitOr32 = false;
    bool hasBitXor32 = false;
    bool hasBitNot32 = false;
    bool hasNegate32 = false;
    bool hasRelaxedGlobalI32AtomicAdd = false;
    bool hasSubtract32 = false;
    bool hasSignedComparison32 = false;
    bool hasEqualityComparison32 = false;
};

static SlangResult _summarizePTXEntry(
    const UnownedStringSlice& ptx,
    const UnownedStringSlice& entryPointName,
    PTXEntrySummary& outSummary)
{
    outSummary.parameterBitWidths.clear();
    outSummary.hasGlobalLoad32 = false;
    outSummary.hasGlobalStore32 = false;
    outSummary.hasAdd32 = false;
    outSummary.hasFloatAdd32 = false;
    outSummary.hasFloatSubtract32 = false;
    outSummary.hasFloatMultiply32 = false;
    outSummary.hasFloatDivide32 = false;
    outSummary.hasFloatNegate32 = false;
    outSummary.hasFloatComparison32 = false;
    outSummary.hasMultiply32 = false;
    outSummary.hasBitAnd32 = false;
    outSummary.hasBitOr32 = false;
    outSummary.hasBitXor32 = false;
    outSummary.hasBitNot32 = false;
    outSummary.hasNegate32 = false;
    outSummary.hasRelaxedGlobalI32AtomicAdd = false;
    outSummary.hasSubtract32 = false;
    outSummary.hasSignedComparison32 = false;
    outSummary.hasEqualityComparison32 = false;

    String signature;
    String body;
    SLANG_RETURN_ON_FAIL(_extractPTXEntry(ptx, entryPointName, signature, body));
    SLANG_RETURN_ON_FAIL(
        _collectPTXParameterWidths(signature.getUnownedSlice(), outSummary.parameterBitWidths));
    outSummary.hasGlobalLoad32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("ld.global"), 32);
    outSummary.hasGlobalStore32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("st.global"), 32);
    outSummary.hasAdd32 = _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("add"), 32);
    outSummary.hasFloatAdd32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("add.f32"), 32);
    outSummary.hasFloatSubtract32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("sub.f32"), 32);
    outSummary.hasFloatMultiply32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("mul.f32"), 32);
    outSummary.hasFloatDivide32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("div"), 32);
    outSummary.hasFloatNegate32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("neg.f32"), 32);
    outSummary.hasFloatComparison32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.eq.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.neu.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.gt.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.leu.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.le.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.gtu.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.ge.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.ltu.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.lt.f32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.geu.f32"), 32);
    outSummary.hasMultiply32 = _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("mul"), 32);
    outSummary.hasBitAnd32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("and.b32"), 32);
    outSummary.hasBitOr32 = _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("or.b32"), 32);
    outSummary.hasBitXor32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("xor.b32"), 32);
    outSummary.hasBitNot32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("not.b32"), 32);
    outSummary.hasNegate32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("neg.s32"), 32);
    outSummary.hasRelaxedGlobalI32AtomicAdd =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("atom.global.add.u32"), 32);
    outSummary.hasSubtract32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("sub.s32"), 32);
    outSummary.hasSignedComparison32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.lt.s32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.ge.s32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.gt.s32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.le.s32"), 32);
    outSummary.hasEqualityComparison32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.eq.s32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.ne.s32"), 32);
    return SLANG_OK;
}

static bool _hasPTXParameterWidths(
    const PTXEntrySummary& summary,
    const uint32_t* expectedBitWidths,
    Index expectedCount)
{
    if (summary.parameterBitWidths.getCount() != expectedCount)
        return false;
    for (Index i = 0; i < expectedCount; ++i)
    {
        if (summary.parameterBitWidths[i] != expectedBitWidths[i])
            return false;
    }
    return true;
}

static bool _haveEqualPTXParameterWidths(const PTXEntrySummary& left, const PTXEntrySummary& right)
{
    return _hasPTXParameterWidths(
        left,
        right.parameterBitWidths.getBuffer(),
        right.parameterBitWidths.getCount());
}

static bool _ptxContainsEntry(IArtifact* artifact, const UnownedStringSlice& entryPointName)
{
    if (!artifact)
        return false;
    ComPtr<ISlangBlob> ptxBlob;
    if (SLANG_FAILED(artifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())) || !ptxBlob)
        return false;

    const UnownedStringSlice ptx(
        static_cast<const char*>(ptxBlob->getBufferPointer()),
        ptxBlob->getBufferSize());
    StringBuilder expected;
    expected << ".visible .entry " << entryPointName;
    return ptx.indexOf(expected.getUnownedSlice()) >= 0;
}

struct TempPtxasOutput
{
    String lockPath;
    String cubinPath;

    ~TempPtxasOutput()
    {
        if (cubinPath.getLength())
            File::remove(cubinPath);
        if (lockPath.getLength())
            File::remove(lockPath);
    }
};

static SlangResult _assemblePTX(IArtifact* ptxArtifact, const String& ptxasPath)
{
    if (!ptxArtifact || !File::exists(ptxasPath))
        return SLANG_E_NOT_FOUND;

    ComPtr<IOSFileArtifactRepresentation> ptxFile;
    SLANG_RETURN_ON_FAIL(ptxArtifact->requireFile(ArtifactKeep::No, ptxFile.writeRef()));
    if (!ptxFile)
        return SLANG_FAIL;

    TempPtxasOutput tempOutput;
    SLANG_RETURN_ON_FAIL(File::generateTemporary(toSlice("slang-nvvm-ptxas"), tempOutput.lockPath));
    tempOutput.cubinPath = tempOutput.lockPath + ".cubin";

    CommandLine commandLine;
    commandLine.setExecutableLocation(
        ExecutableLocation(ExecutableLocation::Type::Path, ptxasPath));
    commandLine.addArg("-arch=sm_75");
    commandLine.addArg("-v");
    commandLine.addArg(ptxFile->getPath());
    commandLine.addArg("-o");
    commandLine.addArg(tempOutput.cubinPath);

    ExecuteResult executeResult;
    const SlangResult executeCallResult = ProcessUtil::execute(commandLine, executeResult);
    if (SLANG_FAILED(executeCallResult) || executeResult.resultCode != 0)
    {
        StringBuilder message;
        message << "ptxas stdout:\n" << executeResult.standardOutput;
        message << "\nptxas stderr:\n" << executeResult.standardError;
        getTestReporter()->message(TestMessageType::Info, message.getBuffer());
        return SLANG_FAILED(executeCallResult) ? executeCallResult : SLANG_FAIL;
    }
    return File::exists(tempOutput.cubinPath) ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _findToolkitFromCUDAPath(String& outCudaRoot)
{
    outCudaRoot = String();
    StringBuilder cudaRootBuilder;
    if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(toSlice("CUDA_PATH"), cudaRootBuilder)) ||
        !cudaRootBuilder.getLength())
    {
        return SLANG_E_NOT_FOUND;
    }
    outCudaRoot = cudaRootBuilder.produceString();
    return SLANG_OK;
}

static SlangResult _findPtxasInToolkit(const String& cudaRoot, String& outPtxasPath)
{
    outPtxasPath = Path::combine(
        Path::combine(cudaRoot, "bin"),
        String("ptxas") + String(Process::getExecutableSuffix()));
    return File::exists(outPtxasPath) ? SLANG_OK : SLANG_E_NOT_FOUND;
}

static SlangResult _findPtxasFromCUDAPath(String& outCudaRoot, String& outPtxasPath)
{
    SLANG_RETURN_ON_FAIL(_findToolkitFromCUDAPath(outCudaRoot));
    return _findPtxasInToolkit(outCudaRoot, outPtxasPath);
}

static SlangResult _findLibdeviceNVVMToolkitFromCUDAPath(String& outCudaRoot)
{
    SLANG_RETURN_ON_FAIL(_findToolkitFromCUDAPath(outCudaRoot));
    const String libdevicePath =
        Path::combine(Path::combine(outCudaRoot, "nvvm", "libdevice"), "libdevice.10.bc");
    if (!File::exists(libdevicePath))
        return SLANG_E_NOT_FOUND;

    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_RETURN_ON_FAIL(_locateRealNVVM(outCudaRoot, set, compiler));
    return compiler ? SLANG_OK : SLANG_FAIL;
}

struct TempDirectory
{
    String path;

    ~TempDirectory()
    {
        if (path.getLength())
            Path::removeNonEmpty(path);
    }
};

static SlangResult _createTempDirectory(TempDirectory& outDirectory)
{
    SLANG_RETURN_ON_FAIL(
        File::generateTemporary(toSlice("slang-nvvm-discovery"), outDirectory.path));
    SLANG_RETURN_ON_FAIL(File::remove(outDirectory.path));
    if (!Path::createDirectoryRecursive(outDirectory.path))
        return SLANG_FAIL;
    return SLANG_OK;
}
static SlangResult _createFakeNVVMToolkit(
    const String& root,
    const void* libdeviceBytes,
    size_t libdeviceSize,
    String& outCandidatePath,
    String& outLibdevicePath)
{
#if SLANG_WINDOWS_FAMILY
    const String nvvmBinaryDirectory = Path::combine(Path::combine(root, "nvvm"), "bin");
    outCandidatePath = Path::combine(nvvmBinaryDirectory, "nvvm64_40_0.dll");
#elif SLANG_LINUX_FAMILY
    const String nvvmBinaryDirectory = Path::combine(Path::combine(root, "nvvm"), "lib64");
    outCandidatePath = Path::combine(nvvmBinaryDirectory, "libnvvm.so.4");
#else
    SLANG_UNUSED(root);
    SLANG_UNUSED(libdeviceBytes);
    SLANG_UNUSED(libdeviceSize);
    outCandidatePath = String();
    outLibdevicePath = String();
    return SLANG_E_NOT_AVAILABLE;
#endif
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    const String libdeviceDirectory = Path::combine(Path::combine(root, "nvvm"), "libdevice");
    if (!Path::createDirectoryRecursive(nvvmBinaryDirectory) ||
        !Path::createDirectoryRecursive(libdeviceDirectory))
    {
        return SLANG_FAIL;
    }
    SLANG_RETURN_ON_FAIL(File::writeAllText(outCandidatePath, String()));
    outLibdevicePath = Path::combine(libdeviceDirectory, "libdevice.10.bc");
    if (libdeviceBytes || libdeviceSize)
        SLANG_RETURN_ON_FAIL(File::writeAllBytes(outLibdevicePath, libdeviceBytes, libdeviceSize));
    return SLANG_OK;
#endif
}

static void _checkRejectedCompiledResult(FakeResultMode resultMode)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.resultMode = resultMode;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    const SlangResult result =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);

    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(diagnostics->getCount() >= 1);
    SLANG_CHECK(outputArtifact->getRepresentations().count == 0);
    SLANG_CHECK(gFakeNVVM.getResultSizeCallCount == 1);
    SLANG_CHECK(gFakeNVVM.getResultCallCount == 1);
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
}

static void _checkRejectedInputResult(SlangResult result, IArtifact* artifact)
{
    SLANG_CHECK(SLANG_FAILED(result));
    SLANG_CHECK_ABORT(artifact != nullptr);
    SLANG_CHECK(
        artifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics = _findDiagnostics(artifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(diagnostics->getCount() >= 1);
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

enum class NVVMLLVMLoadOrder
{
    LLVMFirst,
    NVVMFirst,
};

static const char kNVVMCoexistenceChildEnv[] = "SLANG_NVVM_COEXISTENCE_CHILD_ORDER";
static const char kNVVMCoexistenceTestName[] =
    "slang-unit-test-tool/nvvmIRBuilderCoexistsWithLLVM21";

static SlangResult _queryLLVM21(UnitTestContext* context)
{
    int major = 0;
    int minor = 0;
    SLANG_RETURN_ON_FAIL(context->slangGlobalSession->getDownstreamCompilerVersion(
        SLANG_PASS_THROUGH_LLVM,
        &major,
        &minor));
    return major == 21 && minor == 1 ? SLANG_OK : SLANG_FAIL;
}

static SlangResult _buildCoexistenceProbe(
    const NVVMIRBuilder& builder,
    const UnownedStringSlice& kernelName)
{
    ComPtr<ISlangBlob> assembly;
    ComPtr<ISlangBlob> bitcode;
    SLANG_RETURN_ON_FAIL(_buildEmptyNVVMKernel(builder, kernelName, assembly, bitcode));
    if (!assembly || !bitcode || bitcode->getBufferSize() <= 4)
        return SLANG_FAIL;
    return SLANG_OK;
}

// Runs inside a fully isolated test-server. Unlike slang-test itself, test-server creates a global
// session without eagerly probing every downstream compiler, so the first call here determines the
// actual process load order.
static SlangResult _exerciseNVVMLLVMCoexistence(UnitTestContext* context, NVVMLLVMLoadOrder order)
{
    NVVMIRBuilder builder;
    if (order == NVVMLLVMLoadOrder::NVVMFirst)
    {
        SLANG_RETURN_ON_FAIL(_loadRealNVVMBuilder(context, builder));
        if (builder.getAPI().llvmVersionMajor != 14)
        {
            return SLANG_FAIL;
        }
        SLANG_RETURN_ON_FAIL(
            _buildCoexistenceProbe(builder, toSlice("slangSlice3bNVVMBeforeLLVM")));
    }

    SLANG_RETURN_ON_FAIL(_queryLLVM21(context));

    if (order == NVVMLLVMLoadOrder::LLVMFirst)
    {
        SLANG_RETURN_ON_FAIL(_loadRealNVVMBuilder(context, builder));
        if (builder.getAPI().llvmVersionMajor != 14)
        {
            return SLANG_FAIL;
        }
    }

    SLANG_RETURN_ON_FAIL(_buildCoexistenceProbe(
        builder,
        order == NVVMLLVMLoadOrder::LLVMFirst ? toSlice("slangSlice3bLLVMBeforeNVVM")
                                              : toSlice("slangSlice3bNVVMAfterLLVM")));
    return _queryLLVM21(context);
}

static void _reportCoexistenceChildFailure(
    const char* order,
    SlangResult executeResult,
    const ExecuteResult& childResult)
{
    StringBuilder message;
    message << "Fresh-process NVVM/LLVM coexistence probe failed for " << order << ".\n";
    message << "ProcessUtil result: " << executeResult
            << ", child exit code: " << childResult.resultCode << "\n";
    message << "child stdout:\n" << childResult.standardOutput;
    message << "\nchild stderr:\n" << childResult.standardError;
    getTestReporter()->message(TestMessageType::TestFailure, message.getBuffer());
}

} // namespace
