#ifdef _MSC_VER
// A narrowing conversion is caused inside an LLVM header, which we can't fix
// now. So let's just disable the warning.
#pragma warning(disable : 4267)
#endif

#include "compiler-core/slang-artifact-associated-impl.h"
#include "compiler-core/slang-artifact-desc-util.h"
#include "compiler-core/slang-artifact-associated.h"
#include "core/slang-blob.h"
#include "core/slang-list.h"
#include "core/slang-com-object.h"
#include "slang-llvm-jit-shared-library.h"
#include "slang-llvm-builder.h"

#include <filesystem>
#include <llvm/AsmParser/Parser.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Linker/Linker.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

using namespace slang;

namespace slang_llvm
{

using namespace Slang;

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

static LLVMInst* wrap(llvm::Value* val)
{
    return reinterpret_cast<LLVMInst*>(val);
}

static LLVMType* wrap(llvm::Type* type)
{
    return reinterpret_cast<LLVMType*>(type);
}

static LLVMDebugNode* wrap(llvm::DINode* node)
{
    return reinterpret_cast<LLVMDebugNode*>(node);
}

static llvm::Value* unwrap(LLVMInst* val)
{
    return reinterpret_cast<llvm::Value*>(val);
}

static llvm::Type* unwrap(LLVMType* type)
{
    return reinterpret_cast<llvm::Type*>(type);
}

static llvm::DINode* unwrap(LLVMDebugNode* node)
{
    return reinterpret_cast<llvm::DINode*>(node);
}

template<typename T>
static llvm::ArrayRef<decltype(unwrap(T()))> unwrap(Slice<T> slice)
{
    using TargetType = decltype(unwrap(T()));
    return llvm::ArrayRef(reinterpret_cast<const TargetType*>(slice.begin()), slice.count);
}

template<typename T, typename U>
static llvm::ArrayRef<T*> upcast(Slice<U> slice)
{
    return llvm::ArrayRef(reinterpret_cast<T* const*>(slice.begin()), slice.count);
}

llvm::StringRef charSliceToLLVM(TerminatedCharSlice slice)
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

    llvm::GlobalVariable* promotedGlobalInsts;

    // "Global constructors" are functions that get called when the executable
    // or library is loaded, similar to constructors of global variables in C++.
    List<llvm::Constant*> globalCtors;
    llvm::StructType* llvmCtorType;

    // This one is cached here, because we spam it constantly with every GEP.
    llvm::Type* byteType;

public:
    typedef ComBaseObject Super;

    // IUnknown
    SLANG_COM_BASE_IUNKNOWN_ALL

    LLVMBuilder(
        LLVMBuilderOptions options,
        IArtifact** outErrorArtifact);
    ~LLVMBuilder();

    void* getInterface(const Guid& guid);

    IArtifact* createErrorArtifact(const ArtifactDiagnostic& diagnostic);

    void optimize();
    void finalize();

    int getPointerSizeInBits() override;
    int getStoreSizeOf(LLVMInst* value) override;

    LLVMType* getVoidType() override;
    LLVMType* getIntType(int bitSize) override;
    LLVMType* getFloatType(int bitSize) override;
    LLVMType* getPointerType() override;
    LLVMType* getVectorType(int elementCount, LLVMType* elementType) override;
    LLVMType* getBufferType() override;
    LLVMType* getFunctionType(LLVMType* returnType, Slice<LLVMType*> paramTypes, bool variadic) override;

    LLVMInst* emitAlloca(int size, int alignment) override;
    LLVMInst* emitGetElementPtr(LLVMInst* ptr, int stride, LLVMInst* index) override;
    LLVMInst* emitStore(LLVMInst* value, LLVMInst* ptr, int alignment, bool isVolatile = false) override;
    LLVMInst* emitLoad(LLVMType* type, LLVMInst* ptr, int alignment, bool isVolatile = false) override;
    LLVMInst* emitIntResize(LLVMInst* value, LLVMType* into, bool isSigned = false) override;
    LLVMInst* emitCopy(LLVMInst* dstPtr, int dstAlign, LLVMInst* srcPtr, int srcAlign, int bytes, bool isVolatile = false) override;

