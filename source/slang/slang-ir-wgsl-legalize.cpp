#include "slang-ir-wgsl-legalize.h"

#include "slang-ir-insts.h"
#include "slang-ir-legalize-binary-operator.h"
#include "slang-ir-legalize-global-values.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// Returns true if the `argIndex`-th parameter of `call`'s callee is passed with
// copy-in/out (`out`/`inout`) semantics. Such an argument may be bridged through a
// `function`-space temporary, whereas a `ref`/borrow parameter aliases the caller's
// object (e.g. `workgroupUniformLoad` on groupshared memory) and must receive the
// real pointer. Returns false when the callee signature is unavailable, so an
// argument of unknown mode is left untouched.
static bool paramHasCopyInOutSemantics(IRCall* call, UInt argIndex)
{
    auto funcType = as<IRFuncType>(call->getCallee()->getFullType());
    if (!funcType || argIndex >= funcType->getParamCount())
        return false;
    auto paramType = funcType->getParamType(argIndex);
    return as<IROutParamType>(paramType) || as<IRBorrowInOutParamType>(paramType);
}

static void legalizeCall(IRCall* call)
{
    // WGSL does not allow forming a pointer to a sub part of a composite value.
    // For example, if we have
    // ```
    // struct S { float x; float y; };
    // void foo(inout float v) { v = 1.0f; }
    // void main() { S s; foo(s.x); }
    // ```
    // The call to `foo(s.x)` is illegal in WGSL because `s.x` is a sub part of `s`.
    // And trying to form `&s.x` in WGSL is illegal.
    // To work around this, we will create a local variable to hold the sub part of
    // the composite value.
    // And then pass the local variable to the function.
    // After the call, we will write back the local variable to the sub part of the
    // composite value.
    //
    IRBuilder builder(call);
    builder.setInsertBefore(call);
    struct WritebackPair
    {
        IRInst* dest;
        IRInst* value;
    };
    ShortList<WritebackPair> pendingWritebacks;

    for (UInt i = 0; i < call->getArgCount(); i++)
    {
        auto arg = call->getArg(i);
        auto ptrType = as<IRPtrTypeBase>(arg->getDataType());
        if (!ptrType)
            continue;

        auto argOp = arg->getOp();

        // A by-ref `Param` and a block-local `Var` are already in the `function` address
        // space, so they can be passed to a `ptr<function, T>` parameter directly.
        if (argOp == kIROp_Param)
            continue;
        if (argOp == kIROp_Var && as<IRBlock>(arg->getParent()))
            continue;

        // A module-scope object (`GlobalVar`/`GlobalParam`, or a `Var` outside a block)
        // lives in a module-scope space such as `private`. Passing it directly to a
        // `ptr<function, T>` (`out`/`inout`) parameter is the address-space mismatch WGSL
        // rejects (#12173), so bridge it through the copy-in/out temp below. A `ref`/borrow
        // parameter instead aliases the caller's real object (e.g. `workgroupUniformLoad`
        // on groupshared memory), so it must keep receiving that object directly.
        if (argOp == kIROp_Var || argOp == kIROp_GlobalVar || argOp == kIROp_GlobalParam)
        {
            if (!paramHasCopyInOutSemantics(call, i))
                continue;
        }
        // Otherwise the argument is a pointer to a sub-part of a composite value (e.g.
        // `s.x`), which WGSL cannot address directly — bridge it unconditionally, as before.

        // Create a local variable to hold the input argument.
        auto var = builder.emitVar(ptrType->getValueType(), AddressSpace::Function);

        // Store the input argument into the local variable.
        builder.emitStore(var, builder.emitLoad(arg));
        builder.replaceOperand(call->getArgs() + i, var);
        pendingWritebacks.add({arg, var});
    }

    // Perform writebacks after the call.
    builder.setInsertAfter(call);
    for (auto& pair : pendingWritebacks)
    {
        builder.emitStore(pair.dest, builder.emitLoad(pair.value));
    }
}

static void legalizeFunc(IRFunc* func)
{
    // Insert casts to convert integer return types
    auto funcReturnType = func->getResultType();
    if (isIntegralType(funcReturnType))
    {
        for (auto block : func->getBlocks())
        {
            if (auto returnInst = as<IRReturn>(block->getTerminator()))
            {
                auto returnedValue = returnInst->getOperand(0);
                auto returnedValueType = returnedValue->getDataType();
                if (isIntegralType(returnedValueType))
                {
                    IRBuilder builder(returnInst);
                    builder.setInsertBefore(returnInst);
                    auto newOp = builder.emitCast(funcReturnType, returnedValue);
                    builder.replaceOperand(returnInst->getOperands(), newOp);
                }
            }
        }
    }
}

