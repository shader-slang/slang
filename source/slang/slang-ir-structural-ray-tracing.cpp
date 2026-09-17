#include "slang-ir-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-mangle.h"
#include "slang-module.h"

// Structural ray-tracing stage interfaces need identities that survive AST lifetime, module
// serialization, linking, and specialization. The ordinary source module initially lowers every
// interface to `IRInterfaceType`. Once the compiler has verified that it loaded the packaged
// `slang.raytracing` module, this file replaces those ordinary opcodes with stage-specific
// interface opcodes. Downstream IR code can then recognize a stage contract from the instruction
// class instead of depending on source names or AST declaration pointers.

namespace Slang
{

IROp getStructuralRayTracingStageInterfaceOp(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return kIROp_ClosestHitStageInterface;
    case StructuralRayTracingStageKind::AnyHit:
        return kIROp_AnyHitStageInterface;
    case StructuralRayTracingStageKind::Intersection:
        return kIROp_IntersectionStageInterface;
    case StructuralRayTracingStageKind::Miss:
        return kIROp_MissStageInterface;
    case StructuralRayTracingStageKind::Callable:
        return kIROp_CallableStageInterface;
    default:
        return kIROp_Invalid;
    }
}

bool isCompilerOwnedStructuralRayTracingIROp(IROp op)
{
    if (op >= kIROp_FirstRaytracingStageInterface && op <= kIROp_LastRaytracingStageInterface)
        return true;
    return op == kIROp_StructuralRayTracingEntryPointInfoDecoration ||
           op == kIROp_StructuralRayTracingSourceOperationDecoration;
}

void addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRInst* entryPointValue,
    const StructuralRayTracingEntryPointIRInfo& info)
{
    SLANG_RELEASE_ASSERT(
        entryPointValue && info.stageType && info.stageSourceTypeName && info.stageTypeIdentity);

    // `void` is the canonical absent value for stage-specific types. Keeping a fixed operand list
    // makes this decoration stable across stage kinds and module serialization.
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.stageKind)),
        info.stageType,
        info.stageSourceTypeName,
        info.stageTypeIdentity,
        info.contextType ? info.contextType : voidType,
        info.payloadType ? info.payloadType : voidType,
        info.recordType ? info.recordType : voidType,
        info.hitAttributesType ? info.hitAttributesType : voidType,
        info.callableDataType ? info.callableDataType : voidType,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(info.hitAttributesKind)),
    };
    builder.addDecoration(
        entryPointValue,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

void addStructuralRayTracingSourceOperation(
    IRBuilder& builder,
    IRFunc* func,
    StructuralRayTracingSourceOperationKind kind)
{
    SLANG_RELEASE_ASSERT(
        func && kind >= StructuralRayTracingSourceOperationKind::TraceExplicitPayload &&
        kind < StructuralRayTracingSourceOperationKind::Count);

    if (auto existing = func->findDecoration<IRStructuralRayTracingSourceOperationDecoration>())
    {
        auto existingKind = as<IRIntLit>(existing->getOperationKind());
        SLANG_RELEASE_ASSERT(existingKind && existingKind->getValue() == IRIntegerValue(kind));
        return;
    }

    builder.addDecoration(
        func,
        kIROp_StructuralRayTracingSourceOperationDecoration,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(kind)));
}

// Returns the interface type represented by a mangled symbol. A generic interface's symbol names
// the enclosing `IRGeneric`, whose return value is the actual interface-type instruction.
static IRInterfaceType* _findInterfaceType(IRInst* inst)
{
    if (auto generic = as<IRGeneric>(inst))
        inst = findInnerMostGenericReturnVal(generic);
    return as<IRInterfaceType>(inst);
}

// Returns the function represented by a mangled symbol. Generic extension and method parameters
// can wrap the function in one or more `IRGeneric` instructions, so use the canonical helper that
// follows their return values instead of assuming a fixed nesting depth.
static IRFunc* _findFunc(IRInst* inst)
{
    if (auto generic = as<IRGeneric>(inst))
        inst = findInnerMostGenericReturnVal(generic);
    return as<IRFunc>(inst);
}

bool identifyStructuralRayTracingStageInterfaces(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind* outMissingStage)
{
    auto irModule = module->getIRModule();
    auto astBuilder = module->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        auto kind = StructuralRayTracingStageKind(i);
        auto interfaceDecl = registry.getStageInterface(kind);
        auto mangledName = getMangledName(astBuilder, interfaceDecl);
        auto symbols = irModule->findSymbolByMangledName(ImmutableHashedString(mangledName));
        auto expectedOp = getStructuralRayTracingStageInterfaceOp(kind);
        bool found = false;

        // Mangled-name lookup can return more than one symbol after module composition. Retag only
        // an ordinary interface or an interface already carrying this exact stage identity; an
        // unrelated instruction with the same symbol is never a valid normalization candidate.
        for (auto symbol : symbols)
        {
            auto interfaceType = _findInterfaceType(symbol);
            if (!interfaceType)
                continue;
            if (interfaceType->getOp() != kIROp_InterfaceType &&
                interfaceType->getOp() != expectedOp)
            {
                continue;
            }

            // All stage-interface ops have the same storage and operand layout as
            // IRInterfaceType. The trusted-module load is the point where the ordinary serialized
            // interface receives its compiler-owned nominal identity.
            interfaceType->m_op = expectedOp;
            found = true;
        }

        if (!found)
        {
            if (outMissingStage)
                *outMissingStage = kind;
            return false;
        }
    }
    return true;
}

bool identifyStructuralRayTracingSourceOperations(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry)
{
    auto irModule = module->getIRModule();
    auto astBuilder = module->getASTBuilder();
    SLANG_AST_BUILDER_RAII(astBuilder);

    IRBuilder builder(irModule);
    builder.setInsertInto(irModule->getModuleInst());

    List<StructuralRayTracingSourceOperation> operations;
    registry.getSourceOperations(operations);
    for (auto operation : operations)
    {
        auto mangledName = getMangledName(astBuilder, operation.functionDecl);
        auto symbols = irModule->findSymbolByMangledName(ImmutableHashedString(mangledName));
        bool found = false;
        for (auto symbol : symbols)
        {
            auto func = _findFunc(symbol);
            if (!func)
                continue;

            addStructuralRayTracingSourceOperation(builder, func, operation.kind);
            found = true;
        }
        if (!found)
            return false;
    }
    return true;
}

} // namespace Slang
