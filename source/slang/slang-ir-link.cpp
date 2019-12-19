// slang-ir-link.cpp
#include "slang-ir-link.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-mangle.h"
#include "slang-ir-string-hash.h"

namespace Slang
{

    /// Find a suitable layout for `entryPoint` in `programLayout`.
    ///
    /// TODO: This function should be eliminated. See its body
    /// for an explanation of the problems.
EntryPointLayout* findEntryPointLayout(
    ProgramLayout*          programLayout,
    EntryPoint*             entryPoint);

struct IRSpecSymbol : RefObject
{
    IRInst*                 irGlobalValue;
    RefPtr<IRSpecSymbol>    nextWithSameName;
};

struct IRSpecEnv
{
    IRSpecEnv*  parent = nullptr;

    // A map from original values to their cloned equivalents.
    typedef Dictionary<IRInst*, IRInst*> ClonedValueDictionary;
    ClonedValueDictionary clonedValues;
};

struct IRSharedSpecContext
{
    // The code-generation target in use
    CodeGenTarget target;

    // The specialized module we are building
    RefPtr<IRModule>   module;

    // A map from mangled symbol names to zero or
    // more global IR values that have that name,
    // in the *original* module.
    typedef Dictionary<String, RefPtr<IRSpecSymbol>> SymbolDictionary;
    SymbolDictionary symbols;

    SharedIRBuilder sharedBuilderStorage;
    IRBuilder builderStorage;

    // The "global" specialization environment.
    IRSpecEnv globalEnv;
};

struct IRSpecContextBase
{
    IRSharedSpecContext* shared;

    IRSharedSpecContext* getShared() { return shared; }

    IRModule* getModule() { return getShared()->module; }

    IRSharedSpecContext::SymbolDictionary& getSymbols() { return getShared()->symbols; }

    // The current specialization environment to use.
    IRSpecEnv* env = nullptr;
    IRSpecEnv* getEnv()
    {
        // TODO: need to actually establish environments on contexts we create.
        //
        // Or more realistically we need to change the whole approach
        // to specialization and cloning so that we don't try to share
        // logic between two very different cases.


        return env;
    }

    // The IR builder to use for creating nodes
    IRBuilder*  builder;

    // A callback to be used when a value that is not registerd in `clonedValues`
    // is needed during cloning. This gives the subtype a chance to intercept
    // the operation and clone (or not) as needed.
    virtual IRInst* maybeCloneValue(IRInst* originalVal)
    {
        return originalVal;
    }
};

void registerClonedValue(
    IRSpecContextBase*  context,
    IRInst*    clonedValue,
    IRInst*    originalValue)
{
    if(!originalValue)
        return;

    // TODO: now that things are scoped using environments, we
    // shouldn't be running into the cases where a value with
    // the same key already exists. This should be changed to
    // an `Add()` call.
    //
    context->getEnv()->clonedValues[originalValue] = clonedValue;
}

// Information on values to use when registering a cloned value
struct IROriginalValuesForClone
{
    IRInst*        originalVal = nullptr;
    IRSpecSymbol*   sym = nullptr;

    IROriginalValuesForClone() {}

    IROriginalValuesForClone(IRInst* originalValue)
        : originalVal(originalValue)
    {}

    IROriginalValuesForClone(IRSpecSymbol* symbol)
        : sym(symbol)
    {}
};

void registerClonedValue(
    IRSpecContextBase*              context,
    IRInst*                        clonedValue,
    IROriginalValuesForClone const& originalValues)
{
    registerClonedValue(context, clonedValue, originalValues.originalVal);
    for( auto s = originalValues.sym; s; s = s->nextWithSameName )
    {
        registerClonedValue(context, clonedValue, s->irGlobalValue);
    }
}

IRInst* cloneInst(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRInst*                         originalInst,
    IROriginalValuesForClone const& originalValues);

IRInst* cloneInst(
    IRSpecContextBase*  context,
    IRBuilder*          builder,
    IRInst*             originalInst)
{
    return cloneInst(context, builder, originalInst, originalInst);
}

    /// Clone any decorations from `originalValue` onto `clonedValue`
void cloneDecorations(
    IRSpecContextBase*  context,
    IRInst*             clonedValue,
    IRInst*             originalValue)
{
    // TODO: In many cases we might be able to use this as a general-purpose
    // place to do cloning of *all* the children of an instruction, and
    // not just its decorations. We should look to refactor this code
    // later.

    IRBuilder builderStorage = *context->builder;
    IRBuilder* builder = &builderStorage;
    builder->setInsertInto(clonedValue);


    SLANG_UNUSED(context);
    for(auto originalDecoration : originalValue->getDecorations())
    {
        cloneInst(context, builder, originalDecoration);
    }

    // We will also clone the location here, just because this is a convenient bottleneck
    clonedValue->sourceLoc = originalValue->sourceLoc;
}