static void legalizeSwitch(IRSwitch* switchInst)
{
    // WGSL Requires all switch statements to contain a default case.
    // If the switch statement does not contain a default case, we will add one.
    if (switchInst->getDefaultLabel() != switchInst->getBreakLabel())
        return;
    IRBuilder builder(switchInst);
    auto defaultBlock = builder.createBlock();
    builder.setInsertInto(defaultBlock);
    builder.emitBranch(switchInst->getBreakLabel());
    defaultBlock->insertBefore(switchInst->getBreakLabel());
    List<IRInst*> cases;
    for (UInt i = 0; i < switchInst->getCaseCount(); i++)
    {
        cases.add(switchInst->getCaseValue(i));
        cases.add(switchInst->getCaseLabel(i));
    }
    builder.setInsertBefore(switchInst);
    auto newSwitch = builder.emitSwitch(
        switchInst->getCondition(),
        switchInst->getBreakLabel(),
        defaultBlock,
        (UInt)cases.getCount(),
        cases.getBuffer());
    switchInst->transferDecorationsTo(newSwitch);
    switchInst->removeAndDeallocate();
}

static void processInst(IRInst* inst, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    switch (inst->getOp())
    {
    case kIROp_Call:
        legalizeCall(static_cast<IRCall*>(inst));
        break;

    case kIROp_Switch:
        legalizeSwitch(as<IRSwitch>(inst));
        break;

    // For all binary operators, make sure both side of the operator have the same type
    // (vector-ness and matrix-ness).
    case kIROp_Add:
    case kIROp_Sub:
    case kIROp_Mul:
    case kIROp_Div:
    case kIROp_FRem:
    case kIROp_IRem:
    case kIROp_And:
    case kIROp_Or:
    case kIROp_BitAnd:
    case kIROp_BitOr:
    case kIROp_BitXor:
    case kIROp_Lsh:
    case kIROp_Rsh:
    case kIROp_Eql:
    case kIROp_Neq:
    case kIROp_Greater:
    case kIROp_Less:
    case kIROp_Geq:
    case kIROp_Leq:
        legalizeBinaryOp(inst, sink, targetProgram);
        break;

    case kIROp_Func:
        legalizeFunc(static_cast<IRFunc*>(inst));
        [[fallthrough]];
    default:
        for (auto child : inst->getModifiableChildren())
        {
            processInst(child, targetProgram, sink);
        }
    }
}

struct GlobalInstInliningContext : public GlobalInstInliningContextGeneric
{
    bool isLegalGlobalInstForTarget(IRInst* inst) override
    {
        switch (inst->getOp())
        {
        case kIROp_MakeStruct:
        case kIROp_MakeArray:
        case kIROp_MakeArrayFromElement:
        case kIROp_MakeVector:
        case kIROp_MakeMatrix:
        case kIROp_MakeMatrixFromScalar:
        case kIROp_MakeVectorFromScalar:
            return true;
        default:
            break;
        }
        // The global instructions that are generically considered legal are fine for
        // WGSL.
        return false;
    }

    bool isInlinableGlobalInstForTarget(IRInst* /* inst */) override
    {
        // The global instructions that are generically considered inlineable are fine
        // for WGSL.
        return false;
    }

    bool shouldBeInlinedForTarget(IRInst* /* user */) override
    {
        // WGSL doesn't do any extra inlining beyond what is generically done by default.
        return false;
    }

    IRInst* getOutsideASM(IRInst* beforeInst) override
    {
        // Not needed for WGSL, check e.g. the SPIR-V case to see why this is used.
        return beforeInst;
    }
};

