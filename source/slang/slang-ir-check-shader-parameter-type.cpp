#include "slang-ir-check-shader-parameter-type.h"

#include "slang-ir-util.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

template<typename P>
auto isOrContains(P predicate, IRType* type) -> decltype(predicate(type))
{
    HashSet<IRType*> visited;

    auto go = [&visited, &predicate](auto&& self, IRType* type) -> decltype(predicate(type))
    {
        // Prevent infinite recursion by tracking visited types
        if (!visited.add(type))
            return {};

        // Check if the current type matches the predicate
        if (auto result = predicate(type))
            return result;

        // Recursively check struct fields
        if (auto structType = as<IRStructType>(type))
        {
            for (auto field : structType->getFields())
            {
                auto fieldType = field->getFieldType();
                if (auto result = self(self, fieldType))
                    return result;
            }
        }

        return {};
    };

    return go(go, type);
}

enum class MetalInvalidParameterBlockType
{
    None,
    ConstantBufferWithResource,
    SubpassInput,
};

static MetalInvalidParameterBlockType findMetalInvalidParameterBlockType(IRType* elementType)
{
    auto isSubpassInput = [](IRType* t) -> std::optional<IRType*>
    {
        if (as<IRSubpassInputType>(t))
            return t;
        return {};
    };
    if (isOrContains(isSubpassInput, elementType))
        return MetalInvalidParameterBlockType::SubpassInput;

    auto isConstantBufferWithResource = [](IRType* t) -> std::optional<IRType*>
    {
        if (t->getOp() != kIROp_ConstantBufferType)
            return {};
        auto innerType = as<IRType>(t->getOperand(0));
        auto hasResource = [](IRType* inner) -> std::optional<IRType*>
        {
            if (isResourceType(inner))
                return inner;
            return {};
        };
        if (isOrContains(hasResource, innerType))
            return t;
        return {};
    };
    if (isOrContains(isConstantBufferWithResource, elementType))
        return MetalInvalidParameterBlockType::ConstantBufferWithResource;

    return MetalInvalidParameterBlockType::None;
}

static SourceLoc pickDiagnosticLoc(IRInst* inst)
{
    for (auto use = inst->firstUse; use; use = use->nextUse)
    {
        auto user = use->getUser();
        if (user->sourceLoc.isValid())
            return user->sourceLoc;
    }
    return inst->sourceLoc;
}

void checkForInvalidShaderParameterTypeForMetal(IRModule* module, DiagnosticSink* sink)
{
    for (auto inst : module->getGlobalInsts())
    {
        if (inst->getOp() != kIROp_ParameterBlockType)
            continue;

        auto elementType = as<IRType>(inst->getOperand(0));
        auto loc = pickDiagnosticLoc(inst);
        switch (findMetalInvalidParameterBlockType(elementType))
        {
        case MetalInvalidParameterBlockType::SubpassInput:
            sink->diagnose(
                Diagnostics::SubpassInputInParameterBlockNotAllowedOnMetal{.location = loc});
            break;
        case MetalInvalidParameterBlockType::ConstantBufferWithResource:
            sink->diagnose(
                Diagnostics::ResourceTypesInConstantBufferInParameterBlockNotAllowedOnMetal{
                    .location = loc});
            break;
        case MetalInvalidParameterBlockType::None:
            break;
        }
    }
}
void checkForInvalidShaderParameterType(
    IRModule* module,
    TargetRequest* target,
    DiagnosticSink* sink)
{
    if (isMetalTarget(target))
        checkForInvalidShaderParameterTypeForMetal(module, sink);
}
} // namespace Slang
