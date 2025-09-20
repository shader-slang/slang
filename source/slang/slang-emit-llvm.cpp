#include "slang-emit-llvm.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "../core/slang-char-util.h"
#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/ModuleSummaryIndex.h>
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

struct LLVMEmitter
{
    llvm::LLVMContext llvmContext;
    llvm::IRBuilder<> llvmBuilder;
    llvm::Module llvmModule;
    llvm::ModuleSummaryIndex llvmSummaryIndex;

    // The LLVM value class is closest to Slang's IRInst, as it can represent
    // constants, instructions and functions, whereas llvm::Instruction only
    // handles instructions.
    Dictionary<IRInst*, llvm::Value*> mapInstToLLVM;
    Dictionary<IRType*, llvm::Type*> mapTypeToLLVM;

    CodeGenContext* codeGenContext;

    LLVMEmitter(CodeGenContext* codeGenContext)
        : llvmBuilder(llvmContext), llvmModule("module", llvmContext),
          llvmSummaryIndex(true),
          codeGenContext(codeGenContext)
    {
        auto session = codeGenContext->getSession();
        String prelude = session->getPreludeForLanguage(SourceLanguage::LLVM);

        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

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
        return llvmBuilder.CreateCmp(pred, findValue(inst->getOperand(0)), findValue(inst->getOperand(1)));
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
                return isFloat ? llvmBuilder.CreateFNeg(llvmValue) : llvmBuilder.CreateNeg(llvmValue);
            case kIROp_Not:
            case kIROp_BitNot:
                return llvmBuilder.CreateNot(llvmValue);
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
            return llvmBuilder.CreateBinOp(op, findValue(inst->getOperand(0)), findValue(inst->getOperand(1)));
        }
        else
        {
            SLANG_UNEXPECTED("Unexpected number of operands for arithmetic op");
        }

