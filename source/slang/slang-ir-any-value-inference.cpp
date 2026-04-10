#include "slang-ir-any-value-inference.h"

#include "../core/slang-func-ptr.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

void _findDependenciesOfTypeInSet(
    IRType* type,
    HashSet<IRInterfaceType*>& targetSet,
    List<IRInterfaceType*>& result)
{
    switch (type->getOp())
    {
    case kIROp_InterfaceType:
        {
            auto interfaceType = cast<IRInterfaceType>(type);
            if (targetSet.contains(interfaceType))
            {
                result.add(interfaceType);
                return;
            }
        }
        break;
    case kIROp_StructType:
        {
            auto structType = cast<IRStructType>(type);
            for (auto field : structType->getFields())
            {
                _findDependenciesOfTypeInSet(field->getFieldType(), targetSet, result);
            }
        }
        break;
    case kIROp_PtrType:
    case kIROp_IntPtrType:
    case kIROp_UIntPtrType:
    case kIROp_RawPointerType:
    case kIROp_NativePtrType:
    case kIROp_ComPtrType:
    case kIROp_PseudoPtrType:
    case kIROp_RTTIPointerType:
    case kIROp_OutParamType:
    case kIROp_BorrowInOutParamType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_NativeStringType:
    case kIROp_FuncType:
        // Pointer-like and fixed-size types don't embed their pointee/operand,
        // so they break dependency cycles. Do not recurse into operand types.
        break;
    default:
        {
            for (UInt i = 0; i < type->getOperandCount(); i++)
            {
                if (auto operandType = as<IRType>(type->getOperand(i)))
                    _findDependenciesOfTypeInSet(operandType, targetSet, result);
            }
        }
        break;
    }
}

List<IRInterfaceType*> findDependenciesOfTypeInSet(
    IRType* type,
    HashSet<IRInterfaceType*> targetSet)
{
    List<IRInterfaceType*> result;
    _findDependenciesOfTypeInSet(type, targetSet, result);

    return result;
}

void _sortTopologically(
    IRInterfaceType* interfaceType,
    HashSet<IRInterfaceType*>& visited,
    List<IRInterfaceType*>& sortedInterfaceTypes,
    const Func<HashSet<IRInterfaceType*>, IRInterfaceType*>& getDependencies)
{
    if (visited.contains(interfaceType))
        return;

    visited.add(interfaceType);

    for (auto dependency : getDependencies(interfaceType))
    {
        _sortTopologically(dependency, visited, sortedInterfaceTypes, getDependencies);
    }

    sortedInterfaceTypes.add(interfaceType);
}

List<IRInterfaceType*> sortTopologically(
    HashSet<IRInterfaceType*> interfaceTypes,
    const Func<HashSet<IRInterfaceType*>, IRInterfaceType*>& getDependencies)
{
    List<IRInterfaceType*> sortedInterfaceTypes;
    HashSet<IRInterfaceType*> visited;
    for (auto interfaceType : interfaceTypes)
    {
        _sortTopologically(interfaceType, visited, sortedInterfaceTypes, getDependencies);
    }
    return sortedInterfaceTypes;
}

// Shared dependency analysis used by both diagnoseCircularConformances and
// inferAnyValueSizeWhereNecessary. Collects interface types, their
// implementations, builds the interface dependency graph, and computes SCCs.
//
// dependencyMap excludes direct self-edges so self-referential implementations
// do not break topological ordering for size inference. implDeps retains the
// original per-implementation dependency lists so direct self-cycles can still
// be diagnosed.
struct InterfaceDependencyAnalysis
{
    HashSet<IRInterfaceType*> interfaceTypes;
    Dictionary<IRInterfaceType*, List<IRInst*>> implMap;
    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>> dependencyMap;
    Dictionary<IRInterfaceType*, List<IRInst*>> selfReferentialImpls;
    Dictionary<IRInterfaceType*, List<IRInst*>> nonSelfReferentialImpls;

    // Per-implementation dependency lists (used by cycle detection).
    Dictionary<IRInst*, List<IRInterfaceType*>> implDeps;
    Dictionary<IRInterfaceType*, Index> interfaceToComponent;
    HashSet<IRInterfaceType*> interfacesInDependencyCycle;

