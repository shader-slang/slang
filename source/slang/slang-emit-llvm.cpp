#include "slang-emit-llvm.h"
#include "slang-ir-insts.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Scalar/Reassociate.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Scalar/LoopUnrollPass.h>
#include <llvm/Transforms/Scalar/IndVarSimplify.h>
#include <llvm/Transforms/Scalar/DeadStoreElimination.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>
#include <llvm/Transforms/Utils/LoopSimplify.h>
#include <llvm/Transforms/IPO/Inliner.h>
#include <llvm/Transforms/IPO/ModuleInliner.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

using namespace slang;

namespace Slang
{

struct BinaryLLVMOutputStream: public llvm::raw_pwrite_stream
{
    List<uint8_t>& output;

    BinaryLLVMOutputStream(List<uint8_t>& output)
        : output(output)
    {
        SetUnbuffered();
    }

    void write_impl(const char *Ptr, size_t Size) override
    {
        output.insertRange(output.getCount(), reinterpret_cast<const uint8_t*>(Ptr), Size);
    }

    void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override
    {
        output.insertRange(Offset, reinterpret_cast<const uint8_t*>(Ptr), Size);
    }

    uint64_t current_pos() const override
    {
        return output.getCount();
    }

    void reserveExtraSpace(uint64_t ExtraSize) override
    {
        output.reserve(tell() + ExtraSize);
    }
};

struct LLVMEmitter
{
    llvm::LLVMContext llvmContext;
    llvm::IRBuilder<> llvmBuilder;
    llvm::Module llvmModule;

    // The LLVM value class is closest to Slang's IRInst, as it can represent
    // constants, instructions and functions, whereas llvm::Instruction only
    // handles instructions.
    Dictionary<IRInst*, llvm::Value*> mapInstToLLVM;
    Dictionary<IRType*, llvm::Type*> mapTypeToLLVM;

    CodeGenContext* codeGenContext;

    LLVMEmitter(CodeGenContext* codeGenContext)
        : llvmBuilder(llvmContext), llvmModule("module", llvmContext), codeGenContext(codeGenContext)
    {
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
    }

    ~LLVMEmitter()
    {
    }

    bool maybeGetName(llvm::StringRef* nameOut, IRInst* irInst)
    {
        UnownedStringSlice name;
        if (auto externCppDecoration = irInst->findDecoration<IRExternCppDecoration>())
        {
            name = externCppDecoration->getName();
        }
        else if (auto nameDecor = irInst->findDecoration<IRNameHintDecoration>())
        {
            name = nameDecor->getName();
        }
        else if (auto linkageDecoration = irInst->findDecoration<IRLinkageDecoration>())
        {
            name = linkageDecoration->getMangledName();
        }
        else return false;

        *nameOut = llvm::StringRef(name.begin(), name.getLength());
        return true;
    }

    llvm::Type* ensureType(IRType* type)
    {
        if (mapTypeToLLVM.containsKey(type))
            return mapTypeToLLVM.getValue(type);

        llvm::Type* llvmType = nullptr;

        switch (type->getOp())
        {
        case kIROp_VoidType:
            llvmType = llvm::Type::getVoidTy(llvmContext);
            break;
        case kIROp_Int8Type:
        case kIROp_UInt8Type:
        case kIROp_BoolType:
            llvmType = llvm::Type::getInt8Ty(llvmContext);
            break;
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
            llvmType = llvm::Type::getInt16Ty(llvmContext);
            break;
        case kIROp_IntType:
        case kIROp_UIntType:
#if SLANG_PTR_IS_32
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
#endif
            llvmType = llvm::Type::getInt32Ty(llvmContext);
            break;
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
#if SLANG_PTR_IS_64
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
#endif
            llvmType = llvm::Type::getInt64Ty(llvmContext);
            break;
        case kIROp_RawPointerType:
        case kIROp_RTTIPointerType:
        case kIROp_PtrType:
        case kIROp_NativePtrType:
        case kIROp_NativeStringType:
        case kIROp_OutType:
        case kIROp_InOutType:
        case kIROp_RefType:
        case kIROp_ConstRefType:
            // LLVM only has opaque pointers now, so everything that lowers as
            // a pointer is just that same opaque pointer.
            llvmType = llvm::PointerType::get(llvmContext, 0);
            break;
        case kIROp_HalfType:
            llvmType = llvm::Type::getHalfTy(llvmContext);
            break;
        case kIROp_FloatType:
            llvmType = llvm::Type::getFloatTy(llvmContext);
            break;
        case kIROp_DoubleType:
            llvmType = llvm::Type::getDoubleTy(llvmContext);
            break;
        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(type);
                llvm::Type* elemType = ensureType(vecType->getElementType());
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                llvmType = llvm::VectorType::get(elemType, llvm::ElementCount::getFixed(elemCount));
            }
            break;
        case kIROp_MatrixType:
            {
                auto matType = static_cast<IRMatrixType*>(type);
                llvm::Type* elemType = ensureType(matType->getElementType());
                auto elemCount =
                    int(getIntVal(matType->getRowCount())) *
                    int(getIntVal(matType->getColumnCount()));
                llvmType = llvm::VectorType::get(elemType, llvm::ElementCount::getFixed(elemCount));
            }
            break;
        case kIROp_ArrayType:
            {
                auto arrayType = static_cast<IRArrayType*>(type);
                auto elemType = ensureType(arrayType->getElementType());
                auto elemCount = int(getIntVal(arrayType->getElementCount()));

                llvmType = llvm::ArrayType::get(elemType, elemCount);
            }
            break;
        case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(type);

                List<llvm::Type*> fieldTypes;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;
                    fieldTypes.add(ensureType(fieldType));
                }

