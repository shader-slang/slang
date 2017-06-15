#ifndef BAKER_SL_COMPILED_PROGRAM_H
#define BAKER_SL_COMPILED_PROGRAM_H

#include "../core/basic.h"
#include "diagnostics.h"
#include "syntax.h"
#include "type-layout.h"

namespace Slang
{
    void IndentString(StringBuilder & sb, String src);

    struct EntryPointResult
    {
        String outputSource;
    };

    struct TranslationUnitResult
    {
        String outputSource;
        List<EntryPointResult> entryPoints;
    };

    class CompileResult
    {
    public:
        DiagnosticSink* mSink = nullptr;

        // Per-translation-unit results
        List<TranslationUnitResult> translationUnits;

        CompileResult()
        {}
        ~CompileResult()
        {
        }
        DiagnosticSink * GetErrorWriter()
        {
            return mSink;
        }
        int GetErrorCount()
        {
            return mSink->GetErrorCount();
        }
    };

}

#endif