    void build(IRModule* module)
    {
        collectInterfaceTypes(module);
        if (interfaceTypes.getCount() == 0)
            return;
        collectImplementations(module);
        buildDependencyGraph();
        computeStronglyConnectedComponents();
    }

    bool implCreatesCircularConformance(IRInterfaceType* interfaceType, IRInst* impl) const
    {
        auto deps = implDeps.tryGetValue(impl);
        if (!deps)
            return false;

        auto interfaceComponent = interfaceToComponent.getValue(interfaceType);
        for (auto dep : *deps)
        {
            if (dep == interfaceType)
                return true;

            if (interfacesInDependencyCycle.contains(interfaceType) &&
                interfaceToComponent.getValue(dep) == interfaceComponent)
            {
                return true;
            }
        }

        return false;
    }

private:
    void collectInterfaceTypes(IRModule* module)
    {
        // First pass: find which interfaces have at least one witness table.
        HashSet<IRInst*> implementedInterfaces;
        for (auto inst : module->getGlobalInsts())
        {
            if (inst->getOp() == kIROp_WitnessTable)
            {
                auto iface = cast<IRWitnessTableType>(inst->getDataType())->getConformanceType();
                implementedInterfaces.add(iface);
            }
        }

        // Second pass: collect qualifying interface types.
        for (auto inst : module->getGlobalInsts())
        {
            auto interfaceType = as<IRInterfaceType>(inst);
            if (!interfaceType)
                continue;
            if (isComInterfaceType((IRType*)interfaceType))
                continue;
            if (interfaceType->findDecoration<IRBuiltinDecoration>())
                continue;
            if (!implementedInterfaces.contains(interfaceType))
                continue;
            interfaceTypes.add(interfaceType);
        }
    }

    void collectImplementations(IRModule* module)
    {
        for (auto interfaceType : interfaceTypes)
        {
            List<IRInst*> impls;

            for (auto use = interfaceType->firstUse; use; use = use->nextUse)
            {
                auto wtt = as<IRWitnessTableType>(use->getUser());
                if (!wtt || wtt->getConformanceType() != interfaceType || !wtt->hasUses())
                    continue;

                for (auto wtUse = wtt->firstUse; wtUse; wtUse = wtUse->nextUse)
                {
                    auto wt = as<IRWitnessTable>(wtUse->getUser());
                    if (!wt || wt->getDataType() != wtt)
                        continue;
                    auto impl = wt->getConcreteType();
                    if (impl->getParent() != module->getModuleInst())
                        continue;

                    impls.add(impl);
                }
            }

            implMap.add(interfaceType, impls);
        }
    }

    void buildDependencyGraph()
    {
        for (auto interfaceType : interfaceTypes)
        {
            HashSet<IRInterfaceType*> deps;
            List<IRInst*> selfRefList;
            List<IRInst*> nonSelfRefList;

            for (auto impl : implMap[interfaceType])
            {
                auto depsForImpl = findDependenciesOfTypeInSet((IRType*)impl, interfaceTypes);
                bool hasSelfReference = false;
                for (auto dep : depsForImpl)
                {
                    if (dep == interfaceType)
                        hasSelfReference = true;
                    else
                        deps.add(dep);
                }

                if (hasSelfReference)
                    selfRefList.add(impl);
                else
                    nonSelfRefList.add(impl);

                if (!implDeps.containsKey(impl))
                    implDeps.add(impl, depsForImpl);
            }

            dependencyMap.add(interfaceType, deps);
            selfReferentialImpls.add(interfaceType, selfRefList);
            nonSelfReferentialImpls.add(interfaceType, nonSelfRefList);
        }
    }

    void computeStronglyConnectedComponents()
    {
        Dictionary<IRInterfaceType*, Index> indexMap;
        Dictionary<IRInterfaceType*, Index> lowLinkMap;
        HashSet<IRInterfaceType*> onStack;
        List<IRInterfaceType*> stack;
        Index nextIndex = 0;

        for (auto interfaceType : interfaceTypes)
        {
            if (!indexMap.containsKey(interfaceType))
            {
                strongConnect(interfaceType, nextIndex, indexMap, lowLinkMap, onStack, stack);
            }
        }
    }

