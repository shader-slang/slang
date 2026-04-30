// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-compiler.h"
#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
template<typename P, typename D>
struct DiffInstPair
{
    P primal;
    D differential;
    DiffInstPair() = default;
    DiffInstPair(P primal, D differential)
        : primal(primal), differential(differential)
    {
    }
    HashCode getHashCode() const
    {
        Hasher hasher;
        hasher << primal << differential;
        return hasher.getResult();
    }
    bool operator==(const DiffInstPair& other) const
    {
        return primal == other.primal && differential == other.differential;
    }
};

typedef DiffInstPair<IRInst*, IRInst*> InstPair;

inline bool isConstExprRateQualifiedType(IRType* type)
{
    auto rateQualifiedType = as<IRRateQualifiedType>(type);
    return rateQualifiedType && as<IRConstExprRate>(rateQualifiedType->getRate());
}

enum class DiffConformanceKind
{
    Any = 0,  // Perform actions for any conformance (infer from context)
    Ptr = 1,  // Perform actions for IDifferentiablePtrType
    Value = 2 // Perform actions for IDifferentiable
};

ParameterDirectionInfo transposeDirection(ParameterDirectionInfo direction);

struct AutoDiffSharedContext
{
    TargetProgram* targetProgram = nullptr;

    IRModuleInst* moduleInst = nullptr;

    // A reference to the builtin IDifferentiable interface type.
    // We use this to look up all the other types (and type exprs)
    // that conform to a base type.
    //
    IRInterfaceType* differentiableInterfaceType = nullptr;

    // Reference to the generic IForwardDifferentiable<F>
    // and IBackwardDifferentiable<F> interfaces.
    //
    IRGeneric* forwardDifferentiableInterfaceType = nullptr;
    IRGeneric* backwardDifferentiableInterfaceType = nullptr;
    IRGeneric* backwardCallableInterfaceType = nullptr;

    // The struct key for the 'Differential' associated type
    // defined inside IDifferential. We use this to lookup the differential
    // type in the conformance table associated with the concrete type.
    //
    IRStructKey* differentialAssocTypeStructKey = nullptr;

    // The struct key for the witness that `Differential` associated type conforms to
    // `IDifferential`.
    IRStructKey* differentialAssocTypeWitnessStructKey = nullptr;
    IRWitnessTableType* differentialAssocTypeWitnessTableType = nullptr;

    // The struct key for the 'zero()' associated type
    // defined inside IDifferential. We use this to lookup the
    // implementation of zero() for a given type.
    //
    IRStructKey* zeroMethodStructKey = nullptr;
    IRFuncType* zeroMethodType = nullptr;

    // The struct key for the 'add()' associated type
    // defined inside IDifferential. We use this to lookup the
    // implementation of add() for a given type.
    //
    IRStructKey* addMethodStructKey = nullptr;
    IRFuncType* addMethodType = nullptr;

    // Refernce to NullDifferential struct type. These are used
    // as sentinel values for uninitialized existential (interface-typed)
    // differentials.
    //
    IRStructType* nullDifferentialStructType = nullptr;

    // Reference to the NullDifferential : IDifferentiable witness.
    //
    IRInst* nullDifferentialWitness = nullptr;


    // A reference to the builtin IDifferentiablePtrType interface type.
    IRInterfaceType* differentiablePtrInterfaceType = nullptr;

    // The struct key for the 'Differential' associated type
    // defined inside IDifferentialPtrType. We use this to lookup the differential
    // type in the conformance table associated with the concrete type.
    //
    IRStructKey* differentialAssocRefTypeStructKey = nullptr;

    // The struct key for the witness that `Differential` associated type conforms to
    // `IDifferentialPtrType`.
    IRStructKey* differentialAssocRefTypeWitnessStructKey = nullptr;
    IRWitnessTableType* differentialAssocRefTypeWitnessTableType = nullptr;

    // Modules that don't use differentiable types
    // won't have the IDifferentiable interface type available.
    // Set to false to indicate that we are uninitialized.
    //
    bool isInterfaceAvailable = false;
    bool isPtrInterfaceAvailable = false;

    AutoDiffSharedContext(TargetProgram* targetProgram, IRModuleInst* inModuleInst);
};

struct DifferentiableTypeConformanceContext
{
    AutoDiffSharedContext* sharedContext;

