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

bool hasDependencyPath(
    IRInterfaceType* source,
    IRInterfaceType* target,
    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>>& dependencyMap,
    HashSet<IRInterfaceType*>& visited)
{
    if (source == target)
        return true;

    if (visited.contains(source))
        return false;
    visited.add(source);

    if (auto deps = dependencyMap.tryGetValue(source))
    {
        for (auto dependency : *deps)
        {
            if (hasDependencyPath(dependency, target, dependencyMap, visited))
                return true;
        }
    }

    return false;
}

// Helper: collect interface types and their implementations from the module.
// Shared between diagnoseCircularConformances and inferAnyValueSizeWhereNecessary.
static void collectInterfaceTypesAndImplementations(
    IRModule* module,
    HashSet<IRInterfaceType*>& interfaceTypes,
    Dictionary<IRInterfaceType*, List<IRInst*>>& mapInterfaceToImplementations)
{
    HashSet<IRInst*> implementedInterfaces;
    for (auto inst : module->getGlobalInsts())
    {
        if (inst->getOp() == kIROp_WitnessTable)
        {
            auto interfaceType =
                cast<IRWitnessTableType>(inst->getDataType())->getConformanceType();
            implementedInterfaces.add(interfaceType);
        }
    }

    for (auto inst : module->getGlobalInsts())
    {
        if (inst->getOp() == kIROp_InterfaceType)
        {
            auto interfaceType = cast<IRInterfaceType>(inst);

            if (isComInterfaceType((IRType*)interfaceType))
                continue;

            if (interfaceType->findDecoration<IRBuiltinDecoration>())
                continue;

            if (!implementedInterfaces.contains(interfaceType))
                continue;

            interfaceTypes.add(interfaceType);
        }
    }

    for (auto interfaceType : interfaceTypes)
    {
        IRWitnessTableType* witnessTableType = nullptr;
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

        SLANG_ASSERT(witnessTableType);

        List<IRInst*> implList;
        for (auto use = witnessTableType->firstUse; use; use = use->nextUse)
        {
            auto witnessTable = as<IRWitnessTable>(use->getUser());
            if (!witnessTable || witnessTable->getDataType() != witnessTableType)
                continue;

            auto concreteImpl = witnessTable->getConcreteType();

            if (concreteImpl->getParent() == module->getModuleInst())
                implList.add(concreteImpl);
        }

        mapInterfaceToImplementations.add(interfaceType, implList);
    }
}

void diagnoseCircularConformances(
    IRModule* module,
    DiagnosticSink* sink)
{
    HashSet<IRInterfaceType*> interfaceTypes;
    Dictionary<IRInterfaceType*, List<IRInst*>> mapInterfaceToImplementations;
    collectInterfaceTypesAndImplementations(module, interfaceTypes, mapInterfaceToImplementations);

    if (interfaceTypes.getCount() == 0)
        return;

    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>> interfaceDependencyMap;
    Dictionary<IRInterfaceType*, List<IRInst*>> selfReferentialImpls;
    Dictionary<IRInterfaceType*, List<IRInst*>> nonSelfReferentialImpls;

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
                    hasSelfReference = true;
                else
                    dependencySet.add(dependency);
            }

            if (hasSelfReference)
                selfRefList.add(impl);
            else
                nonSelfRefList.add(impl);
        }

        interfaceDependencyMap.add(interfaceType, dependencySet);
        selfReferentialImpls.add(interfaceType, selfRefList);
        nonSelfReferentialImpls.add(interfaceType, nonSelfRefList);

        // Diagnose each self-referential implementation.
        for (auto impl : selfRefList)
        {
            sink->diagnose(Diagnostics::CircularConformance{
                .type = impl,
                .interfaceType = interfaceType,
                .location = impl->sourceLoc,
            });
        }

        if (nonSelfRefList.getCount() == 0 && selfRefList.getCount() > 0)
        {
            sink->diagnose(Diagnostics::CyclicInterfaceDependency{
                .interfaceType = interfaceType,
            });
        }
    }

    // Diagnose non-self-referential implementations that participate in
    // cross-interface cycles.
    for (auto interfaceType : interfaceTypes)
    {
        for (auto impl : nonSelfReferentialImpls[interfaceType])
        {
            auto deps = findDependenciesOfTypeInSet((IRType*)impl, interfaceTypes);
            for (auto dep : deps)
            {
                if (dep == interfaceType)
                    continue;

                HashSet<IRInterfaceType*> visited;
                if (hasDependencyPath(dep, interfaceType, interfaceDependencyMap, visited))
                {
                    sink->diagnose(Diagnostics::CircularConformance{
                        .type = impl,
                        .interfaceType = interfaceType,
                        .location = impl->sourceLoc,
                    });
                    break;
                }
            }
        }
    }
}