    void strongConnect(
        IRInterfaceType* interfaceType,
        Index& nextIndex,
        Dictionary<IRInterfaceType*, Index>& indexMap,
        Dictionary<IRInterfaceType*, Index>& lowLinkMap,
        HashSet<IRInterfaceType*>& onStack,
        List<IRInterfaceType*>& stack)
    {
        indexMap.add(interfaceType, nextIndex);
        lowLinkMap.add(interfaceType, nextIndex);
        nextIndex++;

        stack.add(interfaceType);
        onStack.add(interfaceType);

        for (auto dependency : dependencyMap[interfaceType])
        {
            if (!indexMap.containsKey(dependency))
            {
                strongConnect(dependency, nextIndex, indexMap, lowLinkMap, onStack, stack);
                lowLinkMap[interfaceType] =
                    Math::Min(lowLinkMap[interfaceType], lowLinkMap[dependency]);
            }
            else if (onStack.contains(dependency))
            {
                lowLinkMap[interfaceType] =
                    Math::Min(lowLinkMap[interfaceType], indexMap[dependency]);
            }
        }

        if (lowLinkMap[interfaceType] != indexMap[interfaceType])
            return;

        Index componentIndex = interfaceToComponent.getCount();
        List<IRInterfaceType*> componentMembers;
        while (true)
        {
            auto member = stack.getLast();
            stack.removeLast();
            onStack.remove(member);
            componentMembers.add(member);
            interfaceToComponent.add(member, componentIndex);
            if (member == interfaceType)
                break;
        }

        if (componentMembers.getCount() > 1)
        {
            for (auto member : componentMembers)
                interfacesInDependencyCycle.add(member);
        }
    }
};

// Detect and diagnose circular interface conformances before specialization.
// This must run before specializeModule because circular conformance IR causes
// crashes in the IR translate pass during specialization.
void diagnoseCircularConformances(IRModule* module, DiagnosticSink* sink)
{
    InterfaceDependencyAnalysis analysis;
    analysis.build(module);

    if (analysis.interfaceTypes.getCount() == 0)
        return;

    // For each (interface, impl), check if the impl creates a cycle back to
    // its own interface. Self-referential is the trivial case where the impl
    // directly contains its own interface type.
    for (auto interfaceType : analysis.interfaceTypes)
    {
        Index circularCount = 0;

        for (auto impl : analysis.implMap[interfaceType])
        {
            if (analysis.implCreatesCircularConformance(interfaceType, impl))
            {
                sink->diagnose(Diagnostics::CircularConformance{
                    .type = impl,
                    .interfaceType = interfaceType,
                    .location = impl->sourceLoc,
                });
                circularCount++;
            }
        }

        if (circularCount > 0 && circularCount == analysis.implMap[interfaceType].getCount())
        {
            sink->diagnose(Diagnostics::CyclicInterfaceDependency{
                .interfaceType = interfaceType,
            });
        }
    }
}

