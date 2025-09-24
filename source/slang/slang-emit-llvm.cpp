#include "slang-emit-llvm.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-layout.h"
#include "../core/slang-char-util.h"
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ModuleSummaryIndex.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <filesystem>

using namespace slang;

namespace Slang
{

struct BinaryLLVMOutputStream: public llvm::raw_pwrite_stream
{
    List<uint8_t>& output;

    BinaryLLVMOutputStream(List<uint8_t>& output)
        : raw_pwrite_stream(true), output(output)
    {
        SetUnbuffered();
    }

    void write_impl(const char *Ptr, size_t Size) override
    {
        output.insertRange(output.getCount(), reinterpret_cast<const uint8_t*>(Ptr), Size);
    }

    void pwrite_impl(const char *Ptr, size_t Size, uint64_t Offset) override
    {
        memcpy(
            output.getBuffer() + Offset,
            reinterpret_cast<const uint8_t*>(Ptr),
            Size);
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

static UInt parseNumber(const char*& cursor, const char* end)
{
    char d = *cursor;
    SLANG_RELEASE_ASSERT(CharUtil::isDigit(d));
    UInt n = 0;
    while (CharUtil::isDigit(d))
    {
        n = n * 10 + (d - '0');
        cursor++;
        if (cursor == end)
            break;
        d = *cursor;
    }
    return n;
}

struct VariableDebugInfo
{
    llvm::DIVariable* debugVar;
    bool attached = false;
    bool isStackVar = false;
};

struct LLVMEmitter
{
    llvm::LLVMContext llvmContext;
    llvm::Module llvmModule;
    llvm::IRBuilderBase* llvmBuilder;
    llvm::DIBuilder llvmDebugBuilder;
    llvm::ModuleSummaryIndex llvmSummaryIndex;
    llvm::DICompileUnit* compileUnit;

    // The LLVM value class is closest to Slang's IRInst, as it can represent
    // constants, instructions and functions, whereas llvm::Instruction only
    // handles instructions.
    Dictionary<IRInst*, llvm::Value*> mapInstToLLVM;
    Dictionary<IRType*, llvm::Type*> mapTypeToLLVM;
    Dictionary<IRType*, llvm::DIType*> mapTypeToDebugLLVM;
    List<llvm::Constant*> globalCtors;
    llvm::StructType* llvmCtorType;

    Dictionary<IRInst*, llvm::DIFile*> sourceDebugInfo;
    Dictionary<IRInst*, llvm::DISubprogram*> functionDebugInfo;
    Dictionary<IRInst*, VariableDebugInfo> variableDebugInfoMap;

    List<llvm::DIScope*> debugScopeStack;

    CodeGenContext* codeGenContext;

    llvm::TargetMachine* targetMachine;

    bool debug = false;

    LLVMEmitter(CodeGenContext* codeGenContext)
        : llvmModule("module", llvmContext),
          llvmDebugBuilder(llvmModule),
          llvmSummaryIndex(true),
          codeGenContext(codeGenContext)
    {
        auto session = codeGenContext->getSession();

        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        if (getOptions().getOptimizationLevel() == OptimizationLevel::None)
        {
            // The default IR builder has a built-in constant folder; for
            // absolutely zero optimization, we need to disable it like this.
            llvmBuilder = new llvm::IRBuilder<llvm::NoFolder>(llvmContext);
        }
        else
        {
            llvmBuilder = new llvm::IRBuilder<>(llvmContext);
        }

        llvmCtorType = llvm::StructType::get(
            llvmBuilder->getInt32Ty(),
            llvmBuilder->getPtrTy(0),
            llvmBuilder->getPtrTy(0)
        );

        auto targetTripleOption = getOptions().getStringOption(CompilerOptionName::LLVMTargetTriple).getUnownedSlice();
        std::string targetTripleStr = llvm::sys::getDefaultTargetTriple();
        if (targetTripleOption.getLength() != 0)
            targetTripleStr = std::string(targetTripleOption.begin(), targetTripleOption.getLength());

        std::string error;
        const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTripleStr, error);
        if (!target)
        {
            codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::unrecognizedTargetTriple, targetTripleStr.c_str(), error.c_str());
            return;
        }

        llvm::TargetOptions opt;

        opt.setFPDenormalMode(getLLVMDenormalMode(getOptions().getDenormalModeFp64()));
        opt.setFP32DenormalMode(getLLVMDenormalMode(getOptions().getDenormalModeFp32()));

        opt.NoTrappingFPMath = 1;
        switch(getOptions().getFloatingPointMode())
        {
        default:
        case FloatingPointMode::Default:
            break;
        case FloatingPointMode::Fast:
            opt.UnsafeFPMath = 1;
            opt.NoSignedZerosFPMath = 1;
            opt.ApproxFuncFPMath = 1;
            break;
        case FloatingPointMode::Precise:
            opt.UnsafeFPMath = 0;
            opt.NoInfsFPMath = 0;
            opt.NoNaNsFPMath = 0;
            opt.NoSignedZerosFPMath = 0;
            opt.ApproxFuncFPMath = 0;
            break;
        }

        llvm::CodeGenOptLevel optLevel = llvm::CodeGenOptLevel::None;
        switch(getOptions().getOptimizationLevel())
        {
        case OptimizationLevel::None:
            optLevel = llvm::CodeGenOptLevel::None;
            break;
        default:
        case OptimizationLevel::Default:
            optLevel = llvm::CodeGenOptLevel::Less;
            break;
        case OptimizationLevel::High:
            optLevel = llvm::CodeGenOptLevel::Default;
            break;
        case OptimizationLevel::Maximal:
            optLevel = llvm::CodeGenOptLevel::Aggressive;
            break;
        }

        llvm::Triple targetTriple(targetTripleStr);
        targetMachine = target->createTargetMachine(
            targetTriple, "generic", "", opt, llvm::Reloc::PIC_, std::nullopt,
            optLevel
        );

        debug = getOptions().getDebugInfoLevel() != DebugInfoLevel::None;

        if (debug)
        {
            llvmModule.addModuleFlag(
                llvm::Module::Warning,
                "Debug Info Version",
                 llvm::DEBUG_METADATA_VERSION
            );

            StringBuilder sb;
            getOptions().writeCommandLineArgs(codeGenContext->getSession(), sb);

            auto params = sb.toString();

            // TODO: Separate debug info - that'll need to use the SplitName
            // parameter!
            compileUnit = llvmDebugBuilder.createCompileUnit(
                // TODO: We should probably apply for a language ID in DWARF? Not
                // sure how that process goes. Anyway, let's just use C as the ID
                // until this target is properly usable.
                llvm::dwarf::DW_LANG_C,

                // The "compile unit" model doesn't work super well for Slang -
                // we're supposed to only have one for the debug info per output
                // file, as if we were building each .slang-module into a separate
                // `.o`. Which we can't really do without losing large swathes of
                // language features.
                //
                // Anyway, for now, we'll give no name to the "compile unit" and
                // just operate with individual files separately. This doesn't
                // prevent the debugger from seeing our source in any way.
                llvmDebugBuilder.createFile("<slang-llvm>", "."),

                "Slang compiler",

                getOptions().getOptimizationLevel() != OptimizationLevel::None,

                llvm::StringRef(params.begin(), params.getLength()),

                0
            );
        }

        debugScopeStack.add(compileUnit);

        String prelude = session->getPreludeForLanguage(SourceLanguage::LLVM);
        llvm::SMDiagnostic diag;
        llvm::MemoryBufferRef buf(
            llvm::StringRef(prelude.begin(), prelude.getLength()),
            llvm::StringRef("prelude")
        );
        if(llvm::parseAssemblyInto(buf, &llvmModule, &llvmSummaryIndex, diag))
        {
            SLANG_UNEXPECTED("Failed to parse LLVM prelude!");
        }
    }

    ~LLVMEmitter()
    {
        delete targetMachine;
        delete llvmBuilder;
    }

    CompilerOptionSet& getOptions()
    {
        return codeGenContext->getTargetProgram()->getOptionSet();
    }

    llvm::DenormalMode getLLVMDenormalMode(FloatingPointDenormalMode mode)
    {
        switch(mode)
        {
        default:
        case FloatingPointDenormalMode::Any:
            return llvm::DenormalMode::getDefault();
        case FloatingPointDenormalMode::Preserve:
            return llvm::DenormalMode::getPreserveSign();
        case FloatingPointDenormalMode::FlushToZero:
            return llvm::DenormalMode::getPositiveZero();
        }
    }

    // Finds the value of an instruction that has already been emitted, OR
    // creates the value if it's a constant.
    llvm::Value* findValue(IRInst* inst)
    {
        if (mapInstToLLVM.containsKey(inst))
            return mapInstToLLVM.getValue(inst);

        llvm::Value* llvmValue = nullptr;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_PtrLit:
        case kIROp_MakeVector:
            llvmValue = maybeEnsureConstant(inst);
            break;
        default:
            SLANG_UNEXPECTED(
                "Unsupported value type for LLVM target, or referring to an "
                "instruction that hasn't been emitted yet!");
            break;
        }

