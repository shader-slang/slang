// unit-test-nvvm-compiler.cpp

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
    VerifyProgram,
    CompileProgram,
    GetResultSize,
    GetResult,
    GetLogSize,
    GetLog,
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
struct FakeNVVMBuilderPointerTypeStorage
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
struct FakeNVVMBuilderIntegerBinaryStorage
{
};
struct FakeNVVMBuilderIntegerComparisonStorage
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
struct FakeNVVMBuilderIntegerMultiplyStorage
{
};
struct FakeNVVMBuilderIntegerBitAndStorage
{
};

enum class FakeNVVMBuilderValueKind
{
    Parameter,
    Load,
    IntegerBinary,
    IntegerConstant,
    IntegerPhi,
    Call,
    PointerOffset,
    ArrayElementPointer,
    IntegerMultiply,
    IntegerBitAnd,
};

struct FakeNVVMBuilderValueRef
{
    FakeNVVMBuilderValueKind kind = FakeNVVMBuilderValueKind::Parameter;
    Index index = -1;
    Index functionIndex = -1;
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
    ArrayPointer,
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
        getArrayTypeCallCount = 0;
        getPointerTypeCallCount = 0;
        getFunctionParameterCallCount = 0;
        emitLoadCallCount = 0;
        emitStoreCallCount = 0;
        emitIntegerBinaryCallCount = 0;
        emitIntegerSignedLessThanCallCount = 0;
        emitBranchCallCount = 0;
        emitConditionalBranchCallCount = 0;
        getIntegerConstantCallCount = 0;
        emitIntegerPhiCallCount = 0;
        addIntegerPhiIncomingCallCount = 0;
        emitIntegerCallCallCount = 0;
        emitIntegerReturnCallCount = 0;
        emitPointerOffsetCallCount = 0;
        emitArrayElementPointerCallCount = 0;
        emitIntegerMultiplyCallCount = 0;
        emitIntegerBitAndCallCount = 0;
        integerBitWidth = 0;
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
        integerBinaryOperations.clear();
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
        integerBinaryLeftParameterIndices.clear();
        integerBinaryRightParameterIndices.clear();
        integerBinaryLeftValueRefs.clear();
        integerBinaryRightValueRefs.clear();
        comparisonLeftParameterIndices.clear();
        comparisonRightParameterIndices.clear();
        comparisonLeftValueRefs.clear();
        comparisonRightValueRefs.clear();
        storeValueRefs.clear();
        storeBlockIndices.clear();
        integerBinaryBlockIndices.clear();
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
        integerMultiplyCallerBlockIndices.clear();
        integerMultiplyLeftValueRefs.clear();
        integerMultiplyRightValueRefs.clear();
        integerBitAndCallerBlockIndices.clear();
        integerBitAndLeftValueRefs.clear();
        integerBitAndRightValueRefs.clear();
        loadPointerValueRefs.clear();
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
        omitAPISymbol = false;
        omitAPIV2Symbol = true;
        libraryUnavailable = false;
        returnNullModule = false;
        returnNullIntegerType = false;
        returnNullArrayType = false;
        returnNullArrayElementPointer = false;
        returnNullIntegerMultiply = false;
        returnNullIntegerBitAnd = false;
        failIntegerTypeAfterWrite = false;
        failArrayTypeAfterWrite = false;
        failIntegerBinaryAfterWrite = false;
        failIntegerConstantAfterWrite = false;
        failIntegerPhiAfterWrite = false;
        failIntegerCallAfterWrite = false;
        failIntegerReturn = false;
        failPointerOffsetAfterWrite = false;
        failArrayElementPointerAfterWrite = false;
        failIntegerMultiplyAfterWrite = false;
        failIntegerBitAndAfterWrite = false;
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
    bool omitAPISymbol = false;
    bool omitAPIV2Symbol = true;
    bool libraryUnavailable = false;
    bool returnNullModule = false;
    bool returnNullIntegerType = false;
    bool returnNullArrayType = false;
    bool returnNullArrayElementPointer = false;
    bool returnNullIntegerMultiply = false;
    bool returnNullIntegerBitAnd = false;
    bool failIntegerTypeAfterWrite = false;
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
    FakeNVVMBuilderPointerTypeStorage pointerTypeStorage;
    FakeNVVMBuilderArrayTypeStorage arrayTypeStorage;
    FakeNVVMBuilderArrayPointerTypeStorage arrayPointerTypeStorage;
    FakeNVVMBuilderParameterStorage parameterStorage[64];
    FakeNVVMBuilderLoadStorage loadStorage;
    FakeNVVMBuilderIntegerBinaryStorage integerBinaryStorage[8];
    FakeNVVMBuilderIntegerComparisonStorage integerComparisonStorage;
    FakeNVVMBuilderIntegerConstantStorage integerConstantStorage[8];
    FakeNVVMBuilderIntegerPhiStorage integerPhiStorage[8];
    FakeNVVMBuilderCallStorage callStorage[16];
    FakeNVVMBuilderPointerOffsetStorage pointerOffsetStorage[16];
    FakeNVVMBuilderArrayElementPointerStorage arrayElementPointerStorage[16];
    FakeNVVMBuilderIntegerMultiplyStorage integerMultiplyStorage[16];
    FakeNVVMBuilderIntegerBitAndStorage integerBitAndStorage[16];

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
    int getArrayTypeCallCount = 0;
    int getPointerTypeCallCount = 0;
    int getFunctionParameterCallCount = 0;
    int emitLoadCallCount = 0;
    int emitStoreCallCount = 0;
    int emitIntegerBinaryCallCount = 0;
    int emitIntegerSignedLessThanCallCount = 0;
    int emitBranchCallCount = 0;
    int emitConditionalBranchCallCount = 0;
    int getIntegerConstantCallCount = 0;
    int emitIntegerPhiCallCount = 0;
    int addIntegerPhiIncomingCallCount = 0;
    int emitIntegerCallCallCount = 0;
    int emitIntegerReturnCallCount = 0;
    int emitPointerOffsetCallCount = 0;
    int emitArrayElementPointerCallCount = 0;
    int emitIntegerMultiplyCallCount = 0;
    int emitIntegerBitAndCallCount = 0;
    uint32_t integerBitWidth = 0;
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
    List<SlangNVVMIntegerBinaryOp_2> integerBinaryOperations;
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
    List<size_t> integerBinaryLeftParameterIndices;
    List<size_t> integerBinaryRightParameterIndices;
    List<FakeNVVMBuilderValueRef> integerBinaryLeftValueRefs;
    List<FakeNVVMBuilderValueRef> integerBinaryRightValueRefs;
    List<size_t> comparisonLeftParameterIndices;
    List<size_t> comparisonRightParameterIndices;
    List<FakeNVVMBuilderValueRef> comparisonLeftValueRefs;
    List<FakeNVVMBuilderValueRef> comparisonRightValueRefs;
    List<FakeNVVMBuilderValueRef> storeValueRefs;
    List<Index> storeBlockIndices;
    List<Index> integerBinaryBlockIndices;
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
    List<Index> integerMultiplyCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> integerMultiplyLeftValueRefs;
    List<FakeNVVMBuilderValueRef> integerMultiplyRightValueRefs;
    List<Index> integerBitAndCallerBlockIndices;
    List<FakeNVVMBuilderValueRef> integerBitAndLeftValueRefs;
    List<FakeNVVMBuilderValueRef> integerBitAndRightValueRefs;
    List<FakeNVVMBuilderValueRef> loadPointerValueRefs;
    List<FakeNVVMBuilderValueRef> storePointerValueRefs;
    List<Index> kernelFunctionIndices;
    Index currentInsertBlockIndex = -1;
    Index conditionalSourceBlockIndex = -1;
    Index conditionalTrueBlockIndex = -1;
    Index conditionalFalseBlockIndex = -1;
    String moduleName;
    String functionName;
    String blockName;
    bool failIntegerBinaryAfterWrite = false;
    bool failIntegerConstantAfterWrite = false;
    bool failIntegerPhiAfterWrite = false;
    bool failIntegerCallAfterWrite = false;
    bool failIntegerReturn = false;
    bool failPointerOffsetAfterWrite = false;
    bool failArrayElementPointerAfterWrite = false;
    bool failIntegerMultiplyAfterWrite = false;
    bool failIntegerBitAndAfterWrite = false;
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

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.pointerTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderArrayType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.arrayTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderArrayPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.arrayPointerTypeStorage);
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderLoad()
{
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.loadStorage);
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderIntegerBinary(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.integerBinaryStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.integerBinaryStorage[index]);
}

static bool _getFakeNVVMBuilderIntegerBinaryIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.integerBinaryOperations.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderIntegerBinary(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderIntegerComparison()
{
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.integerComparisonStorage);
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

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderIntegerMultiply(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.integerMultiplyStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(
        &gFakeNVVMBuilder.integerMultiplyStorage[index]);
}

static bool _getFakeNVVMBuilderIntegerMultiplyIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.integerMultiplyLeftValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderIntegerMultiply(i))
        {
            outIndex = i;
            return true;
        }
    }
    return false;
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderIntegerBitAnd(Index index = 0)
{
    SLANG_ASSERT(index >= 0 && index < SLANG_COUNT_OF(gFakeNVVMBuilder.integerBitAndStorage));
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.integerBitAndStorage[index]);
}

static bool _getFakeNVVMBuilderIntegerBitAndIndex(SlangNVVMValueHandle_1 value, Index& outIndex)
{
    for (Index i = 0; i < gFakeNVVMBuilder.integerBitAndLeftValueRefs.getCount(); ++i)
    {
        if (value == _getFakeNVVMBuilderIntegerBitAnd(i))
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
    if (value == _getFakeNVVMBuilderLoad())
    {
        outRef = {FakeNVVMBuilderValueKind::Load, 0};
        return true;
    }
    if (_getFakeNVVMBuilderIntegerBinaryIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::IntegerBinary, valueIndex};
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
    if (_getFakeNVVMBuilderIntegerMultiplyIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::IntegerMultiply, valueIndex};
        return true;
    }
    if (_getFakeNVVMBuilderIntegerBitAndIndex(value, valueIndex))
    {
        outRef = {FakeNVVMBuilderValueKind::IntegerBitAnd, valueIndex};
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
    case FakeNVVMBuilderValueKind::IntegerBinary:
    case FakeNVVMBuilderValueKind::IntegerConstant:
    case FakeNVVMBuilderValueKind::IntegerPhi:
    case FakeNVVMBuilderValueKind::Call:
    case FakeNVVMBuilderValueKind::IntegerMultiply:
    case FakeNVVMBuilderValueKind::IntegerBitAnd:
        return true;
    case FakeNVVMBuilderValueKind::PointerOffset:
    case FakeNVVMBuilderValueKind::ArrayElementPointer:
        return false;
    }
    return false;
}

static bool _isFakeNVVMBuilderPointerValue(SlangNVVMValueHandle_1 value)
{
    FakeNVVMBuilderValueRef valueRef;
    if (!_getFakeNVVMBuilderValueRef(value, valueRef))
        return false;

    if (valueRef.kind == FakeNVVMBuilderValueKind::PointerOffset ||
        valueRef.kind == FakeNVVMBuilderValueKind::ArrayElementPointer)
        return true;
    FakeNVVMBuilderParameterTypeKind parameterTypeKind;
    return _getFakeNVVMBuilderParameterTypeKind(valueRef, parameterTypeKind) &&
           parameterTypeKind == FakeNVVMBuilderParameterTypeKind::Pointer;
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
            parameterTypes[i] != _getFakeNVVMBuilderPointerType() &&
            parameterTypes[i] != _getFakeNVVMBuilderArrayPointerType())
        {
            return SLANG_E_INVALID_ARG;
        }
    }
    for (size_t i = 0; i < parameterCount; ++i)
    {
        const FakeNVVMBuilderParameterTypeKind parameterTypeKind =
            parameterTypes[i] == _getFakeNVVMBuilderIntegerType()
                ? FakeNVVMBuilderParameterTypeKind::Integer
            : parameterTypes[i] == _getFakeNVVMBuilderPointerType()
                ? FakeNVVMBuilderParameterTypeKind::Pointer
                : FakeNVVMBuilderParameterTypeKind::ArrayPointer;
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
         pointeeType != _getFakeNVVMBuilderArrayType()) ||
        !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.pointerPointeeTypes.add(pointeeType);
    gFakeNVVMBuilder.pointerAddressSpaces.add(addressSpace);
    *outType = pointeeType == _getFakeNVVMBuilderIntegerType()
                   ? _getFakeNVVMBuilderPointerType()
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
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef) || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }
    _getFakeNVVMBuilderParameterRef(pointer, pointerFunctionIndex, pointerIndex);
    gFakeNVVMBuilder.loadPointerParameterIndices.add(pointerIndex);
    gFakeNVVMBuilder.loadPointerValueRefs.add(pointerRef);
    *outValue = _getFakeNVVMBuilderLoad();
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
    if (module != _getFakeNVVMBuilderModule() || !_isFakeNVVMBuilderIntegerValue(value) ||
        !_isFakeNVVMBuilderPointerValue(pointer) ||
        !_getFakeNVVMBuilderValueRef(pointer, pointerRef))
    {
        return SLANG_E_INVALID_ARG;
    }
    _getFakeNVVMBuilderParameterRef(pointer, pointerFunctionIndex, pointerIndex);
    gFakeNVVMBuilder.storePointerFunctionIndices.add(pointerFunctionIndex);
    gFakeNVVMBuilder.storePointerParameterIndices.add(pointerIndex);
    gFakeNVVMBuilder.storePointerValueRefs.add(pointerRef);
    FakeNVVMBuilderValueRef valueRef;
    SLANG_ASSERT(_getFakeNVVMBuilderValueRef(value, valueRef));
    gFakeNVVMBuilder.storeValueRefs.add(valueRef);
    gFakeNVVMBuilder.storeBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    size_t valueParameterIndex = size_t(-1);
    Index valueParameterFunctionIndex = -1;
    Index valueBinaryIndex = -1;
    if (_getFakeNVVMBuilderParameterRef(value, valueParameterFunctionIndex, valueParameterIndex))
        gFakeNVVMBuilder.storeValueKinds.add(FakeNVVMBuilderValueKind::Parameter);
    else if (value == _getFakeNVVMBuilderLoad())
        gFakeNVVMBuilder.storeValueKinds.add(FakeNVVMBuilderValueKind::Load);
    else if (_getFakeNVVMBuilderIntegerBinaryIndex(value, valueBinaryIndex))
    {
        gFakeNVVMBuilder.storeValueKinds.add(FakeNVVMBuilderValueKind::IntegerBinary);
    }
    else
    {
        gFakeNVVMBuilder.storeValueKinds.add(valueRef.kind);
    }
    gFakeNVVMBuilder.storeValueParameterIndices.add(valueParameterIndex);
    gFakeNVVMBuilder.storeValueBinaryIndices.add(valueBinaryIndex);
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBinary(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMIntegerBinaryOp_2 operation,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerBinaryCallCount;
    if (module != _getFakeNVVMBuilderModule() ||
        (operation != SLANG_NVVM_INTEGER_BINARY_OP_ADD &&
         operation != SLANG_NVVM_INTEGER_BINARY_OP_SUB) ||
        !_isFakeNVVMBuilderIntegerValue(left) || !_isFakeNVVMBuilderIntegerValue(right) ||
        !outValue ||
        gFakeNVVMBuilder.integerBinaryOperations.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.integerBinaryStorage))
    {
        return SLANG_E_INVALID_ARG;
    }
    const Index resultIndex = gFakeNVVMBuilder.integerBinaryOperations.getCount();
    Index leftFunctionIndex = -1;
    Index rightFunctionIndex = -1;
    size_t leftIndex = size_t(-1);
    size_t rightIndex = size_t(-1);
    FakeNVVMBuilderValueRef leftRef;
    FakeNVVMBuilderValueRef rightRef;
    SLANG_ASSERT(_getFakeNVVMBuilderValueRef(left, leftRef));
    SLANG_ASSERT(_getFakeNVVMBuilderValueRef(right, rightRef));
    _getFakeNVVMBuilderParameterRef(left, leftFunctionIndex, leftIndex);
    _getFakeNVVMBuilderParameterRef(right, rightFunctionIndex, rightIndex);
    gFakeNVVMBuilder.integerBinaryOperations.add(operation);
    gFakeNVVMBuilder.integerBinaryLeftParameterIndices.add(leftIndex);
    gFakeNVVMBuilder.integerBinaryRightParameterIndices.add(rightIndex);
    gFakeNVVMBuilder.integerBinaryLeftValueRefs.add(leftRef);
    gFakeNVVMBuilder.integerBinaryRightValueRefs.add(rightRef);
    gFakeNVVMBuilder.integerBinaryBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    *outValue = _getFakeNVVMBuilderIntegerBinary(resultIndex);
    return gFakeNVVMBuilder.failIntegerBinaryAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerSignedLessThan(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount;
    if (module != _getFakeNVVMBuilderModule() || !_isFakeNVVMBuilderIntegerValue(left) ||
        !_isFakeNVVMBuilderIntegerValue(right) || !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }
    Index leftFunctionIndex = -1;
    Index rightFunctionIndex = -1;
    size_t leftIndex = size_t(-1);
    size_t rightIndex = size_t(-1);
    FakeNVVMBuilderValueRef leftRef;
    FakeNVVMBuilderValueRef rightRef;
    SLANG_ASSERT(_getFakeNVVMBuilderValueRef(left, leftRef));
    SLANG_ASSERT(_getFakeNVVMBuilderValueRef(right, rightRef));
    _getFakeNVVMBuilderParameterRef(left, leftFunctionIndex, leftIndex);
    _getFakeNVVMBuilderParameterRef(right, rightFunctionIndex, rightIndex);
    gFakeNVVMBuilder.comparisonLeftParameterIndices.add(leftIndex);
    gFakeNVVMBuilder.comparisonRightParameterIndices.add(rightIndex);
    gFakeNVVMBuilder.comparisonLeftValueRefs.add(leftRef);
    gFakeNVVMBuilder.comparisonRightValueRefs.add(rightRef);
    *outValue = _getFakeNVVMBuilderIntegerComparison();
    return SLANG_OK;
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
    if (module != _getFakeNVVMBuilderModule() ||
        condition != _getFakeNVVMBuilderIntegerComparison() ||
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerMultiply(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerMultiplyCallCount;
    if (outValue)
        *outValue = nullptr;

    FakeNVVMBuilderValueRef leftRef;
    FakeNVVMBuilderValueRef rightRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderIntegerValue(left) || !_getFakeNVVMBuilderValueRef(left, leftRef) ||
        !_isFakeNVVMBuilderIntegerValue(right) || !_getFakeNVVMBuilderValueRef(right, rightRef) ||
        !outValue ||
        gFakeNVVMBuilder.integerMultiplyLeftValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.integerMultiplyStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.integerMultiplyLeftValueRefs.getCount();
    gFakeNVVMBuilder.integerMultiplyCallerBlockIndices.add(
        gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.integerMultiplyLeftValueRefs.add(leftRef);
    gFakeNVVMBuilder.integerMultiplyRightValueRefs.add(rightRef);
    *outValue = gFakeNVVMBuilder.returnNullIntegerMultiply
                    ? nullptr
                    : _getFakeNVVMBuilderIntegerMultiply(resultIndex);
    return gFakeNVVMBuilder.failIntegerMultiplyAfterWrite ? SLANG_FAIL : SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitIntegerBitAnd(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 left,
    SlangNVVMValueHandle_1 right,
    SlangNVVMValueHandle_1* outValue)
{
    ++gFakeNVVMBuilder.emitIntegerBitAndCallCount;
    if (outValue)
        *outValue = nullptr;

    FakeNVVMBuilderValueRef leftRef;
    FakeNVVMBuilderValueRef rightRef;
    if (module != _getFakeNVVMBuilderModule() || gFakeNVVMBuilder.currentInsertBlockIndex < 0 ||
        !_isFakeNVVMBuilderIntegerValue(left) || !_getFakeNVVMBuilderValueRef(left, leftRef) ||
        !_isFakeNVVMBuilderIntegerValue(right) || !_getFakeNVVMBuilderValueRef(right, rightRef) ||
        !outValue ||
        gFakeNVVMBuilder.integerBitAndLeftValueRefs.getCount() >=
            SLANG_COUNT_OF(gFakeNVVMBuilder.integerBitAndStorage))
    {
        return SLANG_E_INVALID_ARG;
    }

    const Index resultIndex = gFakeNVVMBuilder.integerBitAndLeftValueRefs.getCount();
    gFakeNVVMBuilder.integerBitAndCallerBlockIndices.add(gFakeNVVMBuilder.currentInsertBlockIndex);
    gFakeNVVMBuilder.integerBitAndLeftValueRefs.add(leftRef);
    gFakeNVVMBuilder.integerBitAndRightValueRefs.add(rightRef);
    *outValue = gFakeNVVMBuilder.returnNullIntegerBitAnd
                    ? nullptr
                    : _getFakeNVVMBuilderIntegerBitAnd(resultIndex);
    return gFakeNVVMBuilder.failIntegerBitAndAfterWrite ? SLANG_FAIL : SLANG_OK;
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
        verifyProgramCallCount = 0;
        compileProgramCallCount = 0;
        getResultSizeCallCount = 0;
        getResultCallCount = 0;
        getLogSizeCallCount = 0;
        getLogCallCount = 0;
        addedModule = String();
        addedModuleName = String();
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
    int verifyProgramCallCount = 0;
    int compileProgramCallCount = 0;
    int getResultSizeCallCount = 0;
    int getResultCallCount = 0;
    int getLogSizeCallCount = 0;
    int getLogCallCount = 0;

    String addedModule;
    String addedModuleName;
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
    gFakeNVVM.addedModule = String(UnownedStringSlice(buffer, size));
    gFakeNVVM.addedModuleName = name;
    return _fakeFailureResult(FakeFailure::AddModule);
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
    SLANG_UNUSED(program);
    SLANG_UNUSED(buffer);
    SLANG_UNUSED(size);
    SLANG_UNUSED(name);
    return TestNVVMResult::Success;
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
    bool addFakeCompilerArgument = false;
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
    TerminatedCharSlice fakeArgument("-fake-nvvm-option");

    DownstreamCompileOptions options;
    options.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    options.targetType = SLANG_PTX;
    options.optimizationLevel = settings.optimizationLevel;
    options.debugInfoType = settings.debugInfoType;
    options.floatingPointMode = settings.floatingPointMode;
    options.denormalModeFp32 = settings.denormalModeFp32;
    options.sourceArtifacts = makeSlice(sourceArtifacts, SLANG_COUNT_OF(sourceArtifacts));
    options.requiredCapabilityVersions = makeSlice(&capability, 1);
    if (settings.addFakeCompilerArgument)
        options.compilerSpecificArguments = makeSlice(&fakeArgument, 1);
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

static void _resetDirectNVVMFakes()
{
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
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

static SlangResult _populateIntegerMultiplyKernel(
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
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(builder.declareFunction(
        module,
        functionType,
        toSlice(kMultiplyScalarKernelName),
        function));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, x));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, y));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle_1 product = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerMultiply(module, x, y, product));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, product, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildIntegerMultiplyModule(
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
        builder.createModule(toSlice("slang-nvvm-integer-multiply"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateIntegerMultiplyKernel(builder, scope.module));
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

static SlangResult _populateIntegerBitAndKernel(
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
    SLANG_RETURN_ON_FAIL(builder.getFunctionType(
        module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType));
    SLANG_RETURN_ON_FAIL(
        builder.declareFunction(module, functionType, toSlice(kBitAndScalarKernelName), function));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 0, destination));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 1, x));
    SLANG_RETURN_ON_FAIL(builder.getFunctionParameter(module, function, 2, y));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SLANG_RETURN_ON_FAIL(builder.createBlock(module, function, toSlice("entry"), entryBlock));
    SLANG_RETURN_ON_FAIL(builder.setInsertBlock(module, entryBlock));

    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_RETURN_ON_FAIL(builder.emitIntegerBitAnd(module, x, y, value));
    SLANG_RETURN_ON_FAIL(builder.emitStore(module, value, destination, 4));
    SLANG_RETURN_ON_FAIL(builder.emitReturnVoid(module));
    SLANG_RETURN_ON_FAIL(builder.markFunctionAsKernel(module, function));
    return SLANG_OK;
}