    IRGlobalValueWithCode* parentFunc = nullptr;

    IRFunc* existentialDAddFunc = nullptr;

    DifferentiableTypeConformanceContext(AutoDiffSharedContext* shared)
        : sharedContext(shared)
    {
    }

    IRType* resolveType(IRBuilder* builder, IRInst* typeInst)
    {
        if (auto funcType = as<IRFuncType>(typeInst))
        {
            // resolve the parameter and result types.
            List<IRType*> paramTypes;
            for (UIndex i = 0; i < funcType->getParamCount(); ++i)
            {
                paramTypes.add(resolveType(builder, funcType->getParamType(i)));
            }

            auto resultType = resolveType(builder, funcType->getResultType());
            return builder->getFuncType(paramTypes, resultType);
        }

        switch (typeInst->getOp())
        {
        case kIROp_ForwardDiffFuncType:
            {
                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));
                List<IRType*> paramTypes;
                for (UIndex i = 0; i < innerFnType->getParamCount(); ++i)
                {
                    const auto& [paramDirection, paramType] =
                        splitParameterDirectionAndType(innerFnType->getParamType(i));

                    if (auto diffPairType = tryGetAssociationOfKind(
                            paramType,
                            AnnotationKind::DifferentialPairType))
                    {
                        paramTypes.add(
                            fromDirectionAndType(builder, paramDirection, (IRType*)diffPairType));
                    }
                    else if (
                        auto diffPtrPairType = tryGetAssociationOfKind(
                            paramType,
                            AnnotationKind::DifferentialPtrPairType))
                    {
                        paramTypes.add(fromDirectionAndType(
                            builder,
                            paramDirection,
                            (IRType*)diffPtrPairType));
                    }
                    else
                    {
                        paramTypes.add(innerFnType->getParamType(i));
                    }
                }

                // Do the same for the result type.
                IRType* resultType = innerFnType->getResultType();
                if (auto diffPairType = tryGetAssociationOfKind(
                        innerFnType->getResultType(),
                        AnnotationKind::DifferentialPairType))
                {
                    resultType = (IRType*)diffPairType;
                }
                else if (
                    auto diffPtrPairType = tryGetAssociationOfKind(
                        innerFnType->getResultType(),
                        AnnotationKind::DifferentialPtrPairType))
                {
                    resultType = (IRType*)diffPtrPairType;
                }

                return builder->getFuncType(paramTypes, resultType);
            }
        case kIROp_BackwardDiffFuncType:
            {
                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));

                List<IRType*> origParamTypes;
                for (UIndex i = 0; i < innerFnType->getParamCount(); ++i)
                {
                    origParamTypes.add(innerFnType->getParamType(i));
                }

                origParamTypes.add(fromDirectionAndType(
                    builder,
                    {ParameterDirectionInfo::Kind::Out},
                    innerFnType->getResultType()));

                List<IRType*> paramTypes;
                for (auto origParamType : origParamTypes)
                {
                    const auto& [paramDirection, paramType] =
                        splitParameterDirectionAndType(origParamType);

                    if (isDifferentiableValueType(paramType))
                    {
                        // Differentiable
                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::In:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::BorrowInOut},
                                (IRType*)tryGetAssociationOfKind(
                                    paramType,
                                    AnnotationKind::DifferentialPairType)));
                            break;
                        case ParameterDirectionInfo::Kind::Out:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::In},
                                (IRType*)getDifferentialForType(paramType)));
                            break;
                        case ParameterDirectionInfo::Kind::BorrowInOut:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::BorrowInOut},
                                (IRType*)tryGetAssociationOfKind(
                                    paramType,
                                    AnnotationKind::DifferentialPairType)));
                            break;
                        default:
                            SLANG_UNEXPECTED(
                                "Unhandled parameter direction in backward diff func type");
                        }
                    }
                    else if (isDifferentiablePtrType(paramType))
                    {
                        // Differentiable Ptr
                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::In:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::In},
                                (IRType*)tryGetAssociationOfKind(
                                    paramType,
                                    AnnotationKind::DifferentialPtrPairType)));
                            break;
                        default:
                            SLANG_UNEXPECTED(
                                "Unhandled parameter direction in backward diff func type");
                        }
                    }
                    else
                    {
                        // Non-differentiable
                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::In:
                        case ParameterDirectionInfo::Kind::Ref:
                        case ParameterDirectionInfo::Kind::BorrowIn:
                            paramTypes.add(
                                fromDirectionAndType(builder, paramDirection, paramType));
                            break;
                        case ParameterDirectionInfo::Kind::Out:
                            // skip.
                            break;
                        case ParameterDirectionInfo::Kind::BorrowInOut:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                ParameterDirectionInfo::Kind::In,
                                paramType));
                            break;
                        default:
                            SLANG_UNEXPECTED(
                                "Unhandled parameter direction in backward diff func type");
                        }
                    }
                }

                return builder->getFuncType(paramTypes, builder->getVoidType());
            }
        case kIROp_ApplyForBwdFuncType:
            {
                auto contextType = typeInst->getOperand(1);

                // Copy the func's parameter types as-is and replace the result type with
                // the context type (MinimalContext in the new design).
                //
                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));

                List<IRType*> paramTypes;
                for (UIndex i = 0; i < innerFnType->getParamCount(); ++i)
                {
                    if (isDifferentiablePtrType(innerFnType->getParamType(i)))
                    {
                        // For differentiable ptr types, we need to replace with the
                        // differential ptr pair type.
                        const auto& [paramDirection, paramType] =
                            splitParameterDirectionAndType(innerFnType->getParamType(i));
                        paramTypes.add(fromDirectionAndType(
                            builder,
                            paramDirection,
                            (IRType*)tryGetAssociationOfKind(
                                paramType,
                                AnnotationKind::DifferentialPtrPairType)));
                    }
                    else
                        paramTypes.add(innerFnType->getParamType(i));
                }

                // Result type: Tuple<ResultType, ContextType> for non-void,
                // ContextType for void.
                IRType* resultType;
                if (as<IRVoidType>(innerFnType->getResultType()))
                    resultType = (IRType*)contextType;
                else
                    resultType =
                        builder->getTupleType(innerFnType->getResultType(), (IRType*)contextType);

                return builder->getFuncType(paramTypes, resultType);
                break;
            }
        case kIROp_RematFuncType:
            {
                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));
                auto minimalCtxType = (IRType*)typeInst->getOperand(1);
                auto fullCtxType = (IRType*)typeInst->getOperand(2);

                List<IRType*> paramTypes;
                // First parameter: MinimalCtxType.
                paramTypes.add(minimalCtxType);
                // Remaining parameters: same as original function params (with ptr pair wrapping).
                for (UIndex i = 0; i < innerFnType->getParamCount(); ++i)
                {
                    if (isDifferentiablePtrType(innerFnType->getParamType(i)))
                    {
                        const auto& [paramDirection, paramType] =
                            splitParameterDirectionAndType(innerFnType->getParamType(i));
                        paramTypes.add(fromDirectionAndType(
                            builder,
                            paramDirection,
                            (IRType*)tryGetAssociationOfKind(
                                paramType,
                                AnnotationKind::DifferentialPtrPairType)));
                    }
                    else
                        paramTypes.add(innerFnType->getParamType(i));
                }

                return builder->getFuncType(paramTypes, fullCtxType);
                break;
            }
        case kIROp_BwdCallableFuncType:
            {
                auto bwdContextType = typeInst->getOperand(1);

                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));
                List<IRType*> paramTypes;

                paramTypes.add((IRType*)bwdContextType);
                for (UIndex i = 0; i < innerFnType->getParamCount(); ++i)
                {
                    const auto& [paramDirection, paramType] =
                        splitParameterDirectionAndType(innerFnType->getParamType(i));
                    if (auto diffValueType = tryGetDifferentiableValueType(paramType))
                    {
                        // If the parameter type is a differentiable value type, we replace it with
                        // the differential type.
                        //
                        paramTypes.add(fromDirectionAndType(
                            builder,
                            transposeDirection(paramDirection),
                            (IRType*)diffValueType));
                    }
                    else if (isConstExprRateQualifiedType(innerFnType->getParamType(i)))
                    {
                        paramTypes.add(innerFnType->getParamType(i));
                    }
                    else
                        paramTypes.add(builder->getVoidType());
                }

                // Add the differential of the result type.
                if (auto resultDiffType =
                        tryGetDifferentiableValueType(innerFnType->getResultType()))
                {
                    paramTypes.add((IRType*)resultDiffType);
                }

                return builder->getFuncType(paramTypes, builder->getVoidType());
                break;
            }
        }

        return (IRType*)typeInst;
    }

    IRInst* tryGetAssociationOfKind(IRInst* target, AnnotationKind kind);

    IRInst* getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* tryGetDifferentiableValueType(IRType* origType)
    {
        IRBuilder builder(sharedContext->moduleInst);
        if (auto typePack = as<IRTypePack>(origType))
        {
            List<IRType*> diffTypes;
            for (UInt i = 0; i < typePack->getOperandCount(); i++)
            {
                auto elemDiff = tryGetDifferentiableValueType((IRType*)typePack->getOperand(i));
                if (!elemDiff)
                    return nullptr;
                diffTypes.add((IRType*)elemDiff);
            }
            return builder.getTypePack(diffTypes.getCount(), diffTypes.getBuffer());
        }

        return tryGetAssociationOfKind(origType, AnnotationKind::DifferentialType);
    }

    IRInst* tryGetDifferentiablePtrType(IRType* origType)
    {
        IRBuilder builder(sharedContext->moduleInst);
        if (auto typePack = as<IRTypePack>(origType))
        {
            List<IRType*> diffTypes;
            for (UInt i = 0; i < typePack->getOperandCount(); i++)
            {
                auto elemDiff = tryGetDifferentiablePtrType((IRType*)typePack->getOperand(i));
                if (!elemDiff)
                    return nullptr;
                diffTypes.add((IRType*)elemDiff);
            }
            return builder.getTypePack(diffTypes.getCount(), diffTypes.getBuffer());
        }

        return tryGetAssociationOfKind(origType, AnnotationKind::DifferentialPtrType);
    }

    IRType* tryGetDiffPairType(IRType* originalType)
    {
        IRBuilder builder(sharedContext->moduleInst);

        if (auto typePack = as<IRTypePack>(originalType))
        {
            List<IRType*> pairTypes;
            for (UInt i = 0; i < typePack->getOperandCount(); i++)
            {
                auto elemPair = tryGetDiffPairType((IRType*)typePack->getOperand(i));
                if (!elemPair)
                    return nullptr;
                pairTypes.add(elemPair);
            }
            return builder.getTypePack(pairTypes.getCount(), pairTypes.getBuffer());
        }

        // In case we're dealing with a parameter type, we need to split out the direction first.
        // If we're not, then the split & merge are no-ops.
        //
        auto [passingMode, baseType] = splitParameterDirectionAndType(originalType);
        auto basePairType = tryGetAssociationOfKind(baseType, AnnotationKind::DifferentialPairType);

        if (!basePairType)
        {
            basePairType =
                tryGetAssociationOfKind(baseType, AnnotationKind::DifferentialPtrPairType);
        }

        if (!basePairType)
            return nullptr;

        return fromDirectionAndType(&builder, passingMode, (IRType*)basePairType);
    }

    // Lookup and return the 'Differential' type declared in the concrete type
    // in order to conform to the IDifferentiable/IDifferentiablePtrType interfaces
    //
    IRInst* tryGetDifferentialForType(IRType* origType)
    {
        auto diffValueType = tryGetDifferentiableValueType(origType);
        auto diffPtrType = tryGetDifferentiablePtrType(origType);

        SLANG_ASSERT(
            !(diffValueType && diffPtrType) &&
            "Type cannot conform to both IDifferentiable and IDifferentiablePtrType");

        return diffValueType ? diffValueType : diffPtrType;
    }

    // Lookup and return the 'Differential' type declared in the concrete type
    // in order to conform to the IDifferentiable/IDifferentiablePtrType interfaces
    // Asserts that the differential exists.
    IRInst* getDifferentialForType(IRType* origType)
    {
        auto diffValueType = tryGetDifferentiableValueType(origType);
        auto diffPtrType = tryGetDifferentiablePtrType(origType);

        SLANG_ASSERT(
            !(diffValueType && diffPtrType) &&
            "Type cannot conform to both IDifferentiable and IDifferentiablePtrType");

        SLANG_RELEASE_ASSERT(diffValueType || diffPtrType);
        return diffValueType ? diffValueType : diffPtrType;
    }

    bool isDifferentiableType(IRType* origType)
    {
        return isDifferentiableValueType(origType) || isDifferentiablePtrType(origType);
    }

    bool isDifferentiableValueType(IRType* origType)
    {
        return tryGetDifferentiableValueType(origType) != nullptr;
    }

    bool isDifferentiablePtrType(IRType* origType)
    {
        return tryGetDifferentiablePtrType(origType) != nullptr;
    }

    IRInst* getZeroMethodForType(IRBuilder* builder, IRType* origType)
    {
        SLANG_UNUSED(builder);
        return tryGetAssociationOfKind(origType, AnnotationKind::DifferentialZero);
    }

    IRInst* getAddMethodForType(IRBuilder* builder, IRType* origType)
    {
        SLANG_UNUSED(builder);
        return tryGetAssociationOfKind(origType, AnnotationKind::DifferentialAdd);
    }

    IRInst* emitNullDifferential(IRBuilder* builder)
    {
        return builder->emitCallInst(
            sharedContext->nullDifferentialStructType,
            getZeroMethodForType(builder, sharedContext->nullDifferentialStructType),
            List<IRInst*>());
    }

    IRFunc* getOrCreateExistentialDAddMethod();

    IRInst* buildDifferentiablePairWitness(
        IRBuilder* builder,
        IRDifferentialPairTypeBase* pairType,
        DiffConformanceKind target);

    IRInst* emitDAddOfDiffInstType(
        IRBuilder* builder,
        IRType* primalType,
        IRInst* op1,
        IRInst* op2);

    IRInst* emitDAddForExistentialType(
        IRBuilder* builder,
        IRType* primalType,
        IRInst* op1,
        IRInst* op2);

    IRInst* emitDZeroOfDiffInstType(IRBuilder* builder, IRType* primalType);

    void markDiffTypeInst(IRBuilder* builder, IRInst* diffInst, IRType* primalType);

    void markDiffPairTypeInst(IRBuilder* builder, IRInst* diffPairInst, IRType* pairType);
};


