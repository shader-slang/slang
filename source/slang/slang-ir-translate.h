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

void initializeTranslationDictionary(IRModule* module);

void clearTranslationDictionary(IRModule* module);
struct TranslationContext
{
    // AD 2.0 Translators

public:
    TranslationContext(TargetProgram* target, IRModule* module, DiagnosticSink* inSink)
        : irModule(module)
        , sink(inSink)
        , targetProgram(target)
        , autodiffContext(target, module->getModuleInst())
    {
        initializeTranslationDictionary(module);
    }

    IRInst* maybeTranslateInst(IRInst* inst);

    IRInst* resolveInst(IRInst* inst);

    IRModule* getModule() const { return irModule; }

    TargetProgram* getTargetProgram() const { return targetProgram; }

private:
    IRModule* irModule;

    TargetProgram* targetProgram;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink* sink;

    // Shared context.
    AutoDiffSharedContext autodiffContext;

    // Shallow translation.
    bool m_translateWitnessesOnly = false;
};

}; // namespace Slang