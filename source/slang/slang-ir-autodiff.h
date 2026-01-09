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


struct AutoDiffTranscriberBase;

enum class DiffConformanceKind
{
    Any = 0,  // Perform actions for any conformance (infer from context)
    Ptr = 1,  // Perform actions for IDifferentiablePtrType
    Value = 2 // Perform actions for IDifferentiable
};

ParameterDirectionInfo transposeDirection(ParameterDirectionInfo direction);

struct AutoDiffSharedContext
{
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

    IRStructKey* mulMethodStructKey = nullptr;

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

    AutoDiffSharedContext(IRModuleInst* inModuleInst);

private:
    IRInst* findDifferentiableInterface();
    IRInst* findForwardDifferentiableInterface();
    IRInst* findBackwardDifferentiableInterface();
    IRInst* findBackwardCallableInterface();

    IRStructType* findNullDifferentialStructType();

    IRInst* findNullDifferentialWitness();

    IRStructKey* findDifferentialTypeStructKey()
    {
        return cast<IRStructKey>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 0)->getRequirementKey());
    }

    IRStructKey* findDifferentialTypeWitnessStructKey()
    {
        return cast<IRStructKey>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 1)->getRequirementKey());
    }

    IRWitnessTableType* findDifferentialTypeWitnessTableType()
    {
        return cast<IRWitnessTableType>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 1)->getRequirementVal());
    }

    IRStructKey* findZeroMethodStructKey()
    {
        return cast<IRStructKey>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 2)->getRequirementKey());
    }

    IRStructKey* findAddMethodStructKey()
    {
        return cast<IRStructKey>(
            getInterfaceEntryAtIndex(differentiableInterfaceType, 5)->getRequirementKey());
    }

    IRStructKey* findDifferentialPtrTypeStructKey()
    {
        return cast<IRStructKey>(
            getInterfaceEntryAtIndex(differentiablePtrInterfaceType, 0)->getRequirementKey());
    }

    IRStructKey* findDifferentialPtrTypeWitnessStructKey()
    {
        return cast<IRStructKey>(
            getInterfaceEntryAtIndex(differentiablePtrInterfaceType, 1)->getRequirementKey());
    }

    IRWitnessTableType* findDifferentialPtrTypeWitnessTableType()
    {
        return cast<IRWitnessTableType>(
            getInterfaceEntryAtIndex(differentiablePtrInterfaceType, 1)->getRequirementVal());
    }

    // IRStructKey* getIDifferentiableStructKeyAtIndex(UInt index);
    IRInterfaceRequirementEntry* getInterfaceEntryAtIndex(IRInterfaceType* interface, UInt index);
};

struct DifferentiableTypeConformanceContext
{
    AutoDiffSharedContext* sharedContext;

    IRGlobalValueWithCode* parentFunc = nullptr;

    /*
    struct ValAssociationCacheKey
    {
        IRInst* inst;
        ValAssociationKind associationKind;
        bool operator==(const ValAssociationCacheKey& other) const
        {
            return inst == other.inst && associationKind == other.associationKind;
        }

        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher.hashValue(inst);
            hasher.hashValue(static_cast<int>(associationKind));
            return hasher.getResult();
        }
    };

    // (inst, association-kind) -> associated-inst
    Dictionary<ValAssociationCacheKey, IRInst*> associationCache;*/

    IRFunc* existentialDAddFunc = nullptr;

    DifferentiableTypeConformanceContext(AutoDiffSharedContext* shared)
        : sharedContext(shared)
    {
    }