void inferAnyValueSizeWhereNecessary(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink)
{
    HashSet<IRInterfaceType*> interfaceTypes;
    Dictionary<IRInterfaceType*, List<IRInst*>> mapInterfaceToImplementations;
    collectInterfaceTypesAndImplementations(module, interfaceTypes, mapInterfaceToImplementations);

    if (interfaceTypes.getCount() == 0)
        return;

    Dictionary<IRInterfaceType*, HashSet<IRInterfaceType*>> interfaceDependencyMap;
    Dictionary<IRInterfaceType*, List<IRInst*>> selfReferentialImpls;
    Dictionary<IRInterfaceType*, List<IRInst*>> nonSelfReferentialImpls;
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
                    hasSelfReference = true;
                else
                    dependencySet.add(dependency);
            }

            if (hasSelfReference)
                selfRefList.add(impl);
            else
                nonSelfRefList.add(impl);
        }

        interfaceDependencyMap.add(interfaceType, dependencySet);
        selfReferentialImpls.add(interfaceType, selfRefList);
        nonSelfReferentialImpls.add(interfaceType, nonSelfRefList);

        if (nonSelfRefList.getCount() == 0 && selfRefList.getCount() > 0)
            fullyCircularInterfaces.add(interfaceType);
    }

    // Track cyclic implementations to exclude from "does not fit" diagnostics.
    HashSet<IRInst*> cyclicImpls;

    for (auto interfaceType : interfaceTypes)
    {
        for (auto impl : selfReferentialImpls[interfaceType])
            cyclicImpls.add(impl);
    }

    for (auto interfaceType : interfaceTypes)
    {
        for (auto impl : nonSelfReferentialImpls[interfaceType])
        {
            auto deps = findDependenciesOfTypeInSet((IRType*)impl, interfaceTypes);
            for (auto dep : deps)
            {
                if (dep == interfaceType)
                    continue;

                HashSet<IRInterfaceType*> visited;
                if (hasDependencyPath(dep, interfaceType, interfaceDependencyMap, visited))
                {
                    cyclicImpls.add(impl);
                    break;
                }
            }
        }
    }

    // Sort the interface types in topological order.
    List<IRInterfaceType*> sortedInterfaceTypes = sortTopologically(
        interfaceTypes,
        [&](IRInterfaceType* interfaceType) { return interfaceDependencyMap[interfaceType]; });

    for (auto interfaceType : sortedInterfaceTypes)
    {
        // Skip interfaces where all impls are circular — size inference is impossible.
        if (fullyCircularInterfaces.contains(interfaceType))
            continue;

        IRIntegerValue existingMaxSize = (IRIntegerValue)kMaxInt;
        if (auto existingAnyValueDecor = interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            existingMaxSize = existingAnyValueDecor->getSize();
        }

        IRIntegerValue maxAnyValueSize = -1;

        // Calculate size from non-cyclic implementations.
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

        // Infer the AnyValue size from valid, non-cyclic implementations only.
        if (maxAnyValueSize >= 0 && !interfaceType->findDecoration<IRAnyValueSizeDecoration>())
        {
            IRBuilder builder(module);
            builder.addAnyValueSizeDecoration(interfaceType, maxAnyValueSize);
        }

        // All implementations may have been filtered out because they were
        // diagnosed as cross-interface cycles. In that case there is nothing
        // left to infer for this interface.
        if (maxAnyValueSize < 0)
            continue;

        // Update the AnyValue size if the inferred size is larger.
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