                auto llvmStructType = llvm::StructType::get(
                    llvmContext,
                    llvm::ArrayRef<llvm::Type*>(fieldTypes.getBuffer(), fieldTypes.getCount())
                );
                llvm::StringRef name;
                if (maybeGetName(&name, type))
                    llvmStructType->setName(name);
                llvmType = llvmStructType;
            }
            break;
        case kIROp_FuncType:
            {
                auto funcType = static_cast<IRFuncType*>(type);

                auto returnType = ensureType(funcType->getResultType());
                List<llvm::Type*> paramTypes;
                for (UInt i = 0; i < funcType->getParamCount(); ++i)
                {
                    IRType* paramType = funcType->getParamType(i);
                    paramTypes.add(ensureType(paramType));
                }

                llvmType = llvm::FunctionType::get(
                    returnType,
                    llvm::ArrayRef(paramTypes.begin(), paramTypes.end()),
                    false);
            }
            break;
        default:
            SLANG_UNEXPECTED("Unsupported type for LLVM target!");
        }

        mapTypeToLLVM[type] = llvmType;
        return llvmType;
    }

    bool isExternallyVisible(IRFunc* func)
    {
        for (auto decor : func->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_ExportDecoration:
            case kIROp_DownstreamModuleExportDecoration:
            case kIROp_HLSLExportDecoration:
            case kIROp_DllExportDecoration:
                {
                    return true;
                }
            default:
                break;
            }
        }
        return false;
    }

    llvm::Value* ensureFuncDecl(IRFunc* func)
    {
        auto funcType = static_cast<IRFuncType*>(func->getDataType());
        llvm::FunctionType* llvmFuncType = llvm::cast<llvm::FunctionType>(ensureType(funcType));

        auto linkageType = llvm::Function::PrivateLinkage;
        if (isExternallyVisible(func))
            linkageType = llvm::Function::ExternalLinkage;

        llvm::Function* llvmFunc = llvm::Function::Create(
            llvmFuncType,
            linkageType,
            "", // Name is conditionally set below.
            llvmModule
        );

        llvm::StringRef name;
        if (maybeGetName(&name, func))
            llvmFunc->setName(name);

        UInt i = 0;
        for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
        {
            // TODO: Add attributes to the params. We can determine some attrs
            // based on whether we've got an OutType, InOutType, RefType,
            // ConstRefType and so on. Not adding attributes is safe but
            // prevents many optimizations.
            auto llvmArg = llvmFunc->getArg(i);
            llvm::StringRef name;
            if (maybeGetName(&name, pp))
                llvmArg->setName(name);
        }

        mapInstToLLVM[func] = llvmFunc;
        return llvmFunc;
    }

    void emitGlobalDeclarations(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                ensureFuncDecl(func);
            }
        }
    }

    void processModule(IRModule* irModule)
    {
        // Start by emitting all function declarations, so that the functions
        // can freely refer to each other later on.
        emitGlobalDeclarations(irModule);
    }

    // Optimizes the LLVM IR and destroys mapInstToLLVM.
    void optimize()
    {
        mapInstToLLVM.clear();

        llvm::ModuleAnalysisManager moduleAnalysisManager;
        llvm::FunctionAnalysisManager functionAnalysisManager;
        llvm::LoopAnalysisManager loopAnalysisManager;
        llvm::CGSCCAnalysisManager CGSCCAnalysisManager;

        llvm::PassBuilder passBuilder;
        passBuilder.registerModuleAnalyses(moduleAnalysisManager);
        passBuilder.registerFunctionAnalyses(functionAnalysisManager);
        passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, CGSCCAnalysisManager, moduleAnalysisManager);

        llvm::PassInstrumentationCallbacks passInstrumentationCallbacks;
        llvm::StandardInstrumentations standardInstrumentations(llvmContext, true);
        standardInstrumentations.registerCallbacks(passInstrumentationCallbacks, &moduleAnalysisManager);

        // TODO: Make the passes configurable, at least via -On
        llvm::FunctionPassManager functionPassManager;
        functionPassManager.addPass(llvm::ReassociatePass());
        functionPassManager.addPass(llvm::GVNPass());
        functionPassManager.addPass(llvm::SimplifyCFGPass());
        functionPassManager.addPass(llvm::PromotePass());
        functionPassManager.addPass(llvm::LoopSimplifyPass());
        functionPassManager.addPass(llvm::LoopUnrollPass());
        functionPassManager.addPass(llvm::DSEPass());
        functionPassManager.addPass(llvm::DCEPass());
        //functionPassManager.addPass(llvm::InstCombinePass());
        functionPassManager.addPass(llvm::AggressiveInstCombinePass());

        llvm::ModulePassManager modulePassManager;
        modulePassManager.addPass(llvm::ModuleInlinerPass());

        // Run the actual optimizations.
        modulePassManager.run(llvmModule, moduleAnalysisManager);
        for(llvm::Function& f: llvmModule)
        {
            if(!f.isDeclaration())
                functionPassManager.run(f, functionAnalysisManager);
        }
    }

    void finalize()
    {
        llvm::verifyModule(llvmModule, &llvm::errs());

        // TODO: Make this optional.
        //optimize();
    }

    void dumpAssembly(String& assemblyOut)
    {
        std::string out;
        llvm::raw_string_ostream rso(out);
        llvmModule.print(rso, nullptr);
        assemblyOut = out.c_str();
    }

    void generateObjectCode(List<uint8_t>& objectOut)
    {
        // TODO: Take the target triple as a parameter to allow
        // cross-compilation.
        std::string target_triple = llvm::sys::getDefaultTargetTriple();
        std::string error;
        const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, error);
        if (!target)
        {
            codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::unrecognizedTargetTriple, target_triple.c_str(), error.c_str());
            return;
        }

        llvm::TargetOptions opt;
        llvm::TargetMachine* target_machine = target->createTargetMachine(target_triple, "generic", "", opt, llvm::Reloc::PIC_);
        SLANG_DEFER(delete target_machine);

        llvmModule.setDataLayout(target_machine->createDataLayout());
        llvmModule.setTargetTriple(target_triple);

        BinaryLLVMOutputStream output(objectOut);

        llvm::legacy::PassManager pass;
        auto fileType = llvm::CodeGenFileType::ObjectFile;
        if (target_machine->addPassesToEmitFile(pass, output, nullptr, fileType))
        {
            codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::llvmCodegenFailed);
            return;
        }

        pass.run(llvmModule);
    }
};

SlangResult emitLLVMAssemblyFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    String& assemblyOut)
{
    LLVMEmitter emitter(codeGenContext);
    emitter.processModule(irModule);
    emitter.finalize();
    emitter.dumpAssembly(assemblyOut);
    return SLANG_OK;
}

SlangResult emitLLVMObjectFromIR(
    CodeGenContext* codeGenContext,
    IRModule* irModule,
    List<uint8_t>& objectOut)
{
    LLVMEmitter emitter(codeGenContext);
    emitter.processModule(irModule);
    emitter.finalize();
    emitter.generateObjectCode(objectOut);
    return SLANG_OK;
}

} // namespace Slang
