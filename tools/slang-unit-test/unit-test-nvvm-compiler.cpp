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
struct FakeNVVMBuilderParameterStorage
{
};
struct FakeNVVMBuilderLoadStorage
{
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
        getPointerTypeCallCount = 0;
        getFunctionParameterCallCount = 0;
        emitLoadCallCount = 0;
        emitStoreCallCount = 0;
        integerBitWidth = 0;
        pointerAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
        functionParameterIndex = 0;
        loadAlignment = 0;
        storeAlignment = 0;
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
        returnNullModule = false;
        returnNullIntegerType = false;
        failIntegerTypeAfterWrite = false;
        reportMismatchedWriteSize = false;
        verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
        serializationWithDiagnosticsResult = SLANG_OK;
        verificationDiagnostic = String();
        omitValidSerializedOutput = false;
        reportMismatchedSerializedDiagnosticWriteSize = false;
        reportMismatchedVerificationDiagnosticWriteSize = false;
        reportMismatchedVerificationStatus = false;
        loadedPath = String();
        liveLibraryCount = 0;
        destroyedLibraryCount = 0;
        resetCalls();
    }

    SlangNVVMBuilderAPI_V1 api = {};
    SlangNVVMBuilderAPI_V2 apiV2 = {};
    bool omitAPISymbol = false;
    bool omitAPIV2Symbol = true;
    bool returnNullModule = false;
    bool returnNullIntegerType = false;
    bool failIntegerTypeAfterWrite = false;
    bool reportMismatchedWriteSize = false;
    SlangNVVMVerificationStatus_2 verificationStatus = SLANG_NVVM_VERIFICATION_VALID;
    SlangNVVMResult_1 serializationWithDiagnosticsResult = SLANG_OK;
    String verificationDiagnostic;
    bool omitValidSerializedOutput = false;
    bool reportMismatchedSerializedDiagnosticWriteSize = false;
    bool reportMismatchedVerificationDiagnosticWriteSize = false;
    bool reportMismatchedVerificationStatus = false;
    String loadedPath;
    int liveLibraryCount = 0;
    int destroyedLibraryCount = 0;

    FakeNVVMBuilderModuleStorage moduleStorage;
    FakeNVVMBuilderVoidTypeStorage voidTypeStorage;
    FakeNVVMBuilderFunctionTypeStorage functionTypeStorage;
    FakeNVVMBuilderFunctionStorage functionStorage;
    FakeNVVMBuilderBlockStorage blockStorage;
    FakeNVVMBuilderIntegerTypeStorage integerTypeStorage;
    FakeNVVMBuilderPointerTypeStorage pointerTypeStorage;
    FakeNVVMBuilderParameterStorage parameterStorage;
    FakeNVVMBuilderLoadStorage loadStorage;

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
    int getPointerTypeCallCount = 0;
    int getFunctionParameterCallCount = 0;
    int emitLoadCallCount = 0;
    int emitStoreCallCount = 0;
    uint32_t integerBitWidth = 0;
    SlangNVVMAddressSpace_2 pointerAddressSpace = SLANG_NVVM_ADDRESS_SPACE_GENERIC;
    size_t functionParameterIndex = 0;
    uint32_t loadAlignment = 0;
    uint32_t storeAlignment = 0;
    String moduleName;
    String functionName;
    String blockName;
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

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderFunctionType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.functionTypeStorage);
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderFunction()
{
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.functionStorage);
}

static SlangNVVMBlockHandle_1 _getFakeNVVMBuilderBlock()
{
    return reinterpret_cast<SlangNVVMBlockHandle_1>(&gFakeNVVMBuilder.blockStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderIntegerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.integerTypeStorage);
}

static SlangNVVMTypeHandle_1 _getFakeNVVMBuilderPointerType()
{
    return reinterpret_cast<SlangNVVMTypeHandle_1>(&gFakeNVVMBuilder.pointerTypeStorage);
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderParameter()
{
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.parameterStorage);
}

