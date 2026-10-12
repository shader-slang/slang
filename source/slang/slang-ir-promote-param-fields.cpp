// slang-ir-promote-param-fields.cpp
#include "slang-ir-promote-param-fields.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

// This pass looks for functions that take a large aggregate value (a `struct`
// or a fixed-size array) as an `in` parameter, but only read a few of its
// fields. For example:
//
//      struct State { uint a; uint unused[1024]; uint b; }
//      uint readFields(State s) { return s.a + s.b; }
//
// is rewritten into:
//
//      uint readFields(uint s_a, uint s_b) { return s_a + s_b; }
//
// and every call site `readFields(x)` becomes `readFields(x.a, x.b)`.
//
// The motivation is thread-local state. An aggregate `in` argument is a copy of
// the caller's value, and on targets that pass aggregates by pointer (see
// `transformParamsToConstRef`) that copy is a call-site temporary in
// thread-local memory. If the downstream compiler does not inline the callee,
// the temporary survives, so a function that reads two fields of a 4 KB struct
// still costs 4 KB of thread-local memory per call site. Passing only the fields
// the callee reads removes the temporary.
//
// A parameter is only promoted when every use of it in the callee is a chain of
// field extractions and constant-index element extractions. That makes the set
// of values the callee can read known at compile time. Because `in` parameters
// are copied at the call, reading those values at the call site instead of in
// the callee does not change behavior, even if the callee only reads some of
// them conditionally.

struct PromoteParamFieldsContext
{
    IRModule* module;
    TargetRequest* targetReq;
    IRBuilder builder;

    // Aggregates smaller than this are cheap enough to copy that splitting them
    // is not worth the larger parameter list.
    static const IRIntegerValue kMinParamSize = 64;

    // Only promote a parameter when the fields that are read add up to at most
    // half of the aggregate, so the promotion clearly reduces the amount of
    // thread-local state.
    static const IRIntegerValue kMaxPromotedSizeDivisor = 2;

    // Do not split one parameter into more than this many parameters.
    static const Index kMaxPromotedParamsPerParam = 16;

    PromoteParamFieldsContext(IRModule* module, TargetRequest* targetReq)
        : module(module), targetReq(targetReq), builder(module)
    {
    }

    // One step of an access path from a parameter to a value derived from it:
    // either a struct field or a constant array index.
    struct AccessStep
    {
        // The struct key, for a field access. Null for an array element access.
        IRInst* fieldKey = nullptr;

        // The array index, for an array element access.
        IRIntegerValue index = 0;
    };

    // The uses of a parameter form a tree: the root is the parameter itself, and
    // each child is one field or constant-index element read from its parent.
    struct PathNode
    {
        // How this node is reached from its parent. Unused at the root.
        AccessStep step;
        Index parent = -1;
        IRType* type = nullptr;

        // True if something other than a field or element extraction uses the
        // value at this node, so the callee needs that value in full.
        bool usedWhole = false;

        // The instructions in the callee that compute the value at this node.
        List<IRInst*> insts;

        List<Index> children;

        // The new parameter that replaces this node, if it is promoted.
        IRParam* newParam = nullptr;
    };

    // What to do with one parameter of the function being rewritten.
    struct ParamPlan
    {
        IRParam* param = nullptr;
        List<PathNode> nodes;

        // The nodes that become new parameters, in parameter order.
        List<Index> promoted;
    };

    static bool isSameStep(const AccessStep& a, const AccessStep& b)
    {
        if (a.fieldKey || b.fieldKey)
            return a.fieldKey == b.fieldKey;
        return a.index == b.index;
    }