    /// Clone any decorations and children from `originalValue` onto `clonedValue`
void cloneDecorationsAndChildren(
    IRSpecContextBase*  context,
    IRInst*             clonedValue,
    IRInst*             originalValue)
{
    IRBuilder builderStorage = *context->builder;
    IRBuilder* builder = &builderStorage;
    builder->setInsertInto(clonedValue);

    SLANG_UNUSED(context);
    for(auto originalItem : originalValue->getDecorationsAndChildren())
    {
        cloneInst(context, builder, originalItem);
    }

    // We will also clone the location here, just because this is a convenient bottleneck
    clonedValue->sourceLoc = originalValue->sourceLoc;
}

// We use an `IRSpecContext` for the case where we are cloning
// code from one or more input modules to create a "linked" output
// module. Along the way, we will resolve profile-specific functions
// to the best definition for a given target.
//
struct IRSpecContext : IRSpecContextBase
{
    // Override the "maybe clone" logic so that we always clone
    virtual IRInst* maybeCloneValue(IRInst* originalVal) override;
};


IRInst* cloneGlobalValue(IRSpecContext* context, IRInst* originalVal);

IRInst* cloneValue(
    IRSpecContextBase*  context,
    IRInst*        originalValue);

IRType* cloneType(
    IRSpecContextBase*  context,
    IRType*             originalType);

IRInst* IRSpecContext::maybeCloneValue(IRInst* originalValue)
{
    switch (originalValue->op)
    {
    case kIROp_StructType:
    case kIROp_Func:
    case kIROp_Generic:
    case kIROp_GlobalVar:
    case kIROp_GlobalParam:
    case kIROp_StructKey:
    case kIROp_GlobalGenericParam:
    case kIROp_WitnessTable:
    case kIROp_TaggedUnionType:
        return cloneGlobalValue(this, originalValue);

    case kIROp_BoolLit:
        {
            IRConstant* c = (IRConstant*)originalValue;
            return builder->getBoolValue(c->value.intVal != 0);
        }
        break;


    case kIROp_IntLit:
        {
            IRConstant* c = (IRConstant*)originalValue;
            return builder->getIntValue(cloneType(this, c->getDataType()), c->value.intVal);
        }
        break;

    case kIROp_FloatLit:
        {
            IRConstant* c = (IRConstant*)originalValue;
            return builder->getFloatValue(cloneType(this, c->getDataType()), c->value.floatVal);
        }
        break;

    case kIROp_StringLit:
        {
            IRConstant* c = (IRConstant*)originalValue;
            return builder->getStringValue(c->getStringSlice());
        }
        break;

    case kIROp_PtrLit:
        {
            IRConstant* c = (IRConstant*)originalValue;
            return builder->getPtrValue(c->value.ptrVal);
        }
        break;

    default:
        {
            // In the deafult case, assume that we have some sort of "hoistable"
            // instruction that requires us to create a clone of it.

            UInt argCount = originalValue->getOperandCount();
            IRInst* clonedValue = builder->createIntrinsicInst(
                cloneType(this, originalValue->getFullType()),
                originalValue->op,
                argCount, nullptr);
            registerClonedValue(this, clonedValue, originalValue);
            for (UInt aa = 0; aa < argCount; ++aa)
            {
                IRInst* originalArg = originalValue->getOperand(aa);
                IRInst* clonedArg = cloneValue(this, originalArg);
                clonedValue->getOperands()[aa].init(clonedValue, clonedArg);
            }
            cloneDecorationsAndChildren(this, clonedValue, originalValue);

            addHoistableInst(builder, clonedValue);

            return clonedValue;
        }
        break;
    }
}

IRInst* cloneValue(
    IRSpecContextBase*  context,
    IRInst*        originalValue);

// Find a pre-existing cloned value, or return null if none is available.
IRInst* findClonedValue(
    IRSpecContextBase*  context,
    IRInst*        originalValue)
{
    IRInst* clonedValue = nullptr;
    for (auto env = context->getEnv(); env; env = env->parent)
    {
        if (env->clonedValues.TryGetValue(originalValue, clonedValue))
        {
            return clonedValue;
        }
    }

    return nullptr;
}

IRInst* cloneValue(
    IRSpecContextBase*  context,
    IRInst*        originalValue)
{
    if (!originalValue)
        return nullptr;

    if (IRInst* clonedValue = findClonedValue(context, originalValue))
        return clonedValue;

    return context->maybeCloneValue(originalValue);
}

IRType* cloneType(
    IRSpecContextBase*  context,
    IRType*             originalType)
{
    return (IRType*)cloneValue(context, originalType);
}

void cloneGlobalValueWithCodeCommon(
    IRSpecContextBase*              context,
    IRGlobalValueWithCode*          clonedValue,
    IRGlobalValueWithCode*          originalValue,
    IROriginalValuesForClone const& originalValues);

IRRate* cloneRate(
    IRSpecContextBase*  context,
    IRRate*             rate)
{
    return (IRRate*) cloneType(context, rate);
}

void maybeSetClonedRate(
    IRSpecContextBase*  context,
    IRBuilder*          builder,
    IRInst*             clonedValue,
    IRInst*             originalValue)
{
    if(auto rate = originalValue->getRate() )
    {
        clonedValue->setFullType(builder->getRateQualifiedType(
            cloneRate(context, rate),
            clonedValue->getFullType()));
    }
}

IRGlobalVar* cloneGlobalVarImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRGlobalVar*                    originalVar,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedVar = builder->createGlobalVar(
        cloneType(context, originalVar->getDataType()->getValueType()));

    maybeSetClonedRate(context, builder, clonedVar, originalVar);

    registerClonedValue(context, clonedVar, originalValues);

    // Clone any code in the body of the variable, since this
    // represents the initializer.
    cloneGlobalValueWithCodeCommon(
        context,
        clonedVar,
        originalVar,
        originalValues);

    return clonedVar;
}

    /// Clone certain special decorations for `clonedInst` from its (potentially multiple) definitions.
    ///
    /// In most cases, once we've decided on the "best" definition to use for an IR instruction,
    /// we only want the linking process to use the decorations from the single best definition.
    /// In some casses, though, the canonical best definition might not have all the information.
    ///
    /// A concrete example is the `[bindExistentialsSlots(...)]` decorations for global shader
    /// parameters and entry points. These decorations are only generated as part of the IR
    /// associated with a specialization of a program, and not the original IR for the modules
    /// of the program.
    ///
    /// This function scans through all the `originalValues` that were considered for `clonedInst`,
    /// and copies over any decorations that are allowed to come from a non-"best" definition.
    /// For a given decoration opcode, only one such decoration will ever be copied, and nothing
    /// will be copied if the instruction already has a matching decoration (that was cloned
    /// from the "best" definition).
    /// 
