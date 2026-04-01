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
    HashSet<IRInterfaceType*>& onStack,
    List<IRInterfaceType*>& sortedInterfaceTypes,
    const Func<HashSet<IRInterfaceType*>, IRInterfaceType*>& getDependencies)
{
    if (visited.contains(interfaceType))
        return;

    visited.add(interfaceType);
    onStack.add(interfaceType);

    for (auto dependency : getDependencies(interfaceType))
    {
        // Skip back-edges to prevent infinite recursion on cycles.
        if (onStack.contains(dependency))
            continue;
        _sortTopologically(dependency, visited, onStack, sortedInterfaceTypes, getDependencies);
    }

    onStack.remove(interfaceType);
    sortedInterfaceTypes.add(interfaceType);
}

List<IRInterfaceType*> sortTopologically(
    HashSet<IRInterfaceType*> interfaceTypes,
    const Func<HashSet<IRInterfaceType*>, IRInterfaceType*>& getDependencies)
{
    List<IRInterfaceType*> sortedInterfaceTypes;
    HashSet<IRInterfaceType*> visited;
    HashSet<IRInterfaceType*> onStack;
    for (auto interfaceType : interfaceTypes)
    {
        _sortTopologically(interfaceType, visited, onStack, sortedInterfaceTypes, getDependencies);
    }
    return sortedInterfaceTypes;
}

// Detect which interfaces participate in cycles in the dependency graph.
// Uses DFS with an explicit stack so that when a back-edge is found,
// ALL nodes on the stack from the back-edge target onwards are marked
// as cyclic (not just the endpoints).
void _findCyclicInterfaces(
    IRInterfaceType* interfaceType,
    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>>& dependencyMap,
    HashSet<IRInterfaceType*>& visited,
    HashSet<IRInterfaceType*>& onStackSet,
    List<IRInterfaceType*>& dfsStack,
    HashSet<IRInterfaceType*>& cyclicInterfaces)
{
    visited.add(interfaceType);
    onStackSet.add(interfaceType);
    dfsStack.add(interfaceType);

    if (auto deps = dependencyMap.tryGetValue(interfaceType))
    {
        for (auto dependency : *deps)
        {
            if (!visited.contains(dependency))
            {
                _findCyclicInterfaces(
                    dependency,
                    dependencyMap,
                    visited,
                    onStackSet,
                    dfsStack,
                    cyclicInterfaces);
            }
            else if (onStackSet.contains(dependency))
            {
                // Back edge found: mark ALL nodes on the stack from
                // the target (dependency) to the current node.
                for (Index i = dfsStack.getCount() - 1; i >= 0; i--)
                {
                    cyclicInterfaces.add(dfsStack[i]);
                    if (dfsStack[i] == dependency)
                        break;
                }
            }
        }
    }

    dfsStack.removeLast();
    onStackSet.remove(interfaceType);
}

HashSet<IRInterfaceType*> findCyclicInterfaces(
    HashSet<IRInterfaceType*>& interfaceTypes,
    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>>& dependencyMap)
{
    HashSet<IRInterfaceType*> visited;
    HashSet<IRInterfaceType*> onStackSet;
    List<IRInterfaceType*> dfsStack;
    HashSet<IRInterfaceType*> cyclicInterfaces;

    for (auto interfaceType : interfaceTypes)
    {
        if (!visited.contains(interfaceType))
        {
            _findCyclicInterfaces(
                interfaceType,
                dependencyMap,
                visited,
                onStackSet,
                dfsStack,
                cyclicInterfaces);
        }
    }

    return cyclicInterfaces;
}