        SLANG_ASSERT(llvmValue);

        if(llvmValue)
            mapInstToLLVM[inst] = llvmValue;
        return llvmValue;
    }

    void getUnderlyingTypeInfo(IRInst* inst, bool& isFloat, bool& isSigned)
    {
        IRType* elementType = getVectorOrCoopMatrixElementType(inst->getOperand(0)->getDataType());
        IRBasicType* basicType = as<IRBasicType>(elementType);
        isFloat = isFloatingType(basicType);
        isSigned = isSignedType(basicType);
    }

    llvm::Value* _emitCompare(IRInst* inst)
    {
        bool isFloat, isSigned;
        getUnderlyingTypeInfo(inst, isFloat, isSigned);

        llvm::CmpInst::Predicate pred;
        switch (inst->getOp())
        {
        case kIROp_Less:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OLT :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SLT : llvm::CmpInst::Predicate::ICMP_ULT;
            break;
        case kIROp_Leq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OLE :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SLE : llvm::CmpInst::Predicate::ICMP_ULE;
            break;
        case kIROp_Eql:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OEQ : llvm::CmpInst::Predicate::ICMP_EQ;
            break;
        case kIROp_Neq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_ONE : llvm::CmpInst::Predicate::ICMP_NE;
            break;
        case kIROp_Greater:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OGT :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SGT : llvm::CmpInst::Predicate::ICMP_UGT;
            break;
        case kIROp_Geq:
            pred = isFloat ? llvm::CmpInst::Predicate::FCMP_OGE :
                isSigned ? llvm::CmpInst::Predicate::ICMP_SGE : llvm::CmpInst::Predicate::ICMP_UGE;
            break;
        default:
            SLANG_UNEXPECTED("Unsupported compare op");
            break;
        }

        SLANG_ASSERT(inst->getOperandCount() == 2);
        return llvmBuilder->CreateCmp(pred, findValue(inst->getOperand(0)), findValue(inst->getOperand(1)));
    }

    llvm::Value* _emitArithmetic(IRInst* inst)
    {
        bool isFloat, isSigned;
        getUnderlyingTypeInfo(inst, isFloat, isSigned);

        if (inst->getOperandCount() == 1)
        {
            auto llvmValue = findValue(inst->getOperand(0));
            switch (inst->getOp())
            {
            case kIROp_Neg:
                return isFloat ? llvmBuilder->CreateFNeg(llvmValue) : llvmBuilder->CreateNeg(llvmValue);
            case kIROp_Not:
            case kIROp_BitNot:
                return llvmBuilder->CreateNot(llvmValue);
            default:
                SLANG_UNEXPECTED("Unsupported unary arithmetic op");
                break;
            }
        }
        else if (inst->getOperandCount() == 2)
        {
            llvm::Instruction::BinaryOps op;
            switch (inst->getOp())
            {
            case kIROp_Add:
                op = isFloat ? llvm::Instruction::FAdd : llvm::Instruction::Add;
                break;
            case kIROp_Sub:
                op = isFloat ? llvm::Instruction::FSub : llvm::Instruction::Sub;
                break;
            case kIROp_Mul:
                op = isFloat ? llvm::Instruction::FMul : llvm::Instruction::Mul;
                break;
            case kIROp_Div:
                op = isFloat ? llvm::Instruction::FDiv : isSigned ? llvm::Instruction::SDiv : llvm::Instruction::UDiv;
                break;
            case kIROp_IRem:
                op = isSigned ? llvm::Instruction::SRem : llvm::Instruction::URem;
                break;
            case kIROp_FRem:
                op = llvm::Instruction::FRem;
                break;
            case kIROp_And:
                op = llvm::Instruction::And;
                break;
            case kIROp_Or:
                op = llvm::Instruction::Or;
                break;
            case kIROp_BitAnd:
                op = llvm::Instruction::And;
                break;
            case kIROp_BitOr:
                op = llvm::Instruction::Or;
                break;
            case kIROp_BitXor:
                op = llvm::Instruction::Xor;
                break;
            case kIROp_Rsh:
                op = isSigned ? llvm::Instruction::AShr : llvm::Instruction::LShr;
                break;
            case kIROp_Lsh:
                op = llvm::Instruction::Shl;
                break;
            default:
                SLANG_UNEXPECTED("Unsupported binary arithmetic op");
                break;
            }
            return llvmBuilder->CreateBinOp(op, findValue(inst->getOperand(0)), findValue(inst->getOperand(1)));
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected number of operands for arithmetic op");
        }

        return nullptr;
    }

    // Returns the index of the given struct field in the corresponding LLVM
    // struct. There may be differences due to added padding fields.
    UInt mapStructIndexToLLVM(IRStructType* irStruct, UInt irIndex)
    {
        // TODO: Once padded structs exist, update this to map the indices
        // correctly.
        (void)irStruct;
        return irIndex;
    }

    IRStructField* findStructField(IRStructType* irStruct, UInt irIndex)
    {
        UInt i = 0;
        for (auto field : irStruct->getFields())
        {
            if (i == irIndex)
                return field;
            i++;
        }
        return nullptr;
    }

    UInt getStructIndexByKey(IRStructType* irStruct, IRStructKey* irKey)
    {
        UInt i = 0;
        for (auto field : irStruct->getFields())
        {
            if(field->getKey() == irKey)
                return i;
            i++;
        }
        SLANG_UNEXPECTED("Requested key that doesn't exist in struct!");
    }

    bool isVolatile(IRInst* value)
    {
        if (auto memoryQualifier = value->findDecoration<IRMemoryQualifierSetDecoration>())
        {
            if (memoryQualifier->getMemoryQualifierBit() & MemoryQualifierSetModifier::Flags::kVolatile)
            {
                return true;
            }
        }
        return false;
    }

    IRIntegerValue getOffset(IRStructField* field)
    {
        IRIntegerValue offset = 0;
        if (getOptions().shouldUseCLayout())
        {
            Slang::getOffset(
                getOptions(),
                IRTypeLayoutRules::get(IRTypeLayoutRuleName::C),
                field,
                &offset);
        }
        else if (getOptions().shouldUseScalarLayout())
        {
            Slang::getOffset(
                getOptions(),
                IRTypeLayoutRules::get(IRTypeLayoutRuleName::Scalar),
                field,
                &offset);
        }
        else
        {
            auto structType = as<IRStructType>(field->getParent());
            UInt index = 0;
            for (auto ff : structType->getFields())
            {
                if(ff == field)
                    break;
                index++;
            }
            auto llvmType = llvm::cast<llvm::StructType>(ensureType(structType));
            auto dataLayout = targetMachine->createDataLayout();
            const llvm::StructLayout* llvmStructLayout = dataLayout.getStructLayout(llvmType);
            offset = llvmStructLayout->getElementOffset(index);
        }
        return offset;
    }

    IRSizeAndAlignment getSizeAndAlignment(IRInst* val)
    {
        if (auto type = as<IRType>(val))
        {
            IRSizeAndAlignment elementSizeAlignment;

            if (getOptions().shouldUseCLayout())
            {
                Slang::getSizeAndAlignment(
                    getOptions(),
                    IRTypeLayoutRules::get(IRTypeLayoutRuleName::C),
                    type,
                    &elementSizeAlignment);
            }
            else if (getOptions().shouldUseScalarLayout())
            {
                Slang::getSizeAndAlignment(
                    getOptions(),
                    IRTypeLayoutRules::get(IRTypeLayoutRuleName::Scalar),
                    type,
                    &elementSizeAlignment);
            }
            else
            {
                // Align according to LLVM rules for maximum performance.
                // We can't always use this, as the llvmType may be packed and
                // specify an alignment of 1 for itself, even though we have
                // better guarantees.
                auto llvmType = ensureType(type);
                auto dataLayout = targetMachine->createDataLayout();
                elementSizeAlignment.alignment = dataLayout.getABITypeAlign(llvmType).value();
                elementSizeAlignment.size = dataLayout.getTypeStoreSize(llvmType);
            }

            return elementSizeAlignment;
        }
        else return getSizeAndAlignment(val->getDataType());
    }

    llvm::Value* emitAlloca(IRType* type, size_t count = 1)
    {
        auto llvmType = ensureType(type);
        IRSizeAndAlignment sizeAlign = getSizeAndAlignment(type);

        return llvmBuilder->Insert(new llvm::AllocaInst(
            llvmType, 0, llvmBuilder->getInt32(count), llvm::Align(sizeAlign.alignment)
        ));
    }

