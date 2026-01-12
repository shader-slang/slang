#define SLANG_LLVM_IMPL
#include "slang-llvm-builder.h"

#include "slang-llvm-jit-shared-library.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"

#include <compiler-core/slang-artifact-associated-impl.h>
#include <compiler-core/slang-artifact-associated.h>
#include <compiler-core/slang-artifact-desc-util.h>
#include <core/slang-blob.h>
#include <core/slang-com-object.h>
#include <core/slang-list.h>

using namespace slang;

namespace slang_llvm
{

using namespace Slang;

// This instance needs to exist so that we gain access to codegen flags in the
// downstream arguments.
static llvm::codegen::RegisterCodeGenFlags CGF;

// There doesn't appear to be an LLVM output stream for writing into a memory
// buffer. This implementation saves all written data directly into Slang's List
// type.
struct BinaryLLVMOutputStream : public llvm::raw_pwrite_stream
{
    List<uint8_t>& output;

    BinaryLLVMOutputStream(List<uint8_t>& output)
        : raw_pwrite_stream(true), output(output)
    {
        SetUnbuffered();
    }

    void write_impl(const char* Ptr, size_t Size) override
    {
        output.insertRange(output.getCount(), reinterpret_cast<const uint8_t*>(Ptr), Size);
    }

    void pwrite_impl(const char* Ptr, size_t Size, uint64_t Offset) override
    {
        memcpy(output.getBuffer() + Offset, reinterpret_cast<const uint8_t*>(Ptr), Size);
    }

    uint64_t current_pos() const override { return output.getCount(); }

    void reserveExtraSpace(uint64_t ExtraSize) override { output.reserve(tell() + ExtraSize); }
};

llvm::DenormalMode getLLVMDenormalMode(SlangFpDenormalMode mode)
{
    switch (mode)
    {
    default:
    case SLANG_FP_DENORM_MODE_ANY:
        return llvm::DenormalMode::getDefault();
    case SLANG_FP_DENORM_MODE_PRESERVE:
        return llvm::DenormalMode::getPreserveSign();
    case SLANG_FP_DENORM_MODE_FTZ:
        return llvm::DenormalMode::getPositiveZero();
    }
}

template<typename T, typename U>
static llvm::ArrayRef<T*> sliceToArrayRef(Slice<U> slice)
{
    // TODO: Should probably assert, but we're not at C++20 yet
    // static_assert(std::is_pointer_interconvertible_base_of_v<U, T>());
    return llvm::ArrayRef(reinterpret_cast<T* const*>(slice.begin()), slice.count);
}

llvm::StringRef charSliceToLLVM(CharSlice slice)
{
    return llvm::StringRef(slice.begin(), slice.count);
}

class LLVMBuilder : public ComBaseObject, public ILLVMBuilder
{
    LLVMBuilderOptions options;

    std::unique_ptr<llvm::LLVMContext> llvmContext;
    std::unique_ptr<llvm::Module> llvmModule;
    // This only links LLVM modules together, not object code. It's used for
    // inline IR.
    std::unique_ptr<llvm::Linker> llvmLinker;

    // These builder variants need to be stored separately. IRBuilderBase does
    // not have a virtual destructor :/ Only one should exist at a time and the
    // pointer is copied to llvmBuilder, which you should always use instead of
    // these directly.
    std::unique_ptr<llvm::IRBuilder<>> llvmBuilderOpt;
    std::unique_ptr<llvm::IRBuilder<llvm::NoFolder>> llvmBuilderNoOpt;
    llvm::IRBuilderBase* llvmBuilder;
    llvm::DICompileUnit* compileUnit;
    llvm::TargetMachine* targetMachine;
    llvm::DataLayout targetDataLayout;

    std::unique_ptr<llvm::DIBuilder> llvmDebugBuilder;

    // "Global constructors" are functions that get called when the executable
    // or library is loaded, similar to constructors of global variables in C++.
    List<llvm::Constant*> globalCtors;
    llvm::StructType* llvmCtorType;

    // This one is cached here, because we spam it constantly with every GEP.
    llvm::Type* byteType;

    llvm::DIScope* currentLocalScope = nullptr;

    struct VariableDebugInfo
    {
        // attached is true when the first related DebugValue has been
        // processed.
        bool attached = false;
        // isStackVar is true when the variable has been alloca'd and declared
        // as debugValue. Doesn't necessarily need further DebugValue tracking
        // after that, since a real variable now actually exists in LLVM.
        bool isStackVar = false;
    };
    Dictionary<llvm::DILocalVariable*, VariableDebugInfo> variableDebugInfoMap;

    // These enums represent built-in functions whose implementations must be
    // externally provided.
    enum class ExternalFunc
    {
        Printf
        // TODO: Texture sampling, RT functions?
    };
    // Map of external builtins. These are only forward-declared in the emitted
    // LLVM IR.
    Dictionary<ExternalFunc, llvm::Function*> externalFuncs;

public:
    typedef ComBaseObject Super;

    LLVMBuilder(LLVMBuilderOptions options, IArtifact** outErrorArtifact);
    ~LLVMBuilder();

    IArtifact* createErrorArtifact(const ArtifactDiagnostic& diagnostic);

    void optimize();
    void finalize();

    // This function inserts the given LLVM IR in the global scope.
    // Uses std::string due to that being what LLVM emits and ingests.
    void emitGlobalLLVMIR(const std::string& textIR);

    llvm::Function* getExternalBuiltin(ExternalFunc extFunc);

    void makeVariadicArgsCCompatible(
        Slice<LLVMInst*> args,
        Slice<bool> argIsSigned,
        List<LLVMInst*>& outArgs);


    //==========================================================================
    // IUnknown
    //==========================================================================
    SLANG_COM_BASE_IUNKNOWN_ALL

    void* getInterface(const Guid& guid);

    //==========================================================================
    // ICastable
    //==========================================================================
    SLANG_NO_THROW void* SLANG_MCALL castAs(const Guid& guid) override;

    //==========================================================================
    // ILLVMBuilder
    //==========================================================================

    SLANG_NO_THROW int SLANG_MCALL getPointerSizeInBits() override;
    SLANG_NO_THROW int SLANG_MCALL getStoreSizeOf(LLVMInst* value) override;

    SLANG_NO_THROW void SLANG_MCALL printType(String& outStr, LLVMType* type) override;
    SLANG_NO_THROW void SLANG_MCALL
    printValue(String& outStr, LLVMInst* value, bool withType) override;

    SLANG_NO_THROW LLVMType* SLANG_MCALL getVoidType() override;
    SLANG_NO_THROW LLVMType* SLANG_MCALL getIntType(int bitSize) override;
    SLANG_NO_THROW LLVMType* SLANG_MCALL getFloatType(int bitSize) override;
    SLANG_NO_THROW LLVMType* SLANG_MCALL getPointerType() override;
    SLANG_NO_THROW LLVMType* SLANG_MCALL
    getVectorType(int elementCount, LLVMType* elementType) override;
    SLANG_NO_THROW LLVMType* SLANG_MCALL getBufferType() override;
    SLANG_NO_THROW LLVMType* SLANG_MCALL
    getFunctionType(LLVMType* returnType, Slice<LLVMType*> paramTypes, bool variadic) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    declareFunction(LLVMType* funcType, CharSlice name, uint32_t attributes) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getFunctionArg(LLVMInst* funcDecl, int argIndex) override;
    SLANG_NO_THROW void SLANG_MCALL
    setArgInfo(LLVMInst* arg, CharSlice name, uint32_t attribute) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL declareGlobalVariable(
        LLVMInst* initializer,
        int64_t alignment,
        bool externallyVisible) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    declareGlobalVariable(int64_t size, int64_t alignment, bool externallyVisible) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL declareGlobalConstructor() override;

    SLANG_NO_THROW void SLANG_MCALL
    beginFunction(LLVMInst* func, LLVMDebugNode* debugFunc) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBlock(LLVMInst* func) override;
    SLANG_NO_THROW void SLANG_MCALL insertIntoBlock(LLVMInst* func) override;
    SLANG_NO_THROW void SLANG_MCALL endFunction(LLVMInst* func) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitAlloca(int64_t size, int64_t alignment) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitGetElementPtr(LLVMInst* ptr, int64_t stride, LLVMInst* index) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitStore(LLVMInst* value, LLVMInst* ptr, int64_t alignment, bool isVolatile) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitLoad(LLVMType* type, LLVMInst* ptr, int64_t alignment, bool isVolatile) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitIntResize(LLVMInst* value, LLVMType* into, bool isSigned) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitCopy(
        LLVMInst* dstPtr,
        int64_t dstAlign,
        LLVMInst* srcPtr,
        int64_t srcAlign,
        int64_t bytes,
        bool isVolatile) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitCall(LLVMInst* func, Slice<LLVMInst*> params) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitExtractElement(LLVMInst* vector, LLVMInst* index) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitInsertElement(LLVMInst* vector, LLVMInst* element, LLVMInst* index) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitVectorSplat(LLVMInst* element, int64_t count) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitVectorShuffle(LLVMInst* vector, Slice<int> mask) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBitfieldExtract(
        LLVMInst* value,
        LLVMInst* offset,
        LLVMInst* bits,
        LLVMType* resultType,
        bool isSigned) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBitfieldInsert(
        LLVMInst* value,
        LLVMInst* insert,
        LLVMInst* offset,
        LLVMInst* bits,
        LLVMType* resultType) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCompareOp(LLVMCompareOp op, LLVMInst* a, bool aIsSigned, LLVMInst* b, bool bIsSigned)
        override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitUnaryOp(LLVMUnaryOp op, LLVMInst* val) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBinaryOp(
        LLVMBinaryOp op,
        LLVMInst* a,
        LLVMInst* b,
        LLVMType* resultType,
        bool resultIsSigned) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitSelect(LLVMInst* cond, LLVMInst* trueValue, LLVMInst* falseValue) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitReturn(LLVMInst* returnValue) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBranch(LLVMInst* targetBlock) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCondBranch(LLVMInst* cond, LLVMInst* trueBlock, LLVMInst* falseBlock) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitSwitch(
        LLVMInst* cond,
        Slice<LLVMInst*> values,
        Slice<LLVMInst*> blocks,
        LLVMInst* defaultBlock) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitUnreachable() override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitCast(LLVMInst* src, LLVMType* dstType, bool srcIsSigned, bool dstIsSigned) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitBitCast(LLVMInst* src, LLVMType* dstType) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitPrintf(LLVMInst* format, Slice<LLVMInst*> args, Slice<bool> argIsSigned) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitGetBufferPtr(LLVMInst* buffer) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitGetBufferSize(LLVMInst* buffer) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitChangeBufferStride(LLVMInst* buffer, int64_t prevStride, int64_t newStride) override;