static SlangResult _buildIntegerBitAndModule(
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
    SLANG_RETURN_ON_FAIL(builder.createModule(toSlice("slang-nvvm-integer-bit-and"), scope.module));
    SLANG_RETURN_ON_FAIL(_populateIntegerBitAndKernel(builder, scope.module));
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

enum class ScalarRuntimeOperation
{
    Write,
    Copy,
    Choose,
    Multiply,
    BitAnd,
};

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
        parameters = writeParameters;
        break;
    case ScalarRuntimeOperation::Copy:
        parameters = copyParameters;
        break;
    case ScalarRuntimeOperation::Choose:
    case ScalarRuntimeOperation::Multiply:
    case ScalarRuntimeOperation::BitAnd:
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

static SlangResult _getPTXIntegerBitWidth(const UnownedStringSlice& text, uint32_t& outBitWidth)
{
    outBitWidth = 0;
    static const char* k32BitSpellings[] = {".b32", ".s32", ".u32"};
    static const char* k64BitSpellings[] = {".b64", ".s64", ".u64"};

    int matchCount = 0;
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
        SLANG_RETURN_ON_FAIL(_getPTXIntegerBitWidth(declaration, bitWidth));
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
            SLANG_SUCCEEDED(_getPTXIntegerBitWidth(instruction, instructionBitWidth)) &&
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
    bool hasMultiply32 = false;
    bool hasBitAnd32 = false;
    bool hasSignedComparison32 = false;
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
    outSummary.hasMultiply32 = false;
    outSummary.hasBitAnd32 = false;
    outSummary.hasSignedComparison32 = false;

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
    outSummary.hasMultiply32 = _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("mul"), 32);
    outSummary.hasBitAnd32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("and.b32"), 32);
    outSummary.hasSignedComparison32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.lt.s32"), 32) ||
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("setp.ge.s32"), 32);
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

static SlangResult _findPtxasFromCUDAPath(String& outCudaRoot, String& outPtxasPath)
{
    outCudaRoot = String();
    outPtxasPath = String();
    StringBuilder cudaRootBuilder;
    if (SLANG_FAILED(PlatformUtil::getEnvironmentVariable(toSlice("CUDA_PATH"), cudaRootBuilder)) ||
        !cudaRootBuilder.getLength())
    {
        return SLANG_E_NOT_FOUND;
    }

    outCudaRoot = cudaRootBuilder.produceString();
    outPtxasPath = Path::combine(
        Path::combine(outCudaRoot, "bin"),
        String("ptxas") + String(Process::getExecutableSuffix()));
    return File::exists(outPtxasPath) ? SLANG_OK : SLANG_E_NOT_FOUND;
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

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidABI)
{
    NVVMIRBuilder builder;
    SlangNVVMModuleHandle_1 module = nullptr;
    SLANG_CHECK(builder.createModule(toSlice("uninitialized"), module) == SLANG_E_UNINITIALIZED);
    SLANG_CHECK(module == nullptr);

    gFakeNVVMBuilder.reset();
    SlangNVVMBuilderAPI_V1 invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.structureSize -= 1;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.abiVersion += 1;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.llvmVersionMajor = 15;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.pointerModel = 0;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    invalidAPI = _makeFakeNVVMBuilderAPI();
    invalidAPI.serializeModule = nullptr;
    SLANG_CHECK(NVVMIRBuilder::initialize(invalidAPI, nullptr, builder) == SLANG_E_NO_INTERFACE);
    SLANG_CHECK(
        NVVMIRBuilder::initialize(_makeFakeNVVMBuilderAPI(), nullptr, builder) ==
        SLANG_E_INVALID_ARG);

    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.omitAPISymbol = true;
    ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
    SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
    SLANG_CHECK(gFakeNVVMBuilder.loadedPath == "slang-llvm-nvvm");
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // Once a provider advertises V2, a malformed V2 table is an incompatibility rather than a
    // reason to silently downgrade to its otherwise valid V1 export.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.serializeModuleWithDiagnostics = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder rejectedBuilder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, rejectedBuilder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!rejectedBuilder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    {
        ComPtr<ISlangSharedLibraryLoader> nullHandleLoader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder nullHandleBuilder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), nullHandleLoader, nullHandleBuilder)));
        gFakeNVVMBuilder.returnNullModule = true;
        module = nullptr;
        SLANG_CHECK(
            nullHandleBuilder.createModule(toSlice("missing-output"), module) == SLANG_FAIL);
        SLANG_CHECK(module == nullptr);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderForwardsVersionedABI)
{
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.isInitialized());
        SLANG_CHECK(!builder.supportsSerializationDiagnostics());
        SLANG_CHECK(builder.getAPIV2() == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 1);
        SLANG_CHECK(builder.getAPI().llvmVersionMajor == 14);
        SLANG_CHECK(builder.getAPI().llvmVersionMinor == 0);
        SLANG_CHECK(builder.getAPI().llvmVersionPatch == 6);
        SLANG_CHECK(builder.getAPI().nvvmIRVersionMajor == 2);
        SLANG_CHECK(builder.getAPI().nvvmIRVersionMinor == 0);
        SLANG_CHECK(builder.getAPI().pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED);

        static const char kModuleName[] = {'f', 'a', 'k', 'e', 0, 'm', 'o', 'd'};
        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createModule(
            UnownedStringSlice(kModuleName, SLANG_COUNT_OF(kModuleName)),
            scope.module)));

        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SlangNVVMValueHandle_1 function = nullptr;
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, nullptr, 0, functionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("callerNamedEmpty"),
            function)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(scope.module)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(scope.module, function)));

        size_t requiredSize = 0;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getAPI().serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &requiredSize)));
        uint8_t insufficientStorage[1] = {0xa5};
        size_t reportedSize = 0;
        SLANG_CHECK(
            builder.getAPI().serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                insufficientStorage,
                sizeof(insufficientStorage),
                &reportedSize) == SLANG_E_BUFFER_TOO_SMALL);
        SLANG_CHECK(reportedSize == requiredSize);
        SLANG_CHECK(insufficientStorage[0] == 0xa5);

        ComPtr<ISlangBlob> assembly;
        ComPtr<ISlangBlob> bitcode;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
            assembly)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode)));
        SLANG_CHECK(assembly->getBufferSize() == ::strlen("fake LLVM assembly"));
        SLANG_CHECK(
            ::memcmp(
                assembly->getBufferPointer(),
                "fake LLVM assembly",
                assembly->getBufferSize()) == 0);
        static const uint8_t kExpectedBitcode[] = {0x42, 0x43, 0xc0, 0xde, 0x00, 0x11};
        SLANG_CHECK(bitcode->getBufferSize() == SLANG_COUNT_OF(kExpectedBitcode));
        SLANG_CHECK(
            ::memcmp(
                bitcode->getBufferPointer(),
                kExpectedBitcode,
                SLANG_COUNT_OF(kExpectedBitcode)) == 0);

        ComPtr<ISlangBlob> unavailableDiagnosticSerialization;
        String unavailableDiagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                unavailableDiagnosticSerialization,
                unavailableDiagnostics) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(unavailableDiagnosticSerialization == nullptr);
        SLANG_CHECK(unavailableDiagnostics.getLength() == 0);

        gFakeNVVMBuilder.reportMismatchedWriteSize = true;
        ComPtr<ISlangBlob> malformedOutput;
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                malformedOutput) == SLANG_FAIL);
        SLANG_CHECK(malformedOutput == nullptr);
        gFakeNVVMBuilder.reportMismatchedWriteSize = false;

        SLANG_CHECK(gFakeNVVMBuilder.moduleName.getLength() == SLANG_COUNT_OF(kModuleName));
        SLANG_CHECK(
            ::memcmp(
                gFakeNVVMBuilder.moduleName.getBuffer(),
                kModuleName,
                SLANG_COUNT_OF(kModuleName)) == 0);
        SLANG_CHECK(gFakeNVVMBuilder.functionName == "callerNamedEmpty");
        SLANG_CHECK(gFakeNVVMBuilder.blockName == "entry");
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.setInsertBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeQueryCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWriteCallCount == 4);

        builder.destroyModule(scope.module);
        scope.module = nullptr;
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarAPI)
{
    // A V1-only provider cannot expose the appended operations. The host rejects every scalar
    // wrapper locally and clears every handle output before returning.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(!builder.supportsScalarOperations());
        SLANG_CHECK(builder.getAPIV2() == nullptr);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-v1-scalar-module"), scope.module)));

        SlangNVVMTypeHandle_1 integerType = _getFakeNVVMBuilderVoidType();
        SlangNVVMTypeHandle_1 pointerType = _getFakeNVVMBuilderVoidType();
        SlangNVVMValueHandle_1 parameter = _getFakeNVVMBuilderFunction();
        SlangNVVMValueHandle_1 loadedValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(integerType == nullptr);
        SLANG_CHECK(
            builder.getPointerType(
                scope.module,
                _getFakeNVVMBuilderIntegerType(),
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                pointerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(pointerType == nullptr);
        SLANG_CHECK(
            builder
                .getFunctionParameter(scope.module, _getFakeNVVMBuilderFunction(), 0, parameter) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(parameter == nullptr);
        SLANG_CHECK(
            builder.emitLoad(scope.module, _getFakeNVVMBuilderParameter(), 4, loadedValue) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(loadedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(
                scope.module,
                _getFakeNVVMBuilderLoad(),
                _getFakeNVVMBuilderParameter(),
                4) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A new provider must honor a Slice 3b caller's reported capacity even when the caller has
    // additional sentinel storage after that prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    struct BoundedV2Query
    {
        SlangNVVMBuilderAPI_V2 api;
        uint8_t trailingSentinels[16];
    } boundedQuery;
    ::memset(&boundedQuery, 0xa5, sizeof(boundedQuery));
    boundedQuery.api.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE);
    boundedQuery.api.abiVersion = SLANG_NVVM_BUILDER_ABI_VERSION_2;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_fakeGetNVVMBuilderAPIV2(&boundedQuery.api)));
    SLANG_CHECK(boundedQuery.api.structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    const uint8_t* boundedBytes = reinterpret_cast<const uint8_t*>(&boundedQuery);
    for (Index i = SLANG_NVVM_BUILDER_API_V2_MIN_SIZE; i < Index(sizeof(boundedQuery)); ++i)
        SLANG_CHECK(boundedBytes[i] == 0xa5);

    // A Slice 3b-sized V2 provider remains usable for diagnostics, but none of the appended
    // scalar-memory wrappers may call beyond the prefix the provider reported.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsSerializationDiagnostics());
        SLANG_CHECK(!builder.supportsScalarOperations());
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == SLANG_NVVM_BUILDER_API_V2_MIN_SIZE);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-old-v2-module"), scope.module)));

        ComPtr<ISlangBlob> bitcode;
        String diagnostics = "stale diagnostics";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode,
            diagnostics)));
        SLANG_CHECK_ABORT(bitcode != nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);

        SlangNVVMTypeHandle_1 integerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(integerType == nullptr);

        SlangNVVMTypeHandle_1 pointerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getPointerType(
                scope.module,
                _getFakeNVVMBuilderIntegerType(),
                SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
                pointerType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(pointerType == nullptr);

        SlangNVVMValueHandle_1 parameter = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder
                .getFunctionParameter(scope.module, _getFakeNVVMBuilderFunction(), 7, parameter) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(parameter == nullptr);

        SlangNVVMValueHandle_1 loadedValue = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitLoad(scope.module, _getFakeNVVMBuilderParameter(), 4, loadedValue) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(loadedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(
                scope.module,
                _getFakeNVVMBuilderLoad(),
                _getFakeNVVMBuilderParameter(),
                4) == SLANG_E_NOT_AVAILABLE);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A provider may expose exactly the old prefix or the complete scalar prefix, but no
    // intermediate byte count can describe a coherent callable capability.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder rejectedBuilder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, rejectedBuilder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!rejectedBuilder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // Claiming the complete scalar prefix makes every function in that prefix mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitStore = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder rejectedBuilder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, rejectedBuilder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!rejectedBuilder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A complete scalar prefix is forwarded exactly through the host wrappers.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-scalar-v2-module"), scope.module)));

        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SlangNVVMValueHandle_1 parameter = nullptr;
        SlangNVVMValueHandle_1 loadedValue = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK(integerType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        SLANG_CHECK(pointerType == _getFakeNVVMBuilderPointerType());
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 scalarFunctionType = nullptr;
        SlangNVVMValueHandle_1 scalarFunction = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionType(scope.module, voidType, &pointerType, 1, scalarFunctionType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            scalarFunctionType,
            toSlice("fakeScalarMemory"),
            scalarFunction)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(scope.module, scalarFunction, 0, parameter)));
        SLANG_CHECK(parameter == _getFakeNVVMBuilderParameter());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitLoad(scope.module, parameter, 4, loadedValue)));
        SLANG_CHECK(loadedValue == _getFakeNVVMBuilderLoad());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitStore(scope.module, loadedValue, parameter, 4)));

        gFakeNVVMBuilder.returnNullIntegerType = true;
        integerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_FAIL);
        SLANG_CHECK(integerType == nullptr);
        gFakeNVVMBuilder.returnNullIntegerType = false;

        gFakeNVVMBuilder.failIntegerTypeAfterWrite = true;
        integerType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(builder.getIntegerType(scope.module, 32, integerType) == SLANG_FAIL);
        SLANG_CHECK(integerType == nullptr);
        gFakeNVVMBuilder.failIntegerTypeAfterWrite = false;

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitWidth == 32);
        SLANG_CHECK(gFakeNVVMBuilder.pointerAddressSpace == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    // A future provider reports its complete size, but the host retains only its local prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarControlFlowAPI)
{
    // An exact Slice 4 provider remains valid, but none of the appended control-flow wrappers may
    // read or call beyond that frozen prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarOperations());
        SLANG_CHECK(!builder.supportsScalarControlFlow());

        SlangNVVMValueHandle_1 value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBinary(
                _getFakeNVVMBuilderModule(),
                SLANG_NVVM_INTEGER_BINARY_OP_ADD,
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(),
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerSignedLessThan(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(),
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            builder.emitBranch(_getFakeNVVMBuilderModule(), _getFakeNVVMBuilderBlock()) ==
            SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(
            builder.emitConditionalBranch(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerComparison(),
                _getFakeNVVMBuilderBlock(),
                _getFakeNVVMBuilderBlock()) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A byte count inside the new all-or-none block is malformed.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Claiming the complete prefix makes all four operations mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitConditionalBranch = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A complete prefix is forwarded exactly, and a provider failure cannot leak a written
    // output handle through the host wrapper.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarControlFlow());
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-control-flow=1") >= 0);

        SlangNVVMTypeHandle_1 integerFunctionType = nullptr;
        const SlangNVVMTypeHandle_1 integerType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            _getFakeNVVMBuilderModule(),
            integerType,
            &integerType,
            1,
            integerFunctionType)));
        SlangNVVMValueHandle_1 integerFunction = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            _getFakeNVVMBuilderModule(),
            integerFunctionType,
            toSlice("fakeControlFunction"),
            integerFunction)));
        SlangNVVMValueHandle_1 integerParameter = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(
            _getFakeNVVMBuilderModule(),
            integerFunction,
            0,
            integerParameter)));

        SlangNVVMValueHandle_1 binary = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
            _getFakeNVVMBuilderModule(),
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            integerParameter,
            integerParameter,
            binary)));
        SLANG_CHECK(binary == _getFakeNVVMBuilderIntegerBinary());

        SlangNVVMValueHandle_1 condition = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(
            _getFakeNVVMBuilderModule(),
            integerParameter,
            binary,
            condition)));
        SLANG_CHECK(condition == _getFakeNVVMBuilderIntegerComparison());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitBranch(_getFakeNVVMBuilderModule(), _getFakeNVVMBuilderBlock())));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitConditionalBranch(
            _getFakeNVVMBuilderModule(),
            condition,
            _getFakeNVVMBuilderBlock(),
            _getFakeNVVMBuilderBlock())));

        gFakeNVVMBuilder.failIntegerBinaryAfterWrite = true;
        binary = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBinary(
                _getFakeNVVMBuilderModule(),
                SLANG_NVVM_INTEGER_BINARY_OP_SUB,
                integerParameter,
                integerParameter,
                binary) == SLANG_FAIL);
        SLANG_CHECK(binary == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarSSAAPI)
{
    // The exact Slice 7 provider remains valid and cannot be called through the appended wrappers.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarControlFlow());
        SLANG_CHECK(!builder.supportsScalarSSA());

        SlangNVVMValueHandle_1 value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.getIntegerConstant(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerType(),
                1,
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        value = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerPhi(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderBlock(),
                _getFakeNVVMBuilderIntegerType(),
                value) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(value == nullptr);
        SLANG_CHECK(
            builder.addIntegerPhiIncoming(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerPhi(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderBlock()) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The three Slice 8 functions form one all-or-none prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.addIntegerPhiIncoming = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete prefix forwards values and sanitizes provider-written outputs on failure.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarSSA());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-ssa=1") >= 0);

        SlangNVVMValueHandle_1 constant = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerConstant(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderIntegerType(),
            -17,
            constant)));
        SLANG_CHECK(constant == _getFakeNVVMBuilderIntegerConstant());

        SlangNVVMValueHandle_1 phi = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerPhi(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderBlock(1),
            _getFakeNVVMBuilderIntegerType(),
            phi)));
        SLANG_CHECK(phi == _getFakeNVVMBuilderIntegerPhi());
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(
            _getFakeNVVMBuilderModule(),
            phi,
            constant,
            _getFakeNVVMBuilderBlock())));

        gFakeNVVMBuilder.failIntegerConstantAfterWrite = true;
        constant = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.getIntegerConstant(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerType(),
                5,
                constant) == SLANG_FAIL);
        SLANG_CHECK(constant == nullptr);
        gFakeNVVMBuilder.failIntegerConstantAfterWrite = false;

        gFakeNVVMBuilder.failIntegerPhiAfterWrite = true;
        phi = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerPhi(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderBlock(2),
                _getFakeNVVMBuilderIntegerType(),
                phi) == SLANG_FAIL);
        SLANG_CHECK(phi == nullptr);

        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == -17);
        SLANG_CHECK(gFakeNVVMBuilder.integerPhiTargetBlockIndices[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.integerPhiIncomingValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::IntegerConstant);
        SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[0] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarFunctionAPI)
{
    // An exact Slice 8 provider remains valid, but the appended call/valued-return wrappers are
    // unavailable and must not dispatch through the shorter table.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarSSA());
        SLANG_CHECK(!builder.supportsScalarFunctions());

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        const SlangNVVMValueHandle_1 argument = _getFakeNVVMBuilderParameter();
        SLANG_CHECK(
            builder.emitIntegerCall(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderFunction(),
                &argument,
                1,
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(
            builder.emitIntegerReturn(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter()) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The two Slice 9 functions form one all-or-none prefix.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitIntegerCall = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitIntegerReturn = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A complete prefix forwards the direct-call graph and clears a provider-written result when
    // the provider reports failure.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarFunctions());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-functions=1") >= 0);

        SlangNVVMTypeHandle_1 integerFunctionType = nullptr;
        const SlangNVVMTypeHandle_1 parameterType = _getFakeNVVMBuilderIntegerType();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            _getFakeNVVMBuilderModule(),
            _getFakeNVVMBuilderIntegerType(),
            &parameterType,
            1,
            integerFunctionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            _getFakeNVVMBuilderModule(),
            integerFunctionType,
            toSlice("fakeScalarHelper"),
            function)));
        SlangNVVMValueHandle_1 parameter = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.getFunctionParameter(_getFakeNVVMBuilderModule(), function, 0, parameter)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createBlock(_getFakeNVVMBuilderModule(), function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.setInsertBlock(_getFakeNVVMBuilderModule(), block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitIntegerCall(_getFakeNVVMBuilderModule(), function, &parameter, 1, result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderCall());
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitIntegerReturn(_getFakeNVVMBuilderModule(), result)));

        gFakeNVVMBuilder.failIntegerCallAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerCall(_getFakeNVVMBuilderModule(), function, &parameter, 1, result) ==
            SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        gFakeNVVMBuilder.failIntegerReturn = true;
        SLANG_CHECK(
            builder.emitIntegerReturn(_getFakeNVVMBuilderModule(), parameter) == SLANG_FAIL);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarPointerArithmeticAPI)
{
    // An exact Slice 9 provider remains valid, but the appended pointer-offset wrapper cannot
    // dispatch beyond that frozen prefix and must clear its output locally.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarFunctions());
        SLANG_CHECK(!builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=0") >= 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitPointerOffset(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // No byte count inside the one-function suffix describes a coherent capability.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Claiming the complete suffix makes its only operation mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitPointerOffset = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A complete suffix forwards the exact base/index topology. The fake validates parameter
    // roles through their owning function type, and the host clears provider-written failures.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-pointer-arithmetic=1") >= 0);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-pointer-offset-module"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, pointerType, integerType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakePointerOffset"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 source = nullptr;
        SlangNVVMValueHandle_1 index = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, source)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, index)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitPointerOffset(scope.module, destination, index, result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderPointerOffset());
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs[0].index == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs[0].index == 2);

        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitPointerOffset(scope.module, index, index, result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitPointerOffset(scope.module, source, source, result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);

        gFakeNVVMBuilder.failPointerOffsetAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.emitPointerOffset(scope.module, source, index, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 2);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarArrayAddressingAPI)
{
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitIntegerMultiply) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE);

    // The frozen Slice 10 prefix remains valid and cannot dispatch either array operation.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarPointerArithmetic());
        SLANG_CHECK(!builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=0") >= 0);

        SlangNVVMTypeHandle_1 arrayType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getArrayType(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderIntegerType(),
                4,
                arrayType) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(arrayType == nullptr);
        SlangNVVMValueHandle_1 elementPointer = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                elementPointer) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(elementPointer == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A byte count ending inside either appended function pointer is not a coherent prefix.
    const uint32_t partialSizes[] = {
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE + 1),
        uint32_t(offsetof(SlangNVVMBuilderAPI_V2, emitArrayElementPointer) + 1),
    };
    for (uint32_t partialSize : partialSizes)
    {
        gFakeNVVMBuilder.reset();
        gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
        gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
        gFakeNVVMBuilder.apiV2.structureSize = partialSize;
        gFakeNVVMBuilder.omitAPIV2Symbol = false;
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
        loader.setNull();
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    // Both operations are mandatory once the complete Slice 11 prefix is advertised.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.getArrayType = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitArrayElementPointer = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete suffix forwards exact type and value identities. Failed provider calls never
    // leak their written handles through the host wrappers.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-array-addressing-module"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 arrayType = nullptr;
        SlangNVVMTypeHandle_1 arrayPointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getArrayType(scope.module, integerType, 4, arrayType)));
        SLANG_CHECK(arrayType == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == integerType);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            arrayType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            arrayPointerType)));
        SLANG_CHECK(arrayPointerType == _getFakeNVVMBuilderArrayPointerType());

        const SlangNVVMTypeHandle_1 parameterTypes[] = {
            arrayPointerType,
            arrayPointerType,
            integerType,
        };
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeArrayAddressing"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 source = nullptr;
        SlangNVVMValueHandle_1 index = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, source)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, index)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 destinationElement = nullptr;
        SlangNVVMValueHandle_1 sourceElement = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitArrayElementPointer(scope.module, destination, index, destinationElement)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.emitArrayElementPointer(scope.module, source, index, sourceElement)));
        SLANG_CHECK(destinationElement == _getFakeNVVMBuilderArrayElementPointer(0));
        SLANG_CHECK(sourceElement == _getFakeNVVMBuilderArrayElementPointer(1));
        SlangNVVMValueHandle_1 loadedValue = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitLoad(scope.module, sourceElement, 4, loadedValue)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.emitStore(scope.module, loadedValue, destinationElement, 4)));
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, index, index, result) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, source, source, result) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);

        gFakeNVVMBuilder.failArrayTypeAfterWrite = true;
        SlangNVVMTypeHandle_1 failedArrayType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getArrayType(scope.module, integerType, 4, failedArrayType) == SLANG_FAIL);
        SLANG_CHECK(failedArrayType == nullptr);
        gFakeNVVMBuilder.failArrayTypeAfterWrite = false;

        gFakeNVVMBuilder.returnNullArrayType = true;
        failedArrayType = _getFakeNVVMBuilderVoidType();
        SLANG_CHECK(
            builder.getArrayType(scope.module, integerType, 4, failedArrayType) == SLANG_FAIL);
        SLANG_CHECK(failedArrayType == nullptr);
        gFakeNVVMBuilder.returnNullArrayType = false;

        gFakeNVVMBuilder.returnNullArrayElementPointer = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, source, index, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        gFakeNVVMBuilder.returnNullArrayElementPointer = false;

        gFakeNVVMBuilder.failArrayElementPointerAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitArrayElementPointer(scope.module, source, index, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 6);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount() == 4);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarIntegerMultiplyAPI)
{
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitAnd) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE);

    // The frozen Slice 11 prefix retains array addressing but cannot dispatch multiplication.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(!builder.supportsScalarIntegerMultiply());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-integer-multiply=0") >= 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerMultiply(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerMultiplyCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // No byte count inside the one-function suffix describes a coherent capability.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE + 1);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // Claiming the complete suffix makes its dedicated operation mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitIntegerMultiply = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A future provider is accepted, advertises multiplication, and is clamped to the known table.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerMultiply());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-integer-multiply=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete suffix forwards exact parameter identities. The host rejects success without
    // a handle and clears provider-written handles from failed calls.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerMultiply());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-integer-multiply=1") >= 0);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-integer-multiply-module"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeIntegerMultiply"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 x = nullptr;
        SlangNVVMValueHandle_1 y = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, x)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, y)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerMultiply(scope.module, x, y, result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderIntegerMultiply());
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyCallerBlockIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.integerMultiplyLeftValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyLeftValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyLeftValueRefs[0].index == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.integerMultiplyRightValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyRightValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyRightValueRefs[0].index == 2);

        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerMultiply(scope.module, destination, y, result) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerMultiply(scope.module, x, destination, result) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);

        gFakeNVVMBuilder.returnNullIntegerMultiply = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.emitIntegerMultiply(scope.module, x, y, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        gFakeNVVMBuilder.returnNullIntegerMultiply = false;

        gFakeNVVMBuilder.failIntegerMultiplyAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.emitIntegerMultiply(scope.module, x, y, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerMultiplyCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyLeftValueRefs.getCount() == 3);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyRightValueRefs.getCount() == 3);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesScalarIntegerBitAndAPI)
{
    SLANG_CHECK(
        offsetof(SlangNVVMBuilderAPI_V2, emitIntegerBitAnd) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE);
    SLANG_CHECK(
        sizeof(SlangNVVMBuilderAPI_V2) ==
        SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE);

    // An uninitialized host cannot dispatch the operation and must not expose a stale handle.
    {
        NVVMIRBuilder builder;
        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBitAnd(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_UNINITIALIZED);
        SLANG_CHECK(result == nullptr);
    }

    // The frozen Slice 12 prefix retains multiplication but cannot dispatch bitwise AND.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerMultiply());
        SLANG_CHECK(!builder.supportsScalarIntegerBitAnd());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-integer-bit-and=0") >= 0);

        SlangNVVMValueHandle_1 result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBitAnd(
                _getFakeNVVMBuilderModule(),
                _getFakeNVVMBuilderParameter(),
                _getFakeNVVMBuilderParameter(1),
                result) == SLANG_E_NOT_AVAILABLE);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBitAndCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // No byte count inside the one-function suffix describes a coherent capability.
    for (uint32_t partialSize =
             uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE + 1);
         partialSize < uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_BIT_AND_MIN_SIZE);
         ++partialSize)
    {
        gFakeNVVMBuilder.reset();
        gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
        gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
        gFakeNVVMBuilder.apiV2.structureSize = partialSize;
        gFakeNVVMBuilder.omitAPIV2Symbol = false;
        {
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
            NVVMIRBuilder builder;
            SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
            SLANG_CHECK(!builder.isInitialized());
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    }

    // Claiming the complete suffix makes its dedicated operation mandatory.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.emitIntegerBitAnd = nullptr;
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK(NVVMIRBuilder::load(String(), loader, builder) == SLANG_E_NO_INTERFACE);
        SLANG_CHECK(!builder.isInitialized());
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // A future provider is accepted, advertises bitwise AND, and is clamped to the known table.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerBitAnd());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-integer-bit-and=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);

    // The complete suffix forwards exact parameter identities. The host rejects success without
    // a handle and clears provider-written handles from failed calls.
    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    gFakeNVVMBuilder.omitAPIV2Symbol = false;
    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.supportsScalarIntegerBitAnd());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-integer-bit-and=1") >= 0);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder.createModule(toSlice("fake-integer-bit-and-module"), scope.module)));
        SlangNVVMTypeHandle_1 voidType = nullptr;
        SlangNVVMTypeHandle_1 integerType = nullptr;
        SlangNVVMTypeHandle_1 pointerType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(scope.module, voidType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(scope.module, 32, integerType)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
            scope.module,
            integerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            pointerType)));
        const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
        SlangNVVMTypeHandle_1 functionType = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
            scope.module,
            voidType,
            parameterTypes,
            SLANG_COUNT_OF(parameterTypes),
            functionType)));
        SlangNVVMValueHandle_1 function = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            scope.module,
            functionType,
            toSlice("fakeIntegerBitAnd"),
            function)));
        SlangNVVMValueHandle_1 destination = nullptr;
        SlangNVVMValueHandle_1 x = nullptr;
        SlangNVVMValueHandle_1 y = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 0, destination)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 1, x)));
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.getFunctionParameter(scope.module, function, 2, y)));
        SlangNVVMBlockHandle_1 block = nullptr;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createBlock(scope.module, function, toSlice("entry"), block)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(scope.module, block)));

        SlangNVVMValueHandle_1 result = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBitAnd(scope.module, x, y, result)));
        SLANG_CHECK(result == _getFakeNVVMBuilderIntegerBitAnd());
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndCallerBlockIndices[0] == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.integerBitAndLeftValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndLeftValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndLeftValueRefs[0].index == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.integerBitAndRightValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndRightValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndRightValueRefs[0].index == 2);

        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBitAnd(scope.module, destination, y, result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(
            builder.emitIntegerBitAnd(scope.module, x, destination, result) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(result == nullptr);

        gFakeNVVMBuilder.returnNullIntegerBitAnd = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.emitIntegerBitAnd(scope.module, x, y, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        gFakeNVVMBuilder.returnNullIntegerBitAnd = false;

        gFakeNVVMBuilder.failIntegerBitAndAfterWrite = true;
        result = _getFakeNVVMBuilderFunction();
        SLANG_CHECK(builder.emitIntegerBitAnd(scope.module, x, y, result) == SLANG_FAIL);
        SLANG_CHECK(result == nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBitAndCallCount == 5);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndLeftValueRefs.getCount() == 3);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndRightValueRefs.getCount() == 3);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderNegotiatesV2Diagnostics)
{
    gFakeNVVMBuilder.reset();
    {
        ComPtr<ISlangSharedLibrary> library(new FakeNVVMBuilderLibrary);
        NVVMIRBuilder rejectedBuilder;

        SlangNVVMBuilderAPI_V2 invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE - 1);
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);

        invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.abiVersion += 1;
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);

        invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.serializeModuleWithDiagnostics = nullptr;
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);

        invalidAPI = _makeFakeNVVMBuilderAPIV2();
        invalidAPI.baseAPI.llvmVersionMajor = 15;
        SLANG_CHECK(
            NVVMIRBuilder::initialize(invalidAPI, library, rejectedBuilder) ==
            SLANG_E_NO_INTERFACE);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);

    gFakeNVVMBuilder.reset();
    gFakeNVVMBuilder.api = _makeFakeNVVMBuilderAPI();
    gFakeNVVMBuilder.apiV2 = _makeFakeNVVMBuilderAPIV2();
    // A provider may append fields to V2. The host accepts its known prefix without reading the
    // unknown tail, then clamps the retained table size to the local prefix it actually stores.
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(sizeof(SlangNVVMBuilderAPI_V2) + 16);
    gFakeNVVMBuilder.omitAPIV2Symbol = false;

    {
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMBuilderLoader);
        NVVMIRBuilder builder;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(NVVMIRBuilder::load(String(), loader, builder)));
        loader.setNull();
        SLANG_CHECK(builder.isInitialized());
        SLANG_CHECK(builder.supportsSerializationDiagnostics());
        SLANG_CHECK(builder.supportsScalarArrayAddressing());
        SLANG_CHECK(builder.getVersionString().indexOf("scalar-array-addressing=1") >= 0);
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
        SLANG_CHECK(builder.getAPI().llvmVersionMajor == 14);

        ScopedNVVMBuilderModule scope;
        scope.builder = &builder;
        SLANG_CHECK_ABORT(
            SLANG_SUCCEEDED(builder.createModule(toSlice("fake-v2-module"), scope.module)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _populateEmptyNVVMKernel(builder, scope.module, toSlice("fakeV2Kernel"))));

        ComPtr<ISlangBlob> bitcode;
        String diagnostics = "stale diagnostics";
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode,
            diagnostics)));
        SLANG_CHECK_ABORT(bitcode != nullptr);
        SLANG_CHECK_ABORT(bitcode->getBufferSize() == sizeof(kFakeNVVMBuilderBitcode));
        SLANG_CHECK(
            ::memcmp(
                bitcode->getBufferPointer(),
                kFakeNVVMBuilderBitcode,
                sizeof(kFakeNVVMBuilderBitcode)) == 0);
        SLANG_CHECK(diagnostics.getLength() == 0);

        // Make both result buffers non-empty, then undersize only the diagnostic buffer. Atomic
        // failure means the otherwise sufficient serialization buffer must retain its sentinels.
        gFakeNVVMBuilder.verificationDiagnostic = "fake valid verification note";
        size_t requiredSerializedSize = 0;
        size_t requiredDiagnosticSize = 0;
        SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(gFakeNVVMBuilder.apiV2.serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &requiredSerializedSize,
            nullptr,
            0,
            &requiredDiagnosticSize,
            &verificationStatus)));
        SLANG_CHECK(requiredSerializedSize == sizeof(kFakeNVVMBuilderBitcode));
        SLANG_CHECK(requiredDiagnosticSize > 1);
        SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);

        List<uint8_t> serializedSentinels;
        serializedSentinels.setCount(Index(requiredSerializedSize));
        for (auto& value : serializedSentinels)
            value = 0xa5;
        uint8_t diagnosticSentinel = 0x5a;
        size_t reportedSerializedSize = 0;
        size_t reportedDiagnosticSize = 0;
        verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
        SLANG_CHECK(
            gFakeNVVMBuilder.apiV2.serializeModuleWithDiagnostics(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                serializedSentinels.getBuffer(),
                size_t(serializedSentinels.getCount()),
                &reportedSerializedSize,
                &diagnosticSentinel,
                1,
                &reportedDiagnosticSize,
                &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
        SLANG_CHECK(reportedSerializedSize == requiredSerializedSize);
        SLANG_CHECK(reportedDiagnosticSize == requiredDiagnosticSize);
        SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
        for (const auto value : serializedSentinels)
            SLANG_CHECK(value == 0xa5);
        SLANG_CHECK(diagnosticSentinel == 0x5a);

        diagnostics = "stale diagnostics";
        bitcode.setNull();
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            bitcode,
            diagnostics)));
        SLANG_CHECK_ABORT(bitcode != nullptr);
        SLANG_CHECK(diagnostics == gFakeNVVMBuilder.verificationDiagnostic);

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        static const char kEmbeddedDiagnostic[] =
            {'f', 'a', 'k', 'e', 0, 'L', 'L', 'V', 'M', ' ', 'e', 'r', 'r', 'o', 'r'};
        gFakeNVVMBuilder.verificationDiagnostic =
            String(UnownedStringSlice(kEmbeddedDiagnostic, SLANG_COUNT_OF(kEmbeddedDiagnostic)));
        diagnostics = "stale diagnostics";
        bitcode.setNull();
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK_ABORT(diagnostics.getLength() == SLANG_COUNT_OF(kEmbeddedDiagnostic));
        SLANG_CHECK(
            ::memcmp(
                diagnostics.getBuffer(),
                kEmbeddedDiagnostic,
                SLANG_COUNT_OF(kEmbeddedDiagnostic)) == 0);

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
        gFakeNVVMBuilder.verificationDiagnostic = String();
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);

        // Malformed successful responses are rejected after the query, before the wrapper asks the
        // provider to write either buffer.
        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        const int emptyInvalidQueryCount = gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount;
        const int emptyInvalidWriteCount = gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == emptyInvalidQueryCount + 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == emptyInvalidWriteCount);

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
        gFakeNVVMBuilder.omitValidSerializedOutput = true;
        const int emptyValidQueryCount = gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount;
        const int emptyValidWriteCount = gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == emptyValidQueryCount + 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == emptyValidWriteCount);
        gFakeNVVMBuilder.omitValidSerializedOutput = false;

        gFakeNVVMBuilder.serializationWithDiagnosticsResult = SLANG_FAIL;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        gFakeNVVMBuilder.serializationWithDiagnosticsResult = SLANG_OK;
        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_VALID;

        gFakeNVVMBuilder.reportMismatchedSerializedDiagnosticWriteSize = true;
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        gFakeNVVMBuilder.reportMismatchedSerializedDiagnosticWriteSize = false;

        gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
        gFakeNVVMBuilder.verificationDiagnostic = "fake stable diagnostic";
        gFakeNVVMBuilder.reportMismatchedVerificationDiagnosticWriteSize = true;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        gFakeNVVMBuilder.reportMismatchedVerificationDiagnosticWriteSize = false;

        gFakeNVVMBuilder.reportMismatchedVerificationStatus = true;
        diagnostics = "stale diagnostics";
        SLANG_CHECK(
            builder.serializeModule(
                scope.module,
                SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
                bitcode,
                diagnostics) == SLANG_FAIL);
        SLANG_CHECK(bitcode == nullptr);
        SLANG_CHECK(diagnostics.getLength() == 0);
        gFakeNVVMBuilder.reportMismatchedVerificationStatus = false;

        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount > 0);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount > 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderSerializesEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    const SlangNVVMBuilderAPI_V1& api = builder.getAPI();
    SLANG_CHECK(api.llvmVersionMajor == 14);
    SLANG_CHECK(api.llvmVersionMinor == 0);
    SLANG_CHECK(api.llvmVersionPatch == 6);
    SLANG_CHECK(api.nvvmIRVersionMajor == 2);
    SLANG_CHECK(api.nvvmIRVersionMinor == 0);
    SLANG_CHECK(api.pointerModel == SLANG_NVVM_POINTER_MODEL_TYPED);

    static const char kKernelName[] = "slangSlice3aEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);

    const String assembly(UnownedStringSlice(
        static_cast<const char*>(assemblyBlob->getBufferPointer()),
        assemblyBlob->getBufferSize()));
    static const char kExpectedDataLayout[] =
        "target datalayout = \"e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
        "i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-"
        "v128:128:128-n16:32:64\"";
    SLANG_CHECK(assembly.indexOf(kExpectedDataLayout) >= 0);
    SLANG_CHECK(assembly.indexOf("target triple = \"nvptx64-nvidia-cuda\"") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @slangSlice3aEmpty()") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvm.annotations") >= 0);
    SLANG_CHECK(assembly.indexOf("void ()* @slangSlice3aEmpty") >= 0);
    SLANG_CHECK(assembly.indexOf("!\"kernel\", i32 1") >= 0);

    SLANG_CHECK(bitcodeBlob->getBufferSize() > 4);
    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, sizeof(kBitcodeMagic)) == 0);
    bool hasEmbeddedNull = false;
    const uint8_t* bitcodeBytes = static_cast<const uint8_t*>(bitcodeBlob->getBufferPointer());
    for (size_t i = 0; i < bitcodeBlob->getBufferSize(); ++i)
        hasEmbeddedNull = hasEmbeddedNull || bitcodeBytes[i] == 0;
    SLANG_CHECK(hasEmbeddedNull);
}

