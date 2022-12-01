// slang-ir-autodiff-fwd.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

#include "slang-ir-clone.h"
#include "slang-ir-dce.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-util.h"
#include "slang-ir-inst-pass-base.h"

namespace Slang
{

struct AutoDiffSharedContext
{
    IRModuleInst* moduleInst = nullptr;

    SharedIRBuilder* sharedBuilder = nullptr;

    // A reference to the builtin IDifferentiable interface type.
    // We use this to look up all the other types (and type exprs)
    // that conform to a base type.
    // 
    IRInterfaceType* differentiableInterfaceType = nullptr;

    // The struct key for the 'Differential' associated type
    // defined inside IDifferential. We use this to lookup the differential
    // type in the conformance table associated with the concrete type.
    // 
    IRStructKey* differentialAssocTypeStructKey = nullptr;

    // The struct key for the witness that `Differential` associated type conforms to
    // `IDifferential`.
    IRStructKey* differentialAssocTypeWitnessStructKey = nullptr;


    // The struct key for the 'zero()' associated type
    // defined inside IDifferential. We use this to lookup the 
    // implementation of zero() for a given type.
    // 
    IRStructKey* zeroMethodStructKey = nullptr;

    // The struct key for the 'add()' associated type
    // defined inside IDifferential. We use this to lookup the 
    // implementation of add() for a given type.
    // 
    IRStructKey* addMethodStructKey = nullptr;

    IRStructKey* mulMethodStructKey = nullptr;


    // Modules that don't use differentiable types
    // won't have the IDifferentiable interface type available. 
    // Set to false to indicate that we are uninitialized.
    // 
    bool                                    isInterfaceAvailable = false;


    AutoDiffSharedContext(IRModuleInst* inModuleInst);

private:

    IRInst* findDifferentiableInterface();

    IRStructKey* findDifferentialTypeStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(0);
    }

    IRStructKey* findDifferentialTypeWitnessStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(1);
    }

    IRStructKey* findZeroMethodStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(2);
    }

    IRStructKey* findAddMethodStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(3);
    }

    IRStructKey* findMulMethodStructKey()
    {
        return getIDifferentiableStructKeyAtIndex(4);
    }

    IRStructKey* getIDifferentiableStructKeyAtIndex(UInt index);
};

struct DifferentiableTypeConformanceContext
{
    AutoDiffSharedContext* sharedContext;

    IRGlobalValueWithCode* parentFunc = nullptr;
    OrderedDictionary<IRType*, IRInst*> differentiableWitnessDictionary;

    DifferentiableTypeConformanceContext(AutoDiffSharedContext* shared)
        : sharedContext(shared)
    {}

    void setFunc(IRGlobalValueWithCode* func);

    void buildGlobalWitnessDictionary();

    // Lookup a witness table for the concreteType. One should exist if concreteType
    // inherits (successfully) from IDifferentiable.
    // 
    IRInst* lookUpConformanceForType(IRInst* type);

    IRInst* lookUpInterfaceMethod(IRBuilder* builder, IRType* origType, IRStructKey* key);

    // Lookup and return the 'Differential' type declared in the concrete type
    // in order to conform to the IDifferentiable interface.
    // Note that inside a generic block, this will be a witness table lookup instruction
    // that gets resolved during the specialization pass.
    // 
    IRInst* getDifferentialForType(IRBuilder* builder, IRType* origType)
    {
        switch (origType->getOp())
        {
        case kIROp_FloatType:
        case kIROp_HalfType:
        case kIROp_DoubleType:
        case kIROp_VectorType:
            return origType;
        }
        return lookUpInterfaceMethod(builder, origType, sharedContext->differentialAssocTypeStructKey);
    }

    IRInst* getZeroMethodForType(IRBuilder* builder, IRType* origType)
    {
        return lookUpInterfaceMethod(builder, origType, sharedContext->zeroMethodStructKey);
    }

    IRInst* getAddMethodForType(IRBuilder* builder, IRType* origType)
    {
        return lookUpInterfaceMethod(builder, origType, sharedContext->addMethodStructKey);
    }

};

struct DifferentialPairTypeBuilder
{
    struct LoweredPairTypeInfo
    {
        IRInst* loweredType;
        bool isTrivial;
    };

    DifferentialPairTypeBuilder() = default;

    DifferentialPairTypeBuilder(AutoDiffSharedContext* sharedContext) : sharedContext(sharedContext) {}

    IRStructField* findField(IRInst* type, IRStructKey* key);

    IRInst* findSpecializationForParam(IRInst* specializeInst, IRInst* genericParam);

    IRInst* emitFieldAccessor(IRBuilder* builder, IRInst* baseInst, IRStructKey* key);

    IRInst* emitPrimalFieldAccess(IRBuilder* builder, IRInst* baseInst);

    IRInst* emitDiffFieldAccess(IRBuilder* builder, IRInst* baseInst);

    IRStructKey* _getOrCreateDiffStructKey();

    IRStructKey* _getOrCreatePrimalStructKey();

    IRInst* _createDiffPairType(IRType* origBaseType, IRType* diffType);

    IRInst* getDiffTypeFromPairType(IRBuilder* builder, IRDifferentialPairType* type);

    IRInst* getDiffTypeWitnessFromPairType(IRBuilder* builder, IRDifferentialPairType* type);

    LoweredPairTypeInfo lowerDiffPairType(IRBuilder* builder, IRType* originalPairType);


    Dictionary<IRInst*, LoweredPairTypeInfo> pairTypeCache;

    IRStructKey* globalPrimalKey = nullptr;

    IRStructKey* globalDiffKey = nullptr;

    IRInst* genericDiffPairType = nullptr;

    List<IRInst*> generatedTypeList;

    AutoDiffSharedContext* sharedContext = nullptr;
};

void stripAutoDiffDecorations(IRModule* module);

IRInst* _lookupWitness(IRBuilder* builder, IRInst* witness, IRInst* requirementKey);

struct IRAutodiffPassOptions
{
    // Nothing for now...
};

bool processAutodiffCalls(
    IRModule*                           module,
    DiagnosticSink*                     sink,
    IRAutodiffPassOptions const&   options = IRAutodiffPassOptions());

};