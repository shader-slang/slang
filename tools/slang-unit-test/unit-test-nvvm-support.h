#pragma once

// Shared implementation support for the decomposed NVVM unit-test translation units.

#include "compiler-core/slang-artifact-representation.h"
#include "compiler-core/slang-artifact-util.h"
#include "compiler-core/slang-downstream-compiler-util.h"
#include "compiler-core/slang-nvrtc-compiler.h"
#include "compiler-core/slang-nvvm-compiler.h"
#include "compiler-core/slang-nvvm-ir-builder.h"
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
struct FakeNVVMBuilderFloatTypeStorage
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
struct FakeNVVMBuilderParameterStorage
{
};
struct FakeNVVMBuilderLoadStorage
{
};
struct FakeNVVMBuilderIntegerConstantStorage
{
};
struct FakeNVVMBuilderIntegerPhiStorage
{
};
struct FakeNVVMBuilderCallStorage
{
};
struct FakeNVVMBuilderPointerOffsetStorage
{
};
struct FakeNVVMBuilderArrayElementPointerStorage
{
};
struct FakeNVVMBuilderScalarOperationStorage
{
};
struct FakeNVVMBuilderRelaxedGlobalI32AtomicAddStorage
{
};
struct FakeNVVMBuilderRawRWStructuredBufferI32TypeStorage
{
};
struct FakeNVVMBuilderRawRWStructuredBufferI32ElementPointerStorage
{
};

enum class FakeNVVMBuilderValueKind
{
    Parameter,
    Load,
    ScalarOperation,
    IntegerConstant,
    IntegerPhi,
    Call,
    PointerOffset,
    ArrayElementPointer,
    RelaxedGlobalI32AtomicAdd,
    RawRWStructuredBufferI32ElementPointer,
};

enum class FakeNVVMBuilderScalarFamily : uint32_t
{
    Unary,
    Binary,
    Compare,
    FloatingBinary,
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
    Index callerBlockIndex = -1;
    FakeNVVMBuilderValueRef operands[2];
    uint32_t operandCount = 0;
};

enum class FakeNVVMBuilderResultTypeKind
{
    Void,
    Integer,
};

enum class FakeNVVMBuilderParameterTypeKind
{
    Integer,
    Pointer,
    Float,
    FloatPointer,
    ArrayPointer,
    RawRWStructuredBufferI32,
};

enum class FakeNVVMBuilderScalarTypeKind
{
    Integer,
    Float,
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
        getPointerTypeCallCount = 0;
        getFunctionParameterCallCount = 0;
        emitLoadCallCount = 0;
        emitStoreCallCount = 0;
        emitBranchCallCount = 0;
        emitConditionalBranchCallCount = 0;
        getIntegerConstantCallCount = 0;
        emitIntegerPhiCallCount = 0;
        addIntegerPhiIncomingCallCount = 0;
        emitIntegerCallCallCount = 0;
        emitIntegerReturnCallCount = 0;
        emitPointerOffsetCallCount = 0;
        emitArrayElementPointerCallCount = 0;
        getRawRWStructuredBufferI32TypeCallCount = 0;
        emitRawRWStructuredBufferI32ElementPointerCallCount = 0;
        emitRelaxedGlobalI32AtomicAddCallCount = 0;
        for (Index family = 0; family < Index(FakeNVVMBuilderScalarFamily::Count); ++family)
        {
            scalarFamilyCallCounts[family] = 0;
            scalarV3FamilyCallCounts[family] = 0;
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
        pointerAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
        pointerPointeeTypes.clear();
        pointerAddressSpaces.clear();
        functionParameterIndex = 0;
        functionParameterCount = 0;
        functionParameterTypeKinds.clear();
        functionTypeResultKinds.clear();
        functionTypeParameterCounts.clear();
        functionTypeParameterKindOffsets.clear();
        functionNames.clear();
        functionTypeIndices.clear();
        blockFunctionIndices.clear();
        loadAlignment = 0;
        storeAlignment = 0;
        scalarOperations.clear();
        scalarV3Operations.clear();
        integerConstantValues.clear();
        integerPhiTargetBlockIndices.clear();
        integerPhiIncomingPhiIndices.clear();
        integerPhiIncomingValueRefs.clear();
        integerPhiIncomingPredecessorBlockIndices.clear();
        functionParameterIndices.clear();
        loadPointerParameterIndices.clear();
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
        integerReturnBlockIndices.clear();
        integerReturnValueRefs.clear();
        pointerOffsetCallerBlockIndices.clear();
        pointerOffsetBaseValueRefs.clear();
        pointerOffsetElementValueRefs.clear();
        arrayElementPointerCallerBlockIndices.clear();
        arrayElementPointerBaseValueRefs.clear();
        arrayElementPointerIndexValueRefs.clear();
        rawRWStructuredBufferI32ElementPointerCallerBlockIndices.clear();
        rawRWStructuredBufferI32ElementPointerBufferValueRefs.clear();
        rawRWStructuredBufferI32ElementPointerIndexValueRefs.clear();
        relaxedGlobalI32AtomicAddCallerBlockIndices.clear();
        relaxedGlobalI32AtomicAddPointerValueRefs.clear();
        relaxedGlobalI32AtomicAddValueRefs.clear();
        loadPointerValueRefs.clear();
        loadResultTypeKinds.clear();
        storePointerValueRefs.clear();
        kernelFunctionIndices.clear();
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
        apiV2 = {};
        apiV3 = {};
        omitAPISymbol = false;
        omitAPIV2Symbol = true;
        omitAPIV3Symbol = true;
        libraryUnavailable = false;
        returnNullModule = false;
        returnNullIntegerType = false;
        returnNullFloatingPointType = false;
        returnNullArrayType = false;
        returnNullArrayElementPointer = false;
        returnNullRawRWStructuredBufferI32Type = false;
        returnNullRawRWStructuredBufferI32ElementPointer = false;
        returnNullScalarOperation = {};
        returnNullRelaxedGlobalI32AtomicAdd = false;
        failIntegerTypeAfterWrite = false;
        failFloatingPointTypeAfterWrite = false;
        failArrayTypeAfterWrite = false;
        failIntegerConstantAfterWrite = false;
        failIntegerPhiAfterWrite = false;
        failIntegerCallAfterWrite = false;
        failIntegerReturn = false;
        failPointerOffsetAfterWrite = false;
        failArrayElementPointerAfterWrite = false;
        failRawRWStructuredBufferI32TypeAfterWrite = false;
        failRawRWStructuredBufferI32ElementPointerAfterWrite = false;
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

    SlangNVVMBuilderAPI_V1 api = {};
    SlangNVVMBuilderAPI_V2 apiV2 = {};
    SlangNVVMBuilderAPI_V3 apiV3 = {};
    bool omitAPISymbol = false;
    bool omitAPIV2Symbol = true;
    bool omitAPIV3Symbol = true;
    bool libraryUnavailable = false;
    bool returnNullModule = false;
    bool returnNullIntegerType = false;
    bool returnNullFloatingPointType = false;
    bool returnNullArrayType = false;
    bool returnNullArrayElementPointer = false;
    bool returnNullRawRWStructuredBufferI32Type = false;
    bool returnNullRawRWStructuredBufferI32ElementPointer = false;
    FakeNVVMBuilderScalarOperationKey returnNullScalarOperation;
    bool returnNullRelaxedGlobalI32AtomicAdd = false;
    bool failIntegerTypeAfterWrite = false;
    bool failFloatingPointTypeAfterWrite = false;
    bool failArrayTypeAfterWrite = false;
    bool reportMismatchedWriteSize = false;
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
    SlangNVVMResult_1 serializationWithDiagnosticsResult = SLANG_OK;
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
    FakeNVVMBuilderBlockStorage blockStorage[16];
    FakeNVVMBuilderIntegerTypeStorage integerTypeStorage;
    FakeNVVMBuilderFloatTypeStorage floatTypeStorage;
    FakeNVVMBuilderPointerTypeStorage pointerTypeStorage;
    FakeNVVMBuilderFloatPointerTypeStorage floatPointerTypeStorage;
    FakeNVVMBuilderArrayTypeStorage arrayTypeStorage;
    FakeNVVMBuilderArrayPointerTypeStorage arrayPointerTypeStorage;
    FakeNVVMBuilderRawRWStructuredBufferI32TypeStorage rawRWStructuredBufferI32TypeStorage;
    FakeNVVMBuilderParameterStorage parameterStorage[64];
    FakeNVVMBuilderLoadStorage loadStorage[16];
    FakeNVVMBuilderScalarOperationStorage scalarOperationStorage[64];
    FakeNVVMBuilderIntegerConstantStorage integerConstantStorage[8];
    FakeNVVMBuilderIntegerPhiStorage integerPhiStorage[8];
    FakeNVVMBuilderCallStorage callStorage[16];
    FakeNVVMBuilderPointerOffsetStorage pointerOffsetStorage[16];
    FakeNVVMBuilderArrayElementPointerStorage arrayElementPointerStorage[16];
    FakeNVVMBuilderRawRWStructuredBufferI32ElementPointerStorage
        rawRWStructuredBufferI32ElementPointerStorage[16];
    FakeNVVMBuilderRelaxedGlobalI32AtomicAddStorage relaxedGlobalI32AtomicAddStorage[16];

    int createModuleCallCount = 0;
    int destroyModuleCallCount = 0;
    int getVoidTypeCallCount = 0;
    int getFunctionTypeCallCount = 0;
    int declareFunctionCallCount = 0;
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
    int getPointerTypeCallCount = 0;
    int getFunctionParameterCallCount = 0;
    int emitLoadCallCount = 0;
    int emitStoreCallCount = 0;
    int emitBranchCallCount = 0;
    int emitConditionalBranchCallCount = 0;
    int getIntegerConstantCallCount = 0;
    int emitIntegerPhiCallCount = 0;
    int addIntegerPhiIncomingCallCount = 0;
    int emitIntegerCallCallCount = 0;
    int emitIntegerReturnCallCount = 0;
    int emitPointerOffsetCallCount = 0;
    int emitArrayElementPointerCallCount = 0;
    int getRawRWStructuredBufferI32TypeCallCount = 0;
    int emitRawRWStructuredBufferI32ElementPointerCallCount = 0;
    int emitRelaxedGlobalI32AtomicAddCallCount = 0;
    int scalarFamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Count)] = {};
    int scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Count)] = {};
    int scalarOperationCallCounts[Index(FakeNVVMBuilderScalarFamily::Count)][8] = {};
    uint32_t integerBitWidth = 0;
    uint32_t floatingPointBitWidth = 0;
    uint32_t arrayElementCount = 0;
    SlangNVVMTypeHandle_1 arrayElementType = nullptr;
    SlangNVVMAddressSpace_2 pointerAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
    List<SlangNVVMTypeHandle_1> pointerPointeeTypes;
    List<SlangNVVMAddressSpace_2> pointerAddressSpaces;
    size_t functionParameterIndex = 0;
    size_t functionParameterCount = 0;
    List<FakeNVVMBuilderParameterTypeKind> functionParameterTypeKinds;
    List<FakeNVVMBuilderResultTypeKind> functionTypeResultKinds;
    List<size_t> functionTypeParameterCounts;
    List<Index> functionTypeParameterKindOffsets;
    List<String> functionNames;
    List<Index> functionTypeIndices;
    List<Index> blockFunctionIndices;
    uint32_t loadAlignment = 0;
    uint32_t storeAlignment = 0;
    List<FakeNVVMBuilderScalarOperation> scalarOperations;
    List<FakeNVVMBuilderScalarOperationKey> scalarV3Operations;
    List<int64_t> integerConstantValues;
    List<Index> integerPhiTargetBlockIndices;
    List<Index> integerPhiIncomingPhiIndices;
    List<FakeNVVMBuilderValueRef> integerPhiIncomingValueRefs;
    List<Index> integerPhiIncomingPredecessorBlockIndices;
    List<size_t> functionParameterIndices;
    List<size_t> loadPointerParameterIndices;
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
    List<Index> integerReturnBlockIndices;
    List<FakeNVVMBuilderValueRef> integerReturnValueRefs;
    List<Index> pointerOffsetCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> pointerOffsetBaseValueRefs;
    List<FakeNVVMBuilderValueRef> pointerOffsetElementValueRefs;
    List<Index> arrayElementPointerCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> arrayElementPointerBaseValueRefs;
    List<FakeNVVMBuilderValueRef> arrayElementPointerIndexValueRefs;
    List<Index> rawRWStructuredBufferI32ElementPointerCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> rawRWStructuredBufferI32ElementPointerBufferValueRefs;
    List<FakeNVVMBuilderValueRef> rawRWStructuredBufferI32ElementPointerIndexValueRefs;
    List<Index> relaxedGlobalI32AtomicAddCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> relaxedGlobalI32AtomicAddPointerValueRefs;
    List<FakeNVVMBuilderValueRef> relaxedGlobalI32AtomicAddValueRefs;
    List<FakeNVVMBuilderValueRef> loadPointerValueRefs;
    List<FakeNVVMBuilderScalarTypeKind> loadResultTypeKinds;
    List<FakeNVVMBuilderValueRef> storePointerValueRefs;
    List<Index> kernelFunctionIndices;
    Index currentInsertBlockIndex = -1;
    Index conditionalSourceBlockIndex = -1;
    Index conditionalTrueBlockIndex = -1;
    Index conditionalFalseBlockIndex = -1;
    String moduleName;
    String functionName;
    String blockName;
    bool failIntegerConstantAfterWrite = false;
    bool failIntegerPhiAfterWrite = false;
    bool failIntegerCallAfterWrite = false;
    bool failIntegerReturn = false;
    bool failPointerOffsetAfterWrite = false;
    bool failArrayElementPointerAfterWrite = false;
    bool failRawRWStructuredBufferI32TypeAfterWrite = false;
    bool failRawRWStructuredBufferI32ElementPointerAfterWrite = false;
    FakeNVVMBuilderScalarOperationKey failScalarOperationAfterWrite;
    bool failRelaxedGlobalI32AtomicAddAfterWrite = false;
};