SLANG_UNIT_TEST(nvvmIRBuilderRealProviderPreservesShortBuffers)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsSerializationDiagnostics());

    ScopedNVVMBuilderModule scope;
    scope.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("real-short-buffer"), scope.module)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _populateEmptyNVVMKernel(builder, scope.module, toSlice("realShortBufferKernel"))));

    size_t requiredLegacySize = 0;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getAPI().serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &requiredLegacySize)));
    uint8_t legacySentinels[8];
    ::memset(legacySentinels, 0xa5, sizeof(legacySentinels));
    size_t reportedLegacySize = 0;
    SLANG_CHECK(requiredLegacySize > sizeof(legacySentinels));
    SLANG_CHECK(
        builder.getAPI().serializeModule(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            legacySentinels,
            sizeof(legacySentinels),
            &reportedLegacySize) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedLegacySize == requiredLegacySize);
    for (const auto value : legacySentinels)
        SLANG_CHECK(value == 0xa5);

    const SlangNVVMBuilderAPI_V2* v2API = builder.getAPIV2();
    SLANG_CHECK_ABORT(v2API != nullptr);
    SLANG_CHECK_ABORT(v2API->serializeModuleWithDiagnostics != nullptr);

    size_t requiredSerializedSize = 0;
    size_t requiredDiagnosticSize = 0;
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(v2API->serializeModuleWithDiagnostics(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &requiredSerializedSize,
        nullptr,
        0,
        &requiredDiagnosticSize,
        &verificationStatus)));
    SLANG_CHECK(requiredSerializedSize == requiredLegacySize);
    SLANG_CHECK(requiredDiagnosticSize == 0);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);

    // A query has both destinations null. Supplying even an otherwise-unneeded diagnostic
    // destination makes this a write, so omitting the non-empty serialized destination must fail
    // without touching the diagnostic sentinel.
    uint8_t mixedDiagnosticSentinel = 0x3c;
    size_t mixedSerializedSize = 0;
    size_t mixedDiagnosticSize = 1;
    verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            nullptr,
            0,
            &mixedSerializedSize,
            &mixedDiagnosticSentinel,
            1,
            &mixedDiagnosticSize,
            &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(mixedSerializedSize == requiredSerializedSize);
    SLANG_CHECK(mixedDiagnosticSize == requiredDiagnosticSize);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
    SLANG_CHECK(mixedDiagnosticSentinel == 0x3c);

    uint8_t serializedSentinels[8];
    ::memset(serializedSentinels, 0x5a, sizeof(serializedSentinels));
    size_t reportedSerializedSize = 0;
    size_t reportedDiagnosticSize = 1;
    verificationStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(requiredSerializedSize > sizeof(serializedSentinels));
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            scope.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            serializedSentinels,
            sizeof(serializedSentinels),
            &reportedSerializedSize,
            nullptr,
            0,
            &reportedDiagnosticSize,
            &verificationStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedSerializedSize == requiredSerializedSize);
    SLANG_CHECK(reportedDiagnosticSize == requiredDiagnosticSize);
    SLANG_CHECK(verificationStatus == SLANG_NVVM_VERIFICATION_VALID);
    for (const auto value : serializedSentinels)
        SLANG_CHECK(value == 0x5a);

    ComPtr<ISlangBlob> bitcode;
    String diagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        scope.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        bitcode,
        diagnostics)));
    SLANG_CHECK_ABORT(bitcode != nullptr);
    SLANG_CHECK(bitcode->getBufferSize() == requiredSerializedSize);
    SLANG_CHECK(diagnostics.getLength() == 0);
}

// Exercise the module boundary's rejected input shapes with handles produced by the real LLVM 14
// implementation. These checks keep malformed LLVM IR from becoming a libNVVM diagnostic later.
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("first-module"), firstModule.module)));
    ScopedNVVMBuilderModule secondModule;
    secondModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("second-module"), secondModule.module)));

    SlangNVVMTypeHandle_1 firstVoidType = nullptr;
    SlangNVVMTypeHandle_1 secondVoidType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, firstVoidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(secondModule.module, secondVoidType)));

    SlangNVVMTypeHandle_1 invalidFunctionType = nullptr;
    SLANG_CHECK(
        builder.getFunctionType(
            firstModule.module,
            firstVoidType,
            &firstVoidType,
            1,
            invalidFunctionType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunctionType == nullptr);
    SLANG_CHECK(
        builder
            .getFunctionType(firstModule.module, secondVoidType, nullptr, 0, invalidFunctionType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunctionType == nullptr);

    SlangNVVMTypeHandle_1 firstFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(firstModule.module, firstVoidType, nullptr, 0, firstFunctionType)));
    SlangNVVMValueHandle_1 firstFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        firstModule.module,
        firstFunctionType,
        toSlice("uniqueKernel"),
        firstFunction)));

    SlangNVVMValueHandle_1 invalidFunction = nullptr;
    SLANG_CHECK(
        builder.declareFunction(
            firstModule.module,
            firstFunctionType,
            toSlice("uniqueKernel"),
            invalidFunction) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunction == nullptr);
    SLANG_CHECK(
        builder.declareFunction(
            secondModule.module,
            firstFunctionType,
            toSlice("foreignType"),
            invalidFunction) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidFunction == nullptr);

    SlangNVVMBlockHandle_1 invalidBlock = nullptr;
    SLANG_CHECK(
        builder.createBlock(
            secondModule.module,
            firstFunction,
            toSlice("foreignFunction"),
            invalidBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(invalidBlock == nullptr);

    SlangNVVMBlockHandle_1 firstBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), firstBlock)));
    SLANG_CHECK(builder.setInsertBlock(secondModule.module, firstBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.markFunctionAsKernel(secondModule.module, firstFunction) == SLANG_E_INVALID_ARG);

    const SlangNVVMSerializationFormat_1 unknownFormat =
        SlangNVVMSerializationFormat_1(SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE + 1);
    size_t legacyUnknownFormatSize = 1;
    SLANG_CHECK(
        builder.getAPI().serializeModule(
            firstModule.module,
            unknownFormat,
            nullptr,
            0,
            &legacyUnknownFormatSize) == SLANG_FAIL);
    SLANG_CHECK(legacyUnknownFormatSize == 1);

    const SlangNVVMBuilderAPI_V2* v2API = builder.getAPIV2();
    SLANG_CHECK_ABORT(v2API != nullptr);
    SLANG_CHECK_ABORT(v2API->serializeModuleWithDiagnostics != nullptr);

    size_t v2UnknownFormatSerializedSize = 1;
    size_t v2UnknownFormatDiagnosticSize = 1;
    SlangNVVMVerificationStatus_2 v2UnknownFormatStatus = SLANG_NVVM_VERIFICATION_VALID;
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            firstModule.module,
            unknownFormat,
            nullptr,
            0,
            &v2UnknownFormatSerializedSize,
            nullptr,
            0,
            &v2UnknownFormatDiagnosticSize,
            &v2UnknownFormatStatus) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(v2UnknownFormatSerializedSize == 0);
    SLANG_CHECK(v2UnknownFormatDiagnosticSize == 0);
    SLANG_CHECK(v2UnknownFormatStatus == SLANG_NVVM_VERIFICATION_NOT_RUN);

    size_t invalidSerializedSize = 1;
    size_t invalidDiagnosticSize = 0;
    SlangNVVMVerificationStatus_2 invalidStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(v2API->serializeModuleWithDiagnostics(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        nullptr,
        0,
        &invalidSerializedSize,
        nullptr,
        0,
        &invalidDiagnosticSize,
        &invalidStatus)));
    SLANG_CHECK(invalidSerializedSize == 0);
    SLANG_CHECK(invalidDiagnosticSize > 0);
    SLANG_CHECK(invalidStatus == SLANG_NVVM_VERIFICATION_INVALID);

    uint8_t invalidSerializedSentinel = 0xa5;
    uint8_t invalidDiagnosticSentinel = 0x5a;
    size_t reportedInvalidSerializedSize = 1;
    size_t reportedInvalidDiagnosticSize = 0;
    invalidStatus = SLANG_NVVM_VERIFICATION_NOT_RUN;
    SLANG_CHECK(
        v2API->serializeModuleWithDiagnostics(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            &invalidSerializedSentinel,
            1,
            &reportedInvalidSerializedSize,
            &invalidDiagnosticSentinel,
            1,
            &reportedInvalidDiagnosticSize,
            &invalidStatus) == SLANG_E_BUFFER_TOO_SMALL);
    SLANG_CHECK(reportedInvalidSerializedSize == 0);
    SLANG_CHECK(reportedInvalidDiagnosticSize == invalidDiagnosticSize);
    SLANG_CHECK(invalidStatus == SLANG_NVVM_VERIFICATION_INVALID);
    SLANG_CHECK(invalidSerializedSentinel == 0xa5);
    SLANG_CHECK(invalidDiagnosticSentinel == 0x5a);

    ComPtr<ISlangBlob> invalidBitcode;
    SLANG_CHECK(
        builder.serializeModule(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            invalidBitcode) == SLANG_FAIL);
    SLANG_CHECK(invalidBitcode == nullptr);

    String verifierDiagnostics = "stale diagnostics";
    SLANG_CHECK(
        builder.serializeModule(
            firstModule.module,
            SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
            invalidBitcode,
            verifierDiagnostics) == SLANG_FAIL);
    SLANG_CHECK(invalidBitcode == nullptr);
    SLANG_CHECK(verifierDiagnostics.indexOf("does not have terminator") >= 0);
    SLANG_CHECK(verifierDiagnostics.indexOf("uniqueKernel") >= 0);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, firstBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.markFunctionAsKernel(firstModule.module, firstFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        invalidBitcode)));
    SLANG_CHECK(invalidBitcode != nullptr);

    invalidBitcode.setNull();
    verifierDiagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_BITCODE,
        invalidBitcode,
        verifierDiagnostics)));
    SLANG_CHECK(invalidBitcode != nullptr);
    SLANG_CHECK(verifierDiagnostics.getLength() == 0);
}