        return nullptr;
    }

    std::size_t findStructKeyIndex(IRStructType* irStruct, IRStructKey* irKey)
    {
        std::size_t index = 0;
        for (auto field : irStruct->getFields())
        {
            if(field->getKey() == irKey)
                return index;
            index++;
        }
        SLANG_UNEXPECTED("Requested key that doesn't exist in struct!");
    }

    // Caution! This is only for emitting things which are considered
    // instructions in LLVM! It won't work for IRBlocks, IRFuncs & such.
    llvm::Value* emitLLVMInstruction(IRInst* inst)
    {
        llvm::Value* llvmInst = nullptr;
        switch (inst->getOp())
        {
        case kIROp_IntLit:
        case kIROp_BoolLit:
        case kIROp_FloatLit:
        case kIROp_StringLit:
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
                    llvmInst = llvmBuilder.CreateRetVoid();
                }
                else
                {
                    llvmInst = llvmBuilder.CreateRet(findValue(retVal));
                }
            }
            break;

        case kIROp_Var:
            {
                auto var = static_cast<IRVar*>(inst);
                auto ptrType = var->getDataType();
                auto llvmType = ensureType(ptrType->getValueType());

                llvm::AllocaInst* llvmVar = llvmBuilder.CreateAlloca(llvmType);

                llvm::StringRef name;
                if (maybeGetName(&name, inst))
                    llvmVar->setName(name);

                llvmInst = llvmVar;
            }
            break;

        case kIROp_Loop:
        case kIROp_UnconditionalBranch:
            {
                auto branch = as<IRUnconditionalBranch>(inst);
                auto llvmTarget = llvm::cast<llvm::BasicBlock>(findValue(branch->getTargetBlock()));
                llvmInst = llvmBuilder.CreateBr(llvmTarget);
            }
            break;

        case kIROp_IfElse:
            {
                auto ifelseInst = static_cast<IRIfElse*>(inst);
                auto trueBlock = llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getTrueBlock()));
                auto falseBlock = llvm::cast<llvm::BasicBlock>(findValue(ifelseInst->getFalseBlock()));
                auto cond = findValue(ifelseInst->getCondition());
                llvmInst = llvmBuilder.CreateCondBr(cond, trueBlock, falseBlock);
            }
            break;

        case kIROp_Switch:
            {
                auto switchInst = static_cast<IRSwitch*>(inst);
                auto llvmMergeBlock = llvm::cast<llvm::BasicBlock>(findValue(switchInst->getBreakLabel()));
                auto llvmCondition = findValue(switchInst->getCondition());

                auto llvmSwitch = llvmBuilder.CreateSwitch(llvmCondition, llvmMergeBlock, switchInst->getCaseCount());
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
                auto llvmPtr = findValue(storeInst->getPtr());
                auto llvmVal = findValue(storeInst->getVal());
                // TODO: isVolatile
                llvmInst = llvmBuilder.CreateStore(llvmVal, llvmPtr);
            }
            break;

        case kIROp_Load:
            {
                auto loadInst = static_cast<IRLoad*>(inst);
                auto ptrType = as<IRPtrTypeBase>(loadInst->getPtr()->getDataType());
                auto llvmType = ensureType(ptrType->getValueType());
                auto llvmPtr = findValue(loadInst->getPtr());
                // TODO: isVolatile
                llvmInst = llvmBuilder.CreateLoad(llvmType, llvmPtr);
            }
            break;

        case kIROp_MakeArray:
        case kIROp_MakeStruct:
            {
                auto llvmType = ensureType(inst->getDataType());

                llvmInst = llvm::PoisonValue::get(llvmType);
                for (UInt aa = 0; aa < inst->getOperandCount(); ++aa)
                {
                    llvmInst = llvmBuilder.CreateInsertValue(llvmInst, findValue(inst->getOperand(aa)), aa);
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
                    llvmInst = llvmBuilder.CreateCmp(
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
                    llvmInst = llvmBuilder.CreateCast(cast, llvmValue, ensureType(toTypeV));
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
                    llvmInst = llvmBuilder.CreateCast(
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
                llvmInst = llvmBuilder.CreateCast(
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
                    llvmInst = llvmBuilder.CreateCmp(
                        llvm::CmpInst::Predicate::FCMP_UNE,
                        llvmValue,
                        llvm::ConstantInt::get(ensureType(fromType), 0)
                    );
                }
                else
                {
                    llvmInst = llvmBuilder.CreateCast(
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
                llvmInst = llvmBuilder.CreateCast(
                    llvm::Instruction::CastOps::PtrToInt,
                    findValue(fromValue),
                    ensureType(toTypeV)
                );
            }
            break;

        case kIROp_CastPtrToBool:
            {
                auto fromValue = inst->getOperand(0);
                llvmInst = llvmBuilder.CreateIsNotNull(findValue(fromValue));
            }
            break;

        case kIROp_CastIntToPtr:
            {
                auto fromValue = inst->getOperand(0);
                auto toTypeV = inst->getDataType();
                llvmInst = llvmBuilder.CreateCast(
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
                llvmInst = llvmBuilder.CreateCast(
                    llvm::Instruction::CastOps::BitCast,
                    findValue(fromValue),
                    ensureType(toTypeV)
                );
            }
            break;

        case kIROp_FieldExtract:
            {
                auto fieldExtractInst = static_cast<IRFieldExtract*>(inst);
                auto structType = as<IRStructType>(fieldExtractInst->getBase()->getDataType());
                auto llvmBase = findValue(fieldExtractInst->getBase());
                unsigned idx = findStructKeyIndex(structType, as<IRStructKey>(fieldExtractInst->getField()));
                llvmInst = llvmBuilder.CreateExtractValue(llvmBase, idx);
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

                std::size_t index = findStructKeyIndex(baseStructType, key);

                auto llvmStructType = ensureType(baseStructType);
                auto llvmBase = findValue(base);
                llvm::Value* idx[2] = {
                    llvmBuilder.getInt32(0),
                    llvmBuilder.getInt32(index)
                };

                llvmInst = llvmBuilder.CreateGEP(llvmStructType, llvmBase, idx);
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

                llvmInst = llvmBuilder.CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
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
                                llvmValue = llvmBuilder.CreateCast(
                                    llvm::Instruction::CastOps::FPExt,
                                    llvmValue,
                                    llvm::Type::getDoubleTy(llvmContext)
                                );
                            }
                            else if (valueType->isIntegerTy() && valueType->getScalarSizeInBits() < 32)
                            {
                                // Ints are upcasted to at least i32.
                                llvmValue = llvmBuilder.CreateCast(
                                    llvm::Instruction::CastOps::SExt,
                                    llvmValue,
                                    llvm::Type::getInt32Ty(llvmContext)
                                );
                            }
                            args.add(llvmValue);
                        }
                    }
                }

                llvmInst = llvmBuilder.CreateCall(llvmFunc, llvm::ArrayRef(args.begin(), args.end()));
            }
            break;

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
            break;
        }

        mapTypeToLLVM[type] = llvmType;
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
            // TODO: Add attributes to the params. We can determine some attrs
            // based on whether we've got an OutType, InOutType, RefType,
            // ConstRefType and so on. Not adding attributes is safe but
            // prevents many optimizations.
            auto llvmArg = llvmFunc->getArg(i);
            llvm::StringRef name;
            if (maybeGetName(&name, pp))
                llvmArg->setName(name);
            mapInstToLLVM[pp] = llvmArg;
        }

        mapInstToLLVM[func] = llvmFunc;
        return llvmFunc;
    }

    llvm::Constant* maybeEnsureConstant(IRInst* inst)
    {
        if (mapInstToLLVM.containsKey(inst))
            return llvm::cast<llvm::Constant>(mapInstToLLVM.getValue(inst));

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
            {
                auto litInst = static_cast<IRConstant*>(inst);
                llvmConstant = llvmBuilder.CreateGlobalString(
                    llvm::StringRef(
                        litInst->getStringSlice().begin(),
                        litInst->getStringSlice().getLength()
                    )
                );
            }
            break;
            // TODO: constant arrays, structs, vectors, matrices?
        default:
            break;
        }

        if (llvmConstant)
            mapInstToLLVM[inst] = llvmConstant;
        return llvmConstant;
    }

    llvm::Value* ensureGlobalVar(IRGlobalVar* var)
    {
        if (mapInstToLLVM.containsKey(var))
            return mapInstToLLVM.getValue(var);

        IRPtrType* ptrType = var->getDataType();
        auto varType = ensureType(ptrType->getValueType());
        // TODO: When is the global variable a constant?
        llvm::GlobalVariable* llvmVar = new llvm::GlobalVariable(varType, false, getLinkageType(var));

        if (auto firstBlock = var->getFirstBlock())
        {
            llvm::Constant* constantValue = nullptr;
            if (auto returnInst = as<IRReturn>(firstBlock->getTerminator()))
            {
                // If the initializer is constant, we can emit that to the
                // variable directly.
                IRInst* val = returnInst->getVal();
                constantValue = maybeEnsureConstant(val);
            }

            if (constantValue)
                llvmVar->setInitializer(constantValue);
            else
            {
                // TODO: Create a function that initializes the global variable,
                // and append that to llvm.global_ctors.
            }
        }

        llvm::StringRef name;
        if (maybeGetName(&name, var))
            llvmVar->setName(name);

        llvmModule.insertGlobalVariable(llvmVar);
        mapInstToLLVM[var] = llvmVar;
        return llvmVar;
    }

    void emitGlobalDeclarations(IRModule* irModule)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                ensureFuncDecl(func);
            }
            else if(auto globalVar = as<IRGlobalVar>(inst))
            {
                ensureGlobalVar(globalVar);
            }
        }
    }

    void emitPhi(IRParam* param)
    {
        IRBlock* parentBlock = as<IRBlock>(param->getParent());

        int predecessorCount = 0;

        for (auto use = parentBlock->firstUse; use; use = use->nextUse)
        {
            auto branchInst = as<IRUnconditionalBranch>(use->getUser());
            if (!branchInst)
                continue;
            if (branchInst->getTargetBlock() != parentBlock)
                continue;
            if (branchInst->getOp() != kIROp_UnconditionalBranch && branchInst->getOp() != kIROp_Loop)
                continue;
            predecessorCount++;
        }

        auto llvmType = ensureType(param->getFullType());
        auto llvmPhi = llvmBuilder.CreatePHI(llvmType, predecessorCount);

        mapInstToLLVM[param] = llvmPhi;
    }

    void finishPhi(IRParam* param, int index)
    {
        IRBlock* parentBlock = as<IRBlock>(param->getParent());
        llvm::PHINode* llvmPhi = llvm::cast<llvm::PHINode>(mapInstToLLVM.getValue(param));

        for (auto use = parentBlock->firstUse; use; use = use->nextUse)
        {
            auto branchInst = as<IRUnconditionalBranch>(use->getUser());
            if (!branchInst)
                continue;
            if (branchInst->getTargetBlock() != parentBlock)
                continue;

            UInt argStartIndex = 0;
            switch (branchInst->getOp())
            {
            case kIROp_UnconditionalBranch:
                argStartIndex = 1;
                break;
            case kIROp_Loop:
                argStartIndex = 3;
                break;
            default:
                // A phi argument can only come from an unconditional branch inst.
                // Other uses are not relavent so we should skip.
                continue;
            }
            auto sourceBlock = llvm::cast<llvm::BasicBlock>(mapInstToLLVM.getValue(branchInst->getParent()));
            auto valueInst = branchInst->getOperand(argStartIndex + index);
            llvmPhi->addIncoming(findValue(valueInst), sourceBlock);
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


    void emitFuncDefinition(IRFunc* func)
    {
        llvm::Function* llvmFunc = ensureFuncDecl(func);

        UnownedStringSlice intrinsicDef;
        IRInst* intrinsicInst;
        if (Slang::findTargetIntrinsicDefinition(func, codeGenContext->getTargetReq()->getTargetCaps(), intrinsicDef, intrinsicInst))
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
            mapInstToLLVM[func] = llvmFunc;
        }
        else
        {
            // Create all blocks first
            for (auto irBlock : func->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::BasicBlock::Create(llvmContext, "", llvmFunc);
                mapInstToLLVM[irBlock] = llvmBlock;
            }

            // Then, fill in the blocks. Lucky for us, there appears to basically be
            // a 1:1 correspondence between Slang IR blocks and LLVM IR blocks, so
            // this is straightforward.
            for (auto irBlock : func->getBlocks())
            {
                llvm::BasicBlock* llvmBlock = llvm::cast<llvm::BasicBlock>(mapInstToLLVM.getValue(irBlock));
                llvmBuilder.SetInsertPoint(llvmBlock);

                // Insert block parameters as Phi nodes.
                if (irBlock != func->getFirstBlock())
                {
                    for (auto irParam : irBlock->getParams())
                    {
                        // Create phi nodes. Can't set them yet, since the instructions
                        // they refer to may have not yet been created.
                        emitPhi(irParam);
                    }
                }

                // Then, add the regular instructions.
                for (auto irInst: irBlock->getOrdinaryInsts())
                {
                    emitLLVMInstruction(irInst);
                }
            }

            // Finally, fill in the Phi nodes.
            for (auto irBlock : func->getBlocks())
            {
                int index = 0;
                if (irBlock != func->getFirstBlock())
                {
                    for (auto irParam : irBlock->getParams())
                    {
                        finishPhi(irParam, index);
                        index++;
                    }
                }
            }
        }
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
        }
    }

    void processModule(IRModule* irModule)
    {
        // Start by emitting all function declarations, so that the functions
        // can freely refer to each other later on.
        emitGlobalDeclarations(irModule);
        emitGlobalFunctions(irModule);
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
        std::string target_triple_str = llvm::sys::getDefaultTargetTriple();
        std::string error;
        const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple_str, error);
        if (!target)
        {
            codeGenContext->getSink()->diagnose(SourceLoc(), Diagnostics::unrecognizedTargetTriple, target_triple_str.c_str(), error.c_str());
            return;
        }

        llvm::TargetOptions opt;
        llvm::Triple target_triple(target_triple_str);
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