    LLVMInst* getPoison(LLVMType* type) override;
    LLVMInst* getConstantInt(LLVMType* type, uint64_t value) override;
    LLVMInst* getConstantPtr(uint64_t value) override;
    LLVMInst* getConstantFloat(LLVMType* type, double value) override;
    LLVMInst* getConstantArray(Slice<LLVMInst*> values) override;
    LLVMInst* getConstantString(TerminatedCharSlice literal) override;
    LLVMInst* getConstantStruct(Slice<LLVMInst*> values) override;
    LLVMInst* getConstantVector(Slice<LLVMInst*> values) override;
    LLVMInst* getConstantVector(LLVMInst* value, int count) override;
    LLVMInst* getConstantExtractElement(LLVMInst* value, int index) override;

    LLVMDebugNode* getDebugFallbackType(TerminatedCharSlice name) override;
    LLVMDebugNode* getDebugVoidType() override;
    LLVMDebugNode* getDebugIntType(const char* name, bool isSigned, int bitSize) override;
    LLVMDebugNode* getDebugFloatType(const char* name, int bitSize) override;
    LLVMDebugNode* getDebugPointerType(LLVMDebugNode* pointee) override;
    LLVMDebugNode* getDebugReferenceType(LLVMDebugNode* pointee) override;
    LLVMDebugNode* getDebugStringType() override;
    LLVMDebugNode* getDebugVectorType(int sizeBytes, int alignBytes, int elementCount, LLVMDebugNode* elementType) override;
    LLVMDebugNode* getDebugArrayType(int sizeBytes, int alignBytes, int elementCount, LLVMDebugNode* elementType) override;
    LLVMDebugNode* getDebugStructField(
        LLVMDebugNode* type,
        TerminatedCharSlice name,
        int offset,
        int size,
        int alignment,
        LLVMDebugNode* file,
        int line
    ) override;
    LLVMDebugNode* getDebugStructType(
        Slice<LLVMDebugNode*> fields,
        TerminatedCharSlice name,
        int size,
        int alignment,
        LLVMDebugNode* file,
        int line
    ) override;
    LLVMDebugNode* getDebugFunctionType(LLVMDebugNode* returnType, Slice<LLVMDebugNode*> paramTypes) override;

    SlangResult generateAssembly(IArtifact** outArtifact) override;
    SlangResult generateObjectCode(IArtifact** outArtifact) override;
    SlangResult generateJITLibrary(IArtifact** outArtifact) override;
};

LLVMBuilder::LLVMBuilder(
    LLVMBuilderOptions options,
    IArtifact** outErrorArtifact
): options(options)
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

    // TODO: Should probably complain here if the target machine's pointer
    // size doesn't match SLANG_PTR_IS_32 & SLANG_PTR_IS_64. Although, I'd
    // rather just fix the whole pointer size mechanism in Slang.
    std::string targetTripleStr = llvm::sys::getDefaultTargetTriple();
    if (options.targetTriple.count != 0 && !options.useJIT)
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

    if (options.debugLevel != SLANG_DEBUG_INFO_LEVEL_NONE)
    {
        llvmModule->addModuleFlag(
            llvm::Module::Warning,
            "Debug Info Version",
            llvm::DEBUG_METADATA_VERSION);

        // TODO: Support separate debug info - that'll need to use the SplitName
        // parameter!
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
    // Everything else is managed and automatically free'd by llvmModule.
    delete targetMachine;
}

void* LLVMBuilder::getInterface(const Guid& guid)
{
    if (guid == ISlangUnknown::getTypeGuid() || guid == ILLVMBuilder::getTypeGuid())
    {
        return static_cast<ILLVMBuilder*>(this);
    }
    return nullptr;
}

IArtifact* LLVMBuilder::createErrorArtifact(const ArtifactDiagnostic& diagnostic)
{
    ComPtr<IArtifactDiagnostics> diagnostics(new ArtifactDiagnostics);
    diagnostics->add(diagnostic);
    diagnostics->setResult(SLANG_FAIL);

    auto artifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(ArtifactKind::None, ArtifactPayload::None));
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