    llvm::Value* emitGetElementPtr(llvm::Value* ptr, llvm::Value* indexInst, IRType* aggregateType, IRType*& elementType)
    {
        if(auto structType = as<IRStructType>(aggregateType))
        {
            // 'index' must be constant for struct types.
            int64_t index = 0;
            if (auto intLit = llvm::cast<llvm::ConstantInt>(indexInst))
            {
                index = intLit->getValue().getLimitedValue();
            }
            else
            {
                SLANG_ASSERT_FAILURE("GetElement on structs only supports constant indices on LLVM");
            }

            elementType = findStructField(structType, index)->getFieldType();
            UInt llvmIndex = mapStructIndexToLLVM(structType, index);

            llvm::Value* indices[2] = {
                llvmBuilder->getInt32(0),
                llvmBuilder->getInt32(llvmIndex),
            };
            return llvmBuilder->CreateGEP(ensureType(aggregateType), ptr, indices);
        }
        else if(auto arrayType = as<IRArrayType>(aggregateType))
        {
            elementType = arrayType->getElementType();
        }
        else if(auto vectorType = as<IRVectorType>(aggregateType))
        {
            elementType = vectorType->getElementType();
        }
        else
        {
            SLANG_ASSERT_FAILURE("Unhandled type for GetElementPtr!");
        }

        llvm::Value* indices[2] = {
            llvmBuilder->getInt32(0),
            indexInst
        };
        return llvmBuilder->CreateGEP(ensureType(aggregateType), ptr, indices);
    }

    llvm::Value* emitLoad(llvm::Value* llvmPtr, IRType* srcType, bool isVolatile = false)
    {
        switch(srcType->getOp())
        {
        case kIROp_ArrayType:
        case kIROp_StructType:
            {
                llvm::Value* llvmVar = emitAlloca(srcType);
                IRSizeAndAlignment elementSizeAlignment = getSizeAndAlignment(srcType);

                // Pointer-to-pointer copy, so generate inline memcpy.
                llvmBuilder->CreateMemCpyInline(
                    llvmVar,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    llvmPtr,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    llvmBuilder->getInt32(elementSizeAlignment.size),
                    isVolatile
                );
                return llvmVar;
            }
        default:
            {
                auto llvmType = ensureType(srcType);
                return llvmBuilder->CreateLoad(llvmType, llvmPtr, isVolatile);
            }
        }
    }

    llvm::Value* emitLoad(IRInst* srcPtr, bool isVolatile = false)
    {
        auto ptrType = as<IRPtrTypeBase>(srcPtr->getDataType());
        auto valueType = ptrType->getValueType();

        auto llvmPtr = findValue(srcPtr);
        return emitLoad(llvmPtr, valueType, isVolatile);
    }

    // All CreateStore calls must be bottlenecked here.
    llvm::Value* emitStore(llvm::Value* llvmPtr, IRInst* srcVal, bool isVolatile = false)
    {
        auto llvmVal = findValue(srcVal);

        switch(srcVal->getDataType()->getOp())
        {
        case kIROp_ArrayType:
        case kIROp_StructType:
            {
                // Arrays and struct values are always represented with an alloca
                // pointer.
                SLANG_ASSERT(llvmVal->getType()->isPointerTy());

                IRSizeAndAlignment elementSizeAlignment = getSizeAndAlignment(srcVal);

                // Pointer-to-pointer copy, so generate inline memcpy.
                return llvmBuilder->CreateMemCpyInline(
                    llvmPtr,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    llvmVal,
                    llvm::MaybeAlign(elementSizeAlignment.alignment),
                    llvmBuilder->getInt32(elementSizeAlignment.size),
                    isVolatile
                );
            }
        default:
            return llvmBuilder->CreateStore(llvmVal, llvmPtr, isVolatile);
        }
    }

    void declareAllocaDebugVar(llvm::StringRef name, llvm::Value* llvmVar, IRType* type)
    {
        if (!debug)
            return;

        auto varType = ensureDebugType(type);

        llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
        auto debugVar = llvmDebugBuilder.createAutoVariable(
            debugScopeStack.getLast(), name, loc->getFile(), loc->getLine(),
            varType, getOptions().getOptimizationLevel() == OptimizationLevel::None
        );
        llvmDebugBuilder.insertDeclare(
            llvmVar,
            llvm::cast<llvm::DILocalVariable>(debugVar),
            llvmDebugBuilder.createExpression(),
            loc,
            llvmBuilder->GetInsertBlock()
        );
    }