    // Some operations in Slang IR may have mixed scalar and vector parameters,
    // whereas LLVM IR requires only scalars or only vectors. This function
    // helps you promote each type as required.
    SLANG_NO_THROW void SLANG_MCALL
    operationPromote(LLVMInst** aVal, bool aIsSigned, LLVMInst** bVal, bool bIsSigned);

    SLANG_NO_THROW LLVMInst* SLANG_MCALL getPoison(LLVMType* type) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantInt(LLVMType* type, uint64_t value) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantPtr(uint64_t value) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantFloat(LLVMType* type, double value) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantArray(Slice<LLVMInst*> values) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantString(CharSlice literal) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantStruct(Slice<LLVMInst*> values) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantVector(Slice<LLVMInst*> values) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL getConstantVector(LLVMInst* value, int count) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    getConstantExtractElement(LLVMInst* value, int index) override;

    SLANG_NO_THROW void SLANG_MCALL setName(LLVMInst* inst, CharSlice) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugFallbackType(CharSlice name) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugVoidType() override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugIntType(const char* name, bool isSigned, int bitSize) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugFloatType(const char* name, int bitSize) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugPointerType(LLVMDebugNode* pointee) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugReferenceType(LLVMDebugNode* pointee) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugStringType() override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugVectorType(
        int64_t sizeBytes,
        int64_t alignBytes,
        int64_t elementCount,
        LLVMDebugNode* elementType) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugArrayType(
        int64_t sizeBytes,
        int64_t alignBytes,
        int64_t elementCount,
        LLVMDebugNode* elementType) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugStructField(
        LLVMDebugNode* type,
        CharSlice name,
        int64_t offset,
        int64_t size,
        int64_t alignment,
        LLVMDebugNode* file,
        int line) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugStructType(
        Slice<LLVMDebugNode*> fields,
        CharSlice name,
        int64_t size,
        int64_t alignment,
        LLVMDebugNode* file,
        int line) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugFunctionType(LLVMDebugNode* returnType, Slice<LLVMDebugNode*> paramTypes) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL getDebugFunction(
        LLVMDebugNode* funcType,
        CharSlice name,
        CharSlice linkageName,
        LLVMDebugNode* file,
        int line) override;
    SLANG_NO_THROW void SLANG_MCALL setDebugLocation(int line, int column) override;
    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    getDebugFile(CharSlice filename, CharSlice directory, CharSlice source) override;

    SLANG_NO_THROW LLVMDebugNode* SLANG_MCALL
    emitDebugVar(CharSlice name, LLVMDebugNode* type, LLVMDebugNode* file, int line, int argIndex)
        override;
    SLANG_NO_THROW void SLANG_MCALL
    emitDebugValue(LLVMDebugNode* debugVar, LLVMInst* value) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitInlineIRFunction(LLVMInst* func, CharSlice content) override;

    SLANG_NO_THROW LLVMInst* SLANG_MCALL emitComputeEntryPointWorkGroup(
        LLVMInst* entryPointFunc,
        CharSlice name,
        int xSize,
        int ySize,
        int zSize,
        int subgroupSize) override;
    SLANG_NO_THROW LLVMInst* SLANG_MCALL
    emitComputeEntryPointDispatcher(LLVMInst* workGroupFunc, CharSlice name) override;

    SLANG_NO_THROW SlangResult SLANG_MCALL generateAssembly(IArtifact** outArtifact) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL generateObjectCode(IArtifact** outArtifact) override;
    SLANG_NO_THROW SlangResult SLANG_MCALL generateJITLibrary(IArtifact** outArtifact) override;
};

LLVMBuilder::LLVMBuilder(LLVMBuilderOptions options, IArtifact** outErrorArtifact)
    : options(options)
{
    llvmContext.reset(new llvm::LLVMContext());
    llvmModule.reset(new llvm::Module("module", *llvmContext));
    llvmLinker.reset(new llvm::Linker(*llvmModule));
    llvmDebugBuilder.reset(new llvm::DIBuilder(*llvmModule));

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    if (options.optLevel == SLANG_OPTIMIZATION_LEVEL_NONE)
    {
        // The default IR builder has a built-in constant folder; for
        // absolutely zero optimization, we need to disable it like this.
        llvmBuilderNoOpt.reset(new llvm::IRBuilder<llvm::NoFolder>(*llvmContext));
        llvmBuilder = llvmBuilderNoOpt.get();
    }
    else
    {
        llvmBuilderOpt.reset(new llvm::IRBuilder<>(*llvmContext));
        llvmBuilder = llvmBuilderOpt.get();
    }

    // Functions that initialize global variables are "constructors" in
    // LLVM; they're called automatically at the start of the program and
    // need to be recorded with this type.
    llvmCtorType = llvm::StructType::get(
        llvmBuilder->getInt32Ty(),
        llvmBuilder->getPtrTy(0),
        llvmBuilder->getPtrTy(0));

    std::string targetTripleStr = llvm::sys::getDefaultTargetTriple();
    if (options.targetTriple.count != 0)
        targetTripleStr = std::string(options.targetTriple.begin(), options.targetTriple.count);

    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTripleStr, error);
    if (!target)
    {
        ArtifactDiagnostic diagnostic;
        diagnostic.severity = ArtifactDiagnostic::Severity::Error;
        diagnostic.stage = ArtifactDiagnostic::Stage::Link;
        diagnostic.text = TerminatedCharSlice(error.c_str(), error.length());
        *outErrorArtifact = createErrorArtifact(diagnostic);
        return;
    }

    llvm::TargetOptions opt;

    opt.NoTrappingFPMath = 1;

    opt.setFPDenormalMode(getLLVMDenormalMode(options.fp64DenormalMode));
    opt.setFP32DenormalMode(getLLVMDenormalMode(options.fp64DenormalMode));

    switch (options.fpMode)
    {
    default:
    case SLANG_FLOATING_POINT_MODE_DEFAULT:
        break;
    case SLANG_FLOATING_POINT_MODE_FAST:
        opt.UnsafeFPMath = 1;
        opt.NoSignedZerosFPMath = 1;
        opt.ApproxFuncFPMath = 1;
        break;
    case SLANG_FLOATING_POINT_MODE_PRECISE:
        opt.UnsafeFPMath = 0;
        opt.NoInfsFPMath = 0;
        opt.NoNaNsFPMath = 0;
        opt.NoSignedZerosFPMath = 0;
        opt.ApproxFuncFPMath = 0;
        break;
    }

    llvm::CodeGenOptLevel optLevel = llvm::CodeGenOptLevel::None;
    switch (options.optLevel)
    {
    case SLANG_OPTIMIZATION_LEVEL_NONE:
        optLevel = llvm::CodeGenOptLevel::None;
        break;
    default:
    case SLANG_OPTIMIZATION_LEVEL_DEFAULT:
        optLevel = llvm::CodeGenOptLevel::Less;
        break;
    case SLANG_OPTIMIZATION_LEVEL_HIGH:
        optLevel = llvm::CodeGenOptLevel::Default;
        break;
    case SLANG_OPTIMIZATION_LEVEL_MAXIMAL:
        optLevel = llvm::CodeGenOptLevel::Aggressive;
        break;
    }

    llvm::Triple targetTriple(targetTripleStr);

    targetMachine = target->createTargetMachine(
        targetTriple,
        charSliceToLLVM(options.cpu),
        charSliceToLLVM(options.features),
        opt,
        llvm::Reloc::PIC_,
        std::nullopt,
        optLevel);

    targetDataLayout = targetMachine->createDataLayout();
    llvmModule->setDataLayout(targetDataLayout);
    llvmModule->setTargetTriple(targetTriple);

    byteType = llvmBuilder->getInt8Ty();

    // ParseCommandLineOptions below assumes that the first parameter is the
    // name of the executable, but we do this cute trick to make it complain
    // about parameters forwarded to LLVM instead:
    std::vector<const char*> llvmArgs = {"-Xllvm"};
    for (TerminatedCharSlice arg : options.llvmArguments)
        llvmArgs.push_back(arg.data);

    // Parse LLVM options. These can be used to adjust the behavior of various
    // LLVM passes.
    llvm::raw_string_ostream errorStream(error);
    if (!llvm::cl::ParseCommandLineOptions(
            llvmArgs.size(),
            llvmArgs.data(),
            "slangc",
            &errorStream))
    {
        ArtifactDiagnostic diagnostic;
        diagnostic.severity = ArtifactDiagnostic::Severity::Error;
        diagnostic.stage = ArtifactDiagnostic::Stage::Link;
        diagnostic.text = TerminatedCharSlice(error.c_str(), error.length());
        *outErrorArtifact = createErrorArtifact(diagnostic);
        return;
    }

    if (options.debugLevel != SLANG_DEBUG_INFO_LEVEL_NONE)
    {
        llvmModule->addModuleFlag(
            llvm::Module::Warning,
            "Debug Info Version",
            llvm::DEBUG_METADATA_VERSION);

        compileUnit = llvmDebugBuilder->createCompileUnit(
            // TODO: We should probably apply for a language ID in DWARF? Not
            // sure how that process goes. Anyway, let's just use C++ as the
            // ID until this target stabilizes. C doesn't work because
            // debuggers won't use the un-mangled name in that language.
            llvm::dwarf::DW_LANG_C_plus_plus,

            // The "compile unit" model doesn't work super well for Slang -
            // we're supposed to only have one for the debug info per output
            // file, as if we were building each .slang-module into a separate
            // `.o`. Which we can't really do without losing large swathes of
            // language features.
            //
            // Anyway, for now, we'll give no name to the "compile unit" and
            // just operate with individual files separately. This doesn't
            // prevent the debugger from seeing our source in any way.
            llvmDebugBuilder->createFile("<slang-llvm>", "."),

            "Slang compiler",

            options.optLevel != SLANG_OPTIMIZATION_LEVEL_NONE,

            charSliceToLLVM(options.debugCommandLineArgs),

            0);
    }
}

LLVMBuilder::~LLVMBuilder()
{
    // Everything else is managed and automatically freed by llvmModule.
    delete targetMachine;
}