static void cloneExtraDecorations(
    IRSpecContextBase*              context,
    IRInst*                         clonedInst,
    IROriginalValuesForClone const& originalValues)
{
    IRBuilder builderStorage = *context->builder;
    IRBuilder* builder = &builderStorage;
    builder->setInsertInto(clonedInst);

    // If the `clonedInst` already has any non-decoration
    // children, then we want to insert before those,
    // to maintain the invariant that decorations always
    // precede non-decoration instructions in the list of
    // decorations and children.
    //
    if(auto firstChild = clonedInst->getFirstChild())
    {
        builder->setInsertBefore(firstChild);
    }

    for(auto sym = originalValues.sym; sym; sym = sym->nextWithSameName)
    {
        for(auto decoration : sym->irGlobalValue->getDecorations())
        {
            switch(decoration->op)
            {
            default:
                break;

            case kIROp_BindExistentialSlotsDecoration:
            case kIROp_LayoutDecoration:
                if(!clonedInst->findDecorationImpl(decoration->op))
                {
                    cloneInst(context, builder, decoration);
                }
                break;
            }
        }

        // We will also copy over source location information from the alternative
        // values, in case any of them has it available.
        //
        if(sym->irGlobalValue->sourceLoc.isValid() && !clonedInst->sourceLoc.isValid())
        {
            clonedInst->sourceLoc = sym->irGlobalValue->sourceLoc;
        }
    }
}

void cloneSimpleGlobalValueImpl(
    IRSpecContextBase*              context,
    IRInst*                         originalInst,
    IROriginalValuesForClone const& originalValues,
    IRInst*                         clonedInst,
    bool                            registerValue = true)
{
    if (registerValue)
        registerClonedValue(context, clonedInst, originalValues);

    // Set up an IR builder for inserting into the inst
    IRBuilder builderStorage = *context->builder;
    IRBuilder* builder = &builderStorage;
    builder->setInsertInto(clonedInst);

    // Clone any children of the instruction
    for (auto child : originalInst->getDecorationsAndChildren())
    {
        cloneInst(context, builder, child);
    }

    // Also clone certain decorations if they appear on *any*
    // definition of the symbol (not necessarily the one
    // we picked as the primary/best).
    //
    cloneExtraDecorations(context, clonedInst, originalValues);
}

IRGlobalParam* cloneGlobalParamImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRGlobalParam*                  originalVal,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedVal = builder->createGlobalParam(
        cloneType(context, originalVal->getFullType()));
    cloneSimpleGlobalValueImpl(context, originalVal, originalValues, clonedVal);

    return clonedVal;
}

IRGlobalConstant* cloneGlobalConstantImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRGlobalConstant*               originalVal,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedType = cloneType(context, originalVal->getFullType());
    IRGlobalConstant* clonedVal = nullptr;
    if(auto originalInitVal = originalVal->getValue())
    {
        auto clonedInitVal = cloneValue(context, originalInitVal);
        clonedVal = builder->emitGlobalConstant(clonedType, clonedInitVal);
    }
    else
    {
        clonedVal = builder->emitGlobalConstant(clonedType);
    }

    cloneSimpleGlobalValueImpl(context, originalVal, originalValues, clonedVal);

    return clonedVal;
}

IRGeneric* cloneGenericImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRGeneric*                      originalVal,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedVal = builder->emitGeneric();
    registerClonedValue(context, clonedVal, originalValues);

    // Clone any code in the body of the generic, since this
    // computes its result value.
    cloneGlobalValueWithCodeCommon(
        context,
        clonedVal,
        originalVal,
        originalValues);

    return clonedVal;
}

IRStructKey* cloneStructKeyImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRStructKey*                    originalVal,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedVal = builder->createStructKey();
    cloneSimpleGlobalValueImpl(context, originalVal, originalValues, clonedVal);
    return clonedVal;
}

IRGlobalGenericParam* cloneGlobalGenericParamImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRGlobalGenericParam*           originalVal,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedVal = builder->emitGlobalGenericParam(originalVal->getFullType());
    cloneSimpleGlobalValueImpl(context, originalVal, originalValues, clonedVal);
    return clonedVal;
}


IRWitnessTable* cloneWitnessTableImpl(
    IRSpecContextBase*  context,
    IRBuilder*          builder,
    IRWitnessTable* originalTable,
    IROriginalValuesForClone const& originalValues,
    IRWitnessTable* dstTable = nullptr,
    bool registerValue = true)
{
    auto clonedTable = dstTable ? dstTable : builder->createWitnessTable();
    cloneSimpleGlobalValueImpl(context, originalTable, originalValues, clonedTable, registerValue);
    return clonedTable;
}

IRWitnessTable* cloneWitnessTableWithoutRegistering(
    IRSpecContextBase*  context,
    IRBuilder*          builder,
    IRWitnessTable* originalTable,
    IRWitnessTable* dstTable = nullptr)
{
    return cloneWitnessTableImpl(context, builder, originalTable, IROriginalValuesForClone(), dstTable, false);
}

IRStructType* cloneStructTypeImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRStructType*                   originalStruct,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedStruct = builder->createStructType();
    cloneSimpleGlobalValueImpl(context, originalStruct, originalValues, clonedStruct);
    return clonedStruct;
}


IRInterfaceType* cloneInterfaceTypeImpl(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRInterfaceType*                originalInterface,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedInterface = builder->createInterfaceType();
    cloneSimpleGlobalValueImpl(context, originalInterface, originalValues, clonedInterface);
    return clonedInterface;
}