// Rewrites the module-scope variables that hold entry-point inputs so that the inputs are passed
// as function parameters ("threaded" through the call graph) instead of being read from the
// variables.
//
// `translateGlobalVaryingVar` gives every entry-point input that a function other than the entry
// point reads (such as the `subgroup_size` built-in behind `WaveGetLaneCount()`) a module-scope
// variable, which each entry point sets from its parameter. On WGSL that variable is a
// `var<private>`, and WGSL's uniformity analysis treats every read of a `var<private>` as possibly
// non-uniform: a subgroup operation in a loop bounded by `WaveGetLaneCount()` fails validation
// (issue #8774), although the built-in is uniform. Values passed as parameters keep their
// uniformity, so we rewrite the variable: entry points use the stored value directly, and every
// function that reads the variable, directly or through its callees, receives the value as an
// extra parameter.
//
// The rewrite is done here rather than in `translateGlobalVaryingVar` because that pass is shared
// by all targets, and only WGSL's uniformity analysis needs the parameter form. It runs at the end
// of WGSL legalization, after `GlobalInstInliningContext`, and after
// `moveGlobalVarInitializationToEntryPoints` has put the calls of module-scope initializers at the
// start of each entry point, before the stores of the inputs.
//
// A variable whose uses do not have the expected shape is left as it is; the output is then the
// same as without this rewrite.
struct EntryPointGlobalThreadingContext
{
    explicit EntryPointGlobalThreadingContext(IRModule* inModule)
        : module(inModule)
    {
    }

    IRModule* module;

    // State for the variable being rewritten, cleared for each one: the value each entry point
    // stores into the variable,
    Dictionary<IRFunc*, IRInst*> valueByEntryPoint;
    // and the parameter added to each other function that needs the value.
    Dictionary<IRFunc*, IRParam*> paramByFunc;

    static bool isEntryPoint(IRFunc* func)
    {
        return func->findDecoration<IREntryPointDecoration>() != nullptr;
    }

    // Is `store` in the entry block of an entry point?
    static bool isEntryBlockStore(IRStore* store)
    {
        auto block = as<IRBlock>(store->getParent());
        auto func = block ? as<IRFunc>(block->getParent()) : nullptr;
        return func && block == func->getFirstBlock() && isEntryPoint(func);
    }

    // Whether nothing before `store` reads the variable, directly or through a call, so that
    // replacing the reads by the stored value keeps what they read. `store` is in the entry block,
    // which has no predecessors, so the instructions before it in the block are everything that
    // runs before it. A call to a function in `neededSet` can read the variable, and so can a call
    // whose callee is not a known function. `neededSet` must already contain every function that
    // reads the variable, directly or through its callees.
    static bool isInitialStore(
        IRStore* store,
        IRGlobalVar* globalVar,
        const HashSet<IRFunc*>& neededSet)
    {
        auto block = as<IRBlock>(store->getParent());
        for (auto inst = block->getFirstInst(); inst && inst != store; inst = inst->getNextInst())
        {
            if (auto call = as<IRCall>(inst))
            {
                auto callee = as<IRFunc>(call->getCallee());
                if (!callee || neededSet.contains(callee))
                    return false;
            }
            for (UInt i = 0; i < inst->getOperandCount(); i++)
            {
                if (inst->getOperand(i) == globalVar)
                    return false;
            }
        }
        return true;
    }