FakeNVVMBuilderState gFakeNVVMBuilder;

static SlangNVVMModuleHandle_1 _getFakeNVVMBuilderModule()
{
    return reinterpret_cast<SlangNVVMModuleHandle_1>(&gFakeNVVMBuilder.moduleStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderVoidType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.voidTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderFunctionType(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.functionTypeStorage));
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.functionTypeStorage[index]);
}

static bool _getFakeNVVMBuilderFunctionTypeIndex(SlangNVVMTypeHandle_1 type, Index& outIndex)
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderFunction(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.functionStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.functionStorage[index]);
}

static bool _getFakeNVVMBuilderFunctionIndex(SlangNVVMValueHandle_1 function, Index& outIndex)
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

static SlangNVVMBlockHandle_1 _getFakeNVVMBuilderBlock(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.blockStorage));
    return reinterpret_cast<SlangNVVMBlockHandle_1>(&gFakeNVVMBuilder.blockStorage[index]);
}

static bool _getFakeNVVMBuilderBlockIndex(SlangNVVMBlockHandle_1 block, Index& outIndex)
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
    SlangNVVMBlockHandle_1 block,
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

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderIntegerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.integerTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderFloatType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.floatTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.pointerTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderFloatPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.floatPointerTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderArrayType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.arrayTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderArrayPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.arrayPointerTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderRawRWStructuredBufferI32Type()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(
        &gFakeNVVMBuilder.rawRWStructuredBufferI32TypeStorage);
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderFunctionParameter(
    Index functionIndex,
    Index parameterIndex)
{
    const Index storageIndex = functionIndex * 8 + parameterIndex;
    SLANG_ASSERT(functionIndex >= 0 && functionIndex < 8);
    SLANG_ASSERT(parameterIndex >= 0 && parameterIndex < 8);
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.parameterStorage[storageIndex]);
}

// Provides the original single-function test view through the canonical function/parameter map.
static SlangNVVMValueHandle_1 _getFakeNVVMBuilderParameter(Index index = 0)
{
    return _getFakeNVVMBuilderFunctionParameter(0, index);
}

static bool _getFakeNVVMBuilderParameterRef(
    SlangNVVMValueHandle_1 value,
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderLoad(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.loadStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.loadStorage[index]);
}

static bool _getFakeNVVMBuilderLoadIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderScalarOperation(Index index)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.scalarOperationStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.scalarOperationStorage[index]);
}

static bool _getFakeNVVMBuilderScalarOperationIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderIntegerConstant(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.integerConstantStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.integerConstantStorage[index]);
}

static bool _getFakeNVVMBuilderIntegerConstantIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderIntegerPhi(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.integerPhiStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.integerPhiStorage[index]);
}

static bool _getFakeNVVMBuilderIntegerPhiIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.integerPhiTargetBlockIndices.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderIntegerPhi(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderCall(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.callStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.callStorage[index]);
}

static bool _getFakeNVVMBuilderCallIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderPointerOffset(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.pointerOffsetStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.pointerOffsetStorage[index]);
}

static bool _getFakeNVVMBuilderPointerOffsetIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderArrayElementPointer(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.arrayElementPointerStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.arrayElementPointerStorage[index]);
}