int LLVMBuilder::getPointerSizeInBits()
{
    return targetDataLayout.getPointerSizeInBits();
}

int LLVMBuilder::getStoreSizeOf(LLVMInst* value)
{
    return targetDataLayout.getTypeStoreSize(unwrap(value)->getType());
}

LLVMType* LLVMBuilder::getVoidType()
{
    return wrap(llvmBuilder->getVoidTy());
}

LLVMType* LLVMBuilder::getIntType(int bitSize)
{
    return wrap(llvmBuilder->getIntNTy(bitSize));
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
    return wrap(type);
}

LLVMType* LLVMBuilder::getPointerType()
{
    return wrap(llvmBuilder->getPtrTy());
}

LLVMType* LLVMBuilder::getVectorType(int elementCount, LLVMType* elementType)
{
    auto type = llvm::VectorType::get(
        unwrap(elementType),
        llvm::ElementCount::getFixed(elementCount)
    );
    return wrap(type);
}

LLVMType* LLVMBuilder::getBufferType()
{
    return wrap(llvm::StructType::get(llvmBuilder->getPtrTy(0), llvmBuilder->getIntPtrTy(targetDataLayout)));
}

LLVMType* LLVMBuilder::getFunctionType(LLVMType* returnType, Slice<LLVMType*> paramTypes, bool variadic)
{
    return wrap(llvm::FunctionType::get(unwrap(returnType), unwrap(paramTypes), variadic));
}

LLVMInst* LLVMBuilder::emitAlloca(int size, int alignment)
{
    return wrap(llvmBuilder->Insert(new llvm::AllocaInst(
        byteType,
        0,
        llvmBuilder->getInt32(size),
        llvm::Align(alignment))));
}

LLVMInst* LLVMBuilder::emitGetElementPtr(LLVMInst* ptr, int stride, LLVMInst* index)
{
    return wrap(llvmBuilder->CreateGEP(
        stride == 1 ? byteType : llvm::ArrayType::get(byteType, stride),
        unwrap(ptr),
        unwrap(index)));
}

LLVMInst* LLVMBuilder::emitStore(LLVMInst* value, LLVMInst* ptr, int alignment, bool isVolatile)
{
    return wrap(llvmBuilder->CreateAlignedStore(
        unwrap(value),
        unwrap(ptr),
        llvm::MaybeAlign(alignment),
        isVolatile));
}

LLVMInst* LLVMBuilder::emitLoad(LLVMType* type, LLVMInst* ptr, int alignment, bool isVolatile)
{
    return wrap(llvmBuilder->CreateAlignedLoad(
        unwrap(type),
        unwrap(ptr),
        llvm::MaybeAlign(alignment),
        isVolatile));
}

LLVMInst* LLVMBuilder::emitIntResize(LLVMInst* value, LLVMType* into, bool isSigned)
{
    auto llvmValue = unwrap(value);
    auto llvmType = unwrap(into);
    return wrap(isSigned ? 
        llvmBuilder->CreateSExtOrTrunc(llvmValue, llvmType) :
        llvmBuilder->CreateZExtOrTrunc(llvmValue, llvmType)
    );
}

LLVMInst* LLVMBuilder::emitCopy(LLVMInst* dstPtr, int dstAlign, LLVMInst* srcPtr, int srcAlign, int bytes, bool isVolatile)
{
    return wrap(llvmBuilder->CreateMemCpyInline(
        unwrap(dstPtr),
        llvm::MaybeAlign(dstAlign),
        unwrap(srcPtr),
        llvm::MaybeAlign(srcAlign),
        llvmBuilder->getInt32(bytes),
        isVolatile));
}

LLVMInst* LLVMBuilder::getPoison(LLVMType* type)
{
    return wrap(llvm::PoisonValue::get(unwrap(type)));
}

LLVMInst* LLVMBuilder::getConstantInt(LLVMType* type, uint64_t value)
{
    return wrap(llvm::ConstantInt::get(unwrap(type), value));
}

