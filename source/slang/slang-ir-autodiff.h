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

enum class FuncBodyTranscriptionTaskType
{
    Forward,
    BackwardPrimal,
    BackwardPropagate,
    Backward
};

struct FuncBodyTranscriptionTask
{
    FuncBodyTranscriptionTaskType type;
    IRFunc* originalFunc;
    IRFunc* resultFunc;
};

struct AutoDiffTranscriberBase;

struct DiffTranscriberSet
{
    AutoDiffTranscriberBase* forwardTranscriber = nullptr;
    AutoDiffTranscriberBase* primalTranscriber = nullptr;
    AutoDiffTranscriberBase* propagateTranscriber = nullptr;
    AutoDiffTranscriberBase* backwardTranscriber = nullptr;
};


enum class DiffConformanceKind
{
    Any = 0,  // Perform actions for any conformance (infer from context)
    Ptr = 1,  // Perform actions for IDifferentiablePtrType
    Value = 2 // Perform actions for IDifferentiable
};

enum class FunctionConformanceKind
{
    Unknown = 0,
    // ForwardDifferentiable = 1,
    // BackwardDifferentiable = 2,
    // BackwardPropCallable = 3,

    ForwardDerivative = 1,
    BackwardApply = 2,
    BackwardContext = 3,
    BackwardContextGetVal = 4,
    BackwardProp = 5,
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

    List<FuncBodyTranscriptionTask> followUpFunctionsToTranscribe;

    DiffTranscriberSet transcriberSet;

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
    OrderedDictionary<IRType*, IRInst*> differentiableTypeWitnessDictionary;