// inferAnyValueSizeWhereNecessary only runs when diagnoseCircularConformances
// has not reported errors, so circular conformances never reach this point.
// Note: we rebuild InterfaceDependencyAnalysis here rather than threading it
// from diagnoseCircularConformances because the two functions run at different
// pipeline stages and the cost is O(interfaces + witness_tables), which is
// negligible for realistic shader sizes.
void inferAnyValueSizeWhereNecessary(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink)
{
    InterfaceDependencyAnalysis analysis;
    analysis.build(module);

    if (analysis.interfaceTypes.getCount() == 0)
        return;

    // Verify the invariant: diagnoseCircularConformances must have already
    // removed both non-trivial interface SCCs and direct self-only cycles.
    // If this fires, circular conformances are reaching size inference.
    for (auto interfaceType : analysis.interfaceTypes)
    {
        bool participatesInDependencyCycle =
            analysis.interfacesInDependencyCycle.contains(interfaceType);
        auto& selfRefList = analysis.selfReferentialImpls[interfaceType];
        auto& nonSelfRefList = analysis.nonSelfReferentialImpls[interfaceType];
        bool hasOnlySelfReferentialImpls =
            nonSelfRefList.getCount() == 0 && selfRefList.getCount() > 0;

        SLANG_ASSERT(!participatesInDependencyCycle && !hasOnlySelfReferentialImpls);
        if (participatesInDependencyCycle || hasOnlySelfReferentialImpls)
            return;
    }

    // Sort the interface types in topological order.
    // This is necessary because we need to infer the any-value-size of an interface type
    // before we infer the any-value-size of an interface type that depends on it.
    // Note: Self-references are excluded from dependencySet so they don't break the sort.
    //
    List<IRInterfaceType*> sortedInterfaceTypes = sortTopologically(
        analysis.interfaceTypes,
        [&](IRInterfaceType* interfaceType) { return analysis.dependencyMap[interfaceType]; });

    for (auto interfaceType : sortedInterfaceTypes)
    {
        IRIntegerValue existingMaxSize = (IRIntegerValue)kMaxInt; // Default to max int.
        if (auto existingAnyValueDecor = interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            existingMaxSize = existingAnyValueDecor->getSize();
        }

        IRIntegerValue maxAnyValueSize = -1;

        // First pass: Calculate size from non-self-referential implementations.
        // This establishes the base AnyValue size for the interface.
        for (auto implType : analysis.nonSelfReferentialImpls[interfaceType])
        {
            IRSizeAndAlignment sizeAndAlignment;
            getNaturalSizeAndAlignment(
                targetProgram->getTargetReq(),
                (IRType*)implType,
                &sizeAndAlignment);

            maxAnyValueSize = Math::Max(maxAnyValueSize, sizeAndAlignment.size);

            if (existingMaxSize < sizeAndAlignment.size)
            {
                sink->diagnose(Diagnostics::TypeDoesNotFitAnyValueSize{
                    .type = implType,
                    .location = implType->sourceLoc,
                });
                sink->diagnose(Diagnostics::TypeAndLimit{
                    .type = implType,
                    .size = sizeAndAlignment.size,
                    .limit = existingMaxSize,
                    .location = implType->sourceLoc,
                });
            }
        }

        // Set the AnyValue size from non-self-referential impls first,
        // so self-referential impls can use it.
        if (maxAnyValueSize >= 0 && !interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            IRBuilder builder(module);
            builder.addAnyValueSizeDecoration(interfaceType, maxAnyValueSize);
        }

        // Second pass: Calculate size from self-referential implementations.
        // These can now use the AnyValue size set above.
        for (auto implType : analysis.selfReferentialImpls[interfaceType])
        {
            IRSizeAndAlignment sizeAndAlignment;
            getNaturalSizeAndAlignment(
                targetProgram->getTargetReq(),
                (IRType*)implType,
                &sizeAndAlignment);

            maxAnyValueSize = Math::Max(maxAnyValueSize, sizeAndAlignment.size);

            if (existingMaxSize < sizeAndAlignment.size)
            {
                sink->diagnose(Diagnostics::TypeDoesNotFitAnyValueSize{
                    .type = implType,
                    .location = implType->sourceLoc,
                });
                sink->diagnose(Diagnostics::TypeAndLimit{
                    .type = implType,
                    .size = sizeAndAlignment.size,
                    .limit = existingMaxSize,
                    .location = implType->sourceLoc,
                });
            }
        }

        // Should not encounter interface types without any conforming implementations.
        SLANG_ASSERT(maxAnyValueSize >= 0);

        // Update the AnyValue size if self-referential impls require a larger size.
        if (maxAnyValueSize >= 0)
        {
            IRBuilder builder(module);
            if (auto existingDecor = interfaceType->findDecoration<IRAnyValueSizeDecoration>())
            {
                if (existingDecor->getSize() < maxAnyValueSize)
                {
                    existingDecor->removeAndDeallocate();
                    builder.addAnyValueSizeDecoration(interfaceType, maxAnyValueSize);
                }
            }
            else
            {
                builder.addAnyValueSizeDecoration(interfaceType, maxAnyValueSize);
            }
        }
    }
}
} // namespace Slang