    // Rewrites `globalVar` if it holds an entry-point input and all its uses have the expected
    // shape, and returns whether it did. Returning false leaves the module unchanged.
    bool tryThread(IRGlobalVar* globalVar)
    {
        // A variable with an initializer is set before the entry point runs.
        if (globalVar->getFirstBlock())
            return false;
        auto ptrType = as<IRPtrTypeBase>(globalVar->getDataType());
        if (!ptrType)
            return false;
        auto valueType = ptrType->getValueType();

        valueByEntryPoint.clear();
        paramByFunc.clear();
        List<IRLoad*> loads;
        List<IRStore*> stores;
        // The entry-point parameter that declares the input, for each entry point that has one.
        Dictionary<IRFunc*, IRParam*> inputParamByEntryPoint;
        IRInst* nameHintSource = nullptr;
        for (auto use = globalVar->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (auto load = as<IRLoad>(user))
            {
                loads.add(load);
                continue;
            }
            // The `GlobalVariableShadowingGlobalParameter` decoration on the entry point's
            // parameter marks a variable that holds an entry-point input; it is removed with the
            // variable.
            if (user->getOp() == kIROp_GlobalVariableShadowingGlobalParameterDecoration)
            {
                // The decoration is on the entry-point parameter that declares the input.
                auto param = as<IRParam>(user->getParent());
                auto block = param ? as<IRBlock>(param->getParent()) : nullptr;
                auto func = block ? as<IRFunc>(block->getParent()) : nullptr;
                if (!func || block != func->getFirstBlock())
                    return false;
                inputParamByEntryPoint[func] = param;
                nameHintSource = param;
                continue;
            }
            auto store = as<IRStore>(user);
            if (!store || store->getPtr() != globalVar || !isEntryBlockStore(store))
                return false;
            auto entryPoint = as<IRFunc>(store->getParent()->getParent());
            // One initial store per entry point.
            if (valueByEntryPoint.containsKey(entryPoint))
                return false;
            valueByEntryPoint[entryPoint] = store->getVal();
            stores.add(store);
        }
        // Only variables that hold an entry-point input (a built-in or varying input that
        // `translateGlobalVaryingVar` stores); other module-scope variables keep their storage.
        if (!nameHintSource || loads.getCount() == 0)
            return false;

        // The functions that need the value: the ones that read the variable, and transitively
        // every caller up to an entry point. Every use of such a function must be a direct call.
        List<IRFunc*> needed;
        HashSet<IRFunc*> neededSet;
        // Accepts `func`, which reads the variable or calls a function that needs the value. An
        // entry point already has the value if it stores it; any other function is added to
        // `needed`. Returns false, so that the variable is left alone, for an entry point that
        // does not store the variable (it reads an input that it does not declare, which happens
        // when several entry points are compiled into one module) and for a read outside a
        // function.
        auto acceptReader = [&](IRFunc* func) -> bool
        {
            if (!func)
                return false;
            if (isEntryPoint(func))
                return valueByEntryPoint.containsKey(func);
            SLANG_ASSERT(func->getFirstBlock());
            if (neededSet.add(func))
                needed.add(func);
            return true;
        };
        for (auto load : loads)
        {
            if (!acceptReader(getParentFunc(load)))
                return false;
        }
        for (Index i = 0; i < needed.getCount(); i++)
        {
            for (auto use = needed[i]->firstUse; use; use = use->nextUse)
            {
                // A function used other than by a direct call (its address taken, for example)
                // cannot get a parameter.
                auto call = as<IRCall>(use->getUser());
                if (!call || call->getCalleeUse() != use)
                    return false;
                if (!acceptReader(getParentFunc(call)))
                    return false;
            }
        }

        // An entry point that declares the input passes its parameter, which is available from the
        // start (a module-scope initializer that reads the input can run before the store).
        // Otherwise the store must come before anything in its entry point that can read the
        // variable.
        for (auto store : stores)
        {
            auto entryPoint = as<IRFunc>(store->getParent()->getParent());
            if (auto param = inputParamByEntryPoint.tryGetValue(entryPoint))
                valueByEntryPoint[entryPoint] = *param;
            else if (!isInitialStore(store, globalVar, neededSet))
                return false;
        }

        // Add the parameter to every function that needs it.
        IRBuilder builder(module);
        for (auto func : needed)
        {
            auto firstBlock = func->getFirstBlock();
            if (auto lastParam = firstBlock->getLastParam())
                builder.setInsertAfter(lastParam);
            else
                builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
            auto param = builder.emitParam(valueType);
            if (auto nameHint = nameHintSource->findDecoration<IRNameHintDecoration>())
                builder.addNameHintDecoration(param, nameHint->getName());
            paramByFunc[func] = param;
            fixUpFuncType(func);
        }

        // Pass it at every call site: the stored value in an entry point, the caller's own
        // parameter elsewhere.
        auto valueIn = [&](IRFunc* func) -> IRInst*
        {
            if (auto value = valueByEntryPoint.tryGetValue(func))
                return *value;
            // Every function that reads the value was added to `needed` and given a parameter.
            auto param = paramByFunc.tryGetValue(func);
            SLANG_ASSERT(param);
            return *param;
        };
        for (auto func : needed)
        {
            List<IRCall*> calls;
            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                if (auto call = as<IRCall>(use->getUser()))
                    calls.add(call);
            }
            for (auto call : calls)
            {
                List<IRInst*> args;
                for (UInt i = 0; i < call->getArgCount(); i++)
                    args.add(call->getArg(i));
                args.add(valueIn(getParentFunc(call)));
                builder.setInsertBefore(call);
                auto newCall = builder.emitCallInst(call->getFullType(), func, args);
                call->transferDecorationsTo(newCall);
                call->replaceUsesWith(newCall);
                call->removeAndDeallocate();
            }
        }