    // Caution! This is only for emitting things which are considered
    // instructions in LLVM! It won't work for IRBlocks, IRFuncs & such.
    llvm::Value* emitLLVMInstruction(IRInst* inst, llvm::Value* storeOnReturn)
    {
        llvm::Value* llvmInst = nullptr;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
        case kIROp_PtrLit:
            llvmInst = maybeEnsureConstant(inst);
            break;

        case kIROp_Less:
        case kIROp_Leq:
        case kIROp_Eql:
        case kIROp_Neq:
        case kIROp_Greater:
        case kIROp_Geq:
            llvmInst = _emitCompare(inst);
            break;

        case kIROp_Add:
        case kIROp_Sub:
        case kIROp_Mul:
        case kIROp_Div:
        case kIROp_IRem:
        case kIROp_FRem:
        case kIROp_Neg:
        case kIROp_Not:
        case kIROp_And:
        case kIROp_Or:
        case kIROp_BitNot:
        case kIROp_BitAnd:
        case kIROp_BitOr:
        case kIROp_BitXor:
        case kIROp_Rsh:
        case kIROp_Lsh:
            llvmInst = _emitArithmetic(inst);
            break;

        case kIROp_Return:
            {
                IRReturn* retInst = static_cast<IRReturn*>(inst);
                auto retVal = retInst->getVal();
                if (retVal->getOp() == kIROp_VoidLit)
                {
                    llvmInst = llvmBuilder->CreateRetVoid();
                }
                else if (storeOnReturn)
                {
                    emitStore(storeOnReturn, retVal);
                    llvmInst = llvmBuilder->CreateRetVoid();
                }
                else
                {
                    llvmInst = llvmBuilder->CreateRet(findValue(retVal));
                }
            }
            break;

        case kIROp_Var:
            {
                auto var = static_cast<IRVar*>(inst);
                auto ptrType = var->getDataType();

                llvm::Value* llvmVar = emitAlloca(ptrType->getValueType());

                llvm::StringRef name;
                if (maybeGetName(&name, inst))
                {
                    llvmVar->setName(name);
                    // TODO: This may be a bit of a hack. DebugVar fails to get
                    // linked to the actual Var sometimes, which is why we do
                    // this :/
                    declareAllocaDebugVar(name, llvmVar, ptrType->getValueType());
                }

                llvmInst = llvmVar;
            }
            break;

        case kIROp_Loop:
        case kIROp_UnconditionalBranch:
            {
                auto branch = as<IRUnconditionalBranch>(inst);
                auto llvmTarget = llvm::cast<llvm::BasicBlock>(findValue(branch->getTargetBlock()));
                llvmInst = llvmBuilder->CreateBr(llvmTarget);
            }
            break;

        case kIROp_IfElse:
            {
                auto ifelseInst = static_cast<IRIfElse*>(inst);
                auto trueBlock = llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getTrueBlock()));
                auto falseBlock = llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getFalseBlock()));
                auto cond = findValue(ifelseInst->getCondition());
                llvmInst = llvmBuilder->CreateCondBr(cond, trueBlock, falseBlock);
            }
            break;

        case kIROp_Switch:
            {
                auto switchInst = static_cast<IRSwitch*>(inst);
                auto llvmMergeBlock = llvm::cast<llvm::BasicBlock>(findValue(switchInst->getBreakLabel()));
                auto llvmCondition = findValue(switchInst->getCondition());

                auto llvmSwitch = llvmBuilder->CreateSwitch(llvmCondition, llvmMergeBlock, switchInst->getCaseCount());
                for (UInt c = 0; c < switchInst->getCaseCount(); c++)
                {
                    auto value = switchInst->getCaseValue(c);
                    auto intLit = as<IRIntLit>(value);
                    SLANG_ASSERT(intLit);

                    auto llvmCaseBlock = llvm::cast<llvm::BasicBlock>(findValue(switchInst->getCaseLabel(c)));
                    auto llvmCaseValue = llvm::cast<llvm::ConstantInt>(maybeEnsureConstant(intLit));

                    llvmSwitch->addCase(llvmCaseValue, llvmCaseBlock);
                }
                llvmInst = llvmSwitch;
            }
            break;

        case kIROp_Store:
            {
                auto storeInst = static_cast<IRStore*>(inst);
                llvmInst = emitStore(findValue(storeInst->getPtr()), storeInst->getVal(), isVolatile(storeInst->getPtr()));
            }
            break;

        case kIROp_Load:
            {
                auto loadInst = static_cast<IRLoad*>(inst);
                llvmInst = emitLoad(loadInst->getPtr(), isVolatile(loadInst->getPtr()));
            }
            break;

        case kIROp_MakeArray:
        case kIROp_MakeStruct:
            {
                llvmInst = emitAlloca(inst->getDataType());
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    IRType* elementType = nullptr;
                    llvm::Value* ptr = emitGetElementPtr(llvmInst, llvmBuilder->getInt32(aa), inst->getDataType(), elementType);
                    emitStore(ptr, inst->getOperand(aa));
                }
            }
            break;

        case kIROp_MakeVector:
            llvmInst = maybeEnsureConstant(inst);
            if (!llvmInst)
            {
                auto llvmType = ensureType(inst->getDataType());
                llvmInst = llvm::PoisonValue::get(llvmType);
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    llvmInst = llvmBuilder->CreateInsertElement(llvmInst, findValue(inst->getOperand(aa)), aa);
                }
            }
            break;

        case kIROp_Swizzle:
            {
                auto swizzleInst = static_cast<IRSwizzle*>(inst);
                auto baseInst = swizzleInst->getBase();
                if (swizzleInst->getElementCount() == 1)
                {
                    llvmInst = llvmBuilder->CreateExtractElement(findValue(baseInst), findValue(swizzleInst->getElementIndex(0)));
                }
                else
                {
                    List<int> mask;
                    mask.reserve(swizzleInst->getElementCount());
                    for (UInt i = 0; i < swizzleInst->getElementCount(); ++i)
                    {
                        int val = as<IRIntLit>(swizzleInst->getElementIndex(i))->getValue();
                        mask.add(val);
                    }
                    llvmInst = llvmBuilder->CreateShuffleVector(findValue(baseInst), llvm::ArrayRef<int>(mask.begin(), mask.end()));
                }
            }
            break;

        case kIROp_IntCast:
            {
                auto llvmValue = findValue(inst->getOperand(0));

                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();
                auto fromType = getVectorOrCoopMatrixElementType(fromTypeV);
                auto toType = getVectorOrCoopMatrixElementType(toTypeV);

                auto fromInfo = getIntTypeInfo(fromType);
                auto toInfo = getIntTypeInfo(toType);

                if (fromInfo.width == toInfo.width)
                {
                    // LLVM integers are sign-ambiguous, so if the width is the
                    // same, there's nothing to do.
                    llvmInst = llvmValue;
                }
                else if(as<IRBoolType>(toType))
                {
                    llvmInst = llvmBuilder->CreateCmp(
                        llvm::CmpInst::Predicate::ICMP_NE,
                        llvmValue,
                        llvm::ConstantInt::get(ensureType(fromType), 0)
                    );
                }
                else
                {
                    llvm::Instruction::CastOps cast = llvm::Instruction::CastOps::Trunc;
                    if (toInfo.width > fromInfo.width)
                    {
                        // Source is signed, so sign extend.
                        cast = fromInfo.isSigned ?
                            llvm::Instruction::CastOps::SExt :
                            llvm::Instruction::CastOps::ZExt;
                    }
                    llvmInst = llvmBuilder->CreateCast(cast, llvmValue, ensureType(toTypeV));
                }
            }
            break;

        case kIROp_FloatCast:
            {
                auto llvmValue = findValue(inst->getOperand(0));

                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();

                auto llvmFromType = ensureType(getVectorOrCoopMatrixElementType(fromTypeV));
                auto llvmToType = ensureType(getVectorOrCoopMatrixElementType(toTypeV));

                auto fromSize = llvmFromType->getScalarSizeInBits();
                auto toSize = llvmToType->getScalarSizeInBits();

                if (fromSize == toSize)
                {
                    llvmInst = llvmValue;
                }
                else
                {
                    llvmInst = llvmBuilder->CreateCast(
                        fromSize < toSize ? llvm::Instruction::CastOps::FPExt : llvm::Instruction::CastOps::FPTrunc,
                        llvmValue, llvmToType
                    );
                }
            }
            break;

        case kIROp_CastIntToFloat:
            {
                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    isSignedType(fromTypeV) ?
                        llvm::Instruction::CastOps::SIToFP : 
                        llvm::Instruction::CastOps::UIToFP,
                    findValue(inst->getOperand(0)),
                    ensureType(toTypeV)
                );
            }
            break;

        case kIROp_CastFloatToInt:
            {
                auto fromTypeV = inst->getOperand(0)->getDataType();
                auto fromType = getVectorOrCoopMatrixElementType(fromTypeV);
                auto toTypeV = inst->getDataType();
                auto toType = getVectorOrCoopMatrixElementType(toTypeV);
                auto llvmValue = findValue(inst->getOperand(0));

                if(as<IRBoolType>(toType))
                {
                    llvmInst = llvmBuilder->CreateCmp(
                        llvm::CmpInst::Predicate::FCMP_UNE,
                        llvmValue,
                        llvm::ConstantInt::get(ensureType(fromType), 0)
                    );
                }
                else
                {
                    llvmInst = llvmBuilder->CreateCast(
                        isSignedType(toTypeV) ?
                            llvm::Instruction::CastOps::FPToSI :
                            llvm::Instruction::CastOps::FPToUI,
                        llvmValue,
                        ensureType(toTypeV)
                    );
                }
            }
            break;

        case kIROp_CastPtrToInt:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    llvm::Instruction::CastOps::PtrToInt,
                    findValue(fromValue),
                    ensureType(toTypeV)
                );
            }
            break;

        case kIROp_CastPtrToBool:
            {
                auto fromValue = inst->getOperand(0);
                llvmInst = llvmBuilder->CreateIsNotNull(findValue(fromValue));
            }
            break;

        case kIROp_CastIntToPtr:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    llvm::Instruction::CastOps::IntToPtr,
                    findValue(fromValue),
                    ensureType(toTypeV)
                );
            }
            break;

        case kIROp_PtrCast:
        case kIROp_BitCast:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder->CreateCast(
                    llvm::Instruction::CastOps::BitCast,
                    findValue(fromValue),
                    ensureType(toTypeV)
                );
            }
            break;

        case kIROp_FieldAddress:
            {
                auto fieldAddressInst = static_cast<IRFieldAddress*>(inst);
                auto base = fieldAddressInst->getBase();
                auto key = as<IRStructKey>(fieldAddressInst->getField());

                IRStructType* baseStructType = nullptr;
                if (auto ptrLikeType = as<IRPointerLikeType>(base->getDataType()))
                {
                    baseStructType = as<IRStructType>(ptrLikeType->getElementType());
                }
                else if (auto ptrType = as<IRPtrTypeBase>(base->getDataType()))
                {
                    baseStructType = as<IRStructType>(ptrType->getValueType());
                }

                UInt index = getStructIndexByKey(baseStructType, key);

                if (as<IRDebugVar>(base))
                {
                    // This is emitted to annotate member accesses of structs.
                    llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();

                    llvm::StringRef name = "";
                    maybeGetName(&name, inst);

                    IRStructField* field = findStructField(baseStructType, index);

                    variableDebugInfoMap[inst] = {
                        llvmDebugBuilder.createAutoVariable(
                            debugScopeStack.getLast(), name, loc->getFile(), loc->getLine(),
                            ensureDebugType(field->getFieldType()),
                            getOptions().getOptimizationLevel() == OptimizationLevel::None
                        ),
                        false,
                        false
                    };
                    return nullptr;
                }

                index = mapStructIndexToLLVM(baseStructType, index);

                auto llvmStructType = ensureType(baseStructType);
                auto llvmBase = findValue(base);
                llvm::Value* idx[2] = {
                    llvmBuilder->getInt32(0),
                    llvmBuilder->getInt32(index)
                };

                llvmInst = llvmBuilder->CreateGEP(llvmStructType, llvmBase, idx);
            }
            break;

        case kIROp_FieldExtract:
            {
                auto fieldExtractInst = static_cast<IRFieldExtract*>(inst);
                auto structType = as<IRStructType>(fieldExtractInst->getBase()->getDataType());
                auto llvmBase = findValue(fieldExtractInst->getBase());
                unsigned idx = mapStructIndexToLLVM(
                    structType,
                    getStructIndexByKey(structType, as<IRStructKey>(fieldExtractInst->getField())));

                IRType* elementType = nullptr;
                llvm::Value* ptr = emitGetElementPtr(llvmBase, llvmBuilder->getInt32(idx), structType, elementType);
                llvmInst = emitLoad(ptr, elementType);
            }
            break;

        case kIROp_GetElementPtr:
            {
                auto gepInst = static_cast<IRGetElementPtr*>(inst);
                auto baseInst = gepInst->getBase();
                auto indexInst = gepInst->getIndex();

                IRType* baseType = nullptr;
                if (auto ptrType = as<IRPtrTypeBase>(baseInst->getDataType()))
                {
                    baseType = ptrType->getValueType();
                }

                IRType* elementType = nullptr;
                llvmInst = emitGetElementPtr(
                    findValue(baseInst),
                    findValue(indexInst),
                    baseType,
                    elementType
                );
            }
            break;

        case kIROp_GetElement:
            {
                auto geInst = static_cast<IRGetElement*>(inst);
                auto baseInst = geInst->getBase();
                auto indexInst = geInst->getIndex();

                auto llvmVal = findValue(baseInst);

                auto baseTy = baseInst->getDataType();
                if (as<IRVectorType>(baseTy))
                {
                    // For vectors, we can use extractelement
                    llvmInst = llvmBuilder->CreateExtractElement(llvmVal, findValue(indexInst));
                }
                else if(as<IRArrayType>(baseTy) || as<IRStructType>(baseTy))
                {
                    // emitGEP + emitLoad.
                    SLANG_ASSERT(llvmVal->getType()->isPointerTy());
                    IRType* elementType = nullptr;
                    llvm::Value* ptr = emitGetElementPtr(llvmVal, findValue(indexInst), baseTy, elementType);
                    llvmInst = emitLoad(ptr, elementType);
                }
            }
            break;

        case kIROp_Call:
            {
                auto callInst = static_cast<IRCall*>(inst);
                auto llvmFunc = llvm::cast<llvm::Function>(findValue(callInst->getCallee()));

                List<llvm::Value*> args;

                for(IRInst* arg : callInst->getArgsList())
                {
                    args.add(findValue(arg));
                }

                auto returnType = ensureType(inst->getDataType());
                llvm::Value* allocValue = nullptr;
                // If attempting to return an aggregate, turn it into an extra
                // output parameter that is passed with a pointer.
                if (returnType->isAggregateType())
                {
                    allocValue = emitAlloca(inst->getDataType());
                    args.add(allocValue);
                }
                auto returnVal = llvmBuilder->CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
                llvmInst = allocValue ? allocValue : returnVal;
            }
            break;

        case kIROp_Printf:
            {
                // This function comes from the prelude.
                auto llvmFunc = cast<llvm::Function>(llvmModule.getNamedValue("printf"));
                SLANG_ASSERT(llvmFunc);

                List<llvm::Value*> args;
                args.add(findValue(inst->getOperand(0)));
                if (inst->getOperandCount() == 2)
                {
                    auto operand = inst->getOperand(1);
                    if (auto makeStruct = as<IRMakeStruct>(operand))
                    {
                        // Flatten the tuple resulting from the variadic pack.
                        for (UInt bb = 0; bb < makeStruct->getOperandCount(); ++bb)
                        {
                            auto llvmValue = findValue(makeStruct->getOperand(bb));
                            auto valueType = llvmValue->getType();

                            if (valueType->isFloatingPointTy() && valueType->getScalarSizeInBits() < 64)
                            {
                                // Floats need to get up-casted to at least f64
                                llvmValue = llvmBuilder->CreateCast(
                                    llvm::Instruction::CastOps::FPExt,
                                    llvmValue,
                                    llvm::Type::getDoubleTy(llvmContext)
                                );
                            }
                            else if (valueType->isIntegerTy() && valueType->getScalarSizeInBits() < 32)
                            {
                                // Ints are upcasted to at least i32.
                                llvmValue = llvmBuilder->CreateCast(
                                    llvm::Instruction::CastOps::SExt,
                                    llvmValue,
                                    llvm::Type::getInt32Ty(llvmContext)
                                );
                            }
                            args.add(llvmValue);
                        }
                    }
                }

                llvmInst = llvmBuilder->CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
            }
            break;

        case kIROp_DebugVar:
            if (debug)
            {
                auto debugVarInst = static_cast<IRDebugVar*>(inst);

                auto ptrType = as<IRPtrType>(debugVarInst->getDataType());
                auto varType = ensureDebugType(ptrType->getValueType());

                auto file = sourceDebugInfo.getValue(debugVarInst->getSource());
                auto line = getIntVal(debugVarInst->getLine());
                IRInst* argIndex = debugVarInst->getArgIndex();
                llvm::StringRef name;
                maybeGetName(&name, inst);

                if (argIndex)
                {
                    variableDebugInfoMap[inst] = {
                        llvmDebugBuilder.createParameterVariable(
                            debugScopeStack.getLast(), name, getIntVal(argIndex), file,
                            line, varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None
                        ),
                        false,
                        false
                    };
                }
                else
                {
                    variableDebugInfoMap[inst] = {
                        llvmDebugBuilder.createAutoVariable(
                            debugScopeStack.getLast(), name, file, line, varType,
                            getOptions().getOptimizationLevel() == OptimizationLevel::None
                        ),
                        false,
                        false
                    };
                }
            }
            return nullptr;

        case kIROp_DebugValue:
            if (debug)
            {
                auto debugValueInst = static_cast<IRDebugValue*>(inst);
                VariableDebugInfo& debugInfo = variableDebugInfoMap.getValue(debugValueInst->getDebugVar());

                llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
                if (!loc)
                    loc = llvm::DILocation::get(llvmContext, debugInfo.debugVar->getLine(), 0, debugInfo.debugVar->getScope());

                llvm::Value* value = findValue(debugValueInst->getValue());
                llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(value);
                if (!debugInfo.attached && alloca)
                {
                    debugInfo.isStackVar = true;
                    llvmDebugBuilder.insertDeclare(
                        alloca,
                        llvm::cast<llvm::DILocalVariable>(debugInfo.debugVar),
                        llvmDebugBuilder.createExpression(),
                        loc,
                        llvmBuilder->GetInsertBlock()
                    );
                }
                else if(!debugInfo.isStackVar)
                {
                    llvmDebugBuilder.insertDbgValueIntrinsic(
                        findValue(debugValueInst->getValue()),
                        llvm::cast<llvm::DILocalVariable>(debugInfo.debugVar),
                        llvmDebugBuilder.createExpression(),
                        loc,
                        llvmBuilder->GetInsertBlock()
                    );
                }
                debugInfo.attached = true;
            }
            return nullptr;

        case kIROp_DebugLine:
            if (debug)
            {
                auto debugLineInst = static_cast<IRDebugLine*>(inst);

                //auto file = sourceDebugInfo.getValue(debugLineInst->getSource());
                auto line = getIntVal(debugLineInst->getLineStart());
                auto col = getIntVal(debugLineInst->getColStart());

                debugLineInst->getLineEnd();
                debugLineInst->getColStart();
                debugLineInst->getColEnd();

                llvmBuilder->SetCurrentDebugLocation(
                    llvm::DILocation::get(llvmContext, line, col, debugScopeStack.getLast())
                );
            }
            return nullptr;

        default:
            SLANG_UNEXPECTED("Unsupported instruction for LLVM target!");
            break;
        }

        SLANG_ASSERT(llvmInst);

        if(llvmInst)
            mapInstToLLVM[inst] = llvmInst;
        return llvmInst;
    }

    bool maybeGetName(llvm::StringRef* nameOut, IRInst* irInst)
    {
        UnownedStringSlice name;
        if (auto externCppDecoration = irInst->findDecoration<IRExternCppDecoration>())
        {
            name = externCppDecoration->getName();
        }
        else if (auto linkageDecoration = irInst->findDecoration<IRLinkageDecoration>())
        {
            name = linkageDecoration->getMangledName();
        }
        else if (auto nameDecor = irInst->findDecoration<IRNameHintDecoration>())
        {
            name = nameDecor->getName();
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

                List<llvm::Type*> paramTypes;
                for (UInt i = 0; i < funcType->getParamCount(); ++i)
                {
                    IRType* paramType = funcType->getParamType(i);
                    auto llvmType = ensureType(paramType);
                    // Aggregates are passed with a pointer.
                    if (llvmType->isAggregateType())
                        llvmType = llvm::PointerType::get(llvmContext, 0);
                    paramTypes.add(llvmType);
                }

                auto returnType = ensureType(funcType->getResultType());
                // If attempting to return an aggregate, turn it into an extra
                // output parameter that is passed with a pointer.
                if (returnType->isAggregateType())
                {
                    paramTypes.add(llvm::PointerType::get(llvmContext, 0));
                    returnType = llvm::Type::getVoidTy(llvmContext);
                }

                llvmType = llvm::FunctionType::get(
                    returnType,
                    llvm::ArrayRef(paramTypes.begin(), paramTypes.end()),
                    false);
            }
            break;
        default:
            SLANG_UNEXPECTED("Unsupported type for LLVM target!");
            break;
        }

        mapTypeToLLVM[type] = llvmType;
        return llvmType;
    }

    void findDebugLocation(IRInst* inst, llvm::DIFile*& file, llvm::DIScope*& scope, unsigned& line)
    {
        if (!inst || inst->getOp() == kIROp_ModuleInst)
        {
            scope = compileUnit;
            file = compileUnit->getFile();
            line = 0;
        }
        else if (auto debugLocation = inst->findDecoration<IRDebugLocationDecoration>())
        {
            file = sourceDebugInfo.getValue(debugLocation->getSource());
            // TODO: More detailed scope info.
            scope = file;
            line = getIntVal(debugLocation->getLine());
        }
        else findDebugLocation(inst->getParent(), file, scope, line);
    }

    llvm::DIType* ensureDebugType(IRType* type)
    {
        if (mapTypeToDebugLLVM.containsKey(type))
            return mapTypeToDebugLLVM.getValue(type);

        llvm::DIType* llvmType = nullptr;
#if SLANG_PTR_IS_32
        const int ptrSize = 32;
#else
        const int ptrSize = 64;
#endif
        switch (type->getOp())
        {
        case kIROp_VoidType:
            llvmType = llvmDebugBuilder.createUnspecifiedType("void");
            break;
        case kIROp_Int8Type:
            llvmType = llvmDebugBuilder.createBasicType("int8_t", 8, llvm::dwarf::DW_ATE_signed);
            break;
        case kIROp_UInt8Type:
            llvmType = llvmDebugBuilder.createBasicType("int8_t", 8, llvm::dwarf::DW_ATE_unsigned);
            break;
        case kIROp_BoolType:
            llvmType = llvmDebugBuilder.createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
            break;
        case kIROp_Int16Type:
            llvmType = llvmDebugBuilder.createBasicType("int16_t", 16, llvm::dwarf::DW_ATE_signed);
            break;
        case kIROp_UInt16Type:
            llvmType = llvmDebugBuilder.createBasicType("uint16_t", 16, llvm::dwarf::DW_ATE_unsigned);
            break;
        case kIROp_IntType:
            llvmType = llvmDebugBuilder.createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
            break;
        case kIROp_UIntType:
            llvmType = llvmDebugBuilder.createBasicType("uint", 32, llvm::dwarf::DW_ATE_unsigned);
            break;

        case kIROp_IntPtrType:
            llvmType = llvmDebugBuilder.createBasicType("intptr", ptrSize, llvm::dwarf::DW_ATE_signed);
            break;

        case kIROp_UIntPtrType:
            llvmType = llvmDebugBuilder.createBasicType("uintptr", ptrSize, llvm::dwarf::DW_ATE_unsigned);
            break;

        case kIROp_Int64Type:
            llvmType = llvmDebugBuilder.createBasicType("int64_t", 64, llvm::dwarf::DW_ATE_signed);
            break;

        case kIROp_UInt64Type:
            llvmType = llvmDebugBuilder.createBasicType("uint64_t", 64, llvm::dwarf::DW_ATE_unsigned);
            break;

        case kIROp_PtrType:
            {
                auto ptr = as<IRPtrType>(type);
                llvmType = llvmDebugBuilder.createPointerType(ensureDebugType(ptr->getValueType()), ptrSize);
            }
            break;

        case kIROp_NativePtrType:
            llvmType = llvmDebugBuilder.createBasicType("NativeRef", ptrSize, llvm::dwarf::DW_ATE_address);
            break;

        case kIROp_NativeStringType:
            llvmType = llvmDebugBuilder.createStringType("NativeString", ptrSize);
            break;

        case kIROp_OutType:
        case kIROp_InOutType:
        case kIROp_RefType:
        case kIROp_ConstRefType:
            {
                auto ptr = as<IRPtrTypeBase>(type);
                llvmType = llvmDebugBuilder.createReferenceType(llvm::dwarf::DW_TAG_reference_type, ensureDebugType(ptr->getValueType()), ptrSize);
            }
            break;

        case kIROp_HalfType:
            llvmType = llvmDebugBuilder.createBasicType("half", 16, llvm::dwarf::DW_ATE_float);
            break;

        case kIROp_FloatType:
            llvmType = llvmDebugBuilder.createBasicType("float", 32, llvm::dwarf::DW_ATE_float);
            break;

        case kIROp_DoubleType:
            llvmType = llvmDebugBuilder.createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
            break;

        case kIROp_VectorType:
            {
                auto vecType = static_cast<IRVectorType*>(type);
                auto elemCount = int(getIntVal(vecType->getElementCount()));
                llvm::DIType* elemType = ensureDebugType(vecType->getElementType());
                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(vecType);

                if (sizeAndAlignment.size < sizeAndAlignment.alignment)
                    sizeAndAlignment.size = sizeAndAlignment.alignment;

                llvm::Metadata *subscript = llvmDebugBuilder.getOrCreateSubrange(0, elemCount);
                llvm::DINodeArray subscriptArray = llvmDebugBuilder.getOrCreateArray(subscript);
                llvmType = llvmDebugBuilder.createVectorType(
                    sizeAndAlignment.size*8,
                    sizeAndAlignment.alignment*8,
                    elemType,
                    subscriptArray
                );
            }
            break;

        case kIROp_ArrayType:
            {
                auto arrayType = static_cast<IRArrayType*>(type);
                llvm::DIType* elemType = ensureDebugType(arrayType->getElementType());
                auto elemCount = int(getIntVal(arrayType->getElementCount()));

                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(arrayType);

                llvm::Metadata *subscript = llvmDebugBuilder.getOrCreateSubrange(0, elemCount);
                llvm::DINodeArray subscriptArray = llvmDebugBuilder.getOrCreateArray(subscript);
                llvmType = llvmDebugBuilder.createArrayType(sizeAndAlignment.size*8, sizeAndAlignment.alignment*8, elemType, subscriptArray);
            }
            break;

        case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(type);

                IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(structType);

                llvm::DILocation* loc = llvmBuilder->getCurrentDebugLocation();
                List<llvm::Metadata*> types;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    if (as<IRVoidType>(fieldType))
                        continue;
                    llvm::DIType* debugType = ensureDebugType(fieldType);

                    IRSizeAndAlignment sizeAndAlignment = getSizeAndAlignment(field->getFieldType());
                    IRIntegerValue offset = getOffset(field);

                    IRStructKey* key = field->getKey();
                    llvm::StringRef name;
                    if (auto nameDecor = key->findDecoration<IRNameHintDecoration>())
                    {
                        auto decorName = nameDecor->getName();
                        name = llvm::StringRef(decorName.begin(), decorName.getLength());
                    }
                    types.add(llvmDebugBuilder.createMemberType(
                        compileUnit, name, loc->getFile(), loc->getLine(),
                        sizeAndAlignment.size * 8, sizeAndAlignment.alignment * 8,
                        offset * 8,
                        llvm::DINode::FlagZero,
                        debugType
                    ));
                }
                llvm::DINodeArray fieldTypes = llvmDebugBuilder.getOrCreateArray(
                    llvm::ArrayRef<llvm::Metadata*>(types.begin(), types.end())
                );

                llvm::StringRef name;
                maybeGetName(&name, type);

                llvm::DIFile* file;
                llvm::DIScope* scope;
                unsigned line;
                findDebugLocation(type, file, scope, line);

                llvmType = llvmDebugBuilder.createStructType(
                    scope,
                    name,
                    file,
                    line,
                    sizeAndAlignment.size*8,
                    sizeAndAlignment.alignment*8,
                    llvm::DINode::FlagZero,
                    nullptr,
                    fieldTypes
                );
            }
            break;

        case kIROp_FuncType:
            {
                auto funcType = static_cast<IRFuncType*>(type);

                List<llvm::Metadata*> elements;
                elements.add(ensureDebugType(funcType->getResultType()));
                for (UInt i = 0; i < funcType->getParamCount(); ++i)
                {
                    IRType* paramType = funcType->getParamType(i);
                    elements.add(ensureDebugType(paramType));
                }

                llvmType = llvmDebugBuilder.createSubroutineType(
                    llvmDebugBuilder.getOrCreateTypeArray(
                        llvm::ArrayRef<llvm::Metadata*>(elements.begin(), elements.end()))
                );
            }
            break;

        default:
            {
                llvm::StringRef name;
                maybeGetName(&name, type);
                llvmType = llvmDebugBuilder.createUnspecifiedType(name);
            }
            break;
        }
        mapTypeToDebugLLVM[type] = llvmType;
        return llvmType;
    }

    llvm::GlobalValue::LinkageTypes getLinkageType(IRInst* inst)
    {
        for (auto decor : inst->getDecorations())
        {
            switch (decor->getOp())
            {
            case kIROp_ExportDecoration:
            case kIROp_DownstreamModuleExportDecoration:
            case kIROp_HLSLExportDecoration:
            case kIROp_DllExportDecoration:
                {
                    return llvm::GlobalValue::ExternalLinkage;
                }
            default:
                break;
            }
        }
        return llvm::GlobalValue::PrivateLinkage;
    }

    llvm::DISubprogram* ensureDebugFunc(IRGlobalValueWithCode* func, IRDebugFunction* debugFunc)
    {
        if (functionDebugInfo.containsKey(func))
        {
            return functionDebugInfo[func];
        }
        llvm::DIFile* file = sourceDebugInfo.getValue(debugFunc->getFile());

        llvm::StringRef name = getStringLitAsLLVMString(debugFunc->getName());
        llvm::StringRef mangledName = name;
        maybeGetName(&mangledName, func);

        int line = getIntVal(debugFunc->getLine());

        auto sp = llvmDebugBuilder.createFunction(
            file,
            name,
            mangledName,
            file,
            line,
            llvm::cast<llvm::DISubroutineType>(ensureDebugType(as<IRType>(debugFunc->getDebugType()))),
            line,
            llvm::DINode::FlagPrototyped,
            llvm::DISubprogram::SPFlagDefinition
        );
        functionDebugInfo[func] = sp;
        return sp;
    }

    llvm::Function* ensureFuncDecl(IRFunc* func)
    {
        if (mapInstToLLVM.containsKey(func))
            return llvm::cast<llvm::Function>(mapInstToLLVM.getValue(func));

        auto funcType = static_cast<IRFuncType*>(func->getDataType());
        llvm::FunctionType* llvmFuncType = llvm::cast<llvm::FunctionType>(ensureType(funcType));

        llvm::Function* llvmFunc = llvm::Function::Create(
            llvmFuncType,
            getLinkageType(func),
            "", // Name is conditionally set below.
            llvmModule
        );

        llvm::StringRef name;
        if (maybeGetName(&name, func))
            llvmFunc->setName(name);

        UInt i = 0;
        for (auto pp = func->getFirstParam(); pp; pp = pp->getNextParam(), ++i)
        {
            auto llvmArg = llvmFunc->getArg(i);

            if (as<IROutType>(funcType->getParamType(i)))
                llvmArg->addAttr(llvm::Attribute::WriteOnly);
            if (as<IRConstRefType>(funcType->getParamType(i)))
                llvmArg->addAttr(llvm::Attribute::ReadOnly);

            llvm::StringRef name;
            if (maybeGetName(&name, pp))
                llvmArg->setName(name);
            mapInstToLLVM[pp] = llvmArg;
        }

        // Attach attributes based on decorations!
        if (func->findDecoration<IRReadNoneDecoration>())
            llvmFunc->setOnlyAccessesArgMemory();
        else if (func->findDecoration<IRForceInlineDecoration>())
            llvmFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
        else if (func->findDecoration<IRNoInlineDecoration>())
            llvmFunc->addFnAttr(llvm::Attribute::AttrKind::NoInline);

        mapInstToLLVM[func] = llvmFunc;
        return llvmFunc;
    }

    llvm::StringRef getStringLitAsLLVMString(IRInst* inst)
    {
        auto source = as<IRStringLit>(inst)->getStringSlice();
        return llvm::StringRef(source.begin(), source.getLength());
    }

    llvm::Constant* maybeEnsureConstant(IRInst* inst)
    {
        if (mapInstToLLVM.containsKey(inst))
            return llvm::dyn_cast<llvm::Constant>(mapInstToLLVM.getValue(inst));

        llvm::Constant* llvmConstant = nullptr;

        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
            {
                auto litInst = static_cast<IRConstant*>(inst);
                IRBasicType* type = as<IRBasicType>(inst->getDataType());
                if (type)
                {
                    llvmConstant = llvm::ConstantInt::get(ensureType(type), litInst->value.intVal);
                }
                break;
            }
        case kIROp_FloatLit:
            {
                auto litInst = static_cast<IRConstant*>(inst);
                IRBasicType* type = as<IRBasicType>(inst->getDataType());
                if (type)
                {
                    llvmConstant = llvm::ConstantFP::get(ensureType(type), litInst->value.floatVal);
                }
            }
            break;
        case kIROp_StringLit:
            llvmConstant = llvmBuilder->CreateGlobalString(getStringLitAsLLVMString(inst));
            break;
        case kIROp_PtrLit:
            {
                auto ptrLit = static_cast<IRPtrLit*>(inst);
                IRPtrType* type = as<IRPtrType>(inst->getDataType());
                auto llvmType = llvm::cast<llvm::PointerType>(ensureType(type));
                if (ptrLit->getValue() == nullptr)
                {
                    llvmConstant = llvm::ConstantPointerNull::get(llvmType);
                }
                else
                {
                    llvmConstant = llvm::ConstantExpr::getIntToPtr(
                        llvm::ConstantInt::get(
                            llvm::IntegerType::get(llvmContext,
#if SLANG_PTR_IS_64
                                64
#else
                                32
#endif
                            ),
                            ((uintptr_t)ptrLit->getValue())),
                        llvmType
                    );
                }
            }
            break;
        case kIROp_MakeVector:
            {
                List<llvm::Constant*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEnsureConstant(inst->getOperand(aa));
                    if (!constVal)
                        return nullptr;
                    values.add(constVal);
                }
                llvmConstant = llvm::ConstantVector::get(llvm::ArrayRef(values.begin(), values.end()));
            }
            break;
        case kIROp_MakeArray:
            {
                auto llvmType = ensureType(inst->getDataType());
                List<llvm::Constant*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEnsureConstant(inst->getOperand(aa));
                    if (!constVal)
                        return nullptr;
                    values.add(constVal);
                }
                llvmConstant = llvm::ConstantArray::get(
                    llvm::cast<llvm::ArrayType>(llvmType),
                    llvm::ArrayRef(values.begin(), values.end()));
            }
            break;
        case kIROp_MakeStruct:
            {
                auto llvmType = ensureType(inst->getDataType());
                List<llvm::Constant*> values;
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    auto constVal = maybeEnsureConstant(inst->getOperand(aa));
                    if (!constVal)
                        return nullptr;
                    values.add(constVal);
                }
                llvmConstant = llvm::ConstantStruct::get(
                    llvm::cast<llvm::StructType>(llvmType),
                    llvm::ArrayRef(values.begin(), values.end()));
            }
            break;
        default:
            break;
        }

        if (llvmConstant)
            mapInstToLLVM[inst] = llvmConstant;
        return llvmConstant;
    }

    llvm::Value* ensureGlobalVarDecl(IRGlobalVar* var)
    {
        if (mapInstToLLVM.containsKey(var))
            return mapInstToLLVM.getValue(var);

        IRPtrType* ptrType = var->getDataType();
        auto varType = ensureType(ptrType->getValueType());
        // The global vars are never emitted as constant, constants are always
        // inlined into where they're needed.
        llvm::GlobalVariable* llvmVar = new llvm::GlobalVariable(varType, false, getLinkageType(var));

        llvm::StringRef name;
        if (maybeGetName(&name, var))
            llvmVar->setName(name);

        llvmModule.insertGlobalVariable(llvmVar);
        mapInstToLLVM[var] = llvmVar;
        return llvmVar;
    }

    llvm::Constant* getZeroInitializer(llvm::Type* type)
    {
        if (type->isVectorTy())
        {
            auto vectorType = llvm::cast<llvm::VectorType>(type);
            return llvm::ConstantVector::getSplat(
                vectorType->getElementCount(),
                getZeroInitializer(vectorType->getElementType())
            );
        }
        else if (type->isPointerTy())
        {
            return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(type));
        }
        else if (type->isFloatingPointTy())
        {
            return llvm::ConstantFP::getZero(type);
        }
        else if (type->isIntegerTy())
        {
            return llvm::ConstantInt::get(type, 0);
        }
        return llvm::ConstantAggregateZero::get(type);
    }

    void emitGlobalVarCtor(IRGlobalVar* var)
    {
        llvm::GlobalVariable* llvmVar = llvm::cast<llvm::GlobalVariable>(mapInstToLLVM.getValue(var));

        auto firstBlock = var->getFirstBlock();

        if(!firstBlock)
            return;

        if (auto returnInst = as<IRReturn>(firstBlock->getTerminator()))
        {
            // If the initializer is constant, we can emit that to the
            // variable directly.
            IRInst* val = returnInst->getVal();
            if (auto constantValue = maybeEnsureConstant(val))
            {
                // Easy case, it's just a constant in LLVM.
                llvmVar->setInitializer(constantValue);
                return;
            }
        }

        // Zero-initialize and add a global ctor.
        llvmVar->setInitializer(getZeroInitializer(llvmVar->getValueType()));

        llvm::FunctionType* ctorType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(llvmContext), {}, false);

        llvm::StringRef varName;
        std::string ctorName = "__slang_init_";
        maybeGetName(&varName, var);
        ctorName += varName;

        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());
        llvm::Function* llvmCtor = llvm::Function::Create(
            ctorType, llvm::GlobalValue::InternalLinkage, ctorName, llvmModule
        );

        emitGlobalValueWithCode(var, llvmCtor, llvmVar);

        llvm::Constant* ctorData[3] = {
            llvmBuilder->getInt32(globalCtors.getCount()),
            llvmCtor,
            llvmVar
        };

        llvm::Constant *ctorEntry = llvm::ConstantStruct::get(llvmCtorType,
            llvm::ArrayRef(ctorData, llvmCtorType->getNumElements()));

        globalCtors.add(ctorEntry);
    }

    void emitGlobalDebugInfo(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto debugSource = as<IRDebugSource>(inst))
            {
                auto filename = as<IRStringLit>(debugSource->getFileName())->getStringSlice();

                std::filesystem::path path(
                    std::string(filename.begin(), filename.getLength())
                );
                sourceDebugInfo[inst] = llvmDebugBuilder.createFile(
                    path.filename().c_str(),
                    path.parent_path().c_str(),
                    std::nullopt,
                    getStringLitAsLLVMString(debugSource->getSource())
                );
            }
        }
    }

    void emitGlobalDeclarations(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
                ensureFuncDecl(func);
            else if(auto globalVar = as<IRGlobalVar>(inst))
                ensureGlobalVarDecl(globalVar);
        }
    }

    // Using std::string as the LLVM API works with that. It's not to be
    // ingested by Slang.
    std::string expandIntrinsic(llvm::Function* llvmFunc, IRFunc* parentFunc, UnownedStringSlice intrinsicText)
    {
        std::string out;
        llvm::raw_string_ostream expanded(out);
        
        auto resultType = parentFunc->getResultType();

        char const* cursor = intrinsicText.begin();
        char const* end = intrinsicText.end();
        llvmFunc->print(expanded);

        while (out.size() > 0 && out.back() != '{')
            out.pop_back();

        expanded << "\n";

        while (cursor < end)
        {
            // Indicates the start of a 'special' sequence
            if (*cursor == '$')
            {
                cursor++;
                SLANG_RELEASE_ASSERT(cursor < end);
                char d = *cursor++;
                switch (d)
                {
                case 'T':
                    // Type of parameter
                    {
                        IRType* type = nullptr;
                        if (*cursor == 'R')
                        {
                            // Get the return type of the call
                            cursor++;
                            type = resultType;
                        }
                        else
                        {
                            UInt argIndex = parseNumber(cursor, end);
                            SLANG_RELEASE_ASSERT(argIndex < parentFunc->getParamCount());
                            type = parentFunc->getParamType(argIndex);
                        }

                        auto llvmType = ensureType(type);
                        llvmType->print(expanded);
                    }
                    break;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    // Function parameter.
                    {
                        --cursor;
                        UInt argIndex = parseNumber(cursor, end);
                        SLANG_RELEASE_ASSERT(argIndex < parentFunc->getParamCount());
                        IRParam* param = nullptr;

                        for (IRParam* p : parentFunc->getParams())
                        {
                            if (argIndex == 0)
                            {
                                param = p;
                                break;
                            }
                            argIndex--;
                        }

                        auto llvmParam = findValue(param);
                        llvmParam->print(expanded);
                    }
                    break;
                default:
                    SLANG_UNEXPECTED("bad format in intrinsic definition");
                    break;
                }
            }
            else
            {
                expanded << *cursor;
                cursor++;
            }
        }

        expanded << "\nret ";
        auto llvmResultType = ensureType(resultType);
        llvmResultType->print(expanded);

        if (as<IRVoidType>(resultType) == nullptr)
        {
            expanded << " %result";
        }

        expanded << "\n}\n";

        return out;
    }

    void emitGlobalValueWithCode(IRGlobalValueWithCode* code, llvm::Function* llvmFunc, llvm::Value* storeOnReturn)
    {
        llvmBuilder->SetCurrentDebugLocation(llvm::DebugLoc());

        auto func = as<IRFunc>(code);
        bool intrinsic = false;
        if (func)
        {
            UnownedStringSlice intrinsicDef;
            IRInst* intrinsicInst;
            intrinsic = Slang::findTargetIntrinsicDefinition(func, codeGenContext->getTargetReq()->getTargetCaps(), intrinsicDef, intrinsicInst);
            if (intrinsic)
            {
                llvm::BasicBlock::Create(llvmContext, "", llvmFunc);

                std::string llvmTextIR = expandIntrinsic(llvmFunc, func, intrinsicDef);
                std::string llvmFuncName = llvmFunc->getName().str();
                llvmFunc->eraseFromParent();

                // This function is defined by an intrinsic.
                llvm::SMDiagnostic diag;
                llvm::MemoryBufferRef buf(
                    llvm::StringRef(llvmTextIR),
                    llvm::StringRef("inline-llvm-ir")
                );
                if(llvm::parseAssemblyInto(buf, &llvmModule, &llvmSummaryIndex, diag))
                {
                    //auto msg = diag.getMessage();
                    //printf("%s\n", llvmTextIR.c_str());
                    //printf("%s\n", msg.str().c_str());
                    SLANG_UNEXPECTED("Failed to parse LLVM inline IR!");
                }

                llvmFunc = cast<llvm::Function>(llvmModule.getNamedValue(llvmFuncName));

                // Intrinsic functions usually do nothing other than call a single
                // instruction; we can just inline them by default.
                llvmFunc->addFnAttr(llvm::Attribute::AttrKind::AlwaysInline);
                mapInstToLLVM[func] = llvmFunc;
            }
        }

        llvm::DISubprogram* sp = nullptr;
        if (auto debugFuncDecoration = code->findDecoration<IRDebugFuncDecoration>(); debugFuncDecoration && debug)
        {
            auto debugFunc = as<IRDebugFunction>(debugFuncDecoration->getDebugFunc());
            sp = ensureDebugFunc(code, debugFunc);
        }

        if (sp != nullptr)
        {
            llvmFunc->setSubprogram(sp);
            debugScopeStack.add(sp);
        }

        if (!intrinsic)
        {
            // Create all blocks first
            for (auto irBlock : code->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(llvmContext, "", llvmFunc);
                mapInstToLLVM[irBlock] = llvmBlock;
            }

            // Then, fill in the blocks. Lucky for us, there appears to basically be
            // a 1:1 correspondence between Slang IR blocks and LLVM IR blocks, so
            // this is straightforward.
            for (auto irBlock : code->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::cast<llvm::BasicBlock>(mapInstToLLVM.getValue(irBlock));
                llvmBuilder->SetInsertPoint(llvmBlock);

                // Then, add the regular instructions.
                for (auto irInst: irBlock->getOrdinaryInsts())
                {
                    emitLLVMInstruction(irInst, storeOnReturn);
                }
            }
        }

        if (sp != nullptr)
            debugScopeStack.removeLast();
    }

    void emitFuncDefinition(IRFunc* func)
    {
        llvm::Function* llvmFunc = ensureFuncDecl(func);

        llvm::Value* storeArg = nullptr;
        if (ensureType(func->getResultType())->isAggregateType())
            storeArg = llvmFunc->getArg(0);
        emitGlobalValueWithCode(func, llvmFunc, storeArg);
    }

    void emitGlobalFunctions(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                if (isDefinition(func))
                    emitFuncDefinition(func);
            }
            else if(auto globalVar = as<IRGlobalVar>(inst))
            {
                emitGlobalVarCtor(globalVar);
            }
        }
    }

    void processModule(IRModule* irModule)
    {
        // Start by emitting all function declarations, so that the functions
        // can freely refer to each other later on.
        emitGlobalDebugInfo(irModule);
        emitGlobalDeclarations(irModule);
        emitGlobalFunctions(irModule);
    }

    // Optimizes the LLVM IR and destroys mapInstToLLVM.
    void optimize()
    {
        mapInstToLLVM.clear();

        llvmModule.setDataLayout(targetMachine->createDataLayout());
        llvmModule.setTargetTriple(targetMachine->getTargetTriple());

        llvm::LoopAnalysisManager loopAnalysisManager;
        llvm::FunctionAnalysisManager functionAnalysisManager;
        llvm::CGSCCAnalysisManager CGSCCAnalysisManager;
        llvm::ModuleAnalysisManager moduleAnalysisManager;

        llvm::PassBuilder passBuilder(targetMachine);
        passBuilder.registerModuleAnalyses(moduleAnalysisManager);
        passBuilder.registerCGSCCAnalyses(CGSCCAnalysisManager);
        passBuilder.registerFunctionAnalyses(functionAnalysisManager);
        passBuilder.registerLoopAnalyses(loopAnalysisManager);
        passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, CGSCCAnalysisManager, moduleAnalysisManager);

        llvm::OptimizationLevel llvmLevel = llvm::OptimizationLevel::O0;

        switch(getOptions().getOptimizationLevel())
        {
        case OptimizationLevel::None:
            llvmLevel = llvm::OptimizationLevel::O0;
            break;
        default:
        case OptimizationLevel::Default:
            llvmLevel = llvm::OptimizationLevel::O1;
            break;
        case OptimizationLevel::High:
            llvmLevel = llvm::OptimizationLevel::O2;
            break;
        case OptimizationLevel::Maximal:
            llvmLevel = llvm::OptimizationLevel::O3;
            break;
        }

        // Run the actual optimizations.
        llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(llvmLevel);
        modulePassManager.run(llvmModule, moduleAnalysisManager);
    }

    void finalize()
    {
        // Dump the global constructors array
        if (globalCtors.getCount() != 0)
        {
            auto ctorArrayType = llvm::ArrayType::get(llvmCtorType, globalCtors.getCount());
            auto ctorArray = llvm::ConstantArray::get(ctorArrayType, llvm::ArrayRef(globalCtors.begin(), globalCtors.end()));
            new llvm::GlobalVariable(
                llvmModule, ctorArrayType, false, llvm::GlobalValue::AppendingLinkage,
                ctorArray, "llvm.global_ctors"
            );
        }
        
        llvm::verifyModule(llvmModule, &llvm::errs());

        if (getOptions().getOptimizationLevel() != OptimizationLevel::None)
        {
            optimize();
        }

        if (debug)
        {
            llvmDebugBuilder.finalize();
        }
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
        // These must always be set when generating object code.
        llvmModule.setDataLayout(targetMachine->createDataLayout());
        llvmModule.setTargetTriple(targetMachine->getTargetTriple());

        BinaryLLVMOutputStream output(objectOut);

        llvm::legacy::PassManager pass;
        auto fileType = llvm::CodeGenFileType::ObjectFile;
        if (targetMachine->addPassesToEmitFile(pass, output, nullptr, fileType))
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