IArtifact* LLVMBuilder::createErrorArtifact(const ArtifactDiagnostic& diagnostic)
{
    ComPtr<IArtifactDiagnostics> diagnostics(new ArtifactDiagnostics);
    diagnostics->add(diagnostic);
    diagnostics->setResult(SLANG_FAIL);

    auto artifact =
        ArtifactUtil::createArtifact(ArtifactDesc::make(ArtifactKind::None, ArtifactPayload::None));
    ArtifactUtil::addAssociated(artifact, diagnostics);
    return artifact.detach();
}

void LLVMBuilder::optimize()
{
    llvm::LoopAnalysisManager loopAnalysisManager;
    llvm::FunctionAnalysisManager functionAnalysisManager;
    llvm::CGSCCAnalysisManager CGSCCAnalysisManager;
    llvm::ModuleAnalysisManager moduleAnalysisManager;

    llvm::PassBuilder passBuilder(targetMachine);
    passBuilder.registerModuleAnalyses(moduleAnalysisManager);
    passBuilder.registerCGSCCAnalyses(CGSCCAnalysisManager);
    passBuilder.registerFunctionAnalyses(functionAnalysisManager);
    passBuilder.registerLoopAnalyses(loopAnalysisManager);
    passBuilder.crossRegisterProxies(
        loopAnalysisManager,
        functionAnalysisManager,
        CGSCCAnalysisManager,
        moduleAnalysisManager);

    llvm::OptimizationLevel llvmLevel = llvm::OptimizationLevel::O0;

    switch (options.optLevel)
    {
    case SLANG_OPTIMIZATION_LEVEL_NONE:
        llvmLevel = llvm::OptimizationLevel::O0;
        break;
    default:
    case SLANG_OPTIMIZATION_LEVEL_DEFAULT:
        llvmLevel = llvm::OptimizationLevel::O1;
        break;
    case SLANG_OPTIMIZATION_LEVEL_HIGH:
        llvmLevel = llvm::OptimizationLevel::O2;
        break;
    case SLANG_OPTIMIZATION_LEVEL_MAXIMAL:
        llvmLevel = llvm::OptimizationLevel::O3;
        break;
    }

    // Run the actual optimizations.
    llvm::ModulePassManager modulePassManager =
        passBuilder.buildPerModuleDefaultPipeline(llvmLevel);
    modulePassManager.run(*llvmModule, moduleAnalysisManager);
}

void LLVMBuilder::finalize()
{
    // Dump the global constructors array
    if (globalCtors.getCount() != 0)
    {
        auto ctorArrayType = llvm::ArrayType::get(llvmCtorType, globalCtors.getCount());
        auto ctorArray = llvm::ConstantArray::get(
            ctorArrayType,
            llvm::ArrayRef(globalCtors.begin(), globalCtors.end()));
        new llvm::GlobalVariable(
            *llvmModule,
            ctorArrayType,
            false,
            llvm::GlobalValue::AppendingLinkage,
            ctorArray,
            "llvm.global_ctors");
    }

    // std::string out;
    // llvm::raw_string_ostream rso(out);
    // llvmModule->print(rso, nullptr);
    // printf("%s\n", out.c_str());

    llvm::verifyModule(*llvmModule, &llvm::errs());

    // O0 is separately handled inside `optimize()`; we need to call it in
    // any case to make sure that `ForceInline` functions get inlined.
    optimize();

    if (options.debugLevel != SLANG_DEBUG_INFO_LEVEL_NONE)
    {
        llvmDebugBuilder->finalize();
    }
}

void LLVMBuilder::emitGlobalLLVMIR(const std::string& textIR)
{
    llvm::SMDiagnostic diag;
    // This creates a temporary module with only the contents of llvmTextIR.
    std::unique_ptr<llvm::Module> sourceModule =
        llvm::parseAssemblyString(textIR, diag, *llvmContext);

    if (!sourceModule)
    { // Failed to parse the intrinsic
        std::string msgStr;
        llvm::raw_string_ostream diagOut(msgStr);
        diag.print("", diagOut, false);

        msgStr = "\n" + textIR + "\n" + msgStr;

        SLANG_ASSERT_FAILURE(msgStr.c_str());
    }

    sourceModule->setDataLayout(targetMachine->createDataLayout());
    sourceModule->setTargetTriple(targetMachine->getTargetTriple());

    // Finally, we merge the contents of the temporary module back to the
    // main llvmModule.
    llvmLinker->linkInModule(std::move(sourceModule), llvm::Linker::OverrideFromSrc);
}

llvm::Function* LLVMBuilder::getExternalBuiltin(ExternalFunc extFunc)
{
    if (externalFuncs.containsKey(extFunc))
        return externalFuncs.getValue(extFunc);

    llvm::Function* func = nullptr;
    auto emitDecl = [&](const char* name,
                        llvm::Type* result,
                        llvm::ArrayRef<llvm::Type*> params,
                        bool isVarArgs)
    {
        llvm::FunctionType* funcType = llvm::FunctionType::get(result, params, isVarArgs);
        return llvm::Function::Create(
            funcType,
            llvm::GlobalValue::ExternalLinkage,
            name,
            *llvmModule);
    };

    switch (extFunc)
    {
    case ExternalFunc::Printf:
        func = emitDecl("printf", llvmBuilder->getInt32Ty(), {llvmBuilder->getPtrTy()}, true);
        llvm::AttrBuilder attrs(*llvmContext);
        attrs.addCapturesAttr(llvm::CaptureInfo::none());
        attrs.addAttribute(llvm::Attribute::NoAlias);
        func->getArg(0)->addAttrs(attrs);
        break;
    }

    externalFuncs[extFunc] = func;
    return func;
}

void LLVMBuilder::makeVariadicArgsCCompatible(
    Slice<LLVMInst*> args,
    Slice<bool> argIsSigned,
    List<LLVMInst*>& outArgs)
{
    // Flatten the tuple resulting from the variadic pack.
    for (Int i = 0; i < args.count; ++i)
    {
        auto llvmValue = args[i];
        auto valueType = llvmValue->getType();

        if (valueType->isFloatingPointTy() && valueType->getScalarSizeInBits() < 64)
        {
            // Floats need to get up-casted to at least f64
            llvmValue = llvmBuilder->CreateCast(
                llvm::Instruction::CastOps::FPExt,
                llvmValue,
                llvmBuilder->getDoubleTy());
        }
        else if (valueType->isIntegerTy() && valueType->getScalarSizeInBits() < 32)
        {
            // Ints are upcasted to at least i32.
            llvmValue =
                emitCast(llvmValue, llvmBuilder->getInt32Ty(), argIsSigned[i], argIsSigned[i]);
        }
        outArgs.add(llvmValue);
    }
}

void* LLVMBuilder::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ILLVMBuilder::getTypeGuid())
    {
        return static_cast<ILLVMBuilder*>(this);
    }
    return nullptr;
}

void* LLVMBuilder::castAs(const Guid& guid)
{
    if (auto ptr = getInterface(guid))
        return ptr;
    return nullptr;
}

int LLVMBuilder::getPointerSizeInBits()
{
    return targetDataLayout.getPointerSizeInBits();
}

int LLVMBuilder::getStoreSizeOf(LLVMInst* value)
{
    return targetDataLayout.getTypeStoreSize(value->getType());
}

void LLVMBuilder::printType(String& outStr, LLVMType* type)
{
    std::string out;
    llvm::raw_string_ostream expanded(out);
    type->print(expanded);
    outStr.append(out.c_str());
}

void LLVMBuilder::printValue(String& outStr, LLVMInst* value, bool withType)
{
    std::string out;
    llvm::raw_string_ostream expanded(out);
    value->printAsOperand(expanded, withType);
    outStr.append(out.c_str());
}

LLVMType* LLVMBuilder::getVoidType()
{
    return llvmBuilder->getVoidTy();
}

LLVMType* LLVMBuilder::getIntType(int bitSize)
{
    return llvmBuilder->getIntNTy(bitSize);
}

LLVMType* LLVMBuilder::getFloatType(int bitSize)
{
    llvm::Type* type = nullptr;
    switch (bitSize)
    {
    case 16:
        type = llvmBuilder->getHalfTy();
        break;
    case 32:
        type = llvmBuilder->getFloatTy();
        break;
    case 64:
        type = llvmBuilder->getDoubleTy();
        break;
    default:
        break;
    }
    return type;
}

LLVMType* LLVMBuilder::getPointerType()
{
    return llvmBuilder->getPtrTy();
}

LLVMType* LLVMBuilder::getVectorType(int elementCount, LLVMType* elementType)
{
    auto type = llvm::VectorType::get(elementType, llvm::ElementCount::getFixed(elementCount));
    return type;
}

LLVMType* LLVMBuilder::getBufferType()
{
    return llvm::StructType::get(
        llvmBuilder->getPtrTy(0),
        llvmBuilder->getIntPtrTy(targetDataLayout));
}

LLVMType* LLVMBuilder::getFunctionType(
    LLVMType* returnType,
    Slice<LLVMType*> paramTypes,
    bool variadic)
{
    return llvm::FunctionType::get(returnType, sliceToArrayRef<llvm::Type>(paramTypes), variadic);
}

LLVMInst* LLVMBuilder::declareFunction(LLVMType* funcType, CharSlice name, uint32_t attributes)
{
    llvm::GlobalValue::LinkageTypes linkage = llvm::GlobalValue::PrivateLinkage;

    if (attributes & SLANG_LLVM_FUNC_ATTR_EXTERNALLYVISIBLE)
        linkage = llvm::GlobalValue::ExternalLinkage;

    auto func = llvm::Function::Create(
        llvm::cast<llvm::FunctionType>(funcType),
        linkage,
        charSliceToLLVM(name),
        *llvmModule);

    if (attributes & SLANG_LLVM_FUNC_ATTR_ALWAYSINLINE)
        func->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
    if (attributes & SLANG_LLVM_FUNC_ATTR_NOINLINE)
        func->addFnAttr(llvm::Attribute::AttrKind::NoInline);
    if (attributes & SLANG_LLVM_FUNC_ATTR_READNONE)
        func->setOnlyAccessesArgMemory();
    return func;
}

LLVMInst* LLVMBuilder::getFunctionArg(LLVMInst* funcDecl, int argIndex)
{
    return llvm::cast<llvm::Function>(funcDecl)->getArg(argIndex);
}