        // Read the value instead of the variable.
        for (auto load : loads)
        {
            load->replaceUsesWith(valueIn(getParentFunc(load)));
            load->removeAndDeallocate();
        }
        // The variable is now only stored to, or named by decorations.
        List<IRInst*> users;
        for (auto use = globalVar->firstUse; use; use = use->nextUse)
            users.add(use->getUser());
        for (auto user : users)
            user->removeAndDeallocate();
        globalVar->removeAndDeallocate();
        return true;
    }

    void processModule()
    {
        List<IRGlobalVar*> globalVars;
        for (auto inst : module->getGlobalInsts())
        {
            if (auto globalVar = as<IRGlobalVar>(inst))
                globalVars.add(globalVar);
        }
        for (auto globalVar : globalVars)
            tryThread(globalVar);
    }
};

void legalizeIRForWGSL(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    List<EntryPointInfo> entryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        IRFunc* const func{as<IRFunc>(inst)};
        if (!func)
            continue;
        IREntryPointDecoration* const entryPointDecor =
            func->findDecoration<IREntryPointDecoration>();
        if (!entryPointDecor)
            continue;
        EntryPointInfo info;
        info.entryPointDecor = entryPointDecor;
        info.entryPointFunc = func;
        entryPoints.add(info);
    }

    legalizeEntryPointVaryingParamsForWGSL(module, sink, entryPoints);

    // Go through every instruction in the module and legalize them as needed.
    processInst(module->getModuleInst(), targetProgram, sink);

    // Some global insts are illegal, e.g. function calls.
    // We need to inline and remove those.
    GlobalInstInliningContext().inlineGlobalValuesAndRemoveIfUnused(module);

    // Pass entry-point inputs that were stored in module-scope variables as values, so that
    // WGSL's uniformity analysis can see that uniform built-ins are uniform.
    EntryPointGlobalThreadingContext(module).processModule();
}

struct WGSLAddressSpaceAssigner : InitialAddressSpaceAssigner
{
    virtual bool tryAssignAddressSpace(IRInst* inst, AddressSpace& outAddressSpace) override
    {
        switch (inst->getOp())
        {
        case kIROp_Var:
            if (as<IRBlock>(inst->getParent()))
                outAddressSpace = AddressSpace::Function;
            else
                outAddressSpace = AddressSpace::ThreadLocal;
            return true;
        case kIROp_RWStructuredBufferGetElementPtr:
            outAddressSpace = AddressSpace::Global;
            return true;
        case kIROp_Load:
            {
                auto addrSpace = getAddressSpaceFromVarType(inst->getDataType());
                if (addrSpace != AddressSpace::Generic)
                {
                    outAddressSpace = addrSpace;
                    return true;
                }
            }
            return false;
        default:
            return false;
        }
    }

    virtual AddressSpace getAddressSpaceFromVarType(IRInst* type) override
    {
        if (as<IRUniformParameterGroupType>(type))
        {
            return AddressSpace::Uniform;
        }
        if (as<IRByteAddressBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRHLSLStructuredBufferTypeBase>(type))
        {
            return AddressSpace::Global;
        }
        if (as<IRGLSLShaderStorageBufferType>(type))
        {
            return AddressSpace::Global;
        }
        if (auto ptrType = as<IRPtrTypeBase>(type))
        {
            if (ptrType->hasAddressSpace())
                return ptrType->getAddressSpace();
            return AddressSpace::Generic;
        }
        return AddressSpace::Generic;
    }

    virtual AddressSpace getLeafInstAddressSpace(IRInst* inst) override
    {
        if (as<IRGroupSharedRate>(inst->getRate()))
            return AddressSpace::GroupShared;
        switch (inst->getOp())
        {
        case kIROp_RWStructuredBufferGetElementPtr:
            return AddressSpace::Global;
        case kIROp_Var:
            if (as<IRBlock>(inst->getParent()))
                return AddressSpace::Function;
            else
                return AddressSpace::ThreadLocal;
            break;
        default:
            break;
        }
        auto type = unwrapAttributedType(inst->getDataType());
        if (!type)
            return AddressSpace::Generic;
        return getAddressSpaceFromVarType(type);
    }
};

void specializeAddressSpaceForWGSL(IRModule* module)
{
    WGSLAddressSpaceAssigner wgslAddressSpaceAssigner;
    specializeAddressSpace(module, &wgslAddressSpaceAssigner);
}

} // namespace Slang