// Scalar-memory calls reject malformed module-owned shapes before they can insert LLVM IR.
SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-first"), firstModule.module)));
    ScopedNVVMBuilderModule secondModule;
    secondModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-second"), secondModule.module)));

    SlangNVVMTypeHandle_1 firstVoidType = nullptr;
    SlangNVVMTypeHandle_1 firstIntegerType = nullptr;
    SlangNVVMTypeHandle_1 firstGlobalPointerType = nullptr;
    SlangNVVMTypeHandle_1 firstConstantPointerType = nullptr;
    SlangNVVMTypeHandle_1 secondVoidType = nullptr;
    SlangNVVMTypeHandle_1 secondIntegerType = nullptr;
    SlangNVVMTypeHandle_1 secondGlobalPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, firstVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(firstModule.module, 32, firstIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        firstIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        firstGlobalPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        firstIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_CONSTANT,
        firstConstantPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(secondModule.module, secondVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(secondModule.module, 32, secondIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        secondModule.module,
        secondIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        secondGlobalPointerType)));

    const SlangNVVMBuilderAPI_V2* scalarAPI = builder.getAPIV2();
    SLANG_CHECK_ABORT(scalarAPI != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getIntegerType != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getPointerType != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->getFunctionParameter != nullptr);
    SLANG_CHECK_ABORT(scalarAPI->emitLoad != nullptr);
    SLANG_CHECK(scalarAPI->getIntegerType(firstModule.module, 32, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        scalarAPI->getPointerType(
            firstModule.module,
            firstIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            nullptr) == SLANG_E_INVALID_ARG);

    SlangNVVMTypeHandle_1 rejectedType = firstVoidType;
    SLANG_CHECK(builder.getIntegerType(firstModule.module, 0, rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    static const uint32_t kMaximumIntegerBitWidth = 1u << 23;
    SlangNVVMTypeHandle_1 maximumIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerType(firstModule.module, kMaximumIntegerBitWidth, maximumIntegerType)));
    SLANG_CHECK(maximumIntegerType != nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getIntegerType(firstModule.module, kMaximumIntegerBitWidth + 1, rejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            firstVoidType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            firstIntegerType,
            SlangNVVMAddressSpace_2(2),
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);
    rejectedType = firstVoidType;
    SLANG_CHECK(
        builder.getPointerType(
            firstModule.module,
            secondIntegerType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedType) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedType == nullptr);

    const SlangNVVMTypeHandle_1 firstParameterTypes[] = {
        firstGlobalPointerType,
        firstIntegerType,
        firstConstantPointerType,
    };
    SlangNVVMTypeHandle_1 firstFunctionType = nullptr;
    SlangNVVMValueHandle_1 firstFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        firstModule.module,
        firstVoidType,
        firstParameterTypes,
        SLANG_COUNT_OF(firstParameterTypes),
        firstFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        firstModule.module,
        firstFunctionType,
        toSlice("rejectInvalidScalarOperations"),
        firstFunction)));

    SlangNVVMValueHandle_1 firstDestination = nullptr;
    SlangNVVMValueHandle_1 firstValue = nullptr;
    SlangNVVMValueHandle_1 firstConstantDestination = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 0, firstDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 1, firstValue)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder
            .getFunctionParameter(firstModule.module, firstFunction, 2, firstConstantDestination)));
    SLANG_CHECK(
        scalarAPI->getFunctionParameter(firstModule.module, firstFunction, 0, nullptr) ==
        SLANG_E_INVALID_ARG);

    const SlangNVVMTypeHandle_1 secondParameterTypes[] = {
        secondGlobalPointerType,
        secondIntegerType,
    };
    SlangNVVMTypeHandle_1 secondFunctionType = nullptr;
    SlangNVVMValueHandle_1 secondFunction = nullptr;
    SlangNVVMValueHandle_1 secondDestination = nullptr;
    SlangNVVMValueHandle_1 secondValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        secondModule.module,
        secondVoidType,
        secondParameterTypes,
        SLANG_COUNT_OF(secondParameterTypes),
        secondFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        secondModule.module,
        secondFunctionType,
        toSlice("foreignScalarFunction"),
        secondFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(secondModule.module, secondFunction, 0, secondDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(secondModule.module, secondFunction, 1, secondValue)));

    SlangNVVMValueHandle_1 rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.getFunctionParameter(
            firstModule.module,
            firstFunction,
            SLANG_COUNT_OF(firstParameterTypes),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.getFunctionParameter(firstModule.module, secondFunction, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // A declared function has no insertion block yet. Both operations must fail without mutation.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, firstDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);

    SlangNVVMBlockHandle_1 firstBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), firstBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, firstBlock)));
    SLANG_CHECK(
        scalarAPI->emitLoad(firstModule.module, firstDestination, 4, nullptr) ==
        SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, firstValue, 4, rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, secondDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    static const uint32_t kInvalidAlignments[] = {0u, 3u};
    for (uint32_t invalidAlignment : kInvalidAlignments)
    {
        rejectedValue = firstFunction;
        SLANG_CHECK(
            builder
                .emitLoad(firstModule.module, firstDestination, invalidAlignment, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
        SLANG_CHECK(
            builder.emitStore(firstModule.module, firstValue, firstDestination, invalidAlignment) ==
            SLANG_E_INVALID_ARG);
    }
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstDestination, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstValue, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, secondValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, secondDestination, 4) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstConstantDestination, 4) ==
        SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, firstDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, firstValue, firstDestination, 4) ==
        SLANG_E_INVALID_ARG);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics = "stale diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf(" = load ") < 0);
    SLANG_CHECK(assembly.indexOf("\n  store ") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarControlOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarControlFlow());

    ScopedNVVMBuilderModule firstModule;
    firstModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-control-first"), firstModule.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-control-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(firstModule.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(firstModule.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        firstModule.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));
    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        firstModule.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));

    auto declareFunction = [&](const char* name, SlangNVVMValueHandle_1& outFunction)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
            firstModule.module,
            functionType,
            UnownedStringSlice(name),
            outFunction)));
    };
    SlangNVVMValueHandle_1 firstFunction = nullptr;
    SlangNVVMValueHandle_1 secondFunction = nullptr;
    declareFunction("firstControlFunction", firstFunction);
    declareFunction("secondControlFunction", secondFunction);

    SlangNVVMValueHandle_1 firstDestination = nullptr;
    SlangNVVMValueHandle_1 firstX = nullptr;
    SlangNVVMValueHandle_1 firstY = nullptr;
    SlangNVVMValueHandle_1 secondDestination = nullptr;
    SlangNVVMValueHandle_1 secondX = nullptr;
    SlangNVVMValueHandle_1 secondY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 0, firstDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 1, firstX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, firstFunction, 2, firstY)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 0, secondDestination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 1, secondX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(firstModule.module, secondFunction, 2, secondY)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignControlFunction"),
        foreignFunction)));
    SlangNVVMValueHandle_1 foreignX = nullptr;
    SlangNVVMValueHandle_1 foreignY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignY)));
    SlangNVVMBlockHandle_1 foreignBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
        foreignModule.module,
        foreignFunction,
        toSlice("foreign-entry"),
        foreignBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(foreignModule.module, foreignBlock)));
    SlangNVVMValueHandle_1 foreignCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(
        foreignModule.module,
        foreignX,
        foreignY,
        foreignCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(foreignModule.module)));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 secondBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("true"), trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("false"), falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(firstModule.module, firstFunction, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.createBlock(
        firstModule.module,
        secondFunction,
        toSlice("second-entry"),
        secondBlock)));

    SlangNVVMValueHandle_1 rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    // Produce a live i1 in a second function, then prove that values and blocks from that function
    // cannot be consumed at the first function's insertion point.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, secondBlock)));
    SlangNVVMValueHandle_1 secondCondition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(firstModule.module, secondX, secondY, secondCondition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, entryBlock)));
    const SlangNVVMBuilderAPI_V2* controlAPI = builder.getAPIV2();
    SLANG_CHECK_ABORT(controlAPI != nullptr);
    SLANG_CHECK(
        controlAPI->emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstY,
            nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        controlAPI->emitIntegerSignedLessThan(firstModule.module, firstX, firstY, nullptr) ==
        SLANG_E_INVALID_ARG);

    // Context ownership is stricter than function ownership: values, conditions, and blocks from
    // another provider module must be rejected before any first-module instruction is created.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            foreignX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerSignedLessThan(firstModule.module, firstY, foreignY, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder
            .emitConditionalBranch(firstModule.module, foreignCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, foreignBlock) == SLANG_E_INVALID_ARG);

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SlangNVVMIntegerBinaryOp_2(SLANG_NVVM_INTEGER_BINARY_OP_SUB + 1),
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstDestination,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            secondX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerSignedLessThan(firstModule.module, firstX, secondY, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, secondX, firstDestination, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitLoad(firstModule.module, secondDestination, 4, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitConditionalBranch(firstModule.module, firstX, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitConditionalBranch(firstModule.module, secondCondition, trueBlock, falseBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitBranch(firstModule.module, secondBlock) == SLANG_E_INVALID_ARG);

    // Every rejected call above must leave the entry block untouched, so one valid graph still
    // verifies and contains exactly the instructions deliberately emitted below.
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerSignedLessThan(firstModule.module, firstX, firstY, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(firstModule.module, condition, trueBlock, falseBlock)));

    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            firstX,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitBranch(firstModule.module, mergeBlock) == SLANG_E_INVALID_ARG);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, trueBlock)));
    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        firstModule.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        firstX,
        firstY,
        sum)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, sum, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, falseBlock)));
    // `sum` belongs to the sibling true block and does not dominate this insertion point. Both
    // instruction-producing and side-effecting consumers must reject it without changing the
    // false block.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_ADD,
            sum,
            firstX,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle_1 difference = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        firstModule.module,
        SLANG_NVVM_INTEGER_BINARY_OP_SUB,
        firstX,
        firstY,
        difference)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(firstModule.module, difference, firstDestination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(firstModule.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(firstModule.module, mergeBlock)));
    // The merge is reachable without executing the true block as well, so the same value remains
    // unavailable here. The final assembly counts below prove these failures added no instructions.
    rejectedValue = firstFunction;
    SLANG_CHECK(
        builder.emitIntegerBinary(
            firstModule.module,
            SLANG_NVVM_INTEGER_BINARY_OP_SUB,
            sum,
            firstY,
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(
        builder.emitStore(firstModule.module, sum, firstDestination, 4) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(firstModule.module)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.markFunctionAsKernel(firstModule.module, firstFunction)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        firstModule.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("icmp slt i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("add i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("sub i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarSSAOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarSSA());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-ssa"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-ssa-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));

    const SlangNVVMBuilderAPI_V2* ssaAPI = builder.getAPIV2();
    SLANG_CHECK_ABORT(ssaAPI != nullptr);
    SLANG_CHECK(
        ssaAPI->getIntegerConstant(module.module, integerType, 0, nullptr) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        ssaAPI->emitIntegerPhi(module.module, nullptr, integerType, nullptr) ==
        SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle_1 rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getIntegerConstant(module.module, voidType, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getIntegerConstant(module.module, foreignIntegerType, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    static const int64_t kOutOfI32Range[] = {INT64_C(2147483648), -INT64_C(2147483649)};
    for (int64_t value : kOutOfI32Range)
    {
        rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.getIntegerConstant(module.module, integerType, value, rejectedValue) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejectedValue == nullptr);
    }
    SlangNVVMValueHandle_1 minimum = nullptr;
    SlangNVVMValueHandle_1 maximum = nullptr;
    SlangNVVMValueHandle_1 foreignOne = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(module.module, integerType, -INT64_C(2147483647) - 1, minimum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(module.module, integerType, INT64_C(2147483647), maximum)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getIntegerConstant(foreignModule.module, foreignIntegerType, 1, foreignOne)));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, integerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 secondFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(module.module, functionType, toSlice("latePhiKernel"), function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("sameModuleForeignFunction"),
        secondFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SlangNVVMValueHandle_1 secondX = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, y)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, secondFunction, 1, secondX)));

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 trueBlock = nullptr;
    SlangNVVMBlockHandle_1 falseBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 orphanBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.true"), trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.false"), falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("if.merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("orphan"), orphanBlock)));

    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerPhi(foreignModule.module, mergeBlock, integerType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerPhi(module.module, mergeBlock, voidType, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, trueBlock, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, trueBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, orphanBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SlangNVVMValueHandle_1 sum = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitIntegerBinary(module.module, SLANG_NVVM_INTEGER_BINARY_OP_ADD, x, y, sum)));
    SlangNVVMValueHandle_1 phi = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerPhi(module.module, mergeBlock, integerType, phi)));
    // Incoming validation requires the complete CFG; merge has no terminator yet.
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, x, trueBlock) == SLANG_E_INVALID_ARG);
    // The explicit target permits late phi insertion and must preserve the current insertion state.
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, phi, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    // All blocks in the phi function are now terminated. Invalid incoming calls must not mutate it.
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, condition, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, secondX, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, foreignOne, trueBlock) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, x, orphanBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, sum, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(module.module, phi, x, trueBlock)));
    SLANG_CHECK(
        builder.addIntegerPhiIncoming(module.module, phi, y, trueBlock) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.addIntegerPhiIncoming(module.module, phi, y, falseBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.markFunctionAsKernel(module.module, function)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    const Index phiIndex = assembly.indexOf("phi i32");
    const Index addIndex = assembly.indexOf("add i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(phiIndex >= 0);
    SLANG_CHECK(addIndex > phiIndex);
    SLANG_CHECK(storeIndex > addIndex);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("phi i32")) == 1);
    UnownedStringSlice phiLine = assembly.getUnownedSlice().tail(phiIndex);
    const Index phiLineEnd = phiLine.indexOf(toSlice("\n"));
    if (phiLineEnd >= 0)
        phiLine = phiLine.head(phiLineEnd);
    SLANG_CHECK(_countOccurrences(phiLine, toSlice("[")) == 2);
    SLANG_CHECK(phiLine.indexOf(toSlice("%if.true")) >= 0);
    SLANG_CHECK(phiLine.indexOf(toSlice("%if.false")) >= 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidScalarFunctionOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarFunctions());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-scalar-function"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-scalar-function-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));

    SlangNVVMTypeHandle_1 helperType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, integerType, &integerType, 1, helperType)));
    SlangNVVMValueHandle_1 helper = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(module.module, helperType, toSlice("invalidCallHelper"), helper)));
    SlangNVVMValueHandle_1 helperValue = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, helper, 0, helperValue)));

    const SlangNVVMTypeHandle_1 callerParameterTypes[] = {integerType, integerType};
    SlangNVVMTypeHandle_1 callerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        integerType,
        callerParameterTypes,
        SLANG_COUNT_OF(callerParameterTypes),
        callerType)));
    SlangNVVMValueHandle_1 caller = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.declareFunction(module.module, callerType, toSlice("invalidCallCaller"), caller)));
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, caller, 0, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, caller, 1, y)));

    SlangNVVMTypeHandle_1 voidFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionType(module.module, voidType, &integerType, 1, voidFunctionType)));
    SlangNVVMValueHandle_1 voidFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        voidFunctionType,
        toSlice("invalidCallVoid"),
        voidFunction)));
    SlangNVVMValueHandle_1 voidFunctionValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, voidFunction, 0, voidFunctionValue)));

    SlangNVVMTypeHandle_1 foreignHelperType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignIntegerType,
        &foreignIntegerType,
        1,
        foreignHelperType)));
    SlangNVVMValueHandle_1 foreignHelper = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignHelperType,
        toSlice("foreignCallHelper"),
        foreignHelper)));
    SlangNVVMValueHandle_1 foreignValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignHelper, 0, foreignValue)));

    // This module has no insertion block yet. Both operations must reject without creating an
    // instruction or selecting function ownership implicitly.
    const SlangNVVMValueHandle_1 noInsertionArguments[] = {x};
    SlangNVVMValueHandle_1 noInsertionResult =
        reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(
            module.module,
            helper,
            noInsertionArguments,
            SLANG_COUNT_OF(noInsertionArguments),
            noInsertionResult) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(noInsertionResult == nullptr);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, x) == SLANG_E_INVALID_ARG);

    SlangNVVMBlockHandle_1 helperBlock = nullptr;
    SlangNVVMBlockHandle_1 callerEntry = nullptr;
    SlangNVVMBlockHandle_1 callerOther = nullptr;
    SlangNVVMBlockHandle_1 voidBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, helper, toSlice("helper.entry"), helperBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, caller, toSlice("caller.entry"), callerEntry)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, caller, toSlice("caller.other"), callerOther)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, voidFunction, toSlice("void.entry"), voidBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, helperBlock)));
    SlangNVVMValueHandle_1 one = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerConstant(module.module, integerType, 1, one)));
    SlangNVVMValueHandle_1 helperResult = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        helperValue,
        one,
        helperResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, helperResult)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerOther)));
    SlangNVVMValueHandle_1 nonDominatingValue = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        x,
        y,
        nonDominatingValue)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, nonDominatingValue)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, voidBlock)));
    SLANG_CHECK(builder.emitIntegerReturn(module.module, voidFunctionValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, callerEntry)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));

    const SlangNVVMValueHandle_1 xArgument[] = {x};
    const SlangNVVMValueHandle_1 conditionArgument[] = {condition};
    const SlangNVVMValueHandle_1 helperArgument[] = {helperValue};
    const SlangNVVMValueHandle_1 foreignArgument[] = {foreignValue};
    const SlangNVVMValueHandle_1 nonDominatingArgument[] = {nonDominatingValue};
    const SlangNVVMValueHandle_1 tooManyArguments[] = {x, y};
    SlangNVVMValueHandle_1 rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->emitIntegerCall(module.module, helper, xArgument, 1, nullptr) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, x, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, foreignHelper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, voidFunction, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nullptr, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nullptr, 0, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(
            module.module,
            helper,
            tooManyArguments,
            SLANG_COUNT_OF(tooManyArguments),
            rejectedValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, conditionArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, helperArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, foreignArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, nonDominatingArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);

    // Invalid valued returns must likewise leave caller.entry unterminated for the valid graph.
    SLANG_CHECK(builder.emitIntegerReturn(module.module, condition) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, helperValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, foreignValue) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(
        builder.emitIntegerReturn(module.module, nonDominatingValue) == SLANG_E_INVALID_ARG);

    SlangNVVMValueHandle_1 callResult = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerCall(module.module, helper, xArgument, 1, callResult)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerReturn(module.module, callResult)));
    rejectedValue = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.emitIntegerCall(module.module, helper, xArgument, 1, rejectedValue) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedValue == nullptr);
    SLANG_CHECK(builder.emitIntegerReturn(module.module, x) == SLANG_E_INVALID_ARG);

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("call i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret i32")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidPointerOffsetOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarPointerArithmetic());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-pointer-offset"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-pointer-offset-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    // The opaque ABI has no aggregate/opaque type constructor, and its only unsized exposed type
    // cannot form a pointer. This pins the construction boundary without forging provider handles.
    SlangNVVMTypeHandle_1 rejectedUnsizedPointer =
        reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getPointerType(
            module.module,
            voidType,
            SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
            rejectedUnsizedPointer) == SLANG_E_INVALID_ARG);
    SLANG_CHECK(rejectedUnsizedPointer == nullptr);

    const SlangNVVMTypeHandle_1 parameterTypes[] = {pointerType, pointerType, integerType};
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidPointerOffset"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherPointerOffset"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SlangNVVMValueHandle_1 otherDestination = nullptr;
    SlangNVVMValueHandle_1 otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherIndex)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SlangNVVMTypeHandle_1 foreignPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignIntegerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignPointerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignPointerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignPointerOffset"),
        foreignFunction)));
    SlangNVVMValueHandle_1 foreignPointer = nullptr;
    SlangNVVMValueHandle_1 foreignIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    auto expectRejectedOffset = [&](SlangNVVMModuleHandle_1 targetModule,
                                    SlangNVVMValueHandle_1 base,
                                    SlangNVVMValueHandle_1 offset)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitPointerOffset(targetModule, base, offset, rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // No insertion point and module ownership failures must be rejected before any instruction is
    // created or a function is inferred from the values.
    expectRejectedOffset(module.module, destination, index);
    expectRejectedOffset(nullptr, destination, index);
    expectRejectedOffset(foreignModule.module, destination, index);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 producerPointer = nullptr;
    SlangNVVMValueHandle_1 producerInteger = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitPointerOffset(module.module, source, index, producerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        index,
        index,
        producerInteger)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getAPIV2()->emitPointerOffset(module.module, destination, index, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedOffset(module.module, index, index);
    expectRejectedOffset(module.module, destination, source);
    expectRejectedOffset(module.module, foreignPointer, index);
    expectRejectedOffset(module.module, destination, foreignIndex);
    expectRejectedOffset(module.module, otherDestination, index);
    expectRejectedOffset(module.module, destination, otherIndex);
    expectRejectedOffset(module.module, producerPointer, index);
    expectRejectedOffset(module.module, destination, producerInteger);

    SlangNVVMValueHandle_1 consumerPointer = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitPointerOffset(module.module, destination, index, consumerPointer)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedOffset(module.module, destination, index);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i32")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidArrayAddressingOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarArrayAddressing());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-array-addressing"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-array-addressing-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 arrayType = nullptr;
    SlangNVVMTypeHandle_1 arrayPointerType = nullptr;
    SlangNVVMTypeHandle_1 scalarPointerType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SlangNVVMTypeHandle_1 foreignArrayType = nullptr;
    SlangNVVMTypeHandle_1 foreignArrayPointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getArrayType(module.module, integerType, 4, arrayType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        arrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        arrayPointerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        scalarPointerType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getArrayType(foreignModule.module, foreignIntegerType, 4, foreignArrayType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        foreignModule.module,
        foreignArrayType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        foreignArrayPointerType)));

    SLANG_CHECK(
        builder.getAPIV2()->getArrayType(module.module, integerType, 4, nullptr) ==
        SLANG_E_INVALID_ARG);
    SlangNVVMTypeHandle_1 rawRejectedType = reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->getArrayType(module.module, voidType, 4, &rawRejectedType) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejectedType == nullptr);
    auto expectRejectedArrayType =
        [&](SlangNVVMModuleHandle_1 targetModule, SlangNVVMTypeHandle_1 elementType, uint32_t count)
    {
        SlangNVVMTypeHandle_1 rejected = reinterpret_cast<SlangNVVMTypeHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.getArrayType(targetModule, elementType, count, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };
    expectRejectedArrayType(nullptr, integerType, 4);
    expectRejectedArrayType(foreignModule.module, integerType, 4);
    expectRejectedArrayType(module.module, foreignIntegerType, 4);
    // Void is the only unsized type exposed by this provider ABI.
    expectRejectedArrayType(module.module, voidType, 4);
    expectRejectedArrayType(module.module, integerType, 0);

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        arrayPointerType,
        arrayPointerType,
        scalarPointerType,
        integerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidArrayAddressing"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherArrayAddressing"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 source = nullptr;
    SlangNVVMValueHandle_1 scalarPointer = nullptr;
    SlangNVVMValueHandle_1 index = nullptr;
    SlangNVVMValueHandle_1 otherDestination = nullptr;
    SlangNVVMValueHandle_1 otherIndex = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, source)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, scalarPointer)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, index)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(module.module, otherFunction, 0, otherDestination)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 3, otherIndex)));

    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignArrayPointerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignBase = nullptr;
    SlangNVVMValueHandle_1 foreignIndex = nullptr;
    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignArrayAddressing"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignBase)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignIndex)));

    auto expectRejectedElement = [&](SlangNVVMModuleHandle_1 targetModule,
                                     SlangNVVMValueHandle_1 base,
                                     SlangNVVMValueHandle_1 elementIndex)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitArrayElementPointer(targetModule, base, elementIndex, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    expectRejectedElement(module.module, destination, index);
    SlangNVVMValueHandle_1 rawRejectedElement =
        reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()
            ->emitArrayElementPointer(module.module, destination, index, &rawRejectedElement) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejectedElement == nullptr);
    expectRejectedElement(nullptr, destination, index);
    expectRejectedElement(foreignModule.module, destination, index);
    expectRejectedElement(module.module, nullptr, index);
    expectRejectedElement(module.module, destination, nullptr);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, index, index, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 nonDominatingIndex = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        index,
        index,
        nonDominatingIndex)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getAPIV2()->emitArrayElementPointer(module.module, destination, index, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedElement(module.module, scalarPointer, index);
    expectRejectedElement(module.module, destination, source);
    expectRejectedElement(module.module, foreignBase, index);
    expectRejectedElement(module.module, destination, foreignIndex);
    expectRejectedElement(module.module, otherDestination, index);
    expectRejectedElement(module.module, destination, otherIndex);
    expectRejectedElement(module.module, destination, nonDominatingIndex);

    SlangNVVMValueHandle_1 destinationElement = nullptr;
    SlangNVVMValueHandle_1 sourceElement = nullptr;
    SlangNVVMValueHandle_1 value = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitArrayElementPointer(module.module, destination, index, destinationElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitArrayElementPointer(module.module, source, index, sourceElement)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitLoad(module.module, sourceElement, 4, value)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitStore(module.module, value, destinationElement, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedElement(module.module, destination, index);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr [4 x i32]")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("i32 0, i32 %3")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidIntegerMultiplyOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarIntegerMultiply());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-integer-multiply"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-integer-multiply-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 wideIntegerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, wideIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        pointerType,
        integerType,
        integerType,
        wideIntegerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidIntegerMultiply"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherIntegerMultiply"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SlangNVVMValueHandle_1 wide = nullptr;
    SlangNVVMValueHandle_1 otherX = nullptr;
    SlangNVVMValueHandle_1 otherY = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, y)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, wide)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 1, otherX)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherY)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignX = nullptr;
    SlangNVVMValueHandle_1 foreignY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignIntegerMultiply"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignY)));

    auto expectRejectedMultiply = [&](SlangNVVMModuleHandle_1 targetModule,
                                      SlangNVVMValueHandle_1 left,
                                      SlangNVVMValueHandle_1 right)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitIntegerMultiply(targetModule, left, right, rejected) ==
            SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // With no insertion point, every path must reject without inferring function ownership.
    expectRejectedMultiply(module.module, x, y);
    SlangNVVMValueHandle_1 rawRejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->emitIntegerMultiply(module.module, x, y, &rawRejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejected == nullptr);
    expectRejectedMultiply(nullptr, x, y);
    expectRejectedMultiply(foreignModule.module, x, y);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 nonDominating = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        x,
        y,
        nonDominating)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getAPIV2()->emitIntegerMultiply(module.module, x, y, nullptr) ==
        SLANG_E_INVALID_ARG);
    expectRejectedMultiply(module.module, nullptr, y);
    expectRejectedMultiply(module.module, x, nullptr);
    expectRejectedMultiply(module.module, destination, y);
    expectRejectedMultiply(module.module, x, destination);
    expectRejectedMultiply(module.module, x, wide);
    expectRejectedMultiply(module.module, foreignX, y);
    expectRejectedMultiply(module.module, x, foreignY);
    expectRejectedMultiply(module.module, otherX, y);
    expectRejectedMultiply(module.module, x, otherY);
    expectRejectedMultiply(module.module, nonDominating, y);
    expectRejectedMultiply(module.module, x, nonDominating);

    SlangNVVMValueHandle_1 product = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerMultiply(module.module, x, y, product)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, product, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedMultiply(module.module, x, y);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("mul i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("mul i64")) == 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    const Index multiplyIndex = assembly.indexOf("mul i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(multiplyIndex >= 0);
    SLANG_CHECK(storeIndex > multiplyIndex);
}