void LLVMBuilder::setArgInfo(LLVMInst* arg, CharSlice name, uint32_t attribute)
{
    auto llvmArg = llvm::cast<llvm::Argument>(arg);
    llvm::AttrBuilder attrs(*llvmContext);
    if (attribute & SLANG_LLVM_ATTR_NOALIAS)
        attrs.addAttribute(llvm::Attribute::NoAlias);
    if (attribute & SLANG_LLVM_ATTR_NOCAPTURE)
        attrs.addCapturesAttr(llvm::CaptureInfo::none());
    if (attribute & SLANG_LLVM_ATTR_READONLY)
        attrs.addAttribute(llvm::Attribute::ReadOnly);
    if (attribute & SLANG_LLVM_ATTR_WRITEONLY)
        attrs.addAttribute(llvm::Attribute::WriteOnly);
    llvmArg->addAttrs(attrs);

    if (name.count > 0)
        llvmArg->setName(charSliceToLLVM(name));
}

LLVMInst* LLVMBuilder::declareGlobalVariable(
    LLVMInst* initializer,
    int64_t alignment,
    bool externallyVisible)
{
    auto llvmVal = llvm::cast<llvm::Constant>(initializer);
    auto llvmVar =
        new llvm::GlobalVariable(llvmVal->getType(), false, llvm::GlobalValue::PrivateLinkage);
    llvmVar->setInitializer(llvmVal);
    llvmVar->setAlignment(llvm::Align(alignment));
    llvmModule->insertGlobalVariable(llvmVar);
    return llvmVar;
}

LLVMInst* LLVMBuilder::declareGlobalVariable(
    int64_t size,
    int64_t alignment,
    bool externallyVisible)
{
    auto llvmType = llvm::ArrayType::get(llvmBuilder->getInt8Ty(), size);
    auto llvmVar = new llvm::GlobalVariable(
        llvmType,
        false,
        externallyVisible ? llvm::GlobalValue::ExternalLinkage : llvm::GlobalValue::PrivateLinkage);
    llvmVar->setInitializer(llvm::PoisonValue::get(llvmType));
    llvmVar->setAlignment(llvm::Align(alignment));
    llvmModule->insertGlobalVariable(llvmVar);
    return llvmVar;
}

LLVMInst* LLVMBuilder::declareGlobalConstructor()
{
    llvm::FunctionType* ctorType = llvm::FunctionType::get(llvmBuilder->getVoidTy(), {}, false);

    std::string ctorName = "__slang_global_constructor";

    llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
    llvm::Function* llvmCtor =
        llvm::Function::Create(ctorType, llvm::GlobalValue::InternalLinkage, ctorName, *llvmModule);

    llvm::Constant* ctorData[3] = {
        llvmBuilder->getInt32(0),
        llvmCtor,
        llvm::ConstantPointerNull::get(llvm::PointerType::get(*llvmContext, 0))};

    llvm::Constant* ctorEntry = llvm::ConstantStruct::get(
        llvmCtorType,
        llvm::ArrayRef(ctorData, llvmCtorType->getNumElements()));

    globalCtors.add(ctorEntry);

    return llvmCtor;
}

void LLVMBuilder::beginFunction(LLVMInst* func, LLVMDebugNode* debugFunc)
{
    if (debugFunc != nullptr)
    {
        auto llvmFunc = llvm::cast<llvm::Function>(func);
        auto llvmDebugFunc = llvm::cast<llvm::DISubprogram>(debugFunc);
        llvmFunc->setSubprogram(llvmDebugFunc);
        currentLocalScope = llvmDebugFunc;
        llvmBuilder->SetCurrentDebugLocation(
            llvm::DILocation::get(*llvmContext, llvmDebugFunc->getLine(), 0, llvmDebugFunc));
    }
    else
    {
        currentLocalScope = nullptr;
        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
    }
}

LLVMInst* LLVMBuilder::emitBlock(LLVMInst* func)
{
    auto llvmFunc = llvm::cast<llvm::Function>(func);
    return llvm::BasicBlock::Create(*llvmContext, "", llvmFunc);
}

void LLVMBuilder::insertIntoBlock(LLVMInst* block)
{
    llvmBuilder->SetInsertPoint(llvm::cast<llvm::BasicBlock>(block));
}

void LLVMBuilder::endFunction(LLVMInst* func)
{
    currentLocalScope = nullptr;
}

LLVMInst* LLVMBuilder::emitAlloca(int64_t size, int64_t alignment)
{
    return llvmBuilder->Insert(new llvm::AllocaInst(
        byteType,
        0,
        size > int64_t(INT32_MAX) ? llvmBuilder->getInt64(size) : llvmBuilder->getInt32(size),
        llvm::Align(alignment)));
}

LLVMInst* LLVMBuilder::emitGetElementPtr(LLVMInst* ptr, int64_t stride, LLVMInst* index)
{
    SLANG_ASSERT(ptr->getType()->isPointerTy());
    return llvmBuilder->CreateGEP(
        stride == 1 ? byteType : llvm::ArrayType::get(byteType, stride),
        ptr,
        index);
}

LLVMInst* LLVMBuilder::emitStore(LLVMInst* value, LLVMInst* ptr, int64_t alignment, bool isVolatile)
{
    SLANG_ASSERT(ptr->getType()->isPointerTy());
    return llvmBuilder->CreateAlignedStore(value, ptr, llvm::MaybeAlign(alignment), isVolatile);
}

LLVMInst* LLVMBuilder::emitLoad(LLVMType* type, LLVMInst* ptr, int64_t alignment, bool isVolatile)
{
    SLANG_ASSERT(ptr->getType()->isPointerTy());
    return llvmBuilder->CreateAlignedLoad(type, ptr, llvm::MaybeAlign(alignment), isVolatile);
}

LLVMInst* LLVMBuilder::emitIntResize(LLVMInst* value, LLVMType* into, bool isSigned)
{
    auto llvmValue = value;
    auto llvmType = into;
    return isSigned ? llvmBuilder->CreateSExtOrTrunc(llvmValue, llvmType)
                    : llvmBuilder->CreateZExtOrTrunc(llvmValue, llvmType);
}

LLVMInst* LLVMBuilder::emitCopy(
    LLVMInst* dstPtr,
    int64_t dstAlign,
    LLVMInst* srcPtr,
    int64_t srcAlign,
    int64_t bytes,
    bool isVolatile)
{
    return llvmBuilder->CreateMemCpyInline(
        dstPtr,
        llvm::MaybeAlign(dstAlign),
        srcPtr,
        llvm::MaybeAlign(srcAlign),
        llvmBuilder->getInt32(bytes),
        isVolatile);
}

LLVMInst* LLVMBuilder::emitCall(LLVMInst* func, Slice<LLVMInst*> params)
{
    return llvmBuilder->CreateCall(
        llvm::cast<llvm::Function>(func),
        sliceToArrayRef<llvm::Value>(params));
}

LLVMInst* LLVMBuilder::emitExtractElement(LLVMInst* vector, LLVMInst* index)
{
    return llvmBuilder->CreateExtractElement(vector, index);
}

LLVMInst* LLVMBuilder::emitInsertElement(LLVMInst* vector, LLVMInst* element, LLVMInst* index)
{
    return llvmBuilder->CreateInsertElement(vector, element, index);
}

LLVMInst* LLVMBuilder::emitVectorSplat(LLVMInst* element, int64_t count)
{
    return llvmBuilder->CreateVectorSplat(count, element);
}

LLVMInst* LLVMBuilder::emitVectorShuffle(LLVMInst* vector, Slice<int> mask)
{
    return llvmBuilder->CreateShuffleVector(vector, llvm::ArrayRef<int>(mask.begin(), mask.end()));
}

LLVMInst* LLVMBuilder::emitBitfieldExtract(
    LLVMInst* value,
    LLVMInst* offset,
    LLVMInst* bits,
    LLVMType* resultType,
    bool isSigned)
{
    value = emitCast(value, resultType, false, false);
    offset = emitCast(offset, resultType, false, false);
    bits = emitCast(bits, resultType, false, false);

    auto shiftedVal =
        llvmBuilder->CreateLShr(value, emitCast(offset, value->getType(), false, false));

    auto numBits = emitCast(
        llvmBuilder->getInt32(resultType->getScalarSizeInBits()),
        value->getType(),
        false,
        false);
    auto highBits = llvmBuilder->CreateSub(numBits, bits);
    shiftedVal = llvmBuilder->CreateShl(shiftedVal, highBits);
    if (isSigned)
    {
        return llvmBuilder->CreateAShr(shiftedVal, highBits);
    }
    else
    {
        return llvmBuilder->CreateLShr(shiftedVal, highBits);
    }
}

LLVMInst* LLVMBuilder::emitBitfieldInsert(
    LLVMInst* value,
    LLVMInst* insert,
    LLVMInst* offset,
    LLVMInst* bits,
    LLVMType* resultType)
{
    value = emitCast(value, resultType, false, false);
    insert = emitCast(insert, resultType, false, false);
    offset = emitCast(offset, resultType, false, false);
    bits = emitCast(bits, resultType, false, false);

    auto one = emitCast(llvmBuilder->getInt32(1), value->getType(), false, false);
    auto mask = llvmBuilder->CreateShl(one, bits);
    mask = llvmBuilder->CreateSub(mask, one);
    mask = llvmBuilder->CreateShl(mask, offset);

    insert = llvmBuilder->CreateShl(insert, offset);
    insert = llvmBuilder->CreateAnd(insert, mask);

    auto notMask = llvmBuilder->CreateNot(mask);

    value = llvmBuilder->CreateAnd(value, notMask);
    return llvmBuilder->CreateOr(value, insert);
}

LLVMInst* LLVMBuilder::emitCompareOp(
    LLVMCompareOp op,
    LLVMInst* a,
    bool aIsSigned,
    LLVMInst* b,
    bool bIsSigned)
{
    operationPromote(&a, aIsSigned, &b, bIsSigned);

    bool isFloat = a->getType()->getScalarType()->isFloatingPointTy();
    bool isSigned = aIsSigned || bIsSigned;

    llvm::CmpInst::Predicate pred;
    switch (op)
    {
    case Slang::LLVMCompareOp::Less:
        pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OLT
               : isSigned ? llvm::CmpInst::Predicate::ICMP_SLT
                          : llvm::CmpInst::Predicate::ICMP_ULT;
        break;
    case Slang::LLVMCompareOp::LessEqual:
        pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OLE
               : isSigned ? llvm::CmpInst::Predicate::ICMP_SLE
                          : llvm::CmpInst::Predicate::ICMP_ULE;
        break;
    case Slang::LLVMCompareOp::Equal:
        pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OEQ : llvm::CmpInst::Predicate::ICMP_EQ;
        break;
    case Slang::LLVMCompareOp::NotEqual:
        pred = isFloat ? llvm::CmpInst::Predicate::FCMP_ONE : llvm::CmpInst::Predicate::ICMP_NE;
        break;
    case Slang::LLVMCompareOp::Greater:
        pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OGT
               : isSigned ? llvm::CmpInst::Predicate::ICMP_SGT
                          : llvm::CmpInst::Predicate::ICMP_UGT;
        break;
    case Slang::LLVMCompareOp::GreaterEqual:
        pred = isFloat    ? llvm::CmpInst::Predicate::FCMP_OGE
               : isSigned ? llvm::CmpInst::Predicate::ICMP_SGE
                          : llvm::CmpInst::Predicate::ICMP_UGE;
        break;
    default:
        SLANG_UNEXPECTED("Unsupported compare op");
        break;
    }

    return llvmBuilder->CreateCmp(pred, a, b);
}

