#include "slang-emit-llvm.h"
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

    void processModule(IRModule* irModule)
    {
        // TODO: The hard part
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
        // TODO: Make this optional.
        optimize();
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