    IRType* lookupContextType(IRBuilder* builder, IRInst* fnInst)
    {
        return (
            IRType*)tryGetAssociationOfKind(fnInst, ValAssociationKind::BackwardDerivativeContext);
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
                            ValAssociationKind::DifferentialPairType))
                    {
                        paramTypes.add(
                            fromDirectionAndType(builder, paramDirection, (IRType*)diffPairType));
                    }
                    else if (
                        auto diffPtrPairType = tryGetAssociationOfKind(
                            paramType,
                            ValAssociationKind::DifferentialPtrPairType))
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
                        ValAssociationKind::DifferentialPairType))
                {
                    resultType = (IRType*)diffPairType;
                }
                else if (
                    auto diffPtrPairType = tryGetAssociationOfKind(
                        innerFnType->getResultType(),
                        ValAssociationKind::DifferentialPtrPairType))
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
                                    ValAssociationKind::DifferentialPairType)));
                            break;
                        case ParameterDirectionInfo::Kind::Out:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::In},
                                (IRType*)getDifferentialForType(builder, paramType)));
                            break;
                        case ParameterDirectionInfo::Kind::BorrowInOut:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::BorrowInOut},
                                (IRType*)tryGetAssociationOfKind(
                                    paramType,
                                    ValAssociationKind::DifferentialPairType)));
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
                                    ValAssociationKind::DifferentialPtrPairType)));
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
                // auto bwdContextType = lookupContextType(builder, typeInst->getOperand(0));
                auto bwdContextType = typeInst->getOperand(1);

                // Copy the func's parameter types as-is and replace the result type with
                // the bwd context type.
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
                                ValAssociationKind::DifferentialPtrPairType)));
                    }
                    else
                        paramTypes.add(innerFnType->getParamType(i));
                }

                return builder->getFuncType(paramTypes, (IRType*)bwdContextType);
                break;
            }
        case kIROp_FuncResultType:
            {
                auto bwdContextType = typeInst->getOperand(1);
                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));

                return builder->getFuncType(
                    List<IRType*>((IRType*)bwdContextType),
                    innerFnType->getResultType());
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
                    if (isDifferentiableValueType(paramType))
                    {
                        // If the parameter type is a differentiable value type, we replace it with
                        // the differential type.
                        //
                        auto diffType = getDifferentialForType(builder, paramType);
                        paramTypes.add(fromDirectionAndType(
                            builder,
                            transposeDirection(paramDirection),
                            (IRType*)diffType));
                    }
                    else
                        paramTypes.add(builder->getVoidType());
                }

                // Add the differential of the result type.
                if (isDifferentiableValueType(innerFnType->getResultType()))
                {
                    auto resultDiffType =
                        getDifferentialForType(builder, innerFnType->getResultType());
                    paramTypes.add((IRType*)resultDiffType);
                }

                return builder->getFuncType(paramTypes, builder->getVoidType());
                break;
            }
        }

        return (IRType*)typeInst;
    }

    IRInst* tryGetAssociationOfKind(IRInst* target, ValAssociationKind kind);

    IRInst* getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* tryGetDifferentiableValueType(IRBuilder* builder, IRType* origType)
    {
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialType);
    }

    IRInst* tryGetDifferentiablePtrType(IRBuilder* builder, IRType* origType)
    {
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialPtrType);
    }

    IRType* tryGetDiffPairType(IRBuilder* builder, IRType* originalType)
    {
        // In case we're dealing with a parameter type, we need to split out the direction first.
        // If we're not, then the split & merge are no-ops.
        //
        auto [passingMode, baseType] = splitParameterDirectionAndType(originalType);
        auto basePairType =
            tryGetAssociationOfKind(baseType, ValAssociationKind::DifferentialPairType);

        if (!basePairType)
        {
            basePairType =
                tryGetAssociationOfKind(baseType, ValAssociationKind::DifferentialPtrPairType);
        }

        if (!basePairType)
            return nullptr;

        return fromDirectionAndType(builder, passingMode, (IRType*)basePairType);
    }

    // Lookup and return the 'Differential' type declared in the concrete type
    // in order to conform to the IDifferentiable/IDifferentiablePtrType interfaces
    // Note that inside a generic block, this will be a witness table lookup instruction
    // that gets resolved during the specialization pass.
    //
    IRInst* getDifferentialForType(IRBuilder* builder, IRType* origType)
    {
        switch (origType->getOp())
        {
        default:
            auto diffValueType =
                tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialType);
            auto diffPtrType =
                tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialPtrType);

            SLANG_ASSERT(
                !(diffValueType && diffPtrType) &&
                "Type cannot conform to both IDifferentiable and IDifferentiablePtrType");

            return diffValueType ? diffValueType : diffPtrType;
        }
    }

    bool isDifferentiableType(IRType* origType)
    {
        return isDifferentiableValueType(origType) || isDifferentiablePtrType(origType);
    }

    bool isDifferentiableValueType(IRType* origType)
    {
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialType) != nullptr;
    }

    bool isDifferentiablePtrType(IRType* origType)
    {
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialPtrType) !=
               nullptr;
    }

    IRInst* getZeroMethodForType(IRBuilder* builder, IRType* origType)
    {
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialZero);
    }

    IRInst* getAddMethodForType(IRBuilder* builder, IRType* origType)
    {
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialAdd);
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

struct IRAutodiffPassOptions
{
    // Nothing for now...
};

void checkAutodiffPatterns(TargetProgram* target, IRModule* module, DiagnosticSink* sink);

bool finalizeAutoDiffPass(TargetProgram* target, IRModule* module);

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

bool isBackwardDifferentiableFunc(IRInst* func);

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

IRInst* getExistentialBaseWitnessTable(IRBuilder* builder, IRType* type);

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
