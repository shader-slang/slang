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
        : irModule(module), sink(inSink), autodiffContext(module->getModuleInst())
    {
        if (!module->getTranslationDict())
        {
            IRBuilder builder(module);
            builder.setInsertInto(module);
            auto dict = cast<IRCompilerDictionary>(builder.emitIntrinsicInst(
                builder.getVoidType(),
                kIROp_CompilerDictionary,
                0,
                nullptr));
            module->setTranslationDict(dict);

            builder.setInsertInto(dict);
            builder.emitIntrinsicInst(
                builder.getVoidType(),
                kIROp_CompilerDictionaryScope,
                0,
                nullptr);
        }
    }

    IRInst* maybeTranslateInst(IRInst* inst);

    IRInst* resolveInst(IRInst* inst);

    IRModule* getModule() const { return irModule; }

private:
    IRModule* irModule;

    // Diagnostic object from the compile request for
    // error messages.
    DiagnosticSink* sink;

    // Shared context.
    AutoDiffSharedContext autodiffContext;

    // Shallow translation.
    bool m_translateWitnessesOnly = false;
};

}; // namespace Slang