static SlangNVVMValueHandle_1 _getFakeNVVMBuilderLoad()
{
    return reinterpret_cast<SlangNVVMValueHandle_1>(&gFakeNVVMBuilder.loadStorage);
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
    ++gFakeNVVMBuilder.getFunctionTypeCallCount;
    if (module != _getFakeNVVMBuilderModule() || resultType != _getFakeNVVMBuilderVoidType() ||
        parameterTypes || parameterCount || !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    *outType = _getFakeNVVMBuilderFunctionType();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderDeclareFunction(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 functionType,
    const char* name,
    size_t nameSize,
    SlangNVVMValueHandle_1* outFunction)
{
    ++gFakeNVVMBuilder.declareFunctionCallCount;
    if (module != _getFakeNVVMBuilderModule() ||
        functionType != _getFakeNVVMBuilderFunctionType() || (!name && nameSize) || !outFunction)
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.functionName = String(UnownedStringSlice(name, nameSize));
    *outFunction = _getFakeNVVMBuilderFunction();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderCreateBlock(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function,
    const char* name,
    size_t nameSize,
    SlangNVVMBlockHandle_1* outBlock)
{
    ++gFakeNVVMBuilder.createBlockCallCount;
    if (module != _getFakeNVVMBuilderModule() || function != _getFakeNVVMBuilderFunction() ||
        (!name && nameSize) || !outBlock)
    {
        return SLANG_E_INVALID_ARG;
    }
    gFakeNVVMBuilder.blockName = String(UnownedStringSlice(name, nameSize));
    *outBlock = _getFakeNVVMBuilderBlock();
    return SLANG_OK;
}

static SlangResult SLANG_NVVM_CALL
_fakeNVVMBuilderSetInsertBlock(SlangNVVMModuleHandle_1 module, SlangNVVMBlockHandle_1 block)
{
    ++gFakeNVVMBuilder.setInsertBlockCallCount;
    return module == _getFakeNVVMBuilderModule() && block == _getFakeNVVMBuilderBlock()
               ? SLANG_OK
               : SLANG_E_INVALID_ARG;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderEmitReturnVoid(SlangNVVMModuleHandle_1 module)
{
    ++gFakeNVVMBuilder.emitReturnVoidCallCount;
    return module == _getFakeNVVMBuilderModule() ? SLANG_OK : SLANG_E_INVALID_ARG;
}

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderMarkFunctionAsKernel(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMValueHandle_1 function)
{
    ++gFakeNVVMBuilder.markFunctionAsKernelCallCount;
    return module == _getFakeNVVMBuilderModule() && function == _getFakeNVVMBuilderFunction()
               ? SLANG_OK
               : SLANG_E_INVALID_ARG;
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

static SlangResult SLANG_NVVM_CALL _fakeNVVMBuilderGetPointerType(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMTypeHandle_1 pointeeType,
    SlangNVVMAddressSpace_2 addressSpace,
    SlangNVVMTypeHandle_1* outType)
{
    ++gFakeNVVMBuilder.getPointerTypeCallCount;
    gFakeNVVMBuilder.pointerAddressSpace = addressSpace;
    if (module != _getFakeNVVMBuilderModule() || pointeeType != _getFakeNVVMBuilderIntegerType() ||
        !outType)
    {
        return SLANG_E_INVALID_ARG;
    }
    *outType = _getFakeNVVMBuilderPointerType();
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
    if (module != _getFakeNVVMBuilderModule() || function != _getFakeNVVMBuilderFunction() ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }
    *outValue = _getFakeNVVMBuilderParameter();
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
    if (module != _getFakeNVVMBuilderModule() || pointer != _getFakeNVVMBuilderParameter() ||
        !outValue)
    {
        return SLANG_E_INVALID_ARG;
    }
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
    return module == _getFakeNVVMBuilderModule() && value == _getFakeNVVMBuilderLoad() &&
                   pointer == _getFakeNVVMBuilderParameter()
               ? SLANG_OK
               : SLANG_E_INVALID_ARG;
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
        if (gFakeNVVMBuilder.loadedPath != "slang-llvm-nvvm")
            return SLANG_E_NOT_FOUND;
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
    SLANG_RETURN_ON_FAIL(
        NVVMDownstreamCompilerUtil::locateCompilers(
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
    SLANG_RETURN_ON_FAIL(
        NVRTCDownstreamCompilerUtil::locateCompilers(
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
    ComPtr<IArtifact> artifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(
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
    location.isExplicit = SLANG_SUCCEEDED(
                              PlatformUtil::getEnvironmentVariable(
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
};

static SlangResult _summarizePTXEntry(
    const UnownedStringSlice& ptx,
    const UnownedStringSlice& entryPointName,
    PTXEntrySummary& outSummary)
{
    outSummary.parameterBitWidths.clear();
    outSummary.hasGlobalLoad32 = false;
    outSummary.hasGlobalStore32 = false;

    String signature;
    String body;
    SLANG_RETURN_ON_FAIL(_extractPTXEntry(ptx, entryPointName, signature, body));
    SLANG_RETURN_ON_FAIL(
        _collectPTXParameterWidths(signature.getUnownedSlice(), outSummary.parameterBitWidths));
    outSummary.hasGlobalLoad32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("ld.global"), 32);
    outSummary.hasGlobalStore32 =
        _ptxEntryHasInstruction(body.getUnownedSlice(), toSlice("st.global"), 32);
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
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            builder
                .getFunctionParameter(scope.module, _getFakeNVVMBuilderFunction(), 7, parameter)));
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
        SLANG_CHECK(gFakeNVVMBuilder.functionParameterIndex == 7);
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
        SLANG_CHECK_ABORT(builder.getAPIV2() != nullptr);
        SLANG_CHECK(builder.getAPIV2()->structureSize == sizeof(SlangNVVMBuilderAPI_V2));
    }
    SLANG_CHECK(gFakeNVVMBuilder.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVMBuilder.destroyedLibraryCount == 1);
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
    if (SLANG_SUCCEEDED(
            PlatformUtil::getEnvironmentVariable(
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
    ComPtr<IArtifact> sourceArtifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(
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
