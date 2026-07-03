// slang-ir-legalize-cuda-param-dynamic-index.cpp
#include "slang-ir-legalize-cuda-param-dynamic-index.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

namespace
{

/// One step of a value-projection chain rooted at a kernel parameter: either a
/// struct-field extraction (`key` is the field key) or an array-element extraction
/// (`key` is the index operand). Recorded while walking from a dynamically-indexed
/// `GetElement` down to the parameter, then replayed in root-to-leaf order to build
/// the equivalent address chain off the parameter's local copy.
struct ProjectionStep
{
    enum class Kind
    {
        Field,
        Element,
    };
    Kind kind;
    IRInst* key;
};

/// A dynamically-indexed access to rewrite: the `GetElement` instruction with the
/// runtime index, the kernel parameter its base chain roots at, and the projection
/// steps from the parameter to that instruction (in root-to-leaf order).
struct DynamicParamAccess
{
    IRGetElement* getElement;
    IRParam* rootParam;
    List<ProjectionStep> steps;
};

struct LegalizeCUDAKernelParamDynamicIndexContext
{
    IRModule* module;

    // One local copy per parameter, shared by all rewritten accesses in the kernel.
    Dictionary<IRParam*, IRVar*> mapParamToLocalVar;

    LegalizeCUDAKernelParamDynamicIndexContext(IRModule* module)
        : module(module)
    {
    }

    /// Returns true if `func` has the CUDA kernel ABI, i.e. its parameters are passed
    /// in `.param` kernel-argument space. Only such parameters exhibit the serial
    /// dynamic-address load pathology; ordinary `__device__` function parameters live
    /// in registers/stack and are excluded (and are already pointer-ized by
    /// `transformParamsToConstRef` anyway).
    bool isKernelABIFunc(IRFunc* func)
    {
        if (!func->isDefinition())
            return false;
        for (auto decoration : func->getDecorations())
        {
            if (as<IREntryPointDecoration>(decoration) || as<IRCudaKernelDecoration>(decoration) ||
                as<IRAutoPyBindCudaDecoration>(decoration))
                return true;
        }
        return false;
    }

    /// If `getElement` is a runtime-indexed array access whose base is a pure
    /// value-projection chain (`FieldExtract`/`GetElement`) rooted at a by-value
    /// parameter of `func`, fill in `outAccess` and return true.
    bool tryMatchDynamicParamAccess(
        IRFunc* func,
        IRGetElement* getElement,
        DynamicParamAccess& outAccess)
    {
        // Only a runtime index is pathological; constant indices resolve to static
        // `.param` offsets, which are the fast path we must not disturb.
        if (as<IRIntLit>(getElement->getIndex()))
            return false;

        // Restrict to fixed-size array bases: that is the shape whose dynamic
        // addressing ptxas serializes (vectors go through the emitter's element
        // intrinsic already, and unsized arrays cannot be by-value parameters).
        if (!as<IRArrayType>(getElement->getBase()->getDataType()))
            return false;

        List<ProjectionStep> reversedSteps;
        IRInst* inst = getElement;
        for (;;)
        {
            if (auto fieldExtract = as<IRFieldExtract>(inst))
            {
                reversedSteps.add({ProjectionStep::Kind::Field, fieldExtract->getField()});
                inst = fieldExtract->getBase();
            }
            else if (auto elementExtract = as<IRGetElement>(inst))
            {
                reversedSteps.add({ProjectionStep::Kind::Element, elementExtract->getIndex()});
                inst = elementExtract->getBase();
            }
            else
                break;
        }

        auto param = as<IRParam>(inst);
        if (!param)
            return false;
        if (param->getParent() != func->getFirstBlock())
            return false;
        // A pointer-typed parameter (e.g. one already rewritten to `borrow in`) is
        // not in `.param` value space; its loads are dynamically addressable as is.
        if (as<IRPtrTypeBase>(param->getDataType()))
            return false;

        outAccess.getElement = getElement;
        outAccess.rootParam = param;
        outAccess.steps.clear();
        for (Index i = reversedSteps.getCount() - 1; i >= 0; --i)
            outAccess.steps.add(reversedSteps[i]);
        return true;
    }

    /// Return the function-local copy of `param`, creating it (a prologue
    /// var + store) on first use.
    IRVar* getOrCreateLocalCopy(IRFunc* func, IRParam* param)
    {
        IRVar* var = nullptr;
        if (mapParamToLocalVar.tryGetValue(param, var))
            return var;

        IRBuilder builder(module);
        builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
        var = builder.emitVar(param->getDataType(), AddressSpace::Function);
        builder.emitStore(var, param);
        mapParamToLocalVar.set(param, var);
        return var;
    }

    void rewriteAccess(IRFunc* func, const DynamicParamAccess& access)
    {
        auto var = getOrCreateLocalCopy(func, access.rootParam);

        IRBuilder builder(module);
        builder.setInsertBefore(access.getElement);
        IRInst* addr = var;
        for (auto& step : access.steps)
        {
            switch (step.kind)
            {
            case ProjectionStep::Kind::Field:
                addr = builder.emitFieldAddress(addr, step.key);
                break;
            case ProjectionStep::Kind::Element:
                addr = builder.emitElementAddress(addr, step.key);
                break;
            }
        }
        auto load = builder.emitLoad(addr);
        access.getElement->replaceUsesWith(load);
        access.getElement->removeAndDeallocate();
    }

    void processFunc(IRFunc* func)
    {
        mapParamToLocalVar.clear();

        // Collect first, then rewrite: the projection chains are recorded as
        // (kind, key) pairs while the IR is still unmodified, so rewriting one
        // access can never invalidate the recorded chain of another (nested
        // dynamic indices each get their own complete chain off the same copy).
        List<DynamicParamAccess> accesses;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto getElement = as<IRGetElement>(inst);
                if (!getElement)
                    continue;
                DynamicParamAccess access;
                if (tryMatchDynamicParamAccess(func, getElement, access))
                    accesses.add(_Move(access));
            }
        }

        for (auto& access : accesses)
            rewriteAccess(func, access);
    }

    void processModule()
    {
        for (auto inst : module->getGlobalInsts())
        {
            auto func = as<IRFunc>(inst);
            if (!func)
                continue;
            if (!isKernelABIFunc(func))
                continue;
            processFunc(func);
        }
    }
};

} // namespace

void legalizeCUDAKernelParamDynamicIndex(IRModule* module)
{
    LegalizeCUDAKernelParamDynamicIndexContext context(module);
    context.processModule();
}

} // namespace Slang
