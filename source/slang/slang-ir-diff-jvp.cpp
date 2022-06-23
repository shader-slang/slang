// slang-ir-diff-jvp.cpp
#include "slang-ir-diff-jvp.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{

struct JVPDerivativeContext
{
    // This type passes over the module and generates
    // forward-mode derivative versions of functions 
    // that are explicitly marked for it.
    //
    IRModule*                       module;

    // Shared builder state for our derivative passes.
    SharedIRBuilder                sharedBuilderStorage;

    bool processModule()
    {
        // We start by initializing our shared IR building state,
        // since we will re-use that state for any code we
        // generate along the way.
        //
        SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
        sharedBuilder->init(module);

        // Run through all the global-level instructions, 
        // looking for callables.
        // Note: We're only processing global callables (IRGlobalValueWithCode)
        // for now.
        IRBuilder builderStorage(sharedBuilderStorage);
        IRBuilder* builder = &builderStorage;
        for (auto inst : module->getGlobalInsts())
        {
            // If the instr is a callable, get all the basic blocks
            if (auto callable = as<IRGlobalValueWithCode>(inst))
            {
                if (isFunctionMarkedForJVP(callable))
                {   
                    SLANG_ASSERT(as<IRFunc>(callable));
                    IRFunc* jvpFunction = emitJVPFunction(&builderStorage, as<IRFunc>(callable));
                    builder->addJVPDerivativeReferenceDecoration(callable, jvpFunction);
                }
            }
        }
        return true;
    }

    // Checks decorators to see if the function should
    // be differentiated (kIROp_JVPDerivativeMarkerDecoration)
    // 
    bool isFunctionMarkedForJVP(IRGlobalValueWithCode* callable)
    {
        for(auto decoration = callable->getFirstDecoration(); 
            decoration;
            decoration = decoration->getNextDecoration())
        {
            if (decoration->getOp() == kIROp_JVPDerivativeMarkerDecoration)
            {
                return true;
            }
            // TODO: Need to remove this decoration or check for 
            // JVPDerivativeReferenceDecoration to avoid re-generating code.
        }
        return false;
    }

    // Perform forward-mode automatic differentiation on 
    // the intstructions.
    IRFunc* emitJVPFunction(IRBuilder* builder,
                            IRFunc*    primalFn)
    {
        // Note (sai): Is this safe? Should we use setInsertInto?
        builder->setInsertBefore(primalFn->getNextInst()); 

        auto jvpFn = builder->createFunc();
        jvpFn->setFullType(primalTypeToJVPType(primalFn->getFullType()));
        if (auto jvpName = getJVPFuncName(builder, primalFn))
            builder->addNameHintDecoration(jvpFn, jvpName);

        builder->setInsertInto(jvpFn);

        // Start with _extremely_ basic functions
        SLANG_ASSERT(primalFn->getFirstBlock() == primalFn->getLastBlock());

        // TODO: Need to emit parameters if it's the first block.
        emitJVPBlock(builder, primalFn->getFirstBlock());

        return jvpFn;
    }

    IRStringLit* getJVPFuncName(IRBuilder*    builder,
                                IRFunc*       func)
    {
        auto oldLoc = builder->getInsertLoc();
        builder->setInsertBefore(func);
        
        IRStringLit* name = nullptr;
        if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
        {
            name = builder->getStringValue((String(linkageDecoration->getMangledName()) + "_jvp").getUnownedSlice());
        }
        else if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
        {
            name = builder->getStringValue((String(namehintDecoration->getName()) + "_jvp").getUnownedSlice());
        }

        builder->setInsertLoc(oldLoc);

        return name;
    }


    IRBlock* emitJVPBlock(IRBuilder*    builder, 
                          IRBlock*      primalBlock)
    {
        // Create and insert into new block.
        auto jvpBlock = builder->emitBlock();

        // Temporarily, we're going to just emit a single return 0 instruction.
        for(auto child = primalBlock->getFirstInst(); child; child = child->getNextInst())
        {
            if (auto returnOp = as<IRReturn>(child))
            {
                auto zeroVal = builder->getFloatValue(returnOp->getVal()->getDataType(), 0.0);
                builder->emitReturn(zeroVal);
            }
        }

        return jvpBlock;
    }

    IRType* primalTypeToJVPType(IRType* primalType)
    {
        // Temporarily, we're going to implement the identity transform.
        // The return type is the same as the primal type.
        return primalType;
    }
};

// Set up context and call main process method.
//
bool processJVPDerivativeMarkers(
        IRModule*                           module,
        IRJVPDerivativePassOptions const&)
{
    JVPDerivativeContext context;
    context.module = module;

    return context.processModule();
}

}