static bool _getFakeNVVMBuilderArrayElementPointerIndex(
    SlangNVVMValueHandle_1 value,
    Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderArrayElementPointer(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderRawRWStructuredBufferI32ElementPointer(
    Index index = 0)
{
    SLANG_ASSERT(
        index >= 0 &&
        index < SLANG_COUNT_OF(gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerStorage[index]);
}

static bool _getFakeNVVMBuilderRawRWStructuredBufferI32ElementPointerIndex(
    SlangNVVMValueHandle_1 value,
    Index& outIndex)
{
    for (Index i = 0;
         i < gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs.getCount();
         ++i)
    {
        if (value == _getFakeNVVMBuilderRawRWStructuredBufferI32ElementPointer(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderRelaxedGlobalI32AtomicAdd(Index index = 0)
{
    SLANG_ASSERT(
        index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.relaxedGlobalI32AtomicAddStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.relaxedGlobalI32AtomicAddStorage[index]);
}

static bool _getFakeNVVMBuilderRelaxedGlobalI32AtomicAddIndex(
    SlangNVVMValueHandle_1 value,
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

static bool _getFakeNVVMBuilderValueRef(
    SlangNVVMValueHandle_1 value,
    FakeNVVMBuilderValueRef& outRef)
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
    if (_getFakeNVVMBuilderIntegerConstantIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::IntegerConstant, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderIntegerPhiIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::IntegerPhi, valueIndex};
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
    if (_getFakeNVVMBuilderArrayElementPointerIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::ArrayElementPointer, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderRawRWStructuredBufferI32ElementPointerIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::RawRWStructuredBufferI32ElementPointer, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderRelaxedGlobalI32AtomicAddIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::RelaxedGlobalI32AtomicAdd, valueIndex};
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
    for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
    {
        const FakeNVVMBuilderValueRef valueRef = gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
        if (gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == phiIndex &&
            valueRef.kind == valueKind && valueRef.index == valueIndex &&
            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i] == predecessorBlockIndex)
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

static bool _isFakeNVVMBuilderIntegerValue(SlangNVVMValueHandle_1 value)
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
    case FakeNVVMBuilderValueKind::IntegerPhi:
    case FakeNVVMBuilderValueKind::Call:
    case FakeNVVMBuilderValueKind::RelaxedGlobalI32AtomicAdd:
        return true;
    case FakeNVVMBuilderValueKind::ScalarOperation:
        return gFakeNVVMBuilder.scalarOperations[valueRef.index].key.family ==
                   FakeNVVMBuilderScalarFamily::Unary ||
               gFakeNVVMBuilder.scalarOperations[valueRef.index].key.family ==
                   FakeNVVMBuilderScalarFamily::Binary;
    case FakeNVVMBuilderValueKind::PointerOffset:
    case FakeNVVMBuilderValueKind::ArrayElementPointer:
    case FakeNVVMBuilderValueKind::RawRWStructuredBufferI32ElementPointer:
        return false;
    }
    return false;
}

static bool _isFakeNVVMBuilderFloatValue(SlangNVVMValueHandle_1 value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    if (valueRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        FakeNVVMBuilderParameterTypeKind parameterTypeKind;
        return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
               parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Float;
    }
    if (valueRef.kind == FakeNVVMBuilderValueKind::Load)
    {
        return valueRef.index >= 0 &&
               valueRef.index < gFakeNVVMBuilder.loadResultTypeKinds.getCount() &&
               gFakeNVVMBuilder.loadResultTypeKinds[valueRef.index] ==
                   FakeNVVMBuilderScalarTypeKind::Float;
    }
    return valueRef.kind == FakeNVVMBuilderValueKind::ScalarOperation &&
           gFakeNVVMBuilder.scalarOperations[valueRef.index].key.family ==
               FakeNVVMBuilderScalarFamily::FloatingBinary;
}

static bool _isFakeNVVMBuilderBooleanValue(SlangNVVMValueHandle_1 value)
{
    Index operationIndex = -1;
    return _getFakeNVVMBuilderScalarOperationIndex(value, operationIndex) &&
           gFakeNVVMBuilder.scalarOperations[operationIndex].key.family ==
               FakeNVVMBuilderScalarFamily::Compare;
}

static bool _isFakeNVVMBuilderPointerValue(SlangNVVMValueHandle_1 value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    if (valueRef.kind == FakeNVVMBuilderValueKind::PointerOffset ||
        valueRef.kind == FakeNVVMBuilderValueKind::ArrayElementPointer ||
        valueRef.kind == FakeNVVMBuilderValueKind::RawRWStructuredBufferI32ElementPointer)
        return true;
    FakeNVVMBuilderParameterTypeKind parameterTypeKind;
    return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
           (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Pointer ||
            parameterTypeKind == FakeNVVMBuilderParameterTypeKind::FloatPointer);
}

static bool _isFakeNVVMBuilderArrayPointerValue(SlangNVVMValueHandle_1 value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    FakeNVVMBuilderParameterTypeKind parameterTypeKind;
    return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
           parameterTypeKind == FakeNVVMBuilderParameterTypeKind::ArrayPointer;
}

static bool _isFakeNVVMBuilderRawRWStructuredBufferI32Value(SlangNVVMValueHandle_1 value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    FakeNVVMBuilderParameterTypeKind parameterTypeKind;
    return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
           parameterTypeKind == FakeNVVMBuilderParameterTypeKind::RawRWStructuredBufferI32;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderCreateModule(
    const char* moduleName,
    size_t moduleNameSize,
    SlangNVVMModuleHandle_1* outModule)
{
    ++gFakeNVVMBuilder.createModuleCallCount;
    if ((!moduleName && moduleNameSize) || !outModule)
        return SLANG_E_INVALID_ARG;
    gFakeNVVMBuilder.moduleName = String(UnownedStringSlice(moduleName, moduleNameSize));
    *outModule = gFakeNVVMBuilder.returnNullModule ? nullptr : _getFakeNVVMBuilderModule();
    return SLANG_OK;
}

static void SLANG_NVVM_CALL _fakeNVVMBuilderDestroyModule(SlangNVVMModuleHandle_1 module)
{
    SLANG_ASSERT(module == _getFakeNVVMBuilderModule());
    ++gFakeNVVMBuilder.destroyModuleCallCount;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderGetVoidType(SlangNVVMModuleHandle_1 module, SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getVoidTypeCallCount;
    if (module != _getFakeNVVMBuilderModule() || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = _getFakeNVVMBuilderVoidType();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFunctionType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 resultType,
    const SlangNVVMTypeHandle_1* parameterTypes,
    size_t parameterCount,
    SlangNVVMTypeHandle_1* outType)
{
    const Index functionTypeIndex = gFakeNVVMBuilder.getFunctionTypeCallCount++;
    const bool hasSupportedResult = resultType == _getFakeNVVMBuilderVoidType() ||
                                    resultType == _getFakeNVVMBuilderIntegerType();
    if (module != _getFakeNVVMBuilderModule() || !hasSupportedResult ||
        (!parameterTypes && parameterCount) || !outType ||
        functionTypeIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.functionTypeStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.functionTypeResultKinds.add(
        resultType == _getFakeNVVMBuilderVoidType() ? FakeNVVMBuilderResultTypeKind::Void
                                                    : FakeNVVMBuilderResultTypeKind::Integer);
    gFakeNVVMBuilder.functionTypeParameterCounts.add(parameterCount);
    gFakeNVVMBuilder.functionTypeParameterKindOffsets.add(
        gFakeNVVMBuilder.functionParameterTypeKinds.getCount());
    for (size_t i = 0; i < parameterCount; ++i)
    {
        if (parameterTypes[i] != _getFakeNVVMBuilderIntegerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderFloatType() &&
            parameterTypes[i] != _getFakeNVVMBuilderPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderFloatPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderArrayPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderRawRWStructuredBufferI32Type())
        {
            return SLANG_E_INVALID_ARG;
        }
    }
    for (size_t i = 0; i < parameterCount; ++i)
    {
        const FakeNVVMBuilderParameterTypeKind parameterTypeKind =
            parameterTypes[i] == _getFakeNVVMBuilderIntegerType()
                ? FakeNVVMBuilderParameterTypeKind::Integer
            : parameterTypes[i] == _getFakeNVVMBuilderFloatType()
                ? FakeNVVMBuilderParameterTypeKind::Float
            : parameterTypes[i] == _getFakeNVVMBuilderPointerType()
                ? FakeNVVMBuilderParameterTypeKind::Pointer
            : parameterTypes[i] == _getFakeNVVMBuilderFloatPointerType()
                ? FakeNVVMBuilderParameterTypeKind::FloatPointer
            : parameterTypes[i] == _getFakeNVVMBuilderArrayPointerType()
                ? FakeNVVMBuilderParameterTypeKind::ArrayPointer
                : FakeNVVMBuilderParameterTypeKind::RawRWStructuredBufferI32;
        gFakeNVVMBuilder.functionParameterTypeKinds.add(parameterTypeKind);
    }
    gFakeNVVMBuilder.functionParameterCount = parameterCount;
    *outType = _getFakeNVVMBuilderFunctionType(functionTypeIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderDeclareFunction(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 functionType,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle_1* outFunction)
{
    const Index functionIndex = gFakeNVVMBuilder.declareFunctionCallCount++;
    Index functionTypeIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderFunctionTypeIndex(functionType, functionTypeIndex) ||
        functionTypeIndex >= gFakeNVVMBuilder.functionTypeResultKinds.getCount() ||
        (!name && nameSize) || !outFunction ||
        functionIndex >= SLANG_COUNT_OF(gFakeNVVMBuilder.functionStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.functionName = String(UnownedStringSlice(name, nameSize));
    gFakeNVVMBuilder.functionNames.add(gFakeNVVMBuilder.functionName);
    gFakeNVVMBuilder.functionTypeIndices.add(functionTypeIndex);
    *outFunction = _getFakeNVVMBuilderFunction(functionIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderCreateBlock(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    const char* name,
    size_t nameSize,
    SlangNVVMBlockHandle_1* outBlock)
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
_fakeNVVMBuilderSetInsertBlock(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 block)
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitReturnVoid(SlangNVVMModuleHandle_1 module)
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderMarkFunctionAsKernel(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus_2* outVerificationStatus)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus_2* outVerificationStatus)
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
    SlangNVVMModuleHandle_1 module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getIntegerTypeCallCount;
    gFakeNVVMBuilder.integerBitWidth = bitWidth;
    if (module != _getFakeNVVMBuilderModule() || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = gFakeNVVMBuilder.returnNullIntegerType ? nullptr : _getFakeNVVMBuilderIntegerType();
    return gFakeNVVMBuilder.failIntegerTypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFloatingPointType(
    SlangNVVMModuleHandle_1 module,
    uint32_t bitWidth,
    SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getFloatingPointTypeCallCount;
    gFakeNVVMBuilder.floatingPointBitWidth = bitWidth;
    if (outType)
        *outType = nullptr;
    if (module != _getFakeNVVMBuilderModule() || bitWidth != 32 || !outType)
        return SLANG_E_INVALID_ARG;
    *outType =
        gFakeNVVMBuilder.returnNullFloatingPointType ? nullptr : _getFakeNVVMBuilderFloatType();
    return gFakeNVVMBuilder.failFloatingPointTypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetRawRWStructuredBufferI32Type(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getRawRWStructuredBufferI32TypeCallCount;
    if (outType)
        *outType = nullptr;
    if (module != _getFakeNVVMBuilderModule() || !outType)
        return SLANG_E_INVALID_ARG;
    *outType = gFakeNVVMBuilder.returnNullRawRWStructuredBufferI32Type
                   ? nullptr
                   : _getFakeNVVMBuilderRawRWStructuredBufferI32Type();
    return gFakeNVVMBuilder.failRawRWStructuredBufferI32TypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetArrayType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 elementType,
    uint32_t elementCount,
    SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getArrayTypeCallCount;
    gFakeNVVMBuilder.arrayElementType = elementType;
    gFakeNVVMBuilder.arrayElementCount = elementCount;
    if (outType)
        *outType = nullptr;
    if (module != _getFakeNVVMBuilderModule() || elementType != _getFakeNVVMBuilderIntegerType() ||
        elementCount == 0 || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    *outType = gFakeNVVMBuilder.returnNullArrayType ? nullptr : _getFakeNVVMBuilderArrayType();
    return gFakeNVVMBuilder.failArrayTypeAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetPointerType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 pointeeType,
    SlangNVVMAddressSpace_2 addressSpace,
    SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getPointerTypeCallCount;
    gFakeNVVMBuilder.pointerAddressSpace = addressSpace;
    if (module != _getFakeNVVMBuilderModule() ||
        (pointeeType != _getFakeNVVMBuilderIntegerType() &&
         pointeeType != _getFakeNVVMBuilderFloatType() &&
         pointeeType != _getFakeNVVMBuilderArrayType()) ||
        !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.pointerPointeeTypes.add(pointeeType);
    gFakeNVVMBuilder.pointerAddressSpaces.add(addressSpace);
    *outType = pointeeType == _getFakeNVVMBuilderIntegerType() ? _getFakeNVVMBuilderPointerType()
               : pointeeType == _getFakeNVVMBuilderFloatType()
                   ? _getFakeNVVMBuilderFloatPointerType()
                   : _getFakeNVVMBuilderArrayPointerType();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetFunctionParameter(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    size_t parameterIndex,
    SlangNVVMValueHandle_1* outValue)
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitLoad(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 pointer,
    uint32_t alignment,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitLoadCallCount;
    gFakeNVVMBuilder.loadAlignment = alignment;
    Index pointerFunctionIndex = -1;
    size_t pointerIndex = size_t(-1);
    FakeNVVMBuilderValueRef pointerRef;
    if (module != _getFakeNVVMBuilderModule() || !_isFakeNVVMBuilderPointerValue(pointer) ||
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef) || !outValue ||
        gFakeNVVMBuilder.loadResultTypeKinds.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.loadStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    FakeNVVMBuilderScalarTypeKind resultTypeKind = FakeNVVMBuilderScalarTypeKind::Integer;
    if (pointerRef.kind == FakeNVVMBuilderValueKind::Parameter)
    {
        FakeNVVMBuilderParameterTypeKind parameterTypeKind;
        if (!_getFakeNVVMBuilderParameterTypeKind(pointerRef, parameterTypeKind))
            return SLANG_E_INVALID_ARG;
        if (parameterTypeKind == FakeNVVMBuilderParameterTypeKind::FloatPointer)
            resultTypeKind = FakeNVVMBuilderScalarTypeKind::Float;
        else if (parameterTypeKind != FakeNVVMBuilderParameterTypeKind::Pointer)
            return SLANG_E_INVALID_ARG;
    }
    else if (
        pointerRef.kind != FakeNVVMBuilderValueKind::PointerOffset &&
        pointerRef.kind != FakeNVVMBuilderValueKind::ArrayElementPointer &&
        pointerRef.kind != FakeNVVMBuilderValueKind::RawRWStructuredBufferI32ElementPointer)
    {
        return SLANG_E_INVALID_ARG;
    }

    _getFakeNVVMBuilderParameterRef(pointer, pointerFunctionIndex, pointerIndex);
    const Index resultIndex = gFakeNVVMBuilder.loadResultTypeKinds.getCount();
    gFakeNVVMBuilder.loadPointerParameterIndices.add(pointerIndex);
    gFakeNVVMBuilder.loadPointerValueRefs.add(pointerRef);
    gFakeNVVMBuilder.loadResultTypeKinds.add(resultTypeKind);
    *outValue = _getFakeNVVMBuilderLoad(resultIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitStore(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1 pointer,
    uint32_t alignment)
{
    ++gFakeNVVMBuilder.emitStoreCallCount;
    gFakeNVVMBuilder.storeAlignment = alignment;
    Index pointerFunctionIndex = -1;
    size_t pointerIndex = size_t(-1);
    FakeNVVMBuilderValueRef pointerRef;
    FakeNVVMBuilderValueRef valueRef;
    if (module != _getFakeNVVMBuilderModule() ||
        (!_isFakeNVVMBuilderIntegerValue(value) && !_isFakeNVVMBuilderFloatValue(value)) ||
        !_getFakeNVVMBuilderValueRef(value, valueRef) || !_isFakeNVVMBuilderPointerValue(pointer) ||
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef))
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

static SlangResult _recordFakeNVVMBuilderScalarOperation(
    SlangNVVMModuleHandle_1 module,
    FakeNVVMBuilderScalarOperationKey key,
    const SlangNVVMValueHandle_1* operands,
    uint32_t operandCount,
    SlangNVVMValueHandle_1* outValue)
{
    SLANG_ASSERT(key.family < FakeNVVMBuilderScalarFamily::Count);
    SLANG_ASSERT(key.operation < SLANG_COUNT_OF(gFakeNVVMBuilder.scalarOperationCallCounts[0]));
    ++gFakeNVVMBuilder.scalarFamilyCallCounts[Index(key.family)];
    ++gFakeNVVMBuilder.scalarOperationCallCounts[Index(key.family)][key.operation];
    if (outValue)
        *outValue = nullptr;

    FakeNVVMBuilderScalarOperation recorded = {};
    recorded.key = key;
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
        const bool isFloating = key.family == FakeNVVMBuilderScalarFamily::FloatingBinary;
        if ((isFloating ? !_isFakeNVVMBuilderFloatValue(operands[i])
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBinary(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_2 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    uint32_t operationV3 = 0;
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_BINARY_OP_ADD:
        operationV3 = SLANG_NVVM_INTEGER_BINARY_OP_3_ADD;
        break;
    case SLANG_NVVM_INTEGER_BINARY_OP_SUB:
        operationV3 = SLANG_NVVM_INTEGER_BINARY_OP_3_SUB;
        break;
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
    const SlangNVVMValueHandle_1 operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Binary, operationV3},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedLessThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    const SlangNVVMValueHandle_1 operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderEmitBranch(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 targetBlock)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 condition,
    SlangNVVMBlockHandle_1 trueBlock,
    SlangNVVMBlockHandle_1 falseBlock)
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetIntegerConstant(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 type,
    int64_t value,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.getIntegerConstantCallCount;
    if (module != _getFakeNVVMBuilderModule() || type != _getFakeNVVMBuilderIntegerType() ||
        !outValue ||
        gFakeNVVMBuilder.integerConstantValues.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.integerConstantStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.integerConstantValues.getCount();
    gFakeNVVMBuilder.integerConstantValues.add(value);
    *outValue = _getFakeNVVMBuilderIntegerConstant(resultIndex);
    return gFakeNVVMBuilder.failIntegerConstantAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerPhi(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMBlockHandle_1 targetBlock,
    SlangNVVMTypeHandle_1 type,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerPhiCallCount;
    Index targetIndex = -1;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderBlockIndex(targetBlock, targetIndex) ||
        type != _getFakeNVVMBuilderIntegerType() || !outValue ||
        gFakeNVVMBuilder.integerPhiTargetBlockIndices.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.integerPhiStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.integerPhiTargetBlockIndices.getCount();
    gFakeNVVMBuilder.integerPhiTargetBlockIndices.add(targetIndex);
    *outValue = _getFakeNVVMBuilderIntegerPhi(resultIndex);
    return gFakeNVVMBuilder.failIntegerPhiAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderAddIntegerPhiIncoming(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 phi,
    SlangNVVMValueHandle_1 value,
    SlangNVVMBlockHandle_1 predecessorBlock)
{
    ++gFakeNVVMBuilder.addIntegerPhiIncomingCallCount;
    Index phiIndex = -1;
    Index predecessorIndex = -1;
    FakeNVVMBuilderValueRef valueRef;
    if (module != _getFakeNVVMBuilderModule() ||
        !_getFakeNVVMBuilderIntegerPhiIndex(phi, phiIndex) ||
        !_getFakeNVVMBuilderValueRef(value, valueRef) ||
        !_getFakeNVVMBuilderBlockIndex(predecessorBlock, predecessorIndex))
    {
        return SLANG_E_INVALID_ARG;
    }

    gFakeNVVMBuilder.integerPhiIncomingPhiIndices.add(phiIndex);
    gFakeNVVMBuilder.integerPhiIncomingValueRefs.add(valueRef);
    gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices.add(predecessorIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerCall(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 callee,
    const SlangNVVMValueHandle_1* arguments,
    size_t argumentCount,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerCallCallCount;
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
    if (gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] !=
            FakeNVVMBuilderResultTypeKind::Integer ||
        gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] != argumentCount)
    {
        return SLANG_E_INVALID_ARG;
    }

    List<FakeNVVMBuilderValueRef> argumentRefs;
    for (size_t i = 0; i < argumentCount; ++i)
    {
        FakeNVVMBuilderValueRef argumentRef;
        if (!_getFakeNVVMBuilderValueRef(arguments[i], argumentRef))
            return SLANG_E_INVALID_ARG;
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
    *outValue = _getFakeNVVMBuilderCall(resultIndex);
    return gFakeNVVMBuilder.failIntegerCallAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderEmitIntegerReturn(SlangNVVMModuleHandle_1 module, SlangNVVMValueHandle_1 value)
{
    ++gFakeNVVMBuilder.emitIntegerReturnCallCount;
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
    if (gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] !=
        FakeNVVMBuilderResultTypeKind::Integer)
    {
        return SLANG_E_INVALID_ARG;
    }

    if (gFakeNVVMBuilder.failIntegerReturn)
        return SLANG_FAIL;
    gFakeNVVMBuilder.integerReturnBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.integerReturnValueRefs.add(valueRef);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitPointerOffset(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 basePointer,
    SlangNVVMValueHandle_1 elementOffset,
    SlangNVVMValueHandle_1* outPointer)
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitArrayElementPointer(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 baseArrayPointer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1* outPointer)
{
    ++gFakeNVVMBuilder.emitArrayElementPointerCallCount;
    if (outPointer)
        *outPointer = nullptr;

    FakeNVVMBuilderValueRef baseRef;
    FakeNVVMBuilderValueRef indexRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderArrayPointerValue(baseArrayPointer) ||
        !_getFakeNVVMBuilderValueRef(baseArrayPointer, baseRef) ||
        !_isFakeNVVMBuilderIntegerValue(elementIndex) ||
        !_getFakeNVVMBuilderValueRef(elementIndex, indexRef) || !outPointer ||
        gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.arrayElementPointerStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount();
    gFakeNVVMBuilder.arrayElementPointerCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.add(baseRef);
    gFakeNVVMBuilder.arrayElementPointerIndexValueRefs.add(indexRef);
    *outPointer = gFakeNVVMBuilder.returnNullArrayElementPointer
                      ? nullptr
                      : _getFakeNVVMBuilderArrayElementPointer(resultIndex);
    return gFakeNVVMBuilder.failArrayElementPointerAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitRawRWStructuredBufferI32ElementPointer(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 buffer,
    SlangNVVMValueHandle_1 elementIndex,
    SlangNVVMValueHandle_1* outPointer)
{
    ++gFakeNVVMBuilder.emitRawRWStructuredBufferI32ElementPointerCallCount;
    if (outPointer)
        *outPointer = nullptr;

    FakeNVVMBuilderValueRef bufferRef;
    FakeNVVMBuilderValueRef indexRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderRawRWStructuredBufferI32Value(buffer) ||
        !_getFakeNVVMBuilderValueRef(buffer, bufferRef) ||
        !_isFakeNVVMBuilderIntegerValue(elementIndex) ||
        !_getFakeNVVMBuilderValueRef(elementIndex, indexRef) || !outPointer ||
        gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex =
        gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs.getCount();
    gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerBufferValueRefs.add(bufferRef);
    gFakeNVVMBuilder.rawRWStructuredBufferI32ElementPointerIndexValueRefs.add(indexRef);
    *outPointer = gFakeNVVMBuilder.returnNullRawRWStructuredBufferI32ElementPointer
                      ? nullptr
                      : _getFakeNVVMBuilderRawRWStructuredBufferI32ElementPointer(resultIndex);
    return gFakeNVVMBuilder.failRawRWStructuredBufferI32ElementPointerAfterWrite ? SLANG_FAIL
                                                                                 : SLANG_OK;
}

static SlangResult _recordFakeNVVMBuilderUnaryOperation(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerUnaryOp_3 operation,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
{
    const SlangNVVMValueHandle_1 operands[] = {value};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Unary, operation},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult _recordFakeNVVMBuilderBinaryOperation(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    const SlangNVVMValueHandle_1 operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Binary, operation},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerMultiply(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitAnd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitOr(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitXor(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderBinaryOperation(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitNot(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderUnaryOperation(
        module,
        SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT,
        value,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerNegate(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderUnaryOperation(
        module,
        SLANG_NVVM_INTEGER_UNARY_OP_NEGATE,
        value,
        outValue);
}
static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitRelaxedGlobalI32AtomicAdd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 pointer,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outOldValue)
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
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerCompareOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    const SlangNVVMValueHandle_1 operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::Compare, operation},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerNotEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedGreaterThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedLessEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL,
        left,
        right,
        outValue);
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedGreaterEqual(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    return _recordFakeNVVMBuilderCompareOperation(
        module,
        SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL,
        left,
        right,
        outValue);
}
static SlangNVVMBuilderAPI_V1 _makeFakeNVVMBuilderAPI()
{
    SlangNVVMBuilderAPI_V1 api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_1;
    api.llvmVersionMajor = 14;
    api.llvmVersionMinor = 0;
    api.llvmVersionPatch = 6;
    api.nvvmIRVersionMajor = 2;
    api.nvvmIRVersionMinor = 0;
    api.pointerModel = SLANG_NVVM_POINTER_MODEL_TYPED;
    api.createModule = _fakeNVVMBuilderCreateModule;
    api.destroyModule = _fakeNVVMBuilderDestroyModule;
    api.getVoidType = _fakeNVVMBuilderGetVoidType;
    api.getFunctionType = _fakeNVVMBuilderGetFunctionType;
    api.declareFunction = _fakeNVVMBuilderDeclareFunction;
    api.createBlock = _fakeNVVMBuilderCreateBlock;
    api.setInsertBlock = _fakeNVVMBuilderSetInsertBlock;
    api.emitReturnVoid = _fakeNVVMBuilderEmitReturnVoid;
    api.markFunctionAsKernel = _fakeNVVMBuilderMarkFunctionAsKernel;
    api.serializeModule = _fakeNVVMBuilderSerializeModule;
    return api;
}

static SlangNVVMBuilderAPI_V2 _makeFakeNVVMBuilderAPIV2()
{
    SlangNVVMBuilderAPI_V2 api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_2;
    api.baseAPI = _makeFakeNVVMBuilderAPI();
    api.serializeModuleWithDiagnostics = _fakeNVVMBuilderSerializeModuleWithDiagnostics;
    api.getIntegerType = _fakeNVVMBuilderGetIntegerType;
    api.getPointerType = _fakeNVVMBuilderGetPointerType;
    api.getFunctionParameter = _fakeNVVMBuilderGetFunctionParameter;
    api.emitLoad = _fakeNVVMBuilderEmitLoad;
    api.emitStore = _fakeNVVMBuilderEmitStore;
    api.emitIntegerBinary = _fakeNVVMBuilderEmitIntegerBinary;
    api.emitIntegerSignedLessThan = _fakeNVVMBuilderEmitIntegerSignedLessThan;
    api.emitBranch = _fakeNVVMBuilderEmitBranch;
    api.emitConditionalBranch = _fakeNVVMBuilderEmitConditionalBranch;
    api.getIntegerConstant = _fakeNVVMBuilderGetIntegerConstant;
    api.emitIntegerPhi = _fakeNVVMBuilderEmitIntegerPhi;
    api.addIntegerPhiIncoming = _fakeNVVMBuilderAddIntegerPhiIncoming;
    api.emitIntegerCall = _fakeNVVMBuilderEmitIntegerCall;
    api.emitIntegerReturn = _fakeNVVMBuilderEmitIntegerReturn;
    api.emitPointerOffset = _fakeNVVMBuilderEmitPointerOffset;
    api.getArrayType = _fakeNVVMBuilderGetArrayType;
    api.emitArrayElementPointer = _fakeNVVMBuilderEmitArrayElementPointer;
    api.emitIntegerMultiply = _fakeNVVMBuilderEmitIntegerMultiply;
    api.emitIntegerBitAnd = _fakeNVVMBuilderEmitIntegerBitAnd;
    api.emitIntegerBitOr = _fakeNVVMBuilderEmitIntegerBitOr;
    api.emitIntegerBitXor = _fakeNVVMBuilderEmitIntegerBitXor;
    api.emitIntegerBitNot = _fakeNVVMBuilderEmitIntegerBitNot;
    api.emitIntegerNegate = _fakeNVVMBuilderEmitIntegerNegate;
    api.emitRelaxedGlobalI32AtomicAdd = _fakeNVVMBuilderEmitRelaxedGlobalI32AtomicAdd;
    api.serializeNVVMIR20AssemblyWithDiagnostics =
        _fakeNVVMBuilderSerializeNVVMIR20AssemblyWithDiagnostics;
    api.emitIntegerEqual = _fakeNVVMBuilderEmitIntegerEqual;
    api.emitIntegerNotEqual = _fakeNVVMBuilderEmitIntegerNotEqual;
    api.emitIntegerSignedGreaterThan = _fakeNVVMBuilderEmitIntegerSignedGreaterThan;
    api.emitIntegerSignedLessEqual = _fakeNVVMBuilderEmitIntegerSignedLessEqual;
    api.emitIntegerSignedGreaterEqual = _fakeNVVMBuilderEmitIntegerSignedGreaterEqual;
    api.getRawRWStructuredBufferI32Type = _fakeNVVMBuilderGetRawRWStructuredBufferI32Type;
    api.emitRawRWStructuredBufferI32ElementPointer =
        _fakeNVVMBuilderEmitRawRWStructuredBufferI32ElementPointer;
    return api;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerUnaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerUnaryOp_3 operation,
    SlangNVVMValueHandle_1 value,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Unary)];
    gFakeNVVMBuilder.scalarV3Operations.add(
        {FakeNVVMBuilderScalarFamily::Unary, uint32_t(operation)});
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT:
        return _recordFakeNVVMBuilderUnaryOperation(module, operation, value, outValue);
    case SLANG_NVVM_INTEGER_UNARY_OP_NEGATE:
        return _recordFakeNVVMBuilderUnaryOperation(module, operation, value, outValue);
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBinaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Binary)];
    gFakeNVVMBuilder.scalarV3Operations.add(
        {FakeNVVMBuilderScalarFamily::Binary, uint32_t(operation)});
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_BINARY_OP_3_ADD:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_SUB:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR:
        return _recordFakeNVVMBuilderBinaryOperation(module, operation, left, right, outValue);
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerCompareV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerCompareOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::Compare)];
    gFakeNVVMBuilder.scalarV3Operations.add(
        {FakeNVVMBuilderScalarFamily::Compare, uint32_t(operation)});
    switch (operation)
    {
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_THAN:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    case SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL:
        return _recordFakeNVVMBuilderCompareOperation(module, operation, left, right, outValue);
    default:
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitFloatingBinaryV3(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMFloatingBinaryOp_3 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.scalarV3FamilyCallCounts[Index(FakeNVVMBuilderScalarFamily::FloatingBinary)];
    gFakeNVVMBuilder.scalarV3Operations.add(
        {FakeNVVMBuilderScalarFamily::FloatingBinary, uint32_t(operation)});
    if (operation != SLANG_NVVM_FLOATING_BINARY_OP_ADD &&
        operation != SLANG_NVVM_FLOATING_BINARY_OP_SUBTRACT)
    {
        if (outValue)
            *outValue = nullptr;
        return SLANG_E_INVALID_ARG;
    }
    const SlangNVVMValueHandle_1 operands[] = {left, right};
    return _recordFakeNVVMBuilderScalarOperation(
        module,
        {FakeNVVMBuilderScalarFamily::FloatingBinary, uint32_t(operation)},
        operands,
        SLANG_COUNT_OF(operands),
        outValue);
}

static SlangNVVMBuilderAPI_V3 _makeFakeNVVMBuilderAPIV3()
{
    SlangNVVMBuilderAPI_V3 api = {};
    api.structureSize = uint32_t(sizeof(api));
    api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_3;
    api.compatibilityAPI = _makeFakeNVVMBuilderAPIV2();
    for (SlangNVVMBuilderFeature_3 feature = 0; feature < SLANG_NVVM_BUILDER_FEATURE_COUNT_3;
         ++feature)
    {
        api.features.words[feature / 64u] |= uint64_t(1) << (feature % 64u);
    }
    api.emitIntegerUnary = _fakeNVVMBuilderEmitIntegerUnaryV3;
    api.emitIntegerBinary = _fakeNVVMBuilderEmitIntegerBinaryV3;
    api.emitIntegerCompare = _fakeNVVMBuilderEmitIntegerCompareV3;
    api.getFloatingPointType = _fakeNVVMBuilderGetFloatingPointType;
    api.emitFloatingBinary = _fakeNVVMBuilderEmitFloatingBinaryV3;
    return api;
}

static SlangResult SLANG_NVVM_CALL _fakeGetNVVMBuilderAPI(SlangNVVMBuilderAPI_V1* outAPI)
{
    if (!outAPI || outAPI->structureSize != sizeof(*outAPI) ||
        outAPI->abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_1)
    {
        return SLANG_E_NO_INTERFACE;
    }
    *outAPI = gFakeNVVMBuilder.api;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeGetNVVMBuilderAPIV2(SlangNVVMBuilderAPI_V2* outAPI)
{
    if (!outAPI || outAPI->structureSize < SLANG_NVVM_BUILDER_API_V2_MIN_SIZE ||
        outAPI->abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_2)
    {
        return SLANG_E_NO_INTERFACE;
    }

    const uint32_t callerSize = outAPI->structureSize;
    const uint32_t providerSize = gFakeNVVMBuilder.apiV2.structureSize;
    uint32_t copySize = callerSize < providerSize ? callerSize : providerSize;
    if (copySize > sizeof(gFakeNVVMBuilder.apiV2))
        copySize = uint32_t(sizeof(gFakeNVVMBuilder.apiV2));
    ::memcpy(outAPI, &gFakeNVVMBuilder.apiV2, copySize);
    outAPI->structureSize = providerSize;
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeGetNVVMBuilderAPIV3(SlangNVVMBuilderAPI_V3* outAPI)
{
    if (!outAPI || outAPI->structureSize < SLANG_NVVM_BUILDER_API_V3_MIN_SIZE ||
        outAPI->abiVersion != SLANG_NVVM_BUILDER_ABI_VERSION_3)
    {
        return SLANG_E_NO_INTERFACE;
    }

    const uint32_t callerSize = outAPI->structureSize;
    const uint32_t providerSize = gFakeNVVMBuilder.apiV3.structureSize;
    uint32_t copySize = callerSize < providerSize ? callerSize : providerSize;
    if (copySize > sizeof(gFakeNVVMBuilder.apiV3))
        copySize = uint32_t(sizeof(gFakeNVVMBuilder.apiV3));
    ::memcpy(outAPI, &gFakeNVVMBuilder.apiV3, copySize);
    outAPI->structureSize = providerSize;
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
        if (!name || gFakeNVVMBuilder.omitAPISymbol)
            return nullptr;
        const UnownedStringSlice symbol(name);
        if (!gFakeNVVMBuilder.omitAPIV3Symbol && symbol == SLANG_NVVM_BUILDER_GET_API_V3_NAME)
        {
            return reinterpret_cast<void*>(_fakeGetNVVMBuilderAPIV3);
        }
        if (!gFakeNVVMBuilder.omitAPIV2Symbol && symbol == SLANG_NVVM_BUILDER_GET_API_V2_NAME)
        {
            return reinterpret_cast<void*>(_fakeGetNVVMBuilderAPIV2);
        }
        return symbol == SLANG_NVVM_BUILDER_GET_API_V1_NAME
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
    SlangNVVMModuleHandle_1 module = nullptr;

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
    SLANG_CHECK_ABORT(outBuilder.getAPIV3() != nullptr);
}

static SlangResult _populateEmptyNVVMKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    const UnownedStringSlice& kernelName)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(module, voidType, nullptr, 0, functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(module, functionType, kernelName, function));
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
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    {
        const SlangNVVMTypeHandle_1 parameterTypes[] = {
            globalIntegerPointerType,
            integerType,
        };
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SlangNVVMValueHandle_1 function = nullptr;
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 value = nullptr;
        SlangNVVMBlockHandle_1 entryBlock = nullptr;
        SLANG_RETURN_ON_FAIL(builder.getFunctionType(
            module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType));
        SLANG_RETURN_ON_FAIL(builder.declareFunction(
            module,
            functionType,
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
        const SlangNVVMTypeHandle_1 parameterTypes[] = {
            globalIntegerPointerType,
            globalIntegerPointerType,
        };
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SlangNVVMValueHandle_1 function = nullptr;
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 source = nullptr;
        SlangNVVMValueHandle_1 value = nullptr;
        SlangNVVMBlockHandle_1 entryBlock = nullptr;
        SLANG_RETURN_ON_FAIL(builder.getFunctionType(
            module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType));
        SLANG_RETURN_ON_FAIL(builder.declareFunction(
            module,
            functionType,
            toSlice(kCopyScalarKernelName),
            function));
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
        SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
        SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
        SLANG_RETURN_ON_FAIL(builder.emitLoad(module, source, 4, value));
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

static SlangResult _populateFloat32BinaryKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    const UnownedStringSlice& kernelName,
    SlangNVVMFloatingBinaryOp_3 operation)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 floatType = nullptr;
    SlangNVVMTypeHandle_1 globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalFloatPointerType,
        floatType,
        floatType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 left = nullptr;
    SlangNVVMValueHandle_1 right = nullptr;
    SlangNVVMValueHandle_1 sum = nullptr;
    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(module, functionType, kernelName, function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, left));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, right));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.emitFloatingBinary(module, operation, left, right, sum));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, sum, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _populateFloat32CopyKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    const UnownedStringSlice& kernelName)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 floatType = nullptr;
    SlangNVVMTypeHandle_1 globalFloatPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getFloatingPointType(module, 32, floatType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        floatType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalFloatPointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalFloatPointerType,
        globalFloatPointerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(module, functionType, kernelName, function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.emitLoad(module, source, 4, value));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
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
static const char kDirectNVVMFloat32CopySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
)";
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
static const char kDirectNVVMConventionalRWStructuredBufferI32StoreSource[] = R"(
RWStructuredBuffer<int> destination;

[numthreads(1, 1, 1)]
void computeMain()
{
    destination[0] = 42;
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
static const char kDirectNVVMRawStructuredBufferI32LoadSource[] = R"(
[CUDAKernel]
void computeMain(
    RWStructuredBuffer<int> destination,
    StructuredBuffer<int> source,
    uniform int index)
{
    destination[0] = source[index];
}
)";
static const char kDirectNVVMRawRWStructuredBufferI32LoadSource[] = R"(
[CUDAKernel]
void computeMain(
    RWStructuredBuffer<int> destination,
    RWStructuredBuffer<int> source,
    uniform int index)
{
    destination[0] = source[index];
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
static const char kDirectNVVMUnsupportedLocalArraySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int index,
    uniform int x,
    uniform int y)
{
    int values[2];
    values[0] = x;
    values[1] = y;
    *destination = values[index];
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
     {FakeNVVMBuilderScalarFamily::Unary, SLANG_NVVM_INTEGER_UNARY_OP_BIT_NOT},
     kDirectNVVMIntegerBitNotSource,
     kBitNotScalarKernelName,
     "xor",
     ScalarRuntimeOperation::BitNot,
     NVVMScalarPTXEvidence::BitNot,
     "integer-bit-NOT"},
    {NVVMScalarTestOperation::Negate,
     {FakeNVVMBuilderScalarFamily::Unary, SLANG_NVVM_INTEGER_UNARY_OP_NEGATE},
     kDirectNVVMIntegerNegateSource,
     kNegateScalarKernelName,
     "sub",
     ScalarRuntimeOperation::Negate,
     NVVMScalarPTXEvidence::Negate,
     "integer-negate"},
};

static const NVVMScalarTestCase kNVVMBinaryScalarTestCases[] = {
    {NVVMScalarTestOperation::Multiply,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_INTEGER_BINARY_OP_3_MULTIPLY},
     kDirectNVVMIntegerMultiplySource,
     kMultiplyScalarKernelName,
     "mul",
     ScalarRuntimeOperation::Multiply,
     NVVMScalarPTXEvidence::Multiply,
     "integer-multiply"},
    {NVVMScalarTestOperation::BitAnd,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_AND},
     kDirectNVVMIntegerBitAndSource,
     kBitAndScalarKernelName,
     "and",
     ScalarRuntimeOperation::BitAnd,
     NVVMScalarPTXEvidence::BitAnd,
     "integer-bit-AND"},
    {NVVMScalarTestOperation::BitOr,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_OR},
     kDirectNVVMIntegerBitOrSource,
     kBitOrScalarKernelName,
     "or",
     ScalarRuntimeOperation::BitOr,
     NVVMScalarPTXEvidence::BitOr,
     "integer-bit-OR"},
    {NVVMScalarTestOperation::BitXor,
     {FakeNVVMBuilderScalarFamily::Binary, SLANG_NVVM_INTEGER_BINARY_OP_3_BIT_XOR},
     kDirectNVVMIntegerBitXorSource,
     kBitXorScalarKernelName,
     "xor",
     ScalarRuntimeOperation::BitXor,
     NVVMScalarPTXEvidence::BitXor,
     "integer-bit-XOR"},
};

static const NVVMScalarTestCase kNVVMCompareScalarTestCases[] = {
    {NVVMScalarTestOperation::Equal,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_EQUAL},
     kDirectNVVMIntegerEqualSource,
     kEqualScalarKernelName,
     "icmp eq",
     ScalarRuntimeOperation::Equal,
     NVVMScalarPTXEvidence::EqualityComparison,
     "integer-equality"},
    {NVVMScalarTestOperation::NotEqual,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_NOT_EQUAL},
     kDirectNVVMIntegerNotEqualSource,
     kNotEqualScalarKernelName,
     "icmp ne",
     ScalarRuntimeOperation::NotEqual,
     NVVMScalarPTXEvidence::EqualityComparison,
     "integer-inequality"},
    {NVVMScalarTestOperation::SignedGreaterThan,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_THAN},
     kDirectNVVMIntegerSignedGreaterThanSource,
     kGreaterThanScalarKernelName,
     "icmp sgt",
     ScalarRuntimeOperation::GreaterThan,
     NVVMScalarPTXEvidence::SignedComparison,
     "integer-signed-greater-than"},
    {NVVMScalarTestOperation::SignedLessEqual,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_LESS_EQUAL},
     kDirectNVVMIntegerSignedLessEqualSource,
     kLessEqualScalarKernelName,
     "icmp sle",
     ScalarRuntimeOperation::LessEqual,
     NVVMScalarPTXEvidence::SignedComparison,
     "integer-signed-less-equal"},
    {NVVMScalarTestOperation::SignedGreaterEqual,
     {FakeNVVMBuilderScalarFamily::Compare, SLANG_NVVM_INTEGER_COMPARE_OP_SIGNED_GREATER_EQUAL},
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
    SlangNVVMModuleHandle_1 module,
    const NVVMScalarTestCase& testCase,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1& outValue)
{
    switch (testCase.key.family)
    {
    case FakeNVVMBuilderScalarFamily::Unary:
        return builder.emitIntegerUnary(
            module,
            SlangNVVMIntegerUnaryOp_3(testCase.key.operation),
            left,
            outValue);
    case FakeNVVMBuilderScalarFamily::Binary:
        return builder.emitIntegerBinaryOperation(
            module,
            SlangNVVMIntegerBinaryOp_3(testCase.key.operation),
            left,
            right,
            outValue);
    case FakeNVVMBuilderScalarFamily::Compare:
        return builder.emitIntegerCompare(
            module,
            SlangNVVMIntegerCompareOp_3(testCase.key.operation),
            left,
            right,
            outValue);
    }
    return SLANG_E_INVALID_ARG;
}

static SlangResult _populateNVVMScalarTestKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module,
    const NVVMScalarTestCase& testCase)
{
    const bool isUnary = testCase.key.family == FakeNVVMBuilderScalarFamily::Unary;
    const bool isCompare = testCase.key.family == FakeNVVMBuilderScalarFamily::Compare;

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(
        builder.getPointerType(module, integerType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, pointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
    const size_t parameterCount = isUnary ? 2 : 3;
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.getFunctionType(module, voidType, parameterTypes, parameterCount, functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        UnownedStringSlice(testCase.kernelName),
        function));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 left = nullptr;
    SlangNVVMValueHandle_1 right = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, left));
    if (!isUnary)
        SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, right));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle_1 result = nullptr;
    SLANG_RETURN_ON_FAIL(
        _emitNVVMScalarTestOperation(builder, module, testCase, left, right, result));
    if (!isCompare)
    {
        SLANG_RETURN_ON_FAIL(builder.emitStore(module, result, destination, 4));
        SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
        SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
        return SLANG_OK;
    }

    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("true"), trueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("false"), falseBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("merge"), mergeBlock));

    SlangNVVMValueHandle_1 zero = nullptr;
    SlangNVVMValueHandle_1 one = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 0, zero));
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 1, one));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, result, trueBlock, falseBlock));

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
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    gFakeNVVM.reset();
    gFakeNVVM.compiledPTX = kFakeDirectPTX;
}

static void _enableFakeNVVMBuilderV3()
{
    gFakeNVVMBuilder.apiV3 = _makeFakeNVVMBuilderAPIV3();
    gFakeNVVMBuilder.omitAPIV3Symbol = false;
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
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalIntegerPointerType,
        integerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(
        builder.declareFunction(module, functionType, toSlice(kChooseScalarKernelName), function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, x));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, y));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("if.true"), trueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("if.false"), falseBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("if.merge"), mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerSignedLessThan(module, x, y, condition));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, condition, trueBlock, falseBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, trueBlock));
    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, x, y, sum));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, sum, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, mergeBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, falseBlock));
    SlangNVVMValueHandle_1 difference = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_SUB, x, y, difference));
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
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 limit = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(
        builder.declareFunction(module, functionType, toSlice(kSumToLimitKernelName), function));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, limit));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 headerBlock = nullptr;
    SlangNVVMBlockHandle_1 bodyBlock = nullptr;
    SlangNVVMBlockHandle_1 continueBlock = nullptr;
    SlangNVVMBlockHandle_1 exitBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, function, toSlice("loop.header"), headerBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("loop.body"), bodyBlock));
    SLANG_RETURN_ON_FAIL(
        builder.createBlock(module, function, toSlice("loop.continue"), continueBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("loop.exit"), exitBlock));

    SlangNVVMValueHandle_1 zero = nullptr;
    SlangNVVMValueHandle_1 one = nullptr;
    SlangNVVMValueHandle_1 i = nullptr;
    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 0, zero));
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 1, one));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerPhi(module, headerBlock, integerType, i));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerPhi(module, headerBlock, integerType, sum));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, headerBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, headerBlock));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerSignedLessThan(module, i, limit, condition));
    SLANG_RETURN_ON_FAIL(builder.emitConditionalBranch(module, condition, bodyBlock, exitBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, bodyBlock));
    SlangNVVMValueHandle_1 nextSum = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, sum, i, nextSum));
    SLANG_RETURN_ON_FAIL(builder.emitBranch(module, continueBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, continueBlock));
    SlangNVVMValueHandle_1 nextI = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitIntegerBinary(module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, i, one, nextI));
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
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    SlangNVVMTypeHandle_1 helperType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(module, integerType, &integerType, 1, helperType));
    SlangNVVMValueHandle_1 helper = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.declareFunction(module, helperType, toSlice(kIncrementScalarHelperName), helper));
    SlangNVVMValueHandle_1 helperValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, helper, 0, helperValue));

    const SlangNVVMTypeHandle_1 kernelParameterTypes[] = {
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 kernelType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        kernelParameterTypes,
        SLANG_COUNT_OF(kernelParameterTypes),
        kernelType));
    SlangNVVMValueHandle_1 kernel = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.declareFunction(module, kernelType, toSlice(kCallScalarKernelName), kernel));
    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 kernelValue = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, kernel, 1, kernelValue));

    SlangNVVMBlockHandle_1 helperBlock = nullptr;
    SlangNVVMBlockHandle_1 kernelBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, helper, toSlice("helper.entry"), helperBlock));
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, kernel, toSlice("kernel.entry"), kernelBlock));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, helperBlock));
    SlangNVVMValueHandle_1 one = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getIntegerConstant(module, integerType, 1, one));
    SlangNVVMValueHandle_1 incremented = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerBinary(
        module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        helperValue,
        one,
        incremented));
    SLANG_RETURN_ON_FAIL(builder.emitIntegerReturn(module, incremented));

    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, kernelBlock));
    SlangNVVMValueHandle_1 callResult = nullptr;
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
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 globalIntegerPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalIntegerPointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalIntegerPointerType,
        globalIntegerPointerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(
        builder.declareFunction(module, functionType, toSlice(kCopyIndexedKernelName), function));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, index));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle_1 destinationElement = nullptr;
    SlangNVVMValueHandle_1 sourceElement = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitPointerOffset(module, destination, index, destinationElement));
    SLANG_RETURN_ON_FAIL(builder.emitPointerOffset(module, source, index, sourceElement));
    SLANG_RETURN_ON_FAIL(builder.emitLoad(module, sourceElement, 4, value));
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