SLANG_UNIT_TEST(nvvmIRBuilderRejectsInvalidIntegerBitAndOperations)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarIntegerBitAnd());

    ScopedNVVMBuilderModule module;
    module.builder = &builder;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.createModule(toSlice("invalid-integer-bit-and"), module.module)));
    ScopedNVVMBuilderModule foreignModule;
    foreignModule.builder = &builder;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createModule(toSlice("invalid-integer-bit-and-foreign"), foreignModule.module)));

    SlangNVVMTypeHandle_1 voidType = nullptr;
    SlangNVVMTypeHandle_1 integerType = nullptr;
    SlangNVVMTypeHandle_1 wideIntegerType = nullptr;
    SlangNVVMTypeHandle_1 pointerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(module.module, voidType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 32, integerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getIntegerType(module.module, 64, wideIntegerType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getPointerType(
        module.module,
        integerType,
        SLANG_NVVM_ADDRESS_SPACE_GLOBAL,
        pointerType)));

    const SlangNVVMTypeHandle_1 parameterTypes[] = {
        pointerType,
        integerType,
        integerType,
        wideIntegerType,
    };
    SlangNVVMTypeHandle_1 functionType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        module.module,
        voidType,
        parameterTypes,
        SLANG_COUNT_OF(parameterTypes),
        functionType)));
    SlangNVVMValueHandle_1 function = nullptr;
    SlangNVVMValueHandle_1 otherFunction = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("invalidIntegerBitAnd"),
        function)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        module.module,
        functionType,
        toSlice("otherIntegerBitAnd"),
        otherFunction)));

    SlangNVVMValueHandle_1 destination = nullptr;
    SlangNVVMValueHandle_1 x = nullptr;
    SlangNVVMValueHandle_1 y = nullptr;
    SlangNVVMValueHandle_1 wide = nullptr;
    SlangNVVMValueHandle_1 otherX = nullptr;
    SlangNVVMValueHandle_1 otherY = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 0, destination)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 1, x)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 2, y)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, function, 3, wide)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 1, otherX)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getFunctionParameter(module.module, otherFunction, 2, otherY)));

    SlangNVVMTypeHandle_1 foreignVoidType = nullptr;
    SlangNVVMTypeHandle_1 foreignIntegerType = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getVoidType(foreignModule.module, foreignVoidType)));
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.getIntegerType(foreignModule.module, 32, foreignIntegerType)));
    const SlangNVVMTypeHandle_1 foreignParameterTypes[] = {
        foreignIntegerType,
        foreignIntegerType,
    };
    SlangNVVMTypeHandle_1 foreignFunctionType = nullptr;
    SlangNVVMValueHandle_1 foreignFunction = nullptr;
    SlangNVVMValueHandle_1 foreignX = nullptr;
    SlangNVVMValueHandle_1 foreignY = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.getFunctionType(
        foreignModule.module,
        foreignVoidType,
        foreignParameterTypes,
        SLANG_COUNT_OF(foreignParameterTypes),
        foreignFunctionType)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.declareFunction(
        foreignModule.module,
        foreignFunctionType,
        toSlice("foreignIntegerBitAnd"),
        foreignFunction)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 0, foreignX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.getFunctionParameter(foreignModule.module, foreignFunction, 1, foreignY)));

    auto expectRejectedBitAnd = [&](SlangNVVMModuleHandle_1 targetModule,
                                    SlangNVVMValueHandle_1 left,
                                    SlangNVVMValueHandle_1 right)
    {
        SlangNVVMValueHandle_1 rejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
        SLANG_CHECK(
            builder.emitIntegerBitAnd(targetModule, left, right, rejected) == SLANG_E_INVALID_ARG);
        SLANG_CHECK(rejected == nullptr);
    };

    // With no insertion point, every path must reject without inferring function ownership.
    expectRejectedBitAnd(module.module, x, y);
    SlangNVVMValueHandle_1 rawRejected = reinterpret_cast<SlangNVVMValueHandle_1>(uintptr_t(1));
    SLANG_CHECK(
        builder.getAPIV2()->emitIntegerBitAnd(module.module, x, y, &rawRejected) ==
        SLANG_E_INVALID_ARG);
    SLANG_CHECK(rawRejected == nullptr);
    expectRejectedBitAnd(nullptr, x, y);
    expectRejectedBitAnd(foreignModule.module, x, y);

    SlangNVVMBlockHandle_1 entryBlock = nullptr;
    SlangNVVMBlockHandle_1 producerBlock = nullptr;
    SlangNVVMBlockHandle_1 consumerBlock = nullptr;
    SlangNVVMBlockHandle_1 mergeBlock = nullptr;
    SlangNVVMBlockHandle_1 otherBlock = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("entry"), entryBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("producer"), producerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("consumer"), consumerBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, function, toSlice("merge"), mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.createBlock(module.module, otherFunction, toSlice("other.entry"), otherBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, entryBlock)));
    SlangNVVMValueHandle_1 condition = nullptr;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(builder.emitIntegerSignedLessThan(module.module, x, y, condition)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        builder.emitConditionalBranch(module.module, condition, producerBlock, consumerBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, producerBlock)));
    SlangNVVMValueHandle_1 nonDominating = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBinary(
        module.module,
        SLANG_NVVM_INTEGER_BINARY_OP_ADD,
        x,
        y,
        nonDominating)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, consumerBlock)));
    SLANG_CHECK(
        builder.getAPIV2()->emitIntegerBitAnd(module.module, x, y, nullptr) == SLANG_E_INVALID_ARG);
    expectRejectedBitAnd(module.module, nullptr, y);
    expectRejectedBitAnd(module.module, x, nullptr);
    expectRejectedBitAnd(module.module, destination, y);
    expectRejectedBitAnd(module.module, x, destination);
    expectRejectedBitAnd(module.module, x, wide);
    expectRejectedBitAnd(module.module, wide, x);
    expectRejectedBitAnd(module.module, foreignX, y);
    expectRejectedBitAnd(module.module, x, foreignY);
    expectRejectedBitAnd(module.module, otherX, y);
    expectRejectedBitAnd(module.module, x, otherY);
    expectRejectedBitAnd(module.module, nonDominating, y);
    expectRejectedBitAnd(module.module, x, nonDominating);

    SlangNVVMValueHandle_1 masked = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitIntegerBitAnd(module.module, x, y, masked)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitStore(module.module, masked, destination, 4)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitBranch(module.module, mergeBlock)));

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, mergeBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));
    expectRejectedBitAnd(module.module, x, y);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.setInsertBlock(module.module, otherBlock)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.emitReturnVoid(module.module)));

    ComPtr<ISlangBlob> assemblyBlob;
    String diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(builder.serializeModule(
        module.module,
        SLANG_NVVM_SERIALIZATION_FORMAT_ASSEMBLY,
        assemblyBlob,
        diagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK(diagnostics.getLength() == 0);
    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("and i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("and i64")) == 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    const Index bitAndIndex = assembly.indexOf("and i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(bitAndIndex >= 0);
    SLANG_CHECK(storeIndex > bitAndIndex);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarReferenceKernels)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("target triple = \"nvptx64-nvidia-cuda\"") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @writeScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @copyScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf(", i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("store i32 %1, i32 addrspace(1)* %0, align 4") >= 0);
    SLANG_CHECK(assembly.indexOf("load i32, i32 addrspace(1)* %1, align 4") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 3);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 2);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(assembly.indexOf("!nvvm.annotations") >= 0);
    SLANG_CHECK(assembly.indexOf("!nvvmir.version") >= 0);
    SLANG_CHECK(assembly.indexOf("@writeScalar, !\"kernel\", i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("@copyScalar, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarConditionalKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarControlFlow());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarConditionalModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @chooseScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("icmp slt i32") >= 0);
    SLANG_CHECK(assembly.indexOf("add i32") >= 0);
    SLANG_CHECK(assembly.indexOf("sub i32") >= 0);
    SLANG_CHECK(assembly.indexOf("br i1") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@chooseScalar, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarSSALoopKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarSSA());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarSSALoopModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define void @sumToLimit(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("phi i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("icmp slt i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("add i32")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("br i1")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("i32 0") >= 0);
    SLANG_CHECK(assembly.indexOf("i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("%entry") >= 0);
    SLANG_CHECK(assembly.indexOf("%loop.body") >= 0);
    SLANG_CHECK(assembly.indexOf("%loop.continue") >= 0);
    SLANG_CHECK(assembly.indexOf("@sumToLimit, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsScalarFunctionKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarFunctions());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarFunctionModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(assembly.indexOf("define i32 @incrementScalar(i32") >= 0);
    SLANG_CHECK(assembly.indexOf("define void @callScalar(i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("call i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(assembly.indexOf("@callScalar, !\"kernel\", i32 1") >= 0);
    SLANG_CHECK(assembly.indexOf("@incrementScalar, !\"kernel\"") < 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsPointerOffsetKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarPointerArithmetic());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildPointerOffsetModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf(
            "define void @copyIndexed(i32 addrspace(1)* %0, i32 addrspace(1)* %1, i32 %2)") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr i32")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("load i32, i32 addrspace(1)*") >= 0);
    SLANG_CHECK(assembly.indexOf("store i32") >= 0 && assembly.indexOf("i32 addrspace(1)*") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@copyIndexed, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsArrayElementKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarArrayAddressing());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildArrayElementModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf("define void @copyArrayElement([4 x i32] addrspace(1)* %0, [4 x i32] "
                         "addrspace(1)* %1, i32 %2)") >= 0);
    SLANG_CHECK(
        _countOccurrences(assembly.getUnownedSlice(), toSlice("getelementptr [4 x i32]")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("i32 0, i32 %2")) == 2);
    SLANG_CHECK(assembly.indexOf("getelementptr inbounds") < 0);
    SLANG_CHECK(assembly.indexOf("addrspacecast") < 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("load i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 2);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@copyArrayElement, !\"kernel\", i32 1") >= 0);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsIntegerMultiplyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarIntegerMultiply());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildIntegerMultiplyModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf("define void @multiplyScalar(i32 addrspace(1)* %0, i32 %1, i32 %2)") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("mul i32")) == 1);
    SLANG_CHECK(assembly.indexOf("mul i32 %1, %2") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@multiplyScalar, !\"kernel\", i32 1") >= 0);
    const Index multiplyIndex = assembly.indexOf("mul i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(multiplyIndex >= 0);
    SLANG_CHECK(storeIndex > multiplyIndex);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderBuildsIntegerBitAndKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarIntegerBitAnd());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics = "stale assembly diagnostics";
    String bitcodeDiagnostics = "stale bitcode diagnostics";
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildIntegerBitAndModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(assemblyBlob != nullptr);
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    const String assembly = _getBlobText(assemblyBlob);
    SLANG_CHECK(
        assembly.indexOf("define void @bitAndScalar(i32 addrspace(1)* %0, i32 %1, i32 %2)") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("and i32")) == 1);
    SLANG_CHECK(assembly.indexOf("and i32 %1, %2") >= 0);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("store i32")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("align 4")) == 1);
    SLANG_CHECK(_countOccurrences(assembly.getUnownedSlice(), toSlice("ret void")) == 1);
    SLANG_CHECK(assembly.indexOf("@bitAndScalar, !\"kernel\", i32 1") >= 0);
    const Index bitAndIndex = assembly.indexOf("and i32");
    const Index storeIndex = assembly.indexOf("store i32");
    SLANG_CHECK_ABORT(bitAndIndex >= 0);
    SLANG_CHECK(storeIndex > bitAndIndex);

    static const uint8_t kBitcodeMagic[] = {0x42, 0x43, 0xc0, 0xde};
    SLANG_CHECK(bitcodeBlob->getBufferSize() > SLANG_COUNT_OF(kBitcodeMagic));
    SLANG_CHECK(
        ::memcmp(bitcodeBlob->getBufferPointer(), kBitcodeMagic, SLANG_COUNT_OF(kBitcodeMagic)) ==
        0);
}

SLANG_UNIT_TEST(nvvmIRBuilderDifferentialScalarPTX)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics;
    String bitcodeDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    ComPtr<IArtifact> nvvmArtifact;
    const SlangResult nvvmResult = _compileRealNVVMBitcode(
        String(),
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        nvvmArtifact);
    if (nvvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar PTX differential test because libNVVM was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmArtifact != nullptr);

    ComPtr<IArtifact> nvrtcArtifact;
    const SlangResult nvrtcResult =
        _compileRealNVRTCSource(toSlice(kScalarReferenceCUDASource), nvrtcArtifact);
    if (nvrtcResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar PTX differential test because NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcArtifact != nullptr);

    String nvvmPTX;
    String nvrtcPTX;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(nvvmArtifact, nvvmPTX)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(nvrtcArtifact, nvrtcPTX)));
    SLANG_CHECK(nvvmPTX.indexOf(".address_size 64") >= 0);
    SLANG_CHECK(nvrtcPTX.indexOf(".address_size 64") >= 0);

    PTXEntrySummary nvvmWrite;
    PTXEntrySummary nvvmCopy;
    PTXEntrySummary nvrtcWrite;
    PTXEntrySummary nvrtcCopy;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice(kWriteScalarKernelName), nvvmWrite)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvvmPTX.getUnownedSlice(), toSlice(kCopyScalarKernelName), nvvmCopy)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        nvrtcPTX.getUnownedSlice(),
        toSlice(kWriteScalarKernelName),
        nvrtcWrite)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _summarizePTXEntry(nvrtcPTX.getUnownedSlice(), toSlice(kCopyScalarKernelName), nvrtcCopy)));

    static const uint32_t kWriteParameterWidths[] = {64, 32};
    static const uint32_t kCopyParameterWidths[] = {64, 64};
    SLANG_CHECK(_hasPTXParameterWidths(
        nvvmWrite,
        kWriteParameterWidths,
        SLANG_COUNT_OF(kWriteParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvrtcWrite,
        kWriteParameterWidths,
        SLANG_COUNT_OF(kWriteParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvvmCopy,
        kCopyParameterWidths,
        SLANG_COUNT_OF(kCopyParameterWidths)));
    SLANG_CHECK(_hasPTXParameterWidths(
        nvrtcCopy,
        kCopyParameterWidths,
        SLANG_COUNT_OF(kCopyParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmWrite, nvrtcWrite));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmCopy, nvrtcCopy));

    SLANG_CHECK(nvvmWrite.hasGlobalStore32);
    SLANG_CHECK(nvrtcWrite.hasGlobalStore32);
    SLANG_CHECK(nvvmCopy.hasGlobalLoad32);
    SLANG_CHECK(nvvmCopy.hasGlobalStore32);
    SLANG_CHECK(nvrtcCopy.hasGlobalLoad32);
    SLANG_CHECK(nvrtcCopy.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmIRBuilderCompilesScalarBitcodeThroughRegistry)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics;
    String bitcodeDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    auto session = static_cast<Slang::Session*>(globalSession.get());
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring registered NVVM handoff test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }
    IDownstreamCompiler* compiler = session->m_downstreamCompilers[int(PassThroughMode::NVVM)];
    SLANG_CHECK_ABORT(compiler != nullptr);
    SLANG_CHECK_ABORT(compiler->getDesc().type == SLANG_PASS_THROUGH_NVVM);

    ComPtr<IArtifact> sourceArtifact =
        _createNVVMBitcodeArtifact(bitcodeBlob->getBufferPointer(), bitcodeBlob->getBufferSize());
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    const SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
        _reportArtifactDiagnostics(outputArtifact);

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kCopyScalarKernelName)));
}