void cloneGlobalValueWithCodeCommon(
    IRSpecContextBase*      context,
    IRGlobalValueWithCode*  clonedValue,
    IRGlobalValueWithCode*  originalValue,
    IROriginalValuesForClone const& originalValues)
{
    // Next we are going to clone the actual code.
    IRBuilder builderStorage = *context->builder;
    IRBuilder* builder = &builderStorage;
    builder->setInsertInto(clonedValue);

    cloneDecorations(context, clonedValue, originalValue);
    cloneExtraDecorations(context, clonedValue, originalValues);

    // We will walk through the blocks of the function, and clone each of them.
    //
    // We need to create the cloned blocks first, and then walk through them,
    // because blocks might be forward referenced (this is not possible
    // for other cases of instructions).
    for (auto originalBlock = originalValue->getFirstBlock();
        originalBlock;
        originalBlock = originalBlock->getNextBlock())
    {
        IRBlock* clonedBlock = builder->createBlock();
        clonedValue->addBlock(clonedBlock);
        registerClonedValue(context, clonedBlock, originalBlock);

#if 0
        // We can go ahead and clone parameters here, while we are at it.
        builder->curBlock = clonedBlock;
        for (auto originalParam = originalBlock->getFirstParam();
            originalParam;
            originalParam = originalParam->getNextParam())
        {
            IRParam* clonedParam = builder->emitParam(
                context->maybeCloneType(
                    originalParam->getFullType()));
            cloneDecorations(context, clonedParam, originalParam);
            registerClonedValue(context, clonedParam, originalParam);
        }
#endif
    }

    // Okay, now we are in a good position to start cloning
    // the instructions inside the blocks.
    {
        IRBlock* ob = originalValue->getFirstBlock();
        IRBlock* cb = clonedValue->getFirstBlock();
        while (ob)
        {
            SLANG_ASSERT(cb);

            builder->setInsertInto(cb);
            for (auto oi = ob->getFirstInst(); oi; oi = oi->getNextInst())
            {
                cloneInst(context, builder, oi);
            }

            ob = ob->getNextBlock();
            cb = cb->getNextBlock();
        }
    }

}

void checkIRDuplicate(IRInst* inst, IRInst* moduleInst, UnownedStringSlice const& mangledName)
{
#ifdef _DEBUG
    for (auto child : moduleInst->getDecorationsAndChildren())
    {
        if (child == inst)
            continue;

        if(auto childLinkage = child->findDecoration<IRLinkageDecoration>())
        {
            if(mangledName == childLinkage->getMangledName())
            {
                SLANG_UNEXPECTED("duplicate global instruction");
            }
        }
    }
#else
    SLANG_UNREFERENCED_PARAMETER(inst);
    SLANG_UNREFERENCED_PARAMETER(moduleInst);
    SLANG_UNREFERENCED_PARAMETER(mangledName);
#endif
}

void cloneFunctionCommon(
    IRSpecContextBase*              context,
    IRFunc*                         clonedFunc,
    IRFunc*                         originalFunc,
    IROriginalValuesForClone const& originalValues,
    bool                            checkDuplicate = true)
{
    // First clone all the simple properties.
    clonedFunc->setFullType(cloneType(context, originalFunc->getFullType()));

    cloneGlobalValueWithCodeCommon(
        context,
        clonedFunc,
        originalFunc,
        originalValues);

    // Shuffle the function to the end of the list, because
    // it needs to follow its dependencies.
    //
    // TODO: This isn't really a good requirement to place on the IR...
    clonedFunc->moveToEnd();

    if( checkDuplicate )
    {
        if( auto linkage = clonedFunc->findDecoration<IRLinkageDecoration>() )
        {
            checkIRDuplicate(clonedFunc, context->getModule()->getModuleInst(), linkage->getMangledName());
        }
    }
}

// We will forward-declare the subroutine for eagerly specializing
// an IR-level generic to argument values, because `specializeIRForEntryPoint`
// needs to perform this operation even though it is logically part of
// the later generic specialization pass.
//
IRInst* specializeGeneric(
    IRSpecialize*   specializeInst);

    /// Copy layout information for an entry-point function to its parameters.
    ///
    /// When layout information is initially attached to an IR entry point,
    /// it may be attached to a declaration that would have no `IRParam`s
    /// to represent the entry-point parameters.
    ///
    /// After linking, we expect an entry point to have a full definition,
    /// so it becomes possible to copy per-parameter layout information
    /// from the entry-point layout down to the individual parameters,
    /// which simplifies subsequent IR steps taht want to look for
    /// per-parameter layout information.
    ///
    /// TODO: This step should probably be hoisted out to be an independent
    /// IR pass that runs after linking, rather than being folded into
    /// the linking step.
    ///
static void maybeCopyLayoutInformationToParameters(
    IRFunc* func,
    IRBuilder* builder)
{
    auto layoutDecor = func->findDecoration<IRLayoutDecoration>();
    if(!layoutDecor)
        return;

    auto entryPointLayout = as<IREntryPointLayout>(layoutDecor->getLayout());
    if(!entryPointLayout)
        return;

    if( auto firstBlock = func->getFirstBlock() )
    {
        auto paramsStructLayout = getScopeStructLayout(entryPointLayout);
        Index paramLayoutCount = paramsStructLayout->getFieldCount();
        Index paramCounter = 0;
        for( auto pp = firstBlock->getFirstParam(); pp; pp = pp->getNextParam() )
        {
            Index paramIndex = paramCounter++;
            if( paramIndex < paramLayoutCount )
            {
                auto paramLayout = paramsStructLayout->getFieldLayout(paramIndex);

                auto offsetParamLayout = applyOffsetToVarLayout(
                    builder,
                    paramLayout,
                    entryPointLayout->getParamsLayout());

                builder->addLayoutDecoration(
                    pp,
                    offsetParamLayout);
            }
            else
            {
                SLANG_UNEXPECTED("too many parameters");
            }
        }
    }
}