    struct WitnessTableCacheKey
    {
        IRInst* inst;
        FunctionConformanceKind conformanceType;
        bool operator==(const WitnessTableCacheKey& other) const
        {
            return inst == other.inst && conformanceType == other.conformanceType;
        }

        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher.hashValue(inst);
            hasher.hashValue(static_cast<int>(conformanceType));
            return hasher.getResult();
        }
    };

    // (inst, conformance-type) -> witness-table
    Dictionary<WitnessTableCacheKey, IRInst*> witnessTableCache;

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
    Dictionary<ValAssociationCacheKey, IRInst*> associationCache;

    IRFunc* existentialDAddFunc = nullptr;

    DifferentiableTypeConformanceContext(AutoDiffSharedContext* shared)
        : sharedContext(shared)
    {
        // Populate dictionary with null differential type.
        if (sharedContext->nullDifferentialStructType)
            differentiableTypeWitnessDictionary.add(
                sharedContext->nullDifferentialStructType,
                sharedContext->nullDifferentialWitness);
    }

    IRType* lookupContextType(IRBuilder* builder, IRInst* fnInst)
    {
        /*auto bwdDiffWitness =
            tryGetWitnessOfKind(fnInst, FunctionConformanceKind::BackwardDifferentiable);
        SLANG_ASSERT(bwdDiffWitness);
        auto bwdDiffWitnessType = as<IRWitnessTableType>(bwdDiffWitness->getDataType());
        auto bwdDiffWitnessInterface =
            as<IRInterfaceType>(bwdDiffWitnessType->getConformanceType());
        // TODO: remove hardcoded index (use key)
        IRInterfaceRequirementEntry* contextTypeEntry =
            as<IRInterfaceRequirementEntry>(bwdDiffWitnessInterface->getOperand(0));
        auto bwdContextType = builder->emitLookupInterfaceMethodInst(
            builder->getTypeKind(),
            bwdDiffWitness,
            contextTypeEntry->getRequirementKey());*/

        return (
            IRType*)tryGetAssociationOfKind(fnInst, ValAssociationKind::BackwardDerivativeContext);
        // return (IRType*)bwdContextType;
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
                    if (auto witness = tryGetDifferentiableWitness(
                            builder,
                            paramType,
                            DiffConformanceKind::Any))
                    {
                        paramTypes.add(fromDirectionAndType(
                            builder,
                            paramDirection,
                            getOrCreateDiffPairType(builder, paramType, witness)));
                    }
                    else
                        paramTypes.add(innerFnType->getParamType(i));
                }

                // Do the same for the result type.
                IRType* resultType = innerFnType->getResultType();
                if (auto witness =
                        tryGetDifferentiableWitness(builder, resultType, DiffConformanceKind::Any))
                    resultType = getOrCreateDiffPairType(builder, resultType, witness);

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

                    if (auto witness = tryGetDifferentiableWitness(
                            builder,
                            paramType,
                            DiffConformanceKind::Any))
                    {
                        // Differentiable
                        switch (paramDirection.kind)
                        {
                        case ParameterDirectionInfo::Kind::In:
                            paramTypes.add(fromDirectionAndType(
                                builder,
                                {ParameterDirectionInfo::Kind::BorrowInOut},
                                getOrCreateDiffPairType(builder, paramType, witness)));
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
                                getOrCreateDiffPairType(builder, paramType, witness)));
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
                    paramTypes.add(innerFnType->getParamType(i));
                }

                return builder->getFuncType(paramTypes, (IRType*)bwdContextType);
                break;
            }
        case kIROp_FuncResultType:
            {
                // auto bwdContextType = lookupContextType(builder, typeInst->getOperand(0));
                auto bwdContextType = typeInst->getOperand(1);
                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));

                return builder->getFuncType(
                    List<IRType*>((IRType*)bwdContextType),
                    innerFnType->getResultType());
                break;
            }
        case kIROp_BwdCallableFuncType:
            {
                // auto bwdContextType = lookupContextType(builder, typeInst->getOperand(0));
                auto bwdContextType = typeInst->getOperand(1);

                auto innerFnType = cast<IRFuncType>(resolveType(builder, typeInst->getOperand(0)));
                List<IRType*> paramTypes;

                paramTypes.add((IRType*)bwdContextType);
                for (UIndex i = 0; i < innerFnType->getParamCount(); ++i)
                {
                    const auto& [paramDirection, paramType] =
                        splitParameterDirectionAndType(innerFnType->getParamType(i));
                    if (auto diffType = getDifferentialForType(builder, paramType))
                    {
                        // If the parameter type is a differentiable type, we replace it with
                        // the differential type.

                        paramTypes.add(fromDirectionAndType(
                            builder,
                            transposeDirection(paramDirection),
                            (IRType*)diffType));
                    }
                    else
                        paramTypes.add(builder->getVoidType());
                }

                // Add the differential of the result type.
                if (auto resultDiffType =
                        getDifferentialForType(builder, innerFnType->getResultType()))
                    paramTypes.add((IRType*)resultDiffType);

                return builder->getFuncType(paramTypes, builder->getVoidType());
                break;
            }
        }

        return (IRType*)typeInst;
    }

    /*FunctionConformanceKind getFunctionConformanceKind(IRInst* conformanceType)
    {
        if (auto knownBuiltinDecoration =
                conformanceType->findDecoration<IRKnownBuiltinDecoration>())
        {
            auto name = knownBuiltinDecoration->getName();
            // TODO: We really need something better than doing a string comparison..
            if (name == toSlice("IForwardDifferentiable"))
            {
                return FunctionConformanceKind::ForwardDifferentiable;
            }
            else if (name == toSlice("IBackwardDifferentiable"))
            {
                return FunctionConformanceKind::BackwardDifferentiable;
            }
            else if (name == toSlice("IBwdCallable"))
            {
                return FunctionConformanceKind::BackwardPropCallable;
            }
        }

        return FunctionConformanceKind::Unknown;
    }*/

    void setFunc(IRInst* inst);

    template<typename T>
    List<T*> getAnnotations(IRGlobalValueWithCode* inst);

    template<typename T>
    List<T*> getAnnotations(IRModuleInst* inst);

    IRInst* tryGetWitnessOfKind(IRInst* target, FunctionConformanceKind kind);

    IRInst* tryGetAssociationOfKind(IRInst* target, ValAssociationKind kind);

    void buildGlobalWitnessDictionary();

    // Lookup a witness table for the concreteType. One should exist if concreteType
    // inherits (successfully) from IDifferentiable.
    //
    IRInst* lookUpConformanceForType(IRInst* type, DiffConformanceKind kind);

    IRInst* lookUpInterfaceMethod(
        IRBuilder* builder,
        IRType* origType,
        IRStructKey* key,
        IRType* resultType = nullptr,
        DiffConformanceKind kind = DiffConformanceKind::Any);

    IRType* differentiateType(IRBuilder* builder, IRInst* primalType);

    IRInst* tryGetDifferentiableWitness(
        IRBuilder* builder,
        IRInst* originalType,
        DiffConformanceKind kind);

    IRType* getOrCreateDiffPairType(IRBuilder* builder, IRInst* primalType, IRInst* witness);

    IRInst* getDifferentialTypeFromDiffPairType(
        IRBuilder* builder,
        IRDifferentialPairTypeBase* diffPairType);

    IRInst* getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* getDiffZeroMethodFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    IRInst* getDiffAddMethodFromPairType(IRBuilder* builder, IRDifferentialPairTypeBase* type);

    void addTypeToDictionary(IRType* type, IRInst* witness);

    IRInterfaceType* getConformanceTypeFromWitness(IRInst* witness);

    IRInst* tryExtractConformanceFromInterfaceType(
        IRBuilder* builder,
        IRInterfaceType* interfaceType,
        IRWitnessTable* witnessTable);

    List<IRInterfaceRequirementEntry*> findInterfaceLookupPath(
        IRInterfaceType* supType,
        IRInterfaceType* type);

    // Lookup and return the 'Differential' type declared in the concrete type
    // in order to conform to the IDifferentiable/IDifferentiablePtrType interfaces
    // Note that inside a generic block, this will be a witness table lookup instruction
    // that gets resolved during the specialization pass.
    //
    IRInst* getDifferentialForType(IRBuilder* builder, IRType* origType)
    {
        switch (origType->getOp())
        {
        case kIROp_InterfaceType:
            {
                if (isDifferentiableValueType(origType))
                    return this->sharedContext->differentiableInterfaceType;
                else if (isDifferentiablePtrType(origType))
                    return this->sharedContext->differentiablePtrInterfaceType;
                else
                    return nullptr;
            }
        case kIROp_ArrayType:
            {
                auto diffElementType = (IRType*)getDifferentialForType(
                    builder,
                    as<IRArrayType>(origType)->getElementType());
                if (!diffElementType)
                    return nullptr;
                return builder->getArrayType(
                    diffElementType,
                    as<IRArrayType>(origType)->getElementCount());
            }
        case kIROp_TupleType:
        case kIROp_TypePack:
        case kIROp_OptionalType:
            {
                return differentiateType(builder, origType);
            }
        case kIROp_DifferentialPairUserCodeType:
            {
                auto diffPairType = as<IRDifferentialPairTypeBase>(origType);
                auto diffType = getDiffTypeFromPairType(builder, diffPairType);
                auto diffWitness = getDiffTypeWitnessFromPairType(builder, diffPairType);
                return builder->getDifferentialPairUserCodeType((IRType*)diffType, diffWitness);
            }
        case kIROp_DifferentialPtrPairType:
            {
                auto diffPairType = as<IRDifferentialPairTypeBase>(origType);
                auto diffType = getDiffTypeFromPairType(builder, diffPairType);
                auto diffWitness = getDiffTypeWitnessFromPairType(builder, diffPairType);
                return builder->getDifferentialPtrPairType((IRType*)diffType, diffWitness);
            }
        default:
            /*
            if (isDifferentiableValueType(origType))
                return lookUpInterfaceMethod(
                    builder,
                    origType,
                    sharedContext->differentialAssocTypeStructKey,
                    builder->getTypeKind());
            else if (isDifferentiablePtrType(origType))
                return lookUpInterfaceMethod(
                    builder,
                    origType,
                    sharedContext->differentialAssocRefTypeStructKey,
                    builder->getTypeKind());
            else
                return nullptr;
            */
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
        /*
        for (; origType;)
        {
            switch (origType->getOp())
            {
            case kIROp_FloatType:
            case kIROp_HalfType:
            case kIROp_DoubleType:
            case kIROp_DifferentialPairType:
            case kIROp_DifferentialPairUserCodeType:
                return true;
            case kIROp_VectorType:
            case kIROp_ArrayType:
            case kIROp_PtrType:
            case kIROp_OutParamType:
            case kIROp_BorrowInOutParamType:
                origType = (IRType*)origType->getOperand(0);
                continue;
            default:
                return lookUpConformanceForType(origType, DiffConformanceKind::Value) != nullptr;
            }
        }
        return false;
        */
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialType) != nullptr;
    }

    bool isDifferentiablePtrType(IRType* origType)
    {
        /*
        for (; origType;)
        {
            switch (origType->getOp())
            {
            case kIROp_VectorType:
            case kIROp_ArrayType:
            case kIROp_PtrType:
            case kIROp_OutParamType:
            case kIROp_BorrowInOutParamType:
                origType = (IRType*)origType->getOperand(0);
                continue;
            default:
                return lookUpConformanceForType(origType, DiffConformanceKind::Ptr) != nullptr;
            }
        }
        return false;
        */
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialPtrType) !=
               nullptr;
    }

    IRInst* getZeroMethodForType(IRBuilder* builder, IRType* origType)
    {
        /*
        auto result = lookUpInterfaceMethod(
            builder,
            origType,
            sharedContext->zeroMethodStructKey,
            sharedContext->zeroMethodType,
            DiffConformanceKind::Value);
        return result;*/
        return tryGetAssociationOfKind(origType, ValAssociationKind::DifferentialZero);
    }

    IRInst* getAddMethodForType(IRBuilder* builder, IRType* origType)
    {
        /*
        auto result = lookUpInterfaceMethod(
            builder,
            origType,
            sharedContext->addMethodStructKey,
            sharedContext->addMethodType,
            DiffConformanceKind::Value);*/

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

    IRInst* buildArrayWitness(
        IRBuilder* builder,
        IRArrayType* pairType,
        DiffConformanceKind target);

    IRInst* buildTupleWitness(IRBuilder* builder, IRInst* tupleType, DiffConformanceKind target);

    IRInst* buildExtractExistensialTypeWitness(
        IRBuilder* builder,
        IRExtractExistentialType* extractExistentialType,
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
};


struct DifferentialPairTypeBuilder
{
    DifferentialPairTypeBuilder() = default;

    DifferentialPairTypeBuilder(AutoDiffSharedContext* sharedContext)
        : sharedContext(sharedContext)
    {
    }

    IRInst* findSpecializationForParam(IRInst* specializeInst, IRInst* genericParam);

    IRInst* emitFieldAccessor(IRBuilder* builder, IRInst* baseInst, IRStructKey* key);

    IRInst* emitPrimalFieldAccess(IRBuilder* builder, IRType* loweredPairType, IRInst* baseInst);

    IRInst* emitDiffFieldAccess(IRBuilder* builder, IRType* loweredPairType, IRInst* baseInst);

    IRInst* emitExistentialMakePair(
        IRBuilder* builder,
        IRInst* type,
        IRInst* primalInst,
        IRInst* diffInst);

    IRStructKey* _getOrCreateDiffStructKey();

    IRStructKey* _getOrCreatePrimalStructKey();

    IRInst* _createDiffPairType(IRType* origBaseType, IRType* diffType);

    IRInst* _createDiffPairInterfaceRequirement(IRType* origBaseType, IRType* diffType);

    IRInst* lowerDiffPairType(IRBuilder* builder, IRType* originalPairType);

    IRInst* getOrCreateCommonDiffPairInterface(IRBuilder* builder);

    struct PairStructKey
    {
        IRInst* originalType;
        IRInst* diffType;
    };

    // Cache from pair types to lowered type.
    Dictionary<IRInst*, IRInst*> pairTypeCache;

    // Cache from existential pair types to their lowered interface keys.
    // We use a different cache because an interface type can have
    // a regular pair for the pair of interface types, as well as an
    // interface key for the associated pair types used for its implementations
    //
    Dictionary<IRInst*, IRInst*> existentialPairTypeCache;

    // Cache for any interface requirement keys (generated for existential
    // pair types)
    //
    Dictionary<IRInst*, IRStructKey*> assocPairTypeKeyMap;
    Dictionary<IRInst*, IRStructKey*> makePairKeyMap;
    Dictionary<IRInst*, IRStructKey*> getPrimalKeyMap;
    Dictionary<IRInst*, IRStructKey*> getDiffKeyMap;

    // More caches for easier lookups of the types associated with the
    // keys. (avoid having to keep recomputing or performing complicated
    // lookups)
    //
    Dictionary<IRInst*, IRFuncType*> makePairFuncTypeMap;
    Dictionary<IRInst*, IRFuncType*> getPrimalFuncTypeMap;
    Dictionary<IRInst*, IRFuncType*> getDiffFuncTypeMap;

    // Even more caches for easier access to original primal/diff types
    // (Only used for existential pair types). For regular pair types,
    // these are easy to find right on the type itself.
    //
    Dictionary<IRInst*, IRType*> primalTypeMap;
    Dictionary<IRInst*, IRType*> diffTypeMap;


    IRStructKey* globalPrimalKey = nullptr;

    IRStructKey* globalDiffKey = nullptr;

    IRInst* genericDiffPairType = nullptr;

    List<IRInst*> generatedTypeList;

    AutoDiffSharedContext* sharedContext = nullptr;

    IRInterfaceType* commonDiffPairInterface = nullptr;
};

void stripAutoDiffDecorations(IRModule* module);
void stripTempDecorations(IRInst* inst);

bool isNoDiffType(IRType* paramType);
bool isNeverDiffFuncType(IRFuncType* funcType);

IRInst* lookupForwardDerivativeReference(IRInst* primalFunction);

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

bool processAutodiffCalls(
    TargetProgram* target,
    IRModule* module,
    DiagnosticSink* sink,
    IRAutodiffPassOptions const& options = IRAutodiffPassOptions());

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

bool isDifferentiableType(DifferentiableTypeConformanceContext& context, IRInst* typeInst);

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


}; // namespace Slang