SLANG_UNIT_TEST(nvvmSlangEmptyComputeUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            StringBuilder state;
            state << "direct NVVM compile result " << int(compileResult) << "; builder modules "
                  << gFakeNVVMBuilder.createModuleCallCount << "; libNVVM programs "
                  << gFakeNVVM.createProgramCallCount << "; libNVVM modules "
                  << gFakeNVVM.addModuleCallCount;
            getTestReporter()->message(TestMessageType::Info, state.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
        SLANG_CHECK(gFakeNVVMBuilder.blockName == "entry");
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.setInsertBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeQueryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWriteCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);

        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addedModule.getLength() == sizeof(kFakeNVVMBuilderBitcode));
        SLANG_CHECK(
            ::memcmp(
                gFakeNVVM.addedModule.getBuffer(),
                kFakeNVVMBuilderBitcode,
                sizeof(kFakeNVVMBuilderBitcode)) == 0);
        SLANG_CHECK(_hasOption(gFakeNVVM.verifyOptions, "-arch=compute_70"));
        SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-arch=compute_70"));
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVM.successfulLoadCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangScalarMemoryAndConditionalUseDirectPipeline)
{
    struct ExpectedBuilderGraph
    {
        const char* source;
        const FakeNVVMBuilderParameterTypeKind* parameterTypeKinds;
        size_t parameterCount;
        int blockCount;
        int loadCount;
        int storeCount;
        int binaryCount;
        int comparisonCount;
        int branchCount;
        int conditionalBranchCount;
    };
    static const FakeNVVMBuilderParameterTypeKind kWriteParameterTypes[] = {
        FakeNVVMBuilderParameterTypeKind::Pointer,
        FakeNVVMBuilderParameterTypeKind::Integer,
    };
    static const FakeNVVMBuilderParameterTypeKind kCopyParameterTypes[] = {
        FakeNVVMBuilderParameterTypeKind::Pointer,
        FakeNVVMBuilderParameterTypeKind::Pointer,
    };
    static const FakeNVVMBuilderParameterTypeKind kChooseParameterTypes[] = {
        FakeNVVMBuilderParameterTypeKind::Pointer,
        FakeNVVMBuilderParameterTypeKind::Integer,
        FakeNVVMBuilderParameterTypeKind::Integer,
    };
    static const ExpectedBuilderGraph kCases[] = {
        {kDirectNVVMWriteScalarSource, kWriteParameterTypes, 2, 1, 0, 1, 0, 0, 0, 0},
        {kDirectNVVMCopyScalarSource, kCopyParameterTypes, 2, 1, 1, 1, 0, 0, 0, 0},
        {kDirectNVVMChooseScalarSource, kChooseParameterTypes, 3, 4, 0, 2, 2, 1, 2, 1},
    };

    for (const auto& expected : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult compileResult =
                _compileSlangWithDirectNVVM(globalSession, expected.source, code, diagnostics);
            if (SLANG_FAILED(compileResult))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
            SLANG_CHECK(gFakeNVVMBuilder.functionParameterCount == expected.parameterCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds.getCount() ==
                Index(expected.parameterCount));
            for (Index i = 0; i < gFakeNVVMBuilder.functionParameterTypeKinds.getCount(); ++i)
            {
                SLANG_CHECK(
                    gFakeNVVMBuilder.functionParameterTypeKinds[i] ==
                    expected.parameterTypeKinds[i]);
            }
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getVoidTypeCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
            SLANG_CHECK(
                gFakeNVVMBuilder.getFunctionParameterCallCount == int(expected.parameterCount));
            SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == expected.blockCount);
            SLANG_CHECK(gFakeNVVMBuilder.setInsertBlockCallCount == expected.blockCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == expected.loadCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == expected.storeCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == expected.binaryCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == expected.comparisonCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == expected.branchCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.emitConditionalBranchCallCount == expected.conditionalBranchCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
            SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);

            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterIndices.getCount() ==
                Index(expected.parameterCount));
            for (Index i = 0; i < gFakeNVVMBuilder.functionParameterIndices.getCount(); ++i)
                SLANG_CHECK(gFakeNVVMBuilder.functionParameterIndices[i] == size_t(i));
            SLANG_CHECK(
                gFakeNVVMBuilder.storePointerParameterIndices.getCount() == expected.storeCount);
            for (size_t pointerIndex : gFakeNVVMBuilder.storePointerParameterIndices)
                SLANG_CHECK(pointerIndex == 0);

            if (expected.binaryCount)
            {
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryOperations.getCount() == 2);
                bool foundAdd = false;
                bool foundSub = false;
                for (auto operation : gFakeNVVMBuilder.integerBinaryOperations)
                {
                    foundAdd = foundAdd || operation == SLANG_NVVM_INTEGER_BINARY_OP_ADD;
                    foundSub = foundSub || operation == SLANG_NVVM_INTEGER_BINARY_OP_SUB;
                }
                SLANG_CHECK(foundAdd);
                SLANG_CHECK(foundSub);
                SLANG_CHECK(gFakeNVVMBuilder.comparisonLeftParameterIndices.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.comparisonLeftParameterIndices[0] == 1);
                SLANG_CHECK(gFakeNVVMBuilder.comparisonRightParameterIndices[0] == 2);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryLeftParameterIndices.getCount() == 2);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryLeftParameterIndices[0] == 1);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryLeftParameterIndices[1] == 1);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryRightParameterIndices[0] == 2);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryRightParameterIndices[1] == 2);
                SLANG_CHECK(gFakeNVVMBuilder.conditionalTrueBlockIndex == 1);
                SLANG_CHECK(gFakeNVVMBuilder.conditionalFalseBlockIndex == 2);
                SLANG_CHECK(gFakeNVVMBuilder.branchTargetBlockIndices.getCount() == 2);
                SLANG_CHECK(gFakeNVVMBuilder.branchTargetBlockIndices[0] == 3);
                SLANG_CHECK(gFakeNVVMBuilder.branchTargetBlockIndices[1] == 3);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds.getCount() == 2);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueKinds[0] == FakeNVVMBuilderValueKind::IntegerBinary);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueKinds[1] == FakeNVVMBuilderValueKind::IntegerBinary);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueBinaryIndices.getCount() == 2);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueBinaryIndices[0] == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueBinaryIndices[1] == 1);
            }
            else if (expected.loadCount)
            {
                SLANG_CHECK(gFakeNVVMBuilder.loadPointerParameterIndices.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.loadPointerParameterIndices[0] == 1);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds[0] == FakeNVVMBuilderValueKind::Load);
            }
            else
            {
                SLANG_CHECK(gFakeNVVMBuilder.storeValueKinds.getCount() == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueKinds[0] == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueParameterIndices[0] == 1);
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangScalarSSAUsesDirectPipeline)
{
    enum class SSAShape
    {
        Constant,
        Merge,
        Loop,
    };
    struct ExpectedGraph
    {
        const char* source;
        SSAShape shape;
        int blockCount;
        int constantCount;
        int phiCount;
        int incomingCount;
        int binaryCount;
        int comparisonCount;
        int branchCount;
    };
    static const ExpectedGraph kCases[] = {
        {kDirectNVVMIntegerConstantSource, SSAShape::Constant, 1, 1, 0, 0, 1, 0, 0},
        {kDirectNVVMMergePhiSource, SSAShape::Merge, 4, 0, 1, 2, 0, 1, 2},
        {kDirectNVVMFiniteLoopSource, SSAShape::Loop, 6, 2, 2, 4, 2, 1, 4},
    };

    for (const auto& expected : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult result =
                _compileSlangWithDirectNVVM(globalSession, expected.source, code, diagnostics);
            if (SLANG_FAILED(result))
            {
                const String diagnosticText = _getBlobText(diagnostics);
                if (diagnosticText.getLength())
                    getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

            SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
            if (expected.shape == SSAShape::Constant || expected.shape == SSAShape::Loop)
            {
                SLANG_CHECK(gFakeNVVMBuilder.functionParameterCount == 2);
            }
            else
            {
                SLANG_CHECK(gFakeNVVMBuilder.functionParameterCount == 3);
            }
            SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == expected.blockCount);
            SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == expected.constantCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == expected.phiCount);
            SLANG_CHECK(gFakeNVVMBuilder.addIntegerPhiIncomingCallCount == expected.incomingCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == expected.binaryCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == expected.comparisonCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == expected.branchCount);
            SLANG_CHECK(
                gFakeNVVMBuilder.emitConditionalBranchCallCount == expected.comparisonCount);
            SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);

            if (expected.shape == SSAShape::Constant)
            {
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 1);
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.integerBinaryOperations[0] ==
                    SLANG_NVVM_INTEGER_BINARY_OP_ADD);
                SLANG_CHECK(
                    gFakeNVVMBuilder.integerBinaryLeftValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryLeftValueRefs[0].index == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.integerBinaryRightValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerConstant);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerBinary);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryBlockIndices[0] == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == 0);
            }
            if (expected.shape == SSAShape::Merge)
            {
                const Index mergeBlock = gFakeNVVMBuilder.integerPhiTargetBlockIndices[0];
                const Index entryBlock = gFakeNVVMBuilder.conditionalSourceBlockIndex;
                const Index trueBlock = gFakeNVVMBuilder.conditionalTrueBlockIndex;
                const Index falseBlock = gFakeNVVMBuilder.conditionalFalseBlockIndex;
                SLANG_CHECK(entryBlock >= 0);
                SLANG_CHECK(trueBlock >= 0);
                SLANG_CHECK(falseBlock >= 0);
                SLANG_CHECK(mergeBlock >= 0);
                SLANG_CHECK(entryBlock != trueBlock);
                SLANG_CHECK(entryBlock != falseBlock);
                SLANG_CHECK(entryBlock != mergeBlock);
                SLANG_CHECK(trueBlock != falseBlock);
                SLANG_CHECK(trueBlock != mergeBlock);
                SLANG_CHECK(falseBlock != mergeBlock);
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount() == 2);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerPhi);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
                SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == mergeBlock);

                Index xPredecessor = -1;
                Index yPredecessor = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
                {
                    SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == 0);
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
                    SLANG_CHECK(valueRef.kind == FakeNVVMBuilderValueKind::Parameter);
                    if (valueRef.index == 1)
                    {
                        xPredecessor =
                            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
                    }
                    else if (valueRef.index == 2)
                    {
                        yPredecessor =
                            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
                    }
                }
                SLANG_CHECK(xPredecessor == trueBlock);
                SLANG_CHECK(yPredecessor == falseBlock);
                SLANG_CHECK(xPredecessor != yPredecessor);
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(trueBlock, mergeBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(falseBlock, mergeBlock));
            }
            else if (expected.shape == SSAShape::Loop)
            {
                SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues.getCount() == 2);
                Index zeroIndex = -1;
                Index oneIndex = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerConstantValues.getCount(); ++i)
                {
                    if (gFakeNVVMBuilder.integerConstantValues[i] == 0)
                        zeroIndex = i;
                    else if (gFakeNVVMBuilder.integerConstantValues[i] == 1)
                        oneIndex = i;
                }
                SLANG_CHECK(zeroIndex >= 0);
                SLANG_CHECK(oneIndex >= 0);
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiTargetBlockIndices.getCount() == 2);
                const Index headerBlock = gFakeNVVMBuilder.integerPhiTargetBlockIndices[0];
                SLANG_CHECK(headerBlock != 0);
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiTargetBlockIndices[1] == headerBlock);
                SLANG_CHECK(gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount() == 4);
                SLANG_CHECK(
                    gFakeNVVMBuilder.storeValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerPhi);
                SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 1);
                SLANG_CHECK(
                    gFakeNVVMBuilder.comparisonLeftValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::IntegerPhi);
                SLANG_CHECK(gFakeNVVMBuilder.comparisonLeftValueRefs[0].index == 0);
                SLANG_CHECK(
                    gFakeNVVMBuilder.comparisonRightValueRefs[0].kind ==
                    FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(gFakeNVVMBuilder.comparisonRightValueRefs[0].index == 1);

                Index nextSumIndex = -1;
                Index nextIIndex = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerBinaryOperations.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef left =
                        gFakeNVVMBuilder.integerBinaryLeftValueRefs[i];
                    const FakeNVVMBuilderValueRef right =
                        gFakeNVVMBuilder.integerBinaryRightValueRefs[i];
                    SLANG_CHECK(
                        gFakeNVVMBuilder.integerBinaryOperations[i] ==
                        SLANG_NVVM_INTEGER_BINARY_OP_ADD);
                    const bool leftIsI =
                        left.kind == FakeNVVMBuilderValueKind::IntegerPhi && left.index == 0;
                    const bool rightIsI =
                        right.kind == FakeNVVMBuilderValueKind::IntegerPhi && right.index == 0;
                    const bool leftIsSum =
                        left.kind == FakeNVVMBuilderValueKind::IntegerPhi && left.index == 1;
                    const bool rightIsSum =
                        right.kind == FakeNVVMBuilderValueKind::IntegerPhi && right.index == 1;
                    const bool leftIsOne = left.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                                           left.index == oneIndex;
                    const bool rightIsOne =
                        right.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                        right.index == oneIndex;
                    if ((leftIsSum && rightIsI) || (leftIsI && rightIsSum))
                    {
                        nextSumIndex = i;
                    }
                    if ((leftIsI && rightIsOne) || (leftIsOne && rightIsI))
                    {
                        nextIIndex = i;
                    }
                }
                SLANG_CHECK_ABORT(nextSumIndex >= 0);
                SLANG_CHECK_ABORT(nextIIndex >= 0);

                Index entryBlock = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
                    if (gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == 0 &&
                        valueRef.kind == FakeNVVMBuilderValueKind::IntegerConstant &&
                        valueRef.index == zeroIndex)
                    {
                        entryBlock = gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
                        break;
                    }
                }
                SLANG_CHECK(entryBlock >= 0);
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    0,
                    FakeNVVMBuilderValueKind::IntegerConstant,
                    zeroIndex,
                    entryBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    1,
                    FakeNVVMBuilderValueKind::IntegerConstant,
                    zeroIndex,
                    entryBlock));

                Index continueBlock = -1;
                for (Index i = 0; i < gFakeNVVMBuilder.integerPhiIncomingPhiIndices.getCount(); ++i)
                {
                    const FakeNVVMBuilderValueRef valueRef =
                        gFakeNVVMBuilder.integerPhiIncomingValueRefs[i];
                    if (gFakeNVVMBuilder.integerPhiIncomingPhiIndices[i] == 0 &&
                        valueRef.kind == FakeNVVMBuilderValueKind::IntegerBinary &&
                        valueRef.index == nextIIndex)
                    {
                        continueBlock =
                            gFakeNVVMBuilder.integerPhiIncomingPredecessorBlockIndices[i];
                        break;
                    }
                }
                SLANG_CHECK(continueBlock >= 0);
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    0,
                    FakeNVVMBuilderValueKind::IntegerBinary,
                    nextIIndex,
                    continueBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderPhiIncoming(
                    1,
                    FakeNVVMBuilderValueKind::IntegerBinary,
                    nextSumIndex,
                    continueBlock));

                const Index bodyBlock = gFakeNVVMBuilder.conditionalTrueBlockIndex;
                const Index exitBlock = gFakeNVVMBuilder.conditionalFalseBlockIndex;
                const Index breakBlock = gFakeNVVMBuilder.storeBlockIndices[0];
                SLANG_CHECK(gFakeNVVMBuilder.conditionalSourceBlockIndex == headerBlock);
                SLANG_CHECK(bodyBlock >= 0);
                SLANG_CHECK(exitBlock >= 0);
                SLANG_CHECK(breakBlock >= 0);
                SLANG_CHECK(entryBlock != headerBlock);
                SLANG_CHECK(entryBlock != bodyBlock);
                SLANG_CHECK(entryBlock != continueBlock);
                SLANG_CHECK(entryBlock != exitBlock);
                SLANG_CHECK(entryBlock != breakBlock);
                SLANG_CHECK(headerBlock != bodyBlock);
                SLANG_CHECK(headerBlock != continueBlock);
                SLANG_CHECK(headerBlock != exitBlock);
                SLANG_CHECK(headerBlock != breakBlock);
                SLANG_CHECK(bodyBlock != continueBlock);
                SLANG_CHECK(bodyBlock != exitBlock);
                SLANG_CHECK(bodyBlock != breakBlock);
                SLANG_CHECK(continueBlock != exitBlock);
                SLANG_CHECK(continueBlock != breakBlock);
                SLANG_CHECK(exitBlock != breakBlock);
                SLANG_CHECK(gFakeNVVMBuilder.integerBinaryBlockIndices[nextSumIndex] == bodyBlock);
                SLANG_CHECK(
                    gFakeNVVMBuilder.integerBinaryBlockIndices[nextIIndex] == continueBlock);
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(entryBlock, headerBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(bodyBlock, continueBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(continueBlock, headerBlock));
                SLANG_CHECK(_hasFakeNVVMBuilderBranch(exitBlock, breakBlock));
            }
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangScalarFunctionsUseDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarFunctionSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerConstantValues[0] == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);

        const Index kernelFunction = gFakeNVVMBuilder.kernelFunctionIndices[0];
        SLANG_CHECK_ABORT(kernelFunction >= 0);
        SLANG_CHECK_ABORT(kernelFunction < gFakeNVVMBuilder.functionTypeIndices.getCount());
        const Index kernelType = gFakeNVVMBuilder.functionTypeIndices[kernelFunction];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[kernelType] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[kernelType] == 2);
        const Index kernelTypeOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[kernelType];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[kernelTypeOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[kernelTypeOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        Index functionBlocks[3] = {-1, -1, -1};
        for (Index blockIndex = 0; blockIndex < gFakeNVVMBuilder.blockFunctionIndices.getCount();
             ++blockIndex)
        {
            const Index functionIndex = gFakeNVVMBuilder.blockFunctionIndices[blockIndex];
            SLANG_CHECK(functionIndex >= 0 && functionIndex < 3);
            SLANG_CHECK(functionBlocks[functionIndex] == -1);
            functionBlocks[functionIndex] = blockIndex;
        }
        SLANG_CHECK_ABORT(functionBlocks[kernelFunction] >= 0);

        Index incrementFunction = -1;
        Index incrementBinary = -1;
        Index kernelBinary = -1;
        for (Index binaryIndex = 0;
             binaryIndex < gFakeNVVMBuilder.integerBinaryOperations.getCount();
             ++binaryIndex)
        {
            SLANG_CHECK(
                gFakeNVVMBuilder.integerBinaryOperations[binaryIndex] ==
                SLANG_NVVM_INTEGER_BINARY_OP_ADD);
            const Index blockIndex = gFakeNVVMBuilder.integerBinaryBlockIndices[binaryIndex];
            const Index functionIndex = gFakeNVVMBuilder.blockFunctionIndices[blockIndex];
            const FakeNVVMBuilderValueRef left =
                gFakeNVVMBuilder.integerBinaryLeftValueRefs[binaryIndex];
            const FakeNVVMBuilderValueRef right =
                gFakeNVVMBuilder.integerBinaryRightValueRefs[binaryIndex];
            const bool isIncrement =
                ((left.kind == FakeNVVMBuilderValueKind::Parameter &&
                  left.functionIndex == functionIndex && left.index == 0 &&
                  right.kind == FakeNVVMBuilderValueKind::IntegerConstant && right.index == 0) ||
                 (right.kind == FakeNVVMBuilderValueKind::Parameter &&
                  right.functionIndex == functionIndex && right.index == 0 &&
                  left.kind == FakeNVVMBuilderValueKind::IntegerConstant && left.index == 0));
            if (isIncrement)
            {
                incrementFunction = functionIndex;
                incrementBinary = binaryIndex;
            }
            else
            {
                SLANG_CHECK(functionIndex == kernelFunction);
                kernelBinary = binaryIndex;
            }
        }
        SLANG_CHECK_ABORT(incrementFunction >= 0);
        SLANG_CHECK_ABORT(incrementFunction != kernelFunction);
        SLANG_CHECK_ABORT(incrementBinary >= 0);
        SLANG_CHECK_ABORT(kernelBinary >= 0);

        Index incrementTwiceFunction = -1;
        for (Index functionIndex = 0; functionIndex < 3; ++functionIndex)
        {
            if (functionIndex != kernelFunction && functionIndex != incrementFunction)
                incrementTwiceFunction = functionIndex;
        }
        SLANG_CHECK_ABORT(incrementTwiceFunction >= 0);
        const Index helperFunctions[] = {incrementFunction, incrementTwiceFunction};
        for (Index helperFunction : helperFunctions)
        {
            const Index helperType = gFakeNVVMBuilder.functionTypeIndices[helperFunction];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionTypeResultKinds[helperType] ==
                FakeNVVMBuilderResultTypeKind::Integer);
            SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[helperType] == 1);
            const Index helperTypeOffset =
                gFakeNVVMBuilder.functionTypeParameterKindOffsets[helperType];
            SLANG_CHECK(
                gFakeNVVMBuilder.functionParameterTypeKinds[helperTypeOffset] ==
                FakeNVVMBuilderParameterTypeKind::Integer);
        }

        Index incrementTwiceFirstCall = -1;
        Index incrementTwiceSecondCall = -1;
        Index kernelIncrementCall = -1;
        Index kernelIncrementTwiceCall = -1;
        for (Index callIndex = 0; callIndex < gFakeNVVMBuilder.callCalleeFunctionIndices.getCount();
             ++callIndex)
        {
            const Index callerBlock = gFakeNVVMBuilder.callCallerBlockIndices[callIndex];
            const Index callerFunction = gFakeNVVMBuilder.blockFunctionIndices[callerBlock];
            const Index calleeFunction = gFakeNVVMBuilder.callCalleeFunctionIndices[callIndex];
            SLANG_CHECK(gFakeNVVMBuilder.callArgumentCounts[callIndex] == 1);
            const FakeNVVMBuilderValueRef argument =
                gFakeNVVMBuilder
                    .callArgumentValueRefs[gFakeNVVMBuilder.callArgumentOffsets[callIndex]];
            if (callerFunction == incrementTwiceFunction)
            {
                SLANG_CHECK(calleeFunction == incrementFunction);
                if (argument.kind == FakeNVVMBuilderValueKind::Parameter)
                {
                    SLANG_CHECK(argument.functionIndex == incrementTwiceFunction);
                    SLANG_CHECK(argument.index == 0);
                    incrementTwiceFirstCall = callIndex;
                }
                else
                {
                    SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Call);
                    incrementTwiceSecondCall = callIndex;
                }
            }
            else
            {
                SLANG_CHECK(callerFunction == kernelFunction);
                SLANG_CHECK(argument.kind == FakeNVVMBuilderValueKind::Parameter);
                SLANG_CHECK(argument.functionIndex == kernelFunction);
                SLANG_CHECK(argument.index == 1);
                if (calleeFunction == incrementFunction)
                    kernelIncrementCall = callIndex;
                else if (calleeFunction == incrementTwiceFunction)
                    kernelIncrementTwiceCall = callIndex;
                else
                    SLANG_CHECK(false);
            }
        }
        SLANG_CHECK_ABORT(incrementTwiceFirstCall >= 0);
        SLANG_CHECK_ABORT(incrementTwiceSecondCall >= 0);
        SLANG_CHECK_ABORT(kernelIncrementCall >= 0);
        SLANG_CHECK_ABORT(kernelIncrementTwiceCall >= 0);
        const FakeNVVMBuilderValueRef secondCallArgument =
            gFakeNVVMBuilder.callArgumentValueRefs
                [gFakeNVVMBuilder.callArgumentOffsets[incrementTwiceSecondCall]];
        SLANG_CHECK(secondCallArgument.kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(secondCallArgument.index == incrementTwiceFirstCall);

        SLANG_CHECK(gFakeNVVMBuilder.integerReturnValueRefs.getCount() == 2);
        bool sawIncrementReturn = false;
        bool sawIncrementTwiceReturn = false;
        for (Index returnIndex = 0;
             returnIndex < gFakeNVVMBuilder.integerReturnValueRefs.getCount();
             ++returnIndex)
        {
            const Index returnBlock = gFakeNVVMBuilder.integerReturnBlockIndices[returnIndex];
            const Index returnFunction = gFakeNVVMBuilder.blockFunctionIndices[returnBlock];
            const FakeNVVMBuilderValueRef returnValue =
                gFakeNVVMBuilder.integerReturnValueRefs[returnIndex];
            if (returnFunction == incrementFunction)
            {
                SLANG_CHECK(returnValue.kind == FakeNVVMBuilderValueKind::IntegerBinary);
                SLANG_CHECK(returnValue.index == incrementBinary);
                sawIncrementReturn = true;
            }
            else if (returnFunction == incrementTwiceFunction)
            {
                SLANG_CHECK(returnValue.kind == FakeNVVMBuilderValueKind::Call);
                SLANG_CHECK(returnValue.index == incrementTwiceSecondCall);
                sawIncrementTwiceReturn = true;
            }
            else
            {
                SLANG_CHECK(false);
            }
        }
        SLANG_CHECK(sawIncrementReturn);
        SLANG_CHECK(sawIncrementTwiceReturn);

        const FakeNVVMBuilderValueRef kernelLeft =
            gFakeNVVMBuilder.integerBinaryLeftValueRefs[kernelBinary];
        const FakeNVVMBuilderValueRef kernelRight =
            gFakeNVVMBuilder.integerBinaryRightValueRefs[kernelBinary];
        SLANG_CHECK(kernelLeft.kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(kernelRight.kind == FakeNVVMBuilderValueKind::Call);
        SLANG_CHECK(
            (kernelLeft.index == kernelIncrementCall &&
             kernelRight.index == kernelIncrementTwiceCall) ||
            (kernelLeft.index == kernelIncrementTwiceCall &&
             kernelRight.index == kernelIncrementCall));
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerBinary);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == kernelBinary);
        SLANG_CHECK(gFakeNVVMBuilder.storeBlockIndices[0] == functionBlocks[kernelFunction]);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerFunctionIndices[0] == kernelFunction);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerParameterIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerParameterIndices[0] == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Linking must prune an unreachable helper with an otherwise unsupported body. The direct
    // emitter receives only the selected kernel and its one reachable helper.
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMPrunesUnreachableHelperSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangPointerOffsetUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 2] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetElementValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices.getCount() == 2);
        Index destinationOffsetIndex = -1;
        Index sourceOffsetIndex = -1;
        for (Index offsetIndex = 0;
             offsetIndex < gFakeNVVMBuilder.pointerOffsetBaseValueRefs.getCount();
             ++offsetIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.pointerOffsetBaseValueRefs[offsetIndex];
            const FakeNVVMBuilderValueRef element =
                gFakeNVVMBuilder.pointerOffsetElementValueRefs[offsetIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(element.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(element.functionIndex == 0);
            SLANG_CHECK(element.index == 2);
            SLANG_CHECK(gFakeNVVMBuilder.pointerOffsetCallerBlockIndices[offsetIndex] == 0);
            if (base.index == 0)
                destinationOffsetIndex = offsetIndex;
            else if (base.index == 1)
                sourceOffsetIndex = offsetIndex;
            else
                SLANG_CHECK(false);
        }
        SLANG_CHECK_ABORT(destinationOffsetIndex >= 0);
        SLANG_CHECK_ABORT(sourceOffsetIndex >= 0);
        SLANG_CHECK(destinationOffsetIndex != sourceOffsetIndex);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == sourceOffsetIndex);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::PointerOffset);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == destinationOffsetIndex);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);

        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.kernelFunctionIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangFixedDeviceArrayUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementType == _getFakeNVVMBuilderIntegerType());
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementCount == 4);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerPointeeTypes[0] == _getFakeNVVMBuilderArrayType());
        SLANG_CHECK(gFakeNVVMBuilder.pointerAddressSpaces.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.pointerAddressSpaces[0] == SLANG_NVVM_ADDRESS_SPACE_GLOBAL);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::ArrayPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::ArrayPointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 2] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerBaseValueRefs.getCount() == 2);
        SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerIndexValueRefs.getCount() == 2);
        for (Index elementIndex = 0; elementIndex < 2; ++elementIndex)
        {
            const FakeNVVMBuilderValueRef base =
                gFakeNVVMBuilder.arrayElementPointerBaseValueRefs[elementIndex];
            const FakeNVVMBuilderValueRef index =
                gFakeNVVMBuilder.arrayElementPointerIndexValueRefs[elementIndex];
            SLANG_CHECK(base.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(base.functionIndex == 0);
            SLANG_CHECK(base.index == elementIndex);
            SLANG_CHECK(index.kind == FakeNVVMBuilderValueKind::Parameter);
            SLANG_CHECK(index.functionIndex == 0);
            SLANG_CHECK(index.index == 2);
            SLANG_CHECK(gFakeNVVMBuilder.arrayElementPointerCallerBlockIndices[elementIndex] == 0);
        }

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.loadPointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.loadPointerValueRefs[0].index == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind ==
            FakeNVVMBuilderValueKind::ArrayElementPointer);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::Load);
        SLANG_CHECK(gFakeNVVMBuilder.loadAlignment == 4);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangIntegerMultiplyUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerMultiplySource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 2] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerMultiplyCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyCallerBlockIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyCallerBlockIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyLeftValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerMultiplyRightValueRefs.getCount() == 1);
        const FakeNVVMBuilderValueRef left = gFakeNVVMBuilder.integerMultiplyLeftValueRefs[0];
        const FakeNVVMBuilderValueRef right = gFakeNVVMBuilder.integerMultiplyRightValueRefs[0];
        SLANG_CHECK(left.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(left.functionIndex == 0);
        SLANG_CHECK(left.index == 1);
        SLANG_CHECK(right.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(right.functionIndex == 0);
        SLANG_CHECK(right.index == 2);

        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerMultiply);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangIntegerBitAndUsesDirectPipeline)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult result = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerBitAndSource,
            code,
            diagnostics);
        if (SLANG_FAILED(result))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(result));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(_getBlobText(code) == kFakeDirectPTX);

        SLANG_CHECK(gFakeNVVMBuilder.getIntegerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getPointerTypeCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createBlockCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.getFunctionParameterCallCount == 3);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeIndices.getCount() == 1);
        const Index functionTypeIndex = gFakeNVVMBuilder.functionTypeIndices[0];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionTypeResultKinds[functionTypeIndex] ==
            FakeNVVMBuilderResultTypeKind::Void);
        SLANG_CHECK(gFakeNVVMBuilder.functionTypeParameterCounts[functionTypeIndex] == 3);
        const Index parameterKindOffset =
            gFakeNVVMBuilder.functionTypeParameterKindOffsets[functionTypeIndex];
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset] ==
            FakeNVVMBuilderParameterTypeKind::Pointer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 1] ==
            FakeNVVMBuilderParameterTypeKind::Integer);
        SLANG_CHECK(
            gFakeNVVMBuilder.functionParameterTypeKinds[parameterKindOffset + 2] ==
            FakeNVVMBuilderParameterTypeKind::Integer);

        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBitAndCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndCallerBlockIndices.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndCallerBlockIndices[0] == 0);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndLeftValueRefs.getCount() == 1);
        SLANG_CHECK(gFakeNVVMBuilder.integerBitAndRightValueRefs.getCount() == 1);
        const FakeNVVMBuilderValueRef left = gFakeNVVMBuilder.integerBitAndLeftValueRefs[0];
        const FakeNVVMBuilderValueRef right = gFakeNVVMBuilder.integerBitAndRightValueRefs[0];
        SLANG_CHECK(left.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(left.functionIndex == 0);
        SLANG_CHECK(left.index == 1);
        SLANG_CHECK(right.kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(right.functionIndex == 0);
        SLANG_CHECK(right.index == 2);

        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storePointerValueRefs[0].kind == FakeNVVMBuilderValueKind::Parameter);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].functionIndex == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storePointerValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs.getCount() == 1);
        SLANG_CHECK(
            gFakeNVVMBuilder.storeValueRefs[0].kind == FakeNVVMBuilderValueKind::IntegerBitAnd);
        SLANG_CHECK(gFakeNVVMBuilder.storeValueRefs[0].index == 0);
        SLANG_CHECK(gFakeNVVMBuilder.storeAlignment == 4);

        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBinaryCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerMultiplyCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerSignedLessThanCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitConditionalBranchCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getIntegerConstantCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerPhiCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerReturnCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.getArrayTypeCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitReturnVoidCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.markFunctionAsKernelCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsQueryCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.serializeWithDiagnosticsWriteCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarControlFlowCapability)
{
    // A provider frozen at the exact Slice 4 scalar-memory prefix can still compile scalar loads
    // and stores through the public route.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMCopyScalarSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitLoadCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitStoreCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The same provider cannot satisfy the append-only control-flow prefix. Capability rejection
    // happens after discovery but before the emitter creates or mutates a module.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMChooseScalarSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarSSACapability)
{
    // The exact Slice 7 provider retains its published branch capability.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMChooseScalarSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Each Slice 8 shape is legal Slang IR but needs the new complete builder prefix. Discovery
    // succeeds and E52016 is emitted before module creation or libNVVM use.
    static const char* kSources[] = {
        kDirectNVVMIntegerConstantSource,
        kDirectNVVMMergePhiSource,
        kDirectNVVMFiniteLoopSource,
    };
    for (const char* source : kSources)
    {
        _resetDirectNVVMFakes();
        gFakeNVVMBuilder.apiV2.structureSize =
            uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE);
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, source, code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarFunctionCapability)
{
    // A provider frozen at the exact Slice 8 prefix can still compile the complete loop shape
    // published by that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFiniteLoopSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // A reachable scalar helper needs the append-only Slice 9 prefix. Reject it after discovery
    // but before module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize = uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarFunctionSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarPointerArithmeticCapability)
{
    // A provider frozen at the exact Slice 9 prefix retains the complete scalar-function graph
    // published by that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMScalarFunctionSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerCallCallCount == 4);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The same provider cannot satisfy the append-only pointer-arithmetic prefix. Discovery
    // succeeds, then E52016 is reported before module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarArrayAddressingCapability)
{
    // The exact Slice 10 provider retains the pointer-offset program published by that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitPointerOffsetCallCount == 2);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // The array program needs the coherent Slice 11 suffix. Gate after discovery but before any
    // provider module or libNVVM program is created.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarIntegerMultiplyCapability)
{
    // The exact Slice 11 provider retains fixed-array addressing from that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitArrayElementPointerCallCount == 2);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerMultiplyCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Multiplication needs the appended Slice 12 operation. Gate after provider discovery but
    // before builder-module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerMultiplySource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangNegotiatesScalarIntegerBitAndCapability)
{
    // The exact Slice 12 provider retains multiplication from that slice.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerMultiplySource,
            code,
            diagnostics)));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerMultiplyCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBitAndCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    // Bitwise AND needs the appended Slice 13 operation. Gate after provider discovery but before
    // builder-module creation or libNVVM use.
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.apiV2.structureSize =
        uint32_t(SLANG_NVVM_BUILDER_API_V2_SCALAR_INTEGER_MULTIPLY_MIN_SIZE);
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMIntegerBitAndSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVMBuilder.emitIntegerBitAndCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRetainsOnlySelectedCUDAKernel)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMSelectedKernelSource,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String diagnosticText = _getBlobText(diagnostics);
            if (diagnosticText.getLength())
                getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.functionName == "computeMain");
        SLANG_CHECK(gFakeNVVMBuilder.declareFunctionCallCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRejectsConventionalRawKernelParameters)
{
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMConventionalParameterizedComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52017") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangBuilderIdentityAffectsHashAndIsSessionCached)
{
    ComPtr<slang::IBlob> hashWithBuilder;
    _resetDirectNVVMFakes();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::ISession> session;
        ComPtr<slang::IComponentType> program;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createDirectNVVMLinkedProgram(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            session,
            program,
            diagnostics)));
        program->getEntryPointHash(0, 0, hashWithBuilder.writeRef());
        SLANG_CHECK_ABORT(hashWithBuilder != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 1);

        ComPtr<slang::IBlob> code;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);

    ComPtr<slang::IBlob> hashWithoutBuilder;
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.libraryUnavailable = true;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::ISession> session;
        ComPtr<slang::IComponentType> program;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createDirectNVVMLinkedProgram(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            session,
            program,
            diagnostics)));
        program->getEntryPointHash(0, 0, hashWithoutBuilder.writeRef());
        SLANG_CHECK_ABORT(hashWithoutBuilder != nullptr);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
        SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 0);

        ComPtr<slang::IBlob> code;
        SLANG_CHECK(SLANG_FAILED(
            program->getEntryPointCode(0, 0, code.writeRef(), diagnostics.writeRef())));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 1);
    }

    SLANG_CHECK_ABORT(hashWithBuilder->getBufferSize() == hashWithoutBuilder->getBufferSize());
    SLANG_CHECK(
        ::memcmp(
            hashWithBuilder->getBufferPointer(),
            hashWithoutBuilder->getBufferPointer(),
            hashWithBuilder->getBufferSize()) != 0);
}