void stripAutoDiffDecorations(IRModule* module);
void stripTempDecorations(IRInst* inst);

bool isNoDiffType(IRType* paramType);
bool isNeverDiffFuncType(IRFuncType* funcType);

IRInst* _lookupWitness(
    IRBuilder* builder,
    IRInst* witness,
    IRInst* requirementKey,
    IRType* resultType = nullptr);

void checkAutodiffPatterns(IRModule* module, TargetProgram* target, DiagnosticSink* sink);

bool finalizeAutoDiffPass(IRModule* module, TargetProgram* target);

// Utility methods

void copyCheckpointHints(
    IRBuilder* builder,
    IRGlobalValueWithCode* oldInst,
    IRGlobalValueWithCode* newInst);

void cloneCheckpointHint(
    IRBuilder* builder,
    IRCheckpointHintDecoration* oldInst,
    IRGlobalValueWithCode* code);

void stripDerivativeDecorations(IRInst* inst);

bool canTypeBeStored(IRInst* type);

inline bool isRelevantDifferentialPair(IRType* type)
{
    if (as<IRDifferentialPairType>(type))
    {
        return true;
    }
    else if (auto argPtrType = asRelevantPtrType(type))
    {
        if (as<IRDifferentialPairType>(argPtrType->getValueType()))
        {
            return true;
        }
    }
    return false;
}

bool isRuntimeType(IRType* type);

UIndex addPhiOutputArg(
    IRBuilder* builder,
    IRBlock* block,
    IRInst*& inoutTerminatorInst,
    IRInst* arg);

IRUse* findUniqueStoredVal(IRVar* var);
IRUse* findLatestUniqueWriteUse(IRVar* var);
IRUse* findEarliestUniqueWriteUse(IRVar* var);

bool isDerivativeContextVar(IRVar* var);

bool isDiffInst(IRInst* inst);

bool isDifferentialOrRecomputeBlock(IRBlock* block);

void copyDebugInfo(IRInst* srcFunc, IRInst* destFunc);

void copyOriginalDecorations(IRInst* origFunc, IRInst* diffFunc);

inline bool isDifferentialInst(IRInst* inst)
{
    return inst->findDecoration<IRDifferentialInstDecoration>();
}

inline bool isPrimalInst(IRInst* inst)
{
    return inst->findDecoration<IRPrimalInstDecoration>() || (as<IRConstant>(inst) != nullptr);
}

inline bool isMixedDifferentialInst(IRInst* inst)
{
    return inst->findDecoration<IRMixedDifferentialInstDecoration>();
}

}; // namespace Slang