IRFunc* specializeIRForEntryPoint(
    IRSpecContext*      context,
    String const&       mangledName)
{
    // We start by looking up the IR symbol that
    // matches the mangled name given to the
    // function we want to emit.
    //
    // Note: the function decl-ref may refer to
    // a specialization of a generic function,
    // so that the mangled name of the decl-ref is
    // not the same as the mangled name of the decl.
    //
    RefPtr<IRSpecSymbol> sym;
    if (!context->getSymbols().TryGetValue(mangledName, sym))
    {
        String hashedName = getHashedName(mangledName.getUnownedSlice());

        if (!context->getSymbols().TryGetValue(hashedName, sym))
        {
            SLANG_UNEXPECTED("no matching IR symbol");
            return nullptr;
        }
    }
    
    // Note: it is possible that `sym` shows multiple
    // definitions, coming from different IR modules that
    // were input to the linking process. We don't have
    // to do anything special about that here, because
    // we can use *any* of the IR values as the starting
    // point for cloning, and `cloneGlobalValue` will
    // follow the linkage decoration and discover the
    // other values on its own.
    //
    auto originalVal = sym->irGlobalValue;

    // We will start by cloning the entry point reference
    // like any other global value.
    //
    auto clonedVal = cloneGlobalValue(context, originalVal);

    // In the case where the user is requesting a specialization
    // of a generic entry point, we have a bit of a problem.
    //
    // This function is expected to return an `IRFunc` and
    // subsequent passes expect to find, e.g., layout information
    // attached to the parameters of such a func.
    //
    // In the generic case, the `clonedValue` won't be an
    // `IRFunc`, but instead an `IRSpecialize`.
    //
    if(auto clonedSpec = as<IRSpecialize>(clonedVal))
    {
        // The Right Thing to do here is to perform some
        // amount of generic specialization, at least
        // until we get back an `IRFunc`.
        //
        // The dangerous thing is that the generic specialization
        // pass can, in principle, change the signature of
        // functions, so that attaching parameter layout
        // information *after* specialization might not work.
        //
        // The compromise we make here is to directly
        // invoke the logic for specializing a generic.
        //
        // In theory this isn't valid, because there is no
        // way we can register the specialized function we
        // create so that it would be re-used by other instantiations
        // with the same arguments (because we cannot be
        // sure the generic arguments are themselves fully specialized)
        //
        // In practice this isn't really a problem, because
        // we don't want to share the definition between
        // an entry point and an ordinary function anyway.
        //
        auto specializedClone = specializeGeneric(clonedSpec);

        // One special case we need to be aware of is that there
        // might be decorations attached to the `specialize`
        // instruction, which we then want to copy over to the
        // result of specialization.
        //
        cloneExtraDecorations(context, specializedClone, IROriginalValuesForClone(sym));

        // Now we will move to considering the specialized instruction
        // instead of the unspecialized one for all further steps.
        //
        clonedVal = specializedClone;
    }

    auto clonedFunc = as<IRFunc>(clonedVal);
    if(!clonedFunc)
    {
        SLANG_UNEXPECTED("expected entry point to be a function");
        return nullptr;
    }

    if( !clonedFunc->findDecorationImpl(kIROp_KeepAliveDecoration) )
    {
        context->builder->addKeepAliveDecoration(clonedFunc);
    }

    // We will also go on and attach layout information
    // to the function parameters, so that we have it
    // available directly on the parameters, rather
    // than having to look it up on the original entry-point layout.
    maybeCopyLayoutInformationToParameters(clonedFunc, context->builder);

    return clonedFunc;
}

// Get a string form of the target so that we can
// use it to match against target-specialization modifiers
//
// TODO: We shouldn't be using strings for this.
String getTargetName(IRSpecContext* context)
{
    switch( context->shared->target )
    {
    case CodeGenTarget::HLSL:
        return "hlsl";

    case CodeGenTarget::GLSL:
        return "glsl";

    case CodeGenTarget::CSource:
        return "c";

    case CodeGenTarget::CPPSource:
        return "cpp";

    case CodeGenTarget::CUDASource:
        return "cuda";

    case CodeGenTarget::SPIRV:
        return "spirv";


    default:
        SLANG_UNEXPECTED("unhandled case");
        UNREACHABLE_RETURN("unknown");
    }
}

// How specialized is a given declaration for the chosen target?
enum class TargetSpecializationLevel
{
    specializedForOtherTarget = 0,
    notSpecialized,
    specializedForTarget,
};

TargetSpecializationLevel getTargetSpecialiationLevel(
    IRInst*         inVal,
    String const&   targetName)
{
    // HACK: Currently the front-end is placing modifiers related
    // to target specialization on nodes like functions, even when
    // those functions are being returned by a generic. This
    // means that we need to try and inspect the value being
    // returned by the generic if we are looking at a generic.
    IRInst* val = inVal;
    while( auto genericVal = as<IRGeneric>(val) )
    {
        auto firstBlock = genericVal->getFirstBlock();
        if(!firstBlock) break;

        auto returnInst = as<IRReturnVal>(firstBlock->getLastInst());
        if(!returnInst) break;

        val = returnInst->getVal();
    }

    TargetSpecializationLevel result = TargetSpecializationLevel::notSpecialized;
    for(auto dd : val->getDecorations())
    {
        if(dd->op != kIROp_TargetDecoration)
            continue;

        auto decoration = (IRTargetDecoration*) dd;
        if(String(decoration->getTargetName()) == targetName)
            return TargetSpecializationLevel::specializedForTarget;

        result = TargetSpecializationLevel::specializedForOtherTarget;
    }

    return result;
}