SLANG_UNIT_TEST(nvvmSlangBuilderDiagnosticsStopBeforeLibNVVM)
{
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.verificationStatus = SLANG_NVVM_VERIFICATION_INVALID;
    gFakeNVVMBuilder.verificationDiagnostic = "fake direct NVVM verifier failure";
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        const String diagnosticText = _getBlobText(diagnostics);
        SLANG_CHECK(diagnosticText.indexOf("E52018") >= 0);
        SLANG_CHECK(diagnosticText.indexOf(gFakeNVVMBuilder.verificationDiagnostic) >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.destroyModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangUnsupportedIRStopsBeforeEmission)
{
    struct UnsupportedCase
    {
        const char* source;
        const char* expectedConstruct;
    };
    static const UnsupportedCase kCases[] = {
        {kDirectNVVMUnsupportedCallSource, "'helper function result type'"},
        {kDirectNVVMUnsupportedPointerHelperParameterSource, "'helper function parameter'"},
        {kDirectNVVMUnsupportedPointerHelperResultSource, "'helper function result type'"},
        {kDirectNVVMUnsignedPointerOffsetSource, "'signed i32 value'"},
        {kDirectNVVMUnsignedFixedArrayIndexSource, "'signed i32 value'"},
        {kDirectNVVMUnsupportedFloatArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedNestedArraySource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedLocalArraySource, "'var'"},
        {kDirectNVVMUnsupportedStructPointerSource, "'entry-point parameter'"},
        {kDirectNVVMUnsupportedArrayPointerHelperSource, "'helper function parameter'"},
        {kDirectNVVMUnsignedMultiplySource, "'entry-point parameter'"},
        {kDirectNVVMWideIntegerMultiplySource, "'entry-point parameter'"},
        {kDirectNVVMFloatingMultiplySource, "'entry-point parameter'"},
        {kDirectNVVMIntegerBitOrSource, "'or'"},
        {kDirectNVVMIntegerBitXorSource, "'xor'"},
        {kDirectNVVMUnsignedIntegerBitAndSource, "'entry-point parameter'"},
        {kDirectNVVMWideIntegerBitAndSource, "'entry-point parameter'"},
    };

    // The direct subset retains signed-i32 helper/value policy. Adjacent aggregate, local-memory,
    // unsigned/wide/floating multiplication, signed OR/XOR, unsigned/wide AND, unsigned indices,
    // and helper-array-pointer shapes remain deterministic boundaries before builder discovery.
    for (const auto& unsupported : kCases)
    {
        _resetDirectNVVMFakes();
        {
            ComPtr<slang::IGlobalSession> globalSession;
            SLANG_CHECK_ABORT(
                slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
            globalSession->setSharedLibraryLoader(loader);

            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK(SLANG_FAILED(
                _compileSlangWithDirectNVVM(globalSession, unsupported.source, code, diagnostics)));
            SLANG_CHECK(code == nullptr);
            const String diagnosticText = _getBlobText(diagnostics);
            SLANG_CHECK(diagnosticText.indexOf("E52017") >= 0);
            SLANG_CHECK(diagnosticText.indexOf(unsupported.expectedConstruct) >= 0);
            SLANG_CHECK(gFakeNVVMBuilder.loadRequestCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.successfulLoadCount == 0);
            SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
            SLANG_CHECK(gFakeNVVM.successfulLoadCount == 0);
            SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
        }
        SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmSlangMissingBuilderDoesNotFallback)
{
    _resetDirectNVVMFakes();
    gFakeNVVMBuilder.libraryUnavailable = true;
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeDirectNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK(SLANG_FAILED(_compileSlangWithDirectNVVM(
            globalSession,
            kDirectNVVMEmptyComputeSource,
            code,
            diagnostics)));
        SLANG_CHECK(code == nullptr);
        SLANG_CHECK(_getBlobText(diagnostics).indexOf("E52016") >= 0);
        SLANG_CHECK(gFakeNVVMBuilder.createModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmSlangRealEmptyCompute)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct Slang NVVM test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    const SlangResult compileResult = _compileSlangWithDirectNVVM(
        globalSession,
        kDirectNVVMEmptyComputeSource,
        code,
        diagnostics);
    if (SLANG_FAILED(compileResult))
    {
        const String diagnosticText = _getBlobText(diagnostics);
        if (diagnosticText.getLength())
            getTestReporter()->message(TestMessageType::Info, diagnosticText.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(code != nullptr);
    SLANG_CHECK(_getBlobText(code).indexOf(".visible .entry computeMain") >= 0);
}

SLANG_UNIT_TEST(nvvmSlangRealEmptyComputePtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct Slang NVVM ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring direct Slang NVVM ptxas test because libNVVM was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> code;
    ComPtr<slang::IBlob> diagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithDirectNVVM(
        globalSession,
        kDirectNVVMEmptyComputeSource,
        code,
        diagnostics)));
    ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
    SLANG_CHECK_ABORT(ptxArtifact != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
}

SLANG_UNIT_TEST(nvvmSlangRealScalarDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarFunctions());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring Slang scalar PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    struct ScalarCase
    {
        const char* source;
        const uint32_t* parameterWidths;
        Index parameterCount;
        bool expectsLoad;
        bool expectsAdd;
        bool expectsSignedComparison;
    };
    static const uint32_t kWriteWidths[] = {64, 32};
    static const uint32_t kCopyWidths[] = {64, 64};
    static const uint32_t kChooseWidths[] = {64, 32, 32};
    static const ScalarCase kCases[] = {
        {kDirectNVVMWriteScalarSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         false,
         false},
        {kDirectNVVMCopyScalarSource, kCopyWidths, SLANG_COUNT_OF(kCopyWidths), true, false, false},
        {kDirectNVVMChooseScalarSource,
         kChooseWidths,
         SLANG_COUNT_OF(kChooseWidths),
         false,
         true,
         true},
        {kDirectNVVMIntegerConstantSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         true,
         false},
        {kDirectNVVMMergePhiSource,
         kChooseWidths,
         SLANG_COUNT_OF(kChooseWidths),
         false,
         false,
         false},
        {kDirectNVVMFiniteLoopSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         false,
         false},
        {kDirectNVVMScalarFunctionSource,
         kWriteWidths,
         SLANG_COUNT_OF(kWriteWidths),
         false,
         false,
         false},
    };

    for (const auto& scalarCase : kCases)
    {
        ComPtr<slang::IBlob> nvvmCode;
        ComPtr<slang::IBlob> nvvmDiagnostics;
        const SlangResult nvvmResult = _compileSlangWithPTXMethod(
            globalSession,
            scalarCase.source,
            SLANG_EMIT_CUDA_VIA_NVVM,
            nvvmCode,
            nvvmDiagnostics);
        if (SLANG_FAILED(nvvmResult))
        {
            const String text = _getBlobText(nvvmDiagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
        SLANG_CHECK_ABORT(nvvmCode != nullptr);

        ComPtr<slang::IBlob> nvrtcCode;
        ComPtr<slang::IBlob> nvrtcDiagnostics;
        const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
            globalSession,
            scalarCase.source,
            SLANG_EMIT_CUDA_VIA_NVRTC,
            nvrtcCode,
            nvrtcDiagnostics);
        if (SLANG_FAILED(nvrtcResult))
        {
            const String text = _getBlobText(nvrtcDiagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
        SLANG_CHECK_ABORT(nvrtcCode != nullptr);

        PTXEntrySummary nvvmSummary;
        PTXEntrySummary nvrtcSummary;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(nvvmCode).getUnownedSlice(),
            toSlice("computeMain"),
            nvvmSummary)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
            _getBlobText(nvrtcCode).getUnownedSlice(),
            toSlice("computeMain"),
            nvrtcSummary)));
        SLANG_CHECK(_hasPTXParameterWidths(
            nvvmSummary,
            scalarCase.parameterWidths,
            scalarCase.parameterCount));
        SLANG_CHECK(_hasPTXParameterWidths(
            nvrtcSummary,
            scalarCase.parameterWidths,
            scalarCase.parameterCount));
        SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
        SLANG_CHECK(nvvmSummary.hasGlobalStore32);
        SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
        SLANG_CHECK(nvvmSummary.hasGlobalLoad32 == scalarCase.expectsLoad);
        SLANG_CHECK(nvrtcSummary.hasGlobalLoad32 == scalarCase.expectsLoad);
        if (scalarCase.expectsAdd)
        {
            SLANG_CHECK(nvvmSummary.hasAdd32);
            SLANG_CHECK(nvrtcSummary.hasAdd32);
        }
        if (scalarCase.expectsSignedComparison)
        {
            SLANG_CHECK(nvvmSummary.hasSignedComparison32);
            SLANG_CHECK(nvrtcSummary.hasSignedComparison32);
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangRealPointerOffsetDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarPointerArithmetic());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMPointerOffsetSource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMPointerOffsetSource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64, 64, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasGlobalLoad32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalLoad32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealFixedDeviceArrayDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarArrayAddressing());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMFixedDeviceArraySource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMFixedDeviceArraySource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64, 64, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasGlobalLoad32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalLoad32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerMultiplyDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerMultiply());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerMultiplySource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerMultiplySource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasMultiply32);
    SLANG_CHECK(nvrtcSummary.hasMultiply32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitAndDifferentialPTX)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitAnd());

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND PTX differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IBlob> nvvmCode;
    ComPtr<slang::IBlob> nvvmDiagnostics;
    const SlangResult nvvmResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerBitAndSource,
        SLANG_EMIT_CUDA_VIA_NVVM,
        nvvmCode,
        nvvmDiagnostics);
    if (SLANG_FAILED(nvvmResult))
    {
        const String text = _getBlobText(nvvmDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmCode != nullptr);

    ComPtr<slang::IBlob> nvrtcCode;
    ComPtr<slang::IBlob> nvrtcDiagnostics;
    const SlangResult nvrtcResult = _compileSlangWithPTXMethod(
        globalSession,
        kDirectNVVMIntegerBitAndSource,
        SLANG_EMIT_CUDA_VIA_NVRTC,
        nvrtcCode,
        nvrtcDiagnostics);
    if (SLANG_FAILED(nvrtcResult))
    {
        const String text = _getBlobText(nvrtcDiagnostics);
        if (text.getLength())
            getTestReporter()->message(TestMessageType::Info, text.getBuffer());
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcCode != nullptr);

    PTXEntrySummary nvvmSummary;
    PTXEntrySummary nvrtcSummary;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvvmCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvvmSummary)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_summarizePTXEntry(
        _getBlobText(nvrtcCode).getUnownedSlice(),
        toSlice("computeMain"),
        nvrtcSummary)));
    static const uint32_t kParameterWidths[] = {64, 32, 32};
    SLANG_CHECK(
        _hasPTXParameterWidths(nvvmSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(
        _hasPTXParameterWidths(nvrtcSummary, kParameterWidths, SLANG_COUNT_OF(kParameterWidths)));
    SLANG_CHECK(_haveEqualPTXParameterWidths(nvvmSummary, nvrtcSummary));
    SLANG_CHECK(nvvmSummary.hasBitAnd32);
    SLANG_CHECK(nvrtcSummary.hasBitAnd32);
    SLANG_CHECK(nvvmSummary.hasGlobalStore32);
    SLANG_CHECK(nvrtcSummary.hasGlobalStore32);
}

SLANG_UNIT_TEST(nvvmSlangRealScalarPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarFunctions());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring Slang scalar ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring Slang scalar ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const char* kSources[] = {
        kDirectNVVMWriteScalarSource,
        kDirectNVVMCopyScalarSource,
        kDirectNVVMChooseScalarSource,
        kDirectNVVMIntegerConstantSource,
        kDirectNVVMMergePhiSource,
        kDirectNVVMFiniteLoopSource,
        kDirectNVVMScalarFunctionSource,
    };
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (const char* source : kSources)
    {
        for (SlangEmitCUDAMethod method : kMethods)
        {
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _compileSlangWithPTXMethod(globalSession, source, method, code, diagnostics)));
            ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
            SLANG_CHECK_ABORT(ptxArtifact != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangRealPointerOffsetPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarPointerArithmetic());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealFixedDeviceArrayPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarArrayAddressing());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerMultiplyPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerMultiply());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMIntegerMultiplySource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangRealIntegerBitAndPtxasAccepts)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitAnd());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND ptxas test because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMIntegerBitAndSource,
            method,
            code,
            diagnostics)));
        ComPtr<IArtifact> ptxArtifact = _createPTXArtifact(code);
        SLANG_CHECK_ABORT(ptxArtifact != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(ptxArtifact, ptxasPath)));
    }
}

SLANG_UNIT_TEST(nvvmSlangScalarRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarFunctions());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    int computeMinor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMinor,
            kCudaDeviceAttributeComputeCapabilityMinor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    struct RuntimeCase
    {
        const char* source;
        ScalarRuntimeOperation operation;
        int x;
        int y;
        int expected;
    };
    static const RuntimeCase kCases[] = {
        {kDirectNVVMWriteScalarSource, ScalarRuntimeOperation::Write, 37, 0, 37},
        {kDirectNVVMCopyScalarSource, ScalarRuntimeOperation::Copy, -17, 0, -17},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, 2, 5, 7},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, 7, 3, 4},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, 5, 5, 0},
        {kDirectNVVMChooseScalarSource, ScalarRuntimeOperation::Choose, -2, 1, -1},
        {kDirectNVVMIntegerConstantSource, ScalarRuntimeOperation::Write, 41, 0, 42},
        {kDirectNVVMIntegerConstantSource, ScalarRuntimeOperation::Write, -2, 0, -1},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, 2, 5, 2},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, 7, 3, 3},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, 5, 5, 5},
        {kDirectNVVMMergePhiSource, ScalarRuntimeOperation::Choose, -2, 1, -2},
        {kDirectNVVMFiniteLoopSource, ScalarRuntimeOperation::Write, 0, 0, 0},
        {kDirectNVVMFiniteLoopSource, ScalarRuntimeOperation::Write, 5, 0, 10},
        {kDirectNVVMFiniteLoopSource, ScalarRuntimeOperation::Write, 7, 0, 21},
        {kDirectNVVMScalarFunctionSource, ScalarRuntimeOperation::Write, 5, 0, 13},
        {kDirectNVVMScalarFunctionSource, ScalarRuntimeOperation::Write, -2, 0, -1},
    };
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (const auto& runtimeCase : kCases)
    {
        for (SlangEmitCUDAMethod method : kMethods)
        {
            ComPtr<slang::IBlob> code;
            ComPtr<slang::IBlob> diagnostics;
            const SlangResult compileResult = _compileSlangWithPTXMethod(
                globalSession,
                runtimeCase.source,
                method,
                code,
                diagnostics);
            if (SLANG_FAILED(compileResult))
            {
                const String text = _getBlobText(diagnostics);
                if (text.getLength())
                    getTestReporter()->message(TestMessageType::Info, text.getBuffer());
            }
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
            SLANG_CHECK_ABORT(code != nullptr);
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                runtimeCase.operation,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangPointerOffsetRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarPointerArithmetic());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring pointer-offset runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMPointerOffsetSource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runPointerOffsetKernel(cuda, code, 2, false)));
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runPointerOffsetKernel(cuda, code, -1, true)));
    }
}