LLVMInst* LLVMBuilder::getConstantPtr(uint64_t value)
{
    auto llvmType = llvmBuilder->getPtrTy();
    if (value == 0)
    {
        return wrap(llvm::ConstantPointerNull::get(llvmType));
    }
    else
    {
        return wrap(llvm::ConstantExpr::getIntToPtr(
                    llvm::ConstantInt::get(
                        llvmBuilder->getIntPtrTy(targetDataLayout),
                        value),
                    llvmType));
    }
}

LLVMInst* LLVMBuilder::getConstantFloat(LLVMType* type, double value)
{
    return wrap(llvm::ConstantFP::get(unwrap(type), value));
}

LLVMInst* LLVMBuilder::getConstantArray(Slice<LLVMInst*> values)
{
    llvm::ArrayType* type = nullptr;
    if (values.count != 0)
    {
        type = llvm::ArrayType::get(unwrap(values[0])->getType(), values.count);
    }
    else type = llvm::ArrayType::get(byteType, 0);

    return wrap(llvm::ConstantArray::get(type, upcast<llvm::Constant>(values)));
}

LLVMInst* LLVMBuilder::getConstantString(TerminatedCharSlice literal)
{
    return wrap(llvmBuilder->CreateGlobalString(charSliceToLLVM(literal)));
}

LLVMInst* LLVMBuilder::getConstantStruct(Slice<LLVMInst*> values)
{
    return wrap(llvm::ConstantStruct::getAnon(upcast<llvm::Constant>(values), true));
}

LLVMInst* LLVMBuilder::getConstantVector(Slice<LLVMInst*> values)
{
    return wrap(llvm::ConstantVector::get(upcast<llvm::Constant>(values)));
}

LLVMInst* LLVMBuilder::getConstantVector(LLVMInst* value, int count)
{
    return wrap(llvm::ConstantVector::getSplat(
        llvm::ElementCount::getFixed(count),
        llvm::cast<llvm::Constant>(unwrap(value))));
}

LLVMInst* LLVMBuilder::getConstantExtractElement(LLVMInst* value, int index)
{
    auto llvmIndex = llvm::ConstantInt::get(llvmBuilder->getInt32Ty(), index);
    return wrap(llvm::ConstantExpr::getExtractElement(llvm::cast<llvm::Constant>(unwrap(value)), llvmIndex));
}

LLVMDebugNode* LLVMBuilder::getDebugFallbackType(TerminatedCharSlice name)
{
    return wrap(llvmDebugBuilder->createUnspecifiedType(charSliceToLLVM(name)));
}

LLVMDebugNode* LLVMBuilder::getDebugVoidType()
{
    return wrap(llvmDebugBuilder->createUnspecifiedType("void"));
}

LLVMDebugNode* LLVMBuilder::getDebugIntType(const char* name, bool isSigned, int bitSize)
{
    unsigned encoding = isSigned ? llvm::dwarf::DW_ATE_signed : llvm::dwarf::DW_ATE_unsigned;
    if (bitSize == 1) encoding = llvm::dwarf::DW_ATE_boolean;
    return wrap(llvmDebugBuilder->createBasicType(name, bitSize, encoding));
}

LLVMDebugNode* LLVMBuilder::getDebugFloatType(const char* name, int bitSize)
{
    return wrap(llvmDebugBuilder->createBasicType(name, bitSize, llvm::dwarf::DW_ATE_float));
}

LLVMDebugNode* LLVMBuilder::getDebugPointerType(LLVMDebugNode* pointee)
{
    llvm::DIType* llvmPointee = llvm::cast<llvm::DIType>(unwrap(pointee ? pointee : getDebugVoidType()));
    return wrap(llvmDebugBuilder->createPointerType(llvmPointee, targetDataLayout.getPointerSizeInBits()));
}

LLVMDebugNode* LLVMBuilder::getDebugReferenceType(LLVMDebugNode* pointee)
{
    return wrap(llvmDebugBuilder->createReferenceType(
        llvm::dwarf::DW_TAG_reference_type,
        llvm::cast<llvm::DIType>(unwrap(pointee)),
        targetDataLayout.getPointerSizeInBits()));
}