// Is `newVal` marked as being a better match for our
// chosen code-generation target?
//
// TODO: there is a missing step here where we need
// to check if things are even available in the first place...
bool isBetterForTarget(
    IRSpecContext*  context,
    IRInst*         newVal,
    IRInst*         oldVal)
{
    // Anything is better than nothing..
    if (oldVal == nullptr)
    {
        return true;
    }

    String targetName = getTargetName(context);

    // For right now every declaration might have zero or more
    // modifiers, representing the targets for which it is specialized.
    // Each modifier has a single string "tag" to represent a target.
    // We thus decide that a declaration is "more specialized" by:
    //
    // - Does it have a modifier with a tag with the string for the current target?
    //   If yes, it is the most specialized it can be.
    //
    // - Does it have a no tags? Then it is "unspecialized" and that is okay.
    //
    // - Does it have a modifier with a tag for a *different* target?
    //   If yes, then it shouldn't even be usable on this target.
    //
    // Longer term a better approach is to think of this in terms
    // of a "disjunction of conjunctions" that is:
    //
    //     (A and B and C) or (A and D) or (E) or (F and G) ...
    //
    // A code generation target would then consist of a
    // conjunction of individual tags:
    //
    //    (HLSL and SM_4_0 and Vertex and ...)
    //
    // A declaration is *applicable* on a target if one of
    // its conjunctions of tags is a subset of the target's.
    //
    // One declaration is *better* than another on a target
    // if it is applicable and its tags are a superset
    // of the other's.

    auto newLevel = getTargetSpecialiationLevel(newVal, targetName);
    
    auto oldLevel = getTargetSpecialiationLevel(oldVal, targetName);
    if(newLevel != oldLevel)
        return UInt(newLevel) > UInt(oldLevel);

    // All preceding factors being equal, an `[export]` is better
    // than an `[import]`.
    //
    bool newIsExport = newVal->findDecoration<IRExportDecoration>() != nullptr;
    bool oldIsExport = oldVal->findDecoration<IRExportDecoration>() != nullptr;
    if(newIsExport != oldIsExport)
        return newIsExport;

    // All preceding factors being equal, a definition is
    // better than a declaration.
    auto newIsDef = isDefinition(newVal);
    auto oldIsDef = isDefinition(oldVal);
    if (newIsDef != oldIsDef)
        return newIsDef;

    return false;
}

IRFunc* cloneFuncImpl(
    IRSpecContextBase*  context,
    IRBuilder*          builder,
    IRFunc*             originalFunc,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedFunc = builder->createFunc();
    registerClonedValue(context, clonedFunc, originalValues);
    cloneFunctionCommon(context, clonedFunc, originalFunc, originalValues);
    return clonedFunc;
}


IRInst* cloneInst(
    IRSpecContextBase*              context,
    IRBuilder*                      builder,
    IRInst*                         originalInst,
    IROriginalValuesForClone const& originalValues)
{
    switch (originalInst->op)
    {
        // We need to special-case any instruction that is not
        // allocated like an ordinary `IRInst` with trailing args.
    case kIROp_Func:
        return cloneFuncImpl(context, builder, cast<IRFunc>(originalInst), originalValues);

    case kIROp_GlobalVar:
        return cloneGlobalVarImpl(context, builder, cast<IRGlobalVar>(originalInst), originalValues);

    case kIROp_GlobalParam:
        return cloneGlobalParamImpl(context, builder, cast<IRGlobalParam>(originalInst), originalValues);

    case kIROp_GlobalConstant:
        return cloneGlobalConstantImpl(context, builder, cast<IRGlobalConstant>(originalInst), originalValues);

    case kIROp_WitnessTable:
        return cloneWitnessTableImpl(context, builder, cast<IRWitnessTable>(originalInst), originalValues);

    case kIROp_StructType:
        return cloneStructTypeImpl(context, builder, cast<IRStructType>(originalInst), originalValues);

    case kIROp_InterfaceType:
        return cloneInterfaceTypeImpl(context, builder, cast<IRInterfaceType>(originalInst), originalValues);

    case kIROp_Generic:
        return cloneGenericImpl(context, builder, cast<IRGeneric>(originalInst), originalValues);

    case kIROp_StructKey:
        return cloneStructKeyImpl(context, builder, cast<IRStructKey>(originalInst), originalValues);

    case kIROp_GlobalGenericParam:
        return cloneGlobalGenericParamImpl(context, builder, cast<IRGlobalGenericParam>(originalInst), originalValues);

    default:
        break;
    }

    // The common case is that we just need to construct a cloned
    // instruction with the right number of operands, intialize
    // it, and then add it to the sequence.
    UInt argCount = originalInst->getOperandCount();
    IRInst* clonedInst = builder->createIntrinsicInst(
        cloneType(context, originalInst->getFullType()),
        originalInst->op,
        argCount, nullptr);
    registerClonedValue(context, clonedInst, originalValues);
    auto oldBuilder = context->builder;
    context->builder = builder;
    for (UInt aa = 0; aa < argCount; ++aa)
    {
        IRInst* originalArg = originalInst->getOperand(aa);
        IRInst* clonedArg = cloneValue(context, originalArg);
        clonedInst->getOperands()[aa].init(clonedInst, clonedArg);
    }
    builder->addInst(clonedInst);
    context->builder = oldBuilder;
    cloneDecorations(context, clonedInst, originalInst);
    cloneExtraDecorations(context, clonedInst, originalValues);

    return clonedInst;
}

IRInst* cloneGlobalValueImpl(
    IRSpecContext*                  context,
    IRInst*                         originalInst,
    IROriginalValuesForClone const& originalValues)
{
    auto clonedValue = cloneInst(context, &context->shared->builderStorage, originalInst, originalValues);
    clonedValue->moveToEnd();
    return clonedValue;
}


    /// Clone a global value, which has the given `originalLinkage`.
    ///
    /// The `originalVal` is a known global IR value with that linkage, if one is available.
    /// (It is okay for this parameter to be null).
    ///