SLANG_UNIT_TEST(nvvmSlangFixedDeviceArrayRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarArrayAddressing());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because the device is older than sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring fixed-array runtime differential because libNVVM or NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }

    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    static const int kIndices[] = {0, 3};
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMFixedDeviceArraySource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (int index : kIndices)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runPointerOffsetKernel(cuda, code, index, false)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerMultiplyRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerMultiply());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply runtime differential because the device is older than "
            "sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-multiply runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct MultiplyRuntimeCase
    {
        int x;
        int y;
        int expected;
    };
    static const MultiplyRuntimeCase kCases[] = {
        {6, 7, 42},
        {-7, 6, -42},
        {0, -19, 0},
    };
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMIntegerMultiplySource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::Multiply,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmSlangIntegerBitAndRuntimeMatchesNVRTC)
{
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    SLANG_CHECK_ABORT(preflightBuilder.supportsScalarIntegerBitAnd());

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND runtime differential because the CUDA driver is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND runtime differential because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }

    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    if (computeMajor < 7)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND runtime differential because the device is older than "
            "sm_70.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    ComPtr<slang::IGlobalSession> globalSession;
    SLANG_CHECK_ABORT(
        slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
    if (SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)) ||
        SLANG_FAILED(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVRTC)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring integer-bit-AND runtime differential because libNVVM or NVRTC was not "
            "found.");
        SLANG_IGNORE_TEST;
    }

    struct BitAndRuntimeCase
    {
        int x;
        int y;
        int expected;
    };
    static const BitAndRuntimeCase kCases[] = {
        {0x5a, 0x3c, 0x18},
        {-1, 0x12345678, 0x12345678},
        {-2, -4, -4},
        {0, -1, 0},
    };
    static const SlangEmitCUDAMethod kMethods[] = {
        SLANG_EMIT_CUDA_VIA_NVVM,
        SLANG_EMIT_CUDA_VIA_NVRTC,
    };
    for (SlangEmitCUDAMethod method : kMethods)
    {
        ComPtr<slang::IBlob> code;
        ComPtr<slang::IBlob> diagnostics;
        const SlangResult compileResult = _compileSlangWithPTXMethod(
            globalSession,
            kDirectNVVMIntegerBitAndSource,
            method,
            code,
            diagnostics);
        if (SLANG_FAILED(compileResult))
        {
            const String text = _getBlobText(diagnostics);
            if (text.getLength())
                getTestReporter()->message(TestMessageType::Info, text.getBuffer());
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
        SLANG_CHECK_ABORT(code != nullptr);
        for (const auto& runtimeCase : kCases)
        {
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runScalarKernel(
                cuda,
                code,
                ScalarRuntimeOperation::BitAnd,
                runtimeCase.x,
                runtimeCase.y,
                runtimeCase.expected)));
        }
    }
}

SLANG_UNIT_TEST(nvvmIRBuilderCompilesEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    static const char kKernelName[] = "slangSlice3aCompiledEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        String(),
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode compile test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kKernelName)));
}

SLANG_UNIT_TEST(nvvmIRBuilderCoexistsWithLLVM21)
{
    StringBuilder childOrderBuilder;
    if (SLANG_SUCCEEDED(PlatformUtil::getEnvironmentVariable(
            toSlice(kNVVMCoexistenceChildEnv),
            childOrderBuilder)) &&
        childOrderBuilder.getLength())
    {
        const String childOrder = childOrderBuilder.produceString();
        NVVMLLVMLoadOrder order = NVVMLLVMLoadOrder::LLVMFirst;
        if (childOrder == "llvm-first")
            order = NVVMLLVMLoadOrder::LLVMFirst;
        else if (childOrder == "nvvm-first")
            order = NVVMLLVMLoadOrder::NVVMFirst;
        else
        {
            getTestReporter()->message(
                TestMessageType::TestFailure,
                "Unknown NVVM/LLVM coexistence child order.");
            SLANG_CHECK_ABORT(false);
        }

        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_exerciseNVVMLLVMCoexistence(unitTestContext, order)));
        return;
    }

    // Establish availability in the parent so dependency absence ignores this test instead of
    // becoming an apparently successful ignored child. Loading here cannot affect either worker:
    // each child invocation below executes the probe in its own fully isolated test-server.
    NVVMIRBuilder preflightBuilder;
    _requireRealNVVMBuilder(unitTestContext, preflightBuilder);
    const SlangResult llvmResult = _queryLLVM21(unitTestContext);
    if (llvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring LLVM coexistence test because slang-llvm was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(llvmResult));

    struct OrderCase
    {
        const char* name;
        const char* environmentValue;
    };
    static const OrderCase kOrders[] = {
        {"LLVM 21 then LLVM 14 NVVM", "llvm-first"},
        {"LLVM 14 NVVM then LLVM 21", "nvvm-first"},
    };

    for (const auto& order : kOrders)
    {
        CommandLine commandLine;
        commandLine.setExecutableLocation(
            ExecutableLocation(unitTestContext->executableDirectory, "slang-test"));
        commandLine.addArg("-use-fully-isolated-test-server");
        commandLine.addArg("-server-count");
        commandLine.addArg("1");
        commandLine.addArg("-disable-retries");
        commandLine.addArg("-skip-api-detection");
        commandLine.addArg(kNVVMCoexistenceTestName);

        ExecuteResult childResult;
        childResult.init();
        SlangResult executeResult = SLANG_FAIL;
        {
            SlangUnitTest::ScopedEnvVar childOrder(
                kNVVMCoexistenceChildEnv,
                order.environmentValue);
            executeResult = ProcessUtil::execute(commandLine, childResult);
        }
        const bool reportedOnePassingTest =
            childResult.standardOutput.indexOf("100% of tests passed (1/1)") >= 0;
        if (SLANG_FAILED(executeResult) || childResult.resultCode != 0 || !reportedOnePassingTest)
        {
            _reportCoexistenceChildFailure(order.name, executeResult, childResult);
        }
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(executeResult));
        SLANG_CHECK(childResult.resultCode == 0);
        SLANG_CHECK(reportedOnePassingTest);
    }
}

// Exercise the public lazy-discovery path, not just the locator in isolation. This catches a new
// pass-through enum being added without registering its default downstream compiler locator.
SLANG_UNIT_TEST(nvvmPassThroughDiscoversInjectedLibrary)
{
    gFakeNVVM.reset();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        SLANG_CHECK(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM) == SLANG_OK);
        int major = -1;
        int minor = -1;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NVVM, &major, &minor) ==
            SLANG_OK);
        SLANG_CHECK(major == 2);
        SLANG_CHECK(minor == 0);
        SLANG_CHECK(gFakeNVVM.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVM.loadedPath == "nvvm");
    }
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmLocatorRejectsMissingRequiredSymbols)
{
    gFakeNVVM.reset();
    for (const char* missingSymbol : kRequiredSymbols)
    {
        gFakeNVVM.missingSymbol = missingSymbol;
        {
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
            RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
            SlangResult result = NVVMDownstreamCompilerUtil::locateCompilers(String(), loader, set);
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(result != SLANG_E_NOT_FOUND);
            SLANG_CHECK(!set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
        }
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmLocatorAcceptsMissingOptionalSymbols)
{
    gFakeNVVM.reset();
    gFakeNVVM.omitOptionalSymbols = true;
    {
        RefPtr<DownstreamCompilerSet> set;
        IDownstreamCompiler* compiler = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
        SLANG_CHECK(compiler != nullptr);
        SLANG_CHECK(compiler->getDesc().type == SLANG_PASS_THROUGH_NVVM);
        SLANG_CHECK(compiler->getDesc().version == SemanticVersion(2, 0));
    }
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmLocatorRanksNumericCandidates)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    gFakeNVVM.reset();
    TempDirectory tempDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(tempDirectory)));

#if SLANG_WINDOWS_FAMILY
    const String lowerPath = Path::combine(tempDirectory.path, "nvvm64_90_0.dll");
    const String higherPath = Path::combine(tempDirectory.path, "nvvm64_100_0.dll");
    const String expectedLoadPath = Path::getPathWithoutExt(higherPath);
#else
    const String lowerPath = Path::combine(tempDirectory.path, "libnvvm.so.9");
    const String higherPath = Path::combine(tempDirectory.path, "libnvvm.so.10");
    const String expectedLoadPath = higherPath;
#endif
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(lowerPath, String())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(higherPath, String())));

    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        NVVMDownstreamCompilerUtil::locateCompilers(tempDirectory.path, loader, set)));
    SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    SLANG_CHECK(recordingLoader->loadRequests[0] == expectedLoadPath);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmLocatorNormalizesDecoratedExplicitFile)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY
    gFakeNVVM.reset();
    TempDirectory tempDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(tempDirectory)));

#if SLANG_WINDOWS_FAMILY
    const String decoratedPath = Path::combine(tempDirectory.path, "nvvm64_100_0.dll");
    const String expectedLoadPath = Path::getPathWithoutExt(decoratedPath);
#elif SLANG_LINUX_FAMILY
    const String decoratedPath = Path::combine(tempDirectory.path, "libnvvm.so");
    const String expectedLoadPath = Path::combine(tempDirectory.path, "nvvm");
#else
    const String decoratedPath = Path::combine(tempDirectory.path, "libnvvm.dylib");
    const String expectedLoadPath = Path::combine(tempDirectory.path, "nvvm");
#endif
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(decoratedPath, String())));

    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(NVVMDownstreamCompilerUtil::locateCompilers(decoratedPath, loader, set)));
    SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    SLANG_CHECK(recordingLoader->loadRequests[0] == expectedLoadPath);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmCompilerOwnsLibrary)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* foundCompiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, foundCompiler)));
    ComPtr<IDownstreamCompiler> compiler(foundCompiler);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 1);

    set.setNull();
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 1);
    compiler.setNull();
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsInvalidInputs)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    ComPtr<IArtifact> validArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> wrongArtifact =
        ArtifactUtil::createArtifactForCompileTarget(SLANG_HOST_LLVM_IR);
    wrongArtifact->addRepresentationUnknown(StringBlob::create(UnownedStringSlice(kMinimalNVVMIR)));
    ComPtr<IArtifact> hostBitcodeArtifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::LLVMIR, ArtifactStyle::Host));
    hostBitcodeArtifact->addRepresentationUnknown(
        RawBlob::create(kMinimalNVVMBitcode, SLANG_COUNT_OF(kMinimalNVVMBitcode)));

    IArtifact* oneValidSource[] = {validArtifact};
    IArtifact* twoValidSources[] = {validArtifact, validArtifact};
    IArtifact* oneWrongSource[] = {wrongArtifact};
    IArtifact* oneHostBitcodeSource[] = {hostBitcodeArtifact};
    DownstreamCompileOptions::CapabilityVersion validCapability;
    validCapability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    validCapability.version.set(7, 5);
    DownstreamCompileOptions::CapabilityVersion malformedCapability;
    malformedCapability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    malformedCapability.version.set(7, 10);

    DownstreamCompileOptions baseOptions;
    baseOptions.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    baseOptions.targetType = SLANG_PTX;
    baseOptions.debugInfoType = DownstreamCompileOptions::DebugInfoType::None;

    // No source artifacts.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // More than one source artifact.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(twoValidSources, SLANG_COUNT_OF(twoValidSources));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // LLVM IR for the host has the right payload but the wrong artifact style.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneWrongSource, SLANG_COUNT_OF(oneWrongSource));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // Binary LLVM IR is accepted only when it carries the kernel style.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts =
            makeSlice(oneHostBitcodeSource, SLANG_COUNT_OF(oneHostBitcodeSource));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // A valid source still requires an explicit CUDA architecture capability.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneValidSource, SLANG_COUNT_OF(oneValidSource));
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // CUDA architecture minor versions contain one decimal digit.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneValidSource, SLANG_COUNT_OF(oneValidSource));
        options.requiredCapabilityVersions = makeSlice(&malformedCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }
}

SLANG_UNIT_TEST(nvvmCompilerAcceptsLLVMBitcodeArtifact)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    // This deliberately contains several embedded NULs. The artifact descriptor identifies the
    // bytes as bitcode; Slang must forward the complete buffer without treating it as a string.
    static const uint8_t bitcode[] = {0x42, 0x43, 0xc0, 0xde, 0x00, 0x11, 0x00, 0x22};
    ComPtr<IArtifact> sourceArtifact = ArtifactUtil::createArtifact(ArtifactDesc::make(
        ArtifactKind::ObjectCode,
        ArtifactPayload::LLVMIR,
        ArtifactStyle::Kernel));
    sourceArtifact->addRepresentationUnknown(RawBlob::create(bitcode, SLANG_COUNT_OF(bitcode)));

    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_OK);

    ComPtr<ISlangBlob> outputBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, outputBlob.writeRef())));
    SLANG_CHECK(outputBlob->getBufferSize() == ::strlen(kFakePTX));
    SLANG_CHECK(::memcmp(outputBlob->getBufferPointer(), kFakePTX, ::strlen(kFakePTX)) == 0);

    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.addedModule.getLength() == SLANG_COUNT_OF(bitcode));
    SLANG_CHECK(::memcmp(gFakeNVVM.addedModule.getBuffer(), bitcode, SLANG_COUNT_OF(bitcode)) == 0);
    SLANG_CHECK(gFakeNVVM.addedModuleName == "slang-nvvm-input");
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
}

SLANG_UNIT_TEST(nvvmCompilerCompilesTrivialIR)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    ComPtr<slang::IBlob> versionString;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compiler->getVersionString(versionString.writeRef())));
    String version(UnownedStringSlice(
        (const char*)versionString->getBufferPointer(),
        versionString->getBufferSize()));
    SLANG_CHECK(version.indexOf("2.0") >= 0);
    SLANG_CHECK(version.indexOf("nvvm-ir=2.0") >= 0);
    SLANG_CHECK(version.indexOf("debug=3.0") >= 0);

    CompileSettings settings;
    settings.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::None;
    settings.debugInfoType = DownstreamCompileOptions::DebugInfoType::Maximal;
    settings.floatingPointMode = DownstreamCompileOptions::FloatingPointMode::Precise;
    settings.denormalModeFp32 = DownstreamCompileOptions::FloatingPointDenormalMode::Preserve;
    settings.addFakeCompilerArgument = true;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);

    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics =
        findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_OK);

    ComPtr<ISlangBlob> outputBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, outputBlob.writeRef())));
    SLANG_CHECK(outputBlob->getBufferSize() == ::strlen(kFakePTX));
    SLANG_CHECK(::memcmp(outputBlob->getBufferPointer(), kFakePTX, ::strlen(kFakePTX)) == 0);
    if (outputBlob->getBufferSize())
    {
        const char* bytes = (const char*)outputBlob->getBufferPointer();
        SLANG_CHECK(bytes[outputBlob->getBufferSize() - 1] != 0);
    }

    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.addedModule == kMinimalNVVMIR);
    SLANG_CHECK(gFakeNVVM.addedModuleName == "slang-nvvm-input");
    SLANG_CHECK(gFakeNVVM.verifyOptions.getCount() == 8);
    SLANG_CHECK(gFakeNVVM.compileOptions.getCount() == 8);
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-arch=compute_75"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-g"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-opt=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-ftz=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-prec-div=1"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-prec-sqrt=1"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-fma=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-fake-nvvm-option"));
    for (Index i = 0; i < gFakeNVVM.compileOptions.getCount(); ++i)
        SLANG_CHECK(gFakeNVVM.verifyOptions[i] == gFakeNVVM.compileOptions[i]);

    // Maximal debug metadata is only valid for unoptimized code. Reject the combination before
    // creating a libNVVM program so the policy cannot be silently weakened by option ordering.
    gFakeNVVM.resetCalls();
    settings.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::High;
    ComPtr<IArtifact> invalidOutput;
    SlangResult invalidResult =
        _compileNVVM(compiler, sourceArtifact, settings, invalidOutput.writeRef());
    SLANG_CHECK(invalidResult == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(invalidOutput != nullptr);
    diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(invalidOutput);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_E_INVALID_ARG);
    SLANG_CHECK(_diagnosticsContain(diagnostics, "requires optimization to be disabled"));
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsTerminatorOnlyResult)
{
    _checkRejectedCompiledResult(FakeResultMode::TerminatorOnly);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsUnterminatedResult)
{
    _checkRejectedCompiledResult(FakeResultMode::Unterminated);
}

SLANG_UNIT_TEST(nvvmCompilerDestroysProgramsOnFailure)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;

    static const FakeFailure kFailures[] = {
        FakeFailure::CreateProgram,
        FakeFailure::AddModule,
        FakeFailure::VerifyProgram,
        FakeFailure::CompileProgram,
        FakeFailure::GetResultSize,
        FakeFailure::GetResult,
        FakeFailure::GetLogSize,
        FakeFailure::GetLog,
    };
    for (FakeFailure failure : kFailures)
    {
        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = failure;
        ComPtr<IArtifact> outputArtifact;
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        IArtifactDiagnostics* diagnostics =
            findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
        SLANG_CHECK(
            gFakeNVVM.destroyProgramCallCount == (failure == FakeFailure::CreateProgram ? 0 : 1));
    }
    gFakeNVVM.failure = FakeFailure::None;
}

SLANG_UNIT_TEST(nvvmCompilerClassifiesVerificationAndCompilationFailures)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;

    struct FailureCase
    {
        FakeFailure operation;
        TestNVVMResult nvvmResult;
        SlangResult callResult;
        SlangResult diagnosticResult;
    };
    static const FailureCase kCases[] = {
        {FakeFailure::VerifyProgram, TestNVVMResult::Compilation, SLANG_OK, SLANG_FAIL},
        {FakeFailure::CompileProgram, TestNVVMResult::Compilation, SLANG_OK, SLANG_FAIL},
        {FakeFailure::VerifyProgram,
         TestNVVMResult::OutOfMemory,
         SLANG_E_OUT_OF_MEMORY,
         SLANG_E_OUT_OF_MEMORY},
        {FakeFailure::CompileProgram,
         TestNVVMResult::OutOfMemory,
         SLANG_E_OUT_OF_MEMORY,
         SLANG_E_OUT_OF_MEMORY},
        {FakeFailure::VerifyProgram, TestNVVMResult::Cancelled, SLANG_E_ABORT, SLANG_E_ABORT},
        {FakeFailure::CompileProgram, TestNVVMResult::Cancelled, SLANG_E_ABORT, SLANG_E_ABORT},
    };

    for (const auto& failureCase : kCases)
    {
        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = failureCase.operation;
        gFakeNVVM.failureResult = failureCase.nvvmResult;
        ComPtr<IArtifact> outputArtifact;
        SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK(result == failureCase.callResult);
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(diagnostics->getResult() == failureCase.diagnosticResult);
        SLANG_CHECK(diagnostics->getCount() >= 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    }
    gFakeNVVM.failure = FakeFailure::None;
    gFakeNVVM.failureResult = TestNVVMResult::Compilation;
}

SLANG_UNIT_TEST(nvvmCompilerUsesErrorStringForEmptyLog)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.failure = FakeFailure::VerifyProgram;
    gFakeNVVM.programLog = String();

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    // Verification/compilation failures are represented on the artifact so the caller can consume
    // libNVVM's diagnostics through the same channel as other downstream compilers.
    SLANG_CHECK(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    IArtifactDiagnostics* diagnostics =
        findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(_diagnosticsContain(diagnostics, "libNVVM verification failed"));
    SLANG_CHECK(_diagnosticsContain(diagnostics, "fake NVVM compilation failure"));
    SLANG_CHECK(diagnostics->getCount() >= 1);
}

SLANG_UNIT_TEST(nvvmCompilerPreservesVerifierLogOnCompilationFailure)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.usePhaseLogs = true;
    gFakeNVVM.verifierLog = "fake verifier success note";
    gFakeNVVM.compilerLog = "fake compiler failure detail";
    gFakeNVVM.failure = FakeFailure::CompileProgram;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SLANG_CHECK(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));

    const TerminatedCharSlice raw = diagnostics->getRaw();
    const String rawText(UnownedStringSlice(raw.data, raw.count));
    const Index verifierLogIndex = rawText.indexOf(gFakeNVVM.verifierLog);
    const Index compilerLogIndex = rawText.indexOf(gFakeNVVM.compilerLog);
    SLANG_CHECK(verifierLogIndex >= 0);
    SLANG_CHECK(compilerLogIndex > verifierLogIndex);
    SLANG_CHECK(rawText.indexOf("libNVVM compilation failed") > verifierLogIndex);
}

SLANG_UNIT_TEST(nvvmCompilerCompilesEmptyKernel)
{
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SlangResult locateResult = _locateRealNVVM(String(), set, compiler);
    if (locateResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real libNVVM smoke test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outputArtifact);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));

    ComPtr<ISlangBlob> ptxBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())));
    String ptx(
        UnownedStringSlice((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize()));
    SLANG_CHECK(ptx.indexOf(".visible .entry testEmpty") >= 0);
    SLANG_CHECK(ptxBlob->getBufferSize() > 0);
    if (ptxBlob->getBufferSize())
    {
        const char* bytes = (const char*)ptxBlob->getBufferPointer();
        SLANG_CHECK(bytes[ptxBlob->getBufferSize() - 1] != 0);
    }
}

SLANG_UNIT_TEST(nvvmCompilerCompilesEmptyKernelBitcode)
{
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SlangResult locateResult = _locateRealNVVM(String(), set, compiler);
    if (locateResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real libNVVM bitcode test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> sourceArtifact = _createNVVMBitcodeArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
        _reportArtifactDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));

    ComPtr<ISlangBlob> ptxBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())));
    String ptx(
        UnownedStringSlice((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize()));
    SLANG_CHECK(ptx.indexOf(".visible .entry testEmpty") >= 0);
}

SLANG_UNIT_TEST(nvvmPtxasAcceptsEmptyKernel)
{
    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    // Assemble PTX produced from bitcode so the compatibility fixture crosses the entire local
    // offline toolchain. The preceding real test keeps the textual bootstrap path covered.
    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        cudaRoot,
        kMinimalNVVMBitcode,
        SLANG_COUNT_OF(kMinimalNVVMBitcode),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice("testEmpty")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(outputArtifact, ptxasPath)));
}

SLANG_UNIT_TEST(nvvmIRBuilderPtxasAcceptsEmptyKernel)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    static const char kKernelName[] = "slangSlice3aPtxasEmpty";
    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _buildEmptyNVVMKernel(builder, toSlice(kKernelName), assemblyBlob, bitcodeBlob)));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        cudaRoot,
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring generated-bitcode ptxas test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kKernelName)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(outputArtifact, ptxasPath)));
}

SLANG_UNIT_TEST(nvvmIRBuilderPtxasAcceptsScalarReferenceKernels)
{
    NVVMIRBuilder builder;
    _requireRealNVVMBuilder(unitTestContext, builder);
    SLANG_CHECK_ABORT(builder.supportsScalarOperations());

    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar ptxas test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    ComPtr<ISlangBlob> assemblyBlob;
    ComPtr<ISlangBlob> bitcodeBlob;
    String assemblyDiagnostics;
    String bitcodeDiagnostics;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_buildScalarReferenceModule(
        builder,
        assemblyBlob,
        assemblyDiagnostics,
        bitcodeBlob,
        bitcodeDiagnostics)));
    SLANG_CHECK_ABORT(bitcodeBlob != nullptr);
    SLANG_CHECK(assemblyDiagnostics.getLength() == 0);
    SLANG_CHECK(bitcodeDiagnostics.getLength() == 0);

    ComPtr<IArtifact> nvvmArtifact;
    const SlangResult nvvmResult = _compileRealNVVMBitcode(
        cudaRoot,
        bitcodeBlob->getBufferPointer(),
        bitcodeBlob->getBufferSize(),
        nvvmArtifact);
    if (nvvmResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar ptxas test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvvmResult));
    SLANG_CHECK_ABORT(nvvmArtifact != nullptr);

    ComPtr<IArtifact> nvrtcArtifact;
    const SlangResult nvrtcResult =
        _compileRealNVRTCSource(toSlice(kScalarReferenceCUDASource), nvrtcArtifact);
    if (nvrtcResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring scalar ptxas test because NVRTC was not found.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(nvrtcResult));
    SLANG_CHECK_ABORT(nvrtcArtifact != nullptr);

    SLANG_CHECK(_ptxContainsEntry(nvvmArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(nvvmArtifact, toSlice(kCopyScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(nvrtcArtifact, toSlice(kWriteScalarKernelName)));
    SLANG_CHECK(_ptxContainsEntry(nvrtcArtifact, toSlice(kCopyScalarKernelName)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(nvvmArtifact, ptxasPath)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(nvrtcArtifact, ptxasPath)));
}
