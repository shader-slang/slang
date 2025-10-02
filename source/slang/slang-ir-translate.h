#pragma once

#include "slang-compiler.h"
#include "slang-ir-autodiff-fwd.h"
#include "slang-ir-autodiff-pairs.h"
#include "slang-ir-autodiff-rev.h"
#include "slang-ir-autodiff.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-single-return.h"
#include "slang-ir-ssa-simplification.h"
#include "slang-ir-validate.h"
#include "slang-ir.h"

namespace Slang
{

struct TranslationContext
{
    // AD 2.0 Translators

public:
    TranslationContext(IRModule* module, DiagnosticSink* inSink)
        : irModule(module)
        , sink(inSink)
        , autodiffContext(module->getModuleInst())
        , pairBuilderStorage(&autodiffContext)
        , diffTypeConformanceContext(&autodiffContext)
    {
    }

    IRInst* maybeTranslateInst(IRInst* inst);

private:
    IRModule* irModule;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink* sink;

    // Shared context.
    AutoDiffSharedContext autodiffContext;

    // Builder for dealing with differential pair types.
    DifferentialPairTypeBuilder pairBuilderStorage;

    DifferentiableTypeConformanceContext diffTypeConformanceContext;
};

}; // namespace Slang