IRInst* cloneGlobalValueWithLinkage(
    IRSpecContext*          context,
    IRInst*                 originalVal,
    IRLinkageDecoration*    originalLinkage)
{
    // If the global value being cloned is already in target module, don't clone
    // Why checking this?
    //   When specializing a generic function G (which is already in target module),
    //   where G calls a normal function F (which is already in target module),
    //   then when we are making a copy of G via cloneFuncCommom(), it will recursively clone F,
    //   however we don't want to make a duplicate of F in the target module.
    if (originalVal->getParent() == context->getModule()->getModuleInst())
        return originalVal;

    // Check if we've already cloned this value, for the case where
    // an original value has already been established.
    if (originalVal)
    {
        if (IRInst* clonedVal = findClonedValue(context, originalVal))
        {
            return clonedVal;
        }
    }

    if(!originalLinkage)
    {
        // If there is no mangled name, then we assume this is a local symbol,
        // and it can't possibly have multiple declarations.
        return cloneGlobalValueImpl(context, originalVal, IROriginalValuesForClone(originalVal));
    }

    //
    // We will scan through all of the available declarations
    // with the same mangled name as `originalVal` and try
    // to pick the "best" one for our target.

    auto mangledName = String(originalLinkage->getMangledName());
    RefPtr<IRSpecSymbol> sym;
    if( !context->getSymbols().TryGetValue(mangledName, sym) )
    {
        if(!originalVal)
            return nullptr;

        // This shouldn't happen!
        SLANG_UNEXPECTED("no matching values registered");
        UNREACHABLE_RETURN(cloneGlobalValueImpl(context, originalVal, IROriginalValuesForClone()));
    }

    // We will try to track the "best" declaration we can find.
    //
    // Generally, one declaration will be better than another if it is
    // more specialized for the chosen target. Otherwise, we simply favor
    // definitions over declarations.
    //
    IRInst* bestVal = nullptr;
    for(IRSpecSymbol* ss = sym; ss; ss = ss->nextWithSameName )
    {
        IRInst* newVal = ss->irGlobalValue;
        if(isBetterForTarget(context, newVal, bestVal))
            bestVal = newVal;
    }

    if (!bestVal)
    {
        return nullptr;
    }

    // Check if we've already cloned this value, for the case where
    // we didn't have an original value (just a name), but we've
    // now found a representative value.
    if (!originalVal)
    {
        if (IRInst* clonedVal = findClonedValue(context, bestVal))
        {
            return clonedVal;
        }
    }

    return cloneGlobalValueImpl(context, bestVal, IROriginalValuesForClone(sym));
}

// Clone a global value, where `originalVal` is one declaration/definition, but we might
// have to consider others, in order to find the "best" version of the symbol.
IRInst* cloneGlobalValue(IRSpecContext* context, IRInst* originalVal)
{
    // We are being asked to clone a particular global value, but in
    // the IR that comes out of the front-end there could still
    // be multiple, target-specific, declarations of any given
    // global value, all of which share the same mangled name.
    return cloneGlobalValueWithLinkage(
        context,
        originalVal,
        originalVal->findDecoration<IRLinkageDecoration>());
}

void insertGlobalValueSymbol(
    IRSharedSpecContext*    sharedContext,
    IRInst*                 gv)
{
    auto linkage = gv->findDecoration<IRLinkageDecoration>();

    // Don't try to register a symbol for global values
    // that don't have linkage.
    //
    if (!linkage)
        return;

    auto mangledName = String(linkage->getMangledName());

    RefPtr<IRSpecSymbol> sym = new IRSpecSymbol();
    sym->irGlobalValue = gv;

    RefPtr<IRSpecSymbol> prev;
    if (sharedContext->symbols.TryGetValue(mangledName, prev))
    {
        sym->nextWithSameName = prev->nextWithSameName;
        prev->nextWithSameName = sym;
    }
    else
    {
        sharedContext->symbols.Add(mangledName, sym);
    }
}

void insertGlobalValueSymbols(
    IRSharedSpecContext*    sharedContext,
    IRModule*               originalModule)
{
    if (!originalModule)
        return;

    for(auto ii : originalModule->getGlobalInsts())
    {
        insertGlobalValueSymbol(sharedContext, ii);
    }
}

void initializeSharedSpecContext(
    IRSharedSpecContext*    sharedContext,
    Session*                session,
    IRModule*               module,
    CodeGenTarget           target)
{

    SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
    sharedBuilder->module = nullptr;
    sharedBuilder->session = session;

    IRBuilder* builder = &sharedContext->builderStorage;
    builder->sharedBuilder = sharedBuilder;

    if( !module )
    {
        module = builder->createModule();
    }

    sharedBuilder->module = module;
    sharedContext->module = module;
    sharedContext->target = target;
}

struct IRSpecializationState
{
    CodeGenTarget       target;
    TargetRequest*      targetReq;

    IRModule* irModule = nullptr;

    IRSharedSpecContext sharedContextStorage;
    IRSpecContext contextStorage;

    IRSpecEnv globalEnv;

    IRSharedSpecContext* getSharedContext() { return &sharedContextStorage; }
    IRSpecContext* getContext() { return &contextStorage; }

    IRSpecializationState()
    {
        contextStorage.env = &globalEnv;
    }

    ~IRSpecializationState()
    {
        contextStorage = IRSpecContext();
        sharedContextStorage = IRSharedSpecContext();
    }
};