LLVMDebugNode* LLVMBuilder::getDebugStringType()
{
    return wrap(llvmDebugBuilder->createPointerType(
                llvmDebugBuilder->createBasicType("char", 8, llvm::dwarf::DW_ATE_signed_char),
                targetDataLayout.getPointerSizeInBits()));
}

LLVMDebugNode* LLVMBuilder::getDebugVectorType(int sizeBytes, int alignBytes, int elementCount, LLVMDebugNode* elementType)
{
    if (sizeBytes < alignBytes)
        sizeBytes = alignBytes;

    llvm::Metadata* subscript = llvmDebugBuilder->getOrCreateSubrange(0, elementCount);
    llvm::DINodeArray subscriptArray = llvmDebugBuilder->getOrCreateArray(subscript);
    return wrap(llvmDebugBuilder->createVectorType(
        sizeBytes * 8,
        alignBytes * 8,
        llvm::cast<llvm::DIType>(unwrap(elementType)),
        subscriptArray));
}

LLVMDebugNode* LLVMBuilder::getDebugArrayType(int sizeBytes, int alignBytes, int elementCount, LLVMDebugNode* elementType)
{
    llvm::Metadata* subscript = llvmDebugBuilder->getOrCreateSubrange(0, elementCount);
    llvm::DINodeArray subscriptArray = llvmDebugBuilder->getOrCreateArray(subscript);
    return wrap(llvmDebugBuilder->createArrayType(
        sizeBytes * 8,
        alignBytes * 8,
        llvm::cast<llvm::DIType>(unwrap(elementType)),
        subscriptArray));
}

LLVMDebugNode* LLVMBuilder::getDebugStructField(
    LLVMDebugNode* type,
    TerminatedCharSlice name,
    int offset,
    int size,
    int alignment,
    LLVMDebugNode* file,
    int line
){
    llvm::DIFile* llvmFile = llvm::cast<llvm::DIFile>(unwrap(file));
    return wrap(llvmDebugBuilder->createMemberType(
        llvmFile,
        charSliceToLLVM(name),
        llvmFile,
        line,
        size * 8,
        alignment * 8,
        offset * 8,
        llvm::DINode::FlagZero,
        llvm::cast<llvm::DIType>(unwrap(type))));
}

LLVMDebugNode* LLVMBuilder::getDebugStructType(
    Slice<LLVMDebugNode*> fields,
    TerminatedCharSlice name,
    int size,
    int alignment,
    LLVMDebugNode* file,
    int line
){
    llvm::DINodeArray fieldTypes = llvmDebugBuilder->getOrCreateArray(upcast<llvm::Metadata>(fields));
    llvm::DIFile* llvmFile = llvm::cast<llvm::DIFile>(unwrap(file));

    return wrap(llvmDebugBuilder->createStructType(
        llvmFile,
        charSliceToLLVM(name),
        llvmFile,
        line,
        size * 8,
        alignment * 8,
        llvm::DINode::FlagZero,
        nullptr,
        fieldTypes));
}

LLVMDebugNode* LLVMBuilder::getDebugFunctionType(LLVMDebugNode* returnType, Slice<LLVMDebugNode*> paramTypes)
{
    List<llvm::Metadata*> elements;
    elements.add(unwrap(returnType));
    for (LLVMDebugNode* param : paramTypes)
        elements.add(unwrap(param));

    return wrap(llvmDebugBuilder->createSubroutineType(llvmDebugBuilder->getOrCreateTypeArray(
        llvm::ArrayRef<llvm::Metadata*>(elements.begin(), elements.end()))));
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

    ComPtr<ISlangSharedLibrary> sharedLibrary(new LLVMJITSharedLibrary(std::move(jit)));

    const auto targetDesc = ArtifactDescUtil::makeDescForCompileTarget(options.target);

    auto artifact = ArtifactUtil::createArtifact(targetDesc);

    artifact->addRepresentation(sharedLibrary);

    *outArtifact = artifact.detach();

    return SLANG_OK;
}

} // namespace slang_llvm