    // A type is plain data if it is made only of scalars, vectors, matrices,
    // structs and fixed-size arrays. Values of such types can be passed as
    // ordinary parameters on every target; resources, pointers and other handle
    // types may have target-specific restrictions, so we leave them alone.
    static bool isPlainDataType(IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_VectorType:
        case kIROp_MatrixType:
            return true;
        case kIROp_StructType:
            for (auto field : as<IRStructType>(type)->getFields())
            {
                if (!isPlainDataType(field->getFieldType()))
                    return false;
            }
            return true;
        case kIROp_ArrayType:
            return isPlainDataType(as<IRArrayType>(type)->getElementType());
        default:
            return as<IRBasicType>(type) != nullptr;
        }
    }

    static bool isAggregateType(IRType* type)
    {
        return as<IRStructType>(type) || as<IRArrayType>(type);
    }

    // Returns the size of `type` in bytes, or -1 if it has no natural layout.
    IRIntegerValue getSize(IRType* type)
    {
        IRSizeAndAlignment sizeAndAlignment;
        if (SLANG_FAILED(getNaturalSizeAndAlignment(targetReq, type, &sizeAndAlignment)))
            return -1;
        return sizeAndAlignment.size;
    }

    // If `user` reads one field, or one element at a constant index, out of
    // `value`, return true and describe that read in `outStep`.
    static bool getConstantAccessStep(IRInst* user, IRInst* value, AccessStep& outStep)
    {
        auto valueType = value->getDataType();

        if (auto fieldExtract = as<IRFieldExtract>(user))
        {
            if (fieldExtract->getBase() != value)
                return false;
            if (!as<IRStructType>(valueType))
                return false;
            outStep.fieldKey = fieldExtract->getField();
            return true;
        }

        if (auto getElement = as<IRGetElement>(user))
        {
            if (getElement->getBase() != value)
                return false;
            if (!as<IRArrayType>(valueType))
                return false;

            // A dynamic index could read any element, so we cannot tell which
            // part of the array the callee needs.
            auto indexLit = as<IRIntLit>(getElement->getIndex());
            if (!indexLit)
                return false;

            outStep.index = indexLit->getValue();
            return true;
        }

        return false;
    }

    Index findOrAddChild(List<PathNode>& nodes, Index parent, AccessStep step, IRType* type)
    {
        for (auto child : nodes[parent].children)
        {
            if (isSameStep(nodes[child].step, step))
                return child;
        }

        PathNode node;
        node.step = step;
        node.parent = parent;
        node.type = type;
        Index index = nodes.getCount();
        nodes.add(node);
        nodes[parent].children.add(index);
        return index;
    }

    // Add `value`, whose position in the tree is `nodeIndex`, and everything
    // read from it, to the tree.
    void collectUses(List<PathNode>& nodes, Index nodeIndex, IRInst* value)
    {
        nodes[nodeIndex].insts.add(value);

        for (auto use = value->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            AccessStep step;
            if (getConstantAccessStep(user, value, step))
            {
                auto child = findOrAddChild(nodes, nodeIndex, step, user->getDataType());
                collectUses(nodes, child, user);
                continue;
            }

            nodes[nodeIndex].usedWhole = true;
        }
    }

    // The position of a child among its siblings in declaration order: struct
    // field order, or array index order.
    IRIntegerValue getDeclarationOrder(List<PathNode>& nodes, Index parent, Index child)
    {
        auto& step = nodes[child].step;
        if (!step.fieldKey)
            return step.index;

        auto structType = as<IRStructType>(nodes[parent].type);
        IRIntegerValue order = 0;
        for (auto field : structType->getFields())
        {
            if (field->getKey() == step.fieldKey)
                break;
            order++;
        }
        return order;
    }

    // Collect the nodes that become new parameters. A node is promoted if the
    // callee needs its whole value and it is not inside another promoted node.
    // Children are visited in declaration order so the new parameter list is
    // deterministic.
    void collectPromotedNodes(List<PathNode>& nodes, Index nodeIndex, List<Index>& outPromoted)
    {
        if (nodes[nodeIndex].usedWhole)
        {
            outPromoted.add(nodeIndex);
            return;
        }

        List<Index> children = nodes[nodeIndex].children;
        children.sort(
            [&](Index a, Index b) {
                return getDeclarationOrder(nodes, nodeIndex, a) <
                       getDeclarationOrder(nodes, nodeIndex, b);
            });
        for (auto child : children)
            collectPromotedNodes(nodes, child, outPromoted);
    }

    List<AccessStep> getPath(List<PathNode>& nodes, Index nodeIndex)
    {
        List<AccessStep> path;
        for (Index i = nodeIndex; nodes[i].parent != -1; i = nodes[i].parent)
            path.add(nodes[i].step);
        path.reverse();
        return path;
    }

    // Name a promoted parameter after the original parameter and the fields
    // read from it, e.g. `state_normal` for `state.normal`, so generated code
    // stays readable.
    String getPathNameHint(IRParam* param, const List<AccessStep>& path)
    {
        StringBuilder sb;
        if (auto nameHint = param->findDecoration<IRNameHintDecoration>())
            sb << nameHint->getName();
        else
            sb << "param";

        for (auto step : path)
        {
            sb << "_";
            if (!step.fieldKey)
            {
                sb << step.index;
                continue;
            }
            if (auto keyNameHint = step.fieldKey->findDecoration<IRNameHintDecoration>())
                sb << keyNameHint->getName();
            else
                sb << "field";
        }
        return sb.produceString();
    }

    // Parameters that carry decorations other than a name hint (layout, semantics,
    // and so on) have meaning beyond their value, so we keep them as they are.
    static bool hasOnlyNameHintDecorations(IRParam* param)
    {
        for (auto decoration : param->getDecorations())
        {
            if (!as<IRNameHintDecoration>(decoration))
                return false;
        }
        return true;
    }

    // Returns true if passing the promoted fields instead of the whole aggregate
    // is worth it: the aggregate is large, the promoted fields are a small part
    // of it, and the parameter list stays reasonably short.
    bool isPromotionProfitable(ParamPlan& plan, IRIntegerValue paramSize)
    {
        if (plan.promoted.getCount() > kMaxPromotedParamsPerParam)
            return false;

        IRIntegerValue promotedSize = 0;
        for (auto nodeIndex : plan.promoted)
        {
            auto size = getSize(plan.nodes[nodeIndex].type);
            if (size < 0)
                return false;
            promotedSize += size;
        }
        return promotedSize * kMaxPromotedSizeDivisor <= paramSize;
    }

    // Decide whether `param` should be promoted, and if so fill in `outPlan`.
    bool planParam(IRParam* param, ParamPlan& outPlan)
    {
        auto paramType = param->getDataType();
        if (!paramType || !isAggregateType(paramType) || !isPlainDataType(paramType))
            return false;
        if (!hasOnlyNameHintDecorations(param))
            return false;

        auto paramSize = getSize(paramType);
        if (paramSize < kMinParamSize)
            return false;

        outPlan.param = param;
        outPlan.nodes.clear();
        outPlan.promoted.clear();

        PathNode root;
        root.type = paramType;
        outPlan.nodes.add(root);
        collectUses(outPlan.nodes, 0, param);

        // If the callee uses the parameter as a whole anywhere (for example, passes
        // it to another function or returns it), it needs the entire aggregate.
        if (outPlan.nodes[0].usedWhole)
            return false;

        collectPromotedNodes(outPlan.nodes, 0, outPlan.promoted);
        return isPromotionProfitable(outPlan, paramSize);
    }

    // Emit the value at `path` within the aggregate value `arg`.
    IRInst* emitPathValue(IRInst* arg, const List<AccessStep>& path)
    {
        IRInst* value = arg;
        for (auto step : path)
        {
            if (step.fieldKey)
                value = builder.emitFieldExtract(value, step.fieldKey);
            else
                value = builder.emitElementExtract(value, step.index);
        }
        return value;
    }

    // Emit a load of the value at `path` within the aggregate stored at `ptr`.
    IRInst* emitPathLoad(IRInst* ptr, const List<AccessStep>& path)
    {
        IRInst* addr = ptr;
        for (auto step : path)
        {
            if (step.fieldKey)
                addr = builder.emitFieldAddress(addr, step.fieldKey);
            else
                addr = builder.emitElementAddress(addr, step.index);
        }
        return builder.emitLoad(addr);
    }

    static ParamPlan* findPlan(List<ParamPlan>& plans, IRParam* param)
    {
        for (auto& plan : plans)
        {
            if (plan.param == param)
                return &plan;
        }
        return nullptr;
    }

    // Append the promoted values of `arg` for `plan` to `outArgs`.
    //
    // When the argument is loaded from memory, we load just the promoted fields
    // from that memory instead of extracting them from a loaded copy of the whole
    // aggregate. The new loads are placed right after the original load, so they
    // observe the same memory contents.
    void appendPromotedArgs(IRCall* call, IRInst* arg, ParamPlan& plan, List<IRInst*>& outArgs)
    {
        auto argLoad = as<IRLoad>(arg);
        if (argLoad)
            builder.setInsertBefore(argLoad->getNextInst());
        else
            builder.setInsertBefore(call);

        for (auto nodeIndex : plan.promoted)
        {
            auto path = getPath(plan.nodes, nodeIndex);
            IRInst* value = nullptr;
            if (argLoad)
                value = emitPathLoad(argLoad->getPtr(), path);
            else
                value = emitPathValue(arg, path);
            outArgs.add(value);
        }
    }

    void rewriteCallSites(IRFunc* func, List<ParamPlan>& plans, List<IRParam*>& oldParams)
    {
        List<IRCall*> calls;
        traverseUsers<IRCall>(func, [&](IRCall* call) { calls.add(call); });

        for (auto call : calls)
        {
            List<IRInst*> newArgs;
            for (Index i = 0; i < oldParams.getCount(); i++)
            {
                auto arg = call->getArg(i);
                auto plan = findPlan(plans, oldParams[i]);
                if (plan)
                    appendPromotedArgs(call, arg, *plan, newArgs);
                else
                    newArgs.add(arg);
            }

            // Remember the original arguments, so we can delete any that are no
            // longer used once the call is replaced.
            // The same value can be passed in more than one argument, so keep each
            // only once to avoid deleting it twice.
            List<IRInst*> oldArgs;
            for (UInt i = 0; i < call->getArgCount(); i++)
            {
                if (!oldArgs.contains(call->getArg(i)))
                    oldArgs.add(call->getArg(i));
            }

            builder.setInsertBefore(call);
            auto newCall = builder.emitCallInst(call->getFullType(), func, newArgs);
            call->transferDecorationsTo(newCall);
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();

            for (auto oldArg : oldArgs)
            {
                if (as<IRLoad>(oldArg) && !oldArg->hasUses())
                    oldArg->removeAndDeallocate();
            }
        }
    }

    void rewriteCallee(ParamPlan& plan)
    {
        auto param = plan.param;

        for (auto nodeIndex : plan.promoted)
        {
            auto& node = plan.nodes[nodeIndex];
            auto newParam = builder.createParam(node.type);
            auto nameHint = getPathNameHint(param, getPath(plan.nodes, nodeIndex));
            builder.addNameHintDecoration(newParam, nameHint.getUnownedSlice());
            newParam->insertBefore(param);
            node.newParam = newParam;
        }

        // Code that read a promoted value now reads the new parameter. Further
        // extractions from that value (below a promoted node) keep working, since
        // they now extract from the new parameter.
        for (auto nodeIndex : plan.promoted)
        {
            auto& node = plan.nodes[nodeIndex];
            for (auto inst : node.insts)
                inst->replaceUsesWith(node.newParam);
        }

        // The old parameter and the extractions that led from it to the promoted
        // values are now unused. Nodes were collected parent-first, so walking the
        // list backwards removes each instruction after the ones that used it.
        List<IRInst*> treeInsts;
        for (auto& node : plan.nodes)
            treeInsts.addRange(node.insts);

        for (Index i = treeInsts.getCount() - 1; i >= 0; i--)
        {
            auto inst = treeInsts[i];
            if (inst != param && !inst->hasUses())
                inst->removeAndDeallocate();
        }

        SLANG_ASSERT(!param->hasUses());
        param->removeAndDeallocate();
    }

    // Entry points, target intrinsics and functions visible outside the module
    // have signatures that something else depends on, so they must not change.
    static bool hasFixedSignature(IRFunc* func)
    {
        for (auto decoration : func->getDecorations())
        {
            switch (decoration->getOp())
            {
            case kIROp_TargetIntrinsicDecoration:
            case kIROp_EntryPointDecoration:
            case kIROp_CudaKernelDecoration:
            case kIROp_AutoPyBindCudaDecoration:
            case kIROp_CudaDeviceExportDecoration:
            case kIROp_DllExportDecoration:
            case kIROp_DllImportDecoration:
            case kIROp_ExternCppDecoration:
                return true;
            default:
                break;
            }
        }
        return false;
    }

    // Returns true if every use of `func` is a direct call, so that we can find
    // and rewrite all of its call sites. A function used any other way (stored,
    // passed as a callback, put in a witness table) keeps its signature.
    static bool isOnlyCalledDirectly(IRFunc* func)
    {
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (as<IRDecoration>(user))
                continue;

            // Specialization dictionary entries only record the function for
            // later bookkeeping; `transformParamsToConstRef` treats them the same way.
            if (as<IRCompilerDictionaryValue>(user) || as<IRCompilerDictionaryEntry>(user))
                continue;

            auto call = as<IRCall>(user);
            if (!call || call->getCalleeUse() != use)
                return false;
        }
        return true;
    }

    // Inline assembly can refer to parameters in ways we cannot see.
    static bool containsInlineAssembly(IRFunc* func)
    {
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (as<IRGenericAsm>(inst))
                    return true;
            }
        }
        return false;
    }

    static bool canRewriteSignature(IRFunc* func)
    {
        if (!func->isDefinition())
            return false;
        if (hasFixedSignature(func))
            return false;
        if (!isOnlyCalledDirectly(func))
            return false;
        if (containsInlineAssembly(func))
            return false;
        return true;
    }

    void processFunc(IRFunc* func)
    {
        if (!canRewriteSignature(func))
            return;

        List<IRParam*> oldParams;
        for (auto param = func->getFirstParam(); param; param = param->getNextParam())
            oldParams.add(param);

        List<ParamPlan> plans;
        for (auto param : oldParams)
        {
            ParamPlan plan;
            if (planParam(param, plan))
                plans.add(_Move(plan));
        }
        if (plans.getCount() == 0)
            return;

        rewriteCallSites(func, plans, oldParams);
        for (auto& plan : plans)
            rewriteCallee(plan);
        fixUpFuncType(func);
    }

    // Visit callees before their callers. When a function only forwards a
    // parameter to a callee whose parameter was promoted, the forwarding call now
    // reads individual fields, so the caller's parameter can be promoted too.
    void addFuncsInCalleeFirstOrder(IRFunc* func, List<IRFunc*>& order, HashSet<IRFunc*>& visited)
    {
        if (visited.contains(func))
            return;
        visited.add(func);

        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto call = as<IRCall>(inst);
                if (!call)
                    continue;
                if (auto callee = as<IRFunc>(call->getCallee()))
                    addFuncsInCalleeFirstOrder(callee, order, visited);
            }
        }
        order.add(func);
    }

    void processModule()
    {
        List<IRFunc*> order;
        HashSet<IRFunc*> visited;
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
                addFuncsInCalleeFirstOrder(func, order, visited);
        }
        for (auto func : order)
            processFunc(func);
    }
};

void promoteParamFields(IRModule* module, TargetRequest* targetReq)
{
    PromoteParamFieldsContext context(module, targetReq);
    context.processModule();
}

} // namespace Slang