static SlangResult _populateArrayElementKernel(
    const NVVMIRBuilder& builder,
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 arrayType = nullptr;
    SlangNVVMTypeHandle_1 globalArrayPointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(builder.getArrayType(module, integerType, 4, arrayType));
    SLANG_RETURN_ON_FAIL(builder.getPointerType(
        module,
        arrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        globalArrayPointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        globalArrayPointerType,
        globalArrayPointerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        toSlice(kCopyArrayElementKernelName),
        function));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, source));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, index));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle_1 destinationElement = nullptr;
    SlangNVVMValueHandle_1 sourceElement = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_RETURN_ON_FAIL(
        builder.emitArrayElementPointer(module, destination, index, destinationElement));
    SLANG_RETURN_ON_FAIL(builder.emitArrayElementPointer(module, source, index, sourceElement));
    SLANG_RETURN_ON_FAIL(builder.emitLoad(module, sourceElement, 4, value));
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
    SlangNVVMModuleHandle_1 module)
{
    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getVoidType(module, voidType));
    SLANG_RETURN_ON_FAIL(builder.getIntegerType(module, 32, integerType));
    SLANG_RETURN_ON_FAIL(
        builder.getPointerType(module, integerType, SLANG_NVVM_ADDRESS_SPACE_GLOBAL, pointerType));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, pointerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SlangNVVMValueHandle_1 function = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        toSlice(kRelaxedGlobalI32AtomicAddKernelName),
        function));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 oldValueDestination = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, oldValueDestination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, value));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle_1 oldValue = nullptr;
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

static SlangResult _runFloat32BinaryKernel(
    CudaDriverApi& cuda,
    ISlangBlob* ptxBlob,
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

    void* parameters[] = {&destination, &left, &right};
    if (cuda.cuLaunchKernel(function, 1, 1, 1, 1, 1, 1, 0, nullptr, parameters, nullptr) != 0 ||
        cuda.cuCtxSynchronize() != 0)
    {
        return SLANG_FAIL;
    }

    float actual = 0.0f;
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
        if (builder.getAPI().llvmVersionMajor != 14 || !builder.supportsSerializationDiagnostics())
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
        if (builder.getAPI().llvmVersionMajor != 14 || !builder.supportsSerializationDiagnostics())
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