LLVMInst* LLVMBuilder::emitUnaryOp(LLVMUnaryOp op, LLVMInst* val)
{
    bool isFloat = val->getType()->getScalarType()->isFloatingPointTy();
    switch (op)
    {
    case Slang::LLVMUnaryOp::Negate:
        return isFloat ? llvmBuilder->CreateFNeg(val) : llvmBuilder->CreateNeg(val);
    case Slang::LLVMUnaryOp::Not:
        return llvmBuilder->CreateNot(val);
    default:
        SLANG_UNEXPECTED("Unsupported unary arithmetic op");
        return nullptr;
    }
}

LLVMInst* LLVMBuilder::emitBinaryOp(
    LLVMBinaryOp op,
    LLVMInst* a,
    LLVMInst* b,
    LLVMType* resultType,
    bool resultIsSigned)
{
    bool isFloat = resultType ? resultType->getScalarType()->isFloatingPointTy()
                              : a->getType()->getScalarType()->isFloatingPointTy();
    llvm::Instruction::BinaryOps llvmOp;
    switch (op)
    {
    case Slang::LLVMBinaryOp::Add:
        llvmOp = isFloat ? llvm::Instruction::FAdd : llvm::Instruction::Add;
        break;
    case Slang::LLVMBinaryOp::Sub:
        llvmOp = isFloat ? llvm::Instruction::FSub : llvm::Instruction::Sub;
        break;
    case Slang::LLVMBinaryOp::Mul:
        llvmOp = isFloat ? llvm::Instruction::FMul : llvm::Instruction::Mul;
        break;
    case Slang::LLVMBinaryOp::Div:
        llvmOp = isFloat          ? llvm::Instruction::FDiv
                 : resultIsSigned ? llvm::Instruction::SDiv
                                  : llvm::Instruction::UDiv;
        break;
    case Slang::LLVMBinaryOp::Rem:
        llvmOp = isFloat          ? llvm::Instruction::FRem
                 : resultIsSigned ? llvm::Instruction::SRem
                                  : llvm::Instruction::URem;
        break;
    case Slang::LLVMBinaryOp::And:
        llvmOp = llvm::Instruction::And;
        break;
    case Slang::LLVMBinaryOp::Or:
        llvmOp = llvm::Instruction::Or;
        break;
    case Slang::LLVMBinaryOp::Xor:
        llvmOp = llvm::Instruction::Xor;
        break;
    case Slang::LLVMBinaryOp::RightShift:
        llvmOp = resultIsSigned ? llvm::Instruction::AShr : llvm::Instruction::LShr;
        break;
    case Slang::LLVMBinaryOp::LeftShift:
        llvmOp = llvm::Instruction::Shl;
        break;
    default:
        SLANG_UNEXPECTED("Unsupported binary arithmetic op");
        break;
    }

    // Some ops in Slang, e.g. Lsh, may have differing types for the
    // operands. This is not allowed by LLVM. Both sides must match
    // the result type. Hence, we coerce as needed.
    if (resultType)
    {
        a = emitCast(a, resultType, resultIsSigned, resultIsSigned);
        b = emitCast(b, resultType, resultIsSigned, resultIsSigned);
    }
    else
    {
        operationPromote(&a, resultIsSigned, &b, resultIsSigned);
    }

    return llvmBuilder->CreateBinOp(llvmOp, a, b);
}

LLVMInst* LLVMBuilder::emitSelect(LLVMInst* cond, LLVMInst* trueValue, LLVMInst* falseValue)
{
    return llvmBuilder->CreateSelect(cond, trueValue, falseValue);
}

LLVMInst* LLVMBuilder::emitReturn(LLVMInst* returnValue)
{
    if (!returnValue)
        return llvmBuilder->CreateRetVoid();
    else
        return llvmBuilder->CreateRet(returnValue);
}

LLVMInst* LLVMBuilder::emitBranch(LLVMInst* targetBlock)
{
    return llvmBuilder->CreateBr(llvm::cast<llvm::BasicBlock>(targetBlock));
}

LLVMInst* LLVMBuilder::emitCondBranch(LLVMInst* cond, LLVMInst* trueBlock, LLVMInst* falseBlock)
{
    return llvmBuilder->CreateCondBr(
        cond,
        llvm::cast<llvm::BasicBlock>(trueBlock),
        llvm::cast<llvm::BasicBlock>(falseBlock));
}

LLVMInst* LLVMBuilder::emitSwitch(
    LLVMInst* cond,
    Slice<LLVMInst*> values,
    Slice<LLVMInst*> blocks,
    LLVMInst* defaultBlock)
{
    auto llvmSwitch =
        llvmBuilder->CreateSwitch(cond, llvm::cast<llvm::BasicBlock>(defaultBlock), values.count);
    for (Int c = 0; c < values.count; c++)
    {
        auto llvmCaseBlock = llvm::cast<llvm::BasicBlock>(blocks[c]);
        auto llvmCaseValue = llvm::cast<llvm::ConstantInt>(values[c]);

        llvmSwitch->addCase(llvmCaseValue, llvmCaseBlock);
    }
    return llvmSwitch;
}

LLVMInst* LLVMBuilder::emitUnreachable()
{
    return llvmBuilder->CreateUnreachable();
}

LLVMInst* LLVMBuilder::emitCast(
    LLVMInst* src,
    LLVMType* dstType,
    bool srcIsSigned,
    bool dstIsSigned)
{
    auto valType = src->getType();
    auto valWidth = valType->getPrimitiveSizeInBits();
    auto targetWidth = dstType->getPrimitiveSizeInBits();

    auto dst = src;

    bool srcIsFloat = valType->getScalarType()->isFloatingPointTy();
    bool srcIsInt = valType->getScalarType()->isIntegerTy();
    bool srcIsPointer = valType->getScalarType()->isPointerTy();
    bool srcIsBool = srcIsInt && valWidth == 1;
    bool dstIsFloat = dstType->getScalarType()->isFloatingPointTy();
    bool dstIsInt = dstType->getScalarType()->isIntegerTy();
    bool dstIsPointer = dstType->getScalarType()->isPointerTy();
    bool dstIsBool = dstIsInt && targetWidth == 1;

    if (dstType->isVectorTy() && !valType->isVectorTy())
    {
        auto vecType = llvm::cast<llvm::VectorType>(dstType);
        // Splat scalar into vector
        dst = llvmBuilder->CreateVectorSplat(
            vecType->getElementCount(),
            emitCast(src, vecType->getElementType(), srcIsSigned, dstIsSigned));
    }
    else if (dstIsBool)
    {
        if (srcIsPointer)
        {
            dst = llvmBuilder->CreateIsNotNull(src);
        }
        else if (!srcIsBool)
        {
            llvm::Constant* zero = llvm::Constant::getNullValue(valType);
            dst = llvmBuilder->CreateCmp(
                srcIsFloat ? llvm::CmpInst::Predicate::FCMP_UNE : llvm::CmpInst::Predicate::ICMP_NE,
                src,
                zero);
        }
    }
    else if (dstIsFloat)
    {
        if (srcIsInt)
        {
            dst = srcIsSigned ? llvmBuilder->CreateSIToFP(src, dstType)
                              : llvmBuilder->CreateUIToFP(src, dstType);
        }
        else if (valWidth < targetWidth)
        {
            dst = llvmBuilder->CreateFPExt(src, dstType);
        }
        else if (valWidth > targetWidth)
        {
            dst = llvmBuilder->CreateFPTrunc(src, dstType);
        }
    }
    else if (dstIsInt)
    {
        if (srcIsFloat)
        {
            dst = dstIsSigned ? llvmBuilder->CreateFPToSI(src, dstType)
                              : llvmBuilder->CreateFPToUI(src, dstType);
        }
        else if (srcIsPointer)
        {
            dst = llvmBuilder->CreatePtrToInt(src, dstType);
        }
        else if (valWidth != targetWidth)
        {
            dst = srcIsSigned ? llvmBuilder->CreateSExtOrTrunc(src, dstType)
                              : llvmBuilder->CreateZExtOrTrunc(src, dstType);
        }
    }
    else if (dstIsPointer)
    {
        if (!srcIsPointer)
            dst = llvmBuilder->CreateIntToPtr(src, dstType);
    }
    return dst;
}

LLVMInst* LLVMBuilder::emitBitCast(LLVMInst* src, LLVMType* dstType)
{
    auto llvmFromType = src->getType();
    auto op = llvm::Instruction::CastOps::BitCast;
    // It appears that sometimes casts between ints and ptrs occur
    // as bitcasts. Fix the operation in that case.
    if (llvmFromType->isPointerTy() && dstType->isIntegerTy())
    {
        op = llvm::Instruction::CastOps::PtrToInt;
    }
    else if (llvmFromType->isIntegerTy() && dstType->isPointerTy())
    {
        op = llvm::Instruction::CastOps::IntToPtr;
    }
    else if (llvmFromType->isPointerTy() && !dstType->isPointerTy())
    {
        // Cast from pointer to ???, so first cast to int and then
        // perform the bitcast.
        src = llvmBuilder->CreateCast(
            llvm::Instruction::CastOps::PtrToInt,
            src,
            llvmBuilder->getIntPtrTy(targetDataLayout));
    }
    else if (!llvmFromType->isPointerTy() && dstType->isPointerTy())
    {
        // Cast from ??? to pointer, so first bitcast to equally
        // sized int type and then do IntToPtr cast.
        src = llvmBuilder->CreateCast(
            llvm::Instruction::CastOps::BitCast,
            src,
            llvmBuilder->getIntPtrTy(targetDataLayout));
        op = llvm::Instruction::CastOps::IntToPtr;
    }
    return llvmBuilder->CreateCast(op, src, dstType);
}

