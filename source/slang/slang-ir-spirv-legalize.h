// slang-ir-spirv-legalize.h
#pragma once
#include "../core/slang-basic.h"
#include "slang-ir-spirv-snippet.h"
#include "slang-ir-insts.h"

namespace Slang
{

class DiagnosticSink;

struct IRFunc;
struct IRModule;
class TargetRequest;

struct SPIRVEmitSharedContext
{
    IRModule* m_irModule;
    TargetRequest* m_targetRequest;
    TargetProgram* m_targetProgram;
    Dictionary<IRTargetIntrinsicDecoration*, RefPtr<SpvSnippet>> m_parsedSpvSnippets;

    Dictionary<IRInst*, HashSet<IRFunc*>> m_referencingEntryPoints; // The entry-points that directly or transitively reference this global inst.

    DiagnosticSink* m_sink;
    const SPIRVCoreGrammarInfo* m_grammarInfo;
    IRInst* m_voidType;

    SPIRVEmitSharedContext(IRModule* module, TargetProgram* program, DiagnosticSink* sink)
        : m_irModule(module),
          m_targetProgram(program),
          m_targetRequest(program->getTargetReq()),
          m_sink(sink),
          m_grammarInfo(&module->getSession()->getSPIRVCoreGrammarInfo())
    {
        IRBuilder builder(module);
        builder.setInsertInto(module);
        m_voidType = builder.getVoidType();
    }
    SpvSnippet* getParsedSpvSnippet(IRTargetIntrinsicDecoration* intrinsic);
};

void legalizeIRForSPIRV(
    SPIRVEmitSharedContext* context,
    IRModule*               module,
    const List<IRFunc*>& entryPoints,
    CodeGenContext*         codeGenContext);

}