void inferAnyValueSizeWhereNecessary(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink)
{
    // Go through the global insts and collect all interface types.
    // For each interface type, infer its any-value-size, by looking up
    // all witness tables whose conformance type matches the interface type.
    // then using _calcNaturalSizeAndAlignment to find the max size.
    //
    // Note: we only infer any-value-size for interface types that are used
    // as a generic type parameter, because we don't want to infer any-value-size
    // for interface types that are used as a witness table type.
    //

    HashSet<IRInst*> implementedInterfaces;
    // Add all interface type that are implemented by at least one type to a set.
    for (auto inst : module->getGlobalInsts())
    {
        if (inst->getOp() == kIROp_WitnessTable)
        {
            auto interfaceType =
                cast<IRWitnessTableType>(inst->getDataType())->getConformanceType();
            implementedInterfaces.add(interfaceType);
        }
    }

    // Collect all interface types that require inference.
    HashSet<IRInterfaceType*> interfaceTypes;
    for (auto inst : module->getGlobalInsts())
    {
        if (inst->getOp() == kIROp_InterfaceType)
        {
            auto interfaceType = cast<IRInterfaceType>(inst);

            // Do not infer anything for COM interfaces.
            if (isComInterfaceType((IRType*)interfaceType))
                continue;

            // Also skip builtin types.
            if (interfaceType->findDecoration<IRBuiltinDecoration>())
                continue;

            /* If the interface already has an explicit any-value-size, don't infer anything.
            if (interfaceType->findDecoration<IRAnyValueSizeDecoration>())
                continue;
            */

            // Skip interfaces that are not implemented by any type.
            if (!implementedInterfaces.contains(interfaceType))
                continue;

            interfaceTypes.add(interfaceType);
        }
    }

    Dictionary<IRInterfaceType*, List<IRInst*>> mapInterfaceToImplementations;

    // Collect all concrete types that conform to this interface type.
    for (auto interfaceType : interfaceTypes)
    {
        IRWitnessTableType* witnessTableType = nullptr;
        // Find witness table type corresponding to this interface.
        for (auto use = interfaceType->firstUse; use; use = use->nextUse)
        {
            if (auto _witnessTableType = as<IRWitnessTableType>(use->getUser()))
            {
                if (_witnessTableType->getConformanceType() == interfaceType &&
                    _witnessTableType->hasUses())
                {
                    witnessTableType = _witnessTableType;
                    break;
                }
            }
        }

        // If we hit this case, we have an interface without any conforming implementations.
        // This case should be handled before this point.
        //
        SLANG_ASSERT(witnessTableType);

        List<IRInst*> implList;

        // Walk through all the uses of this witness table type to find the witness tables.
        for (auto use = witnessTableType->firstUse; use; use = use->nextUse)
        {
            auto witnessTable = as<IRWitnessTable>(use->getUser());
            if (!witnessTable || witnessTable->getDataType() != witnessTableType)
                continue;

            auto concreteImpl = witnessTable->getConcreteType();

            // Only consider implementations at the top-level (ignore those nested
            // in generics)
            //
            if (concreteImpl->getParent() == module->getModuleInst())
                implList.add(concreteImpl);
        }

        mapInterfaceToImplementations.add(interfaceType, implList);
    }

    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>> interfaceDependencyMap;
    // Track which implementations have self-referential dependencies on their own interface.
    Dictionary<IRInterfaceType*, List<IRInst*>> selfReferentialImpls;
    Dictionary<IRInterfaceType*, List<IRInst*>> nonSelfReferentialImpls;
    // Interfaces where ALL impls are self-referential (no base case for size inference).
    HashSet<IRInterfaceType*> fullyCircularInterfaces;

    // Collect dependencies for each interface, separating self-referential implementations.
    for (auto interfaceType : interfaceTypes)
    {
        HashSet<IRInterfaceType*> dependencySet;
        List<IRInst*> selfRefList;
        List<IRInst*> nonSelfRefList;

        for (auto impl : mapInterfaceToImplementations[interfaceType])
        {
            auto dependencies = findDependenciesOfTypeInSet((IRType*)impl, interfaceTypes);
            bool hasSelfReference = false;
            for (auto dependency : dependencies)
            {
                if (dependency == interfaceType)
                {
                    hasSelfReference = true;
                }
                else
                {
                    dependencySet.add(dependency);
                }
            }

            if (hasSelfReference)
                selfRefList.add(impl);
            else
                nonSelfRefList.add(impl);
        }

        interfaceDependencyMap.add(interfaceType, dependencySet);
        selfReferentialImpls.add(interfaceType, selfRefList);
        nonSelfReferentialImpls.add(interfaceType, nonSelfRefList);

        // Diagnose each self-referential implementation: a struct that conforms to
        // an interface and contains a field of that same interface type creates an
        // inherently unsatisfiable layout constraint (the struct must be larger than
        // the AnyValue it contains, but the AnyValue must fit the struct).
        for (auto impl : selfRefList)
        {
            sink->diagnose(Diagnostics::CircularConformance{
                .type = impl,
                .interfaceType = interfaceType,
                .location = impl->sourceLoc,
            });
        }

        // If ALL implementations are self-referential, there's no base case and
        // AnyValue size cannot be calculated at all.
        if (nonSelfRefList.getCount() == 0 && selfRefList.getCount() > 0)
        {
            sink->diagnose(Diagnostics::CyclicInterfaceDependency{
                .interfaceType = interfaceType,
            });
            fullyCircularInterfaces.add(interfaceType);
        }
    }

    // Detect interfaces that participate in cross-interface dependency cycles.
    // E.g., IFoo depends on IBar (via FooImpl containing IBar) and IBar depends on
    // IFoo (via BarImpl containing IFoo).
    HashSet<IRInterfaceType*> cyclicInterfaces =
        findCyclicInterfaces(interfaceTypes, interfaceDependencyMap);

    // Track all implementations diagnosed with CircularConformance so we can
    // exclude them from the confusing "does not fit" (E41011) diagnostic later,
    // and avoid diagnosing the same impl twice.
    HashSet<IRInst*> cyclicImpls;

    // Record self-referential impls that were already diagnosed above.
    for (auto interfaceType : interfaceTypes)
    {
        for (auto impl : selfReferentialImpls[interfaceType])
            cyclicImpls.add(impl);
    }

    // Diagnose non-self-referential implementations that participate in
    // cross-interface cycles.
    for (auto interfaceType : cyclicInterfaces)
    {
        for (auto impl : nonSelfReferentialImpls[interfaceType])
        {
            auto deps = findDependenciesOfTypeInSet((IRType*)impl, interfaceTypes);
            for (auto dep : deps)
            {
                if (dep != interfaceType && cyclicInterfaces.contains(dep))
                {
                    sink->diagnose(Diagnostics::CircularConformance{
                        .type = impl,
                        .interfaceType = interfaceType,
                        .location = impl->sourceLoc,
                    });
                    cyclicImpls.add(impl);
                    break;
                }
            }
        }
    }

    // Sort the interface types in topological order.
    // This is necessary because we need to infer the any-value-size of an interface type
    // before we infer the any-value-size of an interface type that depends on it.
    // Note: Self-references are excluded from dependencySet so they don't break the sort.
    //
    List<IRInterfaceType*> sortedInterfaceTypes = sortTopologically(
        interfaceTypes,
        [&](IRInterfaceType* interfaceType) { return interfaceDependencyMap[interfaceType]; });

    for (auto interfaceType : sortedInterfaceTypes)
    {
        // Skip interfaces where all impls are circular — size inference is impossible
        // and we already emitted CyclicInterfaceDependency.
        if (fullyCircularInterfaces.contains(interfaceType))
            continue;

        IRIntegerValue existingMaxSize = (IRIntegerValue)kMaxInt; // Default to max int.
        if (auto existingAnyValueDecor = interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            existingMaxSize = existingAnyValueDecor->getSize();
        }

        IRIntegerValue maxAnyValueSize = -1;

        // First pass: Calculate size from non-self-referential implementations.
        // This establishes the base AnyValue size for the interface.
        for (auto implType : nonSelfReferentialImpls[interfaceType])
        {
            if (cyclicImpls.contains(implType))
                continue;

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
        // Skip cyclic impls (already diagnosed with CircularConformance).
        for (auto implType : selfReferentialImpls[interfaceType])
        {
            if (cyclicImpls.contains(implType))
                continue;

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
}; // namespace Slang