LLVMInst* LLVMBuilder::emitPrintf(LLVMInst* format, Slice<LLVMInst*> args, Slice<bool> argIsSigned)
{
    auto llvmFunc = getExternalBuiltin(ExternalFunc::Printf);

    List<llvm::Value*> legalizedArgs;
    legalizedArgs.add(format);
    LLVMBuilder::makeVariadicArgsCCompatible(args, argIsSigned, legalizedArgs);

    return llvmBuilder->CreateCall(
        llvmFunc,
        llvm::ArrayRef(legalizedArgs.begin(), legalizedArgs.end()));
}

LLVMInst* LLVMBuilder::emitGetBufferPtr(LLVMInst* buffer)
{
    return llvmBuilder->CreateExtractValue(buffer, 0);
}

LLVMInst* LLVMBuilder::emitGetBufferSize(LLVMInst* buffer)
{
    return llvmBuilder->CreateExtractValue(buffer, 1);
}

LLVMInst* LLVMBuilder::emitChangeBufferStride(
    LLVMInst* buffer,
    int64_t prevStride,
    int64_t newStride)
{
    auto size = llvmBuilder->CreateExtractValue(buffer, 1);
    if (prevStride != 1)
    {
        size = llvmBuilder->CreateMul(
            size,
            llvm::ConstantInt::get(llvmBuilder->getIntPtrTy(targetDataLayout), prevStride));
    }
    if (newStride != 1)
    {
        size = llvmBuilder->CreateUDiv(
            size,
            llvm::ConstantInt::get(llvmBuilder->getIntPtrTy(targetDataLayout), newStride));
    }
    return llvmBuilder->CreateInsertValue(buffer, size, 1);
}

void LLVMBuilder::operationPromote(
    LLVMInst** aValInOut,
    bool aIsSigned,
    LLVMInst** bValInOut,
    bool bIsSigned)
{
    auto aVal = *aValInOut;
    auto bVal = *bValInOut;
    llvm::Type* aType = aVal->getType();
    llvm::Type* bType = bVal->getType();
    if (aType == bType)
        return;

    llvm::Type* aElementType = aType->getScalarType();
    llvm::Type* bElementType = bType->getScalarType();
    int aElementCount = 1;
    int bElementCount = 1;
    if (aType->isVectorTy())
    {
        auto vecType = llvm::cast<llvm::VectorType>(aType);
        aElementCount = vecType->getElementCount().getFixedValue();
    }
    if (bType->isVectorTy())
    {
        auto vecType = llvm::cast<llvm::VectorType>(bType);
        bElementCount = vecType->getElementCount().getFixedValue();
    }

    int aElementWidth = aElementType->getScalarSizeInBits();
    int bElementWidth = bElementType->getScalarSizeInBits();
    bool aFloat = aElementType->isFloatingPointTy();
    bool bFloat = bElementType->isFloatingPointTy();
    int maxElementCount = aElementCount > bElementCount ? aElementCount : bElementCount;

    llvm::Type* promotedType = aType;
    bool promotedSign = aIsSigned || bIsSigned;

    if (!aFloat && bFloat)
        promotedType = aElementType;
    else if (aFloat && !bFloat)
        promotedType = bElementType;
    else
        promotedType = aElementWidth > bElementWidth ? aElementType : bElementType;

    if (maxElementCount != 1)
        promotedType =
            llvm::VectorType::get(promotedType, llvm::ElementCount::getFixed(maxElementCount));

    *aValInOut = emitCast(*aValInOut, promotedType, aIsSigned, promotedSign);
    *bValInOut = emitCast(*bValInOut, promotedType, bIsSigned, promotedSign);
}

LLVMInst* LLVMBuilder::getPoison(LLVMType* type)
{
    return llvm::PoisonValue::get(type);
}

LLVMInst* LLVMBuilder::getConstantInt(LLVMType* type, uint64_t value)
{
    return llvm::ConstantInt::get(type, value);
}

LLVMInst* LLVMBuilder::getConstantPtr(uint64_t value)
{
    auto llvmType = llvmBuilder->getPtrTy();
    if (value == 0)
    {
        return llvm::ConstantPointerNull::get(llvmType);
    }
    else
    {
        return llvm::ConstantExpr::getIntToPtr(
            llvm::ConstantInt::get(llvmBuilder->getIntPtrTy(targetDataLayout), value),
            llvmType);
    }
}

LLVMInst* LLVMBuilder::getConstantFloat(LLVMType* type, double value)
{
    return llvm::ConstantFP::get(type, value);
}

LLVMInst* LLVMBuilder::getConstantArray(Slice<LLVMInst*> values)
{
    llvm::ArrayType* type = nullptr;
    if (values.count != 0)
    {
        type = llvm::ArrayType::get(values[0]->getType(), values.count);
    }
    else
    {
        type = llvm::ArrayType::get(byteType, 0);
    }

    return llvm::ConstantArray::get(type, sliceToArrayRef<llvm::Constant>(values));
}

LLVMInst* LLVMBuilder::getConstantString(CharSlice literal)
{
    return llvmBuilder->CreateGlobalString(charSliceToLLVM(literal));
}

LLVMInst* LLVMBuilder::getConstantStruct(Slice<LLVMInst*> values)
{
    return llvm::ConstantStruct::getAnon(sliceToArrayRef<llvm::Constant>(values), true);
}

LLVMInst* LLVMBuilder::getConstantVector(Slice<LLVMInst*> values)
{
    return llvm::ConstantVector::get(sliceToArrayRef<llvm::Constant>(values));
}

LLVMInst* LLVMBuilder::getConstantVector(LLVMInst* value, int count)
{
    return llvm::ConstantVector::getSplat(
        llvm::ElementCount::getFixed(count),
        llvm::cast<llvm::Constant>(value));
}

LLVMInst* LLVMBuilder::getConstantExtractElement(LLVMInst* value, int index)
{
    auto llvmIndex = llvm::ConstantInt::get(llvmBuilder->getInt32Ty(), index);
    return llvm::ConstantExpr::getExtractElement(llvm::cast<llvm::Constant>(value), llvmIndex);
}

void LLVMBuilder::setName(LLVMInst* inst, CharSlice name)
{
    inst->setName(charSliceToLLVM(name));
}

LLVMDebugNode* LLVMBuilder::getDebugFallbackType(CharSlice name)
{
    return llvmDebugBuilder->createUnspecifiedType(charSliceToLLVM(name));
}

LLVMDebugNode* LLVMBuilder::getDebugVoidType()
{
    return llvmDebugBuilder->createUnspecifiedType("void");
}

LLVMDebugNode* LLVMBuilder::getDebugIntType(const char* name, bool isSigned, int bitSize)
{
    unsigned encoding = isSigned ? llvm::dwarf::DW_ATE_signed : llvm::dwarf::DW_ATE_unsigned;
    if (bitSize == 1)
        encoding = llvm::dwarf::DW_ATE_boolean;
    return llvmDebugBuilder->createBasicType(name, bitSize, encoding);
}

LLVMDebugNode* LLVMBuilder::getDebugFloatType(const char* name, int bitSize)
{
    return llvmDebugBuilder->createBasicType(name, bitSize, llvm::dwarf::DW_ATE_float);
}

LLVMDebugNode* LLVMBuilder::getDebugPointerType(LLVMDebugNode* pointee)
{
    llvm::DIType* llvmPointee = llvm::cast<llvm::DIType>(pointee ? pointee : getDebugVoidType());
    return llvmDebugBuilder->createPointerType(
        llvmPointee,
        targetDataLayout.getPointerSizeInBits());
}

LLVMDebugNode* LLVMBuilder::getDebugReferenceType(LLVMDebugNode* pointee)
{
    return llvmDebugBuilder->createReferenceType(
        llvm::dwarf::DW_TAG_reference_type,
        llvm::cast<llvm::DIType>(pointee),
        targetDataLayout.getPointerSizeInBits());
}

LLVMDebugNode* LLVMBuilder::getDebugStringType()
{
    return llvmDebugBuilder->createPointerType(
        llvmDebugBuilder->createBasicType("char", 8, llvm::dwarf::DW_ATE_signed_char),
        targetDataLayout.getPointerSizeInBits());
}

LLVMDebugNode* LLVMBuilder::getDebugVectorType(
    int64_t sizeBytes,
    int64_t alignBytes,
    int64_t elementCount,
    LLVMDebugNode* elementType)
{
    if (sizeBytes < alignBytes)
        sizeBytes = alignBytes;

    llvm::Metadata* subscript = llvmDebugBuilder->getOrCreateSubrange(0, elementCount);
    llvm::DINodeArray subscriptArray = llvmDebugBuilder->getOrCreateArray(subscript);
    return llvmDebugBuilder->createVectorType(
        sizeBytes * 8,
        alignBytes * 8,
        llvm::cast<llvm::DIType>(elementType),
        subscriptArray);
}

LLVMDebugNode* LLVMBuilder::getDebugArrayType(
    int64_t sizeBytes,
    int64_t alignBytes,
    int64_t elementCount,
    LLVMDebugNode* elementType)
{
    llvm::Metadata* subscript = llvmDebugBuilder->getOrCreateSubrange(0, elementCount);
    llvm::DINodeArray subscriptArray = llvmDebugBuilder->getOrCreateArray(subscript);
    return llvmDebugBuilder->createArrayType(
        sizeBytes * 8,
        alignBytes * 8,
        llvm::cast<llvm::DIType>(elementType),
        subscriptArray);
}

LLVMDebugNode* LLVMBuilder::getDebugStructField(
    LLVMDebugNode* type,
    CharSlice name,
    int64_t offset,
    int64_t size,
    int64_t alignment,
    LLVMDebugNode* file,
    int line)
{
    if (!file)
        file = compileUnit->getFile();
    llvm::DIFile* llvmFile = llvm::cast<llvm::DIFile>(file);
    return llvmDebugBuilder->createMemberType(
        llvmFile,
        charSliceToLLVM(name),
        llvmFile,
        line,
        size * 8,
        alignment * 8,
        offset * 8,
        llvm::DINode::FlagZero,
        llvm::cast<llvm::DIType>(type));
}

LLVMDebugNode* LLVMBuilder::getDebugStructType(
    Slice<LLVMDebugNode*> fields,
    CharSlice name,
    int64_t size,
    int64_t alignment,
    LLVMDebugNode* file,
    int line)
{
    if (!file)
        file = compileUnit->getFile();
    llvm::DINodeArray fieldTypes =
        llvmDebugBuilder->getOrCreateArray(sliceToArrayRef<llvm::Metadata>(fields));
    llvm::DIFile* llvmFile = llvm::cast<llvm::DIFile>(file);

    return llvmDebugBuilder->createStructType(
        llvmFile,
        charSliceToLLVM(name),
        llvmFile,
        line,
        size * 8,
        alignment * 8,
        llvm::DINode::FlagZero,
        nullptr,
        fieldTypes);
}

