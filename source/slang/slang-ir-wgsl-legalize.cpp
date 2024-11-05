#include "slang-ir-wgsl-legalize.h"

#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-parameter-binding.h"

namespace Slang
{

struct EntryPointInfo
{
    IRFunc* entryPointFunc;
    IREntryPointDecoration* entryPointDecor;
};

struct SystemValLegalizationWorkItem
{
    IRInst* var;
    String attrName;
    UInt attrIndex;
};

struct WGSLSystemValueInfo
{
    String wgslSystemValueName;
    SystemValueSemanticName wgslSystemValueNameEnum;
    ShortList<IRType*> permittedTypes;
    bool isUnsupported = false;
};

struct LegalizeWGSLEntryPointContext
{
    LegalizeWGSLEntryPointContext(DiagnosticSink* sink, IRModule* module)
        : m_sink(sink), m_module(module)
    {
    }

    DiagnosticSink* m_sink;
    IRModule* m_module;

    std::optional<SystemValLegalizationWorkItem> makeSystemValWorkItem(IRInst* var);
    void legalizeSystemValue(EntryPointInfo entryPoint, SystemValLegalizationWorkItem& workItem);
    List<SystemValLegalizationWorkItem> collectSystemValFromEntryPoint(EntryPointInfo entryPoint);
    void legalizeSystemValueParameters(EntryPointInfo entryPoint);
    void legalizeEntryPointForWGSL(EntryPointInfo entryPoint);
    IRInst* tryConvertValue(IRBuilder& builder, IRInst* val, IRType* toType);
    WGSLSystemValueInfo getSystemValueInfo(
        String inSemanticName,
        String* optionalSemanticIndex,
        IRInst* parentVar);
    void legalizeCall(IRCall* call);
    void legalizeSwitch(IRSwitch* switchInst);
    void legalizeBinaryOp(IRInst* inst);
    void processInst(IRInst* inst);
};

IRInst* LegalizeWGSLEntryPointContext::tryConvertValue(
    IRBuilder& builder,
    IRInst* val,
    IRType* toType)
{
    auto fromType = val->getFullType();
    if (auto fromVector = as<IRVectorType>(fromType))
    {
        if (auto toVector = as<IRVectorType>(toType))
        {
            if (fromVector->getElementCount() != toVector->getElementCount())
            {
                fromType = builder.getVectorType(
                    fromVector->getElementType(),
                    toVector->getElementCount());
                val = builder.emitVectorReshape(fromType, val);
            }
        }
        else if (as<IRBasicType>(toType))
        {
            UInt index = 0;
            val = builder.emitSwizzle(fromVector->getElementType(), val, 1, &index);
            if (toType->getOp() == kIROp_VoidType)
                return nullptr;
        }
    }
    else if (auto fromBasicType = as<IRBasicType>(fromType))
    {
        if (fromBasicType->getOp() == kIROp_VoidType)
            return nullptr;
        if (!as<IRBasicType>(toType))
            return nullptr;
        if (toType->getOp() == kIROp_VoidType)
            return nullptr;
    }
    else
    {
        return nullptr;
    }
    return builder.emitCast(toType, val);
}


WGSLSystemValueInfo LegalizeWGSLEntryPointContext::getSystemValueInfo(
    String inSemanticName,
    String* optionalSemanticIndex,
    IRInst* parentVar)
{
    IRBuilder builder(m_module);
    WGSLSystemValueInfo result = {};
    UnownedStringSlice semanticName;
    UnownedStringSlice semanticIndex;

    auto hasExplicitIndex =
        splitNameAndIndex(inSemanticName.getUnownedSlice(), semanticName, semanticIndex);
    if (!hasExplicitIndex && optionalSemanticIndex)
        semanticIndex = optionalSemanticIndex->getUnownedSlice();

    result.wgslSystemValueNameEnum = convertSystemValueSemanticNameToEnum(semanticName);

    switch (result.wgslSystemValueNameEnum)
    {

    case SystemValueSemanticName::DispatchThreadID:
        {
            result.wgslSystemValueName = toSlice("global_invocation_id");
            IRType* const vec3uType{builder.getVectorType(
                builder.getBasicType(BaseType::UInt),
                builder.getIntValue(builder.getIntType(), 3))};
            result.permittedTypes.add(vec3uType);
        }
        break;

    case SystemValueSemanticName::GroupID:
        {
            result.wgslSystemValueName = toSlice("workgroup_id");
            result.permittedTypes.add(builder.getVectorType(
                builder.getBasicType(BaseType::UInt),
                builder.getIntValue(builder.getIntType(), 3)));
        }
        break;

    case SystemValueSemanticName::GroupThreadID:
        {
            result.wgslSystemValueName = toSlice("local_invocation_id");
            result.permittedTypes.add(builder.getVectorType(
                builder.getBasicType(BaseType::UInt),
                builder.getIntValue(builder.getIntType(), 3)));
        }
        break;

    case SystemValueSemanticName::GSInstanceID:
        {
            // No Geometry shaders in WGSL
            result.isUnsupported = true;
        }
        break;

    default:
        {
            m_sink->diagnose(
                parentVar,
                Diagnostics::unimplementedSystemValueSemantic,
                semanticName);
            return result;
        }
    }

    return result;
}

std::optional<SystemValLegalizationWorkItem> LegalizeWGSLEntryPointContext::makeSystemValWorkItem(
    IRInst* var)
{
    if (auto semanticDecoration = var->findDecoration<IRSemanticDecoration>())
    {
        bool svPrefix =
            semanticDecoration->getSemanticName().startsWithCaseInsensitive(toSlice("sv_"));
        if (svPrefix)
        {
            return {
                {var,
                 String(semanticDecoration->getSemanticName()).toLower(),
                 (UInt)semanticDecoration->getSemanticIndex()}};
        }
    }

    auto layoutDecor = var->findDecoration<IRLayoutDecoration>();
    if (!layoutDecor)
        return {};
    auto sysValAttr = layoutDecor->findAttr<IRSystemValueSemanticAttr>();
    if (!sysValAttr)
        return {};
    auto semanticName = String(sysValAttr->getName());
    auto sysAttrIndex = sysValAttr->getIndex();

    return {{var, semanticName, sysAttrIndex}};
}

List<SystemValLegalizationWorkItem> LegalizeWGSLEntryPointContext::collectSystemValFromEntryPoint(
    EntryPointInfo entryPoint)
{
    List<SystemValLegalizationWorkItem> systemValWorkItems;
    for (auto param : entryPoint.entryPointFunc->getParams())
    {
        auto maybeWorkItem = makeSystemValWorkItem(param);
        if (maybeWorkItem.has_value())
            systemValWorkItems.add(std::move(maybeWorkItem.value()));
    }
    return systemValWorkItems;
}

void LegalizeWGSLEntryPointContext::legalizeSystemValue(
    EntryPointInfo entryPoint,
    SystemValLegalizationWorkItem& workItem)
{
    IRBuilder builder(entryPoint.entryPointFunc);

    auto var = workItem.var;
    auto semanticName = workItem.attrName;

    auto indexAsString = String(workItem.attrIndex);
    auto info = getSystemValueInfo(semanticName, &indexAsString, var);

    if (!info.permittedTypes.getCount())
        return;

    builder.addTargetSystemValueDecoration(var, info.wgslSystemValueName.getUnownedSlice());

    bool varTypeIsPermitted = false;
    auto varType = var->getFullType();
    for (auto& permittedType : info.permittedTypes)
    {
        varTypeIsPermitted = varTypeIsPermitted || permittedType == varType;
    }

    if (!varTypeIsPermitted)
    {
        // Note: we do not currently prefer any conversion
        // example:
        // * allowed types for semantic: `float4`, `uint4`, `int4`
        // * user used, `float2`
        // * Slang will equally prefer `float4` to `uint4` to `int4`.
        //   This means the type may lose data if slang selects `uint4` or `int4`.
        bool foundAConversion = false;
        for (auto permittedType : info.permittedTypes)
        {
            var->setFullType(permittedType);
            builder.setInsertBefore(
                entryPoint.entryPointFunc->getFirstBlock()->getFirstOrdinaryInst());

            // get uses before we `tryConvertValue` since this creates a new use
            List<IRUse*> uses;
            for (auto use = var->firstUse; use; use = use->nextUse)
                uses.add(use);

            auto convertedValue = tryConvertValue(builder, var, varType);
            if (convertedValue == nullptr)
                continue;

            foundAConversion = true;
            copyNameHintAndDebugDecorations(convertedValue, var);

            for (auto use : uses)
                builder.replaceOperand(use, convertedValue);
        }
        if (!foundAConversion)
        {
            // If we can't convert the value, report an error.
            for (auto permittedType : info.permittedTypes)
            {
                StringBuilder typeNameSB;
                getTypeNameHint(typeNameSB, permittedType);
                m_sink->diagnose(
                    var->sourceLoc,
                    Diagnostics::systemValueTypeIncompatible,
                    semanticName,
                    typeNameSB.produceString());
            }
        }
    }
}

void LegalizeWGSLEntryPointContext::legalizeSystemValueParameters(EntryPointInfo entryPoint)
{
    List<SystemValLegalizationWorkItem> systemValWorkItems =
        collectSystemValFromEntryPoint(entryPoint);

    for (auto index = 0; index < systemValWorkItems.getCount(); index++)
    {
        legalizeSystemValue(entryPoint, systemValWorkItems[index]);
    }
}

void LegalizeWGSLEntryPointContext::legalizeEntryPointForWGSL(EntryPointInfo entryPoint)
{
    legalizeSystemValueParameters(entryPoint);
}

void LegalizeWGSLEntryPointContext::legalizeCall(IRCall* call)
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
        switch (arg->getOp())
        {
        case kIROp_Var:
        case kIROp_Param: continue;
        default:          break;
        }

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

void LegalizeWGSLEntryPointContext::legalizeSwitch(IRSwitch* switchInst)
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

void LegalizeWGSLEntryPointContext::legalizeBinaryOp(IRInst* inst)
{
    auto isVectorOrMatrix = [](IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_VectorType:
        case kIROp_MatrixType: return true;
        default:               return false;
        }
    };
    if (isVectorOrMatrix(inst->getOperand(0)->getDataType()) &&
        as<IRBasicType>(inst->getOperand(1)->getDataType()))
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        auto newRhs = builder.emitMakeCompositeFromScalar(
            inst->getOperand(0)->getDataType(),
            inst->getOperand(1));
        builder.replaceOperand(inst->getOperands() + 1, newRhs);
    }
    else if (
        as<IRBasicType>(inst->getOperand(0)->getDataType()) &&
        isVectorOrMatrix(inst->getOperand(1)->getDataType()))
    {
        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        auto newLhs = builder.emitMakeCompositeFromScalar(
            inst->getOperand(1)->getDataType(),
            inst->getOperand(0));
        builder.replaceOperand(inst->getOperands(), newLhs);
    }
}

void LegalizeWGSLEntryPointContext::processInst(IRInst* inst)
{
    switch (inst->getOp())
    {
    case kIROp_Call:   legalizeCall(static_cast<IRCall*>(inst)); break;
    case kIROp_Switch: legalizeSwitch(as<IRSwitch>(inst)); break;

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
    case kIROp_Leq:     legalizeBinaryOp(inst); break;

    default:
        for (auto child : inst->getModifiableChildren())
            processInst(child);
    }
}
void legalizeIRForWGSL(IRModule* module, DiagnosticSink* sink)
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

    LegalizeWGSLEntryPointContext context(sink, module);
    for (auto entryPoint : entryPoints)
        context.legalizeEntryPointForWGSL(entryPoint);

    // Go through every instruction in the module and legalize them as needed.
    context.processInst(module->getModuleInst());
}

} // namespace Slang
