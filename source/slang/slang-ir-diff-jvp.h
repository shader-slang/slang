// slang-ir-diff-jvp.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-compiler.h"

namespace Slang
{
    template<typename P, typename D>
    struct DiffInstPair
    {
        P primal;
        D differential;
        DiffInstPair() = default;
        DiffInstPair(P primal, D differential) : primal(primal), differential(differential)
        {}
        HashCode getHashCode() const
        {
            Hasher hasher;
            hasher << primal << differential;
            return hasher.getResult();
        }
        bool operator ==(const DiffInstPair& other) const
        {
            return primal == other.primal && differential == other.differential;
        }
    };

    typedef DiffInstPair<IRInst*, IRInst*> InstPair;

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

    struct IRJVPDerivativePassOptions
    {
        // Nothing for now..
    };

    bool processDifferentiableFuncs(
        IRModule*                           module,
        DiagnosticSink*                     sink,
        IRJVPDerivativePassOptions const&   options = IRJVPDerivativePassOptions());

    void stripAutoDiffDecorations(IRModule* module);
}
