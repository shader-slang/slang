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
    SharedIRBuilder m_sharedIRBuilder;
    Dictionary<IRTargetIntrinsicDecoration*, RefPtr<SpvSnippet>> m_parsedSpvSnippets;
    TargetRequest* m_targetRequest;

    SPIRVEmitSharedContext(IRModule* module, TargetRequest* target)
        : m_sharedIRBuilder(module)
        , m_targetRequest(target)
    {}

    SpvSnippet* getParsedSpvSnippet(IRTargetIntrinsicDecoration* intrinsic)
    {
        RefPtr<SpvSnippet> snippet;
        if (m_parsedSpvSnippets.TryGetValue(intrinsic, snippet))
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
    DiagnosticSink*         sink);

}