LinkedIR linkIR(
    BackEndCompileRequest*  compileRequest,
    Int                     entryPointIndex,
    CodeGenTarget           target,
    TargetProgram*          targetProgram)
{
    auto targetReq = targetProgram->getTargetReq();

    // TODO: We need to make sure that the program we are being asked
    // to compile has been "resolved" so that it has no outstanding
    // unsatisfied requirements.

    IRSpecializationState stateStorage;
    auto state = &stateStorage;

    state->target = target;
    state->targetReq = targetReq;

    auto program = compileRequest->getProgram();

    auto sharedContext = state->getSharedContext();
    initializeSharedSpecContext(
        sharedContext,
        compileRequest->getSession(),
        nullptr,
        target);

    state->irModule = sharedContext->module;

    auto linkage = compileRequest->getLinkage();

    // We need to be able to look up IR definitions for any symbols in
    // modules that the program depends on (transitively). To
    // accelerate lookup, we will create a symbol table for looking
    // up IR definitions by their mangled name.
    //

    List<IRModule*> irModules;
    program->enumerateIRModules([&](IRModule* irModule)
    {
        irModules.add(irModule);
    });
    irModules.addRange(linkage->m_libModules.getBuffer()->readRef(), linkage->m_libModules.getCount());

    
    // Add any modules that were loaded as libraries
    for (IRModule* irModule : irModules)
    {
        insertGlobalValueSymbols(sharedContext, irModule);
    }

    // We will also insert the IR global symbols from the IR module
    // attached to the `TargetProgram`, since this module is
    // responsible for associating layout information to those
    // global symbols via decorations.
    //
    insertGlobalValueSymbols(sharedContext, targetProgram->getExistingIRModuleForLayout());

    auto context = state->getContext();

    // Combine all of the contents of IRGlobalHashedStringLiterals
    {
        StringSlicePool pool(StringSlicePool::Style::Empty);
        IRBuilder& builder = sharedContext->builderStorage;
        for (IRModule* irModule : irModules)
        {
            findGlobalHashedStringLiterals(irModule, pool);
        }
        addGlobalHashedStringLiterals(pool, *builder.sharedBuilder);
    }

    // Set up shared and builder insert point

    context->shared = sharedContext;
    context->builder = &sharedContext->builderStorage;

    context->builder->setInsertInto(context->getModule()->getModuleInst());

    // for now, clone all unreferenced witness tables
    //
    // TODO: This step should *not* be needed with the current IR
    // specialization approach, so we should consider removing it.
    //
    for (auto sym : context->getSymbols())
    {
        if (sym.Value->irGlobalValue->op == kIROp_WitnessTable)
            cloneGlobalValue(context, (IRWitnessTable*)sym.Value->irGlobalValue);
    }

    // Next, we make sure to clone the global value for
    // the entry point function itself, and rely on
    // this step to recursively copy over anything else
    // it might reference.
    //
    // Note: We query the mangled name of the entry point from
    // the `program` instead of the `entryPoint` directly,
    // because the `program` will include any specialization
    // arguments which might end up affecting the mangled
    // entry point name.
    //
    auto entryPointMangledName = program->getEntryPointMangledName(entryPointIndex);
    auto irEntryPoint = specializeIRForEntryPoint(context, entryPointMangledName);

    // Bindings for global generic parameters are currently represented
    // as stand-alone global-scope instructions in the IR module for
    // `SpecializedComponentType`s. These instructions are required for
    // correct codegen, and so we must make sure to copy them all over,
    // even though they are not directly referenced.
    //
    // TODO: We should change these to decorations, akin to how
    // `[bindExistentialSlots(...)]` works, so that they can be attached
    // to the relevant parameters and cloned via `cloneExtraDecorations`.
    // In the long run we do not want to *ever* iterate over all the
    // instructions in all the input modules.
    //
    
    for (IRModule* irModule : irModules)
    {
        for (auto inst : irModule->getGlobalInsts())
        {
            auto bindInst = as<IRBindGlobalGenericParam>(inst);
            if (!bindInst)
                continue;

            cloneValue(context, bindInst);
        }
    }

    // TODO: *technically* we should consider the case where
    // we have global variables with initializers, since
    // these should get run whether or not the entry point
    // references them.
    //
    // Or alternatively we can define by fiat that the initializers
    // on global variables get run at an unspecified time between
    // program startup and the first access to a given global.
    // Such a definition gives us the freedom to eliminate globals
    // that are never accessed, while still doing "eager"
    // initialization for globals that are referenced (instead of
    // having to add the overhead of lazy initialization a la
    // function-`static` variables).

    // Now that we've cloned the entry point and everything
    // it refers to, we can package up the data we return
    // to the caller.
    //
    LinkedIR linkedIR;
    linkedIR.module = state->irModule;
    linkedIR.entryPoint = irEntryPoint;
    return linkedIR;
}

struct ReplaceGlobalConstantsPass
{
    void process(IRModule* module)
    {
        _processInstRec(module->getModuleInst());

        for(auto inst : instsToRemove)
            inst->removeAndDeallocate();
    }

    List<IRInst*> instsToRemove;

    void _processInstRec(IRInst* inst)
    {
        if( auto globalConstant = as<IRGlobalConstant>(inst) )
        {
            if( auto val = globalConstant->getValue() )
            {
                // Attempt to transfer the name hint from the global
                // constant to its value. If multiple constants
                // have the same value, then the first one will "win"
                // and transfer its name over.
                //
                if( auto nameHint = globalConstant->findDecoration<IRNameHintDecoration>() )
                {
                    if( !val->findDecoration<IRNameHintDecoration>() )
                    {
                        nameHint->removeFromParent();
                        nameHint->insertAtStart(val);
                    }
                }

                // Replace the global constant
                globalConstant->replaceUsesWith(val);
                instsToRemove.add(globalConstant);
            }
        }

        for( auto child : inst->getDecorationsAndChildren() )
        {
            _processInstRec(child);
        }
    }
};

void replaceGlobalConstants(IRModule* module)
{
    ReplaceGlobalConstantsPass pass;
    pass.process(module);
}



} // namespace Slang