LLVMDebugNode* LLVMBuilder::getDebugFunctionType(
    LLVMDebugNode* returnType,
    Slice<LLVMDebugNode*> paramTypes)
{
    List<llvm::Metadata*> elements;
    elements.add(returnType);
    for (LLVMDebugNode* param : paramTypes)
        elements.add(param);

    return llvmDebugBuilder->createSubroutineType(llvmDebugBuilder->getOrCreateTypeArray(
        llvm::ArrayRef<llvm::Metadata*>(elements.begin(), elements.end())));
}

LLVMDebugNode* LLVMBuilder::getDebugFunction(
    LLVMDebugNode* funcType,
    CharSlice name,
    CharSlice linkageName,
    LLVMDebugNode* file,
    int line)
{
    if (!file)
        file = compileUnit->getFile();

    return llvmDebugBuilder->createFunction(
        llvm::cast<llvm::DIScope>(file),
        charSliceToLLVM(name),
        charSliceToLLVM(linkageName),
        llvm::cast<llvm::DIFile>(file),
        line,
        llvm::cast<llvm::DISubroutineType>(funcType),
        line,
        llvm::DINode::FlagPrototyped,
        llvm::DISubprogram::SPFlagDefinition);
}

void LLVMBuilder::setDebugLocation(int line, int column)
{
    if (!currentLocalScope)
        return;
    llvmBuilder->SetCurrentDebugLocation(
        llvm::DILocation::get(*llvmContext, line, column, currentLocalScope));
}

LLVMDebugNode* LLVMBuilder::getDebugFile(CharSlice filename, CharSlice directory, CharSlice source)
{
    return llvmDebugBuilder->createFile(
        charSliceToLLVM(filename),
        charSliceToLLVM(directory),
        std::nullopt,
        charSliceToLLVM(source));
}

LLVMDebugNode* LLVMBuilder::emitDebugVar(
    CharSlice name,
    LLVMDebugNode* type,
    LLVMDebugNode* file,
    int line,
    int argIndex)
{
    if (!currentLocalScope)
        return nullptr;

    llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
    if (!file)
        file = loc->getFile();
    if (line < 0)
        line = loc->getLine();

    llvm::DILocalVariable* var;
    if (argIndex >= 0)
    {
        var = llvmDebugBuilder->createParameterVariable(
            currentLocalScope,
            charSliceToLLVM(name),
            argIndex + 1,
            llvm::cast<llvm::DIFile>(file),
            line,
            llvm::cast<llvm::DIType>(type),
            options.optLevel == SLANG_OPTIMIZATION_LEVEL_NONE);
    }
    else
    {
        var = llvmDebugBuilder->createAutoVariable(
            currentLocalScope,
            charSliceToLLVM(name),
            llvm::cast<llvm::DIFile>(file),
            line,
            llvm::cast<llvm::DIType>(type),
            options.optLevel == SLANG_OPTIMIZATION_LEVEL_NONE);
    }
    variableDebugInfoMap[var] = {false, false};
    return var;
}

void LLVMBuilder::emitDebugValue(LLVMDebugNode* debugVar, LLVMInst* value)
{
    llvm::DILocalVariable* var = llvm::cast<llvm::DILocalVariable>(debugVar);
    if (!currentLocalScope || !variableDebugInfoMap.containsKey(var))
        return;

    VariableDebugInfo& debugInfo = variableDebugInfoMap.getValue(var);

    llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
    if (!loc)
        loc = llvm::DILocation::get(*llvmContext, var->getLine(), 0, var->getScope());

    llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(value);
    if (!debugInfo.attached && alloca)
    {
        debugInfo.isStackVar = true;
        llvmDebugBuilder->insertDeclare(
            alloca,
            var,
            llvmDebugBuilder->createExpression(),
            loc,
            llvmBuilder->GetInsertBlock());
    }
    else if (!debugInfo.isStackVar)
    {
        llvmDebugBuilder->insertDbgValueIntrinsic(
            value,
            llvm::cast<llvm::DILocalVariable>(debugVar),
            llvmDebugBuilder->createExpression(),
            loc,
            llvmBuilder->GetInsertBlock());
    }
    debugInfo.attached = true;
}

LLVMInst* LLVMBuilder::emitInlineIRFunction(LLVMInst* func, CharSlice content)
{
    auto llvmFunc = llvm::cast<llvm::Function>(func);
    llvmFunc->setLinkage(llvm::Function::LinkageTypes::ExternalLinkage);

    std::string funcName = llvmFunc->getName().str();

    std::string functionIR;
    llvm::raw_string_ostream expanded(functionIR);

    // This is a bit of a hack. We add a block to the function so that
    // it gets printed as a definition instead of a declaration, and then
    // fill in the actual contents later in this function.
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*llvmContext, "", llvmFunc);
    llvmFunc->print(expanded);
    bb->eraseFromParent();

    while (functionIR.size() > 0 && functionIR.back() != '{')
        functionIR.pop_back();

    expanded << "\nentry:\n" << std::string(content.data, content.count) << "\n}\n";

    emitGlobalLLVMIR(functionIR);

    llvmFunc = llvm::cast<llvm::Function>(llvmModule->getNamedValue(funcName));
    llvmFunc->setLinkage(llvm::Function::LinkageTypes::PrivateLinkage);
    // Intrinsic functions usually do nothing other than call a single
    // instruction; we can just inline them by default.
    llvmFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
    return llvmFunc;
}

LLVMInst* LLVMBuilder::emitComputeEntryPointWorkGroup(
    LLVMInst* entryPointFunc,
    CharSlice name,
    int xSize,
    int ySize,
    int zSize,
    int subgroupSize)
{
    // TODO: Currently unused, but will be needed for warp operations some day
    // in the future, so it's already in the API.
    (void)subgroupSize;

    llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());

    llvm::Type* uintType = llvmBuilder->getInt32Ty();

    llvm::Type* argTypes[3] = {
        llvmBuilder->getPtrTy(0),
        llvmBuilder->getPtrTy(0),
        llvmBuilder->getPtrTy(0)};
    llvm::FunctionType* dispatchFuncType =
        llvm::FunctionType::get(llvmBuilder->getVoidTy(), argTypes, false);

    std::string groupName(name.begin(), name.count);
    groupName.append("_Group");

    llvm::Function* dispatcher = llvm::Function::Create(
        dispatchFuncType,
        llvm::GlobalValue::ExternalLinkage,
        groupName,
        *llvmModule);

    for (auto& arg : dispatcher->args())
        arg.addAttr(llvm::Attribute::NoAlias);

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "entry", dispatcher);
    llvmBuilder->SetInsertPoint(entryBlock);

    // This is the data passed to the actual entry point.
    llvm::StructType* threadVaryingInputType = llvm::StructType::get(
        // groupID
        llvm::VectorType::get(uintType, llvm::ElementCount::getFixed(3)),
        // groupThreadID
        llvm::VectorType::get(uintType, llvm::ElementCount::getFixed(3)));

    // TODO: DeSPMD? That will also require scalarization of vector
    // operations, so it should be done after everything has been emitted.

    llvm::Type* groupIDType = llvm::ArrayType::get(uintType, 3);
    llvm::Value* groupID = dispatcher->getArg(0);
    llvm::Value* threadInput = llvmBuilder->CreateAlloca(threadVaryingInputType);

    llvm::Value* threadID[3];

    llvm::Value* workGroupSize[3] = {
        llvmBuilder->getInt32(xSize),
        llvmBuilder->getInt32(ySize),
        llvmBuilder->getInt32(zSize)};

    for (int i = 0; i < 3; ++i)
    {
        llvm::Value* inIndices[2] = {llvmBuilder->getInt32(0), llvmBuilder->getInt32(i)};

        auto inGroupPtr = llvmBuilder->CreateGEP(groupIDType, groupID, inIndices);
        auto inGroupID = llvmBuilder->CreateLoad(
            uintType,
            inGroupPtr,
            llvm::Twine("ingroupID").concat(llvm::Twine(i)));

        llvm::Value* outIndices[3] = {
            llvmBuilder->getInt32(0),
            llvmBuilder->getInt32(0),
            llvmBuilder->getInt32(i)};
        auto outGroupPtr = llvmBuilder->CreateGEP(
            threadVaryingInputType,
            threadInput,
            outIndices,
            llvm::Twine("groupID").concat(llvm::Twine(i)));
        llvmBuilder->CreateStore(inGroupID, outGroupPtr);

        outIndices[1] = llvmBuilder->getInt32(1);
        threadID[i] = llvmBuilder->CreateGEP(
            threadVaryingInputType,
            threadInput,
            outIndices,
            llvm::Twine("threadID").concat(llvm::Twine(i)));
    }

    llvm::BasicBlock* threadEntryBlocks[3];
    llvm::BasicBlock* threadBodyBlocks[3];
    llvm::BasicBlock* threadEndBlocks[3];
    llvm::BasicBlock* endBlock;

    // Create all blocks first, so that they can jump around.
    for (int i = 0; i < 3; ++i)
    {
        threadEntryBlocks[i] = llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("threadEntry").concat(llvm::Twine(i)),
            dispatcher);
        threadBodyBlocks[i] = llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("threadBody").concat(llvm::Twine(i)),
            dispatcher);
    }

    for (int i = 2; i >= 0; --i)
    {
        threadEndBlocks[i] = llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("threadEnd").concat(llvm::Twine(i)),
            dispatcher);
    }
    endBlock = llvm::BasicBlock::Create(*llvmContext, llvm::Twine("end"), dispatcher);

    // Populate thread dispatch headers.
    for (int i = 0; i < 3; ++i)
    {
        llvmBuilder->CreateStore(llvmBuilder->getInt32(0), threadID[i]);
        llvmBuilder->CreateBr(threadEntryBlocks[i]);
        llvmBuilder->SetInsertPoint(threadEntryBlocks[i]);

        auto id = llvmBuilder->CreateLoad(uintType, threadID[i]);
        auto cond =
            llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, id, workGroupSize[i]);

        auto merge = i == 0 ? endBlock : threadEndBlocks[i - 1];
        llvmBuilder->CreateCondBr(cond, threadBodyBlocks[i], merge);
        llvmBuilder->SetInsertPoint(threadBodyBlocks[i]);
    }

    // Do the call to the actual entry point function.
    llvm::Value* args[3] = {threadInput, dispatcher->getArg(1), dispatcher->getArg(2)};
    llvmBuilder->CreateCall(llvm::cast<llvm::Function>(entryPointFunc), args);
    llvmBuilder->CreateBr(threadEndBlocks[2]);

    // Finish thread dispatch blocks
    for (int i = 2; i >= 0; --i)
    {
        llvmBuilder->SetInsertPoint(threadEndBlocks[i]);
        auto id = llvmBuilder->CreateLoad(uintType, threadID[i]);
        auto inc = llvmBuilder->CreateAdd(id, llvmBuilder->getInt32(1));
        llvmBuilder->CreateStore(inc, threadID[i]);
        llvmBuilder->CreateBr(threadEntryBlocks[i]);
    }

    llvmBuilder->SetInsertPoint(endBlock);
    llvmBuilder->CreateRetVoid();
    return dispatcher;
}

