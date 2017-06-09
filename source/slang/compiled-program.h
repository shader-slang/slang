#ifndef BAKER_SL_COMPILED_PROGRAM_H
#define BAKER_SL_COMPILED_PROGRAM_H

#include "../core/basic.h"
#include "diagnostics.h"
#include "syntax.h"
#include "type-layout.h"

namespace Slang
{
    namespace Compiler
    {
#if 0
        class ShaderMetaData
        {
        public:
            CoreLib::String ShaderName;
            CoreLib::EnumerableDictionary<CoreLib::String, CoreLib::RefPtr<ILModuleParameterSet>> ParameterSets; // bindingName->DescSet
        };

        class StageSource
        {
        public:
            String MainCode;
            List<unsigned char> BinaryCode;
        };

        class CompiledShaderSource
        {
        public:
            EnumerableDictionary<String, StageSource> Stages;
            ShaderMetaData MetaData;
        };
#endif

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

#if 0
            String ScheduleFile;
            RefPtr<ILProgram> Program;
            EnumerableDictionary<String, CompiledShaderSource> CompiledSource; // shader -> stage -> code
#endif

            // Per-translation-unit results
            List<TranslationUnitResult> translationUnits;

#if 0
            void PrintDiagnostics()
            {
                for (int i = 0; i < sink.diagnostics.Count(); i++)
                {
                    fprintf(stderr, "%S(%d): %s %d: %S\n",
                        sink.diagnostics[i].Position.FileName.ToWString(),
                        sink.diagnostics[i].Position.Line,
                        getSeverityName(sink.diagnostics[i].severity),
                        sink.diagnostics[i].ErrorID,
                        sink.diagnostics[i].Message.ToWString());
                }
            }
#endif

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
}

#endif