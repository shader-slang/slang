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
    Dictionary<IRTargetIntrinsicDecoration*, RefPtr<SpvSnippet>> m_parsedSpvSnippets;
    SPIRVEmitSharedContext(IRModule* module, TargetRequest* target)
        : m_irModule(module), m_targetRequest(target)
    {}

    SpvSnippet* getParsedSpvSnippet(IRTargetIntrinsicDecoration* intrinsic)
    {
        RefPtr<SpvSnippet> snippet;
        if (m_parsedSpvSnippets.tryGetValue(intrinsic, snippet))
        {
            return snippet.Ptr();
        }
        snippet = SpvSnippet::parse(intrinsic->getDefinition());
        m_parsedSpvSnippets[intrinsic] = snippet;
        return snippet;
    }
};

void legalizeIRForSPIRV(
    SPIRVEmitSharedContext* context,
    IRModule*               module,
    const List<IRFunc*>& entryPoints,
    CodeGenContext*         codeGenContext);

}
