// slang-ir-cuda-byref-entry-point-params.cpp
#include "slang-ir-cuda-byref-entry-point-params.h"

#include "slang-ir-insts.h"
#include "slang-ir-transform-params-to-constref.h"
#include "slang-ir.h"

namespace Slang
{

/// Returns the parameter-group type layout recorded for `param` at parameter-binding
/// time, or null if the parameter was not laid out as a parameter group.
static IRParameterGroupTypeLayout* findParameterGroupTypeLayout(IRParam* param)
{
    auto layoutDecoration = param->findDecoration<IRLayoutDecoration>();
    if (!layoutDecoration)
        return nullptr;
    auto varLayout = as<IRVarLayout>(layoutDecoration->getLayout());
    if (!varLayout)
        return nullptr;
    return as<IRParameterGroupTypeLayout>(varLayout->getTypeLayout());
}

void reconcileCUDAByRefEntryPointParams(IRModule* module, DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);

    IRBuilder builder(module);
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func)
            continue;
        if (!func->isDefinition())
            continue;
        if (!func->findDecoration<IREntryPointDecoration>())
            continue;

        bool changed = false;
        for (auto param : func->getParams())
        {
            auto groupTypeLayout = findParameterGroupTypeLayout(param);
            if (!groupTypeLayout)
                continue;

            // A parameter that is already group-typed (a source-written
            // `ParameterBlock<T>` / `ConstantBuffer<T>`) already agrees with its
            // layout and flows through the existing pipeline unchanged.
            if (as<IRUniformParameterGroupType>(param->getDataType()))
                continue;

            // The layout says "parameter group" but the IR still passes the element
            // by value: this is a binding-time by-reference decision (see the pass
            // header comment); retype the parameter and rewrite its value uses into
            // loads through addresses.
            auto paramBlockType = builder.getType(kIROp_ParameterBlockType, param->getDataType());
            param->setFullType((IRType*)paramBlockType);
            rewriteValueUsesToAddrUses(builder, param);
            changed = true;

            // The reconciled parameter must now agree with its recorded layout;
            // anything else means the layout and the IR describe different ABIs.
            SLANG_RELEASE_ASSERT(as<IRUniformParameterGroupType>(param->getDataType()));
        }

        if (changed)
            fixUpFuncType(func);
    }
}

} // namespace Slang