LLVMInst* LLVMBuilder::emitComputeEntryPointDispatcher(LLVMInst* workGroupFunc, CharSlice name)
{
    llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());

    llvm::Type* uintType = llvmBuilder->getInt32Ty();

    llvm::Type* argTypes[3] = {
        llvmBuilder->getPtrTy(0),
        llvmBuilder->getPtrTy(0),
        llvmBuilder->getPtrTy(0)};
    llvm::FunctionType* dispatchFuncType =
        llvm::FunctionType::get(llvmBuilder->getVoidTy(), argTypes, false);

    llvm::Function* dispatcher = llvm::Function::Create(
        dispatchFuncType,
        llvm::GlobalValue::ExternalLinkage,
        charSliceToLLVM(name),
        *llvmModule);

    for (auto& arg : dispatcher->args())
        arg.addAttr(llvm::Attribute::NoAlias);

    // This has to be ABI-compatible with the C++ target. So don't go
    // changing this without a good reason to.
    llvm::StructType* varyingInputType = llvm::StructType::get(
        // startGroupID
        llvm::ArrayType::get(uintType, 3),
        // endGroupID
        llvm::ArrayType::get(uintType, 3));

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(*llvmContext, "entry", dispatcher);
    llvmBuilder->SetInsertPoint(entryBlock);

    llvm::Value* varyingInput = dispatcher->getArg(0);
    llvm::Type* groupIDType = llvm::ArrayType::get(uintType, 3);
    llvm::Value* groupIDVar = llvmBuilder->CreateAlloca(groupIDType);

    llvm::Value* startGroupID[3];
    llvm::Value* groupID[3];
    llvm::Value* endGroupID[3];

    for (int i = 0; i < 3; ++i)
    {
        llvm::Value* varyingIndices[3] = {
            llvmBuilder->getInt32(0),
            llvmBuilder->getInt32(0), // startGroupID && groupID
            llvmBuilder->getInt32(i),
        };

        auto startGroupPtr = llvmBuilder->CreateGEP(varyingInputType, varyingInput, varyingIndices);
        startGroupID[i] = llvmBuilder->CreateLoad(
            uintType,
            startGroupPtr,
            llvm::Twine("startGroupID").concat(llvm::Twine(i)));

        llvm::Value* groupIndices[2] = {llvmBuilder->getInt32(0), llvmBuilder->getInt32(i)};
        groupID[i] = llvmBuilder->CreateGEP(
            groupIDType,
            groupIDVar,
            groupIndices,
            llvm::Twine("groupID").concat(llvm::Twine(i)));
        varyingIndices[1] = llvmBuilder->getInt32(1);
        auto endGroupPtr = llvmBuilder->CreateGEP(varyingInputType, varyingInput, varyingIndices);
        endGroupID[i] = llvmBuilder->CreateLoad(
            uintType,
            endGroupPtr,
            llvm::Twine("endGroupID").concat(llvm::Twine(i)));
    }
    // We need 3 nested loops... :(

    llvm::BasicBlock* wgEntryBlocks[3];
    llvm::BasicBlock* wgBodyBlocks[3];
    llvm::BasicBlock* wgEndBlocks[3];
    llvm::BasicBlock* endBlock;

    // Create all blocks first, so that they can jump around.
    for (int i = 0; i < 3; ++i)
    {
        wgEntryBlocks[i] = llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("dispatchEntry").concat(llvm::Twine(i)),
            dispatcher);
        wgBodyBlocks[i] = llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("dispatchBody").concat(llvm::Twine(i)),
            dispatcher);
    }

    for (int i = 2; i >= 0; --i)
    {
        wgEndBlocks[i] = llvm::BasicBlock::Create(
            *llvmContext,
            llvm::Twine("dispatchEnd").concat(llvm::Twine(i)),
            dispatcher);
    }
    endBlock = llvm::BasicBlock::Create(*llvmContext, llvm::Twine("end"), dispatcher);

    // Populate group dispatch headers.
    for (int i = 0; i < 3; ++i)
    {
        llvmBuilder->CreateStore(startGroupID[i], groupID[i]);
        llvmBuilder->CreateBr(wgEntryBlocks[i]);
        llvmBuilder->SetInsertPoint(wgEntryBlocks[i]);
        auto id = llvmBuilder->CreateLoad(uintType, groupID[i]);
        auto cond = llvmBuilder->CreateCmp(llvm::CmpInst::Predicate::ICMP_ULT, id, endGroupID[i]);

        auto merge = i == 0 ? endBlock : wgEndBlocks[i - 1];

        llvmBuilder->CreateCondBr(cond, wgBodyBlocks[i], merge);
        llvmBuilder->SetInsertPoint(wgBodyBlocks[i]);
    }

    // Do the call to the actual entry point function.
    llvm::Value* args[3] = {groupIDVar, dispatcher->getArg(1), dispatcher->getArg(2)};
    llvmBuilder->CreateCall(llvm::cast<llvm::Function>(workGroupFunc), args);
    llvmBuilder->CreateBr(wgEndBlocks[2]);

    // Finish group dispatch blocks
    for (int i = 2; i >= 0; --i)
    {
        llvmBuilder->SetInsertPoint(wgEndBlocks[i]);
        auto id = llvmBuilder->CreateLoad(uintType, groupID[i]);
        auto inc = llvmBuilder->CreateAdd(id, llvmBuilder->getInt32(1));
        llvmBuilder->CreateStore(inc, groupID[i]);
        llvmBuilder->CreateBr(wgEntryBlocks[i]);
    }

    llvmBuilder->SetInsertPoint(endBlock);
    llvmBuilder->CreateRetVoid();
    return dispatcher;
}

SlangResult LLVMBuilder::generateAssembly(IArtifact** outArtifact)
{
    finalize();

    std::string out;
    llvm::raw_string_ostream rso(out);
    llvmModule->print(rso, nullptr);
    String assembly = out.c_str();
    ComPtr<ISlangBlob> blob = StringBlob::moveCreate(assembly);
    auto artifact = ArtifactUtil::createArtifactForCompileTarget(options.target);
    artifact->addRepresentationUnknown(blob);
    *outArtifact = artifact.detach();
    return SLANG_OK;
}

SlangResult LLVMBuilder::generateObjectCode(IArtifact** outArtifact)
{
    finalize();

    List<uint8_t> objectData;
    BinaryLLVMOutputStream output(objectData);

    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;
    if (targetMachine->addPassesToEmitFile(pass, output, nullptr, fileType))
    {
        ArtifactDiagnostic diagnostic;

        diagnostic.severity = ArtifactDiagnostic::Severity::Error;
        diagnostic.stage = ArtifactDiagnostic::Stage::Link;
        diagnostic.text = TerminatedCharSlice("LLVM codegen failed!");

        *outArtifact = createErrorArtifact(diagnostic);
        return SLANG_FAIL;
    }

    pass.run(*llvmModule);

    ComPtr<ISlangBlob> blob = RawBlob::create(objectData.getBuffer(), objectData.getCount());
    auto artifact = ArtifactUtil::createArtifactForCompileTarget(options.target);
    artifact->addRepresentationUnknown(blob);
    *outArtifact = artifact.detach();

    return SLANG_OK;
}

SlangResult LLVMBuilder::generateJITLibrary(IArtifact** outArtifact)
{
    finalize();

    std::unique_ptr<llvm::orc::LLJIT> jit;
    {
        llvm::orc::LLJITBuilder jitBuilder;
        llvm::Expected<std::unique_ptr<llvm::orc::LLJIT>> expectJit = jitBuilder.create();

        if (!expectJit)
        {
            auto err = expectJit.takeError();

            std::string jitErrorString;
            llvm::raw_string_ostream jitErrorStream(jitErrorString);

            jitErrorStream << err;

            ArtifactDiagnostic diagnostic;

            StringBuilder buf;
            buf << "Unable to create JIT engine: " << jitErrorString.c_str();

            diagnostic.severity = ArtifactDiagnostic::Severity::Error;
            diagnostic.stage = ArtifactDiagnostic::Stage::Link;
            diagnostic.text = TerminatedCharSlice(buf.getBuffer(), buf.getLength());

            *outArtifact = createErrorArtifact(diagnostic);
            return SLANG_FAIL;
        }
        jit = std::move(*expectJit);
    }
    llvm::orc::ThreadSafeModule threadSafeModule(std::move(llvmModule), std::move(llvmContext));

    if (auto err = jit->addIRModule(std::move(threadSafeModule)))
    {
        return SLANG_FAIL;
    }

    if (auto err = jit->initialize(jit->getMainJITDylib()))
    {
        return SLANG_FAIL;
    }
    ComPtr<ISlangSharedLibrary> sharedLibrary(new LLVMJITSharedLibrary(std::move(jit)));

    const auto targetDesc = ArtifactDescUtil::makeDescForCompileTarget(options.target);

    auto artifact = ArtifactUtil::createArtifact(targetDesc);

    artifact->addRepresentation(sharedLibrary);

    *outArtifact = artifact.detach();

    return SLANG_OK;
}

} // namespace slang_llvm

extern "C" SLANG_DLL_EXPORT SlangResult createLLVMBuilder_V2(
    const SlangUUID& intfGuid,
    Slang::ILLVMBuilder** out,
    Slang::LLVMBuilderOptions options,
    Slang::IArtifact** outErrorArtifact)
{
    Slang::ComPtr<slang_llvm::LLVMBuilder> builder(
        new slang_llvm::LLVMBuilder(options, outErrorArtifact));

    if (auto ptr = builder->castAs(intfGuid))
    {
        builder.detach();
        *out = (Slang::ILLVMBuilder*)ptr;
        return SLANG_OK;
    }

    return SLANG_E_NO_INTERFACE;
}
